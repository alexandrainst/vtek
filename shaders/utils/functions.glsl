//
// Utilities for gamma correction, logarithmic functions, and
// rgb <-> luminance conversion.
const float kGamma = 2.2;
const float kInvGamma = 1.0 / 2.2;

vec3 vec3_pow(vec3 v, float p)
{
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

vec3 linear_to_srgb(vec3 col)
{
	return vec3_pow(col, kInvGamma);
}

vec3 srgb_to_linear(vec3 col)
{
	return vec3_pow(col, kGamma);
}

float saturate(float x)
{
	return clamp(x, 0.0f, 1.0f);
}

vec2 saturate(vec2 x)
{
	return clamp(x, vec2(0.0f), vec2(1.0f));
}

vec3 saturate(vec3 x)
{
	return clamp(x, vec3(0.0f), vec3(1.0f));
}

vec3 mul(mat3 m, vec3 v)
{
	return m * v;
}



//
// Tonemapping operators
//

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
const mat3 ACESInputMat = mat3(
	vec3(0.59719, 0.07600, 0.02840),
    vec3(0.35458, 0.90834, 0.13383),
    vec3(0.04823, 0.01566, 0.83777)
);

// ODT_SAT => XYZ => D60_2_D65 => sRGB
const mat3 ACESOutputMat = mat3(
	vec3(1.60475, -0.10208, -0.00327),
	vec3(-0.53108, 1.10813, -0.07276),
    vec3(-0.07367, -0.00605, 1.07602)
);

vec3 RRTAndODTFit(vec3 col)
{
    vec3 a = col * (col + 0.0245786f) - 0.000090537f;
    vec3 b = col * (0.983729f * col + 0.4329510f) + 0.238081f;
    return a / b;
}

vec3 ACESFitted(vec3 col, float exposure)
{
	col *= exposure;
    col = ACESInputMat * col;

    // Apply RRT and ODT
    col = RRTAndODTFit(col);

    col = mul(ACESOutputMat, col);

    // Clamp to [0, 1]
    col = saturate(col);

    return col;
}
