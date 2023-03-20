// standard
#include <algorithm>
#include <vector>
#include <vulkan/vulkan.h>

// vtek
#include "vtek_device.h"
#include "vtek_host_allocator.h"
#include "vtek_logging.h"
#include "vtek_physical_device.h"
#include "vtek_swapchain.h"
#include "vtek_vulkan_helpers.h"


/* struct implementation */
struct vtek::Swapchain
{
	VkSwapchainKHR vulkanHandle {VK_NULL_HANDLE};
	uint32_t length {0};
	VkExtent2D imageExtent {0, 0};
	VkFormat imageFormat;
	VkFormat depthImageFormat;

	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;
	uint32_t currentImageIndex {0};

	bool isInvalidated {false};
};


/* host allocator */
static vtek::HostAllocator<vtek::Swapchain> sAllocator("vtek_swapchain");



/* helper data types */

// When the presentation queue is full, the application can either
// wait for an image to be ready, which will result in lower power
// usage; or replace an existing queued image, which will increase
// power usage but result in lower input latency and "newer" images
// being displayed. Replacing images should *only* be chosen when
// lower input latency is strictly required, otherwise waiting on
// images should be preferred (will be more performant).
enum class QueueFullPolicyType
{
	// Wait on the presentation queue when it is full, which will then
	// block the application (both CPU and GPU) until the next vertical
	// blank. This will result in lower power consumption, but may
	// result in higher perceived input latency.
	WaitForImage,
	// Replace queued images when the queue is full instead of waiting
	// for vertical blank. This will result in lower perceived input
	// latency at the cost of higher power consumption.
	// Using this mode is strongly discouraged for mobile devices,
	// but recommended for fast-paced input latency sensitive games.
	ReplaceQueuedImage
};

struct PresentModeOptions
{
	bool AllowScreenTearing;

	// This field only applies when `AllowScreenTearing` is true.
	// When true, rendered images are submitted for presentation
	// immediately without being placed in a presentation queue.
	// When false, images will be submitted to a queue for wait on
	// vertical blank except for when the queue is empty, in which
	// case images are drawn to screen immediately.
	// In either case screen tearing may occur.
	bool EnforceWaitOnVerticalBlank;

	// This field only applies when `AllowScreenTearing` is false,
	// and determines what to do when the presentation queue is full.
	QueueFullPolicyType QueueFullPolicy;
};



/* helper functions */
static VkSurfaceFormatKHR chooseSurfaceFormat(
	const std::vector<VkSurfaceFormatKHR>& supportedFormats)
{
	// If none of our criteria were met, we can "settle" for the first format
	// that is specified. This will be good enough in most cases.
	VkSurfaceFormatKHR chosenFormat = supportedFormats[0];

	// NOTE: Manually creating sRGB framebuffers is only suitable for forward
	// rendering.
	constexpr bool kSRGBFramebuffer = false;

	// Each `VkSurfaceFormatKHR` entry contains a format and a colorSpace
	// member. We prefer SRGB because it results in more accurate perceived
	// colors, and because it is a good standard.
	if (kSRGBFramebuffer)
	{
		for (const auto& format : supportedFormats)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
			    format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				chosenFormat = format;
				break;
			}
		}
	}
	else
	{
		for (const auto& format : supportedFormats)
		{
			if ((format.format == VK_FORMAT_R8G8B8A8_UNORM ||
			     format.format == VK_FORMAT_B8G8R8A8_UNORM))
			{
				chosenFormat = format;
				break;
			}
		}
	}

	return chosenFormat;
}

static VkPresentModeKHR choosePresentMode(
	const std::vector<VkPresentModeKHR>& supportedPresentModes,
	const PresentModeOptions& options)
{
	// The presentation mode represents conditions for showing images to the
	// screen. There are four possible modes available in Vulkan:
	// - VK_PRESENT_MODE_IMMEDIATE_KHR:
	//     Images submitted by the application are transferred to the screen
	//     right away, which may result in tearing.
	//     I don't know why you would want this.
	// - VK_PRESENT_MODE_FIFO_KHR:
	//     Swapchain is a queue, where the display takes an image from the
	//     front when the display is refreshed, and the program inserts
	//     rendered images at the back. If the queue is full the program has
	//     to wait. This is most similar to vertical sync found in games.
	// - VK_PRESENT_MODE_FIFO_RELAXED_KHR:
	//     Similar to the previous, except that rendered images are
	//     transferred to the screen immediately when they arrive if the
	//     queue is empty, without waiting for display refresh, which may
	//     result in visible tearing.
	//     This mode can be used successfully if rendering time often
	//     exceeds expected average (e.g. 60 FPS but often 18ms). In such
	//     cases, instead of waiting for vertical blank which may reduce
	//     to 30 FPS (bad!), images are presented immediately.
	// - VK_PRESENT_MODE_MAILBOX_KHR:
	//     Similar to second mode, but instead of blocking the application
	//     when queue is full, queued images are replaced with the newer
	//     ones. This mode can be used to implement triple buffering, which
	//     allows us to avoid tearing with significantly less latency issues
	//     than standard vertical sync with double buffering.
	//     In other words, the GPU doesn't block if its running faster than
	//     the display.
	//
	// Only `VK_PRESENT_MODE_FIFO_KHR` is guaranteed to be available, so we
	// need to check for available modes.
	auto begin = supportedPresentModes.begin();
	auto end = supportedPresentModes.end();
	bool mailboxSupported =
		std::find(begin, end, VK_PRESENT_MODE_MAILBOX_KHR) != end;
	bool immediateSupported =
		std::find(begin, end, VK_PRESENT_MODE_IMMEDIATE_KHR) != end;
	bool fifoRelaxedSupported =
		std::find(begin, end, VK_PRESENT_MODE_FIFO_RELAXED_KHR) != end;

	// 1) Screen tearing shall be avoided.
	if (!options.AllowScreenTearing)
	{
		if (options.QueueFullPolicy == QueueFullPolicyType::ReplaceQueuedImage)
		{
			if (mailboxSupported)
			{
				return VK_PRESENT_MODE_MAILBOX_KHR;
			}
			else
			{
				// Mailbox mode, and hence triple-buffering, is unfortunately not supported.
			}
		}
		// This mode is guaranteed to be supported, so no need to check for
		// it explicitly.
		return VK_PRESENT_MODE_FIFO_KHR;
	}
	// 2) Screen tearing shall be allowed.
	else
	{
		if (!options.EnforceWaitOnVerticalBlank && immediateSupported)
		{
			return VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
		else if (fifoRelaxedSupported)
		{
			// Immediate mode not supported, so we fallback to relaxed queue
			// mode, where we wait for vertical blank except in cases where the
			// presentation queue is empty, in which case we submit the images
			// for presentation immediately. This may result in visible tearing.
			return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
		}
	}

	// Never mind, screen tearing is not supported on the physical device.
	// But since screen tearing has been allowed, we must assume that strictly
	// waiting on vertical blank is not desired, so we first try to check for
	// compatibility with MAILBOX mode.
	if (mailboxSupported) return VK_PRESENT_MODE_MAILBOX_KHR;

	// When all else fails, we use default FIFO mode which is guaranteed to
	// be supported.
	return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D chooseImageExtent(
	const VkSurfaceCapabilitiesKHR& capabilities,
	const vtek::SwapchainCreateInfo* info)
{
	// The swap extent is the resolution of the swapchain images, and is
	// almost always equal to the window resolution in pixels.

	// The range of possible resolutions is defined in the struct
	// `VkSurfaceCapabilitiesKHR`. Vulkan tells us to match the resolution
	// of the window by setting the width and height members of the
	// `currentExtent` member, but some window managers allow us to differ
	// here, indicated by the special value `UINT32_MAX`, in which case we
	// pick the resolution that best matches the window within the min
	// and max allowed resolutions.
	if (capabilities.currentExtent.width == UINT32_MAX)
	{
		// Window resolutions are measured in virtual screen coordinates,
		// but swap image sizes are measured in pixels. These do not always
		// correspond, e.g. on high DPI displays such as Apple's Retina.
		// So after the window is created, we should query the window library
		// for the framebuffer size, which is always in pixels, and use this
		// value to determine an appropriate swap image size.
		VkExtent2D actualExtent = {
			static_cast<uint32_t>(info->framebufferWidth),
			static_cast<uint32_t>(info->framebufferHeight)
		};

		// Clamp the swap extent within the mininum and maximum allowed
		// image extents.
		actualExtent.width = std::max(
			capabilities.minImageExtent.width, std::min(
				capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(
			capabilities.minImageExtent.height, std::min(
				capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
	else
	{
		return capabilities.currentExtent;
	}
}

static bool createImageViews(vtek::Swapchain* swapchain, VkDevice dev)
{
	swapchain->imageViews.resize(swapchain->length, VK_NULL_HANDLE);

	for (uint32_t i = 0; i < swapchain->length; i++)
	{
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapchain->images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapchain->imageFormat;
		// We don't need to swizzle (swap around) any of the color channels
		createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_A;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		// No mipmapping for the swapchain images.
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		// No multiple layers for these images.
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;
		createInfo.flags = 0;

		VkResult result = vkCreateImageView(dev, &createInfo, nullptr, &swapchain->imageViews[i]);
		if (result != VK_SUCCESS)
		{
			return false;
		}
	}

	return true;
}

static void destroy_swapchain_image_views(vtek::Swapchain* swapchain, VkDevice dev)
{
	for (auto& imageView : swapchain->imageViews)
	{
		if (imageView != VK_NULL_HANDLE)
		{
			vkDestroyImageView(dev, imageView, nullptr);
		}
	}

	swapchain->images.clear();
	swapchain->imageViews.clear();
	swapchain->length = 0;
}

static void destroy_swapchain_handle(vtek::Swapchain* swapchain, VkDevice dev)
{
	if (swapchain->vulkanHandle != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(dev, swapchain->vulkanHandle, nullptr);
		swapchain->vulkanHandle = VK_NULL_HANDLE;
	}
}

static uint32_t chooseSwapchainLength(bool mayTripleBuffer, VkSurfaceCapabilitiesKHR& capabilities)
{
	const uint32_t minImageCount = capabilities.minImageCount;
	const uint32_t maxImageCount = capabilities.maxImageCount;
	uint32_t desiredLength = (mayTripleBuffer) ? 3 : 2;

	// Now decide how many images we want to have in the swapchain.
	// Sticking to the minimum means we may sometimes have to wait on the
	// driver to complete internal operations before we can aqcuire another
	// image to render to, so by recommendation we request at least one more
	// image than the minimum.
	//
	// We will also prioritize triple buffering to double buffering, since
	// that may result in higher, more consistent frame rates when rendering
	// takes longer than desired frame time.
	//
	// We should also make sure not to exceed the maximum number of images,
	// where 0 is a special value indicating that there is no maximum.
	uint32_t length = std::max(desiredLength, minImageCount);
	if (maxImageCount > 0 && length > maxImageCount)
	{
		length = maxImageCount;
	}

	return length;
}

static VkSurfaceTransformFlagBitsKHR choosePreTransform(VkSurfaceCapabilitiesKHR& capabilities)
{
	// We can specify that a certain transformation should be applied to
	// swapchain images, if it is supported (`supportedTransforms` in
	// `capabilities`), like a 90 degree clockwise rotation or horizontal
	// flip. If no such transformation is desired specify the current one.
	VkSurfaceTransformFlagBitsKHR current = capabilities.currentTransform;

	// TODO: Could be fun to try out some of the options available:
	// VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
	// VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR
	// VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR
	// VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR
	// VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR
	// VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR
	// VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR
	// VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR
	// VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR

	// https://www.intel.com/content/www/us/en/developer/articles/training/api-without-secrets-introduction-to-vulkan-part-2.html
	// On Some platforms we may want our image to be transformed. This is
	// usually the case on tablets when they are oriented in a way other than
	// their default orientation.
	// If the selected pre-transform is other than the current transformation
	// (also found in surface capabilities) the presentation engine will
	// apply the selected transformation. On some platforms this may cause
	// performance degradation (probably not noticeable but worth mentioning).
	// In the sample code [inserted below], I don't want any transformations
	// but, of course, I must check whether it is supported. If not, I'm
	// just using the same transformation that is currently used.
	if (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		return current;
	}
}

VkSurfaceCapabilitiesKHR getSurfaceCapabilities(
	VkPhysicalDevice physicalDevice, const VkSurfaceKHR& surface)
{
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

	return capabilities;
}

std::vector<VkSurfaceFormatKHR> getSupportedSurfaceFormats(
	VkPhysicalDevice physicalDevice, const VkSurfaceKHR& surface)
{
	uint32_t count;
	std::vector<VkSurfaceFormatKHR> formats;

	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, nullptr);
	if (count == 0)
	{
		vtek_log_error("Failed to find any supported surface formats for swapchain!");
		return formats;
	}

	formats.resize(count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &count, formats.data());
	return formats;
}

std::vector<VkPresentModeKHR> getSupportedPresentModes(
	VkPhysicalDevice physicalDevice, const VkSurfaceKHR& surface)
{
	uint32_t count;
	std::vector<VkPresentModeKHR> modes;

	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, nullptr);
	if (count == 0)
	{
		vtek_log_error("Failed to find any supported present modes for swapchain!");
		return modes;
	}

	modes.resize(count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &count, modes.data());
	return modes;
}



/* swapchain interface */
vtek::Swapchain* vtek::swapchain_create(
	const SwapchainCreateInfo* info, VkSurfaceKHR surface,
	const vtek::PhysicalDevice* physicalDevice, vtek::Device* device)
{
	VkDevice dev = vtek::device_get_handle(device);
	VkPhysicalDevice physDev = vtek::physical_device_get_handle(physicalDevice);

	// Simply checking if the swapchain extension is available is not
	// sufficient, because it may not be compatible with the window surface.
	// We need to check for the following 3 kinds of properties:
	// - Basic surface capabilities (min/max number of images in swapchain,
	//   min/max width and height of images).
	// - Surface formats (pixel format, color space)
	// - Available presentation modes
	auto capabilities = getSurfaceCapabilities(physDev, surface);
	auto formats = getSupportedSurfaceFormats(physDev, surface);
	auto presentModes = getSupportedPresentModes(physDev, surface);
	if ((formats.size() == 0) || (presentModes.size() == 0))
	{
		return nullptr;
	}

	// 1) choose swapchain surface format
	VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(formats);

	// 2) choose swapchain present mode
	PresentModeOptions options{};
	options.AllowScreenTearing = !(info->vsync);
	options.QueueFullPolicy =
		(info->prioritizeLowLatency)
		? QueueFullPolicyType::ReplaceQueuedImage
		: QueueFullPolicyType::WaitForImage;
	VkPresentModeKHR presentMode = choosePresentMode(presentModes, options);

	// 3) choose swapchain image extent
	VkExtent2D imageExtent = chooseImageExtent(capabilities, info);

	// Choose swapchain length
	bool mayTripleBuffer = (presentMode == VK_PRESENT_MODE_MAILBOX_KHR);
	uint32_t swapchainLength = chooseSwapchainLength(mayTripleBuffer, capabilities);

	// Create info struct
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = swapchainLength;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = imageExtent;
	createInfo.imageArrayLayers = 1;

	// There are 8 potential bits that can be used.
	// - VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT:
	//     This is a color image we're rendering to.
	// - VK_IMAGE_USAGE_TRANSFER_SRC_BIT:
	//     We'll be copying this image somewhere (screenshot, postprocess).
	// TODO: This needs to be modifiable!
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
	{
		// The swapchain will have support for screenshot capture
		// TODO: Probably create a flag for this so clients can know!
		createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}
	createInfo.presentMode = presentMode;

	// Specify how to handle swapchain images that will be used across
	// multiple queue families, e.g. if graphics queue family is different
	// from presentation queue family. We will be rendering to swapchain images
	// from the graphics queue and submitting them to the screen on the
	// presentation queue.
	// There are two ways to handle images that are accessed from multiple
	// queue families:
	// - VK_SHARING_MODE_EXCLUSIVE:
	//     An image is owned by one queue family at a time, and ownership
	//     must be explicitly transferred before it can be used in another
	//     queue family. This option offers the best performance.
	// - VK_SHARING_MODE_CONCURRENT:
	//     Images can be used across multiple queue families without
	//     explicit transferring of ownership.
	// TODO: What to do if we don't use graphics but instead present from compute queue?? baaaaah
	vtek::Queue* graphicsQueue = vtek::device_get_graphics_queue(device);
	vtek::Queue* presentQueue = vtek::device_get_present_queue(device);
	uint32_t qf_indices[2] = {
		vtek::queue_get_family_index(graphicsQueue),
		vtek::queue_get_family_index(presentQueue)
	};
	if (vtek::queue_is_same_family(graphicsQueue, presentQueue))
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}
	else
	{
		// Graphics and Present are two distinct queue families, so we pick
		// concurrent access to remove the need for manual transferring of
		// ownership of swapchain images between queue families.
		// REVIEW: This could be optimized at some point!
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = qf_indices;
	}

	createInfo.preTransform = choosePreTransform(capabilities);

	// The `compositeAlpha` field specifies if the alpha channel should be
	// used for blending with other windows in the window system. We leave
	// the window opaque (alpha is ignored).
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	// If `clipped` is set to `VK_TRUE`, then that means that we don't care
	// about pixels that are obscured, e.g. due to being occluded by another
	// application window in front of them. Unless we really need to read
	// these pixels back and get predictable results, we can obtain the best
	// performance by enabling clipping.
	// NEXT: Likely only important if an overlay performs screenshots!
	// REVIEW: Consider if we need to do something here!
	createInfo.clipped = VK_TRUE;

	// With Vulkan it's possible that the swapchain becomes invalid or
	// unoptimized while the application is running, e.g. due to resizing
	// the window. In such a case the swapchain needs to be re-created,
	// and a reference to the old one must be specified in this field.
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// Create the swap chain.
	vtek::Swapchain* swapchain = sAllocator.alloc();
	if (swapchain == nullptr)
	{
		vtek_log_error("Failed to allocate swapchain!");
		return nullptr;
	}

	VkResult result = vkCreateSwapchainKHR(
		dev, &createInfo, nullptr, &swapchain->vulkanHandle);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to create swapchain!");
		return nullptr;
	}

	// After the swap chain has been created, we query for the handles to
	// the images which we will reference during rendering operations.
	// No cleanup of these images is required - that is handled
	// automatically when the swap chain is destroyed.
	vkGetSwapchainImagesKHR(dev, swapchain->vulkanHandle, &swapchainLength, nullptr);
	swapchain->images.resize(swapchainLength, VK_NULL_HANDLE);
	vkGetSwapchainImagesKHR(dev, swapchain->vulkanHandle, &swapchainLength, swapchain->images.data());

	// Fill out the data members of the swapchain struct
	swapchain->imageFormat = surfaceFormat.format;
	swapchain->imageExtent = imageExtent;
	swapchain->length = swapchainLength;
	swapchain->isInvalidated = false;

	// Create image views
	if (!createImageViews(swapchain, dev))
	{
		vtek_log_error("Failed to create swapchain image views!");
		return nullptr;
	}

	// Determine image format for framebuffer depth attachments.
	// Even though depth images are not used by the swapchain, they are
	// linked to the framebuffers that use the swapchain, so this is a
	// logical place to determine depth format.
	std::vector<VkFormat> depthFormatCandidates = {
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT
	};
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

	if (!findSupportedImageFormat(
		    physDev, depthFormatCandidates, tiling, features, &swapchain->depthImageFormat))
	{
		vtek_log_error("Failed to find supported depth image format!");
		return nullptr;
	}

	// All done
	return swapchain;
}

bool vtek::swapchain_recreate(vtek::Swapchain* swapchain)
{
	vtek_log_error("vtek::swapchain_recreate(): Not implemented!");
	return false;
}

void vtek::swapchain_destroy(vtek::Swapchain* swapchain, const vtek::Device* device)
{
	if (swapchain == nullptr || swapchain->vulkanHandle == VK_NULL_HANDLE) return;

	VkDevice dev = vtek::device_get_handle(device);

	destroy_swapchain_image_views(swapchain, dev);
	destroy_swapchain_handle(swapchain, dev);
	swapchain->isInvalidated = false;

	sAllocator.free(swapchain);
}

bool vtek::swapchain_acquire_next_image_index(vtek::Swapchain* swapchain, uint32_t* outImageIndex)
{
	return false;
}

bool vtek::swapchain_present_image(vtek::Swapchain* swapchain, uint32_t presentImageIndex)
{
	return false;
}
