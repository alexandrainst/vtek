#include "vtek_vulkan.pch"
#include "vtek_commands.hpp"


void cmd_dynamic_rendering_begin_barrier(CommandBuffer* commandBuffer)
{
	// Transition from whatever (probably present src) to color attachment
	VkImageMemoryBarrier beginBarrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = nullptr,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.srcQueueFamilyIndex = queueIndex,
		.dstQueueFamilyIndex = queueIndex,
		.image = vtek::swapchain_get_image(swapchain, i),
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	vkCmdPipelineBarrier(
		cmdBuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr,
		0, nullptr, 1, &beginBarrier);




}
