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
	vtek::Image2D* image {nullptr};
	vtek::ClearValue clearValue {};
	vtek::SupportedFormat supportedFormat {};
};

/* struct implementation */
struct vtek::Framebuffer
{
	VkFramebuffer handle {VK_NULL_HANDLE};
	std::vector<FramebufferAttachment> colorAttachments {};
	FramebufferAttachment depthStencilAttachment {};
	bool useDepth {false};
	bool useStencil {false};
	glm::uvec2 resolution {1,1};
	uint32_t graphicsQueueFamilyIndex {0};
	bool dynamicRenderingOnly {false};
	// TODO: Attachments can have had explicit queue ownership transfers!
};



/* helper functions */
static bool validate_framebuffer_info(
	const vtek::FramebufferInfo* info, bool useDepth, bool useStencil)
{
	auto dsfmt = info->depthStencilAttachment.supportedFormat;

	if (info->colorAttachments.empty() && !(useDepth || useStencil))
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
			vtek_log_error("Unsupported color attachment format -- {}",
			               "cannot create framebuffer!");
			return false;
		}
	}
	if ((info->depthStencil != vtek::DepthStencilMode::none) &&
	    !(dsfmt.is_valid()))
	{
		vtek_log_error("Unsupported depth/stencil attachment format -- {}",
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

	if (useDepth && !dsfmt.has_depth())
	{
		vtek_log_error(
			"Depth buffer specified, {} -- {}",
			"but depth/stencil format doesn't support depth",
			"cannot create framebuffer!");
		return false;
	}
	if (useStencil && !dsfmt.has_stencil())
	{
		vtek_log_error(
			"Stencil buffer specified, {} -- {}",
			"but depth/stencil format doesn't support stencil",
			"cannot create framebuffer!");
		return false;
	}

	return true;
}

static bool check_multisample_support(
	const vtek::FramebufferInfo* info, vtek::Device* device,
	bool useDepth, bool useStencil, vtek::MultisampleType& targetMsaa)
{
	vtek::SampleCountQuery msaaQuery{};
	msaaQuery.color = !info->colorAttachments.empty();
	if (info->depthStencil != vtek::DepthStencilMode::none)
	{
		const auto& fmt = info->depthStencilAttachment.supportedFormat;
		if (!fmt.is_depth_stencil())
		{
			vtek_log_error("Invalid depth/stencil attachment format -- {}",
			               "cannot create framebuffer!");
			return false;
		}
		msaaQuery.depth = fmt.has_depth() && useDepth;
		msaaQuery.stencil = fmt.has_stencil() && useStencil;
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

	targetMsaa = vtek::get_multisample_enum(requestedMsaa);
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
	barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
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
		cmdBuf, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
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
	barrier.dstAccessMask
		= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	// TODO: Attachments can have had explicit queue ownership transfers!
	barrier.srcQueueFamilyIndex = srcQueueFamilyIndex;
	barrier.dstQueueFamilyIndex = dstQueueFamilyIndex;
	barrier.image = vtek::image2d_get_handle(attachment.image);
	barrier.subresourceRange.aspectMask = 0;
	if (attachment.supportedFormat.has_depth()) {
		barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	if (attachment.supportedFormat.has_stencil()) {
		barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	const VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	const VkPipelineStageFlags dstStageMask
		= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

	vkCmdPipelineBarrier(
		cmdBuf, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}



/* interface */
vtek::Framebuffer* vtek::framebuffer_create(
	const vtek::FramebufferInfo* info, vtek::Device* device)
{
	// Depth/stencil check
	const bool useDepth =
		(info->depthStencil == vtek::DepthStencilMode::depth) ||
		(info->depthStencil == vtek::DepthStencilMode::depth_and_stencil);
	const bool useStencil =
		(info->depthStencil == vtek::DepthStencilMode::stencil) ||
		(info->depthStencil == vtek::DepthStencilMode::depth_and_stencil);

	// Input validation
	if (!validate_framebuffer_info(info, useDepth, useStencil)) { return nullptr; }

	// Image createInfo for the attachments
	vtek::Image2DInfo imageInfo{};
	imageInfo.requireDedicatedAllocation = true;
	imageInfo.extent =  { info->resolution.x, info->resolution.y };
	imageInfo.initialLayout = vtek::ImageInitialLayout::undefined; // default
	imageInfo.useMipmaps = false; // default

	// check multisampling support
	MultisampleType targetMsaa = vtek::MultisampleType::none;
	if (!check_multisample_support(
		    info, device, useDepth, useStencil, targetMsaa))
	{
		return nullptr;
	}
	imageInfo.multisampling = targetMsaa;

	// Usage flag for all attachments, also depth/stencil buffers
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

		framebuffer->colorAttachments.push_back({
				image, att.clearValue, att.supportedFormat });
		if (!att.supportedFormat.is_valid()) {
			vtek_log_error("Invalid color format!");
		}
	}

	// Create depth/stencil attachment
	if (useDepth || useStencil)
	{
		const FramebufferAttachmentInfo& attInfo = info->depthStencilAttachment;
		imageInfo.supportedFormat = attInfo.supportedFormat;
		imageInfo.usageFlags =
			sharedUsageFlags | vtek::ImageUsageFlag::depth_stencil_attachment;
		imageInfo.imageViewInfo.aspectFlags = 0U;

		if (useDepth) {
			imageInfo.imageViewInfo.aspectFlags |= vtek::ImageAspectFlag::depth;
			framebuffer->useDepth = true;
		}
		if (useStencil) {
			imageInfo.imageViewInfo.aspectFlags |= vtek::ImageAspectFlag::stencil;
			framebuffer->useStencil = true;
		}

		vtek::Image2D* image = vtek::image2d_create(&imageInfo, device);
		if (image == nullptr)
		{
			vtek_log_error(
				"Failed to create framebuffer depth/stencil attachment!");
			vtek::framebuffer_destroy(framebuffer, device);
			return nullptr;
		}

		const auto& fmt = info->depthStencilAttachment.supportedFormat;
		framebuffer->depthStencilAttachment = { image, attInfo.clearValue, fmt };
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

std::vector<vtek::Framebuffer*> vtek::framebuffer_create(
	const vtek::FramebufferInfo* info, uint32_t count, vtek::Device* device)
{
	// TODO: This can be optimized by performing internal checks only once!
	std::vector<vtek::Framebuffer*> framebuffers;
	for (uint32_t i = 0; i < count; i++)
	{
		auto fb = vtek::framebuffer_create(info, device);
		if (fb == nullptr) { return {}; } // return empty vector!

		framebuffers.push_back(fb);
	}

	return framebuffers; // RVO
}

void vtek::framebuffer_destroy(
	vtek::Framebuffer* framebuffer, vtek::Device* device)
{
	if (device == nullptr || framebuffer == nullptr) { return; }

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

void vtek::framebuffer_destroy(
	std::vector<vtek::Framebuffer*>& framebuffers, vtek::Device* device)
{
	if (device == nullptr) { return; }

	for (auto fb : framebuffers)
	{
		vtek::framebuffer_destroy(fb, device);
	}

	framebuffers.clear();
}

bool vtek::framebuffer_dynamic_rendering_only(vtek::Framebuffer* framebuffer)
{
	return framebuffer->dynamicRenderingOnly;
}

bool vtek::framebuffer_dynamic_rendering_begin(
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

	// Create attachment info structs for each color attachment
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
		attachInfo.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
		// TODO: Multisampled framebuffer
		attachInfo.resolveMode = VK_RESOLVE_MODE_NONE;
		attachInfo.resolveImageView = VK_NULL_HANDLE;
		attachInfo.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		attachInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachInfo.clearValue = att.clearValue.get();

		colorAttachmentInfos.push_back(std::move(attachInfo));
	}

	// Dynamic rendering struct
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
	renderingInfo.pDepthAttachment = nullptr;
	renderingInfo.pStencilAttachment = nullptr;

	// Create attachment info for depth/stencil attachment if such exists
	VkRenderingAttachmentInfo depthStencilAttachmentInfo{};
	const bool useDepth = framebuffer->useDepth;
	const bool useStencil = framebuffer->useStencil;

	if (useDepth || useStencil)
	{
		auto& att = framebuffer->depthStencilAttachment;

		// TODO: Attachments can have had explicit queue ownership transfers!
		uint32_t queueFamilyIndex = framebuffer->graphicsQueueFamilyIndex;
		depth_stencil_memory_barrier_begin(
			cmdBuf, att, queueFamilyIndex, queueFamilyIndex);

		VkRenderingAttachmentInfo& info = depthStencilAttachmentInfo;

		info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		info.pNext = nullptr;
		info.imageView = vtek::image2d_get_view_handle(att.image);
		info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		// TODO: Multisampled framebuffer
		info.resolveMode = VK_RESOLVE_MODE_NONE;
		info.resolveImageView = VK_NULL_HANDLE;
		info.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		info.clearValue = att.clearValue.get();

		if (useDepth) {
			renderingInfo.pDepthAttachment = &depthStencilAttachmentInfo;
		}
		if (useStencil) {
			renderingInfo.pStencilAttachment = &depthStencilAttachmentInfo;
		}
	}

	// Begin dynamic rendering
	vkCmdBeginRendering(cmdBuf, &renderingInfo);
	return true;
}

void vtek::framebuffer_dynamic_rendering_end(
	vtek::Framebuffer* framebuffer, vtek::CommandBuffer* commandBuffer)
{
	if (!vtek::command_buffer_is_recording(commandBuffer))
	{
		vtek_log_error(
			"Command buffer is not recording -- cannot end dynamic rendering!");
		return;
	}

	auto cmdBuf = vtek::command_buffer_get_handle(commandBuffer);

	vkCmdEndRendering(cmdBuf);

	// "applications must ensure that all memory writes have been made available
	// before a layout transition is executed."
	// NOTE: This probably means we need a fence!?

	std::vector<VkRenderingAttachmentInfo> colorAttachmentInfos{};

	// Transition attachments into read
	for (auto& att : framebuffer->colorAttachments)
	{
		// Image memory barrier for each color attachment
		// TODO: Attachments can have had explicit queue ownership transfers!
		uint32_t queueFamilyIndex = framebuffer->graphicsQueueFamilyIndex;
		color_memory_barrier_end(cmdBuf, att, queueFamilyIndex, queueFamilyIndex);
	}
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

glm::uvec2 vtek::framebuffer_get_resolution(vtek::Framebuffer* framebuffer)
{
	return framebuffer->resolution;
}

std::vector<vtek::Image2D*> vtek::framebuffer_get_color_images(
	vtek::Framebuffer* framebuffer)
{
	std::vector<vtek::Image2D*> images;
	for (auto& att : framebuffer->colorAttachments)
	{
		images.push_back(att.image);
	}

	return images; // RVO
}

void vtek::framebuffer_set_clear_color(
	vtek::Framebuffer* framebuffer, uint32_t attachmentIndex,
	vtek::ClearValue clearValue)
{
	if (attachmentIndex >= framebuffer->colorAttachments.size())
	{
		vtek_log_error(
			"Cannot set clear value on framebuffer attachment - {}",
			"index is out of bounds!");
		return;
	}

	framebuffer->colorAttachments[attachmentIndex].clearValue = clearValue;
}
