#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "vtek_device.hpp"
#include "vtek_format_support.hpp"
#include "vtek_glm_includes.hpp"
#include "vtek_object_handles.hpp"
#include "vtek_vulkan_types.hpp"


namespace vtek
{
	struct FramebufferAttachmentInfo
	{
		SupportedFormat supportedFormat {};
		ClearValue clearValue {};
	};

	struct FramebufferInfo
	{
		std::vector<FramebufferAttachmentInfo> colorAttachments;
		FramebufferAttachmentInfo depthStencilAttachment {};
		bool useDepthStencil {false};
		glm::uvec2 resolution {1,1};
		vtek::MultisampleType multisampling {vtek::MultisampleType::none};

		// A list of queues which need access to the framebuffer attachments.
		// If empty, only the device's graphics queue is considered.
		std::vector<vtek::Queue*> sharingQueues;

		// TODO: A render pass may be added here
		RenderPass* renderPass {nullptr};

		// NOTE: With dynamic rendering, the framebuffer object itself is
		// not strictly needed - only its attachment images. But for a more
		// comprehensive API we construct a framebuffer object anyways.
		bool useDynamicRendering {false};
	};

	Framebuffer* framebuffer_create(
		const FramebufferInfo* info, Device* device);
	void framebuffer_destroy(Framebuffer* framebuffer, Device* device);

	bool framebuffer_dynamic_rendering_only(Framebuffer* framebuffer);

	bool framebuffer_dynrender_begin(
		Framebuffer* framebuffer, CommandBuffer* commandBuffer);
	void framebuffer_dynrender_end(
		Framebuffer* framebuffer, CommandBuffer* commandBuffer);
}
