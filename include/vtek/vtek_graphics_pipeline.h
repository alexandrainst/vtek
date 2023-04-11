#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>
#include "vtek_device.h"


namespace vtek
{
	enum class RenderPassType { renderpass, dynamic };

	// NOTE: cpp-enum-proposal for bitmasks!
	enum class PipelineDynamicState : uint32_t
	{
		viewport                    = 0x00000001,
		scissor                     = 0x00000002,
		line_width                  = 0x00000004,
		depth_bias                  = 0x00000008,
		blend_constants             = 0x00000010,
		depth_bounds                = 0x00000020,
		stencil_compare_mask        = 0x00000040,
		stencil_write_mask          = 0x00000080,
		stencil_reference           = 0x00000100,
#if defined(VK_API_VERSION_1_3)
		cull_mode                   = 0x00000200,
		front_face                  = 0x00000400,
		primitive_topology          = 0x00000800,
		viewport_with_count         = 0x00001000,
		scissor_with_count          = 0x00002000,
		vertex_input_binding_stride = 0x00004000,
		depth_test_enable           = 0x00008000,
		depth_write_enable          = 0x00010000,
		depth_compare_op            = 0x00020000,
		depth_bounds_test_enable    = 0x00040000,
		stencil_test_enable         = 0x00080000,
		stencil_op                  = 0x00100000,
		rasterizer_discard_enable   = 0x00200000,
		depth_bias_enable           = 0x00400000,
		primitive_restart_enable    = 0x00800000,
#endif
	};

	typedef uint32_t PipelineDynamicStateFlags;

	struct GraphicsPipelineCreateInfo
	{
		RenderPassType renderPassType {vtek::RenderPassType::renderpass};
		VkRenderPass renderPass {}; // TODO: Use vtek type instead!

		// TODO: Parameters:
		// GraphicsShader* shader;
		// RenderPass* renderPass;
		// DescriptorSetLayout descriptorSetLayout;

		/* Neat and ordered: */

		// shader stages

		// vertex input

		// input assembler

		// viewport state

		// rasterization

		// multisample state

		// depth and stencil testing

		// color blending

		// dynamic states
		PipelineDynamicStateFlags dynamicStateFlags {0U};

		// pipeline layout

	};

	struct GraphicsPipeline; // opaque handle


	GraphicsPipeline* graphics_pipeline_create(
		const GraphicsPipelineCreateInfo& info, Device* device);
	void graphics_pipeline_destroy(GraphicsPipeline* pipeline);

	VkPipeline graphics_pipeline_get_handle(GraphicsPipeline* pipeline);
	VkPipelineLayout graphics_pipeline_get_layout(GraphicsPipeline* pipeline);
	RenderPassType graphics_pipeline_get_render_pass_type(GraphicsPipeline* pipeline);
}
