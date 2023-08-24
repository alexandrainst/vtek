#pragma once

#include <cstdint>

#include "vtek_object_handles.hpp"
#include "vtek_types.hpp"


namespace vtek
{
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

	// 1:1 mapping of formats in the Vulkan specification
	enum class Format
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
		r64_uint = 110,
		r64_sint = 111,
		r64_sfloat = 112,
		r64g64_uint = 113,
		r64g64_sint = 114,
		r64g64_sfloat = 115,
		r64g64b64_uint = 116,
		r64g64b64_sint = 117,
		r64g64b64_sfloat = 118,
		r64g64b64a64_uint = 119,
		r64g64b64a64_sint = 120,
		r64g64b64a64_sfloat = 121,
		b10g11r11_ufloat_pack32 = 122,
		e5b9g9r9_ufloat_pack32 = 123,
		d16_unorm = 124,
		x8_d24_unorm_pack32 = 125,
		d32_sfloat = 126,
		s8_uint = 127,
		d16_unorm_s8_uint = 128,
		d24_unorm_s8_uint = 129,
		d32_sfloat_s8_uint = 130,
		bc1_rgb_unorm_block = 131,
		bc1_rgb_srgb_block = 132,
		bc1_rgba_unorm_block = 133,
		bc1_rgba_srgb_block = 134,
		bc2_unorm_block = 135,
		bc2_srgb_block = 136,
		bc3_unorm_block = 137,
		bc3_srgb_block = 138,
		bc4_unorm_block = 139,
		bc4_snorm_block = 140,
		bc5_unorm_block = 141,
		bc5_snorm_block = 142,
		bc6h_ufloat_block = 143,
		bc6h_sfloat_block = 144,
		bc7_unorm_block = 145,
		bc7_srgb_block = 146,
		etc2_r8g8b8_unorm_block = 147,
		etc2_r8g8b8_srgb_block = 148,
		etc2_r8g8b8a1_unorm_block = 149,
		etc2_r8g8b8a1_srgb_block = 150,
		etc2_r8g8b8a8_unorm_block = 151,
		etc2_r8g8b8a8_srgb_block = 152,
		eac_r11_unorm_block = 153,
		eac_r11_snorm_block = 154,
		eac_r11g11_unorm_block = 155,
		eac_r11g11_snorm_block = 156,
		astc_4x4_unorm_block = 157,
		astc_4x4_srgb_block = 158,
		astc_5x4_unorm_block = 159,
		astc_5x4_srgb_block = 160,
		astc_5x5_unorm_block = 161,
		astc_5x5_srgb_block = 162,
		astc_6x5_unorm_block = 163,
		astc_6x5_srgb_block = 164,
		astc_6x6_unorm_block = 165,
		astc_6x6_srgb_block = 166,
		astc_8x5_unorm_block = 167,
		astc_8x5_srgb_block = 168,
		astc_8x6_unorm_block = 169,
		astc_8x6_srgb_block = 170,
		astc_8x8_unorm_block = 171,
		astc_8x8_srgb_block = 172,
		astc_10x5_unorm_block = 173,
		astc_10x5_srgb_block = 174,
		astc_10x6_unorm_block = 175,
		astc_10x6_srgb_block = 176,
		astc_10x8_unorm_block = 177,
		astc_10x8_srgb_block = 178,
		astc_10x10_unorm_block = 179,
		astc_10x10_srgb_block = 180,
		astc_12x10_unorm_block = 181,
		astc_12x10_srgb_block = 182,
		astc_12x12_unorm_block = 183,
		astc_12x12_srgb_block = 184,

		// Provided by VK_VERSION_1_1
		g8b8g8r8_422_unorm = 1000156000,
		b8g8r8g8_422_unorm = 1000156001,
		g8_b8_r8_3plane_420_unorm = 1000156002,
		g8_b8r8_2plane_420_unorm = 1000156003,
		g8_b8_r8_3plane_422_unorm = 1000156004,
		g8_b8r8_2plane_422_unorm = 1000156005,
		g8_b8_r8_3plane_444_unorm = 1000156006,
		r10x6_unorm_pack16 = 1000156007,
		r10x6g10x6_unorm_2pack16 = 1000156008,
		r10x6g10x6b10x6a10x6_unorm_4pack16 = 1000156009,
		g10x6b10x6g10x6r10x6_422_unorm_4pack16 = 1000156010,
		b10x6g10x6r10x6g10x6_422_unorm_4pack16 = 1000156011,
		g10x6_b10x6_r10x6_3plane_420_unorm_3pack16 = 1000156012,
		g10x6_b10x6r10x6_2plane_420_unorm_3pack16 = 1000156013,
		g10x6_b10x6_r10x6_3plane_422_unorm_3pack16 = 1000156014,
		g10x6_b10x6r10x6_2plane_422_unorm_3pack16 = 1000156015,
		g10x6_b10x6_r10x6_3plane_444_unorm_3pack16 = 1000156016,
		r12x4_unorm_pack16 = 1000156017,
		r12x4g12x4_unorm_2pack16 = 1000156018,
		r12x4g12x4b12x4a12x4_unorm_4pack16 = 1000156019,
		g12x4b12x4g12x4r12x4_422_unorm_4pack16 = 1000156020,
		b12x4g12x4r12x4g12x4_422_unorm_4pack16 = 1000156021,
		g12x4_b12x4_r12x4_3plane_420_unorm_3pack16 = 1000156022,
		g12x4_b12x4r12x4_2plane_420_unorm_3pack16 = 1000156023,
		g12x4_b12x4_r12x4_3plane_422_unorm_3pack16 = 1000156024,
		g12x4_b12x4r12x4_2plane_422_unorm_3pack16 = 1000156025,
		g12x4_b12x4_r12x4_3plane_444_unorm_3pack16 = 1000156026,
		g16b16g16r16_422_unorm = 1000156027,
		b16g16r16g16_422_unorm = 1000156028,
		g16_b16_r16_3plane_420_unorm = 1000156029,
		g16_b16r16_2plane_420_unorm = 1000156030,
		g16_b16_r16_3plane_422_unorm = 1000156031,
		g16_b16r16_2plane_422_unorm = 1000156032,
		g16_b16_r16_3plane_444_unorm = 1000156033,

		// Provided by VK_VERSION_1_3
		g8_b8r8_2plane_444_unorm = 1000330000,
		g10x6_b10x6r10x6_2plane_444_unorm_3pack16 = 1000330001,
		g12x4_b12x4r12x4_2plane_444_unorm_3pack16 = 1000330002,
		g16_b16r16_2plane_444_unorm = 1000330003,
		a4r4g4b4_unorm_pack16 = 1000340000,
		a4b4g4r4_unorm_pack16 = 1000340001,
		astc_4x4_sfloat_block = 1000066000,
		astc_5x4_sfloat_block = 1000066001,
		astc_5x5_sfloat_block = 1000066002,
		astc_6x5_sfloat_block = 1000066003,
		astc_6x6_sfloat_block = 1000066004,
		astc_8x5_sfloat_block = 1000066005,
		astc_8x6_sfloat_block = 1000066006,
		astc_8x8_sfloat_block = 1000066007,
		astc_10x5_sfloat_block = 1000066008,
		astc_10x6_sfloat_block = 1000066009,
		astc_10x8_sfloat_block = 1000066010,
		astc_10x10_sfloat_block = 1000066011,
		astc_12x10_sfloat_block = 1000066012,
		astc_12x12_sfloat_block = 1000066013
	};

	enum class ImageTiling
	{
		optimal, linear
	};

	struct FormatQuery
	{
		ImageTiling tiling {ImageTiling::optimal};
		EnumBitmask<FormatFeature> features {};
	};

	// Check if the device supports a given image format, specified by overloads:
	// - ImageFormatInfo: Specific number of channels, sRGB, compression, etc.
	// - FormatQuery
	bool has_format_support(
		const ImageFormatInfo* info, const FormatQuery* query,
		const Device* device, SupportedFormat* outSupport);

	bool has_format_support(
		const FormatQuery* query, const Device* device,
		SupportedFormat* outSupport);


	class SupportedFormat
	{
	public:
		VkFormat get() const;
		inline SupportedFormat() {}
		SupportedFormat& operator=(const SupportedFormat& sf) {} // TODO: Invalid!

		// Retrieve format properties
		ImageChannels get_num_channels() const;
		bool is_srgb() const;
		bool is_compressed() const;
		bool is_linear_tiling_supported() const;
		ImageCompressionScheme get_compression_scheme() const;
		ImagePixelStorageFormat get_storage_format() const;
		EnumBitmask<ImageUsageFlag> get_supported_usage_flags() const; // TODO: ?
		EnumBitmask<FormatFeatureFlag> get_supported_format_features() const;


	private:
		inline SupportedFormat(VkFormat _format) : format(_format) {}

		const VkFormat format {VK_FORMAT_UNDEFINED};
		// TODO: Store VkFormatFeatureFlags ?
	};
};
