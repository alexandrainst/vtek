#include "vtek_graphics_pipeline.h"

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
	dynamicStateFlags flags = info->dynamicStateFlags;

	// Provided by VK_VERSION_1_0
	if (flags & PDState::viewport)
		states.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	if (flags & PDState::scissor)
		states.push_back(VK_DYNAMIC_STATE_SCISSOR);
	if (flags & PDState::line_width)
		states.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);
	if (flags & PDState::depth_bias)
		states.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
	if (flags & PDState::blend_constants)
		states.push_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
	if (flags & PDState::depth_bounds)
		states.push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
	if (flags & PDState::stencil_compare_mask)
		states.push_back(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
	if (flags & PDState::stencil_write_mask)
		states.push_back(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
	if (flags & PDState::stencil_reference)
		states.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);

	// TODO: Error handling for when the device doesn't support >= Vulkan 1.3!
	// Provided by VK_VERSION_1_3
#if defined(VK_API_VERSION_1_3)
	if (apiVersion->major() >= 1 && apiVersion->minor() >= 3)
	{
		if (flags & PDState::cull_mode)
			states.push_back(VK_DYNAMIC_STATE_CULL_MODE);
		if (flags & PDState::front_face)
			states.push_back(VK_DYNAMIC_STATE_FRONT_FACE);
		if (flags & PDState::primitive_topology)
			states.push_back(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY);
		if (flags & PDState::viewport_with_count)
			states.push_back(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
		if (flags & PDState::scissor_with_count)
			states.push_back(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
		if (flags & PDState::vertex_input_binding_stride)
			states.push_back(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
		if (flags & PDState::depth_test_enable)
			states.push_back(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE);
		if (flags & PDState::depth_write_enable)
			states.push_back(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE);
		if (flags & PDState::depth_compare_op)
			states.push_back(VK_DYNAMIC_STATE_DEPTH_COMPARE_OP);
		if (flags & PDState::depth_bounds_test_enable)
			states.push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE);
		if (flags & PDState::stencil_test_enable)
			states.push_back(VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE);
		if (flags & PDState::stencil_op)
			states.push_back(VK_DYNAMIC_STATE_STENCIL_OP);
		if (flags & PDState::rasterizer_discard_enable)
			states.push_back(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE);
		if (flags & PDState::depth_bias_enable)
			states.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE);
		if (flags & PDState::primitive_restart_enable)
			states.push_back(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE);
	}
#endif
}


/* interface */
vtek::GraphicsPipeline* vtek::graphics_pipeline_create(
	const vtek::GraphicsPipelineCreateInfo* info, vtek::Device* device)
{
	VkGraphicsPipelineCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.pNext = nullptr;

	/* Neat and ordered: */

	// shader stages

	// vertex input

	// input assembler
	PrimitiveTopology primitiveTopology {PrimitiveTopology::triangle_list};
	bool enablePrimitiveRestart {false};

	// viewport state
	if (info->viewportState == nullptr)
	{
		vtek_log_error("No viewport state provided - cannot create graphics pipeline!");
		return nullptr;
	}
	viewportState viewportState = *(info->viewportState); // copy-by-value
	VkViewport viewport{};
	viewport.x = viewportState.viewportRegion.offset.x;
	viewport.y = viewportState.viewportRegion.offset.y;
	viewport.width = viewportState.viewportRegion.extent.width;
	viewport.width = viewportState.viewportRegion.extent.width;

	// rasterization

	// multisample state

	// depth and stencil testing

	// color blending

	// dynamic states
	std::vector<VkDynamicState> dynamicStates;
	get_enabled_dynamic_states(info, device, dynamicStates);
	if (dynamicStates.size() > 0)
	{
		createInfo.dynamicStateCount = dynamicStates.size();
		createInfo.pDynamicStates = dynamicStates.data();
	}
	else
	{
		createInfo.dynamicStateCount = 0;
		createInfo.pDynamicStates = nullptr;
	}

	// pipeline layout

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
	VkResult result = vkCreateGraphicsPipelines


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
