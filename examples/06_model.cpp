#include <vtek/vtek.hpp>
#include <iostream>

// global data
vtek::ApplicationWindow* gWindow = nullptr;
uint32_t gFramebufferWidth = 0U;
uint32_t gFramebufferHeight = 0U;
vtek::KeyboardMap gKeyboardMap;
vtek::Camera* gCamera = nullptr;
vtek::Uniform_m4 uniform;


// helper functions
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
		case KeyboardKey::p:
			{
				glm::vec3 pos = vtek::camera_get_position(gCamera);
				glm::vec3 front = vtek::camera_get_front(gCamera);
				glm::vec3 up = vtek::camera_get_up(gCamera);

				log_debug("[Camera] pos=({},{},{}), front=({},{},{}), up=({},{},{})",
				          pos.x, pos.y, pos.z, front.x, front.y, front.z, up.x, up.y, up.z);
			}
			break;

		// NOTE: Un-comment for checking correct angular displacement.
		// case KeyboardKey::q:
		// 	vtek::camera_roll_left_radians(gCamera, glm::pi<float>() * 0.25f);
		// 	break;
		// case KeyboardKey::e:
		// 	vtek::camera_roll_right_radians(gCamera, glm::pi<float>() * 0.25f);
		// 	break;

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

void mouse_move_callback(double x, double y)
{
	vtek::camera_on_mouse_move(gCamera, x, y);
}





int main()
{
	// Initialize vtek
	vtek::InitInfo initInfo{};
	initInfo.disableLogging = false;
	initInfo.applicationTitle = "06_model";
	initInfo.useGLFW = true;
	if (!vtek::initialize(&initInfo))
	{
		std::cerr << "Failed to initialize vtek!" << std::endl;
		return -1;
	}

	// Keyboard map
	gKeyboardMap.reset();

	// Create window
	vtek::WindowCreateInfo windowInfo{};
	windowInfo.title = "vtek example 06: Model loading";
	windowInfo.resizeable = false;
	windowInfo.cursorDisabled = true;
	gWindow = vtek::window_create(&windowInfo);
	if (gWindow == nullptr)
	{
		log_error("Failed to create window!");
		return -1;
	}
	vtek::window_set_key_handler(gWindow, key_callback);
	vtek::window_set_mouse_move_handler(gWindow, mouse_move_callback);

	// Vulkan instance
	vtek::InstanceCreateInfo instanceInfo{};
	instanceInfo.applicationName = "06_model";
	instanceInfo.applicationVersion = vtek::VulkanVersion(1, 0, 0);
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
	vtek::DeviceCreateInfo deviceCreateInfo{};
	vtek::Device* device = vtek::device_create(
		&deviceCreateInfo, instance, physicalDevice);
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
	vtek::window_get_framebuffer_size(
		gWindow, &gFramebufferWidth, &gFramebufferHeight);

	// Swapchain
	vtek::SwapchainInfo swapchainInfo{};
	swapchainInfo.vsync = true;
	swapchainInfo.prioritizeLowLatency = false;
	swapchainInfo.framebufferWidth = gFramebufferWidth;
	swapchainInfo.framebufferHeight = gFramebufferHeight;
	swapchainInfo.createDepthBuffers = true;
	vtek::Swapchain* swapchain = vtek::swapchain_create(
		&swapchainInfo, surface, physicalDevice, device);
	if (swapchain == nullptr)
	{
		log_error("Failed to create swapchain!");
		return -1;
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

	// Command buffers
	const uint32_t commandBufferCount = vtek::swapchain_get_length(swapchain);
	std::vector<vtek::CommandBuffer*> commandBuffers =
		vtek::command_pool_alloc_buffers(
			graphicsCommandPool, vtek::CommandBufferUsage::primary,
			commandBufferCount, device);
	if (commandBuffers.empty())
	{
		log_error("Failed to create command buffers!");
		return -1;
	}
	if (commandBufferCount != commandBuffers.size())
	{
		log_error("Number of command buffers created not same as number asked!");
		return -1;
	}

	// Camera
	gCamera = vtek::camera_create();
	glm::vec3 camPos {8.0f, 0.0f, 0.0f};
	glm::vec3 camFront {-1.0f, 0.0f, 0.0f};
	glm::vec3 camUp {0.0f, 0.0f, 1.0f};
	vtek::camera_set_lookat(gCamera, camPos, camFront, camUp);
	vtek::camera_set_window_size(gCamera, gFramebufferWidth, gFramebufferHeight);
	vtek::camera_set_perspective_frustrum(gCamera, 45.0f, 0.1f, 100.0f);
	vtek::camera_update(gCamera);

	// Model
	const char* modeldirstr = "../models/";
	vtek::Directory* modeldir = vtek::directory_open(modeldirstr);
	if (modeldir == nullptr)
	{
		log_error("Failed to open model directory!");
		return -1;
	}
	vtek::Model* model = vtek::model_load_obj(modeldir, "armored_car.obj");
	if (model == nullptr)
	{
		log_error("Failed to load obj model!");
		return -1;
	}





	// Cleanup
	vtek::device_wait_idle(device);

	vtek::camera_destroy(gCamera);
	vtek::command_pool_free_buffers(graphicsCommandPool, commandBuffers, device);
	vtek::command_pool_destroy(graphicsCommandPool, device);
	vtek::swapchain_destroy(swapchain, device);
	vtek::device_destroy(device);
	vtek::physical_device_release(physicalDevice);
	vtek::window_surface_destroy(surface, instance);
	vtek::instance_destroy(instance);
	vtek::window_destroy(gWindow);

	log_info("Program Success!");
	vtek::terminate();

	return 0;
}
