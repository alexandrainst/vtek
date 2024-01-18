#pragma once

#include <vulkan/vulkan.h>

#include "vtek_types.hpp"


namespace vtek
{
	// Image tiling specifies the tiling arrangement of texel blocks in an
	// image. This is used for querying for supported image formats, and for
	// creating images and framebuffer attachments.
	enum class ImageTiling
	{
		// In almost all cases, optimal tiling is preferred. The internal
		// arrangement of texels is implementation-dependent for more
		// efficient memory access.
		optimal,

		// Linear tiling specifies texels to be laid out in row-major order,
		// possibly with some padding on each row. This makes it easier to
		// read/write texel data to/from the CPU without performing image
		// layout transition barriers, but makes for less efficient memory
		// access during rendering.
		// NOTE: Linear tiling has limited supported, so optimal tiling
		// should be preferred.
		linear
	};

	VkImageTiling get_image_tiling(ImageTiling tiling);


	// Multisampling is one way to perform anti-aliasing, by sampling multiple
	// fragments per-pixel, and then resolving those fragments. This smooths out
	// polygon edges. Multisampling is not recommended for deferred rendering.
	enum class MultisampleType
	{
		none, msaa_x2, msaa_x4, msaa_x8, msaa_x16, msaa_x32, msaa_x64
	};

	VkSampleCountFlagBits get_multisample_count(MultisampleType sample);
	MultisampleType get_multisample_enum(VkSampleCountFlagBits count);


	// Cull mode determines if any face of rendered polygons should be culled
	// (ie. discarded) by pipeline rasterization. Back culling is recommended
	// to avoid overdraw.
	enum class CullMode
	{
		none, front, back, front_and_back
	};

	VkCullModeFlags get_cull_mode(CullMode mode);


	// Pipeline stages, which may be used for synchronization of resources,
	// such as signaling of fences/semaphores, or issuing pipeline barriers.
	enum class PipelineStage
	{
		// Equivalent to `::all_commands` with access flags set to 0 in the
		// second synchronization scope, but specifies no stage of execution in
		// the first scope.
		top_of_pipe,

		// Where `VkDrawIndirect*/VkDispatchIndirec*/VkTraceRaysIndirect*`
		// data structures are consumed
		draw_indirect,

		// Where vertex and index buffers are consumed
		vertex_input,

		// Shader stages
		vertex_shader,
		tess_control_shader,
		tess_eval_shader,
		geometry_shader,
		fragment_shader,

		// Where early tests (depth/stencil) are performed, before fragment shading
		early_fragment_tests,
		// Where late tests (depth/stencil) are performed, after fragment shading
		late_fragment_tests,

		// After blending, where the final colors are output from the pipeline
		color_attachment_output,

		// Compute shader execution
		compute_shader,

		// All copy commands including vkCmdCopyQueryPoolResults, blit image and
		// resolve image commands, all clear commands except vkCmdClearAttachments.
		transfer,

		// Equivalent to `::all_commands` with access flags set to 0 in the
		// first synchronization scope, but specifies no stage of execution in
		// the second scope.
		bottom_of_pipe,

		// A pseudo-stage indicating host execution of read/write access on
		// device memory. This stage is not invoked by any commands recorded
		// in a command buffer.
		host,

		// Equivalent to a logical OR on all the graphics pipeline stages
		all_graphics,
		all_commands,

		// Provided by VK_VERSION_1_3
		// Specifies no stage of execution
		none
	};

	VkPipelineStageFlags get_pipeline_stage(PipelineStage stage);


	// Bits which can be set to pipeline barriers and subpass dependencies,
	// specifying access behavior.
	enum class AccessMask : uint32_t
	{
		indirect_command_read          = 0x00000001U,
		index_read                     = 0x00000002U,
		vertex_attribute_read          = 0x00000004U,
		uniform_read                   = 0x00000008U,
		input_attachment_read          = 0x00000010U,
		shader_read                    = 0x00000020U,
		shader_write                   = 0x00000040U,
		color_attachment_read          = 0x00000080U,
		color_attachment_write         = 0x00000100U,
		depth_stencil_attachment_read  = 0x00000200U,
		depth_stencil_attachment_write = 0x00000400U,
		transfer_read                  = 0x00000800U,
		transfer_write                 = 0x00001000U,
		host_read                      = 0x00002000U,
		host_write                     = 0x00004000U,
		memory_read                    = 0x00008000U,
		memory_write                   = 0x00010000U,
		// Provided by VK_VERSION_1_3
		none                           = 0x00020000U
	};

	VkAccessFlags get_access_mask(EnumBitmask<AccessMask> accessMask);


	class ClearValue
	{
	public:
		// Get the underlying Vulkan format
		inline VkClearValue get() const { return _value; }

		inline void setDepth(float d) {
			_value = { .depthStencil = { .depth = d, .stencil = 0 }};
		}
		inline void setDepthStencil(float d, uint32_t s) {
			_value = { .depthStencil = { .depth = d, .stencil = s } };
		}

		inline void setColorFloat(float r) {
			_value = { .color = { .float32 = {r, 0.0f, 0.0f, 1.0f} } };
		}
		inline void setColorFloat(float r, float g) {
			_value = { .color = { .float32 = {r, g, 0.0f, 1.0f} } };
		}
		inline void setColorFloat(float r, float g, float b) {
			_value = { .color = { .float32 = {r, g, b, 1.0f} } };
		}
		inline void setColorFloat(float r, float g, float b, float a) {
			_value = { .color = { .float32 = {r, g, b, a} } };
		}

		inline void setColorInt(int32_t r) {
			_value = { .color = { .int32 = {r, 0, 0, 0} } };
		}
		inline void setColorInt(int32_t r, int32_t g) {
			_value = { .color = { .int32 = {r, g, 0, 0} } };
		}
		inline void setColorInt(int32_t r, int32_t g, int32_t b) {
			_value = { .color = { .int32 = {r, g, b, 0} } };
		}
		inline void setColorInt(int32_t r, int32_t g, int32_t b, int32_t a) {
			_value = { .color = { .int32 = {r, g, b, a} } };
		}

		inline void setColorUint(uint32_t r) {
			_value = { .color = { .uint32 = {r, 0U, 0U, 0U} } };
		}
		inline void setColorUint(uint32_t r, uint32_t g) {
			_value = { .color = { .uint32 = {r, g, 0U, 0U} } };
		}
		inline void setColorUint(uint32_t r, uint32_t g, uint32_t b) {
			_value = { .color = { .uint32 = {r, g, b, 0U} } };
		}
		inline void setColorUint(uint32_t r, uint32_t g, uint32_t b, uint32_t a) {
			_value = { .color = { .uint32 = {r, g, b, a} } };
		}

	private:
		VkClearValue _value { .color = { .float32 = {0.0f, 0.0f, 0.0f, 1.0f}}};
	};
}
