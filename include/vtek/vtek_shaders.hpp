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
#include "vtek_object_handles.hpp"


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

	// TODO: Comments may be deleted arbitrarily at will
	enum class ShaderStageRayTracing : uint32_t
	{
		// Entry-point from where rays are generated (may also generate in
		// fragment/compute shaders).
		raygen       = 0x01U,

		// Executed after an intersection shader is triggered. Its main use is
		// to determine whether the hit should be processed further or ignored.
		any_hit      = 0x02U,

		// Triggered the first time a ray hits a primitive
		closest_hit  = 0x04U,

		// Triggered if the ray doesn't hit any primitive
		miss         = 0x08U,

		// Allows the application to implement custom geometry primitives.
		// In Vulkan, we can only define triangles and axis-aligned bounding boxes.
		intersection = 0x10U,

		// Shaders that can be called from within an existing shader
		callable     = 0x20U
	};

	VkShaderStageFlagBits get_shader_stage(ShaderStage stage);
	VkShaderStageFlagBits get_shader_stage_graphics(ShaderStageGraphics stage);
	VkShaderStageFlagBits get_shader_stage_ray_tracing(ShaderStageRayTracing stage);

	VkShaderStageFlags get_shader_stage_flags(EnumBitmask<ShaderStage> mask);
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


	// ========================== //
	// === Raytracing shaders === //
	// ========================== //
	struct RaytracingShaderInfo
	{

	};

	RaytracingShader raytracing_shader_load_glsl(
		const RaytracingShaderInfo* info, const Directory* shaderdir, Device* device);
	RaytracingShader raytracing_shader_load_spirv(
		const RaytracingShaderInfo* info, const Directory* shaderdir, Device* device);

	void raytracing_shader_destroy(RaytracingShader* shader, Device* device);
}
