#include "vtek_vulkan.pch"
#include "vtek_format_support.hpp"

using vfmt = vtek::Format;


/* helper functions */
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



/* supported format class */
static constexpr uint32_t kChannelBits      = 0x0003;
static constexpr uint32_t kAlphaBit         = 0x0004;
static constexpr uint32_t kSrgbBit          = 0x0008;
static constexpr uint32_t kCompressedBit    = 0x0010;
static constexpr uint32_t kLinearTilingBit  = 0x0020;
static constexpr uint32_t kBlueEndianBit    = 0x0040;
static constexpr uint32_t kAlphaFirstBit    = 0x0080;
static constexpr uint32_t kDepthBit         = 0x0100;
static constexpr uint32_t kStencilBit       = 0x0200;
static constexpr uint32_t kDepthStencilBits = kDepthBit | kStencilBit;

VkFormat SupportedFormat::get() const
{
	return fmt;
}

bool SupportedFormat::operator==(vtek::Format _format)
{
	return format == _format;
}

vtek::ImageChannels SupportedFormat::get_num_channels()
{
	// # channels stored in first two bits
	return static_cast<vtek::ImageChannels>(propertyMask & kChannelBits);
}

bool SupportedFormat::has_alpha()
{
	return propertyMask & kAlphaBit;
}

bool SupportedFormat::is_srgb()
{
	return propertyMask & kSrgbBit;
}

bool SupportedFormat::is_compressed()
{
	return propertyMask & kCompressedBit;
}

bool SupportedFormat::is_linear_tiling_supported()
{
	return propertyMask & kLinearTilingBit;
}

bool SupportedFormat::is_blue_endian()
{
	return propertyMask & kBlueEndianBit;
}

bool SupportedFormat::is_alpha_first()
{
	return propertyMask & kAlphaFirstBit;
}

bool SupportedFormat::is_depth_stencil()
{
	return propertyMask & kDepthStencilBits;
}

bool SupportedFormat::has_depth() const
{
	return propertyMask & kDepthBit;
}

bool SupportedFormat::has_stencil() const
{
	return propertyMask & kStencilBit;
}
