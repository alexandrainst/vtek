#include "vtek_graphics_pipeline.h"
#include "vtek_logging.h"
#include "vtek_shaders.h"
#include "impl/vtek_vulkan_helpers.h"

// TODO: Temporary
#include <iostream>

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
	vtek::PipelineDynamicStateFlags flags = info->dynamicStateFlags;

	// Provided by VK_VERSION_1_0
	if (flags & static_cast<uint32_t>(PDState::viewport))
		states.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	if (flags & static_cast<uint32_t>(PDState::scissor))
		states.push_back(VK_DYNAMIC_STATE_SCISSOR);
	if (flags & static_cast<uint32_t>(PDState::line_width))
		states.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);
	if (flags & static_cast<uint32_t>(PDState::depth_bias))
		states.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
	if (flags & static_cast<uint32_t>(PDState::blend_constants))
		states.push_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
	if (flags & static_cast<uint32_t>(PDState::depth_bounds))
		states.push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
	if (flags & static_cast<uint32_t>(PDState::stencil_compare_mask))
		states.push_back(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
	if (flags & static_cast<uint32_t>(PDState::stencil_write_mask))
		states.push_back(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
	if (flags & static_cast<uint32_t>(PDState::stencil_reference))
		states.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);

	// TODO: Error handling for when the device doesn't support >= Vulkan 1.3!
	// Provided by VK_VERSION_1_3
#if defined(VK_API_VERSION_1_3)
	if (apiVersion->major() >= 1 && apiVersion->minor() >= 3)
	{
		if (flags & static_cast<uint32_t>(PDState::cull_mode))
			states.push_back(VK_DYNAMIC_STATE_CULL_MODE);
		if (flags & static_cast<uint32_t>(PDState::front_face))
			states.push_back(VK_DYNAMIC_STATE_FRONT_FACE);
		if (flags & static_cast<uint32_t>(PDState::primitive_topology))
			states.push_back(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY);
		if (flags & static_cast<uint32_t>(PDState::viewport_with_count))
			states.push_back(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
		if (flags & static_cast<uint32_t>(PDState::scissor_with_count))
			states.push_back(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
		if (flags & static_cast<uint32_t>(PDState::vertex_input_binding_stride))
			states.push_back(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
		if (flags & static_cast<uint32_t>(PDState::depth_test_enable))
			states.push_back(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE);
		if (flags & static_cast<uint32_t>(PDState::depth_write_enable))
			states.push_back(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE);
		if (flags & static_cast<uint32_t>(PDState::depth_compare_op))
			states.push_back(VK_DYNAMIC_STATE_DEPTH_COMPARE_OP);
		if (flags & static_cast<uint32_t>(PDState::depth_bounds_test_enable))
			states.push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE);
		if (flags & static_cast<uint32_t>(PDState::stencil_test_enable))
			states.push_back(VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE);
		if (flags & static_cast<uint32_t>(PDState::stencil_op))
			states.push_back(VK_DYNAMIC_STATE_STENCIL_OP);
		if (flags & static_cast<uint32_t>(PDState::rasterizer_discard_enable))
			states.push_back(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE);
		if (flags & static_cast<uint32_t>(PDState::depth_bias_enable))
			states.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE);
		if (flags & static_cast<uint32_t>(PDState::primitive_restart_enable))
			states.push_back(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE);
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
	}

	// ==================== //
	// === Vertex input === //
	// ==================== //
	// TODO: What to do here?
	//auto& bindingDesc = vtek::vertex_binding_description(info->vertexType);
	auto& attributeDesc = vtek::vertex_attribute_descriptions(info->vertexType);

	VkPipelineVertexInputStateCreateInfo vertexInfo{};
	vertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInfo.vertexBindingDescriptionCount = 1; // ??
	vertexInfo.pVertexBindingDescriptions = nullptr; // ?? shading->bindingDesc ??
	vertexInfo.vertexAttributeDescriptionCount = attributeDesc.size();
	vertexInfo.pVertexAttributeDescriptions = attributeDesc.data();

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
	viewport.width = viewportState.viewportRegion.extent.width;
	// TODO: Multiple viewport states?
	// TODO: Using multiple viewports/scissors requires enabling a feature
	// TODO: during device creation!
	VkPipelineViewportStateCreateInfo viewportInfo{};
	viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportInfo.viewportCount = 1; // ??
	viewportInfo.pViewports = &viewport;
	viewportInfo.scissorCount = (viewportState.useScissorRegion) ? 1 : 0; // ??
	viewportInfo.pScissors = &viewportState.scissorRegion;

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
	// TODO: Also check for enabled device features! This include:
	// depthClampEnable, polygonMode, lineWidth.

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
	// TODO: Perhaps check that device supports number of samples provided?
	multisample.rasterizationSamples = get_multisample_count(multisampleState.numSamples);
	multisample.sampleShadingEnable = multisampleState.enableSampleRateShading.get();
	multisample.pSampleMask = nullptr; // ?? TODO: what is a sample mask ??
	multisample.alphaToCoverageEnable = multisampleState.enableAlphaToCoverage.get();
	multisample.alphaToOneEnable = multisampleState.enableAlphaToOne.get();

	// =========================== //
	// === Depth stencil state === //
	// =========================== //
	if (info->multisampleState == nullptr)
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
		vtek_log_error("No color attachments provided in the color blending state,");
		vtek_log_error("and rasterizer discard is not enabled.");
		vtek_log_error("Cannot create graphics pipeline!");
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
	colorBlend.attachmentCount = colorAttachments.size();
	colorBlend.pAttachments = colorAttachments.data();


	// ===================== //
	// === Dynamic state === //
	// ===================== //
	// TODO: We need a data structure for dynamic states (and correctness check!)
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
	// TODO: Get this from shader!
	VkPipelineLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.pNext = nullptr;
	layoutInfo.flags = 0U; // reserved for future use (Vulkan 1.3)
	layoutInfo.setLayoutCount = 0; // ??
	layoutInfo.pSetLayouts = nullptr; // ??
	layoutInfo.pushConstantRangeCount = 0; // ??
	layoutInfo.pPushConstantRanges = nullptr; // ??
	// TODO: Library to extract descriptor layout from Spir-V ??

	VkPipelineLayout layout {VK_NULL_HANDLE};
	VkResult layoutResult = vkCreatePipelineLayout(dev, &layoutInfo, nullptr, &layout);
	if (layoutResult != VK_SUCCESS)
	{
		vtek_log_error("Failed to create graphics pipeline layout!");
		return nullptr;
	}

	// Dynamic rendering -- alternative to providing a render pass!
	VkPipelineRenderingCreateInfo renderingCreateInfo{};
	bool useDynamicRendering = (info->renderPassType == vtek::RenderPassType::dynamic);
	if (!useDynamicRendering && (info->renderPass == nullptr))
	{
		vtek_log_error("No render pass provided for non-dynamic rendering.");
		vtek_log_error("--> cannot create graphics pipeline!");
		return nullptr;
	}
	renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO; // TODO: guess!
	renderingCreateInfo.pNext = nullptr;
	renderingCreateInfo.viewMask = 0; // TODO: uint32_t ?
	renderingCreateInfo.colorAttachmentCount = 0; // TODO: uint32_t ?
	renderingCreateInfo.pColorAttachmentFormats = nullptr; // TODO: VkFormat* ?
	renderingCreateInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED; // TODO: VkFormat ?
	renderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED; // TODO: VkFormat ?

	// ============================= //
	// === Creating the pipeline === //
	// ============================= //
	// TODO: How to handle pipeline cache?
	// TODO: How to handle multiple pipelines?
	VkGraphicsPipelineCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.pNext = (useDynamicRendering) ? &renderingCreateInfo : nullptr;
	createInfo.flags = 0U; // bitmask of VkPipelineCreateFlagBits...
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







	//createInfo.renderPass = vtek::render_pass_get_handle(info->renderPass);

	VkPipeline outHandle = VK_NULL_HANDLE; // TODO: Temporary!
	VkResult result = vkCreateGraphicsPipelines(
		dev, VK_NULL_HANDLE, 1, &createInfo, nullptr, &outHandle);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to create graphics pipeline!");
		return nullptr;
	}


	return nullptr;
}

void vtek::graphics_pipeline_destroy(vtek::GraphicsPipeline* pipeline)
{

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
