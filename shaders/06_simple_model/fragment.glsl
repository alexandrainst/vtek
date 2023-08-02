#version 450
#extension GL_GOOGLE_include_directive : enable

#include "../utils/lighting.glsl"

layout (location = 0) in vec3 worldNormal;
layout (location = 1) in vec3 worldPos;
layout (location = 2) flat in vec3 viewPos;

layout (location = 0) out vec4 fColor;

layout (set = 0, binding = 1) uniform LightUniform
{
	vec4 positionFalloff;
	vec4 colorIntensity;
};


void main()
{
	vec3 result = vec3(0.0f);
	vec3 norm = normalize(worldNormal); // was interpolated.
	vec3 viewDir = normalize(viewPos - worldPos);
	vec3 matCol = vec3(0.45f, 0.45f, 0.45f);

	result += calcPointLight_NoAttenuation(
		positionFalloff.xyz, worldPos, colorIntensity,
		matCol, norm, viewDir);

	fColor = vec4(result, 1.0f);
}
