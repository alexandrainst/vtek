#include "vtek_graphics_pipeline.h"

/* struct implementation */
struct vtek::GraphicsPipeline
{
	VkPipeline vulkanHandle {VK_NULL_HANDLE};
	VkPipelineLayout layoutHandle {VK_NULL_HANDLE};
	vtek::RenderPassType renderPassType {vtek::RenderPassType::renderpass};
};


/* helper functions */
static void get_enabled_dynamic_states(std::vector<VkDynamicState>& states)
{
	using PDState = vtek::PipelineDynamicState;
	states.clear();

	// Provided by VK_VERSION_1_0
	if (states & PDState::viewport)
		states.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	if (states & PDState::scissor)
		states.push_back(VK_DYNAMIC_STATE_SCISSOR);
	if (states & PDState::line_width)
		states.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);
	if (states & PDState::depth_bias)
		states.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
	if (states & PDState::blend_constants)
		states.push_back(VK_DYNAMIC_STATE_BLEND_CONSTANTS);
	if (states & PDState::depth_bounds)
		states.push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS);
	if (states & PDState::stencil_compare_mask)
		states.push_back(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
	if (states & PDState::stencil_write_mask)
		states.push_back(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
	if (states & PDState::stencil_reference)
		states.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);

	// TODO: Error handling for when the device doesn't support >= Vulkan 1.3!
	// Provided by VK_VERSION_1_3
#if defined(VK_API_VERSION_1_3)
    if (states & PDState::cull_mode)
	    states.push_back(VK_DYNAMIC_STATE_CULL_MODE);
    if (states & PDState::front_face)
	    states.push_back(VK_DYNAMIC_STATE_FRONT_FACE);
    if (states & PDState::primitive_topology)
	    states.push_back(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY);
    if (states & PDState::viewport_with_count)
	    states.push_back(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
    if (states & PDState::scissor_with_count)
	    states.push_back(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
    if (states & PDState::vertex_input_binding_stride)
	    states.push_back(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
    if (states & PDState::depth_test_enable)
	    states.push_back(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE);
    if (states & PDState::depth_write_enable)
	    states.push_back(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE);
    if (states & PDState::depth_compare_op)
	    states.push_back(VK_DYNAMIC_STATE_DEPTH_COMPARE_OP);
    if (states & PDState::depth_bounds_test_enable)
	    states.push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE);
    if (states & PDState::stencil_test_enable)
	    states.push_back(VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE);
    if (states & PDState::stencil_op)
	    states.push_back(VK_DYNAMIC_STATE_STENCIL_OP);
    if (states & PDState::rasterizer_discard_enable)
	    states.push_back(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE);
    if (states & PDState::depth_bias_enable)
	    states.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE);
    if (states & PDState::primitive_restart_enable)
	    states.push_back(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE);
#endif
}


/* interface */
vtek::GraphicsPipeline* vtek::graphics_pipeline_create(
	const vtek::GraphicsPipelineCreateInfo& info, vtek::Device* device)
{
	VkGraphicsPipelineCreateInfo createInfo{};

	// Dynamic state
	VkPipelineRenderingCreateInfo renderingCreateInfo{};
	renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO; // TODO: guess!
	renderingCreateInfo.pNext = nullptr;
	renderingCreateInfo.viewMask = 0; // TODO: uint32_t ?
	renderingCreateInfo.colorAttachmentCount = 0; // TODO: uint32_t ?
	renderingCreateInfo.pColorAttachmentFormats = nullptr; // TODO: VkFormat* ?
	renderingCreateInfo.depthAttachmentFormat = VK_INVALID_FORMAT; // TODO: VkFormat ?
	renderingCreateInfo.stencilAttachmentFormat = VK_INVALID_FORMAT; // TODO: VkFormat ?
	createInfo.pNext = &renderingCreateInfo;

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
