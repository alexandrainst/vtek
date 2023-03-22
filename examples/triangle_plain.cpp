#include <vtek/vtek.h>
#include <iostream>

// global data
vtek::ApplicationWindow* window = nullptr;


void keyCallback(vtek::KeyboardKey key, vtek::InputAction action)
{
	std::cout << "key callback\n";
	using vtek::KeyboardKey;
	using vtek::InputAction;

	if (action == InputAction::press)
	{

	}
	else if (action == InputAction::release)
	{
		switch (key)
		{
		case KeyboardKey::escape:
			vtek::window_set_should_close(window, true);
			break;
		default:
			break;
		}
	}
}


int main()
{
	// Initialize vtek
	vtek::InitInfo initInfo{};
	initInfo.applicationTitle = "triangle_plain";
	initInfo.useGLFW = true;
	if (!vtek::initialize(&initInfo))
	{
		std::cerr << "Failed to initialize vtek!" << std::endl;
		return -1;
	}

	// Create window
	vtek::WindowCreateInfo windowInfo{};
	windowInfo.title = "triangle_plain";
	windowInfo.width = 500;
	windowInfo.height = 500;
	window = vtek::window_create(&windowInfo);
	if (window == nullptr)
	{
		log_error("Failed to create window!");
		return -1;
	}
	vtek::window_set_key_handler(window, keyCallback);

	// Vulkan instance
	vtek::InstanceCreateInfo instanceInfo{};
	instanceInfo.applicationName = "triangle_plain";
	instanceInfo.applicationVersion = vtek::VulkanVersion(1, 0, 0);
	instanceInfo.enableValidationLayers = true;
	auto instance = vtek::instance_create(&instanceInfo);
	if (instance == nullptr)
	{
		log_error("Failed to create Vulkan instance!");
		return -1;
	}

	// Surface
	VkSurfaceKHR surface = vtek::window_create_surface(window, instance);
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
	vtek::PhysicalDevice* physicalDevice = vtek::physical_device_pick(&physicalDeviceInfo, instance, surface);
	if (physicalDevice == nullptr)
	{
		log_error("Failed to pick physical device!");
		return -1;
	}

	// Device
	vtek::LogicalDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.enableSwapchainExtension = true; // TODO: We probably don't want to do this here!
	vtek::Device* device = vtek::device_create(&deviceCreateInfo, instance, physicalDevice);
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

	// Graphics command pool
	vtek::CommandPoolCreateInfo commandPoolCreateInfo{};
	vtek::CommandPool* graphicsCommandPool = vtek::command_pool_create(
		&commandPoolCreateInfo, device, graphicsQueue);

	// Swapchain
	vtek::SwapchainCreateInfo swapchainCreateInfo{};
	swapchainCreateInfo.vsync = true;
	swapchainCreateInfo.prioritizeLowLatency = false;
	vtek::window_get_framebuffer_size(
		window, &swapchainCreateInfo.framebufferWidth, &swapchainCreateInfo.framebufferHeight);
	vtek::Swapchain* swapchain = vtek::swapchain_create(&swapchainCreateInfo, surface, physicalDevice, device);
	if (swapchain == nullptr)
	{
		log_error("Failed to create swapchain!");
		return -1;
	}



	while (vtek::window_get_should_close(window))
	{
		vtek::window_poll_events();
	}



	// Cleanup
	vtek::swapchain_destroy(swapchain, device);
	vtek::command_pool_destroy(graphicsCommandPool, device);
	vtek::device_destroy(device);
	vtek::physical_device_release(physicalDevice);
	vtek::window_surface_destroy(surface, instance);
	vtek::instance_destroy(instance);
	vtek::window_destroy(window);

	log_debug("All went well!");
	vtek::terminate();

	return 0;
}
