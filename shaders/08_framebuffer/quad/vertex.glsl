#version 450

layout (location = 0) in aPosition;
layout (location = 1) in aTexCoord;

layout (location = 0) out vec2 texCoord;

void main()
{
	texCoord = aTexCoord;
	gl_Position = vec4(aPosition, 0.0f, 1.0f);
}
