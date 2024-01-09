#include <vtek/vtek.hpp>
#include <iostream>

/* global data */
vtek::ApplicationWindow* gWindow = nullptr;
uint32_t gFramebufferWidth = 0U;
uint32_t gFramebufferHeight = 0U;
vtek::KeyboardMap gKeyboardMap;
glm::vec2 gMoveOffset {0.0f, 0.0f};

/* input handling */
void key_callback(vtek::KeyboardKey key, vtek::InputAction action)
{
	using vtek::KeyboardKey;

	if (action == vtek::InputAction::press)
	{
		switch (key)
		{
		case KeyboardKey::escape:
			vtek::window_set_should_close(gWindow, true);
			break;

		default:
			gKeyboardMap.press_key(key);
			break;
		}
	}
	else if (action == vtek::InputAction::release)
	{
		gKeyboardMap.release_key(key);
	}
}

void update_movement()
{
	using vtek::KeyboardKey;
	constexpr float moveSpeed = 0.05f;

	// left / right
	if (gKeyboardMap.get_key(KeyboardKey::left)) {
		gMoveOffset.x -= moveSpeed;
	}
	else if (gKeyboardMap.get_key(KeyboardKey::right)) {
		gMoveOffset.x += moveSpeed;
	}

	// up / down
	if (gKeyboardMap.get_key(KeyboardKey::up)) {
		gMoveOffset.y -= moveSpeed;
	}
	else if (gKeyboardMap.get_key(KeyboardKey::down)) {
		gMoveOffset.y += moveSpeed;
	}
}

bool update_uniform(vtek::Buffer* buffer, vtek::Device* device)
{
	vtek::Uniform_v2 uniform {.v2 = gMoveOffset};
	vtek::BufferRegion region {.offset = 0, .size = uniform.size()};

	if (!vtek::buffer_write_data(buffer, &uniform, &region, device))
	{
		log_error("Failed to write data to the uniform buffer!");
		return false;
	}

	return true;
}

bool recordCommandBuffer(
	vtek::CommandBuffer* commandBuffer, vtek::GraphicsPipeline* pipeline,
	vtek::Swapchain* swapchain, vtek::Framebuffer* framebuffer, uint32_t imageIndex,
	vtek::Model* model, vtek::DescriptorSet* descriptorSet)
{
	// imaginative scenario:
	VkPipeline pipl = vtek::graphics_pipeline_get_handle(pipeline);
	VkPipelineLayout pipLayout = vtek::graphics_pipeline_get_layout(pipeline);
	VkCommandBuffer cmdBuf = vtek::command_buffer_get_handle(commandBuffer);

	// 1) begin recording on the command buffer
	if (!vtek::command_buffer_begin(commandBuffer))
	{
		log_error("Failed to begin command buffer {} recording!", imageIndex);
		return false;
	}

	// 2) begin dynamic rendering on the framebuffer
	if (!vtek::framebuffer_dynrender_begin(framebuffer))
	{
		log_error("Failed to begin dynamic rendering on framebuffer!");
		return false;
	}

	// 3) bind pipeline and set dynamic states

	// 4) bind and render each model, including descriptor sets and push constants

	// 5) end dynamic rendering on the framebuffer

	// 6) begin dynamic rendering on the swapchain

	// 7) bind framebuffer targets for reading (descriptor sets?)

	// 8) draw fullscreen quad

	// 9) end dynamic rendering on the swapchain

	// 10) end recording on the command buffer
}



int main()
{
	// Initialize vtek
	vtek::InitInfo initInfo{};
	initInfo.disableLogging = false;
	initInfo.applicationTitle = "07_textured_model";
	initInfo.useGLFW = true;
	if (!vtek::initialize(&initInfo))
	{
		std::cerr << "Failed to initialize vtek!" << std::endl;
		return -1;
	}

	// Keyboard map
	gKeyboardMap.reset();

	// Create window
	vtek::WindowInfo windowInfo{};
	windowInfo.title = "vtek example 07: Drawing a textured model";
	windowInfo.resizeable = false;
	windowInfo.cursorDisabled = true;
	windowInfo.width = 1024;
	windowInfo.height = 1024;
	windowInfo.fullscreen = false;
	gWindow = vtek::window_create(&windowInfo);
	if (gWindow == nullptr)
	{
		log_error("Failed to create window!");
		return -1;
	}
	vtek::window_set_key_handler(gWindow, key_callback);

	// Vulkan instance
	vtek::InstanceInfo instanceInfo{};
	instanceInfo.applicationName = "07_textured_model";
	instanceInfo.applicationVersion = vtek::VulkanVersion(1, 0, 0); // TODO: vtek::AppVersion
	instanceInfo.enableValidationLayers = true;
	auto instance = vtek::instance_create(&instanceInfo);
	if (instance == nullptr)
	{
		log_error("Failed to create Vulkan instance!");
		return -1;
	}

	// Surface
	VkSurfaceKHR surface = vtek::window_create_surface(gWindow, instance);
	if (surface == VK_NULL_HANDLE)
	{
		log_error("Failed to create Vulkan window surface!");
		return -1;
	}

	// Physical device
	vtek::PhysicalDeviceInfo physicalDeviceInfo{};
	physicalDeviceInfo.requireGraphicsQueue = true;
	physicalDeviceInfo.requirePresentQueue = true;
	physicalDeviceInfo.requireSwapchainSupport = true;
	physicalDeviceInfo.requireDynamicRendering = true;
	physicalDeviceInfo.requiredFeatures.depthBounds = true;
	physicalDeviceInfo.requiredFeatures.fillModeNonSolid = true;
	// Allows us to change the uniform without re-recording the command buffers:
	physicalDeviceInfo.updateAfterBindFeatures
		= vtek::UpdateAfterBindFeature::uniform_buffer;
	vtek::PhysicalDevice* physicalDevice = vtek::physical_device_pick(
		&physicalDeviceInfo, instance, surface);
	if (physicalDevice == nullptr)
	{
		log_error("Failed to pick physical device!");
		return -1;
	}

	// Device
	vtek::DeviceInfo deviceInfo{};
	vtek::Device* device = vtek::device_create(
		&deviceInfo, instance, physicalDevice);
	if (device == nullptr)
	{
		log_error("Failed to create device!");
		return -1;
	}

	// Graphics queue
	vtek::Queue* graphicsQueue = vtek::device_get_graphics_queue(device);
	if (graphicsQueue == nullptr)
	{
		log_error("Failed to get graphics queue!");
		return -1;
	}

	// Window size, for swapchain, camera, and other things.
	glm::uvec2 windowSize;
	vtek::window_get_framebuffer_size(gWindow, &windowSize.x, &windowSize.y);
	gFramebufferWidth = windowSize.x;
	gFramebufferHeight = windowSize.y;

	// Swapchain
	vtek::SwapchainInfo swapchainInfo{};
	swapchainInfo.vsync = true;
	swapchainInfo.prioritizeLowLatency = false;
	swapchainInfo.framebufferWidth = gFramebufferWidth;
	swapchainInfo.framebufferHeight = gFramebufferHeight;
	swapchainInfo.depthBuffer = vtek::SwapchainDepthBuffer::none;
	vtek::Swapchain* swapchain = vtek::swapchain_create(
		&swapchainInfo, surface, physicalDevice, device);
	if (swapchain == nullptr)
	{
		log_error("Failed to create swapchain!");
		return -1;
	}

	// Framebuffer attachment formats
	std::vector<vtek::Format> prioritizedColorFormats = {
		vtek::Format::r8g8b8_unorm,
		vtek::Format::r8g8b8a8_unorm
	};
	vtek::SupportedFormat colorFormat;
	vtek::FormatQuery framebufferFormatQuery{};
	framebufferFormatQuery.linearTiling = false;
	framebufferFormatQuery.features
		= vtek::FormatFeature::sampled_image
		| vtek::FormatFeature::color_attachment;
	for (auto fmt : prioritizedColorFormats)
	{
		framebufferFormatQuery.format = fmt;
		if (vtek::has_format_support(
			    &framebufferFormatQuery, device, &colorFormat))
		{
			break;
		}
	}
	if (!colorFormat.is_valid())
	{
		log_error("Failed to find a supported framebuffer color format!");
		return -1;
	}

	// Framebuffer
	vtek::FramebufferAttachment colorAttachment{};
	colorAttachment.type = vtek::AttachmentType::color;
	colorAttachment.supportedFormat = colorFormat;
	vtek::FramebufferInfo framebufferInfo{};
	framebufferInfo.attachments.push_back(colorAttachment);
	vtek::Framebuffer* framebuffer = vtek::framebuffer_create(&framebufferInfo, device);
	if (framebuffer == nullptr)
	{
		log_error("Failed to create framebuffer!");
		return 0;
	}

	// Graphics command pool
	vtek::CommandPoolInfo commandPoolInfo{};
	commandPoolInfo.allowIndividualBufferReset = true;
	vtek::CommandPool* graphicsCommandPool = vtek::command_pool_create(
		&commandPoolInfo, device, graphicsQueue);
	if (graphicsCommandPool == nullptr)
	{
		log_error("Failed to create graphics command pool!");
		return -1;
	}







}
