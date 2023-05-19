#version 450
#extension GL_GOOGLE_include_directive : enable

#include "get_frag_color.glsl"

#define CONSTANT_MUL 0.0357f

layout(location = 0) in vec3 color;
layout(location = 0) out vec4 fColor;

void main()
{
	fColor = vec4(get_frag_color(color) + vec3(CONSTANT_MUL), 1.0f);
}
