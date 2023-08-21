#include "vtek_vulkan.pch"
#include "vtek_image.hpp"

#include "imgutils/vtek_image_load.hpp"
#include "imgutils/vtek_image_formats.hpp"
#include "impl/vtek_queue_struct.hpp"
#include "impl/vtek_vma_helpers.hpp"
#include "vtek_buffer.hpp"
#include "vtek_command_buffer.hpp"
#include "vtek_command_scheduler.hpp"
#include "vtek_device.hpp"
#include "vtek_fileio.hpp"
#include "vtek_logging.hpp"

using IPSFmt = vtek::ImagePixelStorageFormat;
using IAFlag = vtek::ImageAspectFlag;
using IUFlag = vtek::ImageUsageFlag;
using IFType = vtek::ImageFileType;


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

	return image;
}

void vtek::image2d_destroy(vtek::Image2D* image)
{
	if (image == nullptr) { return; }

	vtek::allocator_image2d_destroy(image);
	image->vulkanHandle = nullptr;
	delete image;
}

vtek::Image2D* vtek::image2d_create(
	const vtek::Image2DInfo* info, vtek::Allocator* allocator)
{
	vtek_log_error("vtek::image2d_create(_, Allocator*): not implemented!");

	return nullptr;
}




vtek::Image2D* vtek::image2d_load(
	const vtek::Image2DLoadInfo* info, const vtek::Directory* directory,
	std::string_view filename, vtek::Device* device)
{
	vtek::ImageLoadInfo loadInfo{};
	// TODO: Determine correct number of channels!
	loadInfo.desiredChannels = 4;
	if (info->loadSRGB)
	{

	}

	vtek::ImageLoadData imageData{};

	// 1) Load image from file
	if (!vtek::image_load(directory, filename, &loadInfo, &imageData))
	{
		vtek_log_error(
			"Failed to load image file -- cannot create Vulkan image!");
		return nullptr;
	}

	vtek_log_debug(
		"ImageLoadInfo: w={}, h={}, channels={}, data={}, data16={}, dataf={}",
		imageData.width, imageData.height,
		imageData.channels, imageData.data != nullptr,
		imageData.data16 != nullptr, imageData.fdata != nullptr);

	if (imageData.data == nullptr && info->loadSRGB)
	{
		vtek_log_error(
			"sRGB is not supported for {} -- Cannot load image \"{}\".",
			"image which does not contain 8-bit data!",
			filename);
		vtek::image_load_data_destroy(&imageData);
		return nullptr;
	}
	void* imgData = nullptr;

	vtek::ImageFormatInfo formatInfo{};
	formatInfo.channels = static_cast<vtek::ImageChannels>(imageData.channels);
	formatInfo.imageStorageSRGB = info->loadSRGB;
	formatInfo.swizzleBGR = false; // NOTE: Would require manual pixel swizzling!
	formatInfo.compression = vtek::ImageCompressionFormat::none;

	// Retrieve format information
	if (imageData.data != nullptr) // data is 8-bit
	{
		formatInfo.storageFormat = IPSFmt::unorm;
		formatInfo.channelSize = vtek::ImageChannelSize::channel_8;
		imgData = imageData.data;
	}
	else if (imageData.data16 != nullptr)
	{
		formatInfo.storageFormat = IPSFmt::unorm;
		formatInfo.channelSize = vtek::ImageChannelSize::channel_16;
		imgData = imageData.data16;
	}
	else if (imageData.fdata != nullptr)
	{
		formatInfo.storageFormat = IPSFmt::float32;
		formatInfo.channelSize = vtek::ImageChannelSize::channel_32;
		imgData = imageData.fdata;
	}
	else
	{
		vtek_log_error("vtek_image.cpp: Image data was NULL!");
		vtek::image_load_data_destroy(&imageData);
		return nullptr;
	}

	VkPhysicalDevice physDev = vtek::device_get_physical_handle(device);
	EnumBitmask<vtek::FormatFeatureFlag> featureFlags
		= vtek::FormatFeatureFlag::sampledImage
		| vtek::FormatFeatureFlag::sampledImageFilterLinear;
	// TODO: How if we later create mip-maps by linear blitting?
	// | VK_FORMAT_FEATURE_BLIT_DST_BIT; // ?

	VkFormat format = vtek::get_format_color(&formatInfo, physDev, featureFlags);
	if (format == VK_FORMAT_UNDEFINED)
	{
		vtek_log_error("Failed find a suitable image format for loaded image!");
		vtek::image_load_data_destroy(&imageData);
		return nullptr;
	}

	// 2) Create destination image
	vtek::Image2DInfo createInfo{};
	// NOTE: If implementation supports VK_KHR_dedicated_allocation
	// (or Vulkan >= 1.1), we can ask the implementation if it
	// requires/recommends a dedicated allocation. This is typically
	// suggested for render targets or very large resources.
	// NOTE: For simplicity we just say no always!
	createInfo.requireDedicatedAllocation = false;
	createInfo.extent = { imageData.width, imageData.height };
	createInfo.format = format;
	createInfo.formatInfo = formatInfo;
	createInfo.usageFlags = IUFlag::transfer_dst | IUFlag::sampled;
	createInfo.initialLayout = vtek::ImageInitialLayout::preinitialized;
	createInfo.useMipmaps = info->createMipmaps;
	createInfo.multisampling = vtek::MultisampleType::none;

	// We must check if transfer/graphics queues are from same queue family!
	auto scheduler = vtek::device_get_command_scheduler(device);
	vtek::Queue* transferQueue =
		vtek::command_scheduler_get_transfer_queue(scheduler);
	vtek::Queue* graphicsQueue = vtek::device_get_graphics_queue(device);
	// TODO: Optimization possibility: ONLY owned by transfer queue!!!
	// TODO: Then later, during final layout transition, change ownership to graphics!!!
	constexpr bool kOptimizedOwnership = false;
	if constexpr (kOptimizedOwnership)
	{
		createInfo.sharingMode = vtek::ImageSharingMode::exclusive;
		// NOTE: The sharing queues get ignored by Vulkan when mode is exclusive!
		// TODO: So probably don't add them here => kind pointless!?
		createInfo.sharingQueues = { transferQueue };
	}
	else
	{
		if (transferQueue->familyIndex == graphicsQueue->familyIndex)
		{
			createInfo.sharingMode = vtek::ImageSharingMode::exclusive;
		}
		else
		{
			createInfo.sharingMode = vtek::ImageSharingMode::concurrent;
			createInfo.sharingQueues = { transferQueue, graphicsQueue };
		}
	}

	createInfo.createImageView = true;
	createInfo.imageViewInfo.baseMipLevel = 0;
	createInfo.imageViewInfo.baseArrayLayer = 0;
	createInfo.imageViewInfo.aspectFlags = IAFlag::color;

	vtek::Image2D* image = vtek::image2d_create(&createInfo, device);
	if (image == nullptr)
	{
		vtek_log_error(
			"Failed to create 2D-image -- cannot proceed with image loading!");
		vtek::image_load_data_destroy(&imageData);
		return nullptr;
	}

	uint64_t totalSize = vtek::image_load_data_get_size(&imageData);

	// 3) Create staging buffer
	vtek::BufferInfo stagingInfo{};
	stagingInfo.disallowInternalStagingBuffer = true;
	stagingInfo.requireHostVisibleStorage = true;
	stagingInfo.size = totalSize;
	stagingInfo.usageFlags = vtek::BufferUsageFlag::transfer_src;
	stagingInfo.writePolicy = vtek::BufferWritePolicy::write_once;
	vtek::Buffer* stagingBuffer = vtek::buffer_create(&stagingInfo, device);
	if (stagingBuffer == nullptr)
	{
		vtek_log_error("Failed to create staging buffer for image loading!");
		vtek::image_load_data_destroy(&imageData);
		vtek::image2d_destroy(image);
		return nullptr;
	}

	// 4) Write image data to staging buffer
	vtek::BufferRegion stagingRegion{};
	if (!vtek::buffer_write_data(stagingBuffer, imgData, &stagingRegion, device))
	{
		vtek_log_error("Failed to write image data to staging buffer!");
		vtek::image_load_data_destroy(&imageData);
		vtek::image2d_destroy(image);
		vtek::buffer_destroy(stagingBuffer);
		return nullptr;
	}

	// 5) Copy image data to GPU memory

	// TODO: Extract function when logic is in place
	auto commandBuffer =
		vtek::command_scheduler_begin_transfer(scheduler, device);
	if (commandBuffer == nullptr)
	{
		vtek_log_error(
			"Failed to begin single-use transfer command buffer -- {}",
			"cannot write pixel data to image!");
		vtek::image_load_data_destroy(&imageData);
		vtek::image2d_destroy(image);
		vtek::buffer_destroy(stagingBuffer);
		return nullptr;
	}

	VkBufferImageCopy copyRegion{};
	copyRegion.bufferOffset = 0;
	copyRegion.bufferRowLength = 0;
	copyRegion.bufferImageHeight = 0;
	copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.imageSubresource.mipLevel = 0; // TODO: mip-maps?
	copyRegion.imageSubresource.baseArrayLayer = 0;
	copyRegion.imageSubresource.layerCount = 1; // TODO: mip-maps?
	copyRegion.imageOffset = {0, 0, 0};
	copyRegion.imageExtent = { imageData.width, imageData.height, 1};

	VkCommandBuffer cmdBuf = vtek::command_buffer_get_handle(commandBuffer);
	VkBuffer stgBuf = vtek::buffer_get_handle(stagingBuffer);

	// NOTE: dstImageLayout parameter was VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	vkCmdCopyBufferToImage(
		cmdBuf, stgBuf, image->vulkanHandle, VK_IMAGE_LAYOUT_PREINITIALIZED,
		1, &copyRegion);

	// TODO: Insert semaphore here?


	// 6) Final layout transition
	// TODO: It would be nice to have extract function from this!
	// TODO: This would require adding a few members to the image struct!

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = nullptr;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	barrier.image = image->vulkanHandle;
	barrier.subresourceRange.baseMipLevel = 0; // TODO: Customize.
	barrier.subresourceRange.levelCount = 1; // TODO: mip-maps?
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

	// It is possible to use `VK_IMAGE_LAYOUT_UNDEFINED` as the old layout
	// if we don't care about the existing contents of the image.
	// NOTE: oldLayout was VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	barrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	// If we are using the barrier to transfer queue family ownership, then
	// these two fields should be indiced of the queue families. They must
	// be set to `VK_QUEUE_FAMILY_IGNORED` if we don't want to do this (not
	// the default value!).
	// TODO: Is this right?
	vtek_log_debug(
		"Note the transfer of queue ownership on image memory barrier for final layout transition!");
	barrier.srcQueueFamilyIndex = transferQueue->familyIndex;
	barrier.dstQueueFamilyIndex = graphicsQueue->familyIndex;

	vkCmdPipelineBarrier(
		cmdBuf, VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0, 0, nullptr, 0, nullptr, 1, &barrier);

	if (!vtek::command_scheduler_submit_transfer(scheduler, commandBuffer, device))
	{
		vtek_log_error(
			"Failed to submit transfer of image data to command scheduler!");
		vtek::image_load_data_destroy(&imageData);
		vtek::image2d_destroy(image);
		vtek::buffer_destroy(stagingBuffer);
		return nullptr;
	}

	// Free data
	vtek::image_load_data_destroy(&imageData);
	vtek::buffer_destroy(stagingBuffer);

	return image;
}


VkImage vtek::image2d_get_handle(const vtek::Image2D* image)
{
	return image->vulkanHandle;
}

VkImageView vtek::image2d_get_view_handle(const vtek::Image2D* image)
{
	return image->viewHandle;
}
