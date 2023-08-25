#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

#include "vtek_object_handles.hpp"
#include "vtek_types.hpp"


namespace vtek
{
	// ========================== //
	// === Format description === //
	// ========================== //

	// 1:1 mapping of formats in the Vulkan specification
	enum class Format : uint16_t
	{
		undefined,

		// Packed formats with small channel sizes
		r4g4_unorm_pack8,
		r4g4b4a4_unorm_pack16, b4g4r4a4_unorm_pack16,
		r5g6b5_unorm_pack16, b5g6r5_unorm_pack16,
		r5g5b5a1_unorm_pack16, b5g5r5a1_unorm_pack16, a1r5g5b5_unorm_pack16,

		// Standard single-channel formats
		r8_unorm, r8_snorm, r8_uscaled, r8_sscaled, r8_uint, r8_sint, r8_srgb,

		// Standard two-channel formats
		r8g8_unorm, r8g8_snorm, r8g8_uscaled, r8g8_sscaled, r8g8_uint, r8g8_sint,
		r8g8_srgb,

		// Standard three-channel formats
		r8g8b8_unorm, r8g8b8_snorm, r8g8b8_uscaled, r8g8b8_sscaled,
		r8g8b8_uint, r8g8b8_sint, r8g8b8_srgb,

		// Standard three-channel formats, blue-endian fashion
		b8g8r8_unorm, b8g8r8_snorm, b8g8r8_uscaled, b8g8r8_sscaled,
		b8g8r8_uint, b8g8r8_sint, b8g8r8_srgb,

		// Standard four-channel formats
		r8g8b8a8_unorm, r8g8b8a8_snorm, r8g8b8a8_uscaled, r8g8b8a8_sscaled,
		r8g8b8a8_uint, r8g8b8a8_sint, r8g8b8a8_srgb,

		// Standard four-channel formats, blue-endian fashion
		b8g8r8a8_unorm, b8g8r8a8_snorm, b8g8r8a8_uscaled, b8g8r8a8_sscaled,
		b8g8r8a8_uint, b8g8r8a8_sint, b8g8r8a8_srgb,

		// Standard four-channel formats, blue-endian and alpha first
		a8b8g8r8_unorm_pack32, a8b8g8r8_snorm_pack32, a8b8g8r8_uscaled_pack32,
		a8b8g8r8_sscaled_pack32, a8b8g8r8_uint_pack32, a8b8g8r8_sint_pack32,
		a8b8g8r8_srgb_pack32,

		// Packed four-channel formats, blue-ending and 2-bit alpha first
		a2r10g10b10_unorm_pack32, a2r10g10b10_snorm_pack32,
		a2r10g10b10_uscaled_pack32, a2r10g10b10_sscaled_pack32,
		a2r10g10b10_uint_pack32, a2r10g10b10_sint_pack32,
		a2b10g10r10_unorm_pack32, a2b10g10r10_snorm_pack32,
		a2b10g10r10_uscaled_pack32, a2b10g10r10_sscaled_pack32,
		a2b10g10r10_uint_pack32, a2b10g10r10_sint_pack32,

		// 16-bit single-channel formats
		r16_unorm, r16_snorm, r16_uscaled, r16_sscaled, r16_uint, r16_sint,
		r16_sfloat,

		// 16-bit two-channel formats
		r16g16_unorm, r16g16_snorm, r16g16_uscaled, r16g16_sscaled, r16g16_uint,
		r16g16_sint, r16g16_sfloat,

		// 16-bit three-channel formats
		r16g16b16_unorm, r16g16b16_snorm, r16g16b16_uscaled, r16g16b16_sscaled,
		r16g16b16_uint, r16g16b16_sint, r16g16b16_sfloat,

		// 16-bit four-channel formats
		r16g16b16a16_unorm, r16g16b16a16_snorm, r16g16b16a16_uscaled,
		r16g16b16a16_sscaled, r16g16b16a16_uint, r16g16b16a16_sint,
		r16g16b16a16_sfloat,

		// 32-bit formats
		r32_uint, r32_sint, r32_sfloat,
		r32g32_uint, r32g32_sint, r32g32_sfloat,
		r32g32b32_uint, r32g32b32_sint, r32g32b32_sfloat,
		r32g32b32a32_uint, r32g32b32a32_sint, r32g32b32a32_sfloat,

		// 64-bit formats
		r64_uint, r64_sint, r64_sfloat,
		r64g64_uint, r64g64_sint, r64g64_sfloat,
		r64g64b64_uint, r64g64b64_sint, r64g64b64_sfloat,
		r64g64b64a64_uint, r64g64b64a64_sint, r64g64b64a64_sfloat,

		// Special packed 3-channel format, e.g. used for bloom
		b10g11r11_ufloat_pack32,

		// Special packed 4-channel format; 'e' probably means HDRE radiance!
		e5b9g9r9_ufloat_pack32,

		// Depth/stencil formats
		d16_unorm, x8_d24_unorm_pack32, d32_sfloat, s8_uint,
		d16_unorm_s8_uint, d24_unorm_s8_uint, d32_sfloat_s8_uint,

		// Block-compressed formats
		bc1_rgb_unorm_block, bc1_rgb_srgb_block,
		bc1_rgba_unorm_block, bc1_rgba_srgb_block,
		bc2_unorm_block, bc2_srgb_block,
		bc3_unorm_block, bc3_srgb_block,
		bc4_unorm_block, bc4_snorm_block,
		bc5_unorm_block, bc5_snorm_block,
		bc6h_ufloat_block, bc6h_sfloat_block,
		bc7_unorm_block, bc7_srgb_block,

		// Ericsson compressed formats (ETC2)
		etc2_r8g8b8_unorm_block, etc2_r8g8b8_srgb_block,
		etc2_r8g8b8a1_unorm_block, etc2_r8g8b8a1_srgb_block,
		etc2_r8g8b8a8_unorm_block, etc2_r8g8b8a8_srgb_block,

		// ETC2 alpha-compressed formats
		eac_r11_unorm_block, eac_r11_snorm_block,
		eac_r11g11_unorm_block, eac_r11g11_snorm_block,

		// ASTC-compressed formats, both unorm and sRGB
		astc_4x4_unorm_block, astc_4x4_srgb_block,
		astc_5x4_unorm_block, astc_5x4_srgb_block,
		astc_5x5_unorm_block, astc_5x5_srgb_block,
		astc_6x5_unorm_block, astc_6x5_srgb_block,
		astc_6x6_unorm_block, astc_6x6_srgb_block,
		astc_8x5_unorm_block, astc_8x5_srgb_block,
		astc_8x6_unorm_block, astc_8x6_srgb_block,
		astc_8x8_unorm_block, astc_8x8_srgb_block,
		astc_10x5_unorm_block, astc_10x5_srgb_block,
		astc_10x6_unorm_block, astc_10x6_srgb_block,
		astc_10x8_unorm_block, astc_10x8_srgb_block,
		astc_10x10_unorm_block, astc_10x10_srgb_block,
		astc_12x10_unorm_block, astc_12x10_srgb_block,
		astc_12x12_unorm_block, astc_12x12_srgb_block,

		// ===
		// === All formats below here requires Vulkan >= 1.1
		// ===

		// Two special formats that encode a 2x1 rectangle (see SPEC.)
		g8b8g8r8_422_unorm, b8g8r8g8_422_unorm,

		// Multi-planar formats; probably intended for Y'CrCb conversion (see SPEC.)
		g8_b8_r8_3plane_420_unorm, g8_b8r8_2plane_420_unorm,
		g8_b8_r8_3plane_422_unorm, g8_b8r8_2plane_422_unorm,
		g8_b8_r8_3plane_444_unorm,

		// Special packed 16-bit formats; for each channel x bits are unused
		r10x6_unorm_pack16,
		r10x6g10x6_unorm_2pack16,
		r10x6g10x6b10x6a10x6_unorm_4pack16,
		g10x6b10x6g10x6r10x6_422_unorm_4pack16,
		b10x6g10x6r10x6g10x6_422_unorm_4pack16,

		// Multi-planar formats with x unused bits for each channel
		g10x6_b10x6_r10x6_3plane_420_unorm_3pack16,
		g10x6_b10x6r10x6_2plane_420_unorm_3pack16,
		g10x6_b10x6_r10x6_3plane_422_unorm_3pack16,
		g10x6_b10x6r10x6_2plane_422_unorm_3pack16,
		g10x6_b10x6_r10x6_3plane_444_unorm_3pack16,

		// More special formats, as described above, in order given by SPEC.
		r12x4_unorm_pack16,
		r12x4g12x4_unorm_2pack16,
		r12x4g12x4b12x4a12x4_unorm_4pack16,
		g12x4b12x4g12x4r12x4_422_unorm_4pack16,
		b12x4g12x4r12x4g12x4_422_unorm_4pack16,
		g12x4_b12x4_r12x4_3plane_420_unorm_3pack16,
		g12x4_b12x4r12x4_2plane_420_unorm_3pack16,
		g12x4_b12x4_r12x4_3plane_422_unorm_3pack16,
		g12x4_b12x4r12x4_2plane_422_unorm_3pack16,
		g12x4_b12x4_r12x4_3plane_444_unorm_3pack16,
		g16b16g16r16_422_unorm,
		b16g16r16g16_422_unorm,
		g16_b16_r16_3plane_420_unorm,
		g16_b16r16_2plane_420_unorm,
		g16_b16_r16_3plane_422_unorm,
		g16_b16r16_2plane_422_unorm,
		g16_b16_r16_3plane_444_unorm,

		// ===
		// === All formats below here requires Vulkan >= 1.3
		// ===

		// More special multi-planar formats (see SPEC.)
		g8_b8r8_2plane_444_unorm,
		g10x6_b10x6r10x6_2plane_444_unorm_3pack16,
		g12x4_b12x4r12x4_2plane_444_unorm_3pack16,
		g16_b16r16_2plane_444_unorm,

		// Packed four-channel formats with 4 bits per channel and alpha first
		a4r4g4b4_unorm_pack16, a4b4g4r4_unorm_pack16,

		// ASTC-compressed formats, signed float storage
		astc_4x4_sfloat_block,
		astc_5x4_sfloat_block,
		astc_5x5_sfloat_block,
		astc_6x5_sfloat_block,
		astc_6x6_sfloat_block,
		astc_8x5_sfloat_block,
		astc_8x6_sfloat_block,
		astc_8x8_sfloat_block,
		astc_10x5_sfloat_block,
		astc_10x6_sfloat_block,
		astc_10x8_sfloat_block,
		astc_10x10_sfloat_block,
		astc_12x10_sfloat_block,
		astc_12x12_sfloat_block
	};

	enum class FormatCompression : uint8_t
	{
		// no compression (default)
		none,

		// block compression
		any_bc, bc1, bc2, bc3, bc4, bc5, bc6h, bc7,

		// Ericsson texture compression
		etc2,

		// ETC2 alpha compression
		eac,

		// Adaptive Scalable Texture Compression (LDR profile)
		astc_4x4,
		astc_5x4, astc_5x5,
		astc_6x5, astc_6x6,
		astc_8x5, astc_8x6, astc_8x8,
		astc_10x5, astc_10x6, astc_10x8, astc_10x10,
		astc_12x10, astc_12x12,
	};

	enum class FormatStorageType
	{
		unorm,   // float in range [0, 1]
		snorm,   // float in range [-1, 1]
		uscaled, // unsigned int converted to float
		sscaled, // signed int converted to float
		uint,    // unsigned int
		sint,    // signed int
		ufloat,  // unsigned float
		sfloat,  // signed float
		float16, // half-precision float per-component
		float32, // 32-bit float per-component

		// all channels packed in x bits, with y format
		unorm_pack8,
		unorm_pack16,
		unorm_pack32,
		snorm_pack32,
		ufloat_pack32,
		uscaled_pack32,
		sscaled_pack32,
		uint_pack32,
		sint_pack32
	};

	enum class FormatChannels : uint32_t
	{
		channels_1 = 1,
		channels_2 = 2,
		channels_3 = 3,
		channels_4 = 4
	};

	enum class FormatChannelSize
	{
		// exactly 8, 16, 32, or 64 bits for each channel of each pixel
		channel_8,
		channel_16,
		channel_32,
		channel_64,

		// everything else, like packed/compressed formats
		special
	};

	enum class FormatFeature : uint32_t
	{
		sampled_image               = 0x00000001U,
		storage_image               = 0x00000002U,
		storage_image_atomic        = 0x00000004U,
		uniform_texel_buffer        = 0x00000008U,
		storage_texel_buffer        = 0x00000010U,
		storage_texel_buffer_atomic = 0x00000020U,
		vertex_buffer               = 0x00000040U,
		color_attachment            = 0x00000080U,
		color_attachment_blend      = 0x00000100U,
		depth_stencil_attachment    = 0x00000200U,
		blit_src                    = 0x00000400U,
		blit_dst                    = 0x00000800U,
		sampled_image_filter_linear = 0x00001000U,

		// Provided by Vulkan >= 1.1
		transfer_src                                 = 0x00002000U,
		transfer_dst                                 = 0x00004000U,
		midpoint_chroma_samples                      = 0x00008000U,
		sampled_image_ycbcr_conv_lin_filter          = 0x00010000U,
		sampled_image_ycbcr_conv_sep_rec_filter      = 0x00020000U,
		sampled_image_ycbcr_conv_chroma_Rec_ex       = 0x00040000U,
		sampled_image_ycbcr_conv_chroma_rec_ex_force = 0x00080000U,
		disjoint_bit                                 = 0x00100000U,
		cosited_chroma_samples                       = 0x00200000U,

		// Provided by Vulkan >= 1.2
		sampled_image_filter_minmax = 0x00400000U
	};


	// ============================== //
	// === Supported format query === //
	// ============================== //

	// Forward-declaration, defined further below
	class SupportedFormat;

	struct FormatQuery
	{
		Format format {Format::undefined};
		bool linearTiling {false};
		EnumBitmask<FormatFeature> features {};
	};

	bool has_format_support(
		const FormatQuery* query, const Device* device,
		SupportedFormat* outSupport);


	// Specify how vtek should search for a suitable Vulkan image format.
	// The restrictions are prioritized as such:
	// 1) compression
	// 2) sRGB
	// 3) number of channels
	// 4) channel size
	// 5) storage type
	// 6) swizzled channels
	struct FormatInfo
	{
		FormatChannels channels {FormatChannels::channels_4};
		bool sRGB {false};
		bool swizzleBGR {false}; // TODO: When, why, and how?
		FormatStorageType storageType {FormatStorageType::unorm};
		FormatChannelSize channelSize {FormatChannelSize::channel_8};
		FormatCompression compression {FormatCompression::none};
	};

	// Check if the device supports a given image format, specified by overloads:
	// - ImageFormatInfo: Specific number of channels, sRGB, compression, etc.
	// - FormatQuery
	bool has_format_support(
		const FormatInfo* info, const FormatQuery* query,
		const Device* device, SupportedFormat* outSupport);


	// ======================== //
	// === Supported format === //
	// ======================== //

	class SupportedFormat
	{
	public:
		inline SupportedFormat() {}
		bool operator==(Format _format) const;

		// Get the underlying Vulkan format
		VkFormat get() const;

		// Retrieve format properties
		FormatChannels get_num_channels() const;
		bool has_alpha() const;
		bool is_srgb() const;
		bool is_compressed() const;
		bool is_linear_tiling_supported() const;
		bool is_blue_endian() const;
		bool is_alpha_first() const;
		bool is_depth_stencil() const;
		bool has_depth() const;
		bool has_stencil() const;

		FormatCompression get_compression_scheme() const;
		FormatStorageType get_storage_type() const;
		EnumBitmask<FormatFeature> get_supported_features() const;

	private:
		// Only the friend function may properly construct this object
		friend bool vtek::has_format_support(
			const FormatQuery*,const Device*,SupportedFormat*);
		SupportedFormat(Format _format) : format(_format) {}

		Format format {Format::undefined};
		VkFormat fmt {VK_FORMAT_UNDEFINED};

		FormatCompression compression {FormatCompression::none};
		FormatStorageType storage {FormatStorageType::unorm};
		EnumBitmask<FormatFeature> features {};
		uint32_t propertyMask {0U};
		// TODO: Store VkFormatFeatureFlags ?
	};







	// REVIEW: We might still want to use this class as a global format database
	class FormatSupport
	{
	public:
		FormatSupport(const Device* device);

		inline bool sRGB_channel_1() { return mSRGB & 0x01; }
		inline bool sRGB_channel_2() { return mSRGB & 0x02; }
		inline bool sRGB_channel_3() { return mSRGB & 0x04; }

		//bool

	private:
		VkPhysicalDevice physicalDevice {VK_NULL_HANDLE};
		uint8_t mSRGB {0U};
	};

	// NOTE: Implemented in src/imgutils/vtek_image_formats.cpp

};
