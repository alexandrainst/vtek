#version 450

layout (location = 0) in vec2 aPosition;

// TODO: Uniform buffer for circle center
layout (binding = 0) uniform CircleCenterUniform {
	vec3 circleParams; // x,y is center; z is radius
};

void main()
{
	vec2 pos = (aPosition * circleParams.z) + circleParams.xy;
	gl_Position = vec4(pos, 0.0, 1.0);
}
