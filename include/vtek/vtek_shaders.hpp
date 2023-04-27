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

#include "vtek_fileio.hpp"
#include "vtek_types.hpp"


namespace vtek
{
	//========================= //
	// === Shader utilities === //
	//========================= //

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


	// TODO: Wew
	struct GraphicsShaderInfo
	{
		bool vertex {false};
		bool tess_control {false};
		bool tess_eval {false};
		bool geometry {false};
		bool fragment {false};

		// NOTE: Can optionally run SPIRV-Validator
		// REVIEW: The validator tool is incomplete. But would be cool to have:
		// https://github.com/KhronosGroup/SPIRV-Tools/tree/main
		//bool runValidation {false};
	};

	GraphicsShader* graphics_shader_load_glsl(
		const GraphicsShaderInfo* info, Directory* shaderdir, Device* device);
	GraphicsShader* graphics_shader_load_spirv(
		const GraphicsShaderInfo* info, Directory* shaderdir, Device* device);

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
