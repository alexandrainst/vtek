#version 450

layout(location = 0) in vec3 color;
layout(location = 0) out vec4 fColor;

// push constants block
layout (push_constant) uniform PushConstants
{
	// x,y is offset; z is rotation angle
	vec3 moveRotate;
};

void main()
{
	float x = (color.x * 0.5) + sin(moveRotate.z);
	float y = color.y + moveRotate.x;
	float z = color.z + moveRotate.y;
	fColor = vec4(x, y, z, 1.0f);
}
