#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec4 fragTan;
layout(location = 3) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(set=1, binding = 1) uniform sampler2D tex;
layout(set=1, binding = 2) uniform sampler2D normMap;
layout(set=1, binding = 3) uniform sampler2D matMap;

layout( binding = 0) uniform GlobalUniformBufferObject {
	float cosout;
	float cosin;
	vec3 lightPos;
	vec3 lightDir;
	vec3 lightColor;
	vec3 eyePos;
} gubo;
float ggx(vec3 n, vec3 a, float roughness){
    return 2/(1+pow(1+pow(roughness,2)*(1-pow(clamp(dot(n,a),0.0,1.0),2))/(pow(clamp(dot(n,a),0.0,1.0),2)),0.5));
}
vec3 BRDF(vec3 V, vec3 N, vec3 L, vec3 Md, float F0, float metallic, float roughness) {
	//vec3 V  - direction of the viewer
	//vec3 N  - normal vector to the surface
	//vec3 L  - light vector (from the light model)
	//vec3 Md - main color of the surface
	//float F0 - Base color for the Fresnel term
	//float metallic - parameter that mixes the diffuse with the specular term.
	//                 in particular, parmeter K seen in the slides is: float K = 1.0f - metallic;
	//float roughness - Material roughness (parmeter rho in the slides).
	//specular color Ms is not passed, and implicitely considered white: vec3 Ms = vec3(1.0f);

	vec3 Ms = vec3(1.0f);
	float K = 1.0f - metallic;
	vec3 hl = normalize(L+V);
	float D = pow(roughness,2)/(3.14*pow(pow(clamp(dot(N,hl),0.0,1.0),2)*(pow(roughness,2)-1)+1,2));
	float F = F0 + (1-F0)*pow((1-clamp(dot(V,hl),0.0,1.0)),5);
	float G = ggx(N,V, roughness) * ggx(N,L, roughness);
	vec3 diffuse = Md * clamp(dot(L,N),0.0,1.0); 
	vec3 specular = Ms * (D*F*G)/(4*clamp(dot(V,N), 0.0, 1.0));
    vec3 cook = K * diffuse + (1-K)*specular;
	return cook;
}

const float beta = 1.5f;
const float g = 2.5f;

void main() {
	vec3 Norm = normalize(fragNorm);
	vec3 Tan = normalize(fragTan.xyz - Norm * dot(fragTan.xyz, Norm));
	vec3 Bitan = cross(Norm, Tan) * fragTan.w;
	mat3 tbn = mat3(Tan, Bitan, Norm);
	vec4 nMap = texture(normMap, fragUV);
	vec3 N = normalize(tbn * (nMap.rgb * 2.0 - 1.0));
	float cosin = gubo.cosin;
	float cosout = gubo.cosout;

	vec3 albedo = texture(tex, fragUV).rgb;

	vec4 MRAO = texture(matMap, fragUV);
	float roughness = 0;
	float pex = 160.0f;
	float ao = MRAO.b;
	float metallic = MRAO.r;
	
	vec3 lightPos = gubo.lightPos;
	vec3 temp = lightPos - fragPos;
	vec3 lightDir = temp/length(temp);
	vec3 lightColor =  gubo.lightColor * pow(g/length(temp), beta)  * clamp((dot(lightDir,gubo.lightDir) - cosout)/(cosin-cosout), 0.0, 1.0);
	vec3 L = gubo.lightDir;

	vec3 V = normalize(gubo.eyePos - fragPos);

	vec3 DiffSpec = BRDF(V, N, L, albedo, 0.5f, metallic, roughness);
	vec3 Ambient = albedo * 0.05f * ao;
	
	outColor = vec4(clamp(0.95 * DiffSpec * lightColor.rgb + Ambient,0.0,1.0), 1.0f);
}