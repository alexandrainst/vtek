#pragma once

#include <cstdint>
#include <utility>
#include <vector>
#include <vulkan/vulkan.h>

#include "vtek_types.hpp"
#include "vtek_vertex_data.hpp"


namespace vtek
{
	// ============================== //
	// === Pipeline configuration === //
	// ============================== //
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

	// Multisampling is one way to perform anti-aliasing, by sampling multiple
	// fragments per-pixel, and then resolving those fragments. This smooths out
	// polygon edges. Multisampling is not recommended for deferred rendering.
	enum class MultisampleType
	{
		none, msaa_x2, msaa_x4, msaa_x8, msaa_x16, msaa_x32, msaa_x64
	};

	enum class DepthCompareOp
	{
		never, less, equal, less_equal, greater, not_equal, greater_equal, always
	};

	enum class LogicOp
	{
		clear, and_op, and_reverse, copy, and_inverted, no_op,
		xor_op, or_op, nor, equivalent, invert, or_reverse, copy_inverted,
		or_inverted, nand, set
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


	// ====================== //
	// === Pipeline states == //
	// ====================== //
	struct ViewportState
	{
		VkRect2D viewportRegion {};
		// NOTE: good default values for min/max depth that need not be changed.
		// TODO: Where exactly are these used? Misplaced? Seems like depth/stencil stuff.
		// float minDepth {0.0f};
		// float maxDepth {1.0f};
		FloatRange depthRange {0.0f, 1.0f};
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
		VulkanBool depthClampEnable {false};
		// If enabled, geometry never passes through rasterization, which then
		// entirely discards any output to the framebuffer.
		VulkanBool rasterizerDiscardEnable {false};
		// NOTE: Any other polygon mode than `fill` requires enabling a feature
		// during device creation.
		PolygonMode polygonMode {PolygonMode::fill};
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
		VulkanBool depthBiasEnable {false};
		float depthBiasConstantFactor {0.0f};
		float depthBiasClamp {0.0f};
		float depthBiasSlopeFactor {0.0f};
	};

	struct MultisampleState
	{
		MultisampleType numSamples {MultisampleType::none};
		// Sample rate shading causes multisampling to also affect interior
		// filling of geometry. This improves image quality at an additional cost
		// in performance. It requires enabling the `sampleRateShading` feature
		// during device creation. TODO: ??
		VulkanBool enableSampleRateShading {false};
		float minSampleShading {0.0f}; // range [0, 1], closer to 1 is smoother
		// TODO: Sample mask ?? (const VkSampleMask*, which is an array of ??)
		VulkanBool enableAlphaToCoverage {false};
		VulkanBool enableAlphaToOne {false};
	};

	// TODO: Do this? Essentally as a 1:1 wrapper around VkStencilOpState. Meh.
	struct StencilOpState
	{
		int dummyMember;
		/*
		VkStencilOp    failOp;
		VkStencilOp    passOp;
		VkStencilOp    depthFailOp;
		VkCompareOp    compareOp;
		uint32_t       compareMask;
		uint32_t       writeMask;
		uint32_t       reference;
		*/
	};

	struct DepthStencilState
	{
		VulkanBool depthTestEnable {false};
		VulkanBool depthWriteEnable {false};
		DepthCompareOp depthCompareOp {DepthCompareOp::less};
		VulkanBool depthBoundsTestEnable {false};
		FloatRange depthBounds {0.0f, 1.0f};
		VulkanBool stencilTestEnable {false};
		VkStencilOpState stencilTestFront {};
		VkStencilOpState stencilTestBack {};
	};

	struct ColorBlendAttachment
	{
		VulkanBool blendEnable {false};

		VkBlendFactor srcColorBlendFactor {VK_BLEND_FACTOR_ONE};
		VkBlendFactor dstColorBlendFactor {VK_BLEND_FACTOR_ZERO};
		VkBlendOp colorBlendOp {VK_BLEND_OP_ADD};
		VkBlendFactor srcAlphaBlendFactor {VK_BLEND_FACTOR_ONE};
		VkBlendFactor dstAlphaBlendFactor {VK_BLEND_FACTOR_ZERO};
		VkBlendOp alphaBlendOp {VK_BLEND_OP_ADD};
		VkColorComponentFlags colorWriteMask {0U};

		static ColorBlendAttachment GetBlendingDisabled()
		{
			return ColorBlendAttachment {
				.blendEnable = VK_FALSE
			};
		}

		static ColorBlendAttachment GetAlphaBlending()
		{
			// The most common way to use color blending is to implement alpha
			// blending, which should be computed as follows:
			// final.rgb = new.a * new.rgb + (1 - new.a) * old.rgb;
			// final.a = new.a;
			ColorBlendAttachment attachment;
			attachment.blendEnable = VK_TRUE;
			attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			attachment.colorBlendOp = VK_BLEND_OP_ADD;
			// NOTE: Travis: .srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA
			// NOTE: Travis: .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA
			attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
			attachment.alphaBlendOp = VK_BLEND_OP_ADD;
			attachment.colorWriteMask
				= VK_COLOR_COMPONENT_R_BIT
				| VK_COLOR_COMPONENT_G_BIT
				| VK_COLOR_COMPONENT_B_BIT
				| VK_COLOR_COMPONENT_A_BIT;

			return attachment; // C++ RVO probably
		}
	};

	struct ColorBlendState
	{
		std::vector<ColorBlendAttachment> attachments;

		// If we want to use a bitwise operation as the blending method, we
		// set the `logicOpEnable` field to `true`, and then specify the
		// bitwise operation in the `logicOp` field. This will automatically
		// disable the first blending method (described above) for all the
		// framebuffers! This method also uses the color write mask to
		// determine which color channels are affected.
		// It is possible to disable both modes, in which case the fragment
		// colors are written to the framebuffer unmodified.
		VulkanBool logicOpEnable {false};
		LogicOp logicOp {LogicOp::copy};
	};

	struct PipelineRendering
	{
		std::vector<VkFormat> colorAttachmentFormats;
		VkFormat depthAttachmentFormat {VK_FORMAT_UNDEFINED};
		VkFormat stencilAttachmentFormat {VK_FORMAT_UNDEFINED};
	};


	// ============================== //
	// === Pipeline creation info === //
	// ============================== //

	// NOTE: All fields must be filled out properly so the behaviour of the
	// pipeline is well-defined.
	// TODO: Fields to be values or pointers?
	struct GraphicsPipelineCreateInfo
	{
		// NOTE: If renderPassType == `RenderPassType::dynamic`, then
		//  `renderPass` _should_ be NULL, and `pipelineRendering` *must*
		// be a valid pointer to a `PipelineRendering` struct.
		RenderPassType renderPassType {vtek::RenderPassType::renderpass};
		RenderPass* renderPass {nullptr};
		PipelineRendering* pipelineRendering {nullptr};

		// TODO: Parameters:
		// GraphicsShader* shader;
		// RenderPass* renderPass;
		// DescriptorSetLayout descriptorSetLayout;

		/* Neat and ordered: */

		// shader stages
		GraphicsShader* shader {nullptr};

		// vertex input
		VertexType vertexType {vtek::VertexType::vec2};
		bool instancedRendering {false};

		// input assembler
		PrimitiveTopology primitiveTopology {PrimitiveTopology::triangle_list};
		VulkanBool enablePrimitiveRestart {false};

		// viewport state
		// TODO: It is possible to use multiple viewports and scissor rectangles
		// on some GPUs, which requires enabling a feature during device creation.
		// How to handle that here? / Should we?
		ViewportState* viewportState {nullptr};

		// rasterization
		RasterizationState* rasterizationState {nullptr};

		// multisample state
		MultisampleState* multisampleState {nullptr};

		// depth and stencil testing
		// TODO: Could be a pointer instead?
		DepthStencilState* depthStencilState {nullptr};

		// color blending
		// TODO: Overlap between ColorBlendState and PipelineRendering!
		//       Perhaps merge them somehow!!? ! YESSSHHSH
		ColorBlendState* colorBlendState {nullptr};

		// dynamic states
		PipelineDynamicStateFlags dynamicStateFlags {0U};

		// pipeline layout
		// TODO: Get this from shader!
	};


	GraphicsPipeline* graphics_pipeline_create(
		const GraphicsPipelineCreateInfo* info, Device* device);
	void graphics_pipeline_destroy(GraphicsPipeline* pipeline, Device* device);

	VkPipeline graphics_pipeline_get_handle(GraphicsPipeline* pipeline);
	VkPipelineLayout graphics_pipeline_get_layout(GraphicsPipeline* pipeline);
	RenderPassType graphics_pipeline_get_render_pass_type(GraphicsPipeline* pipeline);
}
