#version 450

layout (location = 0) in vec3 aPosition;
layout (location = 0) out vec3 color;

layout (binding = 0) uniform CircleCenterUniform
{
	mat4 viewProjection;
};


void main()
{
	gl_Position = viewProjection * vec4(aPosition, 1.0f);
	color = vec3(1.0f, aPosition.y, aPosition.z);
}
