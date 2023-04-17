#include "vtek_shaders.h"
#include "vtek_logging.h"

/* struct implementation */
struct vtek::GraphicsShader
{
	uint32_t numShaders {0};

	// For each shader we probably would like some info (maybe?):
	// - shader type
	// - VkShaderModule
	std::vector<VkShaderModule> m; // TODO: Or something more?
};

/* helper functions */



/* interface */
VkShaderStageFlags get_shader_stage(vtek::ShaderStage stage)
{
	switch (stage)
	{
	case vtek::ShaderStage::vertex:
		return VK_SHADER_STAGE_VERTEX_BIT;
	case vtek::ShaderStage::tesselation_control:
		return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case vtek::ShaderStage::tesselation_eval:
		return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	case vtek::ShaderStage::geometry:
		return VK_SHADER_STAGE_GEOMETRY_BIT;
	case vtek::ShaderStage::fragment:
		return VK_SHADER_STAGE_FRAGMENT_BIT;
	case vtek::ShaderStage::compute:
		return VK_SHADER_STAGE_COMPUTE_BIT;

	// NOTE: Making sure code compiles on platforms without these extensions.
	// Provided by VK_KHR_ray_tracing_pipeline
#if device(VK_VERSION_1_1)
#if VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME
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
#endif

	// Provided by VK_EXT_mesh_shader
#if device(VK_VERSION_1_1)
#if VK_EXT_MESH_SHADER_EXTENSION_NAME
	case vtek::ShaderStage::task:
		return VK_SHADER_STAGE_TASK_BIT_EXT;
	case vtek::ShaderStage::mesh:
		return VK_SHADER_STAGE_MESH_BIT_EXT;

	case vtek::ShaderStage::all_graphics:
		return VK_SHADER_STAGE_ALL_GRAPHICS;
	case vtek::ShaderStage::all:
		return VK_SHADER_STAGE_ALL;
#endif
#endif

	default:
	}
}

VkShaderStageFlags vtek::get_shader_stage_graphics(vtek::ShaderStageGraphics stage)
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
	}
}

VkShaderStageFlags vtek::get_shader_stage_ray_tracing(vtek::ShaderStageRayTracing stage)
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
	}
}
