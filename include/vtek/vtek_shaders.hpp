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
#include "vtek_vulkan_handles.hpp"


namespace vtek
{
	//========================= //
	// === Shader utilities === //
	//========================= //
	enum class ShaderStage : uint32_t
	{
		vertex               = 0x0001U,
		tessellation_control = 0x0002U,
		tessellation_eval    = 0x0004U,
		geometry             = 0x0008U,
		fragment             = 0x0010U,
		compute              = 0x0020U,
		raygen               = 0x0040U,
		any_hit              = 0x0080U,
		closest_hit          = 0x0100U,
		miss                 = 0x0200U,
		intersection         = 0x0400U,
		callable             = 0x0800U,
		task                 = 0x1000U,
		mesh                 = 0x2000U,
		all_graphics         = 0x4000U,
		all                  = 0x8000U
	};

	enum class ShaderStageGraphics : uint32_t
	{
		vertex               = 0x01U,
		tessellation_control = 0x02U,
		tessellation_eval    = 0x04U,
		geometry             = 0x08U,
		fragment             = 0x10U
	};

	enum class ShaderStageRayTracing : uint32_t
	{
		raygen       = 0x01U,
		any_hit      = 0x02U,
		closest_hit  = 0x04U,
		miss         = 0x08U,
		intersection = 0x10U,
		callable     = 0x20U
	};

	VkShaderStageFlagBits get_shader_stage(ShaderStage stage);
	VkShaderStageFlagBits get_shader_stage_graphics(ShaderStageGraphics stage);
	VkShaderStageFlagBits get_shader_stage_ray_tracing(ShaderStageRayTracing stage);

	VkShaderStageFlags get_shader_stage_flags_graphics(
		EnumBitmask<ShaderStageGraphics> mask);


	// ======================== //
	// === Graphics shaders === //
	// ======================== //
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
