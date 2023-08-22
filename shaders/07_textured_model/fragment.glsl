#version 450

layout (location = 0) in vec2 texCoord;
layout (location = 0) out vec4 fColor;

layout (set = 0, binding = 1) uniform sampler2D texSampler;

void main()
{
	vec3 tex = texture(texSampler, texCoord).rgb;
	fColor = vec4(tex, 1.0f);
}
