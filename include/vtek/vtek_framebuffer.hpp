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
		glm::uvec2 resolution {1,1};
		MultisampleType multisampling {MultisampleType::none};

		// How the framebuffer handles depth/stencil, which per default
		// is none (no attachments created). If this value is anything other
		// than `none`, a valid format must be provided in the depth/stencil
		// attachment info.
		DepthStencilMode depthStencil {DepthStencilMode::none};

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

	// Create/destroy a single framebuffer
	Framebuffer* framebuffer_create(
		const FramebufferInfo* info, Device* device);

	// Create multiple framebuffers with the same settings. They will each have
	// their own attachments, but otherwise all framebuffers will be identical.
	// This is an ideal solution e.g. for creating gbuffers.
	std::vector<Framebuffer*> framebuffer_create(
		const FramebufferInfo* info, uint32_t count, Device* device);

	void framebuffer_destroy(Framebuffer* framebuffer, Device* device);

	void framebuffer_destroy(
		std::vector<Framebuffer*>& framebuffers, Device* device);

	// If a framebuffer supported only dynamic rendering, then into cannot be
	// used with the old Vulkan render pass setup during rendering.
	bool framebuffer_dynamic_rendering_only(Framebuffer* framebuffer);


	// Dynamic rendering
	bool framebuffer_dynrender_begin(
		Framebuffer* framebuffer, CommandBuffer* commandBuffer);
	void framebuffer_dynrender_end(
		Framebuffer* framebuffer, CommandBuffer* commandBuffer);


	// Properties
	std::vector<Format> framebuffer_get_color_formats(Framebuffer* framebuffer);
	Format framebuffer_get_depth_stencil_format(Framebuffer* framebuffer);
	glm::uvec2 framebuffer_get_resolution(Framebuffer* framebuffer);

	std::vector<Image2D*> framebuffer_get_color_images(Framebuffer* framebuffer);
}
