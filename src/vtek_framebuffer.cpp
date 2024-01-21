#include "vtek_vulkan.pch"
#include "vtek_framebuffer.hpp"

#include "impl/vtek_queue_struct.hpp"
#include "vtek_command_buffer.hpp"
#include "vtek_device.hpp"
#include "vtek_image.hpp"
#include "vtek_logging.hpp"
#include "vtek_queue.hpp"
#include "vtek_render_pass.hpp"


/* attachment helper struct */
struct FramebufferAttachment
{
	// TODO: Can we have 1D or 3D or 4D framebuffer attachment?
	// TODO: Should we?
	vtek::Image2D* image {nullptr};
	vtek::ClearValue clearValue {};
};

/* struct implementation */
struct vtek::Framebuffer
{
	VkFramebuffer handle {VK_NULL_HANDLE};
	std::vector<FramebufferAttachment> colorAttachments {};
	FramebufferAttachment depthStencilAttachment {};
	glm::uvec2 resolution {1,1};
	uint32_t graphicsQueueFamilyIndex {0};
	bool dynamicRenderingOnly {false};
	// TODO: Attachments can have had explicit queue ownership transfers!
};



/* helper functions */
static bool validate_framebuffer_info(const vtek::FramebufferInfo* info)
{
	if (info->colorAttachments.empty() && !info->useDepthStencil)
	{
		vtek_log_error(
			"No attachments specified -- cannot create empty framebuffer!");
		return false;
	}
	if (info->resolution.x == 0U || info->resolution.y == 0U)
	{
		vtek_log_error("Invalid resolution ({},{}) -- {}",
		               info->resolution.x, info->resolution.y,
		               "cannot create framebuffer!");
		return false;
	}
	for (const auto& att : info->colorAttachments)
	{
		if (!att.supportedFormat.is_valid())
		{
			vtek_log_error("Unsupported color attachment format -- {}!",
			               "cannot create framebuffer!");
			return false;
		}
	}
	if (info->useDepthStencil &&
	    !info->depthStencilAttachment.supportedFormat.is_valid())
	{
		vtek_log_error("Unsupported depth/stencil attachment format -- {}!",
		               "cannot create framebuffer!");
		return false;
	}
	if (info->renderPass == nullptr && !info->useDynamicRendering)
	{
		vtek_log_error(
			"{} {}",
			"No render pass specified, and dynamic rendering not indicated.",
			"At least one of those must be present for framebuffer creation!");
		return false;
	}

	return true;
}

static bool create_vulkan_framebuffer_handle(
	vtek::Framebuffer* framebuffer, const vtek::FramebufferInfo* info,
	vtek::Device* device)
{
	std::vector<VkImageView> attachments;
	for (auto& att : framebuffer->colorAttachments)
	{
		attachments.push_back(vtek::image2d_get_view_handle(att.image));
	}
	if (auto img = framebuffer->depthStencilAttachment.image; img != nullptr)
	{
		attachments.push_back(vtek::image2d_get_view_handle(img));
	}

	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = vtek::render_pass_get_handle(info->renderPass);
	createInfo.attachmentCount = attachments.size();
	createInfo.pAttachments = attachments.data();
	createInfo.width = info->resolution.x;
	createInfo.height = info->resolution.y;
	// NOTE: layers is used for multiview render passes which is not supported!
	// NOTE: If render pass has non-zero view mask, layers must be 1!
	createInfo.layers = 1;

	auto dev = vtek::device_get_handle(device);
	VkResult result = vkCreateFramebuffer(
		dev, &createInfo, nullptr, &framebuffer->handle);

	return result == VK_SUCCESS;
}

static void color_memory_barrier_begin(
	VkCommandBuffer cmdBuf, FramebufferAttachment& attachment,
	uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex)
{
	// Transition from whatever (probably present src) to color attachment
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = nullptr;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	// TODO: Attachments can have had explicit queue ownership transfers!
	barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
	barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
	barrier.image = vtek::image2d_get_handle(attachment.image);
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(
		cmdBuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr,
		0, nullptr, 1, &barrier);
}

static void color_memory_barrier_end(
	VkCommandBuffer cmdBuf, FramebufferAttachment& attachment,
	uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex)
{
	// Transition from color attachment to read something
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = nullptr;
	barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	barrier.dstAccessMask
		//= VK_ACCESS_UNIFORM_READ_BIT
		//| VK_ACCESS_SHADER_READ_BIT
		= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT; // NOTE: Probably the best!?
	barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	// TODO: Attachments can have had explicit queue ownership transfers!
	barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
	barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
	barrier.image = vtek::image2d_get_handle(attachment.image);
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(
		cmdBuf, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, // TODO: BOTTOM ?
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr,
		0, nullptr, 1, &barrier);
}

static void depth_stencil_memory_barrier_begin(
	VkCommandBuffer cmdBuf, FramebufferAttachment& attachment,
	uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = nullptr;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	// TODO: Attachments can have had explicit queue ownership transfers!
	barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
	barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
}

static void depth_stencil_memory_barrier_end(
	VkCommandBuffer cmdBuf, FramebufferAttachment& attachment,
	uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = nullptr;
	barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	// TODO: Attachments can have had explicit queue ownership transfers!
	barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
	barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
}



/* interface */
vtek::Framebuffer* vtek::framebuffer_create(
	const vtek::FramebufferInfo* info, vtek::Device* device)
{
	// Input validation
	if (!validate_framebuffer_info(info)) { return nullptr; }

	// Image createInfo for the attachments
	vtek::Image2DInfo imageInfo{};
	imageInfo.requireDedicatedAllocation = true;
	imageInfo.extent =  { info->resolution.x, info->resolution.y };
	imageInfo.initialLayout = vtek::ImageInitialLayout::undefined; // default
	imageInfo.useMipmaps = false; // default

	// check multisampling support
	vtek::SampleCountQuery msaaQuery{};
	msaaQuery.color = !info->colorAttachments.empty();
	if (info->useDepthStencil)
	{
		const auto& fmt = info->depthStencilAttachment.supportedFormat;
		if (!fmt.is_depth_stencil())
		{
			vtek_log_error("Invalid depth/stencil attachment format -- {}",
			               "cannot create framebuffer!");
			return nullptr;
		}
		msaaQuery.depth = fmt.has_depth();
		msaaQuery.stencil = fmt.has_stencil();
	}
	auto supportedMsaa = vtek::device_get_max_sample_count(device, &msaaQuery);
	auto requestedMsaa = vtek::get_multisample_count(info->multisampling);
	if (requestedMsaa > supportedMsaa)
	{
		vtek_log_warn(
			"{} Device supports {} but {} requested. Number will be clamped!",
			"Exceeded multisample count for framebuffer creation.",
			static_cast<uint32_t>(requestedMsaa),
			static_cast<uint32_t>(supportedMsaa));
		requestedMsaa = supportedMsaa;
	}
	imageInfo.multisampling = vtek::get_multisample_enum(requestedMsaa);

	// TODO: Do we need more usage flags?
	vtek::ImageUsageFlag sharedUsageFlags = vtek::ImageUsageFlag::sampled;

	// Check if any of the queues that need to access the framebuffer are from
	// different queues families.
	imageInfo.sharingMode = vtek::ImageSharingMode::exclusive;
	if (info->sharingQueues.empty())
	{
		vtek::Queue* graphicsQueue = vtek::device_get_graphics_queue(device);
		if (graphicsQueue == nullptr)
		{
			vtek_log_error(
				"Framebuffer creation assumes device's graphics queue, {}{}",
				"but device has no graphics queue",
				" -- cannot create framebuffer!");
			return nullptr;
		}
		imageInfo.sharingQueues.push_back(graphicsQueue);
	}
	else
	{
		for (const vtek::Queue* queue : info->sharingQueues)
		{
			imageInfo.sharingQueues.push_back(queue);
			uint32_t index = queue->familyIndex;

			// check if index was already encountered
			for (const vtek::Queue* q : imageInfo.sharingQueues)
			{
				if (q->familyIndex != index)
				{
					imageInfo.sharingMode = vtek::ImageSharingMode::concurrent;
				}
			}
		}
	}

	// Alloc
	auto framebuffer = new vtek::Framebuffer;

	// Create color attachments
	for (const auto& att : info->colorAttachments)
	{
		imageInfo.supportedFormat = att.supportedFormat;
		imageInfo.usageFlags =
			sharedUsageFlags | vtek::ImageUsageFlag::color_attachment;
		imageInfo.imageViewInfo.aspectFlags = vtek::ImageAspectFlag::color;

		vtek::Image2D* image = vtek::image2d_create(&imageInfo, device);
		if (image == nullptr)
		{
			vtek_log_error("Failed to create framebuffer color attachment!");
			vtek::framebuffer_destroy(framebuffer, device);
			return nullptr;
		}

		framebuffer->colorAttachments.push_back({ image, att.clearValue });
	}

	// Create depth/stencil attachment
	if (info->useDepthStencil)
	{
		const FramebufferAttachmentInfo& attInfo = info->depthStencilAttachment;
		imageInfo.supportedFormat = attInfo.supportedFormat;
		imageInfo.usageFlags =
			sharedUsageFlags | vtek::ImageUsageFlag::depth_stencil_attachment;
		imageInfo.imageViewInfo.aspectFlags = 0U;

		const auto& fmt = info->depthStencilAttachment.supportedFormat;
		if (fmt.has_depth()) {
			imageInfo.imageViewInfo.aspectFlags |= vtek::ImageAspectFlag::depth;
		}
		if (fmt.has_stencil()) {
			imageInfo.imageViewInfo.aspectFlags |= vtek::ImageAspectFlag::stencil;
		}

		vtek::Image2D* image = vtek::image2d_create(&imageInfo, device);
		if (image == nullptr)
		{
			vtek_log_error(
				"Failed to create framebuffer depth/stencil attachment!");
			vtek::framebuffer_destroy(framebuffer, device);
			return nullptr;
		}

		framebuffer->depthStencilAttachment = { image, attInfo.clearValue };
	}

	// Fill in data
	framebuffer->resolution = info->resolution;
	vtek::Queue* graphicsQueue = vtek::device_get_graphics_queue(device);
	framebuffer->graphicsQueueFamilyIndex = graphicsQueue->familyIndex;

	//
	// DONE: For dynamic rendering only, no vkFramebuffer can be created!
	//
	// Without a render pass, we cannot create a vulkan framebuffer object.
	// But we can still return a shell to host the image attachments.
	if (info->renderPass == nullptr && info->useDynamicRendering)
	{
		framebuffer->dynamicRenderingOnly = true;
		return framebuffer;
	}

	//
	// NEXT: With render pass object provided, create vkFramebuffer object!
	//
	if (!create_vulkan_framebuffer_handle(framebuffer, info, device))
	{
		vtek_log_error("Failed to create framebuffer!");
		vtek::framebuffer_destroy(framebuffer, device);
		delete framebuffer;
		return nullptr;
	}

	return framebuffer;
}

void vtek::framebuffer_destroy(
	vtek::Framebuffer* framebuffer, vtek::Device* device)
{
	if (framebuffer == nullptr) { return; }

	for (auto& att : framebuffer->colorAttachments)
	{
		vtek::image2d_destroy(att.image, device);
	}
	framebuffer->colorAttachments.clear();

	if (auto img = framebuffer->depthStencilAttachment.image; img != nullptr)
	{
		vtek::image2d_destroy(img, device);
		framebuffer->depthStencilAttachment = {};
	}

	if (framebuffer->handle != VK_NULL_HANDLE)
	{
		auto dev = vtek::device_get_handle(device);

		vkDestroyFramebuffer(dev, framebuffer->handle, nullptr);
		framebuffer->handle = VK_NULL_HANDLE;
	}
}

bool vtek::framebuffer_dynamic_rendering_only(vtek::Framebuffer* framebuffer)
{
	return framebuffer->dynamicRenderingOnly;
}

bool vtek::framebuffer_dynrender_begin(
	vtek::Framebuffer* framebuffer, vtek::CommandBuffer* commandBuffer)
{
	if (!vtek::command_buffer_is_recording(commandBuffer))
	{
		vtek_log_error(
			"Command buffer is not recording -- cannot begin dynamic rendering!");
		return false;
	}

	auto cmdBuf = vtek::command_buffer_get_handle(commandBuffer);

	// TODO: Transition for each color attachment
	std::vector<VkRenderingAttachmentInfo> colorAttachmentInfos{};

	for (auto& att : framebuffer->colorAttachments)
	{
		// Image memory barrier for each color attachment
		// TODO: Attachments can have had explicit queue ownership transfers!
		uint32_t queueFamilyIndex = framebuffer->graphicsQueueFamilyIndex;
		color_memory_barrier_begin(
			cmdBuf, att, queueFamilyIndex, queueFamilyIndex);

		// Color attachment info for dynamic rendering
		VkRenderingAttachmentInfo attachInfo{};
		attachInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		attachInfo.pNext = nullptr;
		attachInfo.imageView = vtek::image2d_get_view_handle(att.image);
		attachInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL; // TODO: Always?
		attachInfo.resolveMode = VK_RESOLVE_MODE_NONE; // TODO: Multisampled framebuffer!
		attachInfo.resolveImageView = VK_NULL_HANDLE; // TODO: Multisampled framebuffer!
		attachInfo.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED; // TODO: Multisampling!
		attachInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachInfo.clearValue = att.clearValue.get();

		colorAttachmentInfos.emplace_back(std::move(attachInfo)); // TODO: std::forward?
	}

	// Begin dynamic rendering
	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.pNext = nullptr;
	renderingInfo.flags = 0;
	renderingInfo.renderArea =
		{ 0U, 0U, framebuffer->resolution.x, framebuffer->resolution.y };
	renderingInfo.layerCount = 1;
	renderingInfo.viewMask = 0;
	renderingInfo.colorAttachmentCount = colorAttachmentInfos.size();
	renderingInfo.pColorAttachments = colorAttachmentInfos.data();
	// TODO: Check depth/stencil attachments!
	renderingInfo.pDepthAttachment = nullptr;
	renderingInfo.pStencilAttachment = nullptr;

	vkCmdBeginRendering(cmdBuf, &renderingInfo);
	return true;
}

void vtek::framebuffer_dynrender_end(
	vtek::Framebuffer* framebuffer, vtek::CommandBuffer* commandBuffer)
{
	if (!vtek::command_buffer_is_recording(commandBuffer))
	{
		vtek_log_error(
			"Command buffer is not recording -- cannot end dynamic rendering!");
		return;
	}

	auto cmdBuf = vtek::command_buffer_get_handle(commandBuffer);

	// "applications must ensure that all memory writes have been made available
	// before a layout transition is executed."
	// NOTE: This probably means we need a fence!?


	// TODO: Transition for each color attachment
	std::vector<VkRenderingAttachmentInfo> colorAttachmentInfos{};

	// Transition attachments into read
	for (auto& att : framebuffer->colorAttachments)
	{
		// Image memory barrier for each color attachment
		// TODO: Attachments can have had explicit queue ownership transfers!
		uint32_t queueFamilyIndex = framebuffer->graphicsQueueFamilyIndex;
		color_memory_barrier_end(cmdBuf, att, queueFamilyIndex, queueFamilyIndex);
	}

	vkCmdEndRendering(cmdBuf);
}

std::vector<vtek::Format> vtek::framebuffer_get_color_formats(
	vtek::Framebuffer* framebuffer)
{
	std::vector<vtek::Format> formats;
	for (auto& att : framebuffer->colorAttachments)
	{
		formats.push_back(vtek::image2d_get_format(att.image));
	}

	return formats; // RVO
}

vtek::Format vtek::framebuffer_get_depth_stencil_format(
	vtek::Framebuffer* framebuffer)
{
	vtek::Image2D* img = framebuffer->depthStencilAttachment.image;
	if (img == nullptr)
	{
		return vtek::Format::undefined;
	}

	return vtek::image2d_get_format(img);
}
