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
	enum class AttachmentType
	{
		color, depth, depth_stencil
	};

	struct FramebufferAttachmentInfo
	{
		AttachmentType type {AttachmentType::color};
		SupportedFormat supportedFormat {};
	};

	struct FramebufferInfo
	{
		std::vector<FramebufferAttachmentInfo> attachments;
		glm::uvec2 resolution {1,1};
		vtek::MultisampleType multisampling {vtek::MultisampleType::none};
		// A list of queues which need access to the framebuffer attachments.
		std::vector<vtek::Queue*> sharingQueues;
	};

	Framebuffer* framebuffer_create(
		const FramebufferInfo* info, Device* device);
	void framebuffer_destroy(Framebuffer* framebuffer);

	bool framebuffer_dynrender_begin(Framebuffer* framebuffer);
	void framebuffer_dynrender_end(Framebuffer* framebuffer);
}
