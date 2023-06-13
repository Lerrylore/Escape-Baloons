#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;


layout(set =1, binding = 1) uniform sampler2D tex;

layout(set=0,binding = 0) uniform GlobalUniformBufferObject {
	float cosout;
	float cosin;
	vec3 lightPos;
	vec3 lightDir;
	vec4 lightColor;
	vec3 eyePos;
} gubo;

vec3 BRDF(vec3 V, vec3 N, vec3 L, vec3 Md, float sigma) {
	//vec3 V  - direction of the viewer
	//vec3 N  - normal vector to the surface
	//vec3 L  - light vector (from the light model)
	//vec3 Md - main color of the surface
	//float sigma - Roughness of the model
	float thetai = acos(dot(L,N));
	float thetar = acos(dot(V,N));
	vec3 vi = normalize(L - (dot(L,N)*N));
	vec3 vr = normalize(V - (dot(V,N)*N));
	float alpha = max(thetai, thetar);
	float beta = min(thetai, thetar);
	float G = max(0, dot(vi,vr));
	vec3 elle = Md * clamp(dot(L,N), 0, 1);
	float A = 1-0.5*pow(sigma,2)/(pow(sigma,2) + 0.09);
	float B = 0.45*pow(sigma,2)/(pow(sigma,2) + 0.09);
	vec3 oren = elle * (A+B*G*sin(alpha) * tan(beta));
	return oren;
}
const float beta = 1.5f;
const float g = 2.5f;

void main() {
	vec3 Norm = normalize(fragNorm);
	vec3 EyeDir = normalize(gubo.eyePos - fragPos);
	float cosin = gubo.cosin;
	float cosout = gubo.cosout;
	
	vec3 lightPos = gubo.lightPos;
	vec3 temp = lightPos - fragPos;
	vec3 lightDir = temp/length(temp);
	vec4 lightColor =  gubo.lightColor * pow(g/length(temp), beta)  * clamp((dot(lightDir,gubo.lightDir) - cosout)/(cosin-cosout), 0.0, 1.0);

	vec3 DiffSpec = BRDF(EyeDir, Norm, lightDir, texture(tex, fragUV).rgb, 0.9f);
	vec3 Ambient = texture(tex, fragUV).rgb * 0.05f;
	
	outColor = vec4(clamp(0.95 * (DiffSpec) * lightColor.rgb + Ambient,0.0,1.0), 1.0f);
}