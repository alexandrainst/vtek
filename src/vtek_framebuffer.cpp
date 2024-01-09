#include "vtek_vulkan.pch"
#include "vtek_framebuffer.hpp"

#include "impl/vtek_queue_struct.hpp"
#include "vtek_device.hpp"
#include "vtek_image.hpp"
#include "vtek_logging.hpp"
#include "vtek_queue.hpp"


/* attachment helper struct */
struct FramebufferAttachment
{
	// TODO: Can we have 1D or 3D or 4D framebuffer attachment?
	// TODO: Should we?
	vtek::Image2D* image {nullptr};
	glm::vec4 clearValue {0.0f};
};

/* struct implementation */
struct vtek::Framebuffer
{
	VkFramebuffer handle {VK_NULL_HANDLE};
	std::vector<FramebufferAttachment> colorAttachments {};
	glm::uvec2 resolution {1,1};
};


/* helper functions */
void color_memory_barrier_begin(
	VkCommandBuffer cmdBuf, FramebufferAttachment& attachment)
{
	// Transition from whatever (probably present src) to color attachment
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = nullptr;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.srcQueueFamilyIndex = ; // TODO: Graphics queue family index
	barrier.dstQueueFamilyIndex = ; // TODO: Graphics queue family index ?
	barrier.image = attachment.image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0; // TODO: Would a framebuffer ever be mipped?
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(
		cmdBuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr,
		0, nullptr, 1, &barrier);
}



/* interface */
vtek::Framebuffer* vtek::framebuffer_create(
	const vtek::FramebufferInfo* info, vtek::Device* device)
{
	auto dev = vtek::device_get_handle(device);

	// Input validation
	if (info->attachments.empty())
	{
		vtek_log_error("No attachments specified -- cannot create framebuffer!");
		return nullptr;
	}
	if (info->sharingQueues.empty())
	{
		vtek_log_error("No sharing queues provided -- cannot create framebuffer!");
		return nullptr;
	}

	// Image createInfo for the attachments
	vtek::Image2DInfo imageInfo{};
	imageInfo.requireDedicatedAllocation = true;
	imageInfo.extent =  { info->resolution.x, info->resolution.y };
	imageInfo.initialLayout = vtek::ImageInitialLayout::undefined; // default
	imageInfo.useMipmaps = false; // default

	// check multisampling support
	vtek::SampleCountQuery msaaQuery{};
	for (const auto& att : info->attachments)
	{
		if (att.type == vtek::AttachmentType::color) {
			msaaQuery.color = true;
		}
		else if (att.type == vtek::AttachmentType::depth) {
			msaaQuery.depth = true;
		}
		else if (att.type == vtek::AttachmentType::depth_stencil) {
			msaaQuery.stencil = true;
		}
	}
	auto supportedMsaa = vtek::device_get_max_sample_count(device, &msaaQuery);
	auto requestedMsaa = vtek::get_multisample_count(info->multisampling);
	if (requestedMsaa > supportedMsaa)
	{
		vtek_log_warn(
			"Requested multisample count for framebuffer creation {}",
			"exceeds device support! Clamping {} down to {}",
			requestedMsaa, supportedMsaa);
		requestedMsaa = supportedMsaa;
	}
	imageInfo.multisampling = vtek::get_multisample_enum(requestedMsaa);

	imageInfo.usageFlags = vtek::ImageUsageFlag::sampled; // TODO: Do we need more flags?
	imageInfo.sharingMode = vtek::ImageSharingMode::exclusive;
	// Check if any of the queues that need to access the framebuffer are from
	// different queues families.
	for (const vtek::Queue* queue : info->sharingQueues)
	{
		imageInfo.sharingQueues.push_back(queue);
		uint32_t index = queue->familyIndex;

		// check if index was already encountered
		for (const vtek::Queue* queue : imageInfo.sharingQueues)
		{
			if (queue->familyIndex != index)
			{
				imageInfo.sharingMode = vtek::ImageSharingMode::concurrent;
			}
		}
	}

	// TODO: Probably create an image for each attachment
	std::vector<vtek::Image2D*> images(info->attachments.size());
	for (const auto& att: info->attachments)
	{
		if (!att.supportedFormat.is_valid())
		{
			vtek_log_error("Framebuffer attachment format is not supported!");
			return nullptr;
		}

		imageInfo.supportedFormat = att.supportedFormat;
		switch (att.type)
		{
		case vtek::AttachmentType::color:
			imageInfo.usageFlags | vtek::ImageUsageFlag::color_attachment;
			break;
		case vtek::AttachmentType::depth:
		case vtek::AttachmentType::depth_stencil:
			imageInfo.usageFlags | vtek::ImageUsageFlag::depth_stencil_attachment;
			break;
		}

		vtek::Image2D* image = vtek::image2d_create(&imageInfo, device);
		if (image == nullptr)
		{
			vtek_log_error("Failed to create framebuffer attachment!");
			return nullptr;
		}
	}

	// Alloc
	auto framebuffer = new vtek::Framebuffer;

	VkFramebufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	createInfo.renderPass = VK_NULL_HANDLE; // ?
	createInfo.attachmentCount = info->attachments.size();
	createInfo.pAttachments = nullptr; // ?
	createInfo.width = info->resolution.x;
	createInfo.height = info->resolution.y;
	createInfo.layers = 1; // ?

	VkResult result = vkCreateFramebuffer(
		dev, &createInfo, nullptr, &framebuffer->handle);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to create framebuffer!");
		delete framebuffer;
		return nullptr;
	}

	// Fill in data
	framebuffer->resolution = info->resolution;

	return framebuffer;
}

void vtek::framebuffer_destroy(vtek::Framebuffer* framebuffer)
{

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
		color_memory_barrier_begin();

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
		glm::vec4 c = att.clearValue;
		attachInfo.clearValue.color.float32 = { c.x, c.y, c.z, c.w };

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
