#version 450

layout(location = 0) out vec3 color;

vec2 positions[3] = vec2[](
	vec2(0.0, -0.5),
	vec2(0.5, 0.5),
	vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

// push constants block
layout (push_constant) uniform constants
{
	// x,y is offset; z is rotation angle
	vec3 moveRotate;
} PushConstants;

void main()
{
	vec2 pos = positions[gl_VertexIndex];
	float sinA = sin(PushConstants.moveRotate.z);
	float cosA = cos(PushConstants.moveRotate.z);

	// A 2d rotation matrix:
	// [ cosA -sinA ]
	// [ sinA  cosA ]
	mat2 rot = mat2(cosA, sinA, -sinA, cosA); // column-major

	vec2 finalPos = PushConstants.moveRotate.xy + (rot * pos);
	gl_Position = vec4(finalPos, 0.0, 1.0);

	//gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
	color = colors[gl_VertexIndex];
}
