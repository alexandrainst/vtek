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
layout (push_constant) uniform PushConstants
{
	// x,y is offset; z is rotation angle
	vec3 moveRotate;
};

void main()
{
	vec2 pos = positions[gl_VertexIndex];
	float sinA = sin(moveRotate.z);
	float cosA = cos(moveRotate.z);

	// A 2d rotation matrix:
	// [ cosA -sinA ]
	// [ sinA  cosA ]
	mat2 rot = mat2(cosA, sinA, -sinA, cosA); // column-major

	vec2 finalPos = moveRotate.xy + (rot * pos);
	gl_Position = vec4(finalPos, 0.0, 1.0);

	color = colors[gl_VertexIndex];
}
