#include "vtek_vulkan.pch"
#include "vtek_swapchain.hpp"

#include "imgutils/vtek_image_formats.hpp"
#include "vtek_command_buffer.hpp"
#include "vtek_device.hpp"
#include "vtek_image.hpp"
#include "vtek_logging.hpp"
#include "vtek_physical_device.hpp"
#include "vtek_queue.hpp"
#include "vtek_submit_info.hpp"

#include <algorithm>
#include <functional>
#include <vector>
#include <vulkan/vulkan.h>


/* struct implementation */
using tDynRenderBegin = std::function<
	void(vtek::Swapchain*, uint32_t, VkCommandBuffer, glm::vec3)>;

struct vtek::Swapchain
{
	// ============================ //
	// === Swapchain properties === //
	// ============================ //
	VkSwapchainKHR vulkanHandle {VK_NULL_HANDLE};
	uint32_t length {0};
	VkExtent2D imageExtent {0, 0};
	vtek::Format imageFormat {vtek::Format::undefined};
	vtek::Format depthImageFormat {vtek::Format::undefined};
	SwapchainDepthBuffer depthBufferType {vtek::SwapchainDepthBuffer::none};
	tDynRenderBegin fDynRenderBegin;

	bool isInvalidated {false};

	VkQueue presentQueue {VK_NULL_HANDLE};
	uint32_t graphicsQueueIndex {0};
	uint32_t presentQueueIndex {0};

	// =================== //
	// === Attachments === //
	// =================== //
	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;
	uint32_t currentImageIndex {0};

	std::vector<vtek::Image2D*> depthImages;
	std::vector<VkImageView> depthImageViews;

	// ============================ //
	// === Frame syncronization === //
	// ============================ //
	uint32_t numFramesInFlight {0U};
	uint32_t currentFrameIndex {0U};

	// Semaphores that will be signaled once a swapchain image becomes ready.
	// Used for acquiring swapchain images.
	VkSemaphore imageAvailableSemaphores[vtek::kMaxFramesInFlight];

	// Semaphores that will be signaled once rendering of a frame has completed.
	// Used for releasing swapchain images to the presentation queue.
	VkSemaphore renderFinishedSemaphores[vtek::kMaxFramesInFlight];

	// Before we can start drawing a frame, the CPU needs to wait for
	// the swapchain image to become ready. This is accomplished by
	// having this list of frames which guard the current frame
	// Before we can draw, we need to wait for fences guarding the current
	// swap chain image.
	VkFence inFlightFences[vtek::kMaxFramesInFlight];

	// This vector will be created to the length of the swapchain,
	// and for each swapchain image hold a pointer to a fence in
	// `inFlightFences`. Used for signalling when it's possible to
	// render to a swapchain image.
	std::vector<VkFence> imagesInFlight;

	// ====================== //
	// === Swapchain info === //
	// ====================== //
	bool vsync {false};
	bool prioritizeLowLatency {false};
	VkPhysicalDevice physDev {VK_NULL_HANDLE};
};



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
static VkSurfaceFormatKHR choose_surface_format(
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

static VkPresentModeKHR choose_present_mode(
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

static VkExtent2D choose_image_extent(
	const VkSurfaceCapabilitiesKHR& capabilities,
	uint32_t framebufferWidth, uint32_t framebufferHeight)
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
		VkExtent2D actualExtent = { framebufferWidth, framebufferHeight };

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

static bool create_image_views(vtek::Swapchain* swapchain, VkDevice dev)
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
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; // VK_COMPONENT_SWIZZLE_R;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY; // VK_COMPONENT_SWIZZLE_G;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY; // VK_COMPONENT_SWIZZLE_B;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY; // VK_COMPONENT_SWIZZLE_A;
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

static bool create_depth_images(
	vtek::Swapchain* swapchain, vtek::Device* device, uint32_t count,
	std::vector<const vtek::Queue*>&& queues)
{
	vtek::Image2DInfo imageInfo{};
	imageInfo.requireDedicatedAllocation = true;
	imageInfo.extent = swapchain->imageExtent;
	imageInfo.format = swapchain->depthImageFormat;
	imageInfo.usageFlags = vtek::ImageUsageFlag::depth_stencil_attachment;
	imageInfo.initialLayout = vtek::ImageInitialLayout::undefined;
	imageInfo.useMipmaps = false;
	imageInfo.multisampling = vtek::MultisampleType::none;
	if (queues.size() > 1)
	{
		imageInfo.sharingMode = vtek::ImageSharingMode::concurrent;
		imageInfo.sharingQueues = queues;
	}
	else
	{
		imageInfo.sharingMode = vtek::ImageSharingMode::exclusive;
	}
	imageInfo.createImageView = true;
	// NOTE: We choose to ignore the stencil aspect, if present in format, because
	// we care only about depth buffering here.
	imageInfo.imageViewInfo.aspectFlags = vtek::ImageAspectFlag::depth;

	swapchain->depthImages.resize(count, nullptr);
	swapchain->depthImageViews.resize(count, VK_NULL_HANDLE);

	for (uint32_t i = 0; i < count; i++)
	{
		vtek::Image2D* image = vtek::image2d_create(&imageInfo, device);
		if (image == nullptr)
		{
			vtek_log_error("Failed to create swapchain depth image {} -- {}",
			               i, "cannot complete swapchain creation!");
			return false;
		}

		swapchain->depthImages[i] = image;
		swapchain->depthImageViews[i] = vtek::image2d_get_view_handle(image);
	}
	return true;
}

static void destroy_depth_images(vtek::Swapchain* swapchain, vtek::Device* device)
{
	for (auto& image : swapchain->depthImages)
	{
		vtek::image2d_destroy(image, device);
	}

	swapchain->depthImageViews.clear();
	swapchain->depthImages.clear();
}

static uint32_t choose_swapchain_length(bool mayTripleBuffer, VkSurfaceCapabilitiesKHR& capabilities)
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

static VkSurfaceTransformFlagBitsKHR choose_pre_transform(VkSurfaceCapabilitiesKHR& capabilities)
{
	// We can specify that a certain transformation should be applied to
	// swapchain images, if it is supported (`supportedTransforms` in
	// `capabilities`), like a 90 degree clockwise rotation or horizontal
	// flip. If no such transformation is desired specify the current one.
	VkSurfaceTransformFlagBitsKHR current = capabilities.currentTransform;

	// NOTE: Alternative options, which may only be picked if the are supported,
	// as queried for by `vkGetPhysicalDeviceSurfaceCapabilitiesKHR`.
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

static VkSurfaceCapabilitiesKHR get_surface_capabilities(
	VkPhysicalDevice physicalDevice, const VkSurfaceKHR& surface)
{
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

	return capabilities;
}

static std::vector<VkSurfaceFormatKHR> get_supported_surface_formats(
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

static std::vector<VkPresentModeKHR> get_supported_present_modes(
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

static bool get_supported_depth_format(
	const vtek::Device* device, vtek::SupportedFormat& out)
{
	// Determine image format for framebuffer depth attachments.
	// Even though depth images are not used by the swapchain, they are
	// linked to the framebuffers that use the swapchain, so this is a
	// logical place to determine depth format.
	std::vector<vtek::Format> depthFormatCandidates = {
		// Place first because it's more performant, and should be used unless
		// the extra precision is really needed.
		vtek::Format::d24_unorm_s8_uint,
		vtek::Format::d32_sfloat,
		vtek::Format::d32_sfloat_s8_uint
	};

}

static bool create_frame_sync_objects(vtek::Swapchain* swapchain, VkDevice dev)
{
	const uint32_t numFrames = std::clamp(swapchain->length - 1, 1U, vtek::kMaxFramesInFlight);
	swapchain->numFramesInFlight = numFrames;
	swapchain->imagesInFlight.resize(swapchain->length, VK_NULL_HANDLE);
	swapchain->currentFrameIndex = 0U;

	const VkSemaphoreCreateInfo semInfo {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0U // reserved for future use
	};

	// All fences are created in signaled state, to avoid initial wait before any rendering.
	const VkFenceCreateInfo fenceInfo {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_FENCE_CREATE_SIGNALED_BIT
	};

	for (uint32_t i = 0; i < vtek::kMaxFramesInFlight; i++)
	{
		vkCreateSemaphore(dev, &semInfo, nullptr, &swapchain->imageAvailableSemaphores[i]);
		vkCreateSemaphore(dev, &semInfo, nullptr, &swapchain->renderFinishedSemaphores[i]);
		vkCreateFence(dev, &fenceInfo, nullptr, &swapchain->inFlightFences[i]);
	}

	return true;
}

static void destroy_frame_sync_objects(vtek::Swapchain* swapchain, VkDevice dev)
{
	for (uint32_t i = 0; i < vtek::kMaxFramesInFlight; i++)
	{
		vkDestroySemaphore(dev, swapchain->imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(dev, swapchain->renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(dev, swapchain->inFlightFences[i], nullptr);
	}

	for (auto& fence : swapchain->imagesInFlight)
	{
		fence = VK_NULL_HANDLE;
	}
	swapchain->imagesInFlight.clear();

	swapchain->numFramesInFlight = 0U;
	swapchain->currentFrameIndex = 0U;
}

static void reset_frame_sync_objects(vtek::Swapchain* swapchain, VkDevice dev)
{
	for (uint32_t i = 0; i < vtek::kMaxFramesInFlight; i++)
	{
		VkFence fence = swapchain->inFlightFences[i];
		VkResult waitResult = vkWaitForFences(dev, 1, &fence, VK_TRUE, 0UL);
		if (waitResult == VK_TIMEOUT)
		{
			vkResetFences(dev, 1, &fence);
		}
	}

	const uint32_t numFrames = std::clamp(swapchain->length - 1, 1U, vtek::kMaxFramesInFlight);
	swapchain->numFramesInFlight = numFrames;
	swapchain->imagesInFlight.resize(numFrames, VK_NULL_HANDLE);
	swapchain->currentFrameIndex = 0U;
}

static void set_image_in_use(
	vtek::Swapchain* swapchain, VkDevice dev, uint32_t frameIndex)
{
	// Mark the image fence as in-use by this frame
	uint32_t currentFrame = swapchain->currentFrameIndex;
	VkFence targetFence = swapchain->inFlightFences[currentFrame];

	swapchain->imagesInFlight[frameIndex] = targetFence;

	// Reset the fence for use on the next frame
	vkResetFences(dev, 1, &targetFence);
}



/* dynamic rendering */
static void dynrender_begin_no_depth(
	vtek::Swapchain* swapchain, uint32_t imageIndex,
	VkCommandBuffer cmdBuf, glm::vec3 clearColor)
{
	const glm::vec3& c = clearColor;
	VkExtent2D extent = swapchain->imageExtent;

	// Transition from whatever (probably present src) to color attachment
	VkImageMemoryBarrier barrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = nullptr,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.srcQueueFamilyIndex = swapchain->presentQueueIndex,
		.dstQueueFamilyIndex = swapchain->graphicsQueueIndex,
		.image = swapchain->images[imageIndex],
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	vkCmdPipelineBarrier(
		cmdBuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr,
		0, nullptr, 1, &barrier);

	VkRenderingAttachmentInfo colorAttachmentInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO, // _KHR ??
		.pNext = nullptr,
		.imageView = swapchain->imageViews[imageIndex],
		.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
		.resolveMode = VK_RESOLVE_MODE_NONE,
		.resolveImageView = VK_NULL_HANDLE,
		.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = { .color = { .float32 = {c.x, c.y, c.z, 1.0f} } },
	};

	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.pNext = nullptr;
	renderingInfo.flags = 0;
	renderingInfo.renderArea = { 0U, 0U, extent.width, extent.height };
	renderingInfo.layerCount = 1;
	renderingInfo.viewMask = 0;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachmentInfo;
	renderingInfo.pDepthAttachment = nullptr;
	renderingInfo.pStencilAttachment = nullptr;

	vkCmdBeginRendering(cmdBuf, &renderingInfo);
}

static void dynrender_begin_shared_depth(
	vtek::Swapchain* swapchain, uint32_t imageIndex,
	VkCommandBuffer cmdBuf, glm::vec3 clearColor)
{
	const glm::vec3& c = clearColor;
	VkExtent2D extent = swapchain->imageExtent;

	// Transition from whatever (probably present src) to color attachment
	VkImageMemoryBarrier barrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = nullptr,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.srcQueueFamilyIndex = swapchain->presentQueueIndex,
		.dstQueueFamilyIndex = swapchain->graphicsQueueIndex,
		.image = swapchain->images[imageIndex],
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};
	vkCmdPipelineBarrier(
		cmdBuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr,
		0, nullptr, 1, &barrier);

	// NOTE: Discussion regarding the depth barrier on StackOverflow:
	// https://stackoverflow.com/a/62398311/6572223
	constexpr VkAccessFlags depthSrcAccessMask
		= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	constexpr VkAccessFlags depthDstAccessMask
		= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
		| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	constexpr VkPipelineStageFlags depthSrcStageMask
		= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
		| VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	constexpr VkPipelineStageFlags depthDstStageMask = depthSrcStageMask;

	VkImageMemoryBarrier depthBarrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = nullptr,
		.srcAccessMask = depthSrcAccessMask,
		.dstAccessMask = depthDstAccessMask,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		.srcQueueFamilyIndex = swapchain->graphicsQueueIndex,
		.dstQueueFamilyIndex = swapchain->graphicsQueueIndex,
		.image = vtek::image2d_get_handle(swapchain->depthImages[0]),
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};
	vkCmdPipelineBarrier(
		cmdBuf, depthSrcStageMask,
		depthDstStageMask, 0, 0, nullptr,
		0, nullptr, 1, &depthBarrier);

	VkRenderingAttachmentInfo colorAttachmentInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO, // _KHR ??
		.pNext = nullptr,
		.imageView = swapchain->imageViews[imageIndex],
		.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
		.resolveMode = VK_RESOLVE_MODE_NONE,
		.resolveImageView = VK_NULL_HANDLE,
		.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = { .color = { .float32 = {c.x, c.y, c.z, 1.0f} } },
	};
	VkRenderingAttachmentInfo depthStencilAttachmentInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO, // _KHR ??
		.pNext = nullptr,
		.imageView = swapchain->depthImageViews[0],
		.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		.resolveMode = VK_RESOLVE_MODE_NONE, // TODO: Multisampling?
		.resolveImageView = VK_NULL_HANDLE,
		.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = { .depthStencil = { 1.0f, 0 } },
	};

	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.pNext = nullptr;
	renderingInfo.flags = 0;
	renderingInfo.renderArea = { 0U, 0U, extent.width, extent.height };
	renderingInfo.layerCount = 1;
	renderingInfo.viewMask = 0;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachmentInfo;
	renderingInfo.pDepthAttachment = &depthStencilAttachmentInfo;
	renderingInfo.pStencilAttachment = nullptr;

	vkCmdBeginRendering(cmdBuf, &renderingInfo);
}

static void dynrender_begin_oneperimage_depth(
	vtek::Swapchain* swapchain, uint32_t imageIndex,
	VkCommandBuffer cmdBuf, glm::vec3 clearColor)
{
	const glm::vec3& c = clearColor;
	VkExtent2D extent = swapchain->imageExtent;

	// Transition from whatever (probably present src) to color attachment
	VkImageMemoryBarrier barrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = nullptr,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.srcQueueFamilyIndex = swapchain->presentQueueIndex,
		.dstQueueFamilyIndex = swapchain->graphicsQueueIndex,
		.image = swapchain->images[imageIndex],
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};
	VkImageMemoryBarrier depthBarrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = nullptr,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		.srcQueueFamilyIndex = swapchain->graphicsQueueIndex,
		.dstQueueFamilyIndex = swapchain->graphicsQueueIndex,
		.image = vtek::image2d_get_handle(swapchain->depthImages[imageIndex]),
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	vkCmdPipelineBarrier(
		cmdBuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr,
		0, nullptr, 1, &barrier);
	vkCmdPipelineBarrier(
		cmdBuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr,
		0, nullptr, 1, &depthBarrier);

	VkRenderingAttachmentInfo colorAttachmentInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO, // _KHR ??
		.pNext = nullptr,
		.imageView = swapchain->imageViews[imageIndex],
		.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
		.resolveMode = VK_RESOLVE_MODE_NONE,
		.resolveImageView = VK_NULL_HANDLE,
		.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = { .color = { .float32 = {c.x, c.y, c.z, 1.0f} } },
	};
	VkRenderingAttachmentInfo depthStencilAttachmentInfo{
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO, // _KHR ??
		.pNext = nullptr,
		.imageView = swapchain->depthImageViews[imageIndex],
		.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		.resolveMode = VK_RESOLVE_MODE_NONE, // TODO: Multisampling?
		.resolveImageView = VK_NULL_HANDLE,
		.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.clearValue = { .depthStencil = { 1.0f, 0 } },
	};

	VkRenderingInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.pNext = nullptr;
	renderingInfo.flags = 0;
	renderingInfo.renderArea = { 0U, 0U, extent.width, extent.height };
	renderingInfo.layerCount = 1;
	renderingInfo.viewMask = 0;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachmentInfo;
	renderingInfo.pDepthAttachment = &depthStencilAttachmentInfo;
	renderingInfo.pStencilAttachment = nullptr;

	vkCmdBeginRendering(cmdBuf, &renderingInfo);
}



/* swapchain interface */
vtek::Swapchain* vtek::swapchain_create(
	const SwapchainInfo* info, VkSurfaceKHR surface,
	const vtek::PhysicalDevice* physicalDevice, vtek::Device* device)
{
	VkDevice dev = vtek::device_get_handle(device);
	VkPhysicalDevice physDev = vtek::physical_device_get_handle(physicalDevice);


	// ========================= //
	// === Swapchain support === //
	// ========================= //
	auto extensions = vtek::device_get_enabled_extensions(device);
	if (!extensions->swapchain)
	{
		vtek_log_error("Swapchain extension not enabled during device creation.");
		vtek_log_error("--> Cannot create swapchain!");
		return nullptr;
	}


	// =============================== //
	// === Swapchain configuration === //
	// =============================== //

	// Simply checking if the swapchain extension is available is not
	// sufficient, because it may not be compatible with the window surface.
	// We need to check for the following 3 kinds of properties:
	// - Basic surface capabilities (min/max number of images in swapchain,
	//   min/max width and height of images).
	// - Surface formats (pixel format, color space)
	// - Available presentation modes
	auto capabilities = get_surface_capabilities(physDev, surface);
	auto formats = get_supported_surface_formats(physDev, surface);
	auto presentModes = get_supported_present_modes(physDev, surface);
	if ((formats.size() == 0) || (presentModes.size() == 0))
	{
		return nullptr;
	}

	// 1) choose swapchain surface format
	VkSurfaceFormatKHR surfaceFormat = choose_surface_format(formats);

	// 2) choose swapchain present mode
	PresentModeOptions options{};
	options.AllowScreenTearing = !(info->vsync);
	options.QueueFullPolicy =
		(info->prioritizeLowLatency)
		? QueueFullPolicyType::ReplaceQueuedImage
		: QueueFullPolicyType::WaitForImage;
	VkPresentModeKHR presentMode = choose_present_mode(presentModes, options);

	// 3) choose swapchain image extent
	VkExtent2D imageExtent = choose_image_extent(
		capabilities, info->framebufferWidth, info->framebufferHeight);

	// Choose swapchain length
	bool mayTripleBuffer = (presentMode == VK_PRESENT_MODE_MAILBOX_KHR);
	uint32_t swapchainLength = choose_swapchain_length(mayTripleBuffer, capabilities);

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
	if (graphicsQueue == nullptr)
	{
		vtek_log_error("Failed to retrieve graphics queue, cannot create swapchain!");
		return nullptr;
	}
	vtek::Queue* presentQueue = vtek::device_get_present_queue(device);
	if (presentQueue == nullptr)
	{
		vtek_log_error("Failed to retrieve present queue, cannot create swapchain!");
		return nullptr;
	}

	std::vector<const vtek::Queue*> queues;
	const uint32_t qf_indices[2] = {
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

		queues.push_back(graphicsQueue);
		queues.push_back(presentQueue);
	}

	createInfo.preTransform = choose_pre_transform(capabilities);

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


	// ========================= //
	// === Create swap chain === //
	// ========================= //
	auto swapchain = new vtek::Swapchain;

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
	swapchain->presentQueue = vtek::queue_get_handle(presentQueue);
	swapchain->graphicsQueueIndex = qf_indices[0];
	swapchain->presentQueueIndex = qf_indices[1];
	swapchain->vsync = info->vsync;
	swapchain->prioritizeLowLatency = info->prioritizeLowLatency;
	swapchain->physDev = physDev;


	// ========================== //
	// === Create image views === //
	// ========================== //
	if (!create_image_views(swapchain, dev))
	{
		vtek_log_error("Failed to create swapchain image views!");
		return nullptr;
	}


	// ======================= //
	// === Depth buffering === //
	// ======================= //

	// Determine image format for framebuffer depth attachments.
	// Even though depth images are not used by the swapchain, they are
	// linked to the framebuffers that use the swapchain, so this is a
	// logical place to determine depth format.
	std::vector<VkFormat> depthFormatCandidates = {
		// Place first because it's more performant, and should be used unless
		// the extra precision is really needed.
		VK_FORMAT_D24_UNORM_S8_UINT,

		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D32_SFLOAT_S8_UINT
	};
	VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
	VkFormatFeatureFlags features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

	if (!vtek::find_supported_image_format(
		physDev, depthFormatCandidates, tiling, features,
		&swapchain->depthImageFormat))
	{
		vtek_log_error("Failed to find supported depth image format!");
		return nullptr;
	}

	switch (info->depthBuffer)
	{
	case vtek::SwapchainDepthBuffer::none:
		swapchain->fDynRenderBegin = dynrender_begin_no_depth;
		swapchain->depthBufferType = vtek::SwapchainDepthBuffer::none;
		break;
	case vtek::SwapchainDepthBuffer::single_shared:
		if (!create_depth_images(swapchain, device, 1, std::move(queues)))
		{
			vtek_log_error("Failed to create shared swapchain depth buffer!");
			destroy_swapchain_image_views(swapchain, dev);
			destroy_swapchain_handle(swapchain, dev);
			delete swapchain;
			return nullptr;
		}
		swapchain->fDynRenderBegin = dynrender_begin_shared_depth;
		swapchain->depthBufferType = vtek::SwapchainDepthBuffer::single_shared;
		break;
	case vtek::SwapchainDepthBuffer::one_per_image:
		if (!create_depth_images(swapchain, device, swapchain->length, std::move(queues)))
		{
			vtek_log_error("Failed to create swapchain depth buffers!");
			destroy_swapchain_image_views(swapchain, dev);
			destroy_swapchain_handle(swapchain, dev);
			delete swapchain;
			return nullptr;
		}
		swapchain->fDynRenderBegin = dynrender_begin_oneperimage_depth;
		swapchain->depthBufferType = vtek::SwapchainDepthBuffer::one_per_image;
		break;
	default:
		vtek_log_error("swapchain_create(): Unrecognized depth buffer type!");
		swapchain->depthBufferType = vtek::SwapchainDepthBuffer::none;
		swapchain->fDynRenderBegin = dynrender_begin_no_depth;
		break;
	}


	// ================================= //
	// === Create frame sync objects === //
	// ================================= //
	if (!create_frame_sync_objects(swapchain, dev))
	{
		vtek_log_error("Failed to create swapchain frame sync objects!");
		return nullptr;
	}

	// All done
	return swapchain;
}

bool vtek::swapchain_recreate(
	vtek::Swapchain* swapchain, vtek::Device* device, VkSurfaceKHR surface,
	uint32_t framebufferWidth, uint32_t framebufferHeight)
{
	vtek_log_trace("vtek::swapchain_recreate()");
	VkDevice dev = vtek::device_get_handle(device);
	VkPhysicalDevice physDev = swapchain->physDev;

	// =============================== //
	// === Swapchain configuration === //
	// =============================== //

	auto capabilities = get_surface_capabilities(physDev, surface);
	auto formats = get_supported_surface_formats(physDev, surface);
	auto presentModes = get_supported_present_modes(physDev, surface);
	if ((formats.size() == 0) || (presentModes.size() == 0))
	{
		return false;
	}

	// 1) choose swapchain surface format
	VkSurfaceFormatKHR surfaceFormat = choose_surface_format(formats);

	// 2) choose swapchain present mode
	PresentModeOptions options{};
	options.AllowScreenTearing = !(swapchain->vsync);
	options.QueueFullPolicy =
		(swapchain->prioritizeLowLatency)
		? QueueFullPolicyType::ReplaceQueuedImage
		: QueueFullPolicyType::WaitForImage;
	VkPresentModeKHR presentMode = choose_present_mode(presentModes, options);

	// 3) choose swapchain image extent
	VkExtent2D imageExtent = choose_image_extent(
		capabilities, framebufferWidth, framebufferHeight);

	// Choose swapchain length
	bool mayTripleBuffer = (presentMode == VK_PRESENT_MODE_MAILBOX_KHR);
	uint32_t swapchainLength = choose_swapchain_length(mayTripleBuffer, capabilities);

	// Create info struct
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = swapchainLength;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = imageExtent;
	createInfo.imageArrayLayers = 1;
	// TODO: This needs to be modifiable!
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (capabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
	{
		// The swapchain will have support for screenshot capture
		// TODO: Probably create a flag for this so clients can know!
		createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}
	createInfo.presentMode = presentMode;

	vtek::Queue* graphicsQueue = vtek::device_get_graphics_queue(device);
	vtek::Queue* presentQueue = vtek::device_get_present_queue(device);

	std::vector<const vtek::Queue*> queues;
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
		// NOTE: We should prefer exclusive sharing mode whenever possible,
		// becuase concurrent mode disables DCC compression.
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = qf_indices;

		queues.push_back(graphicsQueue);
		queues.push_back(presentQueue);
	}

	createInfo.preTransform = choose_pre_transform(capabilities);
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = swapchain->vulkanHandle;

	// (re-)Create the swapchain
	VkSwapchainKHR newSwapchain;
	VkResult result = vkCreateSwapchainKHR(
		dev, &createInfo, nullptr, &newSwapchain);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to re-create swapchain!");
		return false;
	}

	// Cleanup the old swapchain images and handle
	destroy_swapchain_image_views(swapchain, dev);
	destroy_swapchain_handle(swapchain, dev);
	destroy_depth_images(swapchain, device);

	vkGetSwapchainImagesKHR(dev, newSwapchain, &swapchainLength, nullptr);
	swapchain->images.resize(swapchainLength, VK_NULL_HANDLE);
	vkGetSwapchainImagesKHR(
		dev, newSwapchain, &swapchainLength, swapchain->images.data());

	// Fill out the data members of the swapchain struct
	swapchain->vulkanHandle = newSwapchain;
	swapchain->imageFormat = surfaceFormat.format;
	swapchain->imageExtent = imageExtent;
	swapchain->length = swapchainLength;
	swapchain->isInvalidated = false;
	swapchain->graphicsQueueIndex = qf_indices[0];

	// Create image views
	if (!create_image_views(swapchain, dev))
	{
		vtek_log_error("Failed to re-create swapchain image views!");
		return false;
	}

	// Depth buffer format
	std::vector<vtek::Format> depthFormatCandidates = {
		vtek::Format::d32_sfloat,
		vtek::Format::d32_sfloat_s8_uint,
		vtek::Format::d24_unorm_s8_uint
	};
	vtek::FormatInfo depthFormatInfo{};
	depthFormatInfo.tiling = vtek::ImageTiling::optimal;
	depthFormatInfo.features = vtek::FormatFeature::depth_stencil_attachment;

	vtek::SupportedFormat supportedDepthFormat;
	if (!vtek::SupportedFormat::FindFormat(
		    &depthFormatInfo,))

	if (!vtek::find_supported_image_format(
		    physDev, depthFormatCandidates, tiling, features,
		    &swapchain->depthImageFormat))
	{
		vtek_log_error("Failed to find supported depth image format!");
		return false;
	}

	// Create depth images
	switch (swapchain->depthBufferType)
	{
	case vtek::SwapchainDepthBuffer::single_shared:
		if (!create_depth_images(swapchain, device, 1, std::move(queues)))
		{
			vtek_log_error("Failed to re-create shared swapchain depth buffer!");
			return false;
		}
		break;
	case vtek::SwapchainDepthBuffer::one_per_image:
		if (!create_depth_images(
			    swapchain, device, swapchain->length, std::move(queues)))
		{
			vtek_log_error("Failed to re-create swapchain depth buffers!");
			return false;
		}
		break;
	default:
		break;
	}

	// Reset frame sync objects
	reset_frame_sync_objects(swapchain, dev);

	// All done
	return true;
}

void vtek::swapchain_destroy(vtek::Swapchain* swapchain, vtek::Device* device)
{
	if (swapchain == nullptr || swapchain->vulkanHandle == VK_NULL_HANDLE) return;

	VkDevice dev = vtek::device_get_handle(device);

	destroy_frame_sync_objects(swapchain, dev);

	destroy_swapchain_image_views(swapchain, dev);
	destroy_depth_images(swapchain, device);
	destroy_swapchain_handle(swapchain, dev);
	swapchain->isInvalidated = false;

	delete swapchain;
}

uint32_t vtek::swapchain_get_length(vtek::Swapchain* swapchain)
{
	return swapchain->length;
}

VkImage vtek::swapchain_get_image(vtek::Swapchain* swapchain, uint32_t index)
{
	return swapchain->images[index];
}

VkImageView vtek::swapchain_get_image_view(vtek::Swapchain* swapchain, uint32_t index)
{
	return swapchain->imageViews[index];
}

VkFormat vtek::swapchain_get_image_format(vtek::Swapchain* swapchain)
{
	return swapchain->imageFormat;
}

VkExtent2D vtek::swapchain_get_image_extent(const vtek::Swapchain* swapchain)
{
	return swapchain->imageExtent;
}

bool vtek::swapchain_has_depth_buffer(vtek::Swapchain* swapchain)
{
	return swapchain->depthImages.size() > 0;
}

VkFormat vtek::swapchain_get_depth_image_format(vtek::Swapchain* swapchain)
{
	return swapchain->depthImageFormat;
}

vtek::SwapchainStatus vtek::swapchain_wait_begin_frame(
	vtek::Swapchain* swapchain, vtek::Device* device, uint64_t timeout)
{
	// In here, we wait for the fence guarding the current frame index to be in
	// signaled state, after which the frame may commence.

	VkDevice dev = vtek::device_get_handle(device);
	uint32_t index = swapchain->currentFrameIndex;
	VkFence fence = swapchain->inFlightFences[index];

	// Quoting the spec:
	// If timeout is zero, then vkWaitForFences does not wait, but simply returns
	// the current state of the fences. VK_TIMEOUT will be returned in this case
	// if the condition is not satisfied, even though no actual wait was performed.
	VkResult test = vkWaitForFences(dev, 1, &fence, VK_TRUE, 0UL);
	if (test == VK_SUCCESS) { return vtek::SwapchainStatus::ok; }

	VkResult result = vkWaitForFences(dev, 1, &fence, VK_TRUE, timeout);
	switch (result)
	{
	case VK_SUCCESS: return vtek::SwapchainStatus::ok;
	case VK_TIMEOUT: return vtek::SwapchainStatus::timeout;
	default:
		return vtek::SwapchainStatus::error;
	}
}

vtek::SwapchainStatus vtek::swapchain_acquire_next_image(
	vtek::Swapchain* swapchain, vtek::Device* device,
	uint32_t* frameIndex, uint64_t timeout)
{
	VkDevice dev = vtek::device_get_handle(device);
	uint32_t currentFrame = swapchain->currentFrameIndex;
	VkSemaphore semaphore = swapchain->imageAvailableSemaphores[currentFrame];

	VkResult result = vkAcquireNextImageKHR(
		dev, swapchain->vulkanHandle, timeout, semaphore, VK_NULL_HANDLE, frameIndex);

	// NOTE: For optimal rendering efficiency, Nvidia developers give this advice:
	// "Handle both out-of-date and suboptimal swapchains to re-create stale
	//  swapchains when windows resize".
	// https://developer.nvidia.com/blog/advanced-api-performance-vulkan-clearing-and-presenting/
	switch (result)
	{
	case VK_SUCCESS:
	case VK_SUBOPTIMAL_KHR:
		return vtek::SwapchainStatus::ok;

	case VK_ERROR_OUT_OF_DATE_KHR:
		return vtek::SwapchainStatus::outofdate;

	default:
		return vtek::SwapchainStatus::error;
	}
}

vtek::SwapchainStatus vtek::swapchain_wait_image_ready(
	vtek::Swapchain* swapchain, vtek::Device* device,
	uint32_t frameIndex, uint64_t timeout)
{
	VkDevice dev = vtek::device_get_handle(device);
	VkFence fence = swapchain->imagesInFlight[frameIndex];

	// Check if a previous frame is using this image, ie. if there is a fence
	// to wait on.
	if (fence == VK_NULL_HANDLE)
	{
		set_image_in_use(swapchain, dev, frameIndex);
		return vtek::SwapchainStatus::ok;
	}

	// Is the fence already signaled, then return immediately without waiting:
	VkResult test = vkWaitForFences(dev, 1, &fence, VK_TRUE, 0UL);
	if (test == VK_SUCCESS)
	{
		set_image_in_use(swapchain, dev, frameIndex);
		return vtek::SwapchainStatus::ok;
	}

	VkResult result = vkWaitForFences(dev, 1, &fence, VK_TRUE, timeout);
	switch (result)
	{
	case VK_SUCCESS:
		set_image_in_use(swapchain, dev, frameIndex);
		return vtek::SwapchainStatus::ok;
	case VK_TIMEOUT:
		return vtek::SwapchainStatus::timeout;
	default:
		return vtek::SwapchainStatus::error;
	}
}

void vtek::swapchain_fill_queue_submit_info(
	vtek::Swapchain* swapchain, vtek::SubmitInfo* submitInfo)
{
	uint32_t currentIndex = swapchain->currentFrameIndex;

	submitInfo->AddSignalSemaphore(
		swapchain->renderFinishedSemaphores[currentIndex]);
	submitInfo->AddWaitSemaphore(
		swapchain->imageAvailableSemaphores[currentIndex],
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	submitInfo->SetPostSignalFence(swapchain->inFlightFences[currentIndex]);
}

vtek::SwapchainStatus vtek::swapchain_present_frame(
	vtek::Swapchain* swapchain, uint32_t frameIndex)
{
	uint32_t currentFrame = swapchain->currentFrameIndex;

	const VkPresentInfoKHR info{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &swapchain->renderFinishedSemaphores[currentFrame],
		.swapchainCount = 1,
		.pSwapchains = &swapchain->vulkanHandle,
		.pImageIndices = &frameIndex,
		.pResults = nullptr
	};

	// Submit frame to present queue
	VkResult result = vkQueuePresentKHR(swapchain->presentQueue, &info);
	switch (result)
	{
	case VK_SUCCESS:
		{
			// Advance to next frame
			uint32_t& curFrame = swapchain->currentFrameIndex;
			curFrame = (curFrame + 1) % swapchain->numFramesInFlight;
		}
		return vtek::SwapchainStatus::ok;

	case VK_ERROR_OUT_OF_DATE_KHR:
	case VK_SUBOPTIMAL_KHR:
		return vtek::SwapchainStatus::outofdate;

	default:
		return vtek::SwapchainStatus::error;
	}
}

void vtek::swapchain_dynamic_rendering_begin(
	vtek::Swapchain* swapchain, uint32_t imageIndex,
	vtek::CommandBuffer* commandBuffer, glm::vec3 clearColor)
{
	VkCommandBuffer cmdBuf = vtek::command_buffer_get_handle(commandBuffer);
	swapchain->fDynRenderBegin(swapchain, imageIndex, cmdBuf, clearColor);
}

void vtek::swapchain_dynamic_rendering_end(
	vtek::Swapchain* swapchain, uint32_t imageIndex, vtek::CommandBuffer* commandBuffer)
{
	VkCommandBuffer cmdBuf = vtek::command_buffer_get_handle(commandBuffer);

	// End dynamic rendering
	vkCmdEndRendering(cmdBuf);

	// Transition from color attachment to present src
	VkImageMemoryBarrier barrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext = nullptr,
		.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.dstAccessMask = 0,
		.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.srcQueueFamilyIndex = swapchain->graphicsQueueIndex,
		.dstQueueFamilyIndex = swapchain->presentQueueIndex,
		.image = swapchain->images[imageIndex],
		.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	vkCmdPipelineBarrier(
		cmdBuf, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}
