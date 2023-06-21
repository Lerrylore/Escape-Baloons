#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec4 fragTan;
layout(location = 3) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(set = 1,binding = 1) uniform sampler2D tex;
layout(set =1 ,binding = 2) uniform sampler2D normMap;
layout(set=1, binding = 3) uniform sampler2D matMap;

layout(binding = 0) uniform GlobalUniformBufferObject {
	float cosout;
	float cosin;
	vec3 lightPos;
	vec3 lightDir;
	vec3 lightColor;
	vec3 eyePos;
} gubo;

vec3 BRDF(vec3 V, vec3 N, vec3 L, vec3 Md, vec3 Ms, float gamma) {
	//vec3 V  - direction of the viewer
	//vec3 N  - normal vector to the surface
	//vec3 L  - light vector (from the light model)
	//vec3 Md - main color of the surface
	//vec3 Ms - specular color of the surface
	//float gamma - Exponent for power specular term
	vec3 lambert = Md*max(dot(L,N), 0.0);
	vec3 blinn = Ms * pow(clamp(dot(N, normalize(L+V)), 0.0 , 1.0), gamma);
	return lambert+blinn;
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

	vec3 lightPos = gubo.lightPos;
	vec3 temp = lightPos - fragPos;
	vec3 lightDir = temp/length(temp);
	vec3 lightColor =  gubo.lightColor * pow(g/length(temp), beta)  * clamp((dot(lightDir,gubo.lightDir) - cosout)/(cosin-cosout), 0.0, 1.0);

	vec3 albedo = texture(tex, fragUV).rgb;

	vec4 MRAO = texture(matMap, fragUV);
	float roughness = MRAO.g;
	float pex = 160.0f;
	float ao = MRAO.b;
	float metallic = MRAO.r;
	
	vec3 L = gubo.lightDir;

	vec3 V = normalize(gubo.eyePos - fragPos);

	vec3 DiffSpec = BRDF(V, N, L, albedo, 0.9f * vec3(metallic), pex);
	vec3 Ambient = albedo * 0.05f * ao;
	
	outColor = vec4(clamp(0.95 * DiffSpec * lightColor.rgb + Ambient,0.0,1.0), 1.0f);
	//outColor = vec4(vec3(clamp(dot(N,L), 0.0f, 1.0f)), 1.0f);
}