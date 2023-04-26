#include "vtek_shaders.hpp"

#include "vtek_device.hpp"
#include "vtek_logging.hpp"

// External dependency: Spirv-reflect, to extract descriptor bindings from SPIR-V bytecode.
#include <spirv_reflect.h>

// TODO: Also glslang?

/* struct implementation */
struct vtek::GraphicsShader
{
	// TODO: Could have a bit flag telling which shader stages are used, like:
	// VkShaderStageFlags flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	// REVIEW: If such a thing is useful.

	std::vector<vtek::GraphicsShaderModule> modules;
};

/* helper functions */
static bool is_file_source_spirv()
{
	return false;
}
static bool is_file_source_glsl()
{
	return false;
}



/* interface */
VkShaderStageFlagBits vtek::get_shader_stage(vtek::ShaderStage stage)
{
	switch (stage)
	{
	case vtek::ShaderStage::vertex:
		return VK_SHADER_STAGE_VERTEX_BIT;
	case vtek::ShaderStage::tessellation_control:
		return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case vtek::ShaderStage::tessellation_eval:
		return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	case vtek::ShaderStage::geometry:
		return VK_SHADER_STAGE_GEOMETRY_BIT;
	case vtek::ShaderStage::fragment:
		return VK_SHADER_STAGE_FRAGMENT_BIT;
	case vtek::ShaderStage::compute:
		return VK_SHADER_STAGE_COMPUTE_BIT;

	// NOTE: Making sure code compiles on platforms without these extensions.
	// TODO: Check that these are accessible on platforms where extensions ARE present!
	// Provided by VK_KHR_ray_tracing_pipeline
#if defined(VK_VERSION_1_1) && defined(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)
	case vtek::ShaderStage::raygen:
		return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	case vtek::ShaderStage::any_hit:
		return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
	case vtek::ShaderStage::closest_hit:
		return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	case vtek::ShaderStage::miss:
		return VK_SHADER_STAGE_MISS_BIT_KHR;
	case vtek::ShaderStage::intersection:
		return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
	case vtek::ShaderStage::callable:
		return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
#endif

	// Provided by VK_EXT_mesh_shader
#if defined(VK_VERSION_1_1) && defined(VK_EXT_MESH_SHADER_EXTENSION_NAME)
	case vtek::ShaderStage::task:
		return VK_SHADER_STAGE_TASK_BIT_EXT;
	case vtek::ShaderStage::mesh:
		return VK_SHADER_STAGE_MESH_BIT_EXT;

	case vtek::ShaderStage::all_graphics:
		return VK_SHADER_STAGE_ALL_GRAPHICS;
	case vtek::ShaderStage::all:
		return VK_SHADER_STAGE_ALL;
#endif

	default:
		vtek_log_error("vtek::get_shader_stage: Invalid stage!");
		return static_cast<VkShaderStageFlagBits>(0);
	}
}

VkShaderStageFlagBits vtek::get_shader_stage_graphics(vtek::ShaderStageGraphics stage)
{
	switch (stage)
	{
	case vtek::ShaderStageGraphics::vertex:
		return VK_SHADER_STAGE_VERTEX_BIT;
	case vtek::ShaderStageGraphics::tessellation_control:
		return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case vtek::ShaderStageGraphics::tessellation_eval:
		return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	case vtek::ShaderStageGraphics::geometry:
		return VK_SHADER_STAGE_GEOMETRY_BIT;
	case vtek::ShaderStageGraphics::fragment:
		return VK_SHADER_STAGE_FRAGMENT_BIT;

	default:
		vtek_log_error("vtek::get_shader_stage_graphics: Invalid stage!");
		return static_cast<VkShaderStageFlagBits>(0);
	}
}

VkShaderStageFlagBits vtek::get_shader_stage_ray_tracing(vtek::ShaderStageRayTracing stage)
{
	switch (stage)
	{
	case vtek::ShaderStageRayTracing::raygen:
		return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	case vtek::ShaderStageRayTracing::any_hit:
		return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
	case vtek::ShaderStageRayTracing::closest_hit:
		return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	case vtek::ShaderStageRayTracing::miss:
		return VK_SHADER_STAGE_MISS_BIT_KHR;
	case vtek::ShaderStageRayTracing::intersection:
		return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
	case vtek::ShaderStageRayTracing::callable:
		return VK_SHADER_STAGE_CALLABLE_BIT_KHR;

	default:
		vtek_log_error("vtek::get_shader_stage_ray_tracing: Invalid stage!");
		return static_cast<VkShaderStageFlagBits>(0);
	}
}



static bool check_shader_files_exist(
	const vtek::GraphicsShaderInfo* info, vtek::Directory* shaderdir)
{
	bool exist = true;

	if (info->vertex && (!vtek::file_exists(shaderdir, "vertex.spv")))
	{
		vtek_log_error(
			"Failed to find vertex shader file \"{}{}vertex.spv\".",
			vtek::directory_get_name(shaderdir), vtek::get_path_separator());
		exist = false;
	}
	if (info->tess_control && (!vtek::file_exists(shaderdir, "tess_control.spv")))
	{
		vtek_log_error(
			"Failed to find vertex shader file \"{}{}tess_control.spv\".",
			vtek::directory_get_name(shaderdir), vtek::get_path_separator());
		exist = false;
	}
	if (info->tess_eval && (!vtek::file_exists(shaderdir, "tess_eval.spv")))
	{
		vtek_log_error(
			"Failed to find vertex shader file \"{}{}tess_eval.spv\".",
			vtek::directory_get_name(shaderdir), vtek::get_path_separator());
		exist = false;
	}
	if (info->geometry && (!vtek::file_exists(shaderdir, "geometry.spv")))
	{
		vtek_log_error(
			"Failed to find vertex shader file \"{}{}geometry.spv\".",
			vtek::directory_get_name(shaderdir), vtek::get_path_separator());
		exist = false;
	}
	if (info->fragment && (!vtek::file_exists(shaderdir, "fragment.spv")))
	{
		vtek_log_error(
			"Failed to find vertex shader file \"{}{}fragment.spv\".",
			vtek::directory_get_name(shaderdir), vtek::get_path_separator());
		exist = false;
	}

	return exist;
}

static void load_vertex_shader_spirv(vtek::Directory* shaderdir)
{
	auto flags = vtek::FileModeFlag::read | vtek::FileModeFlag::write;
	vtek::File* file = vtek::file_open(shaderdir, "vertex.spv", flags);
	if (file == nullptr)
	{
		vtek_log_error("Failed to open vertex shader file!");
	}
}

vtek::GraphicsShader* vtek::graphics_shader_load_spirv(
	const vtek::GraphicsShaderInfo* info,
	vtek::Directory* shaderdir, vtek::Device* device)
{
	bool exist = check_shader_files_exist(info, shaderdir);
	if (!exist)
	{
		vtek_log_error("--> cannot create graphics shader!");
		return nullptr;
	}

	if (info->vertex)
	{
		// auto vert = ...
		load_vertex_shader_spirv(shaderdir);
	}

	return nullptr;
}









vtek::GraphicsShader* vtek::graphics_shader_create(vtek::Device* device)
{

}

void vtek::graphics_shader_destroy(vtek::GraphicsShader* shader, vtek::Device* device)
{
	if (shader == nullptr) { return; }

	VkDevice dev = vtek::device_get_handle(device);

	for (auto module : shader->modules)
	{
		vkDestroyShaderModule(dev, module.module, nullptr);
	}
}

const std::vector<vtek::GraphicsShaderModule>& vtek::graphics_shader_get_modules(
	vtek::GraphicsShader* shader)
{
	return shader->modules;
}
