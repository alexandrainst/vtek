#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>
#include "vtek_device.h"
#include "vtek_render_pass.h"
#include "vtek_vertex_data.h"


namespace vtek
{
	// ====================== //
	// === Pipeline states == //
	// ====================== //
	enum class PrimitiveTopology
	{
		point_list,
		line_list, line_strip,
		triangle_list, triangle_strip, triangle_fan,
		line_list_with_adjacency, line_strip_with_adjacency,
		triangle_list_with_adjacency, triangle_strip_with_adjacency,
		patch_list
	};

	enum class PolygonMode
	{
		fill, line, point
	};

	enum class CullMode
	{
		none, front, back, front_and_back
	};

	enum class FrontFace
	{
		clockwise, counter_clockwise
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
		// TODO: std::vector<VkRect2D> scissorRegions; ?
		VkRect2D scissorRegion {};
	};

	struct RasterizationState
	{
		// If enabled, fragments outside the near and far view frustrum planes
		// are clamped instead of being discarded. Useful e.g. for shadow mapping.
		// Using this requires enabling a feature during device creation.
		bool depthClampEnable {false};
		// If enabled, geometry never passes through rasterization, which then
		// entirely discards any output to the framebuffer.
		bool rasterizerDiscardEnable {false};
		// NOTE: Any other polygon mode than `fill` requires enabling a feature
		// during device creation.
		PolygonMode polygonMode;
		// Line thickness in terms of number of fragments. Maximum supported value
		// is hardware-dependent, and values larger than `1.0f` requires enabling
		// the `wideLines` feature during device creation.
		float lineWidth {1.0f};
		// Backface culling (Travis Vroman: back and CCW is most common):
		CullMode cullMode {CullMode::none};
		FrontFace frontFace {FrontFace::counter_clockwise};
		// The rasterizer can alter the depth values by adding a constant value
		// or biasing them based on a fragment's slope. This is sometimes used
		// for shadow mapping.
		bool depthBiasEnable {false};
		float depthBiasConstantFactor {0.0f};
		float depthBiasClamp {0.0f};
		float depthBiasSlopeFactor {0.0f};
	};

	// NOTE: cpp-enum-proposal for bitmasks!
	enum class PipelineDynamicState : uint32_t
	{
		viewport                    = 0x00000001U,
		scissor                     = 0x00000002U,
		line_width                  = 0x00000004U,
		depth_bias                  = 0x00000008U,
		blend_constants             = 0x00000010U,
		depth_bounds                = 0x00000020U,
		stencil_compare_mask        = 0x00000040U,
		stencil_write_mask          = 0x00000080U,
		stencil_reference           = 0x00000100U,
#if defined(VK_API_VERSION_1_3)
		cull_mode                   = 0x00000200U,
		front_face                  = 0x00000400U,
		primitive_topology          = 0x00000800U,
		viewport_with_count         = 0x00001000U,
		scissor_with_count          = 0x00002000U,
		vertex_input_binding_stride = 0x00004000U,
		depth_test_enable           = 0x00008000U,
		depth_write_enable          = 0x00010000U,
		depth_compare_op            = 0x00020000U,
		depth_bounds_test_enable    = 0x00040000U,
		stencil_test_enable         = 0x00080000U,
		stencil_op                  = 0x00100000U,
		rasterizer_discard_enable   = 0x00200000U,
		depth_bias_enable           = 0x00400000U,
		primitive_restart_enable    = 0x00800000U,
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
		VertexType vertexType {vtek::VertexType::vec2};

		// input assembler
		PrimitiveTopology primitiveTopology {PrimitiveTopology::triangle_list};
		bool enablePrimitiveRestart {false};

		// viewport state
		// TODO: It is possible to use multiple viewports and scissor rectangles
		// on some GPUs, which requires enabling a feature during device creation.
		// How to handle that here? / Should we?
		ViewportState* viewportState;

		// rasterization
		RasterizationState* rasterizationState;

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
