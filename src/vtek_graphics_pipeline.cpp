#include "vtek_graphics_pipeline.h"
#include "vtek_logging.h"
#include "impl/vtek_vulkan_helpers.h"

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
		return VK_POLYGON_MODE_FILL;
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


/* interface */
vtek::GraphicsPipeline* vtek::graphics_pipeline_create(
	const vtek::GraphicsPipelineCreateInfo* info, vtek::Device* device)
{
	VkDevice dev = vtek::device_get_handle(device);

	// REVIEW: Could extract function for each state to prettify code.

	/* Neat and ordered: */

	// shader stages

	// ==================== //
	// === Vertex input === //
	// ==================== //
	auto& bindingDesc = vtek::vertex_binding_description(info->vertexType);
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
	inputAssembly.primitiveRestartEnable =
		vtek::getVulkanBoolean(info->enablePrimitiveRestart);

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
	viewportInfo.pScissors = viewportState.scissorRegion;

	// rasterization
	if (info->rasterizationState == nullptr)
	{
		vtek_log_error("No rasterization state provided - cannot create graphics pipeline!");
		return nullptr;
	}
	vtek::RasterizationState rasterizationState = *(info->rasterizationState); // copy-by-value
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.pNext = nullptr;
	rasterizer.flags = 0U; // reserved for future use (Vulkan 1.3)
	rasterizer.depthClampEnable =
		vtek::getVulkanBoolean(rasterizationState.depthClampEnable);
	rasterizer.rasterizerDiscardEnable =
		vtek::getVulkanBoolean(rasterizationState.rasterizerDiscardEnable);
	rasterizer.polygonMode = get_polygon_mode(rasterizationState.polygonMode);
	rasterizer.cullMode = get_cull_mode_flags(rasterizationState.cullMode);
	rasterizer.frontFace = get_front_face(rasterizationState.frontFace);
	rasterizer.depthBiasEnable =
		vtek::getVulkanBoolean(rasterizationState.depthBiasEnable);
	rasterizer.depthBiasConstantFactor = rasterizationState.depthBiasConstantFactor;
	rasterizer.depthBiasClamp = rasterizationState.depthBiasClamp;
	rasterizer.depthBiasSlopeFactor = rasterizationState.depthBiasSlopeFactor;
	rasterizer.lineWidth = rasterizationState.lineWidth;
		// TODO: Also check for enabled device features!

	// ========================= //
	// === Multisample state === //
	// ========================= //




	// depth and stencil testing

	// color blending

	// dynamic states
	std::vector<VkDynamicState> dynamicStates;
	get_enabled_dynamic_states(info, device, dynamicStates);
	bool dynState = dynamicStates.size() > 0;
	const uint32_t dynamicStateCount = (dynState) ? dynamicStates.size() : 0U;
	const VkDynamicState* pDynamicStates = (dynState) ? dynamicStates.data() : nullptr;

	// pipeline layout
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

	// Dynamic state -- alternative to providing a render pass!
	VkPipelineRenderingCreateInfo renderingCreateInfo{};
	renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO; // TODO: guess!
	renderingCreateInfo.pNext = nullptr;
	renderingCreateInfo.viewMask = 0; // TODO: uint32_t ?
	renderingCreateInfo.colorAttachmentCount = 0; // TODO: uint32_t ?
	renderingCreateInfo.pColorAttachmentFormats = nullptr; // TODO: VkFormat* ?
	renderingCreateInfo.depthAttachmentFormat = VK_INVALID_FORMAT; // TODO: VkFormat ?
	renderingCreateInfo.stencilAttachmentFormat = VK_INVALID_FORMAT; // TODO: VkFormat ?
	if (info->renderPassType == vtek::RenderPassType::dynamic)
	{
		createInfo.pNext = &renderingCreateInfo;
	}
	else
	{
		createInfo.renderPass = vtek::render_pass_get_handle(info->renderPass);
	}

	/* create the graphics pipeline (finally!) */
	// TODO: How to handle pipeline cache?
	// TODO: How to handle multiple pipelines?
	VkGraphicsPipelineCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0U; // bitmask of VkPipelineCreateFlagBits...
	createInfo.stateCount = 0; // ?? num shader stages ??
	createInfo.pStages = nullptr; // ?? shader stages ??
	createInfo.pVertexInputState = &vertexInfo;
	createInfo.pInputAssemblyState = &inputAssembly;
	createInfo.pTesselationState = nullptr; // ??
	createInfo.pViewportState = &viewportInfo;
	createInfo.pRasterizationState = &rasterizer;
	createInfo.pMultisampleState =        




	createInfo.dynamicStateCount = dynamicStateCount;
	createInfo.pDynamicStates = pDynamicStates;

	VkResult result = vkCreateGraphicsPipelines();


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
