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

	void cmd_set_viewport_scissor(
		CommandBuffer* commandBuffer, glm::uvec2 size, glm::vec2 depthBounds);

	void cmd_push_constant_graphics(
		CommandBuffer* commandBuffer, GraphicsPipeline* pipeline,
		IPushConstant* pushConstant, EnumBitmask<ShaderStageGraphics> stages);

	// ========================= //
	// === Resource bindings === //
	// ========================= //
	void cmd_bind_vertex_buffers();

	// ======================== //
	// === Drawing commands === //
	// ======================== //
	void cmd_draw_vertices(CommandBuffer* commandBuffer, uint32_t numVertices);

}
