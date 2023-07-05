#include "vtek_vulkan.pch"
#include "vtek_descriptor_type.hpp"

#include "vtek_device.hpp"
#include "vtek_logging.hpp"


using DType = vtek::DescriptorType;


std::optional<VkDescriptorType> vtek::get_descriptor_type(
	vtek::DescriptorType type, vtek::Device* device)
{
	switch (type)
	{
	case DType::sampler:
		return VK_DESCRIPTOR_TYPE_SAMPLER;
	case DType::combined_image_sampler:
		return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	case DType::sampled_image:
		return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	case DType::storage_image:
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	case DType::uniform_texel_buffer:
		return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
	case DType::storage_texel_buffer:
		return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
	case DType::uniform_buffer:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case DType::storage_buffer:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	case DType::uniform_buffer_dynamic:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	case DType::storage_buffer_dynamic:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
	case DType::input_attachment:
		return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;

	case DType::inline_uniform_block:
		// Provided by VK_VERSION_1_3
		{
#if defined(VK_VERSION_1_3)
			auto vv = vtek::device_get_vulkan_version(device);
			if (vv.major() > 1 || vv.minor() >= 3)
			{
				return VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
			}
			else
			{
				vtek_log_error(
					"The device must be created with at least Vulkan 1.3 {}",
					"to support descriptors of type INLINE_UNIFORM_BLOCK!");
				return std::nullopt;
			}
#else
			vtek_log_error(
				"At least Vulkan 1.3 must be installed on the system {}",
				"to support descriptors of type INLINE_UNIFORM_BLOCK!");
			return std::nullopt;
#endif
		}
		break; // control should not reach here!

	case DType::acceleration_structure:
		// Provided by VK_KHR_acceleration_structure
		// TODO: Check that this extension is available AND enabled!
		return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

		/*
		// Provided by VK_NV_ray_tracing
		VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV = 1000165000,
		// Provided by VK_QCOM_image_processing
		VK_DESCRIPTOR_TYPE_SAMPLE_WEIGHT_IMAGE_QCOM = 1000440000,
		// Provided by VK_QCOM_image_processing
		VK_DESCRIPTOR_TYPE_BLOCK_MATCH_IMAGE_QCOM = 1000440001,
		// Provided by VK_EXT_mutable_descriptor_type
		VK_DESCRIPTOR_TYPE_MUTABLE_EXT = 1000351000,
		// Provided by VK_EXT_inline_uniform_block
		VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK,
		// Provided by VK_VALVE_mutable_descriptor_type
		VK_DESCRIPTOR_TYPE_MUTABLE_VALVE = VK_DESCRIPTOR_TYPE_MUTABLE_EXT,
		*/

	default:
		vtek_log_error("vtek:descriptor_type.cpp: Invalid enum!");
		return std::nullopt;
	}
}
