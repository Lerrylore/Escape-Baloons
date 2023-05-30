#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
	vec3 lightPos;
	vec3 DlightDir;		// direction of the direct light
	vec3 DlightColor;	// color of the direct light	// ambient light
	vec3 eyePos;		// position of the viewer
} gubo;





layout(set = 1, binding = 1) uniform sampler2D tex;
const float beta = 2.0f;
const float g = 1.5f;
const float cosout = 0.70;
const float cosin  = 0.75;


void main() {
	vec3 Norm = normalize(fragNorm);
	vec3 EyeDir = normalize(gubo.eyePos - fragPos);
	
	// replace the following lines with the code to implement a spot light model (based on point lights)
	// with the light color in gubo.lightColor, the position in gubo.lightPos,
	// and the direction in gubo.lightDir.
	// The exponent of the decay is in constant variable beta, and the base distance
	// is in constant g, the cosines of the inner and outer angle are respectively
	// in constants cosin and cosout
	vec3 lightPos = gubo.lightPos;
	vec3 temp = lightPos - fragPos;
	vec3 lightDir = temp/length(temp);
	vec3 lightColor =  gubo.DlightColor * pow(g/length(temp), beta)  * clamp((dot(lightDir,gubo.DlightDir) - cosout)/(cosin-cosout), 0.0, 1.0);

	vec3 Diffuse = texture(tex, fragUV).rgb * 0.99f * clamp(dot(Norm, lightDir),0.0,1.0);
	vec3 Specular = vec3(pow(clamp(dot(Norm, normalize(lightDir + EyeDir)),0.0,1.0), 160.0f));
	vec3 Ambient = texture(tex, fragUV).rgb * 0.01f;
	
	outColor = vec4(clamp((Diffuse + Specular) * lightColor.rgb + Ambient,0.0,1.0), 1.0f);
	
}