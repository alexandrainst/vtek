#include "vtek_vulkan.pch"
#include "vtek_commands.hpp"

#include "impl/vtek_queue_struct.hpp"
#include "vtek_buffer.hpp"
#include "vtek_command_buffer.hpp"
#include "vtek_descriptor_set.hpp"
#include "vtek_format_support.hpp"
#include "vtek_graphics_pipeline.hpp"
#include "vtek_image.hpp"
#include "vtek_push_constants.hpp"


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
	Format format = vtek::image2d_get_format(info->image);
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

void vtek::cmd_bind_graphics_pipeline(
	vtek::CommandBuffer* commandBuffer, vtek::GraphicsPipeline* pipeline)
{
	auto cmdBuf = vtek::command_buffer_get_handle(commandBuffer);
	auto pipl = vtek::graphics_pipeline_get_handle(pipeline);
	vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipl);
}

void vtek::cmd_set_viewport_scissor(
	vtek::CommandBuffer* commandBuffer, glm::uvec2 size)
{
	auto cmdBuf = vtek::command_buffer_get_handle(commandBuffer);

	VkViewport viewport{};
	viewport.x = 0.0f; viewport.y = 0.0f; // upper-left corner
	viewport.width = static_cast<float>(size.x);
	viewport.height = static_cast<float>(size.y);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = { size.x, size.y };

	vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
	vkCmdSetScissor(cmdBuf, 0, 1, &scissor);
}

void vtek::cmd_set_viewport_scissor(
	vtek::CommandBuffer* commandBuffer, glm::vec2 upperLeftCorner,
	glm::uvec2 size, glm::vec2 depthRange)
{
	auto cmdBuf = vtek::command_buffer_get_handle(commandBuffer);

	VkViewport viewport{};
	viewport.x = upperLeftCorner.x;
	viewport.y = upperLeftCorner.y;
	viewport.width = static_cast<float>(size.x);
	viewport.height = static_cast<float>(size.y);
	viewport.minDepth = depthRange.x;
	viewport.maxDepth = depthRange.y;

	VkRect2D scissor{};
	scissor.offset = {
		static_cast<int32_t>(upperLeftCorner.x),
		static_cast<int32_t>(upperLeftCorner.y)};
	scissor.extent = { size.x, size.y };

	vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
	vkCmdSetScissor(cmdBuf, 0, 1, &scissor);
}

void vtek::cmd_push_constant_graphics(
	vtek::CommandBuffer* commandBuffer, vtek::GraphicsPipeline* pipeline,
	vtek::IPushConstant* pushConstant,
	vtek::EnumBitmask<vtek::ShaderStageGraphics> stages)
{
	auto cmdBuf = vtek::command_buffer_get_handle(commandBuffer);
	auto pipLayout = vtek::graphics_pipeline_get_layout(pipeline);
	auto size = pushConstant->size();
	auto data = pushConstant->data();

	VkShaderStageFlags stageFlags =
		vtek::get_shader_stage_flags_graphics(stages);

	vkCmdPushConstants(cmdBuf, pipLayout, stageFlags, 0, size, data);
}

void vtek::cmd_bind_vertex_buffer(
	vtek::CommandBuffer* commandBuffer, vtek::Buffer* buffer, uint64_t offset)
{
	auto cmdBuf = vtek::command_buffer_get_handle(commandBuffer);
	VkBuffer buffers[1] = { vtek::buffer_get_handle(buffer) };
	VkDeviceSize offsets[1] = { static_cast<VkDeviceSize>(offset) };
	vkCmdBindVertexBuffers(cmdBuf, 0, 1, buffers, offsets);
}

void vtek::cmd_bind_descriptor_set_graphics(
	vtek::CommandBuffer* commandBuffer, vtek::GraphicsPipeline* pipeline,
	vtek::DescriptorSet* descriptorSet)
{
	auto cmdBuf = vtek::command_buffer_get_handle(commandBuffer);
	VkDescriptorSet descrSet = vtek::descriptor_set_get_handle(descriptorSet);
	VkPipelineLayout pipLayout = vtek::graphics_pipeline_get_layout(pipeline);
	vkCmdBindDescriptorSets(
		cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipLayout, 0, 1,
		&descrSet, 0, nullptr); // NOTE: Dynamic offset unused
}

void vtek::cmd_draw_vertices(
	vtek::CommandBuffer* commandBuffer, uint32_t numVertices)
{
	auto cmdBuf = vtek::command_buffer_get_handle(commandBuffer);
	vkCmdDraw(cmdBuf, numVertices, 1, 0, 0);
}
