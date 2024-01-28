#pragma once

#include <vulkan/vulkan.h>

#include "vtek_image.hpp"
#include "vtek_object_handles.hpp"
#include "vtek_push_constants.hpp"
#include "vtek_shaders.hpp"
#include "vtek_vulkan_types.hpp"


namespace vtek
{
	// ========================== //
	// === Layout transitions === //
	// ========================== //

	// It is optional to specify queues, and only needed if the image
	// should have transferred queue ownership.
	struct ImageLayoutTransitionCmdInfo
	{
		Image2D* image {nullptr};
		ImageLayout oldLayout {};
		ImageLayout newLayout {};
		PipelineStage srcStage {};
		PipelineStage dstStage {};
		Queue* srcQueue {nullptr};
		Queue* dstQueue {nullptr};
		EnumBitmask<AccessMask> srcAccessMask {};
		EnumBitmask<AccessMask> dstAccessMask {};
	};

	void cmd_image_layout_transition(
		CommandBuffer* commandBuffer, const ImageLayoutTransitionCmdInfo* info);

	// ========================= //
	// === Graphics pipeline === //
	// ========================= //
	void cmd_bind_graphics_pipeline(
		CommandBuffer* commandBuffer, GraphicsPipeline* pipeline);

	// Set viewport/scissor with explicit size only. Upper-left corner is
	// assumed {0,0}, and depth-range is assumed [0,1].
	void cmd_set_viewport_scissor(CommandBuffer* commandBuffer, glm::uvec2 size);

	// Set viewport/scissor with explicit upper-left corner, size, and
	// depth range.
	void cmd_set_viewport_scissor(
		CommandBuffer* commandBuffer, glm::vec2 upperLeftCorner,
		glm::uvec2 size, glm::vec2 depthRange = {0.0f, 1.0f});

	void cmd_push_constant_graphics(
		CommandBuffer* commandBuffer, GraphicsPipeline* pipeline,
		IPushConstant* pushConstant, EnumBitmask<ShaderStageGraphics> stages);

	// ========================= //
	// === Resource bindings === //
	// ========================= //
	void cmd_bind_vertex_buffer(
		CommandBuffer* commandBuffer, Buffer* buffer, uint64_t offset);
	void cmd_bind_vertex_buffers();

	void cmd_bind_descriptor_set_graphics(
		CommandBuffer* commandBuffer, GraphicsPipeline* pipeline,
		DescriptorSet* descriptorSet);

	// ======================== //
	// === Drawing commands === //
	// ======================== //

	// Issue a draw command, explicitly setting number of vertices and assuming
	// bound vertex buffer(s). Instances are ignored / not assumed.
	void cmd_draw_vertices(CommandBuffer* commandBuffer, uint32_t numVertices);
}
