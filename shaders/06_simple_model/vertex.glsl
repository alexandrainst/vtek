#version 450

layout (location = 0) in vec3 aPosition;
// TODO: Vertex normal

layout (binding = 0) uniform CircleCenterUniform
{
	mat4 viewProjection;
};

layout (push_constant) uniform PushConstants
{
	mat4 modelMatrix;
};


void main()
{
	gl_Position = viewProjection * modelMatrix * vec4(aPosition, 1.0f);
}
