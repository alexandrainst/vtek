#pragma once

#include <vulkan/vulkan.h>

#include "vtek_image.hpp"
#include "vtek_object_handles.hpp"
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

	// ======================= //
	// === Pipeline states === //
	// ======================= //
	void cmd_bind_graphics_pipeline(
		CommandBuffer* commandBuffer, GraphicsPipeline* pipeline);
}
