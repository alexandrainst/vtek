#include "vtek_shaders.hpp"

#include "vtek_device.hpp"
#include "vtek_logging.hpp"
#include "vtek_physical_device.hpp"

#include <forward_list>

// External dependency: Spirv-reflect, to extract descriptor bindings from SPIR-V bytecode.
#include <spirv_reflect.h>

// External dependency: glslang for generating SPIR-V bytecode from GLSL.
// NOTE: We may use the default resource limits, IFF we compile for OpenGL ES.
// --> in practice, this means never! Has two functions though:
// - GetResources() : TBuiltInResource*
// - GetDefaultResources() :TBuiltInResource*
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>


/* shorthand type aliases */
using SStage = vtek::ShaderStage;
using SSGraphics = vtek::ShaderStageGraphics;
using SSRayTrace = vtek::ShaderStageRayTracing;


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



void vtek::build_glslang_resource_limits(const vtek::PhysicalDevice* physicalDevice)
{
	vtek_log_debug("vtek::build_glslang_resource_limits");

	// This is read/write global memory inside the glslang library, which has
	// been loaded (hopefully) prior to calling this function. The parameters
	// are modified here once, and then reused for each subsequent call to
	// `graphics_shader_load_glsl`.
	TBuiltInResource* res = GetResources();

	// Before doing anything, we overwrite by the default values.
	*res = *GetDefaultResources();

	// Read values from `VkPhysicalDeviceLimits` obtained from the selected
	// physical device, and copy the relevant fields over. This will 'program'
	// glslang to compile GLSL shaders to SPIR-V correctly observing the limits
	// of the actual target GPU instead of the default limits which are very
	// conservative.
	auto properties = vtek::physical_device_get_properties(physicalDevice);
	const VkPhysicalDeviceLimits* limits = &properties->limits;

    // res->maxLights;
    // res->maxClipPlanes;
    // res->maxTextureUnits;
    // res->maxTextureCoords;
    // res->maxVertexAttribs;
    // res->maxVertexUniformComponents;
    // res->maxVaryingFloats;
    // res->maxVertexTextureImageUnits;
    // res->maxCombinedTextureImageUnits;
    // res->maxTextureImageUnits;
    // res->maxFragmentUniformComponents;
    // res->maxDrawBuffers;
    // res->maxVertexUniformVectors;
    // res->maxVaryingVectors;
    // res->maxFragmentUniformVectors;
    // res->maxVertexOutputVectors;
    // res->maxFragmentInputVectors;
    // res->minProgramTexelOffset;
    // res->maxProgramTexelOffset;
    // res->maxClipDistances;

    res->maxComputeWorkGroupCountX = limits->maxComputeWorkGroupCount[0];
    res->maxComputeWorkGroupCountY = limits->maxComputeWorkGroupCount[1];
    res->maxComputeWorkGroupCountZ = limits->maxComputeWorkGroupCount[2];
    res->maxComputeWorkGroupSizeX = limits->maxComputeWorkGroupSize[0];
    res->maxComputeWorkGroupSizeY = limits->maxComputeWorkGroupSize[1];
    res->maxComputeWorkGroupSizeZ = limits->maxComputeWorkGroupSize[2];

    // res->maxComputeUniformComponents;
    // res->maxComputeTextureImageUnits;
    // res->maxComputeImageUniforms;
    // res->maxComputeAtomicCounters;
    // res->maxComputeAtomicCounterBuffers;
    // res->maxVaryingComponents;
    // res->maxVertexOutputComponents;
    // res->maxGeometryInputComponents;
    // res->maxGeometryOutputComponents;
    // res->maxFragmentInputComponents;
    // res->maxImageUnits;
    // res->maxCombinedImageUnitsAndFragmentOutputs;
    // res->maxCombinedShaderOutputResources;
    // res->maxImageSamples;
    // res->maxVertexImageUniforms;
    // res->maxTessControlImageUniforms;
    // res->maxTessEvaluationImageUniforms;
    // res->maxGeometryImageUniforms;
    // res->maxFragmentImageUniforms;
    // res->maxCombinedImageUniforms;
    // res->maxGeometryTextureImageUnits;
    // res->maxGeometryOutputVertices;
    // res->maxGeometryTotalOutputComponents;
    // res->maxGeometryUniformComponents;
    // res->maxGeometryVaryingComponents;

    res->maxTessControlInputComponents =
	    limits->maxTessellationControlPerVertexInputComponents;
    res->maxTessControlOutputComponents =
	    limits->maxTessellationControlPerVertexOutputComponents;
    // res->maxTessControlTextureImageUnits;
    // res->maxTessControlUniformComponents;
    res->maxTessControlTotalOutputComponents =
	    limits->maxTessellationControlTotalOutputComponents;
    res->maxTessEvaluationInputComponents =
	    limits->maxTessellationEvaluationInputComponents;
    res->maxTessEvaluationOutputComponents =
	    limits->maxTessellationEvaluationOutputComponents;
    // res->maxTessEvaluationTextureImageUnits;
    // res->maxTessEvaluationUniformComponents;
    // res->maxTessPatchComponents;

    // res->maxPatchVertices;
    // res->maxTessGenLevel;
    // res->maxViewports = limits->maxViewports;
    // res->maxVertexAtomicCounters;
    // res->maxTessControlAtomicCounters;
    // res->maxTessEvaluationAtomicCounters;
    // res->maxGeometryAtomicCounters;
    // res->maxFragmentAtomicCounters;
    // res->maxCombinedAtomicCounters;
    // res->maxAtomicCounterBindings;
    // res->maxVertexAtomicCounterBuffers;
    // res->maxTessControlAtomicCounterBuffers;
    // res->maxTessEvaluationAtomicCounterBuffers;
    // res->maxGeometryAtomicCounterBuffers;
    // res->maxFragmentAtomicCounterBuffers;
    // res->maxCombinedAtomicCounterBuffers;
    // res->maxAtomicCounterBufferSize;
    // res->maxTransformFeedbackBuffers;
    // res->maxTransformFeedbackInterleavedComponents;
    // res->maxCullDistances;
    // res->maxCombinedClipAndCullDistances;
    // res->maxSamples;
    // res->maxMeshOutputVerticesNV;
    // res->maxMeshOutputPrimitivesNV;
    // res->maxMeshWorkGroupSizeX_NV;
    // res->maxMeshWorkGroupSizeY_NV;
    // res->maxMeshWorkGroupSizeZ_NV;
    // res->maxTaskWorkGroupSizeX_NV;
    // res->maxTaskWorkGroupSizeY_NV;
    // res->maxTaskWorkGroupSizeZ_NV;
    // res->maxMeshViewCountNV;
    // res->maxMeshOutputVerticesEXT;
    // res->maxMeshOutputPrimitivesEXT;
    // res->maxMeshWorkGroupSizeX_EXT;
    // res->maxMeshWorkGroupSizeY_EXT;
    // res->maxMeshWorkGroupSizeZ_EXT;
    // res->maxTaskWorkGroupSizeX_EXT;
    // res->maxTaskWorkGroupSizeY_EXT;
    // res->maxTaskWorkGroupSizeZ_EXT;
    // res->maxMeshViewCountEXT;
    // res->maxDualSourceDrawBuffersEXT;
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

static bool spirv_reflect_test(const void* spirv_code, size_t spirv_nbytes)
{
	vtek_log_trace("spirv_reflect_test()");

	// Generate reflection data for a shader
	SpvReflectShaderModule module;
	SpvReflectResult result = spvReflectCreateShaderModule(spirv_nbytes, spirv_code, &module);
	if (result != SPV_REFLECT_RESULT_SUCCESS)
	{
		vtek_log_error("Failed to create SPIRV-Reflect shader module!");
		return false;
	}
	//assert(result == SPV_REFLECT_RESULT_SUCCESS);

	// Enumerate and extract shader's input variables
	uint32_t var_count = 0;
	result = spvReflectEnumerateInputVariables(&module, &var_count, NULL);
	if (result != SPV_REFLECT_RESULT_SUCCESS)
	{
		vtek_log_error("Failed to enumerate shader input variables!");
		spvReflectDestroyShaderModule(&module);
		return false;
	}
	//assert(result == SPV_REFLECT_RESULT_SUCCESS);

	SpvReflectInterfaceVariable** input_vars =
		(SpvReflectInterfaceVariable**)malloc(var_count * sizeof(SpvReflectInterfaceVariable*));
	result = spvReflectEnumerateInputVariables(&module, &var_count, input_vars);
	if (result != SPV_REFLECT_RESULT_SUCCESS)
	{
		vtek_log_error("Failed to enumerate shader input variables!");
		spvReflectDestroyShaderModule(&module);
		return false;
	}
	//assert(result == SPV_REFLECT_RESULT_SUCCESS);

	// Output variables, descriptor bindings, descriptor sets, and push constants
	// can be enumerated and extracted using a similar mechanism.

	// Destroy the reflection data when no longer required.
	spvReflectDestroyShaderModule(&module);

	return true;
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

	// NEXT: Reflection queries...
	if (!spirv_reflect_test(buffer.data(), buffer.size()))
	{
		vtek_log_error(
			"Failed SPIR-V reflection query -- cannot create graphics shader!");
		vkDestroyShaderModule(dev, module, nullptr);
		return VK_NULL_HANDLE;
	}

	return module;
}

static EShLanguage get_glslang_shader_stage(SSGraphics stage)
{
	switch (stage)
	{
	case SSGraphics::vertex:               return EShLangVertex;
	case SSGraphics::tessellation_control: return EShLangTessControl;
	case SSGraphics::tessellation_eval:    return EShLangTessEvaluation;
	case SSGraphics::geometry:             return EShLangGeometry;
	case SSGraphics::fragment:             return EShLangFragment;

	default:
		vtek_log_error("vtek_shaders.cpp -> get_glslang_shader_stage: Invalid stage!");
		return static_cast<EShLanguage>(-1);
	}
}
#include <iostream>
static VkShaderModule load_glsl_shader(
	vtek::Directory* shaderdir, const char* filename, const char* type,
	SSGraphics stage, vtek::Device* device)
{
	VkDevice dev = vtek::device_get_handle(device);

	// Open file
	auto flags = vtek::FileModeFlag::read;
	vtek::File* file = vtek::file_open(shaderdir, filename, flags);
	if (file == nullptr)
	{
		vtek_log_error("Failed to open {} shader file!", type);
		return VK_NULL_HANDLE;
	}

	// TODO: Read file line by line, necessary to check for shader inclusions
	// std::vector<char> accumBuffer;
	// accumBuffer.resize();
	// std::vector<char> buffer;
	// bool read = vtek::file_read_into_buffer(file, accumBuffer);
	// vtek::file_close(file);
	// if (!read)
	// {
	// 	vtek_log_error("Failed to read {} shader file!", type);
	// 	return VK_NULL_HANDLE;
	// }

	// Read file into buffer
	std::vector<char> buffer;
	bool read = vtek::file_read_into_buffer(file, buffer);
	vtek::file_close(file);
	if (!read)
	{
		vtek_log_error("Failed to read {} shader file!", type);
		return VK_NULL_HANDLE;
	}
	buffer.push_back('\0');
	// std::cout << buffer.data() << '\n';

	// Search for statements that include other shader files, ie `#include <file>`.
	// TODO: This could be optimized to run _while_ reading the file into buffer.


	// TODO: I might just concur and use the C API, as it seems much more simple:
	// https://github.com/KhronosGroup/glslang/tree/main#c-functional-interface-new


	// NEXT: New, improved usage glslang and proper (recursive) header inclusion!
	EShLanguage lang = get_glslang_shader_stage(stage);
	if (lang < 0)
	{
		vtek_log_error("--> cannot load GLSL {} shader!", type);
		return VK_NULL_HANDLE;
	}
	glslang::TShader shader(lang);

	// TODO: We need an includer for recursive shader inclusion
	glslang::TShader::ForbidIncluder includer;

	// TODO: Adding sources to the shader
	std::forward_list<const char*> shaderStringsList;
	shaderStringsList.push_front(buffer.data());

	std::vector<const char*> shaderStrings;
	for (const char* str : shaderStringsList)
	{
		shaderStrings.push_back(str);
	}
	shader.setStrings(shaderStrings.data(), shaderStrings.size());



	// NOTE: Version 100 indicated current branch of GLSL->Vulkan extension:
	// https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_vulkan_glsl.txt
	const int version = 100;
	shader.setEnvInput(glslang::EShSourceGlsl, lang, glslang::EShClientVulkan, version);

	// Get the active Vulkan version of the client process.
	// NOTE: This is *probably* a good default.
	const vtek::VulkanVersion apiVersion = *(vtek::device_get_vulkan_version(device));
	glslang::EShTargetClientVersion targetApiVersion;
	switch (apiVersion.minor())
	{
	case 3:  targetApiVersion = glslang::EShTargetVulkan_1_3; break;
	case 2:  targetApiVersion = glslang::EShTargetVulkan_1_2; break;
	case 1:  targetApiVersion = glslang::EShTargetVulkan_1_1; break;
	default: targetApiVersion = glslang::EShTargetVulkan_1_0; break;
	}
	shader.setEnvClient(glslang::EShClientVulkan, targetApiVersion);

	// NOTE: Ray tracing shaders REQUIRED SPIR-V 1.4!
	// TODO: If ray tracing extension is enabled, require SPIR-V 1.4!
	glslang::EShTargetLanguageVersion spirvVersion = glslang::EShTargetSpv_1_3;
	shader.setEnvTarget(glslang::EshTargetSpv, spirvVersion);

	shader.setEntryPoint("main");

	// TODO: Probably need a mapper from physical device limits to glslang resource limits!
	TBuiltInResource* resources = GetResources();
	// int defaultVersion = 110, // use 100 for ES environment, overridden by #version in shader
	const int defaultVersion = 450;
	const bool forwardCompatible = false;
	const EShMessages messageFlags = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
	if (!shader.parse(resources, defaultVersion, forwardCompatible, messageFlags, includer))
	{
		vtek_log_error("Failed to parse shader: {}", shader.getInfoLog());
		// TODO: Absolutely MUST release resources acquired with glslang!
		return VK_NULL_HANDLE;
	}


	// NEXT: Now we probably link the shader to create a program

	// TODO: Do we really need to create program and link?? We are doing 1 shader at a time!
	glslang::TProgram program;
	program.addShader(&shader);
	if (!program.link(messageFlags))
	{
		vtek_log_error("Failed to link shader: {}", program.getInfoLog());
		// TODO: Absolutely MUST release resources acquired with glslang!
		return VK_NULL_HANDLE;
	}

	// Convert the intermediate generated by glslang to Spir-V
	glslang::TIntermediate& intermediateRef = *(program.getIntermediate(lang));
	std::vector<uint32_t> spirv;
	glslang::SpvOptions options{};
	options.validate = true;
	// TODO: We can also provide a logger to glslang! NOICE:
	// glslang::spv::SpvBuildLogger logger; ---> then:
	// glslang::GlslangToSpv(intermediateRef, spirv, &logger, &options);
	glslang::GlslangToSpv(intermediateRef, spirv, &options);

	// Create shader module
	VkShaderModule module;
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = spirv.size();
	createInfo.pCode = spirv.data();
	VkResult result = vkCreateShaderModule(dev, &createInfo, nullptr, &module);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to create {} shader module!", type);
		return VK_NULL_HANDLE;
	}

	return module;

	// NEXT: Reflection queries...
}




/* interface */
VkShaderStageFlagBits vtek::get_shader_stage(SStage stage)
{
	switch (stage)
	{
	case SStage::vertex:               return VK_SHADER_STAGE_VERTEX_BIT;
	case SStage::tessellation_control: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case SStage::tessellation_eval:    return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	case SStage::geometry:             return VK_SHADER_STAGE_GEOMETRY_BIT;
	case SStage::fragment:             return VK_SHADER_STAGE_FRAGMENT_BIT;
	case SStage::compute:              return VK_SHADER_STAGE_COMPUTE_BIT;

	// NOTE: Making sure code compiles on platforms without these extensions.
	// TODO: Check that these are accessible on platforms where extensions ARE present!
	// Provided by VK_KHR_ray_tracing_pipeline
#if defined(VK_VERSION_1_1) && defined(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)
	case SStage::raygen:       return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	case SStage::any_hit:      return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
	case SStage::closest_hit:  return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	case SStage::miss:         return VK_SHADER_STAGE_MISS_BIT_KHR;
	case SStage::intersection: return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
	case SStage::callable:     return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
#endif

	// Provided by VK_EXT_mesh_shader
#if defined(VK_VERSION_1_1) && defined(VK_EXT_MESH_SHADER_EXTENSION_NAME)
	case SStage::task:         return VK_SHADER_STAGE_TASK_BIT_EXT;
	case SStage::mesh:         return VK_SHADER_STAGE_MESH_BIT_EXT;

	case SStage::all_graphics: return VK_SHADER_STAGE_ALL_GRAPHICS;
	case SStage::all:          return VK_SHADER_STAGE_ALL;
#endif

	default:
		vtek_log_error("vtek::get_shader_stage: Invalid stage!");
		return static_cast<VkShaderStageFlagBits>(0);
	}
}

VkShaderStageFlagBits vtek::get_shader_stage_graphics(SSGraphics stage)
{
	switch (stage)
	{
	case SSGraphics::vertex:               return VK_SHADER_STAGE_VERTEX_BIT;
	case SSGraphics::tessellation_control: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	case SSGraphics::tessellation_eval:    return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	case SSGraphics::geometry:             return VK_SHADER_STAGE_GEOMETRY_BIT;
	case SSGraphics::fragment:             return VK_SHADER_STAGE_FRAGMENT_BIT;

	default:
		vtek_log_error("vtek::get_shader_stage_graphics: Invalid stage!");
		return static_cast<VkShaderStageFlagBits>(0);
	}
}

VkShaderStageFlagBits vtek::get_shader_stage_ray_tracing(SSRayTrace stage)
{
	switch (stage)
	{
	case SSRayTrace::raygen:       return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	case SSRayTrace::any_hit:      return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
	case SSRayTrace::closest_hit:  return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	case SSRayTrace::miss:         return VK_SHADER_STAGE_MISS_BIT_KHR;
	case SSRayTrace::intersection: return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
	case SSRayTrace::callable:     return VK_SHADER_STAGE_CALLABLE_BIT_KHR;

	default:
		vtek_log_error("vtek::get_shader_stage_ray_tracing: Invalid stage!");
		return static_cast<VkShaderStageFlagBits>(0);
	}
}

VkShaderStageFlags vtek::get_shader_stage_flags_graphics(
	vtek::EnumBitmask<vtek::ShaderStageGraphics> mask)
{
	VkShaderStageFlags flags = 0U;

	if (mask.has_flag(SSGraphics::vertex)) {
		flags |= VK_SHADER_STAGE_VERTEX_BIT;
	}
	if (mask.has_flag(SSGraphics::tessellation_control)) {
		flags |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	}
	if (mask.has_flag(SSGraphics::tessellation_eval)) {
		flags |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	}
	if (mask.has_flag(SSGraphics::geometry)) {
		flags |= VK_SHADER_STAGE_GEOMETRY_BIT;
	}
	if (mask.has_flag(SSGraphics::fragment)) {
		flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
	}

	return flags;
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

	std::vector<vtek::GraphicsShaderModule> modules;

	// NOTE: Both geometry and tessellation shaders required physical device
	// features be enabled!
	const VkPhysicalDeviceFeatures* physDevFeatures =
		vtek::device_get_enabled_features(device);

	if (info->vertex)
	{
		VkShaderModule vertex = load_glsl_shader(
			shaderdir, sFilenamesGLSL[0], "vertex", SSGraphics::vertex, device);
		if (vertex == VK_NULL_HANDLE)
		{
			vtek_log_error("--> cannot create graphics shader.");
			return nullptr;
		}
		modules.push_back({ SSGraphics::vertex, vertex });
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

		VkShaderModule tess_control = load_glsl_shader(
			shaderdir, sFilenamesGLSL[1], "tessellation control",
			SSGraphics::tessellation_control, device);
		if (tess_control == VK_NULL_HANDLE)
		{
			vtek_log_error("--> cannot create graphics shader.");
			return nullptr;
		}
		modules.push_back({ SSGraphics::tessellation_control, tess_control });
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

		VkShaderModule tess_eval = load_glsl_shader(
			shaderdir, sFilenamesGLSL[2], "tessellation evaluation",
			SSGraphics::tessellation_eval, device);
		if (tess_eval == VK_NULL_HANDLE)
		{
			vtek_log_error("--> cannot create graphics shader.");
			return nullptr;
		}
		modules.push_back({ SSGraphics::tessellation_eval, tess_eval });
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

		VkShaderModule geometry = load_glsl_shader(
			shaderdir, sFilenamesGLSL[3], "geometry", SSGraphics::geometry, device);
		if (geometry == VK_NULL_HANDLE)
		{
			vtek_log_error("--> cannot create graphics shader.");
			return nullptr;
		}
		modules.push_back({ SSGraphics::geometry, geometry });
	}
	if (info->fragment)
	{
		VkShaderModule fragment = load_glsl_shader(
			shaderdir, sFilenamesGLSL[4], "fragment", SSGraphics::fragment, device);
		if (fragment == VK_NULL_HANDLE)
		{
			vtek_log_error("--> cannot create graphics shader.");
			return nullptr;
		}
		modules.push_back({ SSGraphics::fragment, fragment });
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

	// NOTE: Both geometry and tessellation shaders required physical device
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
		modules.push_back({ SSGraphics::vertex, vertex });
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
		modules.push_back({ SSGraphics::tessellation_control, tess_control });
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
		modules.push_back({ SSGraphics::tessellation_eval, tess_eval });
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
		modules.push_back({ SSGraphics::geometry, geometry });
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
		modules.push_back({ SSGraphics::fragment, fragment });
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
