#include "vtek_glsl_shader_utils.hpp"

#include "vtek_logging.hpp"
#include "vtek_physical_device.hpp"

// External dependency: glslang for generating SPIR-V bytecode from GLSL.
// NOTE: We may use the default resource limits, IFF we compile for OpenGL ES.
// --> in practice, this means never! Has two functions though:
// - GetResources() : TBuiltInResource*
// - GetDefaultResources() :TBuiltInResource*
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <forward_list>


/* File I/O shader includer */
using IncludeResult = glslang::TShader::Includer::IncludeResult;

class GlslShaderIncluder : public glslang::TShader::Includer
{
public:
	// NOTE: Documentation taken from the file "ShaderLang.h", copy-pasted
	// here for convenience:
	//
	// Resolves an inclusion request by name, current source name,
	// and include depth.
	// On success, returns an IncludeResult containing the resolved name
	// and content of the include.
	// On failure, returns a nullptr, or an IncludeResult
	// with an empty string for the headerName and error details in the
	// header field.
	// The Includer retains ownership of the contents
	// of the returned IncludeResult value, and those contents must
	// remain valid until the releaseInclude method is called on that
	// IncludeResult object.
	//
	// Note "local" vs. "system" is not an "either/or": "local" is an
	// extra thing to do over "system". Both might get called, as per
	// the C++ specification.
	//
	// For the "system" or <>-style includes; search the "system" paths.
	virtual IncludeResult* includeSystem(
		const char* headerName, const char* includerName, size_t inclusionDepth) override;

	// For the "local"-only aspect of a "" include. Should not search in the
	// "system" paths, because on returning a failure, the parser will
	// call includeSystem() to look in the "system" locations.
	virtual IncludeResult* includeLocal(
		const char* headerName, const char* includerName, size_t inclusionDepth) override;

	virtual void releaseInclude(IncludeResult*) override;
};



/* shorthand type aliases */
using SStage = vtek::ShaderStage;
using SSGraphics = vtek::ShaderStageGraphics;
using SSRayTrace = vtek::ShaderStageRayTracing;



/* helper functions */
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



/* interface */
bool vtek::glsl_utils_initialize()
{
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

void vtek::glsl_utils_terminate()
{
	glslang::FinalizeProcess();
}

void vtek::glsl_utils_build_resource_limits(
	const vtek::PhysicalDevice* physicalDevice)
{
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

std::vector<uint32_t> vtek::glsl_utils_load_shader(
	vtek::Directory* shaderdir, const char* filename,
	vtek::ShaderStageGraphics stage, vtek::VulkanVersion apiVersion,
	VkDevice dev)
{
	// Open file
	auto flags = vtek::FileModeFlag::read;
	vtek::File* file = vtek::file_open(shaderdir, filename, flags);
	if (file == nullptr)
	{
		vtek_log_error("Failed to open shader file \"{}\"!", filename);
		return {};
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
		vtek_log_error("Failed to read shader file \"{}\"!", filename);
		return {};
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
		vtek_log_error("Invalid shader stage -- cannot load GLSL shader!");
		return {};
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

	// ======================= //
	// === Define settings === //
	// ======================= //
	// NOTE: Version 100 indicated current branch of GLSL->Vulkan extension:
	// https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_vulkan_glsl.txt
	const int version = 100;
	shader.setEnvInput(glslang::EShSourceGlsl, lang, glslang::EShClientVulkan, version);

	// Get the active Vulkan version of the client process.
	// NOTE: This is *probably* a good default.
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

	TBuiltInResource* resources = GetResources();
	// int defaultVersion = 110, // use 100 for ES environment, overridden by #version in shader
	const int defaultVersion = 450;
	const bool forwardCompatible = false;
	const EShMessages messageFlags = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
	EProfile defaultProfile = ENoProfile; // NOTE: Only for desktop, before profiles showed up!

	// ============================= //
	// === Preprocess the shader === //
	// ============================= //
	std::string preprocessedStr;
	if (!shader.preprocess(
		    resources, defaultVersion, defaultProfile, false, forwardCompatible,
		    messageFlags, &preprocessedStr, includer))
	{
		vtek_log_error("Failed to preprocess shader: {}", shader.getInfoLog());
		return {};
	}

	// ======================== //
	// === Parse the shader === //
	// ======================== //
	if (!shader.parse(resources, defaultVersion, forwardCompatible, messageFlags, includer))
	{
		vtek_log_error("Failed to parse shader: {}", shader.getInfoLog());
		return {};
	}

	// NEXT: Now we probably link the shader to create a program

	// TODO: Do we really need to create program and link?? We are doing 1 shader at a time!
	glslang::TProgram program;
	program.addShader(&shader);
	if (!program.link(messageFlags))
	{
		vtek_log_error("Failed to link shader: {}", program.getInfoLog());
		return {};
	}

	// Convert the intermediate generated by glslang to Spir-V
	glslang::TIntermediate& intermediateRef = *(program.getIntermediate(lang));
	std::vector<uint32_t> spirv;
	glslang::SpvOptions options{};
	options.validate = true;
	// TODO: We can also provide a logger to glslang! NOICE:
	// glslang::spv::SpvBuildLogger logger;
	// glslang::GlslangToSpv(intermediateRef, spirv, &logger, &options);
	glslang::GlslangToSpv(intermediateRef, spirv, &options);

	return spirv;
}
