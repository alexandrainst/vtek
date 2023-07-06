#version 450

layout (location = 0) in vec3 aPosition;

layout (binding = 0) uniform CircleCenterUniform
{
	mat4 viewProjection;
};

layout (location = 0) out vec3 color;


void main()
{
	gl_Position = viewProjection * vec4(aPosition, 1.0f);
	color = vec3(1.0f, 0.0f, aPosition.z);
}
