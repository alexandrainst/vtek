#include "vtek_vulkan.pch"
#include "vtek_vulkan_types.hpp"

#include "vtek_logging.hpp"


VkSampleCountFlagBits vtek::get_multisample_count(vtek::MultisampleType sample)
{
	switch (sample)
	{
	case vtek::MultisampleType::none:     return VK_SAMPLE_COUNT_1_BIT;
	case vtek::MultisampleType::msaa_x2:  return VK_SAMPLE_COUNT_2_BIT;
	case vtek::MultisampleType::msaa_x4:  return VK_SAMPLE_COUNT_4_BIT;
	case vtek::MultisampleType::msaa_x8:  return VK_SAMPLE_COUNT_8_BIT;
	case vtek::MultisampleType::msaa_x16: return VK_SAMPLE_COUNT_16_BIT;
	case vtek::MultisampleType::msaa_x32: return VK_SAMPLE_COUNT_32_BIT;
	case vtek::MultisampleType::msaa_x64: return VK_SAMPLE_COUNT_64_BIT;
	default:
		vtek_log_error("vtek::get_multisample_count(): Invalid enum value!");
		return VK_SAMPLE_COUNT_1_BIT;
	}
}