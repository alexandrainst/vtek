#include "vtek_vulkan.pch"
#include "vtek_commands.hpp"

#include "imgutils/vtek_image_formats.hpp"
#include "impl/vtek_queue_struct.hpp"
#include "vtek_command_buffer.hpp"
#include "vtek_image.hpp"


void vtek::cmd_image_layout_transition(
	vtek::CommandBuffer* commandBuffer,
	const vtek::ImageLayoutTransitionCmdInfo* info)
{
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = nullptr;
	// It is possible to use `VK_IMAGE_LAYOUT_UNDEFINED` as the old layout
	// if we don't care about the existing contents of the image.
	barrier.oldLayout = vtek::get_image_layout(info->oldLayout);
	barrier.newLayout = vtek::get_image_layout(info->newLayout);
	barrier.image = vtek::image2d_get_handle(info->image);
	barrier.srcAccessMask = vtek::get_access_mask(info->srcAccessMask);
	barrier.dstAccessMask = vtek::get_access_mask(info->dstAccessMask);
	barrier.subresourceRange.aspectMask = 0;
	barrier.subresourceRange.baseMipLevel = 0; // TODO: Customize?
	barrier.subresourceRange.levelCount = 1; // TODO: mip-maps?
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	// Find the aspect mask
	VkFormat format = vtek::image2d_get_format(info->image);
	vtek::FormatDepthStencilTest dsTest =
		vtek::get_format_depth_stencil_test(format);
	switch (dsTest)
	{
	case vtek::FormatDepthStencilTest::depth:
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		break;
	case vtek::FormatDepthStencilTest::stencil:
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
		break;
	case vtek::FormatDepthStencilTest::depth_and_stencil:
		barrier.subresourceRange.aspectMask
			= VK_IMAGE_ASPECT_DEPTH_BIT
			| VK_IMAGE_ASPECT_STENCIL_BIT;
		break;
	default:
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		break;
	}

	// If we are using the barrier to transfer queue family ownership, then
	// these two fields should be indiced of the queue families. They must
	// be set to `VK_QUEUE_FAMILY_IGNORED` if we don't want to do this (not
	// the default value!).
	Queue* srcQueue = info->srcQueue;
	Queue* dstQueue = info->dstQueue;
	barrier.srcQueueFamilyIndex =
		(srcQueue != nullptr) ? srcQueue->familyIndex : VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex =
		(dstQueue != nullptr) ? dstQueue->familyIndex : VK_QUEUE_FAMILY_IGNORED;

	VkPipelineStageFlags srcStage = vtek::get_pipeline_stage(info->srcStage);
	VkPipelineStageFlags dstStage = vtek::get_pipeline_stage(info->dstStage);
	VkCommandBuffer cmdBuf = vtek::command_buffer_get_handle(commandBuffer);

	vkCmdPipelineBarrier(
		cmdBuf, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}
