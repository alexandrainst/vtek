#include "vtek_vulkan.pch"
#include "vtek_framebuffer.hpp"

#include "impl/vtek_queue_struct.hpp"
#include "vtek_device.hpp"
#include "vtek_image.hpp"
#include "vtek_logging.hpp"
#include "vtek_queue.hpp"


/* struct implementation */
struct vtek::Framebuffer
{
	VkFramebuffer handle {VK_NULL_HANDLE};
};



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
	imageInfo.multisampling = info->multisampling; // TODO: Does device support it?
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

	return framebuffer;
}

void vtek::framebuffer_destroy(vtek::Framebuffer* framebuffer)
{

}
