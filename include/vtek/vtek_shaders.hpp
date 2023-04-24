#pragma once

// TODO: Should this be distributed in several files instead? Suggestion:
// - vtek_compute_shader.h, vtek_graphics_shader.h, vtek_raytrace_shader.h,
// - vtek_transform_feedback_shader.h

// TODO: And then do the same with pipeline objects:
// - vtek_graphics_pipeline.h, vtek_compute_pipeline.h,
// - vtek_transform_feedback_pipeline.h, vtek_raytrace_pipeline.h

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

#include "vtek_device.hpp"

//
// NOTE: By Travis: Requirements for a shader system:
// - Loads from configuration
//     - In files or code
// - Is generic
//     - Little to nothing hard-coded
// - Is extensible
//     - Easy to modify, extend, and maintain.




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

	VkShaderStageFlagBits get_shader_stage(ShaderStage stage);
	VkShaderStageFlagBits get_shader_stage_graphics(ShaderStageGraphics stage);
	VkShaderStageFlagBits get_shader_stage_ray_tracing(ShaderStageRayTracing stage);


	struct GraphicsShader; // opaque handle

	GraphicsShader* graphics_shader_create(Device* device);
	void graphics_shader_destroy(GraphicsShader* shader, Device* device);


	struct GraphicsShaderModule
	{
		ShaderStageGraphics stage {ShaderStageGraphics::vertex};
		VkShaderModule module {VK_NULL_HANDLE};
	};

	const std::vector<GraphicsShaderModule>& graphics_shader_get_modules(GraphicsShader* shader);


	// TODO: ?
	VkDescriptorSetLayout graphics_shader_get_descriptor_layout(GraphicsShader* shader);
}
