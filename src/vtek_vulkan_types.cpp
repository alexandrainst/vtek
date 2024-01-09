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

vtek::MultisampleType vtek::get_multisample_enum(VkSampleCountFlagBits count)
{
	switch (count)
	{
	case VK_SAMPLE_COUNT_1_BIT:  return vtek::MultisampleType::none;
	case VK_SAMPLE_COUNT_2_BIT:  return vtek::MultisampleType::msaa_x2;
	case VK_SAMPLE_COUNT_4_BIT:  return vtek::MultisampleType::msaa_x4;
	case VK_SAMPLE_COUNT_8_BIT:  return vtek::MultisampleType::msaa_x8;
	case VK_SAMPLE_COUNT_16_BIT: return vtek::MultisampleType::msaa_x16;
	case VK_SAMPLE_COUNT_32_BIT: return vtek::MultisampleType::msaa_x32;
	case VK_SAMPLE_COUNT_64_BIT: return vtek::MultisampleType::msaa_x64;
	default:
		vtek_log_error("vtek::get_multisample_enum(): Invalid count value!");
		return vtek::MultisampleType::none;
	}
}

VkCullModeFlags vtek::get_cull_mode(vtek::CullMode mode)
{
	switch (mode)
	{
	case vtek::CullMode::none:           return VK_CULL_MODE_NONE;
	case vtek::CullMode::front:          return VK_CULL_MODE_FRONT_BIT;
	case vtek::CullMode::back:           return VK_CULL_MODE_BACK_BIT;
	case vtek::CullMode::front_and_back: return VK_CULL_MODE_FRONT_AND_BACK;
	default:
		vtek_log_error("vtek::get_cull_mode(): Invalid cull mode!");
		return VK_CULL_MODE_NONE;
	}
}

VkPipelineStageFlags vtek::get_pipeline_stage(vtek::PipelineStage stage)
{
	switch (stage)
	{
	case vtek::PipelineStage::top_of_pipe:
		return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	case vtek::PipelineStage::draw_indirect:
		return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	case vtek::PipelineStage::vertex_input:
		return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
	case vtek::PipelineStage::vertex_shader:
		return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
	case vtek::PipelineStage::tess_control_shader:
		return VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
	case vtek::PipelineStage::tess_eval_shader:
		return VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
	case vtek::PipelineStage::geometry_shader:
		return VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
	case vtek::PipelineStage::fragment_shader:
		return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	case vtek::PipelineStage::early_fragment_tests:
		return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	case vtek::PipelineStage::late_fragment_tests:
		return VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	case vtek::PipelineStage::color_attachment_output:
		return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	case vtek::PipelineStage::compute_shader:
		return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	case vtek::PipelineStage::transfer:
		return VK_PIPELINE_STAGE_TRANSFER_BIT;
	case vtek::PipelineStage::bottom_of_pipe:
		return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	case vtek::PipelineStage::host:
		return VK_PIPELINE_STAGE_HOST_BIT;
	case vtek::PipelineStage::all_graphics:
		return VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
	case vtek::PipelineStage::all_commands:
		return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

	case vtek::PipelineStage::none:
#if defined (VK_API_VERSION_1_3)
		return VK_PIPELINE_STAGE_NONE;
#else
		return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
#endif
	default:
		vtek_log_error("vtek::get_pipeline_stage(): Invalid stage!");
		return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	}
}

VkAccessFlags vtek::get_access_mask(
	vtek::EnumBitmask<vtek::AccessMask> accessMask)
{
	VkAccessFlags flags = 0;

	if (accessMask.has_flag(vtek::AccessMask::indirect_command_read)) {
		flags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
	}
	if (accessMask.has_flag(vtek::AccessMask::index_read)) {
		flags |= VK_ACCESS_INDEX_READ_BIT;
	}
	if (accessMask.has_flag(vtek::AccessMask::vertex_attribute_read)) {
		flags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
	}
	if (accessMask.has_flag(vtek::AccessMask::uniform_read)) {
		flags |= VK_ACCESS_UNIFORM_READ_BIT;
	}
	if (accessMask.has_flag(vtek::AccessMask::input_attachment_read)) {
		flags |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	}
	if (accessMask.has_flag(vtek::AccessMask::shader_read)) {
		flags |= VK_ACCESS_SHADER_READ_BIT;
	}
	if (accessMask.has_flag(vtek::AccessMask::shader_write)) {
		flags |= VK_ACCESS_SHADER_WRITE_BIT;
	}
	if (accessMask.has_flag(vtek::AccessMask::color_attachment_read)) {
		flags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
	}
	if (accessMask.has_flag(vtek::AccessMask::color_attachment_write)) {
		flags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}
	if (accessMask.has_flag(vtek::AccessMask::depth_stencil_attachment_read)) {
		flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	}
	if (accessMask.has_flag(vtek::AccessMask::depth_stencil_attachment_write)) {
		flags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}
	if (accessMask.has_flag(vtek::AccessMask::transfer_read)) {
		flags |= VK_ACCESS_TRANSFER_READ_BIT;
	}
	if (accessMask.has_flag(vtek::AccessMask::transfer_write)) {
		flags |= VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	if (accessMask.has_flag(vtek::AccessMask::host_read)) {
		flags |= VK_ACCESS_HOST_READ_BIT;
	}
	if (accessMask.has_flag(vtek::AccessMask::host_write)) {
		flags |= VK_ACCESS_HOST_WRITE_BIT;
	}
	if (accessMask.has_flag(vtek::AccessMask::memory_read)) {
		flags |= VK_ACCESS_MEMORY_READ_BIT;
	}
	if (accessMask.has_flag(vtek::AccessMask::memory_write)) {
		flags |= VK_ACCESS_MEMORY_WRITE_BIT;
	}
	if (accessMask.has_flag(vtek::AccessMask::none)) {
#if defined (VK_API_VERSION_1_3)
		flags |= VK_ACCESS_NONE;
#else
		vtek_log_error(
			"AccessMask::none requires Vulkan >= 1.3 when compiling!");
		vtek_log_warn("This access flag will be ignored.");
#endif
	}

	return flags;
}
