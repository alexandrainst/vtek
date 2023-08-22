#version 450

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

layout (location = 0) out vec2 texCoord;

layout (set = 0, binding = 0) uniform CameraUniform
{
	mat4 viewProjection;
	vec4 viewPosition; // {x,y,z}: pos; w: exposure
};

layout (push_constant) uniform PushConstants
{
	mat4 modelMatrix;
};

void main()
{
	vec3 worldNormal = mat3(modelMatrix) * aNormal;
	vec4 worldPos = modelMatrix * vec4(aPosition, 1.0f);
	texCoord = aTexCoord;
	gl_Position = viewProjection * worldPos;
}
