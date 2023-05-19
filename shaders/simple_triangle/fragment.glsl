#version 450
#extension GL_GOOGLE_include_directive : enable

#include "get_frag_color.glsl"

layout(location = 0) in vec3 color;
layout(location = 0) out vec4 fColor;

void main()
{
	//fColor = vec4(0.0, 0.0, 1.0, 1.0);
	//fColor = vec4(color, 1.0f);
	fColor = vec4(get_frag_color(color), 1.0f);
}
