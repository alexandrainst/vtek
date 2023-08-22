#version 450
#extension GL_GOOGLE_include_directive : enable

#include "../utils/functions.glsl"

layout (location = 0) in vec2 texCoord;
layout (location = 0) out vec4 fColor;

layout (set = 0, binding = 1) uniform sampler2D texSampler;

void main()
{
	vec3 tex = texture(texSampler, texCoord).rgb;
	vec3 aces = ACESFitted(tex, 2.5f);
	vec3 srgb = linear_to_srgb(aces);
	fColor = vec4(srgb, 1.0f);
}
