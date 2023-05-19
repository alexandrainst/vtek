#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include "vtek_vulkan_handles.hpp"


namespace vtek
{
	struct RenderPassCreateInfo
	{
		// TODO: Is this right?
		std::vector<VkAttachmentDescription> attachments;

		// TODO: `glm` dependency for clear color/depth vectors?
		float clearR {0.0f};
		float clearG {0.0f};
		float clearB {0.0f};
		float clearA {1.0f};
	};

	struct RenderPass; // opaque handle


	RenderPass* render_pass_create(RenderPassCreateInfo* info, Device* device);
	void render_pass_destroy(RenderPass* renderPass, Device* device);

	VkRenderPass render_pass_get_handle(RenderPass* renderPass);
}
