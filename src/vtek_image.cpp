#include "vtek_vulkan.pch"
#include "vtek_image.hpp"

#include "impl/vtek_vma_helpers.hpp"
#include "vtek_device.hpp"
#include "vtek_logging.hpp"


/* helper functions */



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
	vtek_log_error("vtek::image2d_get_view_handle(): not implemented!");

	return VK_NULL_HANDLE;
}
