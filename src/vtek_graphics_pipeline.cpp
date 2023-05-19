#include "vtek_graphics_pipeline.hpp"

#include "impl/vtek_vulkan_helpers.hpp"
#include "vtek_device.hpp"
#include "vtek_logging.hpp"
#include "vtek_render_pass.hpp"
#include "vtek_shaders.hpp"
#include "vtek_vulkan_version.hpp"

#include <vulkan/vk_enum_string_helper.h>


/* struct implementation */
struct vtek::GraphicsPipeline
{
	VkPipeline vulkanHandle {VK_NULL_HANDLE};
	VkPipelineLayout layoutHandle {VK_NULL_HANDLE};
	vtek::RenderPassType renderPassType {vtek::RenderPassType::renderpass};
};


/* helper functions */
static void get_enabled_dynamic_states(
	const vtek::GraphicsPipelineCreateInfo* info, vtek::Device* device,
	std::vector<VkDynamicState>& states)
{
	using PDState = vtek::PipelineDynamicState;
	states.clear();

	const vtek::VulkanVersion* apiVersion = vtek::device_get_vulkan_version(device);
	vtek::EnumBitmask<PDState> ds = info->dynamicStateFlags;

	auto add = [&states](VkDynamicState s) { states.push_back(s); };

	// Provided by VK_VERSION_1_0
	if (ds.has_flag(PDState::viewport))             add(VK_DYNAMIC_STATE_VIEWPORT);
	if (ds.has_flag(PDState::scissor))              add(VK_DYNAMIC_STATE_SCISSOR);
	if (ds.has_flag(PDState::line_width))           add(VK_DYNAMIC_STATE_LINE_WIDTH);
	if (ds.has_flag(PDState::depth_bias))           add(VK_DYNAMIC_STATE_DEPTH_BIAS);
	if (ds.has_flag(PDState::blend_constants))      add(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
	if (ds.has_flag(PDState::depth_bounds))         add(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
	if (ds.has_flag(PDState::stencil_compare_mask)) add(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
	if (ds.has_flag(PDState::stencil_write_mask))   add(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
	if (ds.has_flag(PDState::stencil_reference))    add(VK_DYNAMIC_STATE_STENCIL_REFERENCE);

	// TODO: Error handling for when the device doesn't support >= Vulkan 1.3!
	// Provided by VK_VERSION_1_3
#if defined(VK_API_VERSION_1_3)
	if (apiVersion->major() >= 1 && apiVersion->minor() >= 3)
	{
		if (ds.has_flag(PDState::cull_mode))
			add(VK_DYNAMIC_STATE_CULL_MODE);
		if (ds.has_flag(PDState::front_face))
			add(VK_DYNAMIC_STATE_FRONT_FACE);
		if (ds.has_flag(PDState::primitive_topology))
			add(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY);
		if (ds.has_flag(PDState::viewport_with_count))
			add(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
		if (ds.has_flag(PDState::scissor_with_count))
			add(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
		if (ds.has_flag(PDState::vertex_input_binding_stride))
			add(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
		if (ds.has_flag(PDState::depth_test_enable))
			add(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE);
		if (ds.has_flag(PDState::depth_write_enable))
			add(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE);
		if (ds.has_flag(PDState::depth_compare_op))
			add(VK_DYNAMIC_STATE_DEPTH_COMPARE_OP);
		if (ds.has_flag(PDState::depth_bounds_test_enable))
			add(VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE);
		if (ds.has_flag(PDState::stencil_test_enable))
			add(VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE);
		if (ds.has_flag(PDState::stencil_op))
			add(VK_DYNAMIC_STATE_STENCIL_OP);
		if (ds.has_flag(PDState::rasterizer_discard_enable))
			add(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE);
		if (ds.has_flag(PDState::depth_bias_enable))
			add(VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE);
		if (ds.has_flag(PDState::primitive_restart_enable))
			add(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE);
	}
#endif
}

static VkPrimitiveTopology get_primitive_topology(vtek::PrimitiveTopology topology)
{
	switch (topology)
	{
	case vtek::PrimitiveTopology::point_list:
		return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	case vtek::PrimitiveTopology::line_list:
		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	case vtek::PrimitiveTopology::line_strip:
		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	case vtek::PrimitiveTopology::triangle_list:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	case vtek::PrimitiveTopology::triangle_strip:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	case vtek::PrimitiveTopology::triangle_fan:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
	case vtek::PrimitiveTopology::line_list_with_adjacency:
		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
	case vtek::PrimitiveTopology::line_strip_with_adjacency:
		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
	case vtek::PrimitiveTopology::triangle_list_with_adjacency:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
	case vtek::PrimitiveTopology::triangle_strip_with_adjacency:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
	case vtek::PrimitiveTopology::patch_list:
		return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
	default:
		vtek_log_error("vtek_graphics_pipeline.cpp: Invalid primitive topology!");
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	}
}

static VkPolygonMode get_polygon_mode(vtek::PolygonMode mode)
{
	switch (mode)
	{
	case vtek::PolygonMode::fill: return VK_POLYGON_MODE_FILL;
	case vtek::PolygonMode::line: return VK_POLYGON_MODE_LINE;
	case vtek::PolygonMode::point: return VK_POLYGON_MODE_POINT;
	default:
		vtek_log_error("vtek_graphics_pipeline.cpp: Invalid polygon mode!");
		return VK_POLYGON_MODE_FILL;
	}
}

static VkCullModeFlags get_cull_mode_flags(vtek::CullMode mode)
{
	switch (mode)
	{
	case vtek::CullMode::none:           return VK_CULL_MODE_NONE;
	case vtek::CullMode::front:          return VK_CULL_MODE_FRONT_BIT;
	case vtek::CullMode::back:           return VK_CULL_MODE_BACK_BIT;
	case vtek::CullMode::front_and_back: return VK_CULL_MODE_FRONT_AND_BACK;
	default:
		vtek_log_error("vtek_graphics_pipeline.cpp: Invalid cull mode!");
		return VK_CULL_MODE_NONE;
	}
}

static VkFrontFace get_front_face(vtek::FrontFace face)
{
	switch (face)
	{
	case vtek::FrontFace::clockwise:         return VK_FRONT_FACE_COUNTER_CLOCKWISE;
	case vtek::FrontFace::counter_clockwise: return VK_FRONT_FACE_CLOCKWISE;
	default:
		vtek_log_error("vtek_graphics_pipeline.cpp: Invalid front face!");
		return VK_FRONT_FACE_COUNTER_CLOCKWISE;
	}
}

static VkSampleCountFlagBits get_multisample_count(vtek::MultisampleType sample)
{
	switch (sample)
	{
	case vtek::MultisampleType::none:     return VK_SAMPLE_COUNT_1_BIT;
	case vtek::MultisampleType::msaa_x2:  return VK_SAMPLE_COUNT_2_BIT;
	case vtek::MultisampleType::msaa_x4:  return VK_SAMPLE_COUNT_4_BIT;
	case vtek::MultisampleType::msaa_x8:  return VK_SAMPLE_COUNT_8_BIT;
	case vtek::MultisampleType::msaa_x16: return VK_SAMPLE_COUNT_16_BIT;
	case vtek::MultisampleType::msaa_x32: return VK_SAMPLE_COUNT_32_BIT;
	case vtek::MultisampleType::msaa_x64: return VK_SAMPLE_COUNT_64_BIT;
	default:
		vtek_log_error("vtek_graphics_pipeline.cpp: Invalid multisample count!");
		return VK_SAMPLE_COUNT_1_BIT;
	}
}

static VkCompareOp get_depth_compare_op(vtek::DepthCompareOp op)
{
	switch (op)
	{
	case vtek::DepthCompareOp::never:         return VK_COMPARE_OP_NEVER;
	case vtek::DepthCompareOp::less:          return VK_COMPARE_OP_LESS;
	case vtek::DepthCompareOp::equal:         return VK_COMPARE_OP_EQUAL;
	case vtek::DepthCompareOp::less_equal:    return VK_COMPARE_OP_LESS_OR_EQUAL;
	case vtek::DepthCompareOp::greater:       return VK_COMPARE_OP_GREATER;
	case vtek::DepthCompareOp::not_equal:     return VK_COMPARE_OP_NOT_EQUAL;
	case vtek::DepthCompareOp::greater_equal: return VK_COMPARE_OP_GREATER_OR_EQUAL;
	case vtek::DepthCompareOp::always:        return VK_COMPARE_OP_ALWAYS;
	default:
		vtek_log_error("vtek_graphics_pipeline.cpp: Invalid depth compare op!");
		return VK_COMPARE_OP_NEVER;
	}
}

static VkLogicOp get_logic_op(vtek::LogicOp op)
{
	switch (op)
	{
	case vtek::LogicOp::clear:         return VK_LOGIC_OP_CLEAR;
	case vtek::LogicOp::and_op:        return VK_LOGIC_OP_AND;
	case vtek::LogicOp::and_reverse:   return VK_LOGIC_OP_AND_REVERSE;
	case vtek::LogicOp::copy:          return VK_LOGIC_OP_COPY;
	case vtek::LogicOp::and_inverted:  return VK_LOGIC_OP_AND_INVERTED;
	case vtek::LogicOp::no_op:         return VK_LOGIC_OP_NO_OP;
	case vtek::LogicOp::xor_op:        return VK_LOGIC_OP_XOR;
	case vtek::LogicOp::or_op:         return VK_LOGIC_OP_OR;
	case vtek::LogicOp::nor:           return VK_LOGIC_OP_NOR;
	case vtek::LogicOp::equivalent:    return VK_LOGIC_OP_EQUIVALENT;
	case vtek::LogicOp::invert:        return VK_LOGIC_OP_INVERT;
	case vtek::LogicOp::or_reverse:    return VK_LOGIC_OP_OR_REVERSE;
	case vtek::LogicOp::copy_inverted: return VK_LOGIC_OP_COPY_INVERTED;
	case vtek::LogicOp::or_inverted:   return VK_LOGIC_OP_OR_INVERTED;
	case vtek::LogicOp::nand:          return VK_LOGIC_OP_NAND;
	case vtek::LogicOp::set:           return VK_LOGIC_OP_SET;
	default:
		vtek_log_error("vtek_graphics_pipeline.cpp: Invalid logic compare op!");
		return VK_LOGIC_OP_CLEAR;
	}
}


/* interface */
vtek::GraphicsPipeline* vtek::graphics_pipeline_create(
	const vtek::GraphicsPipelineCreateInfo* info, vtek::Device* device)
{
	VkDevice dev = vtek::device_get_handle(device);
	auto devEnabledFeatures = vtek::device_get_enabled_features(device);
	auto enabledExtensions = vtek::device_get_enabled_extensions(device);

	// REVIEW: Could extract function for each state to prettify code.

	/* Neat and ordered: */

	// ===================== //
	// === Shader stages === //
	// ===================== //
	if (info->shader == nullptr)
	{
		vtek_log_error("No shader provided - cannot create graphics pipeline!");
		return nullptr;
	}
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	auto modules = vtek::graphics_shader_get_modules(info->shader);
	bool useFragmentShader = false;
	for (const auto& module : modules)
	{
		VkPipelineShaderStageCreateInfo shaderInfo{};
		shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderInfo.pNext = nullptr;
		shaderInfo.flags = 0;
		shaderInfo.stage = vtek::get_shader_stage_graphics(module.stage);
		shaderInfo.module = module.module;
		shaderInfo.pName = "main"; // REVIEW: How to set this properly?
		shaderInfo.pSpecializationInfo = nullptr; // VkSpecializationInfo*

		shaderStages.emplace_back(shaderInfo);

		if (module.stage == vtek::ShaderStageGraphics::fragment)
		{
			useFragmentShader = true;
		}
	}

	// ==================== //
	// === Vertex input === //
	// ==================== //
	VkPipelineVertexInputStateCreateInfo vertexInfo{};
	vertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vtek::VertexBufferBindings* vertexBindings = info->vertexInputBindings;
	if (vertexBindings == nullptr)
	{
		vertexInfo.vertexBindingDescriptionCount = 0;
		vertexInfo.pVertexBindingDescriptions = nullptr;
		vertexInfo.vertexAttributeDescriptionCount = 0;
		vertexInfo.pVertexAttributeDescriptions = nullptr;
	}
	else
	{
		auto bindings = vertexBindings->GetBindingDescriptions();
		auto attributes = vertexBindings->GetAttributeDescriptions();
		vertexInfo.vertexBindingDescriptionCount = bindings.size();
		vertexInfo.pVertexBindingDescriptions = bindings.data();
		vertexInfo.vertexAttributeDescriptionCount = attributes.size();
		vertexInfo.pVertexAttributeDescriptions = attributes.data();
	}

	// ====================== //
	// === Input assembly === //
	// ====================== //
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = get_primitive_topology(info->primitiveTopology);
	inputAssembly.primitiveRestartEnable = info->enablePrimitiveRestart.get();

	// ====================== //
	// === Viewport state === //
	// ====================== //
	if (info->viewportState == nullptr)
	{
		vtek_log_error("No viewport state provided - cannot create graphics pipeline!");
		return nullptr;
	}
	vtek::ViewportState viewportState = *(info->viewportState); // copy-by-value
	VkViewport viewport{};
	viewport.x = viewportState.viewportRegion.offset.x;
	viewport.y = viewportState.viewportRegion.offset.y;
	viewport.width = viewportState.viewportRegion.extent.width;
	viewport.height = viewportState.viewportRegion.extent.height;
	viewport.minDepth = viewportState.depthRange.min();
	viewport.maxDepth = viewportState.depthRange.max();
	// TODO: Multiple viewport states?
	// TODO: Using multiple viewports/scissors requires enabling a feature
	// TODO: during device creation!
	VkPipelineViewportStateCreateInfo viewportInfo{};
	viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportInfo.viewportCount = 1;
	viewportInfo.pViewports = &viewport;
	viewportInfo.scissorCount = 1;
	viewportInfo.pScissors = (viewportState.useScissorRegion)
		? &viewportState.scissorRegion
		: &viewportState.viewportRegion;

	// =========================== //
	// === Rasterization state === //
	// =========================== //
	if (info->rasterizationState == nullptr)
	{
		vtek_log_error(
			"No rasterization state provided - cannot create graphics pipeline!");
		return nullptr;
	}
	vtek::RasterizationState rasterizationState = *(info->rasterizationState); // copy-by-value
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.pNext = nullptr;
	rasterizer.flags = 0U; // reserved for future use (Vulkan 1.3)
	rasterizer.depthClampEnable = rasterizationState.depthClampEnable.get();
	rasterizer.rasterizerDiscardEnable = rasterizationState.rasterizerDiscardEnable.get();
	rasterizer.polygonMode = get_polygon_mode(rasterizationState.polygonMode);
	rasterizer.cullMode = get_cull_mode_flags(rasterizationState.cullMode);
	rasterizer.frontFace = get_front_face(rasterizationState.frontFace);
	rasterizer.depthBiasEnable = rasterizationState.depthBiasEnable.get();
	rasterizer.depthBiasConstantFactor = rasterizationState.depthBiasConstantFactor;
	rasterizer.depthBiasClamp = rasterizationState.depthBiasClamp;
	rasterizer.depthBiasSlopeFactor = rasterizationState.depthBiasSlopeFactor;
	rasterizer.lineWidth = rasterizationState.lineWidth;

	if ((rasterizer.depthClampEnable == VK_TRUE) &&
	    (devEnabledFeatures->depthClamp == VK_FALSE))
	{
		vtek_log_error("DepthClamp feature was not enabled during device creation!");
		vtek_log_warn("Pipeline will disable it. Application might not work correctly!");
		rasterizer.depthClampEnable = VK_FALSE;
	}
	if ((rasterizer.polygonMode != VK_POLYGON_MODE_FILL) &&
	    (devEnabledFeatures->fillModeNonSolid == VK_FALSE))
	{
		vtek_log_error("FillModeNonSolid feature was not enabled during device creation!");
		vtek_log_warn("Pipeline will set polygonMode to default (=SOLID).");
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	}
	if ((rasterizer.lineWidth > 1.0f) && (devEnabledFeatures->wideLines == VK_FALSE))
	{
		vtek_log_error("WideLines feature was not enabled during device creation!");
		vtek_log_warn("Pipeline will clamp lineWidth to default (=1.0f).");
		rasterizer.lineWidth = 1.0f;
	}

	if ((!rasterizationState.rasterizerDiscardEnable.get()) && (!useFragmentShader))
	{
		vtek_log_error("Rasterizer discard disabled, but no fragment shader module found!");
		vtek_log_error("--> cannot create graphics pipeline!");
		return nullptr;
	}
	else if (rasterizationState.rasterizerDiscardEnable.get() && useFragmentShader)
	{
		vtek_log_error("Rasterizer discard enabled, but fragment shader module found!");
		vtek_log_warn("Rasterization is disabled, and the fragment shader will be ignored.");
	}

	// ========================= //
	// === Multisample state === //
	// ========================= //
	if (info->multisampleState == nullptr)
	{
		vtek_log_error(
			"No multisample state provided - cannot create graphics pipeline!");
		return nullptr;
	}
	vtek::MultisampleState multisampleState = *(info->multisampleState); // copy-by-value
	VkPipelineMultisampleStateCreateInfo multisample{};
	multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample.pNext = nullptr;
	multisample.flags = 0U;
	multisample.rasterizationSamples = get_multisample_count(multisampleState.numSamples);
	multisample.sampleShadingEnable = multisampleState.enableSampleRateShading.get();
	multisample.pSampleMask = nullptr;
	multisample.alphaToCoverageEnable = multisampleState.enableAlphaToCoverage.get();
	multisample.alphaToOneEnable = multisampleState.enableAlphaToOne.get();

	if ((multisample.sampleShadingEnable == VK_TRUE) &&
	    (devEnabledFeatures->sampleRateShading == VK_FALSE))
	{
		vtek_log_error("SampleRateShading feature was not enabled during device creation!");
		vtek_log_warn("Pipeline will disable it.");
		multisample.sampleShadingEnable = VK_FALSE;
	}

	// =========================== //
	// === Depth stencil state === //
	// =========================== //
	if (info->depthStencilState == nullptr)
	{
		vtek_log_error(
			"No depth stencil state provided - cannot create graphics pipeline!");
		return nullptr;
	}
	vtek::DepthStencilState dsState = *(info->depthStencilState); // copy-by-value
	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.pNext = nullptr;
	depthStencil.flags = 0;
	depthStencil.depthTestEnable = dsState.depthTestEnable.get();
	depthStencil.depthWriteEnable = dsState.depthWriteEnable.get();
	depthStencil.depthCompareOp = get_depth_compare_op(dsState.depthCompareOp);
	depthStencil.depthBoundsTestEnable = dsState.depthBoundsTestEnable.get();
	depthStencil.stencilTestEnable = dsState.stencilTestEnable.get();
	if (depthStencil.stencilTestEnable) {
		depthStencil.front = dsState.stencilTestFront;
		depthStencil.back = dsState.stencilTestBack;
	} else {
		depthStencil.front = {};
		depthStencil.back = {};
	}
	depthStencil.minDepthBounds = dsState.depthBounds.min();
	depthStencil.maxDepthBounds = dsState.depthBounds.max();

	// After reading depth/stencil state, we can make some modifications to the
	// multisample count, depending on whether depth/stencil testing has been
	// enabled. This will be equivalent to depth/stencil buffers being attached
	// to the render pass/render pass instance associated with this pipeline.
	vtek::SampleCountQuery sampleQuery = {
		.color = true,
		.depth = (depthStencil.depthTestEnable == VK_TRUE),
		.stencil = (depthStencil.stencilTestEnable == VK_TRUE)
	};
	VkSampleCountFlagBits maxSampleCount =
		vtek::device_get_max_sample_count(device, &sampleQuery);
	if (multisample.rasterizationSamples > maxSampleCount)
	{
		vtek_log_error("Pipeline multisample state was set to {}, but {} {}!",
		               string_VkSampleCountFlags(multisample.rasterizationSamples),
		               "the physical device supports maximum",
		               string_VkSampleCountFlags(maxSampleCount));
		vtek_log_warn("The number of samples will be clamped. {}",
		              "Application might not run correctly!");
		multisample.rasterizationSamples = maxSampleCount;
	}

	// ============================ //
	// === color blending state === //
	// ============================ //
	if (info->colorBlendState == nullptr)
	{
		vtek_log_error(
			"No color blending state provided - cannot create graphics pipeline!");
		return nullptr;
	}
	if (info->colorBlendState->attachments.empty() == !rasterizer.rasterizerDiscardEnable)
	{
		vtek_log_error("No color attachments provided in the color blending state, {}",
		               "and rasterizer discard is not enabled.");
		vtek_log_error("--> Cannot create graphics pipeline!");
		return nullptr;
	}
	std::vector<VkPipelineColorBlendAttachmentState> colorAttachments{};
	for (auto attachment : info->colorBlendState->attachments)
	{
		VkPipelineColorBlendAttachmentState state{};

		state.blendEnable = attachment.blendEnable.get();
		state.srcColorBlendFactor = attachment.srcColorBlendFactor;
		state.dstColorBlendFactor = attachment.dstColorBlendFactor;
		state.colorBlendOp = attachment.colorBlendOp;
		state.srcAlphaBlendFactor = attachment.srcAlphaBlendFactor;
		state.dstAlphaBlendFactor = attachment.dstAlphaBlendFactor;
		state.alphaBlendOp = attachment.alphaBlendOp;
		state.colorWriteMask = attachment.colorWriteMask;

		colorAttachments.emplace_back(state);
	}
	VkPipelineColorBlendStateCreateInfo colorBlend{};
	colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlend.pNext = nullptr;
	colorBlend.flags = 0; // reserved for future use (> Vulkan 1.3)
	colorBlend.logicOpEnable = info->colorBlendState->logicOpEnable.get();
	colorBlend.logicOp = get_logic_op(info->colorBlendState->logicOp);
	colorBlend.attachmentCount = static_cast<uint32_t>(colorAttachments.size());
	colorBlend.pAttachments = colorAttachments.data();
	colorBlend.blendConstants[0] = 0.0f;
	colorBlend.blendConstants[1] = 0.0f;
	colorBlend.blendConstants[2] = 0.0f;
	colorBlend.blendConstants[3] = 0.0f;

	// ===================== //
	// === Dynamic state === //
	// ===================== //
	std::vector<VkDynamicState> dynamicStates;
	get_enabled_dynamic_states(info, device, dynamicStates);
	bool dynState = dynamicStates.size() > 0;
	VkPipelineDynamicStateCreateInfo dynamic{};
	dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic.dynamicStateCount = (dynState) ? dynamicStates.size() : 0U;
	dynamic.pDynamicStates = (dynState) ? dynamicStates.data() : nullptr;

	// ======================= //
	// === Pipeline layout === //
	// ======================= //
	VkPipelineLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.pNext = nullptr;
	layoutInfo.flags = 0U; // reserved for future use (Vulkan 1.3)
	// TODO: Descriptor sets
	layoutInfo.setLayoutCount = 0;
	layoutInfo.pSetLayouts = nullptr;
	layoutInfo.pushConstantRangeCount = 0;
	layoutInfo.pPushConstantRanges = nullptr;
	VkPushConstantRange pushConstantRange{};
	if (info->pushConstantType != vtek::PushConstantType::none)
	{
		if (info->pushConstantShaderStages.empty())
		{
			vtek_log_error(
				"Push constant type was specified, but not its shader stages!");
			vtek_log_error("--> Cannot create graphics pipeline!");
			return nullptr;
		}

		// NOTE: Only 1 push constant is supported, and not multiple ranges!
		// This could be changed, if the need arises.
		pushConstantRange.stageFlags =
			vtek::get_shader_stage_flags_graphics(info->pushConstantShaderStages);
		pushConstantRange.offset = 0;
		pushConstantRange.size = vtek::push_constant_size(info->pushConstantType);

		layoutInfo.pushConstantRangeCount = 1;
		layoutInfo.pPushConstantRanges = &pushConstantRange;
	}

	VkPipelineLayout layout {VK_NULL_HANDLE};
	VkResult layoutResult = vkCreatePipelineLayout(dev, &layoutInfo, nullptr, &layout);
	if (layoutResult != VK_SUCCESS)
	{
		vtek_log_error("Failed to create graphics pipeline layout!");
		return nullptr;
	}


	// TODO: Get this from shader!
	// VkDescriptorSetLayout descriptorSetLayout =
	// 	vtek::graphics_shader_get_descriptor_layout(info->shader);

	// // TODO: This is probably, _maybe_, an issue?
	// // layoutInfo.setLayoutCount = 0; // ??
	// // layoutInfo.pSetLayouts = nullptr; // ??
	// VkDescriptorSetLayoutCreateInfo setLayoutInfo{};
	// setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	// setLayoutInfo.pNext = nullptr;
	// setLayoutInfo.flags = 0; // ??
	// setLayoutInfo.bindingCount = 0;
	// setLayoutInfo.pBindings = nullptr;

	// VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	// VkResult setLayoutResult = vkCreateDescriptorSetLayout(dev, &setLayoutInfo, nullptr, &descriptorSetLayout);
	// if (setLayoutResult != VK_SUCCESS)
	// {
	// 	vtek_log_error("EXPERIMENTAL: Failed to create graphics pipeline descriptor set layout!");
	// 	return nullptr;
	// }

	// VkPipelineLayoutCreateInfo layoutInfo{};
	// layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	// layoutInfo.pNext = nullptr;
	// layoutInfo.flags = 0U; // reserved for future use (Vulkan 1.3)
	// layoutInfo.setLayoutCount = 0; // 1;
	// layoutInfo.pSetLayouts = nullptr; // &descriptorSetLayout;
	// layoutInfo.pushConstantRangeCount = 0; // ??
	// layoutInfo.pPushConstantRanges = nullptr; // ??
	// // TODO: Library to extract descriptor layout from Spir-V ??

	// VkPipelineLayout layout {VK_NULL_HANDLE};
	// VkResult layoutResult = vkCreatePipelineLayout(dev, &layoutInfo, nullptr, &layout);
	// if (layoutResult != VK_SUCCESS)
	// {
	// 	vtek_log_error("Failed to create graphics pipeline layout!");
	// 	return nullptr;
	// }

	// ========================= //
	// === Dynamic rendering === // -- alternative to providing a render pass.
	// ========================= //
	VkPipelineRenderingCreateInfo renderingCreateInfo;
	bool useDynamicRendering = (info->renderPassType == vtek::RenderPassType::dynamic);
	if (useDynamicRendering)
	{
		auto pipRender = info->pipelineRendering;
		const std::vector<VkFormat>& attachments = pipRender->colorAttachmentFormats;

		// Error handling ensured..

		if (info->pipelineRendering == nullptr)
		{
			vtek_log_error("No pipeline rendering object provided for dynamic rendering.");
			vtek_log_error("--> cannot create graphics pipeline!");
			return nullptr;
		}

		if (!enabledExtensions->dynamicRendering)
		{
			vtek_log_error(
				"Dynamic rendering extension was not enabled during device creation!");
			vtek_log_error("--> Require it when picking a physical device.");
			vtek_log_error("--> Cannot create graphics pipeline!");
			return nullptr;
		}

		bool attachmentCountMatch = attachments.size() == colorAttachments.size();
		if (useDynamicRendering && !attachmentCountMatch)
		{
			vtek_log_error(
				"For dynamic rendering, number of attachments must match exactly for:");
			vtek_log_error("--> Color blending state and pipeline rendering state.");
			vtek_log_error("--> Cannot create graphics pipeline!");
			return nullptr;
		}

		bool depthSpecified = pipRender->depthAttachmentFormat != VK_FORMAT_UNDEFINED;
		if (!depthSpecified && dsState.depthWriteEnable.get())
		{
			vtek_log_error("No depth attachment format provided for dynamic rendering.");
			vtek_log_error("--> But was specified for color blending state.");
			vtek_log_error("--> Cannot create graphics pipeline!");
			return nullptr;
		}

		bool stencilSpecified = pipRender->stencilAttachmentFormat != VK_FORMAT_UNDEFINED;
		if (!stencilSpecified && dsState.stencilTestEnable.get())
		{
			vtek_log_error("No stencil attachment format provided for dynamic rendering.");
			vtek_log_error("--> But was specified for color blending state.");
			vtek_log_error("--> Cannot create graphics pipeline!");
			return nullptr;
		}

		// Fill rendering info struct
		renderingCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.pNext = nullptr,
			.viewMask = 0, // TODO: uint32_t ?
			.colorAttachmentCount = static_cast<uint32_t>(attachments.size()),
			.pColorAttachmentFormats = attachments.data(),
			.depthAttachmentFormat = pipRender->depthAttachmentFormat,
			.stencilAttachmentFormat = pipRender->stencilAttachmentFormat
		};
	}
	else
	{
		if (info->renderPass == nullptr)
		{
			vtek_log_error("No render pass provided for non-dynamic rendering.");
			vtek_log_error("--> cannot create graphics pipeline!");
			return nullptr;
		}
	}

	// ============================= //
	// === Creating the pipeline === //
	// ============================= //
	// TODO: How to handle pipeline cache?
	// TODO: How to handle multiple pipelines?
	// TODO: How to handle derived pipelines?
	VkGraphicsPipelineCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.pNext = (useDynamicRendering) ? &renderingCreateInfo : nullptr;
	createInfo.flags = 0U; // bitmask of VkPipelineCreateFlagBits...
	// TODO: Check usage for these flags:
	/*
	// Provided by VK_VERSION_1_3
	VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT = 0x00000100,
	// Provided by VK_VERSION_1_3
	VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT = 0x00000200,
	*/
	createInfo.stageCount = shaderStages.size();
	createInfo.pStages = shaderStages.data();
	createInfo.pVertexInputState = &vertexInfo;
	createInfo.pInputAssemblyState = &inputAssembly;
	createInfo.pTessellationState = nullptr; // ??
	createInfo.pViewportState = &viewportInfo;
	createInfo.pRasterizationState = &rasterizer;
	createInfo.pMultisampleState = &multisample;
	createInfo.pDepthStencilState = &depthStencil;
	createInfo.pColorBlendState = &colorBlend;
	createInfo.pDynamicState = &dynamic;
	createInfo.layout = layout;
	createInfo.renderPass = (useDynamicRendering)
		? VK_NULL_HANDLE : vtek::render_pass_get_handle(info->renderPass);
	createInfo.subpass = int32_t{0};
	createInfo.basePipelineHandle = VK_NULL_HANDLE;
	createInfo.basePipelineIndex = int32_t{0};

	VkPipeline outHandle = VK_NULL_HANDLE; // TODO: Temporary!
	VkResult result = vkCreateGraphicsPipelines(
		dev, VK_NULL_HANDLE, 1, &createInfo, nullptr, &outHandle);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to create graphics pipeline!");
		return nullptr;
	}

	// TODO: We definitely need a pipeline allocator!
	auto pipeline = new vtek::GraphicsPipeline();
	pipeline->vulkanHandle = outHandle;
	pipeline->layoutHandle = layout;
	pipeline->renderPassType = info->renderPassType;

	return pipeline;
}

void vtek::graphics_pipeline_destroy(vtek::GraphicsPipeline* pipeline, vtek::Device* device)
{
	if (pipeline == nullptr) { return; }

	VkDevice dev = vtek::device_get_handle(device);

	vkDestroyPipelineLayout(dev, pipeline->layoutHandle, nullptr);
	vkDestroyPipeline(dev, pipeline->vulkanHandle, nullptr);

	pipeline->layoutHandle = VK_NULL_HANDLE;
	pipeline->vulkanHandle = VK_NULL_HANDLE;

	delete pipeline;
}

VkPipeline vtek::graphics_pipeline_get_handle(vtek::GraphicsPipeline* pipeline)
{
	return pipeline->vulkanHandle;
}

VkPipelineLayout vtek::graphics_pipeline_get_layout(vtek::GraphicsPipeline* pipeline)
{
	return pipeline->layoutHandle;
}

vtek::RenderPassType vtek::graphics_pipeline_get_render_pass_type(
	vtek::GraphicsPipeline* pipeline)
{
	return pipeline->renderPassType;
}
