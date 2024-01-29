#version 450

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTexCoord;

layout (location = 0) out vec2 texCoord;

void main()
{
	texCoord = aTexCoord;
	gl_Position = vec4(aPosition, 0.0f, 1.0f);
}
