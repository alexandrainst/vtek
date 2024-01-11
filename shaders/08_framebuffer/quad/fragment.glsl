#version 450
#extension GL_GOOGLE_include_directive : enable

#include "../utils/functions.glsl"

layout (location = 0) in vec2 texCoord;
layout (location = 0) out vec4 fColor;

layout (set = 0, binding = 0) uniform CameraUniform
{
	mat4 viewProjection;
	vec4 viewPosition; // {x,y,z}: pos; w: exposure
};

layout (set = 1, binding = 0) uniform sampler2D gAlbedo;
layout (set = 1, binding = 1) uniform sampler2D gNormal;

void main()
{
	vec3 albedo = texture(gAlbedo, texCoord).rgb;
	vec3 normal = texture(gNormal, texCoord).rgb;

	// TODO: Lighting with K point-light sources
	vec3 color = albedo;

	vec3 aces = ACESFitted(color, viewPosition.w);
	vec3 srgb = linear_to_srgb(aces);

	fColor = vec4(srgb, 1.0f);
}
