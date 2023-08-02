#version 450

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

layout (location = 0) out vec3 worldNormal;
layout (location = 1) out vec3 worldPos;
layout (location = 2) flat out vec3 viewPos;

layout (set = 0, binding = 0) uniform CameraUniform
{
	mat4 viewProjection;
	vec4 viewPosition; // {x,y,z}: pos; w: unused
};

layout (push_constant) uniform PushConstants
{
	mat4 modelMatrix;
};


void main()
{
	// mat3x3(mat4x4) is possible
	worldNormal = mat3(modelMatrix) * aNormal; // TODO: ?
	viewPos = viewPosition.xyz;

	vec4 worldPos4 = modelMatrix * vec4(aPosition, 1.0f);
	worldPos = worldPos.xyz;

	gl_Position = viewProjection * worldPos4;
}
