#version 450

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexCoord;
layout (location = 0) out vec2 texCoord;

layout (set = 0, binding = 0) uniform CameraUniform
{
	mat4 viewProjection;
	vec4 viewPosition; // {x,y,z}: pos; w: unused
};

void main()
{
	texCoord = aTexCoord;
	gl_Position = viewProjection * vec4(aPosition, 1.0f);
}
