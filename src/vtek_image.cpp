#include "vtek_vulkan.pch"
#include "vtek_image.hpp"

#include "impl/vtek_vma_helpers.hpp"
#include "vtek_device.hpp"
#include "vtek_logging.hpp"

using IAFlag = vtek::ImageAspectFlag;


/* helper functions */
static VkImageAspectFlags get_image_aspect_flags(
	vtek::EnumBitmask<IAFlag> aspectFlags)
{
	VkImageAspectFlags flags = VK_IMAGE_ASPECT_NONE;

	if (aspectFlags.empty()) { return flags; }

	if (aspectFlags.has_flag(IAFlag::color))
	{
		flags |= VK_IMAGE_ASPECT_COLOR_BIT;
	}
	if (aspectFlags.has_flag(IAFlag::depth))
	{
		flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
	}
	if (aspectFlags.has_flag(IAFlag::stencil))
	{
		flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	// Sparse image
	if (aspectFlags.has_flag(IAFlag::metadata))
	{
		flags |= VK_IMAGE_ASPECT_METADATA_BIT;
	}

	if (aspectFlags.has_flag(IAFlag::plane_0))
	{
		if (flags & VK_IMAGE_ASPECT_COLOR_BIT)
		{
			vtek_log_error(
				"get_image_aspect_flags(): {}",
				"color may not be used together with multi-planar image formats!");
			vtek_log_warn("The plane_0 flag will be ignored -- {}",
			              "the application may not work correctly!");
			return flags;
		}
#if defined(VK_API_VERSION_1_1)
		flags |= VK_IMAGE_ASPECT_PLANE_0_BIT;
#else
		vtek_log_error(
			"get_image_aspect_flags(): {}",
			"Vulkan >= 1.1 is required for plane-0 multi-planar format!");
#endif
	}

	else if (aspectFlags.has_flag(IAFlag::plane_1))
	{
		if (flags & VK_IMAGE_ASPECT_COLOR_BIT)
		{
			vtek_log_error(
				"get_image_aspect_flags(): {}",
				"color may not be used together with multi-planar image formats!");
			vtek_log_warn("The plane_1 flag will be ignored -- {}",
			              "the application may not work correctly!");
			return flags;
		}
#if defined(VK_API_VERSION_1_1)
		flags |= VK_IMAGE_ASPECT_PLANE_1_BIT;
#else
		vtek_log_error(
			"get_image_aspect_flags(): {}",
			"Vulkan >= 1.1 is required for plane-1 multi-planar format!");
#endif
	}

	else if (aspectFlags.has_flag(IAFlag::plane_2))
	{
		if (flags & VK_IMAGE_ASPECT_COLOR_BIT)
		{
			vtek_log_error(
				"get_image_aspect_flags(): {}",
				"color may not be used together with multi-planar image formats!");
			vtek_log_warn("The plane_2 flag will be ignored -- {}",
			              "the application may not work correctly!");
			return flags;
		}
#if defined(VK_API_VERSION_1_3)
		flags |= VK_IMAGE_ASPECT_PLANE_2_BIT;
#else
		vtek_log_error(
			"get_image_aspect_flags(): {}",
			"Vulkan >= 1.3 is required for plane-2 multi-planar format!");
#endif
	}

	return flags;
}

static VkImageView create_image2d_view(
	vtek::Image2D* image, const vtek::Image2DViewInfo* viewInfo,
	vtek::Device* device)
{
	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image->vulkanHandle;

	// Specify how the image data should be interpreted. Images can be
	// treated as 1D textures, 2D textures, 3D textures, and cube maps,
	// and the `format` field specifies bit depth, ordering, and
	// interpretation of the color channels.
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = image->format;

	// With the `components` field we can swizzle the color channels
	// around, e.g. map all the channels to the red channel for a
	// monochrome texture, or map constant values of 0 and 1 to a
	// particular channel. Here, we use default mapping.
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	// The `subresourceRange` field describes the image's purpose,
	// and which part of the image should be accessed. Here, we solely
	// use images as color targets without any mipmapping or multilayer
	// setup.
	createInfo.subresourceRange.aspectMask =
		get_image_aspect_flags(viewInfo->aspectFlags);
	createInfo.subresourceRange.baseMipLevel = viewInfo->baseMipLevel;
	createInfo.subresourceRange.levelCount = 1; // TODO: Make configurable
	createInfo.subresourceRange.baseArrayLayer = viewInfo->baseArrayLayer;
	createInfo.subresourceRange.layerCount = 1; // TODO: Make configurable

	VkDevice dev = vtek::device_get_handle(device);

	VkImageView view;
	VkResult result = vkCreateImageView(dev, &createInfo, nullptr, &view);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to create image view for 2d image!");
		return VK_NULL_HANDLE;
	}

	return view;
}



/* interface */
vtek::Image2D* vtek::image2d_create(
	const vtek::Image2DInfo* info, vtek::Device* device)
{
	vtek::Allocator* allocator = vtek::device_get_allocator(device);
	if (allocator == nullptr)
	{
		vtek_log_error("Device does not have a default allocator -- {}",
		               "cannot create buffer!");
		return nullptr;
	}

	auto image = new vtek::Image2D;
	if (!vtek::allocator_image2d_create(allocator, info, image))
	{
		vtek_log_error("Failed to create 2D image!");
		delete image;
		return nullptr;
	}

	if (info->createImageView)
	{
		VkImageView view = create_image2d_view(image, &info->imageViewInfo, device);
		if (view == VK_NULL_HANDLE)
		{
			vtek_log_error("Image view creation failed -- cannot return image!");
			vtek::allocator_image2d_destroy(image);
			delete image;
			return nullptr;
		}

		image->viewHandle = view;
	}

	vtek_log_debug("2D image created!");
	return image;
}

void vtek::image2d_destroy(vtek::Image2D* image)
{
	if (image == nullptr) { return; }

	vtek::allocator_image2d_destroy(image);
}

vtek::Image2D* vtek::image2d_create(
	const vtek::Image2DInfo* info, vtek::Allocator* allocator)
{
	vtek_log_error("vtek::image2d_create(_, Allocator*): not implemented!");

	return nullptr;
}

VkImage vtek::image2d_get_handle(const vtek::Image2D* image)
{
	return image->vulkanHandle;
}

VkImageView vtek::image2d_get_view_handle(const vtek::Image2D* image)
{
	return image->viewHandle;
}
