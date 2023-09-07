#include "color_constants.glsl"

vec3 get_frag_color(vec3 col)
{
	//return col * 0.85f;
	return get_slightly_darker(col);
}
