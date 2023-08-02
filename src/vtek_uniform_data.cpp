#include "vtek_vulkan.pch"
#include "vtek_uniform_data.hpp"

#include "vtek_logging.hpp"


using UBType = vtek::UniformBufferType;


uint64_t vtek::get_uniform_buffer_size(vtek::UniformBufferType type)
{
	switch (type)
	{
		// TODO: Due to size mismatch, experiment here:
	// case UBType::vec3: return sizeof(glm::vec3);
	// case UBType::mat4: return sizeof(glm::mat4);
	// case UBType::mat4_vec4: return sizeof(glm::mat4)+sizeof(glm::vec4);
	// case UBType::point_light: return 2*sizeof(glm::vec4);

	case UBType::vec3:        return vtek::Uniform_v3::static_size();
	case UBType::mat4:        return vtek::Uniform_m4::static_size();
	case UBType::mat4_vec4:   return vtek::Uniform_m4_v4::static_size();
	case UBType::point_light: return vtek::Uniform_PointLight::static_size();

	default:
		vtek_log_error("vtek_get_uniform_buffer_size(): Invalid enum type!");
		return 0;
	}
}



// ===================== //
// === Memory checks === //
// ===================== //
static_assert(sizeof(vtek::Uniform_v3) == 16);
static_assert(alignof(vtek::Uniform_v3) == 16);
static_assert(offsetof(vtek::Uniform_v3, v3) == 0);

static_assert(sizeof(vtek::Uniform_m4) == 64);
static_assert(alignof(vtek::Uniform_m4) == 16);
static_assert(offsetof(vtek::Uniform_m4, m4) == 0);

static_assert(sizeof(vtek::Uniform_m4_v4) == 80);
static_assert(alignof(vtek::Uniform_m4_v4) == 16);
static_assert(offsetof(vtek::Uniform_m4_v4, m4) == 0);
static_assert(offsetof(vtek::Uniform_m4_v4, v4) == 64);

static_assert(sizeof(vtek::Uniform_PointLight) == 32);
static_assert(alignof(vtek::Uniform_PointLight) == 16);
static_assert(offsetof(vtek::Uniform_PointLight, positionFalloff) == 0);
static_assert(offsetof(vtek::Uniform_PointLight, colorIntensity) == 16);
