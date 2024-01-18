#include "vtek_vulkan.pch"
#include "vtek_format_support.hpp"

#include "imgutils/vtek_image_formats.hpp"
#include "vtek_device.hpp"
#include "vtek_logging.hpp"

using vfmt = vtek::Format;


/* helper functions */
// NOTE: Keep this function as it might be needed at some point! (but unused)
static VkFormat get_format(vtek::Format format)
{
	switch (format)
	{
	case vfmt::undefined:             return VK_FORMAT_UNDEFINED;
	case vfmt::r4g4_unorm_pack8:      return VK_FORMAT_R4G4_UNORM_PACK8;
	case vfmt::r4g4b4a4_unorm_pack16: return VK_FORMAT_R4G4B4A4_UNORM_PACK16;
	case vfmt::b4g4r4a4_unorm_pack16: return VK_FORMAT_B4G4R4A4_UNORM_PACK16;
	case vfmt::r5g6b5_unorm_pack16:   return VK_FORMAT_R5G6B5_UNORM_PACK16;
	case vfmt::b5g6r5_unorm_pack16:   return VK_FORMAT_B5G6R5_UNORM_PACK16;
	case vfmt::r5g5b5a1_unorm_pack16: return VK_FORMAT_R5G5B5A1_UNORM_PACK16;
	case vfmt::b5g5r5a1_unorm_pack16: return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
	case vfmt::a1r5g5b5_unorm_pack16: return VK_FORMAT_A1R5G5B5_UNORM_PACK16;
	case vfmt::r8_unorm:              return VK_FORMAT_R8_UNORM;
	case vfmt::r8_snorm:	          return VK_FORMAT_R8_SNORM;
	case vfmt::r8_uscaled:	          return VK_FORMAT_R8_USCALED;
	case vfmt::r8_sscaled:	          return VK_FORMAT_R8_SSCALED;
	case vfmt::r8_uint:		          return VK_FORMAT_R8_UINT;
	case vfmt::r8_sint:		          return VK_FORMAT_R8_SINT;
	case vfmt::r8_srgb:		          return VK_FORMAT_R8_SRGB;
	case vfmt::r8g8_unorm:	          return VK_FORMAT_R8G8_UNORM;
	case vfmt::r8g8_snorm:	          return VK_FORMAT_R8G8_SNORM;
	case vfmt::r8g8_uscaled:          return VK_FORMAT_R8G8_USCALED;
	case vfmt::r8g8_sscaled:          return VK_FORMAT_R8G8_SSCALED;
	case vfmt::r8g8_uint:	          return VK_FORMAT_R8G8_UINT;
	case vfmt::r8g8_sint:	          return VK_FORMAT_R8G8_SINT;
	case vfmt::r8g8_srgb:	          return VK_FORMAT_R8G8_SRGB;
	case vfmt::r8g8b8_unorm:          return VK_FORMAT_R8G8B8_UNORM;
	case vfmt::r8g8b8_snorm:          return VK_FORMAT_R8G8B8_SNORM;
	case vfmt::r8g8b8_uscaled:        return VK_FORMAT_R8G8B8_USCALED;
	case vfmt::r8g8b8_sscaled:        return VK_FORMAT_R8G8B8_SSCALED;
	case vfmt::r8g8b8_uint:	          return VK_FORMAT_R8G8B8_UINT;
	case vfmt::r8g8b8_sint:	          return VK_FORMAT_R8G8B8_SINT;
	case vfmt::r8g8b8_srgb:	          return VK_FORMAT_R8G8B8_SRGB;
	case vfmt::b8g8r8_unorm:          return VK_FORMAT_B8G8R8_UNORM;
	case vfmt::b8g8r8_snorm:          return VK_FORMAT_B8G8R8_SNORM;
	case vfmt::b8g8r8_uscaled:        return VK_FORMAT_B8G8R8_USCALED;
	case vfmt::b8g8r8_sscaled:        return VK_FORMAT_B8G8R8_SSCALED;
	case vfmt::b8g8r8_uint:	          return VK_FORMAT_B8G8R8_UINT;
	case vfmt::b8g8r8_sint:	          return VK_FORMAT_B8G8R8_SINT;
	case vfmt::b8g8r8_srgb:	          return VK_FORMAT_B8G8R8_SRGB;
	case vfmt::r8g8b8a8_unorm:        return VK_FORMAT_R8G8B8A8_UNORM;
	case vfmt::r8g8b8a8_snorm:        return VK_FORMAT_R8G8B8A8_SNORM;
	case vfmt::r8g8b8a8_uscaled:      return VK_FORMAT_R8G8B8A8_USCALED;
	case vfmt::r8g8b8a8_sscaled:      return VK_FORMAT_R8G8B8A8_SSCALED;
	case vfmt::r8g8b8a8_uint:         return VK_FORMAT_R8G8B8A8_UINT;
	case vfmt::r8g8b8a8_sint:         return VK_FORMAT_R8G8B8A8_SINT;
	case vfmt::r8g8b8a8_srgb:         return VK_FORMAT_R8G8B8A8_SRGB;
	case vfmt::b8g8r8a8_unorm:        return VK_FORMAT_B8G8R8A8_UNORM;
	case vfmt::b8g8r8a8_snorm:        return VK_FORMAT_B8G8R8A8_SNORM;
	case vfmt::b8g8r8a8_uscaled:      return VK_FORMAT_B8G8R8A8_USCALED;
	case vfmt::b8g8r8a8_sscaled:      return VK_FORMAT_B8G8R8A8_SSCALED;
	case vfmt::b8g8r8a8_uint:         return VK_FORMAT_B8G8R8A8_UINT;
	case vfmt::b8g8r8a8_sint:         return VK_FORMAT_B8G8R8A8_SINT;
	case vfmt::b8g8r8a8_srgb:         return VK_FORMAT_B8G8R8A8_SRGB;
	case vfmt::a8b8g8r8_unorm_pack32:      return VK_FORMAT_A8B8G8R8_UNORM_PACK32;
	case vfmt::a8b8g8r8_snorm_pack32:      return VK_FORMAT_A8B8G8R8_SNORM_PACK32;
	case vfmt::a8b8g8r8_uscaled_pack32:    return VK_FORMAT_A8B8G8R8_USCALED_PACK32;
	case vfmt::a8b8g8r8_sscaled_pack32:    return VK_FORMAT_A8B8G8R8_SSCALED_PACK32;
	case vfmt::a8b8g8r8_uint_pack32:       return VK_FORMAT_A8B8G8R8_UINT_PACK32;
	case vfmt::a8b8g8r8_sint_pack32:       return VK_FORMAT_A8B8G8R8_SINT_PACK32;
	case vfmt::a8b8g8r8_srgb_pack32:       return VK_FORMAT_A8B8G8R8_SRGB_PACK32;
	case vfmt::a2r10g10b10_unorm_pack32:   return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
	case vfmt::a2r10g10b10_snorm_pack32:   return VK_FORMAT_A2R10G10B10_SNORM_PACK32;
	case vfmt::a2r10g10b10_uscaled_pack32: return VK_FORMAT_A2R10G10B10_USCALED_PACK32;
	case vfmt::a2r10g10b10_sscaled_pack32: return VK_FORMAT_A2R10G10B10_SSCALED_PACK32;
	case vfmt::a2r10g10b10_uint_pack32:    return VK_FORMAT_A2R10G10B10_UINT_PACK32;
	case vfmt::a2r10g10b10_sint_pack32:    return VK_FORMAT_A2R10G10B10_SINT_PACK32;
	case vfmt::a2b10g10r10_unorm_pack32:   return VK_FORMAT_A2B10G10R10_UNORM_PACK32;
	case vfmt::a2b10g10r10_snorm_pack32:   return VK_FORMAT_A2B10G10R10_SNORM_PACK32;
	case vfmt::a2b10g10r10_uscaled_pack32: return VK_FORMAT_A2B10G10R10_USCALED_PACK32;
	case vfmt::a2b10g10r10_sscaled_pack32: return VK_FORMAT_A2B10G10R10_SSCALED_PACK32;
	case vfmt::a2b10g10r10_uint_pack32:    return VK_FORMAT_A2B10G10R10_UINT_PACK32;
	case vfmt::a2b10g10r10_sint_pack32:    return VK_FORMAT_A2B10G10R10_SINT_PACK32;
	case vfmt::r16_unorm:            return VK_FORMAT_R16_UNORM;
	case vfmt::r16_snorm:            return VK_FORMAT_R16_SNORM;
	case vfmt::r16_uscaled:          return VK_FORMAT_R16_USCALED;
	case vfmt::r16_sscaled:          return VK_FORMAT_R16_SSCALED;
	case vfmt::r16_uint:             return VK_FORMAT_R16_UINT;
	case vfmt::r16_sint:             return VK_FORMAT_R16_SINT;
	case vfmt::r16_sfloat:           return VK_FORMAT_R16_SFLOAT;
	case vfmt::r16g16_unorm:         return VK_FORMAT_R16G16_UNORM;
	case vfmt::r16g16_snorm:         return VK_FORMAT_R16G16_SNORM;
	case vfmt::r16g16_uscaled:       return VK_FORMAT_R16G16_USCALED;
	case vfmt::r16g16_sscaled:       return VK_FORMAT_R16G16_SSCALED;
	case vfmt::r16g16_uint:          return VK_FORMAT_R16G16_UINT;
	case vfmt::r16g16_sint:          return VK_FORMAT_R16G16_SINT;
	case vfmt::r16g16_sfloat:        return VK_FORMAT_R16G16_SFLOAT;
	case vfmt::r16g16b16_unorm:      return VK_FORMAT_R16G16B16_UNORM;
	case vfmt::r16g16b16_snorm:      return VK_FORMAT_R16G16B16_SNORM;
	case vfmt::r16g16b16_uscaled:    return VK_FORMAT_R16G16B16_USCALED;
	case vfmt::r16g16b16_sscaled:    return VK_FORMAT_R16G16B16_SSCALED;
	case vfmt::r16g16b16_uint:       return VK_FORMAT_R16G16B16_UINT;
	case vfmt::r16g16b16_sint:       return VK_FORMAT_R16G16B16_SINT;
	case vfmt::r16g16b16_sfloat:     return VK_FORMAT_R16G16B16_SFLOAT;
	case vfmt::r16g16b16a16_unorm:   return VK_FORMAT_R16G16B16A16_UNORM;
	case vfmt::r16g16b16a16_snorm:   return VK_FORMAT_R16G16B16A16_SNORM;
	case vfmt::r16g16b16a16_uscaled: return VK_FORMAT_R16G16B16A16_USCALED;
	case vfmt::r16g16b16a16_sscaled: return VK_FORMAT_R16G16B16A16_SSCALED;
	case vfmt::r16g16b16a16_uint:    return VK_FORMAT_R16G16B16A16_UINT;
	case vfmt::r16g16b16a16_sint:    return VK_FORMAT_R16G16B16A16_SINT;
	case vfmt::r16g16b16a16_sfloat:  return VK_FORMAT_R16G16B16A16_SFLOAT;
	case vfmt::r32_uint:                return VK_FORMAT_R32_UINT;
	case vfmt::r32_sint:                return VK_FORMAT_R32_SINT;
	case vfmt::r32_sfloat:              return VK_FORMAT_R32_SFLOAT;
	case vfmt::r32g32_uint:             return VK_FORMAT_R32G32_UINT;
	case vfmt::r32g32_sint:             return VK_FORMAT_R32G32_SINT;
	case vfmt::r32g32_sfloat:           return VK_FORMAT_R32G32_SFLOAT;
	case vfmt::r32g32b32_uint:          return VK_FORMAT_R32G32B32_UINT;
	case vfmt::r32g32b32_sint:          return VK_FORMAT_R32G32B32_SINT;
	case vfmt::r32g32b32_sfloat:        return VK_FORMAT_R32G32B32_SFLOAT;
	case vfmt::r32g32b32a32_uint:       return VK_FORMAT_R32G32B32A32_UINT;
	case vfmt::r32g32b32a32_sint:       return VK_FORMAT_R32G32B32A32_SINT;
	case vfmt::r32g32b32a32_sfloat:     return VK_FORMAT_R32G32B32A32_SFLOAT;
	case vfmt::r64_uint:                return VK_FORMAT_R64_UINT;
	case vfmt::r64_sint:                return VK_FORMAT_R64_SINT;
	case vfmt::r64_sfloat:              return VK_FORMAT_R64_SFLOAT;
	case vfmt::r64g64_uint:             return VK_FORMAT_R64G64_UINT;
	case vfmt::r64g64_sint:             return VK_FORMAT_R64G64_SINT;
	case vfmt::r64g64_sfloat:           return VK_FORMAT_R64G64_SFLOAT;
	case vfmt::r64g64b64_uint:          return VK_FORMAT_R64G64B64_UINT;
	case vfmt::r64g64b64_sint:          return VK_FORMAT_R64G64B64_SINT;
	case vfmt::r64g64b64_sfloat:        return VK_FORMAT_R64G64B64_SFLOAT;
	case vfmt::r64g64b64a64_uint:       return VK_FORMAT_R64G64B64A64_UINT;
	case vfmt::r64g64b64a64_sint:       return VK_FORMAT_R64G64B64A64_SINT;
	case vfmt::r64g64b64a64_sfloat:     return VK_FORMAT_R64G64B64A64_SFLOAT;
	case vfmt::b10g11r11_ufloat_pack32: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
	case vfmt::e5b9g9r9_ufloat_pack32:  return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
	case vfmt::d16_unorm:            return VK_FORMAT_D16_UNORM;
	case vfmt::x8_d24_unorm_pack32:  return VK_FORMAT_X8_D24_UNORM_PACK32;
	case vfmt::d32_sfloat:           return VK_FORMAT_D32_SFLOAT;
	case vfmt::s8_uint:              return VK_FORMAT_S8_UINT;
	case vfmt::d16_unorm_s8_uint:    return VK_FORMAT_D16_UNORM_S8_UINT;
	case vfmt::d24_unorm_s8_uint:    return VK_FORMAT_D24_UNORM_S8_UINT;
	case vfmt::d32_sfloat_s8_uint:   return VK_FORMAT_D32_SFLOAT_S8_UINT;
	case vfmt::bc1_rgb_unorm_block:  return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
	case vfmt::bc1_rgb_srgb_block:   return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
	case vfmt::bc1_rgba_unorm_block: return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
	case vfmt::bc1_rgba_srgb_block:  return VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
	case vfmt::bc2_unorm_block:      return VK_FORMAT_BC2_UNORM_BLOCK;
	case vfmt::bc2_srgb_block:       return VK_FORMAT_BC2_SRGB_BLOCK;
	case vfmt::bc3_unorm_block:      return VK_FORMAT_BC3_UNORM_BLOCK;
	case vfmt::bc3_srgb_block:       return VK_FORMAT_BC3_SRGB_BLOCK;
	case vfmt::bc4_unorm_block:      return VK_FORMAT_BC4_UNORM_BLOCK;
	case vfmt::bc4_snorm_block:      return VK_FORMAT_BC4_SNORM_BLOCK;
	case vfmt::bc5_unorm_block:      return VK_FORMAT_BC5_UNORM_BLOCK;
	case vfmt::bc5_snorm_block:      return VK_FORMAT_BC5_SNORM_BLOCK;
	case vfmt::bc6h_ufloat_block:    return VK_FORMAT_BC6H_UFLOAT_BLOCK;
	case vfmt::bc6h_sfloat_block:    return VK_FORMAT_BC6H_SFLOAT_BLOCK;
	case vfmt::bc7_unorm_block:      return VK_FORMAT_BC7_UNORM_BLOCK;
	case vfmt::bc7_srgb_block:       return VK_FORMAT_BC7_SRGB_BLOCK;
	case vfmt::etc2_r8g8b8_unorm_block:   return VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
	case vfmt::etc2_r8g8b8_srgb_block:    return VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
	case vfmt::etc2_r8g8b8a1_unorm_block: return VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK;
	case vfmt::etc2_r8g8b8a1_srgb_block:  return VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK;
	case vfmt::etc2_r8g8b8a8_unorm_block: return VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
	case vfmt::etc2_r8g8b8a8_srgb_block:  return VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
	case vfmt::eac_r11_unorm_block:       return VK_FORMAT_EAC_R11_UNORM_BLOCK;
	case vfmt::eac_r11_snorm_block:       return VK_FORMAT_EAC_R11_SNORM_BLOCK;
	case vfmt::eac_r11g11_unorm_block:    return VK_FORMAT_EAC_R11G11_UNORM_BLOCK;
	case vfmt::eac_r11g11_snorm_block:    return VK_FORMAT_EAC_R11G11_SNORM_BLOCK;
	case vfmt::astc_4x4_unorm_block:   return VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
	case vfmt::astc_4x4_srgb_block:    return VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
	case vfmt::astc_5x4_unorm_block:   return VK_FORMAT_ASTC_5x4_UNORM_BLOCK;
	case vfmt::astc_5x4_srgb_block:    return VK_FORMAT_ASTC_5x4_SRGB_BLOCK;
	case vfmt::astc_5x5_unorm_block:   return VK_FORMAT_ASTC_5x5_UNORM_BLOCK;
	case vfmt::astc_5x5_srgb_block:    return VK_FORMAT_ASTC_5x5_SRGB_BLOCK;
	case vfmt::astc_6x5_unorm_block:   return VK_FORMAT_ASTC_6x5_UNORM_BLOCK;
	case vfmt::astc_6x5_srgb_block:    return VK_FORMAT_ASTC_6x5_SRGB_BLOCK;
	case vfmt::astc_6x6_unorm_block:   return VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
	case vfmt::astc_6x6_srgb_block:    return VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
	case vfmt::astc_8x5_unorm_block:   return VK_FORMAT_ASTC_8x5_UNORM_BLOCK;
	case vfmt::astc_8x5_srgb_block:    return VK_FORMAT_ASTC_8x5_SRGB_BLOCK;
	case vfmt::astc_8x6_unorm_block:   return VK_FORMAT_ASTC_8x6_UNORM_BLOCK;
	case vfmt::astc_8x6_srgb_block:    return VK_FORMAT_ASTC_8x6_SRGB_BLOCK;
	case vfmt::astc_8x8_unorm_block:   return VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
	case vfmt::astc_8x8_srgb_block:    return VK_FORMAT_ASTC_8x8_SRGB_BLOCK;
	case vfmt::astc_10x5_unorm_block:  return VK_FORMAT_ASTC_10x5_UNORM_BLOCK;
	case vfmt::astc_10x5_srgb_block:   return VK_FORMAT_ASTC_10x5_SRGB_BLOCK;
	case vfmt::astc_10x6_unorm_block:  return VK_FORMAT_ASTC_10x6_UNORM_BLOCK;
	case vfmt::astc_10x6_srgb_block:   return VK_FORMAT_ASTC_10x6_SRGB_BLOCK;
	case vfmt::astc_10x8_unorm_block:  return VK_FORMAT_ASTC_10x8_UNORM_BLOCK;
	case vfmt::astc_10x8_srgb_block:   return VK_FORMAT_ASTC_10x8_SRGB_BLOCK;
	case vfmt::astc_10x10_unorm_block: return VK_FORMAT_ASTC_10x10_UNORM_BLOCK;
	case vfmt::astc_10x10_srgb_block:  return VK_FORMAT_ASTC_10x10_SRGB_BLOCK;
	case vfmt::astc_12x10_unorm_block: return VK_FORMAT_ASTC_12x10_UNORM_BLOCK;
	case vfmt::astc_12x10_srgb_block:  return VK_FORMAT_ASTC_12x10_SRGB_BLOCK;
	case vfmt::astc_12x12_unorm_block: return VK_FORMAT_ASTC_12x12_UNORM_BLOCK;
	case vfmt::astc_12x12_srgb_block:  return VK_FORMAT_ASTC_12x12_SRGB_BLOCK;

	// Provided by VK_VERSION_1_1
	case vfmt::g8b8g8r8_422_unorm:        return VK_FORMAT_G8B8G8R8_422_UNORM;
	case vfmt::b8g8r8g8_422_unorm:        return VK_FORMAT_B8G8R8G8_422_UNORM;
	case vfmt::g8_b8_r8_3plane_420_unorm: return VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM;
	case vfmt::g8_b8r8_2plane_420_unorm:  return VK_FORMAT_G8_B8R8_2PLANE_420_UNORM;
	case vfmt::g8_b8_r8_3plane_422_unorm: return VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM;
	case vfmt::g8_b8r8_2plane_422_unorm:  return VK_FORMAT_G8_B8R8_2PLANE_422_UNORM;
	case vfmt::g8_b8_r8_3plane_444_unorm: return VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM;
	case vfmt::r10x6_unorm_pack16:        return VK_FORMAT_R10X6_UNORM_PACK16;
	case vfmt::r10x6g10x6_unorm_2pack16:  return VK_FORMAT_R10X6G10X6_UNORM_2PACK16;
	case vfmt::r10x6g10x6b10x6a10x6_unorm_4pack16:
		return VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16;
	case vfmt::g10x6b10x6g10x6r10x6_422_unorm_4pack16:
		return VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16;
	case vfmt::b10x6g10x6r10x6g10x6_422_unorm_4pack16:
		return VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16;
	case vfmt::g10x6_b10x6_r10x6_3plane_420_unorm_3pack16:
		return VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16;
	case vfmt::g10x6_b10x6r10x6_2plane_420_unorm_3pack16:
		return VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16;
	case vfmt::g10x6_b10x6_r10x6_3plane_422_unorm_3pack16:
		return VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16;
	case vfmt::g10x6_b10x6r10x6_2plane_422_unorm_3pack16:
		return VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16;
	case vfmt::g10x6_b10x6_r10x6_3plane_444_unorm_3pack16:
		return VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16;
	case vfmt::r12x4_unorm_pack16:       return VK_FORMAT_R12X4_UNORM_PACK16;
	case vfmt::r12x4g12x4_unorm_2pack16: return VK_FORMAT_R12X4G12X4_UNORM_2PACK16;
	case vfmt::r12x4g12x4b12x4a12x4_unorm_4pack16:
		return VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16;
	case vfmt::g12x4b12x4g12x4r12x4_422_unorm_4pack16:
		return VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16;
	case vfmt::b12x4g12x4r12x4g12x4_422_unorm_4pack16:
		return VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16;
	case vfmt::g12x4_b12x4_r12x4_3plane_420_unorm_3pack16:
		return VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16;
	case vfmt::g12x4_b12x4r12x4_2plane_420_unorm_3pack16:
		return VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16;
	case vfmt::g12x4_b12x4_r12x4_3plane_422_unorm_3pack16:
		return VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16;
	case vfmt::g12x4_b12x4r12x4_2plane_422_unorm_3pack16:
		return VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16;
	case vfmt::g12x4_b12x4_r12x4_3plane_444_unorm_3pack16:
		return VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16;
	case vfmt::g16b16g16r16_422_unorm:       return VK_FORMAT_G16B16G16R16_422_UNORM;
	case vfmt::b16g16r16g16_422_unorm:       return VK_FORMAT_B16G16R16G16_422_UNORM;
	case vfmt::g16_b16_r16_3plane_420_unorm: return VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM;
	case vfmt::g16_b16r16_2plane_420_unorm:  return VK_FORMAT_G16_B16R16_2PLANE_420_UNORM;
	case vfmt::g16_b16_r16_3plane_422_unorm: return VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM;
	case vfmt::g16_b16r16_2plane_422_unorm:  return VK_FORMAT_G16_B16R16_2PLANE_422_UNORM;
	case vfmt::g16_b16_r16_3plane_444_unorm: return VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM;

	// Provided by VK_VERSION_1_3
	case vfmt::g8_b8r8_2plane_444_unorm:    return VK_FORMAT_G8_B8R8_2PLANE_444_UNORM;
	case vfmt::g10x6_b10x6r10x6_2plane_444_unorm_3pack16:
		return VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16;
	case vfmt::g12x4_b12x4r12x4_2plane_444_unorm_3pack16:
		return VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16;
	case vfmt::g16_b16r16_2plane_444_unorm: return VK_FORMAT_G16_B16R16_2PLANE_444_UNORM;
	case vfmt::a4r4g4b4_unorm_pack16:       return VK_FORMAT_A4R4G4B4_UNORM_PACK16;
	case vfmt::a4b4g4r4_unorm_pack16:       return VK_FORMAT_A4B4G4R4_UNORM_PACK16;
	case vfmt::astc_4x4_sfloat_block:       return VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK;
	case vfmt::astc_5x4_sfloat_block:       return VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK;
	case vfmt::astc_5x5_sfloat_block:       return VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK;
	case vfmt::astc_6x5_sfloat_block:       return VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK;
	case vfmt::astc_6x6_sfloat_block:       return VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK;
	case vfmt::astc_8x5_sfloat_block:       return VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK;
	case vfmt::astc_8x6_sfloat_block:       return VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK;
	case vfmt::astc_8x8_sfloat_block:       return VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK;
	case vfmt::astc_10x5_sfloat_block:      return VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK;
	case vfmt::astc_10x6_sfloat_block:      return VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK;
	case vfmt::astc_10x8_sfloat_block:      return VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK;
	case vfmt::astc_10x10_sfloat_block:     return VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK;
	case vfmt::astc_12x10_sfloat_block:     return VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK;
	case vfmt::astc_12x12_sfloat_block:     return VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK;
	default:
		vtek_log_error("vtek_format_support.cpp: unrecognized format enum!");
		return VK_FORMAT_UNDEFINED;
	}
}

static std::string_view get_format_string_helper(vtek::Format format)
{
	switch (format)
	{

	case vfmt::undefined:                 return "undefined";
	case vfmt::r4g4_unorm_pack8:          return "r4g4_unorm_pack8";
	case vfmt::r4g4b4a4_unorm_pack16:     return "r4g4b4a4_unorm_pack16";
	case vfmt::b4g4r4a4_unorm_pack16:     return "b4g4r4a4_unorm_pack16";
	case vfmt::r5g6b5_unorm_pack16:       return "r5g6b5_unorm_pack16";
	case vfmt::b5g6r5_unorm_pack16:       return "b5g6r5_unorm_pack16";
	case vfmt::r5g5b5a1_unorm_pack16:     return "r5g5b5a1_unorm_pack16";
	case vfmt::b5g5r5a1_unorm_pack16:     return "b5g5r5a1_unorm_pack16";
	case vfmt::a1r5g5b5_unorm_pack16:     return "a1r5g5b5_unorm_pack16";
	case vfmt::r8_unorm:                  return "r8_unorm";
	case vfmt::r8_snorm:                  return "r8_snorm";
	case vfmt::r8_uscaled:                return "r8_uscaled";
	case vfmt::r8_sscaled:                return "r8_sscaled";
	case vfmt::r8_uint:                   return "r8_uint";
	case vfmt::r8_sint:                   return "r8_sint";
	case vfmt::r8_srgb:                   return "r8_srgb";
	case vfmt::r8g8_unorm:                return "r8g8_unorm";
	case vfmt::r8g8_snorm:                return "r8g8_snorm";
	case vfmt::r8g8_uscaled:              return "r8g8_uscaled";
	case vfmt::r8g8_sscaled:              return "r8g8_sscaled";
	case vfmt::r8g8_uint:                 return "r8g8_uint";
	case vfmt::r8g8_sint:                 return "r8g8_sint";
	case vfmt::r8g8_srgb:                 return "r8g8_srgb";
	case vfmt::r8g8b8_unorm:              return "r8g8b8_unorm";
	case vfmt::r8g8b8_snorm:              return "r8g8b8_snorm";
	case vfmt::r8g8b8_uscaled:            return "r8g8b8_uscaled";
	case vfmt::r8g8b8_sscaled:            return "r8g8b8_sscaled";
	case vfmt::r8g8b8_uint:               return "r8g8b8_uint";
	case vfmt::r8g8b8_sint:               return "r8g8b8_sint";
	case vfmt::r8g8b8_srgb:               return "r8g8b8_srgb";
	case vfmt::b8g8r8_unorm:              return "b8g8r8_unorm";
	case vfmt::b8g8r8_snorm:              return "b8g8r8_snorm";
	case vfmt::b8g8r8_uscaled:            return "b8g8r8_uscaled";
	case vfmt::b8g8r8_sscaled:            return "b8g8r8_sscaled";
	case vfmt::b8g8r8_uint:               return "b8g8r8_uint";
	case vfmt::b8g8r8_sint:               return "b8g8r8_sint";
	case vfmt::b8g8r8_srgb:               return "b8g8r8_srgb";
	case vfmt::r8g8b8a8_unorm:            return "r8g8b8a8_unorm";
	case vfmt::r8g8b8a8_snorm:            return "r8g8b8a8_snorm";
	case vfmt::r8g8b8a8_uscaled:          return "r8g8b8a8_uscaled";
	case vfmt::r8g8b8a8_sscaled:          return "r8g8b8a8_sscaled";
	case vfmt::r8g8b8a8_uint:             return "r8g8b8a8_uint";
	case vfmt::r8g8b8a8_sint:             return "r8g8b8a8_sint";
	case vfmt::r8g8b8a8_srgb:             return "r8g8b8a8_srgb";
	case vfmt::b8g8r8a8_unorm:            return "b8g8r8a8_unorm";
	case vfmt::b8g8r8a8_snorm:            return "b8g8r8a8_snorm";
	case vfmt::b8g8r8a8_uscaled:          return "b8g8r8a8_uscaled";
	case vfmt::b8g8r8a8_sscaled:          return "b8g8r8a8_sscaled";
	case vfmt::b8g8r8a8_uint:             return "b8g8r8a8_uint";
	case vfmt::b8g8r8a8_sint:             return "b8g8r8a8_sint";
	case vfmt::b8g8r8a8_srgb:             return "b8g8r8a8_srgb";
	case vfmt::a8b8g8r8_unorm_pack32:    return "a8b8g8r8_unorm_pack32";
	case vfmt::a8b8g8r8_snorm_pack32:    return "a8b8g8r8_snorm_pack32";
	case vfmt::a8b8g8r8_uscaled_pack32:  return "a8b8g8r8_uscaled_pack32";
	case vfmt::a8b8g8r8_sscaled_pack32:  return "a8b8g8r8_sscaled_pack32";
	case vfmt::a8b8g8r8_uint_pack32:     return "a8b8g8r8_uint_pack32";
	case vfmt::a8b8g8r8_sint_pack32:     return "a8b8g8r8_sint_pack32";
	case vfmt::a8b8g8r8_srgb_pack32:     return "a8b8g8r8_srgb_pack32";
	case vfmt::a2r10g10b10_unorm_pack32: return "a2r10g10b10_unorm_pack32";
	case vfmt::a2r10g10b10_snorm_pack32: return "a2r10g10b10_snorm_pack32";
	case vfmt::a2r10g10b10_uscaled_pack32: return "a2r10g10b10_uscaled_pack32";
	case vfmt::a2r10g10b10_sscaled_pack32: return "a2r10g10b10_sscaled_pack32";
	case vfmt::a2r10g10b10_uint_pack32:  return "a2r10g10b10_uint_pack32";
	case vfmt::a2r10g10b10_sint_pack32:  return "a2r10g10b10_sint_pack32";
	case vfmt::a2b10g10r10_unorm_pack32: return "a2b10g10r10_unorm_pack32";
	case vfmt::a2b10g10r10_snorm_pack32: return "a2b10g10r10_snorm_pack32";
	case vfmt::a2b10g10r10_uscaled_pack32: return "a2b10g10r10_uscaled_pack32";
	case vfmt::a2b10g10r10_sscaled_pack32: return "a2b10g10r10_sscaled_pack32";
	case vfmt::a2b10g10r10_uint_pack32:  return "a2b10g10r10_uint_pack32";
	case vfmt::a2b10g10r10_sint_pack32:  return "a2b10g10r10_sint_pack32";
	case vfmt::r16_unorm:                return "r16_unorm";
	case vfmt::r16_snorm:                return "r16_snorm";
	case vfmt::r16_uscaled:              return "r16_uscaled";
	case vfmt::r16_sscaled:              return "r16_sscaled";
	case vfmt::r16_uint:                 return "r16_uint";
	case vfmt::r16_sint:                 return "r16_sint";
	case vfmt::r16_sfloat:               return "r16_sfloat";
	case vfmt::r16g16_unorm:             return "r16g16_unorm";
	case vfmt::r16g16_snorm:             return "r16g16_snorm";
	case vfmt::r16g16_uscaled:           return "r16g16_uscaled";
	case vfmt::r16g16_sscaled:           return "r16g16_sscaled";
	case vfmt::r16g16_uint:              return "r16g16_uint";
	case vfmt::r16g16_sint:              return "r16g16_sint";
	case vfmt::r16g16_sfloat:            return "r16g16_sfloat";
	case vfmt::r16g16b16_unorm:          return "r16g16b16_unorm";
	case vfmt::r16g16b16_snorm:          return "r16g16b16_snorm";
	case vfmt::r16g16b16_uscaled:        return "r16g16b16_uscaled";
	case vfmt::r16g16b16_sscaled:        return "r16g16b16_sscaled";
	case vfmt::r16g16b16_uint:           return "r16g16b16_uint";
	case vfmt::r16g16b16_sint:           return "r16g16b16_sint";
	case vfmt::r16g16b16_sfloat:         return "r16g16b16_sfloat";
	case vfmt::r16g16b16a16_unorm:       return "r16g16b16a16_unorm";
	case vfmt::r16g16b16a16_snorm:       return "r16g16b16a16_snorm";
	case vfmt::r16g16b16a16_uscaled:     return "r16g16b16a16_uscaled";
	case vfmt::r16g16b16a16_sscaled:     return "r16g16b16a16_sscaled";
	case vfmt::r16g16b16a16_uint:        return "r16g16b16a16_uint";
	case vfmt::r16g16b16a16_sint:        return "r16g16b16a16_sint";
	case vfmt::r16g16b16a16_sfloat:      return "r16g16b16a16_sfloat";
	case vfmt::r32_uint:                 return "r32_uint";
	case vfmt::r32_sint:                 return "r32_sint";
	case vfmt::r32_sfloat:               return "r32_sfloat";
	case vfmt::r32g32_uint:              return "r32g32_uint";
	case vfmt::r32g32_sint:              return "r32g32_sint";
	case vfmt::r32g32_sfloat:            return "r32g32_sfloat";
	case vfmt::r32g32b32_uint:           return "r32g32b32_uint";
	case vfmt::r32g32b32_sint:           return "r32g32b32_sint";
	case vfmt::r32g32b32_sfloat:         return "r32g32b32_sfloat";
	case vfmt::r32g32b32a32_uint:        return "r32g32b32a32_uint";
	case vfmt::r32g32b32a32_sint:        return "r32g32b32a32_sint";
	case vfmt::r32g32b32a32_sfloat:      return "r32g32b32a32_sfloat";
	case vfmt::r64_uint:                 return "r64_uint";
	case vfmt::r64_sint:                 return "r64_sint";
	case vfmt::r64_sfloat:               return "r64_sfloat";
	case vfmt::r64g64_uint:              return "r64g64_uint";
	case vfmt::r64g64_sint:              return "r64g64_sint";
	case vfmt::r64g64_sfloat:            return "r64g64_sfloat";
	case vfmt::r64g64b64_uint:           return "r64g64b64_uint";
	case vfmt::r64g64b64_sint:           return "r64g64b64_sint";
	case vfmt::r64g64b64_sfloat:         return "r64g64b64_sfloat";
	case vfmt::r64g64b64a64_uint:        return "r64g64b64a64_uint";
	case vfmt::r64g64b64a64_sint:        return "r64g64b64a64_sint";
	case vfmt::r64g64b64a64_sfloat:     return "r64g64b64a64_sfloat";
	case vfmt::b10g11r11_ufloat_pack32: return "b10g11r11_ufloat_pack32";
	case vfmt::e5b9g9r9_ufloat_pack32:  return "e5b9g9r9_ufloat_pack32";
	case vfmt::d16_unorm:            return "d16_unorm";
	case vfmt::x8_d24_unorm_pack32:  return "x8_d24_unorm_pack32";
	case vfmt::d32_sfloat:           return "d32_sfloat";
	case vfmt::s8_uint:              return "s8_uint";
	case vfmt::d16_unorm_s8_uint:    return "d16_unorm_s8_uint";
	case vfmt::d24_unorm_s8_uint:    return "d24_unorm_s8_uint";
	case vfmt::d32_sfloat_s8_uint:   return "d32_sfloat_s8_uint";
	case vfmt::bc1_rgb_unorm_block:  return "bc1_rgb_unorm_block";
	case vfmt::bc1_rgb_srgb_block:   return "bc1_rgb_srgb_block";
	case vfmt::bc1_rgba_unorm_block: return "bc1_rgba_unorm_block";
	case vfmt::bc1_rgba_srgb_block:  return "bc1_rgba_srgb_block";
	case vfmt::bc2_unorm_block:      return "bc2_unorm_block";
	case vfmt::bc2_srgb_block:       return "bc2_srgb_block";
	case vfmt::bc3_unorm_block:      return "bc3_unorm_block";
	case vfmt::bc3_srgb_block:       return "bc3_srgb_block";
	case vfmt::bc4_unorm_block:      return "bc4_unorm_block";
	case vfmt::bc4_snorm_block:      return "bc4_snorm_block";
	case vfmt::bc5_unorm_block:      return "bc5_unorm_block";
	case vfmt::bc5_snorm_block:      return "bc5_snorm_block";
	case vfmt::bc6h_ufloat_block:    return "bc6h_ufloat_block";
	case vfmt::bc6h_sfloat_block:    return "bc6h_sfloat_block";
	case vfmt::bc7_unorm_block:      return "bc7_unorm_block";
	case vfmt::bc7_srgb_block:       return "bc7_srgb_block";
	case vfmt::etc2_r8g8b8_unorm_block:   return "etc2_r8g8b8_unorm_block";
	case vfmt::etc2_r8g8b8_srgb_block:    return "etc2_r8g8b8_srgb_block";
	case vfmt::etc2_r8g8b8a1_unorm_block: return "etc2_r8g8b8a1_unorm_block";
	case vfmt::etc2_r8g8b8a1_srgb_block:  return "etc2_r8g8b8a1_srgb_block";
	case vfmt::etc2_r8g8b8a8_unorm_block: return "etc2_r8g8b8a8_unorm_block";
	case vfmt::etc2_r8g8b8a8_srgb_block:  return "etc2_r8g8b8a8_srgb_block";
	case vfmt::eac_r11_unorm_block:       return "eac_r11_unorm_block";
	case vfmt::eac_r11_snorm_block:       return "eac_r11_snorm_block";
	case vfmt::eac_r11g11_unorm_block:    return "eac_r11g11_unorm_block";
	case vfmt::eac_r11g11_snorm_block:    return "eac_r11g11_snorm_block";
	case vfmt::astc_4x4_unorm_block:   return "astc_4x4_unorm_block";
	case vfmt::astc_4x4_srgb_block:    return "astc_4x4_srgb_block";
	case vfmt::astc_5x4_unorm_block:   return "astc_5x4_unorm_block";
	case vfmt::astc_5x4_srgb_block:    return "astc_5x4_srgb_block";
	case vfmt::astc_5x5_unorm_block:   return "astc_5x5_unorm_block";
	case vfmt::astc_5x5_srgb_block:    return "astc_5x5_srgb_block";
	case vfmt::astc_6x5_unorm_block:   return "astc_6x5_unorm_block";
	case vfmt::astc_6x5_srgb_block:    return "astc_6x5_srgb_block";
	case vfmt::astc_6x6_unorm_block:   return "astc_6x6_unorm_block";
	case vfmt::astc_6x6_srgb_block:    return "astc_6x6_srgb_block";
	case vfmt::astc_8x5_unorm_block:   return "astc_8x5_unorm_block";
	case vfmt::astc_8x5_srgb_block:    return "astc_8x5_srgb_block";
	case vfmt::astc_8x6_unorm_block:   return "astc_8x6_unorm_block";
	case vfmt::astc_8x6_srgb_block:    return "astc_8x6_srgb_block";
	case vfmt::astc_8x8_unorm_block:   return "astc_8x8_unorm_block";
	case vfmt::astc_8x8_srgb_block:    return "astc_8x8_srgb_block";
	case vfmt::astc_10x5_unorm_block:  return "astc_10x5_unorm_block";
	case vfmt::astc_10x5_srgb_block:   return "astc_10x5_srgb_block";
	case vfmt::astc_10x6_unorm_block:  return "astc_10x6_unorm_block";
	case vfmt::astc_10x6_srgb_block:   return "astc_10x6_srgb_block";
	case vfmt::astc_10x8_unorm_block:  return "astc_10x8_unorm_block";
	case vfmt::astc_10x8_srgb_block:   return "astc_10x8_srgb_block";
	case vfmt::astc_10x10_unorm_block: return "astc_10x10_unorm_block";
	case vfmt::astc_10x10_srgb_block:  return "astc_10x10_srgb_block";
	case vfmt::astc_12x10_unorm_block: return "astc_12x10_unorm_block";
	case vfmt::astc_12x10_srgb_block:  return "astc_12x10_srgb_block";
	case vfmt::astc_12x12_unorm_block: return "astc_12x12_unorm_block";
	case vfmt::astc_12x12_srgb_block:  return "astc_12x12_srgb_block";

	// Provided by VK_VERSION_1_1
	case vfmt::g8b8g8r8_422_unorm:        return "g8b8g8r8_422_unorm";
	case vfmt::b8g8r8g8_422_unorm:        return "b8g8r8g8_422_unorm";
	case vfmt::g8_b8_r8_3plane_420_unorm: return "g8_b8_r8_3plane_420_unorm";
	case vfmt::g8_b8r8_2plane_420_unorm:  return "g8_b8r8_2plane_420_unorm";
	case vfmt::g8_b8_r8_3plane_422_unorm: return "g8_b8_r8_3plane_422_unorm";
	case vfmt::g8_b8r8_2plane_422_unorm:  return "g8_b8r8_2plane_422_unorm";
	case vfmt::g8_b8_r8_3plane_444_unorm: return "g8_b8_r8_3plane_444_unorm";
	case vfmt::r10x6_unorm_pack16:        return "r10x6_unorm_pack16";
	case vfmt::r10x6g10x6_unorm_2pack16:  return "r10x6g10x6_unorm_2pack16";
	case vfmt::r10x6g10x6b10x6a10x6_unorm_4pack16: return "r10x6g10x6b10x6a10x6_unorm_4pack16";
	case vfmt::g10x6b10x6g10x6r10x6_422_unorm_4pack16: return "g10x6b10x6g10x6r10x6_422_unorm_4pack16";
	case vfmt::b10x6g10x6r10x6g10x6_422_unorm_4pack16: return "b10x6g10x6r10x6g10x6_422_unorm_4pack16";
	case vfmt::g10x6_b10x6_r10x6_3plane_420_unorm_3pack16: return "g10x6_b10x6_r10x6_3plane_420_unorm_3pack16";
	case vfmt::g10x6_b10x6r10x6_2plane_420_unorm_3pack16: return "g10x6_b10x6r10x6_2plane_420_unorm_3pack16";
	case vfmt::g10x6_b10x6_r10x6_3plane_422_unorm_3pack16: return "g10x6_b10x6_r10x6_3plane_422_unorm_3pack16";
	case vfmt::g10x6_b10x6r10x6_2plane_422_unorm_3pack16: return "g10x6_b10x6r10x6_2plane_422_unorm_3pack16";
	case vfmt::g10x6_b10x6_r10x6_3plane_444_unorm_3pack16: return "g10x6_b10x6_r10x6_3plane_444_unorm_3pack16";
	case vfmt::r12x4_unorm_pack16:       return "r12x4_unorm_pack16";
	case vfmt::r12x4g12x4_unorm_2pack16: return "r12x4g12x4_unorm_2pack16";
	case vfmt::r12x4g12x4b12x4a12x4_unorm_4pack16: return "r12x4g12x4b12x4a12x4_unorm_4pack16";
	case vfmt::g12x4b12x4g12x4r12x4_422_unorm_4pack16: return "g12x4b12x4g12x4r12x4_422_unorm_4pack16";
	case vfmt::b12x4g12x4r12x4g12x4_422_unorm_4pack16: return "b12x4g12x4r12x4g12x4_422_unorm_4pack16";
	case vfmt::g12x4_b12x4_r12x4_3plane_420_unorm_3pack16: return "g12x4_b12x4_r12x4_3plane_420_unorm_3pack16";
	case vfmt::g12x4_b12x4r12x4_2plane_420_unorm_3pack16: return "g12x4_b12x4r12x4_2plane_420_unorm_3pack16";
	case vfmt::g12x4_b12x4_r12x4_3plane_422_unorm_3pack16: return "g12x4_b12x4_r12x4_3plane_422_unorm_3pack16";
	case vfmt::g12x4_b12x4r12x4_2plane_422_unorm_3pack16: return "g12x4_b12x4r12x4_2plane_422_unorm_3pack16";
	case vfmt::g12x4_b12x4_r12x4_3plane_444_unorm_3pack16: return "g12x4_b12x4_r12x4_3plane_444_unorm_3pack16";
	case vfmt::g16b16g16r16_422_unorm:       return "g16b16g16r16_422_unorm";
	case vfmt::b16g16r16g16_422_unorm:       return "b16g16r16g16_422_unorm";
	case vfmt::g16_b16_r16_3plane_420_unorm: return "g16_b16_r16_3plane_420_unorm";
	case vfmt::g16_b16r16_2plane_420_unorm:  return "g16_b16r16_2plane_420_unorm";
	case vfmt::g16_b16_r16_3plane_422_unorm: return "g16_b16_r16_3plane_422_unorm";
	case vfmt::g16_b16r16_2plane_422_unorm:  return "g16_b16r16_2plane_422_unorm";
	case vfmt::g16_b16_r16_3plane_444_unorm: return "g16_b16_r16_3plane_444_unorm";

	// Provided by VK_VERSION_1_3
	case vfmt::g8_b8r8_2plane_444_unorm:    return "g8_b8r8_2plane_444_unorm";
	case vfmt::g10x6_b10x6r10x6_2plane_444_unorm_3pack16: return "g10x6_b10x6r10x6_2plane_444_unorm_3pack16";
	case vfmt::g12x4_b12x4r12x4_2plane_444_unorm_3pack16: return "g12x4_b12x4r12x4_2plane_444_unorm_3pack16";
	case vfmt::g16_b16r16_2plane_444_unorm: return "g16_b16r16_2plane_444_unorm";
	case vfmt::a4r4g4b4_unorm_pack16:       return "a4r4g4b4_unorm_pack16";
	case vfmt::a4b4g4r4_unorm_pack16:       return "a4b4g4r4_unorm_pack16";
	case vfmt::astc_4x4_sfloat_block:       return "astc_4x4_sfloat_block";
	case vfmt::astc_5x4_sfloat_block:       return "astc_5x4_sfloat_block";
	case vfmt::astc_5x5_sfloat_block:       return "astc_5x5_sfloat_block";
	case vfmt::astc_6x5_sfloat_block:       return "astc_6x5_sfloat_block";
	case vfmt::astc_6x6_sfloat_block:       return "astc_6x6_sfloat_block";
	case vfmt::astc_8x5_sfloat_block:       return "astc_8x5_sfloat_block";
	case vfmt::astc_8x6_sfloat_block:       return "astc_8x6_sfloat_block";
	case vfmt::astc_8x8_sfloat_block:       return "astc_8x8_sfloat_block";
	case vfmt::astc_10x5_sfloat_block:      return "astc_10x5_sfloat_block";
	case vfmt::astc_10x6_sfloat_block:      return "astc_10x6_sfloat_block";
	case vfmt::astc_10x8_sfloat_block:      return "astc_10x8_sfloat_block";
	case vfmt::astc_10x10_sfloat_block:     return "astc_10x10_sfloat_block";
	case vfmt::astc_12x10_sfloat_block:     return "astc_12x10_sfloat_block";
	case vfmt::astc_12x12_sfloat_block:     return "astc_12x12_sfloat_block";

	default:
		vtek_log_error("vtek_format_support.cpp: unrecognized format enum!");
		return "undefined";
	}
}

using FCSize = vtek::FormatChannelSize;
using SType = vtek::FormatStorageType;
using FCompr = vtek::FormatCompression;

struct FormatDetails
{
	// Default settings which covers most formats
	uint8_t channels {4};
	FCSize channelSize {FCSize::channel_8};
	bool alpha {false};
	bool sRGB {false};
	vtek::FormatCompression compression {vtek::FormatCompression::none};
	bool blueEndian {false};
	bool alphaFirst {false};
	bool depth {false};
	bool stencil {false};
	SType storageType {SType::unorm};
	bool planar {false};
};

static void get_format_details(vtek::Format format, FormatDetails* details)
{
	switch (format)
	{
	case vfmt::undefined:             return;

	case vfmt::r4g4_unorm_pack8:
		details->channels = 2; details->channelSize = FCSize::channel_4;
		details->storageType = SType::unorm_pack8;
		return;

	case vfmt::r4g4b4a4_unorm_pack16:
		details->alpha = true; details->storageType = SType::unorm_pack16;
		details->channelSize = FCSize::channel_4;
		return;

	case vfmt::b4g4r4a4_unorm_pack16:
		details->alpha = true; details->blueEndian = true;
		details->storageType = SType::unorm_pack16;
		details->channelSize = FCSize::channel_4;
		return;

	case vfmt::r5g6b5_unorm_pack16:
		details->channels = 3; details->storageType = SType::unorm_pack16;
		details->channelSize = FCSize::special;
		return;

	case vfmt::b5g6r5_unorm_pack16:
		details->channels = 3; details->blueEndian = true;
		details->storageType = SType::unorm_pack16;
		details->channelSize = FCSize::special;
		return;

	case vfmt::r5g5b5a1_unorm_pack16:
		details->alpha = true; details->storageType = SType::unorm_pack16;
		details->channelSize = FCSize::special;
		return;

	case vfmt::b5g5r5a1_unorm_pack16:
		details->alpha = true; details->blueEndian = true;
		details->channelSize = FCSize::special;
		details->storageType = SType::unorm_pack16;
		return;

	case vfmt::a1r5g5b5_unorm_pack16:
		details->alpha = true; details->alphaFirst = true;
		details->storageType = SType::unorm_pack16;
		details->channelSize = FCSize::special;
		return;

	case vfmt::r8_unorm:
		details->channels = 1;
		return;

	case vfmt::r8_snorm:
		details->channels = 1; details->storageType = SType::snorm;
		return;

	case vfmt::r8_uscaled:
		details->channels = 1; details->storageType = SType::uscaled;
		return;

	case vfmt::r8_sscaled:
		details->channels = 1; details->storageType = SType::sscaled;
		return;

	case vfmt::r8_uint:
		details->channels = 1; details->storageType = SType::uint;
		return;

	case vfmt::r8_sint:
		details->channels = 1; details->storageType = SType::sint;
		return;

	case vfmt::r8_srgb:
		details->channels = 1; details->sRGB = true;
		details->storageType = SType::srgb;
		return;

	case vfmt::r8g8_unorm:
		details->channels = 2;
		return;

	case vfmt::r8g8_snorm:
		details->channels = 2; details->storageType = SType::snorm;
		return;

	case vfmt::r8g8_uscaled:
		details->channels = 2; details->storageType = SType::uscaled;
		return;

	case vfmt::r8g8_sscaled:
		details->channels = 2; details->storageType = SType::sscaled;
		return;

	case vfmt::r8g8_uint:
		details->channels = 2; details->storageType = SType::uint;
		return;

	case vfmt::r8g8_sint:
		details->channels = 2; details->storageType = SType::sint;
		return;

	case vfmt::r8g8_srgb:
		details->channels = 2; details->storageType = SType::srgb;
		details->sRGB = true;
		return;

	case vfmt::r8g8b8_unorm:
		details->channels = 3;
		return;

	case vfmt::r8g8b8_snorm:
		details->channels = 3; details->storageType = SType::snorm;
		return;

	case vfmt::r8g8b8_uscaled:
		details->channels = 3; details->storageType = SType::uscaled;
		return;

	case vfmt::r8g8b8_sscaled:
		details->channels = 3; details->storageType = SType::sscaled;
		return;

	case vfmt::r8g8b8_uint:
		details->channels = 3; details->storageType = SType::uint;
		return;

	case vfmt::r8g8b8_sint:
		details->channels = 3; details->storageType = SType::sint;
		return;

	case vfmt::r8g8b8_srgb:
		details->channels = 3; details->storageType = SType::srgb;
		details->sRGB = true;
		return;

	case vfmt::b8g8r8_unorm:
		details->channels = 3; details->blueEndian = true;
		return;

	case vfmt::b8g8r8_snorm:
		details->channels = 3; details->blueEndian = true;
		details->storageType = SType::snorm;
		return;

	case vfmt::b8g8r8_uscaled:
		details->channels = 3; details->blueEndian = true;
		details->storageType = SType::uscaled;
		return;

	case vfmt::b8g8r8_sscaled:
		details->channels = 3; details->blueEndian = true;
		details->storageType = SType::sscaled;
		return;

	case vfmt::b8g8r8_uint:
		details->channels = 3; details->blueEndian = true;
		details->storageType = SType::uint;
		return;

	case vfmt::b8g8r8_sint:
		details->channels = 3; details->blueEndian = true;
		details->storageType = SType::sint;
		return;

	case vfmt::b8g8r8_srgb:
		details->channels = 3; details->blueEndian = true;
		details->storageType = SType::srgb; details->sRGB = true;
		return;

	case vfmt::r8g8b8a8_unorm:
		details->alpha = true;
		return;

	case vfmt::r8g8b8a8_snorm:
		details->alpha = true; details->storageType = SType::snorm;
		return;

	case vfmt::r8g8b8a8_uscaled:
		details->alpha = true; details->storageType = SType::uscaled;
		return;

	case vfmt::r8g8b8a8_sscaled:
		details->alpha = true; details->storageType = SType::sscaled;
		return;

	case vfmt::r8g8b8a8_uint:
		details->alpha = true; details->storageType = SType::uint;
		return;

	case vfmt::r8g8b8a8_sint:
		details->alpha = true; details->storageType = SType::sint;
		return;

	case vfmt::r8g8b8a8_srgb:
		details->alpha = true;
		details->sRGB = true; details->storageType = SType::srgb;
		return;

	case vfmt::b8g8r8a8_unorm:
		details->alpha = true; details->blueEndian = true;
		return;

	case vfmt::b8g8r8a8_snorm:
		details->alpha = true; details->blueEndian = true;
		details->storageType = SType::snorm;
		return;

	case vfmt::b8g8r8a8_uscaled:
		details->alpha = true; details->blueEndian = true;
		details->storageType = SType::uscaled;
		return;

	case vfmt::b8g8r8a8_sscaled:
		details->alpha = true; details->blueEndian = true;
		details->storageType = SType::sscaled;
		return;

	case vfmt::b8g8r8a8_uint:
		details->alpha = true; details->blueEndian = true;
		details->storageType = SType::uint;
		return;

	case vfmt::b8g8r8a8_sint:
		details->alpha = true; details->blueEndian = true;
		details->storageType = SType::sint;
		return;

	case vfmt::b8g8r8a8_srgb:
		details->alpha = true; details->blueEndian = true;
		details->sRGB = true; details->storageType = SType::srgb;
		return;

	case vfmt::a8b8g8r8_unorm_pack32:
		details->alpha = true; details->alphaFirst = true;
		details->blueEndian = true; details->storageType = SType::unorm_pack32;
		return;

	case vfmt::a8b8g8r8_snorm_pack32:
		details->alpha = true; details->alphaFirst = true;
		details->blueEndian = true; details->storageType = SType::snorm_pack32;
		return;

	case vfmt::a8b8g8r8_uscaled_pack32:
		details->alpha = true; details->alphaFirst = true;
		details->blueEndian = true; details->storageType = SType::uscaled_pack32;
		return;

	case vfmt::a8b8g8r8_sscaled_pack32:
		details->alpha = true; details->alphaFirst = true;
		details->blueEndian = true; details->storageType = SType::sscaled_pack32;
		return;

	case vfmt::a8b8g8r8_uint_pack32:
		details->alpha = true; details->alphaFirst = true;
		details->blueEndian = true; details->storageType = SType::uint_pack32;
		return;

	case vfmt::a8b8g8r8_sint_pack32:
		details->alpha = true; details->alphaFirst = true;
		details->blueEndian = true; details->storageType = SType::sint_pack32;
		return;

	case vfmt::a8b8g8r8_srgb_pack32:
		details->alpha = true; details->alphaFirst = true; details->blueEndian = true;
		details->sRGB = true; details->storageType = SType::srgb_pack32;
		return;

	case vfmt::a2r10g10b10_unorm_pack32:
		details->alpha = true; details->alphaFirst = true;
		details->storageType = SType::unorm_pack32;
		details->channelSize = FCSize::special;
		return;

	case vfmt::a2r10g10b10_snorm_pack32:
		details->alpha = true; details->alphaFirst = true;
		details->storageType = SType::snorm_pack32;
		details->channelSize = FCSize::special;
		return;

	case vfmt::a2r10g10b10_uscaled_pack32:
		details->alpha = true; details->alphaFirst = true;
		details->storageType = SType::uscaled_pack32;
		details->channelSize = FCSize::special;
		return;

	case vfmt::a2r10g10b10_sscaled_pack32:
		details->alpha = true; details->alphaFirst = true;
		details->storageType = SType::sscaled_pack32;
		details->channelSize = FCSize::special;
		return;

	case vfmt::a2r10g10b10_uint_pack32:
		details->alpha = true; details->alphaFirst = true;
		details->channelSize = FCSize::special;
		details->storageType = SType::uint_pack32;
		return;

	case vfmt::a2r10g10b10_sint_pack32:
		details->alpha = true; details->alphaFirst = true;
		details->storageType = SType::sint_pack32;
		details->channelSize = FCSize::special;
		return;

	case vfmt::a2b10g10r10_unorm_pack32:
		details->alpha = true; details->alphaFirst = true;
		details->blueEndian = true; details->storageType = SType::unorm_pack32;
		details->channelSize = FCSize::special;
		return;

	case vfmt::a2b10g10r10_snorm_pack32:
		details->alpha = true; details->alphaFirst = true;
		details->blueEndian = true; details->storageType = SType::snorm_pack32;
		details->channelSize = FCSize::special;
		return;

	case vfmt::a2b10g10r10_uscaled_pack32:
		details->alpha = true; details->alphaFirst = true;
		details->blueEndian = true; details->storageType = SType::uscaled_pack32;
		details->channelSize = FCSize::special;
		return;

	case vfmt::a2b10g10r10_sscaled_pack32:
		details->alpha = true; details->alphaFirst = true;
		details->blueEndian = true; details->storageType = SType::sscaled_pack32;
		details->channelSize = FCSize::special;
		return;

	case vfmt::a2b10g10r10_uint_pack32:
		details->alpha = true; details->alphaFirst = true;
		details->blueEndian = true; details->storageType = SType::uint_pack32;
		details->channelSize = FCSize::special;
		return;

	case vfmt::a2b10g10r10_sint_pack32:
		details->alpha = true; details->alphaFirst = true;
		details->blueEndian = true; details->storageType = SType::sint_pack32;
		details->channelSize = FCSize::special;
		return;

	case vfmt::r16_unorm:
		details->channels = 1;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16_snorm:
		details->channels = 1; details->storageType = SType::snorm;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16_uscaled:
		details->channels = 1; details->storageType = SType::uscaled;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16_sscaled:
		details->channels = 1; details->storageType = SType::sscaled;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16_uint:
		details->channels = 1; details->storageType = SType::uint;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16_sint:
		details->channels = 1; details->storageType = SType::sint;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16_sfloat:
		details->channels = 1; details->storageType = SType::sfloat;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16g16_unorm:
		details->channels = 2;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16g16_snorm:
		details->channels = 2; details->storageType = SType::snorm;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16g16_uscaled:
		details->channels = 2; details->storageType = SType::uscaled;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16g16_sscaled:
		details->channels = 2; details->storageType = SType::sscaled;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16g16_uint:
		details->channels = 2; details->storageType = SType::uint;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16g16_sint:
		details->channels = 2; details->storageType = SType::sint;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16g16_sfloat:
		details->channels = 2; details->storageType = SType::sfloat;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16g16b16_unorm:
		details->channels = 3;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16g16b16_snorm:
		details->channels = 3; details->storageType = SType::snorm;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16g16b16_uscaled:
		details->channels = 3; details->storageType = SType::uscaled;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16g16b16_sscaled:
		details->channels = 3; details->storageType = SType::sscaled;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16g16b16_uint:
		details->channels = 3; details->storageType = SType::uint;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16g16b16_sint:
		details->channels = 3; details->storageType = SType::sint;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16g16b16_sfloat:
		details->channels = 3; details->storageType = SType::sfloat;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16g16b16a16_unorm:
		details->alpha = true;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16g16b16a16_snorm:
		details->alpha = true; details->storageType = SType::snorm;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16g16b16a16_uscaled:
		details->alpha = true; details->storageType = SType::uscaled;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16g16b16a16_sscaled:
		details->alpha = true; details->storageType = SType::sscaled;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16g16b16a16_uint:
		details->alpha = true; details->storageType = SType::uint;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16g16b16a16_sint:
		details->alpha = true; details->storageType = SType::sint;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r16g16b16a16_sfloat:
		details->alpha = true; details->storageType = SType::sfloat;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::r32_uint:
		details->channels = 1; details->storageType = SType::uint;
		details->channelSize = FCSize::channel_32;
		return;

	case vfmt::r32_sint:
		details->channels = 1; details->storageType = SType::sint;
		details->channelSize = FCSize::channel_32;
		return;

	case vfmt::r32_sfloat:
		details->channels = 1; details->storageType = SType::sfloat;
		details->channelSize = FCSize::channel_32;
		return;

	case vfmt::r32g32_uint:
		details->channels = 2; details->storageType = SType::uint;
		details->channelSize = FCSize::channel_32;
		return;

	case vfmt::r32g32_sint:
		details->channels = 2; details->storageType = SType::sint;
		details->channelSize = FCSize::channel_32;
		return;

	case vfmt::r32g32_sfloat:
		details->channels = 2; details->storageType = SType::sfloat;
		details->channelSize = FCSize::channel_32;
		return;

	case vfmt::r32g32b32_uint:
		details->channels = 3; details->storageType = SType::uint;
		details->channelSize = FCSize::channel_32;
		return;

	case vfmt::r32g32b32_sint:
		details->channels = 3; details->storageType = SType::sint;
		details->channelSize = FCSize::channel_32;
		return;

	case vfmt::r32g32b32_sfloat:
		details->channels = 3; details->storageType = SType::sfloat;
		details->channelSize = FCSize::channel_32;
		return;

	case vfmt::r32g32b32a32_uint:
		details->alpha = true; details->storageType = SType::uint;
		details->channelSize = FCSize::channel_32;
		return;

	case vfmt::r32g32b32a32_sint:
		details->alpha = true; details->storageType = SType::sint;
		details->channelSize = FCSize::channel_32;
		return;

	case vfmt::r32g32b32a32_sfloat:
		details->alpha = true; details->storageType = SType::sfloat;
		details->channelSize = FCSize::channel_32;
		return;

	case vfmt::r64_uint:
		details->channels = 1; details->storageType = SType::uint;
		details->channelSize = FCSize::channel_64;
		return;

	case vfmt::r64_sint:
		details->channels = 1; details->storageType = SType::sint;
		details->channelSize = FCSize::channel_64;
		return;

	case vfmt::r64_sfloat:
		details->channels = 1; details->storageType = SType::sfloat;
		details->channelSize = FCSize::channel_64;
		return;

	case vfmt::r64g64_uint:
		details->channels = 2; details->storageType = SType::uint;
		details->channelSize = FCSize::channel_64;
		return;

	case vfmt::r64g64_sint:
		details->channels = 2; details->storageType = SType::sint;
		details->channelSize = FCSize::channel_64;
		return;

	case vfmt::r64g64_sfloat:
		details->channels = 2; details->storageType = SType::sfloat;
		details->channelSize = FCSize::channel_64;
		return;

	case vfmt::r64g64b64_uint:
		details->channels = 3; details->storageType = SType::uint;
		details->channelSize = FCSize::channel_64;
		return;

	case vfmt::r64g64b64_sint:
		details->channels = 3; details->storageType = SType::sint;
		details->channelSize = FCSize::channel_64;
		return;

	case vfmt::r64g64b64_sfloat:
		details->channels = 3; details->storageType = SType::sfloat;
		details->channelSize = FCSize::channel_64;
		return;

	case vfmt::r64g64b64a64_uint:
		details->alpha = true; details->storageType = SType::uint;
		details->channelSize = FCSize::channel_64;
		return;

	case vfmt::r64g64b64a64_sint:
		details->alpha = true; details->storageType = SType::sint;
		details->channelSize = FCSize::channel_64;
		return;

	case vfmt::r64g64b64a64_sfloat:
		details->alpha = true; details->storageType = SType::sfloat;
		details->channelSize = FCSize::channel_64;
		return;

	case vfmt::b10g11r11_ufloat_pack32: // TODO: How to handle this format?
		details->channels = 3; details->blueEndian = true;
		details->storageType = SType::ufloat_pack32;
		details->channelSize = FCSize::special;
		return;

	case vfmt::e5b9g9r9_ufloat_pack32: // TODO: How to handle this format?
		details->blueEndian = true;
		details->storageType = SType::ufloat_pack32;
		details->channelSize = FCSize::special;
		vtek_log_error(
			"Format \"e5b9g9r9_ufloat_pack32\" is not properly implemented!");
		return;

	case vfmt::d16_unorm:
		details->channels = 1; details->depth = true;
		details->channelSize = FCSize::channel_16;
		return;

	case vfmt::x8_d24_unorm_pack32:
		details->channels = 1; details->storageType = SType::unused8_unorm24_pack32;
		details->depth = true; details->channelSize = FCSize::special;
		return;

	case vfmt::d32_sfloat:
		details->channels = 1; details->storageType = SType::sfloat;
		details->depth = true; details->channelSize = FCSize::channel_32;
		return;

	case vfmt::s8_uint:
		details->channels = 1; details->storageType = SType::uint;
		details->stencil = true; details->channelSize = FCSize::channel_8;
		return;

	case vfmt::d16_unorm_s8_uint:
		details->channels = 2; details->storageType = SType::unorm16_uint8;
		details->depth = true; details->stencil = true;
		details->channelSize = FCSize::special;
		return;

	case vfmt::d24_unorm_s8_uint:
		details->channels = 2; details->storageType = SType::unorm24_uint8;
		details->depth = true; details->stencil = true;
		details->channelSize = FCSize::special;
		return;

	case vfmt::d32_sfloat_s8_uint:
		details->channels = 2; details->storageType = SType::sfloat32_uint8;
		details->depth = true; details->stencil = true;
		details->channelSize = FCSize::special;
		return;

		// NOTE: The detailing of compressed formats might be incomplete!

	case vfmt::bc1_rgb_unorm_block:
		details->compression = FCompr::bc1;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::bc1_rgb_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::bc1;
		return;

	case vfmt::bc1_rgba_unorm_block:
		details->compression = FCompr::bc1; details->alpha = true;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::bc1_rgba_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::bc1; details->alpha = true;
		return;

	case vfmt::bc2_unorm_block:
		details->compression = FCompr::bc2;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::bc2_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::bc2;
		return;

	case vfmt::bc3_unorm_block:
		details->compression = FCompr::bc3;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::bc3_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::bc3;
		return;

	case vfmt::bc4_unorm_block:
		details->compression = FCompr::bc4;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::bc4_snorm_block:
		details->compression = FCompr::bc4;
		details->storageType = SType::snorm_block;
		return;

	case vfmt::bc5_unorm_block:
		details->compression = FCompr::bc5;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::bc5_snorm_block:
		details->compression = FCompr::bc5;
		details->storageType = SType::snorm_block;
		return;

	case vfmt::bc6h_ufloat_block:
		details->compression = FCompr::bc6h;
		details->storageType = SType::ufloat_block;
		return;

	case vfmt::bc6h_sfloat_block:
		details->compression = FCompr::bc6h;
		details->storageType = SType::sfloat_block;
		return;

	case vfmt::bc7_unorm_block:
		details->compression = FCompr::bc7;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::bc7_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::bc7;
		return;

	case vfmt::etc2_r8g8b8_unorm_block:
		details->compression = FCompr::etc2;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::etc2_r8g8b8_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::etc2;
		return;

	case vfmt::etc2_r8g8b8a1_unorm_block:
		details->compression = FCompr::etc2; details->alpha = true;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::etc2_r8g8b8a1_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::etc2; details->alpha = true;
		return;

	case vfmt::etc2_r8g8b8a8_unorm_block:
		details->compression = FCompr::etc2; details->alpha = true;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::etc2_r8g8b8a8_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::etc2; details->alpha = true;
		return;

	case vfmt::eac_r11_unorm_block:
		details->compression = FCompr::eac;
		details->storageType = SType::unorm_block;
		details->channelSize = FCSize::special;
		return;

	case vfmt::eac_r11_snorm_block:
		details->compression = FCompr::eac;
		details->storageType = SType::snorm_block;
		details->channelSize = FCSize::special;
		return;

	case vfmt::eac_r11g11_unorm_block:
		details->compression = FCompr::eac;
		details->storageType = SType::unorm_block;
		details->channelSize = FCSize::special;
		return;

	case vfmt::eac_r11g11_snorm_block:
		details->compression = FCompr::eac;
		details->storageType = SType::snorm_block;
		details->channelSize = FCSize::special;
		return;

	case vfmt::astc_4x4_unorm_block:
		details->compression = FCompr::astc_4x4;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::astc_4x4_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::astc_4x4;
		return;

	case vfmt::astc_5x4_unorm_block:
		details->compression = FCompr::astc_5x4;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::astc_5x4_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::astc_5x4;
		return;

	case vfmt::astc_5x5_unorm_block:
		details->compression = FCompr::astc_5x5;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::astc_5x5_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::astc_5x5;
		return;

	case vfmt::astc_6x5_unorm_block:
		details->compression = FCompr::astc_6x5;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::astc_6x5_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::astc_6x5;
		return;

	case vfmt::astc_6x6_unorm_block:
		details->compression = FCompr::astc_6x6;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::astc_6x6_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::astc_6x6;
		return;

	case vfmt::astc_8x5_unorm_block:
		details->compression = FCompr::astc_8x5;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::astc_8x5_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::astc_8x5;
		return;

	case vfmt::astc_8x6_unorm_block:
		details->compression = FCompr::astc_8x6;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::astc_8x6_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::astc_8x6;
		return;

	case vfmt::astc_8x8_unorm_block:
		details->compression = FCompr::astc_8x8;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::astc_8x8_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::astc_8x8;
		return;

	case vfmt::astc_10x5_unorm_block:
		details->compression = FCompr::astc_10x5;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::astc_10x5_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::astc_10x5;
		return;

	case vfmt::astc_10x6_unorm_block:
		details->compression = FCompr::astc_10x6;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::astc_10x6_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::astc_10x6;
		return;

	case vfmt::astc_10x8_unorm_block:
		details->compression = FCompr::astc_10x8;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::astc_10x8_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::astc_10x8;
		return;

	case vfmt::astc_10x10_unorm_block:
		details->compression = FCompr::astc_10x10;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::astc_10x10_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::astc_10x10;
		return;

	case vfmt::astc_12x10_unorm_block:
		details->compression = FCompr::astc_12x10;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::astc_12x10_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::astc_12x10;
		return;

	case vfmt::astc_12x12_unorm_block:
		details->compression = FCompr::astc_12x12;
		details->storageType = SType::unorm_block;
		return;

	case vfmt::astc_12x12_srgb_block:
		details->sRGB = true; details->storageType = SType::srgb_block;
		details->compression = FCompr::astc_12x12;
		return;

		// NOTE: Most of the formats below are weird and (mostly) unhandled!

	// Provided by VK_VERSION_1_1
	case vfmt::g8b8g8r8_422_unorm:
		details->channels = 3; details->storageType = SType::rect_422;
		return;

	case vfmt::b8g8r8g8_422_unorm:
		details->channels = 3; details->storageType = SType::rect_422;
		return;

	case vfmt::g8_b8_r8_3plane_420_unorm:
		details->planar = true;
		return;

	case vfmt::g8_b8r8_2plane_420_unorm:
		details->planar = true;
		return;

	case vfmt::g8_b8_r8_3plane_422_unorm:
		details->planar = true;
		return;

	case vfmt::g8_b8r8_2plane_422_unorm:
		details->planar = true;
		return;

	case vfmt::g8_b8_r8_3plane_444_unorm:
		details->planar = true;
		return;

	case vfmt::r10x6_unorm_pack16:
		details->channels = 1; details->channelSize = FCSize::special;
		details->storageType = SType::unorm10_unused6_pack16;
		return;

	case vfmt::r10x6g10x6_unorm_2pack16:
		details->channels = 2; details->channelSize = FCSize::special;
		details->storageType = SType::unorm10_unused6_pack16;
		return;

	case vfmt::r10x6g10x6b10x6a10x6_unorm_4pack16:
		details->alpha = true; details->channelSize = FCSize::special;
		details->storageType = SType::unorm10_unused6_pack16;
		return;

	case vfmt::g10x6b10x6g10x6r10x6_422_unorm_4pack16:
		details->channels = 3; details->channelSize = FCSize::special;
		details->storageType = SType::rect_422;
		return;

	case vfmt::b10x6g10x6r10x6g10x6_422_unorm_4pack16:
		details->channels = 3; details->channelSize = FCSize::special;
		details->storageType = SType::rect_422;
		return;

	case vfmt::g10x6_b10x6_r10x6_3plane_420_unorm_3pack16:
		details->planar = true; details->channelSize = FCSize::special;
		return;

	case vfmt::g10x6_b10x6r10x6_2plane_420_unorm_3pack16:
		details->planar = true; details->channelSize = FCSize::special;
		return;

	case vfmt::g10x6_b10x6_r10x6_3plane_422_unorm_3pack16:
		details->planar = true; details->channelSize = FCSize::special;
		return;

	case vfmt::g10x6_b10x6r10x6_2plane_422_unorm_3pack16:
		details->planar = true; details->channelSize = FCSize::special;
		return;

	case vfmt::g10x6_b10x6_r10x6_3plane_444_unorm_3pack16:
		details->planar = true; details->channelSize = FCSize::special;
		return;

	case vfmt::r12x4_unorm_pack16:
		details->channels = 1; details->channelSize = FCSize::special;
		details->storageType = SType::unorm12_unused4_pack16;
		return;

	case vfmt::r12x4g12x4_unorm_2pack16:
		details->channels = 2; details->channelSize = FCSize::special;
		details->storageType = SType::unorm12_unused4_pack16;
		return;

	case vfmt::r12x4g12x4b12x4a12x4_unorm_4pack16:
		details->channelSize = FCSize::special;
		details->storageType = SType::unorm12_unused4_pack16;
		return;

	case vfmt::g12x4b12x4g12x4r12x4_422_unorm_4pack16:
		details->channels = 3; details->channelSize = FCSize::special;
		details->storageType = SType::rect_422;
		return;

	case vfmt::b12x4g12x4r12x4g12x4_422_unorm_4pack16:
		details->channels = 3; details->channelSize = FCSize::special;
		details->storageType = SType::rect_422;
		return;

	case vfmt::g12x4_b12x4_r12x4_3plane_420_unorm_3pack16:
		details->planar = true; details->channelSize = FCSize::special;
		return;

	case vfmt::g12x4_b12x4r12x4_2plane_420_unorm_3pack16:
		details->planar = true; details->channelSize = FCSize::special;
		return;

	case vfmt::g12x4_b12x4_r12x4_3plane_422_unorm_3pack16:
		details->planar = true; details->channelSize = FCSize::special;
		return;

	case vfmt::g12x4_b12x4r12x4_2plane_422_unorm_3pack16:
		details->planar = true; details->channelSize = FCSize::special;
		return;

	case vfmt::g12x4_b12x4_r12x4_3plane_444_unorm_3pack16:
		details->planar = true; details->channelSize = FCSize::special;
		return;

	case vfmt::g16b16g16r16_422_unorm:
		details->channelSize = FCSize::channel_16;
		details->storageType = SType::rect_422;
		return;

	case vfmt::b16g16r16g16_422_unorm:
		details->channelSize = FCSize::channel_16;
		details->storageType = SType::rect_422;
		return;

	case vfmt::g16_b16_r16_3plane_420_unorm:
		details->planar = true; details->channelSize = FCSize::special;
		return;

	case vfmt::g16_b16r16_2plane_420_unorm:
		details->planar = true; details->channelSize = FCSize::special;
		return;

	case vfmt::g16_b16_r16_3plane_422_unorm:
		details->planar = true; details->channelSize = FCSize::special;
		return;

	case vfmt::g16_b16r16_2plane_422_unorm:
		details->planar = true; details->channelSize = FCSize::special;
		return;

	case vfmt::g16_b16_r16_3plane_444_unorm:
		details->planar = true; details->channelSize = FCSize::special;
		return;

	// Provided by VK_VERSION_1_3
	case vfmt::g8_b8r8_2plane_444_unorm:
		details->planar = true; details->channelSize = FCSize::special;
		return;

	case vfmt::g10x6_b10x6r10x6_2plane_444_unorm_3pack16:
		details->planar = true; details->channelSize = FCSize::special;
		return;

	case vfmt::g12x4_b12x4r12x4_2plane_444_unorm_3pack16:
		details->planar = true; details->channelSize = FCSize::special;
		return;

	case vfmt::g16_b16r16_2plane_444_unorm:
		details->planar = true; details->channelSize = FCSize::special;
		return;

	case vfmt::a4r4g4b4_unorm_pack16:
		details->alpha = true; details->alphaFirst = true;
		details->channelSize = FCSize::channel_4;
		details->storageType = SType::unorm_pack16;
		return;

	case vfmt::a4b4g4r4_unorm_pack16:
		details->alpha = true; details->alphaFirst = true;
		details->channelSize = FCSize::channel_4; details->blueEndian = true;
		details->storageType = SType::unorm_pack16;
		return;

	case vfmt::astc_4x4_sfloat_block:
		details->compression = FCompr::astc_4x4;
		details->storageType = SType::sfloat_block;
		details->channelSize = FCSize::special;
		return;

	case vfmt::astc_5x4_sfloat_block:
		details->compression = FCompr::astc_5x4;
		details->storageType = SType::sfloat_block;
		details->channelSize = FCSize::special;
		return;

	case vfmt::astc_5x5_sfloat_block:
		details->compression = FCompr::astc_5x5;
		details->storageType = SType::sfloat_block;
		details->channelSize = FCSize::special;
		return;

	case vfmt::astc_6x5_sfloat_block:
		details->compression = FCompr::astc_6x5;
		details->storageType = SType::sfloat_block;
		details->channelSize = FCSize::special;
		return;

	case vfmt::astc_6x6_sfloat_block:
		details->compression = FCompr::astc_6x6;
		details->storageType = SType::sfloat_block;
		details->channelSize = FCSize::special;
		return;

	case vfmt::astc_8x5_sfloat_block:
		details->compression = FCompr::astc_8x5;
		details->storageType = SType::sfloat_block;
		details->channelSize = FCSize::special;
		return;

	case vfmt::astc_8x6_sfloat_block:
		details->compression = FCompr::astc_8x6;
		details->storageType = SType::sfloat_block;
		details->channelSize = FCSize::special;
		return;

	case vfmt::astc_8x8_sfloat_block:
		details->compression = FCompr::astc_8x8;
		details->storageType = SType::sfloat_block;
		details->channelSize = FCSize::special;
		return;

	case vfmt::astc_10x5_sfloat_block:
		details->compression = FCompr::astc_10x5;
		details->storageType = SType::sfloat_block;
		details->channelSize = FCSize::special;
		return;

	case vfmt::astc_10x6_sfloat_block:
		details->compression = FCompr::astc_10x6;
		details->storageType = SType::sfloat_block;
		details->channelSize = FCSize::special;
		return;

	case vfmt::astc_10x8_sfloat_block:
		details->compression = FCompr::astc_10x8;
		details->storageType = SType::sfloat_block;
		details->channelSize = FCSize::special;
		return;

	case vfmt::astc_10x10_sfloat_block:
		details->compression = FCompr::astc_10x10;
		details->storageType = SType::sfloat_block;
		details->channelSize = FCSize::special;
		return;

	case vfmt::astc_12x10_sfloat_block:
		details->compression = FCompr::astc_12x10;
		details->storageType = SType::sfloat_block;
		details->channelSize = FCSize::special;
		return;

	case vfmt::astc_12x12_sfloat_block:
		details->compression = FCompr::astc_12x12;
		details->storageType = SType::sfloat_block;
		details->channelSize = FCSize::special;
		return;

	default:
		vtek_log_error("vtek_format_support.cpp: unrecognized format enum!");
		return;
	}
}



/* query color format (LONG section) */
static void get_format_color_srgb(
	const vtek::FormatQuery* query, std::vector<VkFormat>& priorities)
{
	uint32_t channels = static_cast<uint32_t>(query->channels);

	if (channels == 1)
	{
		priorities.push_back(VK_FORMAT_R8_SRGB);
	}
	else if (channels == 2)
	{
		priorities.push_back(VK_FORMAT_R8G8_SRGB);
	}
	else if (channels == 3)
	{
		if (query->swizzleBGR)
		{
			priorities.push_back(VK_FORMAT_B8G8R8_SRGB);
		}
		else
		{
			priorities.push_back(VK_FORMAT_R8G8B8_SRGB);
		}
	}
	else if (channels == 4)
	{
		if (query->swizzleBGR &&
		    query->storageType == vtek::FormatStorageType::unorm_pack32)
		{
			priorities.push_back(VK_FORMAT_A8B8G8R8_SRGB_PACK32);
		}
		else if (query->swizzleBGR)
		{
			priorities.push_back(VK_FORMAT_B8G8R8A8_SRGB);
		}
		else
		{
			priorities.push_back(VK_FORMAT_R8G8B8A8_SRGB);
		}
	}
}

static void get_format_color_channel_1(
	const vtek::FormatQuery* query, std::vector<VkFormat>& priorities)
{
	if (query->channelSize == vtek::FormatChannelSize::channel_8)
	{
		switch (query->storageType)
		{
		case vtek::FormatStorageType::unorm:
			priorities.push_back(VK_FORMAT_R8_UNORM);
			break;
		case vtek::FormatStorageType::snorm:
			priorities.push_back(VK_FORMAT_R8_SNORM);
			break;
		case vtek::FormatStorageType::uscaled:
			priorities.push_back(VK_FORMAT_R8_USCALED);
			break;
		case vtek::FormatStorageType::sscaled:
			priorities.push_back(VK_FORMAT_R8_SSCALED);
			break;
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R8_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R8_SINT);
			break;
		default:
			break;
		}
	}
	else if (query->channelSize == vtek::FormatChannelSize::channel_16)
	{
		switch (query->storageType)
		{
		case vtek::FormatStorageType::unorm:
			priorities.push_back(VK_FORMAT_R16_UNORM);
			break;
		case vtek::FormatStorageType::snorm:
			priorities.push_back(VK_FORMAT_R16_SNORM);
			break;
		case vtek::FormatStorageType::uscaled:
			priorities.push_back(VK_FORMAT_R16_USCALED);
			break;
		case vtek::FormatStorageType::sscaled:
			priorities.push_back(VK_FORMAT_R16_SSCALED);
			break;
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R16_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R16_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R16_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (query->channelSize == vtek::FormatChannelSize::channel_32)
	{
		switch (query->storageType)
		{
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R32_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R32_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R32_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (query->channelSize == vtek::FormatChannelSize::channel_64)
	{
		switch (query->storageType)
		{
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R64_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R64_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R64_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (query->channelSize == vtek::FormatChannelSize::special)
	{
		if (query->storageType == vtek::FormatStorageType::unorm_pack16)
		{
			priorities.push_back(VK_FORMAT_R12X4_UNORM_PACK16);
			priorities.push_back(VK_FORMAT_R10X6_UNORM_PACK16);
		}
	}
}

static void get_format_color_channel_2(
	const vtek::FormatQuery* query, std::vector<VkFormat>& priorities)
{
	if (query->channelSize == vtek::FormatChannelSize::channel_8)
	{
		switch (query->storageType)
		{
		case vtek::FormatStorageType::unorm:
			priorities.push_back(VK_FORMAT_R8G8_UNORM);
			break;
		case vtek::FormatStorageType::snorm:
			priorities.push_back(VK_FORMAT_R8G8_SNORM);
			break;
		case vtek::FormatStorageType::uscaled:
			priorities.push_back(VK_FORMAT_R8G8_USCALED);
			break;
		case vtek::FormatStorageType::sscaled:
			priorities.push_back(VK_FORMAT_R8G8_SSCALED);
			break;
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R8G8_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R8G8_SINT);
			break;
		default:
			break;
		}
	}
	else if (query->channelSize == vtek::FormatChannelSize::channel_16)
	{
		switch (query->storageType)
		{
		case vtek::FormatStorageType::unorm:
			priorities.push_back(VK_FORMAT_R16G16_UNORM);
			break;
		case vtek::FormatStorageType::snorm:
			priorities.push_back(VK_FORMAT_R16G16_SNORM);
			break;
		case vtek::FormatStorageType::uscaled:
			priorities.push_back(VK_FORMAT_R16G16_USCALED);
			break;
		case vtek::FormatStorageType::sscaled:
			priorities.push_back(VK_FORMAT_R16G16_SSCALED);
			break;
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R16G16_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R16G16_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R16G16_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (query->channelSize == vtek::FormatChannelSize::channel_32)
	{
		switch (query->storageType)
		{
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R32G32_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R32G32_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R32G32_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (query->channelSize == vtek::FormatChannelSize::channel_64)
	{
		switch (query->storageType)
		{
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R64G64_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R64G64_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R64G64_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (query->channelSize == vtek::FormatChannelSize::special)
	{
		if (query->storageType == vtek::FormatStorageType::unorm_pack8)
		{
			priorities.push_back(VK_FORMAT_R4G4_UNORM_PACK8);
		}
		else if (query->storageType == vtek::FormatStorageType::unorm_pack16)
		{
			priorities.push_back(VK_FORMAT_R12X4G12X4_UNORM_2PACK16);
			priorities.push_back(VK_FORMAT_R10X6G10X6_UNORM_2PACK16);
		}
	}
}

static void get_format_color_channel_3(
	const vtek::FormatQuery* query, std::vector<VkFormat>& priorities)
{
	if (query->channelSize == vtek::FormatChannelSize::channel_8)
	{
		switch (query->storageType)
		{
		case vtek::FormatStorageType::unorm:
			priorities.push_back(
				(query->swizzleBGR)
				? VK_FORMAT_B8G8R8_UNORM : VK_FORMAT_R8G8B8_UNORM);
			break;
		case vtek::FormatStorageType::snorm:
			priorities.push_back(
				(query->swizzleBGR)
				? VK_FORMAT_B8G8R8_SNORM : VK_FORMAT_R8G8B8_SNORM);
			break;
		case vtek::FormatStorageType::uscaled:
			priorities.push_back(
				(query->swizzleBGR)
				? VK_FORMAT_B8G8R8_USCALED : VK_FORMAT_R8G8B8_USCALED);
			break;
		case vtek::FormatStorageType::sscaled:
			priorities.push_back(
				(query->swizzleBGR)
				? VK_FORMAT_B8G8R8_SSCALED : VK_FORMAT_R8G8B8_SSCALED);
			break;
		case vtek::FormatStorageType::uint:
			priorities.push_back(
				(query->swizzleBGR)
				? VK_FORMAT_B8G8R8_UINT : VK_FORMAT_R8G8B8_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(
				(query->swizzleBGR)
				? VK_FORMAT_B8G8R8_SINT : VK_FORMAT_R8G8B8_SINT);
			break;
		default:
			break;
		}
	}
	else if (query->channelSize == vtek::FormatChannelSize::channel_16 &&
	         !query->swizzleBGR)
	{
		switch (query->storageType)
		{
		case vtek::FormatStorageType::unorm:
			priorities.push_back(VK_FORMAT_R16G16B16_UNORM);
			break;
		case vtek::FormatStorageType::snorm:
			priorities.push_back(VK_FORMAT_R16G16B16_SNORM);
			break;
		case vtek::FormatStorageType::uscaled:
			priorities.push_back(VK_FORMAT_R16G16B16_USCALED);
			break;
		case vtek::FormatStorageType::sscaled:
			priorities.push_back(VK_FORMAT_R16G16B16_SSCALED);
			break;
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R16G16B16_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R16G16B16_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R16G16B16_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (query->channelSize == vtek::FormatChannelSize::channel_32 &&
	         !query->swizzleBGR)
	{
		switch (query->storageType)
		{
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R32G32B32_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R32G32B32_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R32G32B32_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (query->channelSize == vtek::FormatChannelSize::channel_64 &&
	         !query->swizzleBGR)
	{
		switch (query->storageType)
		{
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R64G64B64_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R64G64B64_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R64G64B64_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (query->channelSize == vtek::FormatChannelSize::special)
	{
		if (query->storageType == vtek::FormatStorageType::ufloat_pack32)
		{
			priorities.push_back(VK_FORMAT_B10G11R11_UFLOAT_PACK32);
		}
	}
}

static void get_format_color_channel_4(
	const vtek::FormatQuery* query, std::vector<VkFormat>& priorities)
{
	if (query->channelSize == vtek::FormatChannelSize::channel_8)
	{
		switch (query->storageType)
		{
		case vtek::FormatStorageType::unorm:
			priorities.push_back(
				(query->swizzleBGR)
				? VK_FORMAT_B8G8R8A8_UNORM : VK_FORMAT_R8G8B8A8_UNORM);
			break;
		case vtek::FormatStorageType::snorm:
			priorities.push_back(
				(query->swizzleBGR)
				? VK_FORMAT_B8G8R8A8_SNORM : VK_FORMAT_R8G8B8A8_SNORM);
			break;
		case vtek::FormatStorageType::uscaled:
			priorities.push_back(
				(query->swizzleBGR)
				? VK_FORMAT_B8G8R8A8_USCALED : VK_FORMAT_R8G8B8A8_USCALED);
			break;
		case vtek::FormatStorageType::sscaled:
			priorities.push_back(
				(query->swizzleBGR)
				? VK_FORMAT_B8G8R8A8_SSCALED : VK_FORMAT_R8G8B8A8_SSCALED);
			break;
		case vtek::FormatStorageType::uint:
			priorities.push_back(
				(query->swizzleBGR)
				? VK_FORMAT_B8G8R8A8_UINT : VK_FORMAT_R8G8B8A8_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(
				(query->swizzleBGR)
				? VK_FORMAT_B8G8R8A8_SINT : VK_FORMAT_R8G8B8A8_SINT);
			break;
		case vtek::FormatStorageType::unorm_pack32:
			priorities.push_back(VK_FORMAT_A8B8G8R8_UNORM_PACK32);
			break;
		case vtek::FormatStorageType::snorm_pack32:
			priorities.push_back(VK_FORMAT_A8B8G8R8_SNORM_PACK32);
			break;
		case vtek::FormatStorageType::uscaled_pack32:
			priorities.push_back(VK_FORMAT_A8B8G8R8_USCALED_PACK32);
			break;
		case vtek::FormatStorageType::sscaled_pack32:
			priorities.push_back(VK_FORMAT_A8B8G8R8_SSCALED_PACK32);
			break;
		case vtek::FormatStorageType::uint_pack32:
			priorities.push_back(VK_FORMAT_A8B8G8R8_UINT_PACK32);
			break;
		case vtek::FormatStorageType::sint_pack32:
			priorities.push_back(VK_FORMAT_A8B8G8R8_SINT_PACK32);
			break;
		default:
			break;
		}
	}
	else if (query->channelSize == vtek::FormatChannelSize::channel_16)
	{
		switch (query->storageType)
		{
		case vtek::FormatStorageType::unorm:
			priorities.push_back(VK_FORMAT_R16G16B16A16_UNORM);
			break;
		case vtek::FormatStorageType::snorm:
			priorities.push_back(VK_FORMAT_R16G16B16A16_SNORM);
			break;
		case vtek::FormatStorageType::uscaled:
			priorities.push_back(VK_FORMAT_R16G16B16A16_USCALED);
			break;
		case vtek::FormatStorageType::sscaled:
			priorities.push_back(VK_FORMAT_R16G16B16A16_SSCALED);
			break;
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R16G16B16A16_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R16G16B16A16_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R16G16B16A16_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (query->channelSize == vtek::FormatChannelSize::channel_32)
	{
		switch (query->storageType)
		{
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R32G32B32A32_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R32G32B32A32_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R32G32B32A32_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (query->channelSize == vtek::FormatChannelSize::channel_64)
	{
		switch (query->storageType)
		{
		case vtek::FormatStorageType::uint:
			priorities.push_back(VK_FORMAT_R64G64B64A64_UINT);
			break;
		case vtek::FormatStorageType::sint:
			priorities.push_back(VK_FORMAT_R64G64B64A64_SINT);
			break;
		case vtek::FormatStorageType::sfloat:
			priorities.push_back(VK_FORMAT_R64G64B64A64_SFLOAT);
			break;
		default:
			break;
		}
	}
	else if (query->channelSize == vtek::FormatChannelSize::special &&
	         !query->swizzleBGR)
	{
		switch (query->storageType)
		{
		case vtek::FormatStorageType::unorm_pack16:
			priorities.push_back(VK_FORMAT_R4G4B4A4_UNORM_PACK16);
#if defined(VK_API_VERSION_1_3)
			priorities.push_back(VK_FORMAT_A4R4G4B4_UNORM_PACK16);
#endif
			priorities.push_back(VK_FORMAT_R5G5B5A1_UNORM_PACK16);
			priorities.push_back(VK_FORMAT_A1R5G5B5_UNORM_PACK16);
			break;
		case vtek::FormatStorageType::unorm_pack32:
			priorities.push_back(VK_FORMAT_A2R10G10B10_UNORM_PACK32);
			break;
		case vtek::FormatStorageType::snorm_pack32:
			priorities.push_back(VK_FORMAT_A2R10G10B10_SNORM_PACK32);
			break;
		case vtek::FormatStorageType::uscaled_pack32:
			priorities.push_back(VK_FORMAT_A2R10G10B10_USCALED_PACK32);
			break;
		case vtek::FormatStorageType::sscaled_pack32:
			priorities.push_back(VK_FORMAT_A2R10G10B10_SSCALED_PACK32);
			break;
		case vtek::FormatStorageType::uint_pack32:
			priorities.push_back(VK_FORMAT_A2R10G10B10_UINT_PACK32);
			break;
		case vtek::FormatStorageType::sint_pack32:
			priorities.push_back(VK_FORMAT_A2R10G10B10_SINT_PACK32);
			break;
		default:
			break;
		}
	}
	else if (query->channelSize == vtek::FormatChannelSize::special &&
	         query->swizzleBGR)
	{
		switch (query->storageType)
		{
		case vtek::FormatStorageType::unorm_pack16:
			priorities.push_back(VK_FORMAT_B4G4R4A4_UNORM_PACK16);
#if defined(VK_API_VERSION_1_3)
			priorities.push_back(VK_FORMAT_A4B4G4R4_UNORM_PACK16);
#endif
			priorities.push_back(VK_FORMAT_B5G5R5A1_UNORM_PACK16);
			break;
		case vtek::FormatStorageType::unorm_pack32:
			priorities.push_back(VK_FORMAT_A2B10G10R10_UNORM_PACK32);
			break;
		case vtek::FormatStorageType::snorm_pack32:
			priorities.push_back(VK_FORMAT_A2B10G10R10_SNORM_PACK32);
			break;
		case vtek::FormatStorageType::uscaled_pack32:
			priorities.push_back(VK_FORMAT_A2B10G10R10_USCALED_PACK32);
			break;
		case vtek::FormatStorageType::sscaled_pack32:
			priorities.push_back(VK_FORMAT_A2B10G10R10_SSCALED_PACK32);
			break;
		case vtek::FormatStorageType::uint_pack32:
			priorities.push_back(VK_FORMAT_A2B10G10R10_UINT_PACK32);
			break;
		case vtek::FormatStorageType::sint_pack32:
			priorities.push_back(VK_FORMAT_A2B10G10R10_SINT_PACK32);
			break;
		case vtek::FormatStorageType::ufloat_pack32:
			priorities.push_back(VK_FORMAT_E5B9G9R9_UFLOAT_PACK32);
			break;
		default:
			break;
		}
	}
}

static void get_format_compressed_srgb(
	const vtek::FormatQuery* query, std::vector<VkFormat>& priorities)
{
	vtek_log_error("get_format_compressed_srgb(): Not implemented!");
}

static void get_format_compressed(
	const vtek::FormatQuery* query, std::vector<VkFormat>& priorities)
{
	vtek_log_error("get_format_compressed(): Not implemented!");
}





/* supported format class */
constexpr uint32_t kChannelBits      = 0x0003;
constexpr uint32_t kAlphaBit         = 0x0004;
constexpr uint32_t kSrgbBit          = 0x0008;
constexpr uint32_t kCompressedBit    = 0x0010;
constexpr uint32_t kLinearTilingBit  = 0x0020;
constexpr uint32_t kBlueEndianBit    = 0x0040;
constexpr uint32_t kAlphaFirstBit    = 0x0080;
constexpr uint32_t kDepthBit         = 0x0100;
constexpr uint32_t kStencilBit       = 0x0200;
constexpr uint32_t kDepthStencilBits = kDepthBit | kStencilBit;
constexpr uint32_t kPlanarBit        = 0x0400;

// static construction methods
bool vtek::SupportedFormat::FindFormat(
	const vtek::FormatInfo* info, vtek::Format format,
	const vtek::Device* device, vtek::SupportedFormat& out)
{
	if (format == vtek::Format::undefined || info == nullptr) { return false; }

	VkPhysicalDevice physDev = vtek::device_get_physical_handle(device);
	VkFormatFeatureFlags fflags = vtek::get_format_features(info->features);
	VkFormat fmt = get_format(format);
	bool linear = info->linearTiling;
	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(physDev, fmt, &props);

	if (linear && (props.linearTilingFeatures & fflags) == fflags)
	{
		out = vtek::SupportedFormat(format, fmt, linear);
		return true;
	}
	else if (!linear && (props.optimalTilingFeatures & fflags) == fflags)
	{
		out = vtek::SupportedFormat(format, fmt, linear);
		return true;
	}
	else
	{
		return false;
	}
}

bool vtek::SupportedFormat::FindFormat(
	const vtek::FormatInfo* info, const std::vector<vtek::Format>& formats,
	const vtek::Device* device, vtek::SupportedFormat& out)
{
	for (auto fmt : formats)
	{
		if (FindFormat(info, fmt, device, out)) { return true; }
	}
	return false; // no match
}

static bool vtek::SupportedFormat::QueryColorFormat(
	const vtek::FormatQuery* query, const vtek::FormatInfo* info,
	const vtek::Device* device, vtek::SupportedFormat& out)
{
	VkFormat vtek::get_format_color(
		const vtek::FormatInfo* info, VkPhysicalDevice physDev,
		vtek::EnumBitmask<vtek::FormatFeature> featureFlags)
	{
		VkFormat outFormat = VK_FORMAT_UNDEFINED;
		std::vector<vtek::Format> priorities; // TODO: Use `vtek::Format` instead!
		uint32_t channels = static_cast<uint32_t>(info->channels);

		// TODO: Independent function for all compressed formats!
		if (info->compression != vtek::FormatCompression::none)
		{
			if (info->sRGB)
			{
				get_format_compressed_srgb(info, priorities);
			}
			else
			{
				get_format_compressed(info, priorities);
			}
		}
		else if (info->sRGB)
		{
			get_format_color_srgb(info, priorities);
		}
		else if (channels == 1)
		{
			get_format_color_channel_1(info, priorities);
		}
		else if (channels == 2)
		{
			get_format_color_channel_2(info, priorities);
		}
		else if (channels == 3)
		{
			get_format_color_channel_3(info, priorities);
		}
		else if (channels == 4)
		{
			get_format_color_channel_4(info, priorities);
		}

		return FindFormat(info, priorities, device, out);
	}
}

// private constructor
vtek::SupportedFormat::SupportedFormat(
	vtek::Format _format, VkFormat _fmt, bool linearTiling)
{
	propertyMask = 0U;
	// TODO: Switch `format` and `_format` for consistency and readability!
	format = _format;
	fmt = _fmt;

	FormatDetails details{};
	get_format_details(_format, &details);

	// number of channels
	propertyMask |= static_cast<uint32_t>(details.channels - 1) & kChannelBits;

	// alpha
	propertyMask |= (details.alpha) ? kAlphaBit : 0U;
	propertyMask |= (details.alphaFirst) ? kAlphaFirstBit : 0U;

	// sRGB
	propertyMask |= (details.sRGB) ? kSrgbBit : 0U;

	// storage type
	storage = details.storageType;

	// compression
	if (details.compression != vtek::FormatCompression::none)
	{
		propertyMask |= kCompressedBit;
		this->compression = compression;
	}

	// linear tiling; false implies optimal tiling
	propertyMask |= (linearTiling) ? kLinearTilingBit : 0U;

	// blue-endian
	propertyMask |= (details.blueEndian) ? kBlueEndianBit : 0U;

	// depth/stencil
	propertyMask |= (details.depth) ? kDepthBit : 0U;
	propertyMask |= (details.stencil) ? kStencilBit : 0U;

	// planar
	propertyMask |= (details.planar) ? kPlanarBit : 0U;
}

bool vtek::SupportedFormat::operator==(vtek::Format _format) const
{
	return format == _format;
}

VkFormat vtek::SupportedFormat::get() const
{
	return fmt;
}

std::string_view vtek::SupportedFormat::get_format_string() const
{
	return get_format_string_helper(this->format);
}

vtek::FormatChannels vtek::SupportedFormat::get_num_channels() const
{
	// # channels stored in first two bits
	return static_cast<vtek::FormatChannels>((propertyMask & kChannelBits) + 1);
}

bool vtek::SupportedFormat::has_alpha() const
{
	return propertyMask & kAlphaBit;
}

bool vtek::SupportedFormat::is_srgb() const
{
	return propertyMask & kSrgbBit;
}

bool vtek::SupportedFormat::is_compressed() const
{
	return propertyMask & kCompressedBit;
}

bool vtek::SupportedFormat::is_linear_tiling_supported() const
{
	return propertyMask & kLinearTilingBit;
}

bool vtek::SupportedFormat::is_blue_endian() const
{
	return propertyMask & kBlueEndianBit;
}

bool vtek::SupportedFormat::is_alpha_first() const
{
	return propertyMask & kAlphaFirstBit;
}

bool vtek::SupportedFormat::is_depth_stencil() const
{
	return propertyMask & kDepthStencilBits;
}

bool vtek::SupportedFormat::has_depth() const
{
	return propertyMask & kDepthBit;
}

bool vtek::SupportedFormat::has_stencil() const
{
	return propertyMask & kStencilBit;
}

bool vtek::SupportedFormat::is_planar() const
{
	return propertyMask & kPlanarBit;
}

vtek::FormatCompression vtek::SupportedFormat::get_compression_scheme() const
{
	return compression;
}

vtek::FormatStorageType vtek::SupportedFormat::get_storage_type() const
{
	return storage;
}

vtek::EnumBitmask<vtek::FormatFeature>
vtek::SupportedFormat::get_supported_features() const
{
	return features;
}
