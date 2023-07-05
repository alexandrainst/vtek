#include "vtek_vulkan.pch"
#include "vtek_uniform_data.hpp"

#include "vtek_logging.hpp"


using UBType = vtek::UniformBufferType;


uint64_t vtek::get_uniform_buffer_size(vtek::UniformBufferType type)
{
	switch (type)
	{
	case UBType::vec3: return sizeof(glm::vec3);

	default:
		vtek_log_error("vtek_get_uniform_buffer_size(): Invalid enum type!");
		return 0;
	}
}
