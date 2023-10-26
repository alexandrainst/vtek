#pragma once

#include <vulkan/vulkan.h>
#include <vector>

#include "vtek_device.hpp"
#include "vtek_glm_includes.hpp"
#include "vtek_object_handles.hpp"


namespace vtek
{
	enum class AttachmentType
	{
		color, depth, depth_stencil
	};

	struct FramebufferAttachment
	{
		AttachmentType type {AttachmentType::color};
	};

	struct FramebufferInfo
	{
		std::vector<FramebufferAttachment> attachments;
		glm::uvec2 resolution {1,1};
	};

	Framebuffer* framebuffer_create(
		const FramebufferInfo* info, Device* device);
	void framebuffer_destroy(Framebuffer* framebuffer);
}
