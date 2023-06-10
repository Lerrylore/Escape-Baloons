#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set=1, binding = 0) uniform UniformBufferObject {
	float amb;
	float gamma;
	vec3 sColor;
	mat4 mvpMat[20];
	mat4 mMat[20];
	mat4 nMat[20];
} ubo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNorm;
layout(location = 2) in vec4 inTan;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 fragPos;
layout(location = 1) out vec3 fragNorm;
layout(location = 2) out vec4 fragTan;
layout(location = 3) out vec2 fragUV;

void main() {
	gl_Position = ubo.mvpMat[gl_InstanceIndex] * vec4(inPos, 1.0);
	fragPos = (ubo.mMat[gl_InstanceIndex] * vec4(inPos, 1.0)).xyz;
	fragNorm = ((ubo.nMat[gl_InstanceIndex]) * vec4(inNorm, 0.0)).xyz;
	fragTan = vec4(mat3(ubo.nMat[gl_InstanceIndex]) * inTan.xyz, inTan.w);
	fragUV = inUV;
}