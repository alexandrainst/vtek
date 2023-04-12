#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>
#include "vtek_device.h"
#include "vtek_render_pass.h"


namespace vtek
{
	// ====================== //
	// === Pipeline states == //
	// ====================== //
	enum class PrimitiveTopology
	{
		point_list, line_list, line_strip, triangle_list, triangle_strip
	};

	struct ViewportState
	{
		VkRect2D viewportRegion {};
		// NOTE: good default values for min/max depth that need not be changed.
		float minDepth {0.0f};
		float maxDepth {1.0f};
		// Scissor rectangle describes in which region pixels will be stored.
		// Any pixels outside the scissor rectangle are discarded by the rasterizer.
		bool useScissorRegion {false};
		VkRect2D scissorRegion {};
	};

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

	enum class RenderPassType { renderpass, dynamic };


	// ============================== //
	// === Pipeline creation info === //
	// ============================== //

	// NOTE: All fields must be filled out properly so the behaviour of the
	// pipeline is well-defined.
	struct GraphicsPipelineCreateInfo
	{
		// NOTE: If renderPassType == `RenderPassType::dynamic`, then
		//  `renderPass` _should_ be NULL.
		RenderPassType renderPassType {vtek::RenderPassType::renderpass};
		RenderPass* renderPass {nullptr};

		// TODO: Parameters:
		// GraphicsShader* shader;
		// RenderPass* renderPass;
		// DescriptorSetLayout descriptorSetLayout;

		/* Neat and ordered: */

		// shader stages

		// vertex input

		// input assembler
		PrimitiveTopology primitiveTopology {PrimitiveTopology::triangle_list};
		bool enablePrimitiveRestart {false};

		// viewport state
		// TODO: It is possible to use multiple viewports and scissor rectangles
		// on some GPUs, which requires enabling a feature during device creation.
		// How to handle that here? / Should we?
		ViewportState* viewportState;

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
		const GraphicsPipelineCreateInfo* info, Device* device);
	void graphics_pipeline_destroy(GraphicsPipeline* pipeline);

	VkPipeline graphics_pipeline_get_handle(GraphicsPipeline* pipeline);
	VkPipelineLayout graphics_pipeline_get_layout(GraphicsPipeline* pipeline);
	RenderPassType graphics_pipeline_get_render_pass_type(GraphicsPipeline* pipeline);
}
