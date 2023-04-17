#pragma once

// TODO: Should this be distributed in several files instead? Suggestion:
// - vtek_compute_shader.h, vtek_graphics_shader.h, vtek_raytrace_shader.h,
// - vtek_transform_feedback_shader.h

// TODO: And then do the same with pipeline objects:
// - vtek_graphics_pipeline.h, vtek_compute_pipeline.h,
// - vtek_transform_feedback_pipeline.h, vtek_raytrace_pipeline.h

#include <cstdint>
#include <vulkan/vulkan.h>


namespace vtek
{
	enum class ShaderStage
	{
		vertex, tessellation_control, tessellation_eval,
		geometry, fragment, compute,
		raygen, any_hit, closest_hit, miss, intersection, callable,
		task, mesh,
		all_graphics, all
	};

	enum class ShaderStageGraphics
	{
		vertex, tessellation_control, tessellation_eval, geometry, fragment
	};

	enum class ShaderStageRayTracing
	{
		raygen, any_hit, closest_hit, miss, intersection, callable
	};

	// TODO: Or return VkShaderStageFlagBits ?
	VkShaderStageFlags get_shader_stage(ShaderStage stage);
	VkShaderStageFlags get_shader_stage_graphics(ShaderStageGraphics stage);
	VkShaderStageFlags get_shader_stage_ray_tracing(ShaderStageRayTracing stage);

	// constexpr uint32_t kMaxShaderStages = 4; // TODO: Get rid of this!

	struct GraphicsShader; // opaque handle

	graphics_shader_get_pipelin


	// TODO: ?
	VkDescriptorSetLayout graphics_shader_get_descriptor_layout(GraphicsShader* shader);
}
