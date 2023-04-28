#include "vtek_shaders.hpp"

#include "vtek_device.hpp"
#include "vtek_logging.hpp"

// External dependency: Spirv-reflect, to extract descriptor bindings from SPIR-V bytecode.
#include <spirv_reflect.h>

// External dependency: glslang for generating SPIR-V bytecode from GLSL.
//#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/ShaderLang.h>


/* struct implementation */
struct vtek::GraphicsShader
{
	// TODO: Could have a bit flag telling which shader stages are used, like:
	// VkShaderStageFlags flags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	// REVIEW: If such a thing is useful.

	std::vector<vtek::GraphicsShaderModule> modules;

	VkDescriptorSetLayout descriptorLayout {VK_NULL_HANDLE};
};

// TODO: Create an allocator for shader objects?



/* GLSL shader loading */
#include "impl/vtek_init.hpp"

bool vtek::initialize_glsl_shader_loading()
{
	vtek_log_trace("initialize_glsl_shader_loading()");
	glslang::InitializeProcess();

	// TODO: Default resource limits ?? Previously done as:
	// CMakeLists.txt: target_link_libraries(vtek glslang::glslang-default-resource-limits)
	// Code:
	// namespace glslang {
	//     // Implemented in libglslang-default-resource-limits
	//     extern const TBuiltInResource DefaultTBuiltInResource;
	// }
	return true;
}

void vtek::terminate_glsl_shader_loading()
{
	vtek_log_trace("terminate_glsl_shader_loading()");
	glslang::FinalizeProcess();
}



/* helper functions */
static const char* sFilenamesGLSL[5] =
{
	"vertex.glsl", "tess_control.glsl", "tess_eval.glsl",
	"geometry.glsl", "fragment.glsl"
};
static const char* sFilenamesSPIRV[5] = {
	"vertex.spv", "tess_control.spv", "tess_eval.spv",
	"geometry.spv", "fragment.spv"
};

enum class ShaderFileFormat
{
	glsl, spirv
};

static bool check_graphics_shader_files_exist(
	const vtek::GraphicsShaderInfo* info, vtek::Directory* shaderdir,
	ShaderFileFormat format)
{
	bool exist = true;
	const char** filenames =
		(format == ShaderFileFormat::glsl) ? sFilenamesGLSL : sFilenamesSPIRV;

	if (info->vertex && (!vtek::file_exists(shaderdir, filenames[0])))
	{
		vtek_log_error(
			"Failed to find vertex shader file \"{}\".",
			vtek::directory_get_path(shaderdir, filenames[0]));
		exist = false;
	}
	if (info->tess_control && (!vtek::file_exists(shaderdir, filenames[1])))
	{
		vtek_log_error(
			"Failed to find tessellation control shader file \"{}\".",
			vtek::directory_get_path(shaderdir, filenames[1]));
		exist = false;
	}
	if (info->tess_eval && (!vtek::file_exists(shaderdir, filenames[2])))
	{
		vtek_log_error(
			"Failed to find tessellation evaluation shader file \"{}\".",
			vtek::directory_get_path(shaderdir, filenames[2]));
		exist = false;
	}
	if (info->geometry && (!vtek::file_exists(shaderdir, filenames[3])))
	{
		vtek_log_error(
			"Failed to find geometry shader file \"{}\".",
			vtek::directory_get_path(shaderdir, filenames[3]));
		exist = false;
	}
	if (info->fragment && (!vtek::file_exists(shaderdir, filenames[4])))
	{
		vtek_log_error(
			"Failed to find fragment shader file \"{}\".",
			vtek::directory_get_path(shaderdir, filenames[4]));
		exist = false;
	}

	return exist;
}

static VkShaderModule load_spirv_shader(
	vtek::Directory* shaderdir, const char* filename, const char* type,
	VkDevice dev)
{
	// Open file
	auto flags = vtek::FileModeFlag::read | vtek::FileModeFlag::binary;
	vtek::File* file = vtek::file_open(shaderdir, filename, flags);
	if (file == nullptr)
	{
		vtek_log_error("Failed to open {} shader file!", type);
		return VK_NULL_HANDLE;
	}

	// Read file into buffer
	std::vector<char> buffer;
	bool read = vtek::file_read_into_buffer(file, buffer);
	vtek::file_close(file);
	if (!read)
	{
		vtek_log_error("Failed to read {} shader file!", type);
		return VK_NULL_HANDLE;
	}

	// Create shader module
	VkShaderModule module;
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = buffer.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());
	VkResult result = vkCreateShaderModule(dev, &createInfo, nullptr, &module);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to create {} shader module!", type);
		return VK_NULL_HANDLE;
	}

	return module;
}

EShLanguage get_glslang_shader_stage(vtek::ShaderStageGraphics stage)
{
	switch (stage)
	{
	case vtek::ShaderStageGraphics::vertex:               return EShLangVertex;
	case vtek::ShaderStageGraphics::tessellation_control: return EShLangTessControl;
	case vtek::ShaderStageGraphics::tessellation_eval:    return EShLangTessEvaluation;
	case vtek::ShaderStageGraphics::geometry:             return EShLangGeometry;
	case vtek::ShaderStageGraphics::fragment:             return EShLangFragment;

	default:
		vtek_log_error("vtek_shaders.cpp -> get_glslang_shader_stage: Invalid stage!");
		return static_cast<EShLanguage>(-1);
	}
}

static VkShaderModule load_glsl_shader(
	vtek::Directory* shaderdir, const char* filename, const char* type,
	vtek::ShaderStageGraphics stage, vtek::Device* device)
{
	// Open file
	auto flags = vtek::FileModeFlag::read | vtek::FileModeFlag::binary;
	vtek::File* file = vtek::file_open(shaderdir, filename, flags);
	if (file == nullptr)
	{
		vtek_log_error("Failed to open {} shader file!", type);
		return VK_NULL_HANDLE;
	}

	// Read file line by line, necessary to check for shader inclusions
	std::vector<char&> accumBuffer;
	accumBuffer.resize()

	// Read file into buffer
	std::vector<char> buffer;
	bool read = vtek::file_read_into_buffer(file, accumBuffer);
	vtek::file_close(file);
	if (!read)
	{
		vtek_log_error("Failed to read {} shader file!", type);
		return VK_NULL_HANDLE;
	}

	// Search for statements that include other shader files, ie `#include <file>`.
	// TODO: This could be optimized to run _while_ reading the file into buffer.









	// TODO: I might just concur and use the C API, as it seems much more simple:
	// https://github.com/KhronosGroup/glslang/tree/main#c-functional-interface-new










	// NEXT: New, improved usage glslang and proper (recursive) header inclusion!
	glslang::TShader shader;

	EShLanguage lang = get_glslang_shader_stage(stage);
	if (lang < 0)
	{
		vtek_log_error("--> cannot load GLSL {} shader!", type);
		return VK_NULL_HANDLE;
	}
	// NOTE: Version 100 indicated current branch of GLSL->Vulkan extension:
	// https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_vulkan_glsl.txt
	const int version = 100;
	shader.setEnvInput(EShSourceGlsl, lang, EShClientVulkan, version);

	// Get the active Vulkan version of the client process.
	// NOTE: This is *probably* a good default.
	const VulkanVersion apiVersion = *(vtek::device_get_vulkan_version(device));
	EShTargetClientVersion targetApiVersion;
	switch (apiVersion.minor())
	{
	case 3:  targetApiVersion = EShTargetVulkan_1_3; break;
	case 2:  targetApiVersion = EShTargetVulkan_1_2; break;
	case 1:  targetApiVersion = EShTargetVulkan_1_1; break;
	default: targetApiVersion = EShTargetVulkan_1_0; break;
	}
	shader.setEnvClient(EShClientVulkan, targetApiVersion);

	// NOTE: Ray tracing shaders REQUIRED SPIR-V 1.4!
	EShTargetLanguageVersion spirvVersion = EShTargetSpv_1_3;
	shader.setEnvTarget(EshTargetSpv, spirvVersion);

	// TODO: We need an includer for recursive shader inclusion
	glslang::TShader::Includer includer;

	// TODO: Probably need a mapper from physical device limits to glslang resource limits!
	TBuiltInResource resources;
	const int defaultVersion = 0; // TODO: WTF int defaultVersion ?? !!!
	const bool forwardCompatible = false;
	EShMessages messageFlags = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
	if (!shader.parse(&resources, defaultVersion, forwardCompatible, messageFlags, includer))
	{
		vtek_log_error("Failed to parse shader....");
		// TODO: Absolutely MUST release resources acquired with glslang!
		return VK_NULL_HANDLE;
	}

	// TODO: Do we log on error?
	const char* shaderInfoLog = shader.getInfoLog();


	// NEXT: Now we probably link the shader to create a program

	glslang::TProgram program;
	program.addShader(&shader);
	if (!program.link())
	{
		vtek_log_error("Failed to link shader....");
		// TODO: Absolutely MUST release resources acquired with glslang!
		return VK_NULL_HANDLE;
	}

	// TODO: Do we log on error?
	const char* programInfoLog = program.getInfoLog();

	// NEXT: Reflection queries...

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





vtek::GraphicsShader* vtek::graphics_shader_load_glsl(
	const vtek::GraphicsShaderInfo* info,
	vtek::Directory* shaderdir, vtek::Device* device)
{
	bool exist = check_graphics_shader_files_exist(
		info, shaderdir, ShaderFileFormat::glsl);
	if (!exist)
	{
		vtek_log_error("--> cannot create graphics shader!");
		return nullptr;
	}


}

vtek::GraphicsShader* vtek::graphics_shader_load_spirv(
	const vtek::GraphicsShaderInfo* info,
	vtek::Directory* shaderdir, vtek::Device* device)
{
	bool exist = check_graphics_shader_files_exist(
		info, shaderdir, ShaderFileFormat::spirv);
	if (!exist)
	{
		vtek_log_error("--> cannot create graphics shader!");
		return nullptr;
	}

	VkDevice dev = vtek::device_get_handle(device);
	std::vector<vtek::GraphicsShaderModule> modules;

	// TODO: Both geometry and tessellation shaders required physical device
	// features be enabled!
	const VkPhysicalDeviceFeatures* physDevFeatures =
		vtek::device_get_enabled_features(device);

	if (info->vertex)
	{
		VkShaderModule vertex = load_spirv_shader(
			shaderdir, sFilenamesSPIRV[0], "vertex", dev);
		if (vertex == VK_NULL_HANDLE)
		{
			vtek_log_error("--> cannot create graphics shader.");
			return nullptr;
		}
		modules.push_back({ vtek::ShaderStageGraphics::vertex, vertex });
	}
	if (info->tess_control)
	{
		if (physDevFeatures->tessellationShader == VK_FALSE)
		{
			vtek_log_error(
				"Tessellation shader features was not enabled during device creation!");
			vtek_log_error("--> cannot create graphics shader.");
			return nullptr;
		}

		VkShaderModule tess_control = load_spirv_shader(
			shaderdir, sFilenamesSPIRV[1], "tessellation control", dev);
		if (tess_control == VK_NULL_HANDLE)
		{
			vtek_log_error("--> cannot create graphics shader.");
			return nullptr;
		}
		modules.push_back({vtek::ShaderStageGraphics::tessellation_control, tess_control});
	}
	if (info->tess_eval)
	{
		if (physDevFeatures->tessellationShader == VK_FALSE)
		{
			vtek_log_error(
				"Tessellation shader features was not enabled during device creation!");
			vtek_log_error("--> cannot create graphics shader.");
			return nullptr;
		}

		VkShaderModule tess_eval = load_spirv_shader(
			shaderdir, sFilenamesSPIRV[2], "tessellation evaluation", dev);
		if (tess_eval == VK_NULL_HANDLE)
		{
			vtek_log_error("--> cannot create graphics shader.");
			return nullptr;
		}
		modules.push_back({vtek::ShaderStageGraphics::tessellation_eval, tess_eval});
	}
	if (info->geometry)
	{
		if (physDevFeatures->geometryShader == VK_FALSE)
		{
			vtek_log_error(
				"Geometry shader features was not enabled during device creation!");
			vtek_log_error("--> cannot create graphics shader.");
			return nullptr;
		}

		VkShaderModule geometry = load_spirv_shader(
			shaderdir, sFilenamesSPIRV[3], "geometry", dev);
		if (geometry == VK_NULL_HANDLE)
		{
			vtek_log_error("--> cannot create graphics shader.");
			return nullptr;
		}
		modules.push_back({ vtek::ShaderStageGraphics::geometry, geometry });
	}
	if (info->fragment)
	{
		VkShaderModule fragment = load_spirv_shader(
			shaderdir, sFilenamesSPIRV[4], "fragment", dev);
		if (fragment == VK_NULL_HANDLE)
		{
			vtek_log_error("--> cannot create graphics shader.");
			return nullptr;
		}
		modules.push_back({ vtek::ShaderStageGraphics::fragment, fragment });
	}

	// TODO: Do better through a centralized allocation mechanism
	auto shader = new vtek::GraphicsShader();
	shader->modules.swap(modules);

	// TODO: CreateDescriptorSetLayout
	// VkDescriptorSetAllocateInfo
	// TODO: Create descriptor pool!

	vtek_log_info("Loaded SPIR-V shader(s) from directory \"{}\".",
	              vtek::directory_get_path(shaderdir));

	return shader;
}

void vtek::graphics_shader_destroy(vtek::GraphicsShader* shader, vtek::Device* device)
{
	if (shader == nullptr) { return; }

	VkDevice dev = vtek::device_get_handle(device);

	for (auto& module : shader->modules)
	{
		vkDestroyShaderModule(dev, module.module, nullptr);
	}
	shader->modules.clear();
}

const std::vector<vtek::GraphicsShaderModule>& vtek::graphics_shader_get_modules(
	vtek::GraphicsShader* shader)
{
	return shader->modules;
}

VkDescriptorSetLayout vtek::graphics_shader_get_descriptor_layout(
	vtek::GraphicsShader* shader)
{
	return shader->descriptorLayout;
}
