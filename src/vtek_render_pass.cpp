#include "vtek_vulkan.pch"
#include "vtek_render_pass.hpp"

#include "vtek_logging.hpp"

// TODO: This section is unfinished, because dynamic rendering was prioritized.


/* struct definition */
struct vtek::RenderPass
{
	VkRenderPass vulkanHandle {VK_NULL_HANDLE};
};



/* interface */
vtek::RenderPass* vtek::render_pass_create(
	vtek::RenderPassCreateInfo* info, vtek::Device* device)
{
	vtek_log_error("Not implemented: vtek::render_pass_create()");
	return nullptr;
}

void vtek::render_pass_destroy(vtek::RenderPass* renderPass, vtek::Device* device)
{
	vtek_log_error("Not implemented: vtek::render_pass_destroy()");
}

VkRenderPass vtek::render_pass_get_handle(vtek::RenderPass* renderPass)
{
	vtek_log_error("Not implemented: vtek::render_pass_get_handle()");
	return VK_NULL_HANDLE;
	//return renderPass->vulkanHandle;
}
