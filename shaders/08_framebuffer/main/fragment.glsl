#version 450

layout (location = 0) in vec2 texCoord;
layout (location = 1) in vec3 normal;

layout (location = 0) out vec4 gColor;
layout (location = 1) out vec3 gNormal;

layout (set = 0, binding = 1) uniform sampler2D texSampler;

void main()
{
	vec3 tex = texture(texSampler, texCoord).rgb;
	gColor = vec4(tex, 1.0f); // transparency not possible for deferred

	gNormal = normal;
}
