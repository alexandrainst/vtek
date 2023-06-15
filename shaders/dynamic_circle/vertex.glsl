#version 450

layout (location = 0) in vec2 aPosition;

// TODO: Uniform buffer for circle center

void main()
{
	gl_Position = vec4(aPosition, 0.0, 1.0);
}
