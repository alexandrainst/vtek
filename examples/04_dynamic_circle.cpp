#include <vtek/vtek.hpp>
#include <iostream>
#include <vector>

// global data
vtek::ApplicationWindow* window = nullptr;
constexpr uint32_t kVertMax = 16;
vtek::IntClamp<uint32_t, 3, kVertMax> numCircleVertices = 3;
std::vector<glm::vec2> vertices;
glm::vec2 circleCenter{0.0f, 0.0f};
float circleRadius = 0.5f;


void keyCallback(vtek::KeyboardKey key, vtek::InputAction action)
{
	if ((action == vtek::InputAction::release) &&
	    (key == vtek::KeyboardKey::escape))
	{
		vtek::window_set_should_close(window, true);
	}
}

bool fill_buffer(vtek::Buffer* buffer, vtek::Device* device)
{
	vertices.clear();
	vertices.resize(numCircleVertices.get());

	const float angle = glm::two_pi<float>() / (float)numCircleVertices.get();
	for (uint32_t i = 0; i < numCircleVertices.get(); i++)
	{
		float localAngle = (float)i * angle;
		float x = (glm::cos(localAngle) * circleRadius) + circleCenter.x;
		float y = (glm::sin(localAngle) * circleRadius) + circleCenter.y;
		vertices.push_back({x, y});
	}

	vtek::BufferRegion region{
		.offset = 0,
		.size = vertices.size()*sizeof(glm::vec2)
	};
	return vtek::buffer_write_data(buffer, vertices.data(), &region, device);
}


int main()
{
	// Initialize vtek
	vtek::InitInfo initInfo{};
	initInfo.disableLogging = false;
	initInfo.applicationTitle = "03_vertex_buffer";
	initInfo.useGLFW = true;
	if (!vtek::initialize(&initInfo))
	{
		std::cerr << "Failed to initialize vtek!" << std::endl;
		return -1;
	}

	// Create window
	vtek::WindowCreateInfo windowInfo{};
	windowInfo.title = "vtek example 03: Vertex buffer";
	window = vtek::window_create(&windowInfo);
	if (window == nullptr)
	{
		log_error("Failed to create window!");
		return -1;
	}
	vtek::window_set_key_handler(window, keyCallback);

	// Vulkan instance
	vtek::InstanceCreateInfo instanceInfo{};
	instanceInfo.applicationName = "vertex_buffer";
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
	physicalDeviceInfo.requireDynamicRendering = true;
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

	// Graphics command pool
	vtek::CommandPoolCreateInfo commandPoolCreateInfo{};
	vtek::CommandPool* graphicsCommandPool = vtek::command_pool_create(
		&commandPoolCreateInfo, device, graphicsQueue);
	if (graphicsCommandPool == nullptr)
	{
		log_error("Failed to create graphics command pool!");
		return -1;
	}

	// Swapchain
	vtek::SwapchainCreateInfo swapchainCreateInfo{};
	swapchainCreateInfo.vsync = true;
	swapchainCreateInfo.prioritizeLowLatency = false;
	vtek::window_get_framebuffer_size(
		window, &swapchainCreateInfo.framebufferWidth,
		&swapchainCreateInfo.framebufferHeight);
	vtek::Swapchain* swapchain = vtek::swapchain_create(
		&swapchainCreateInfo, surface, physicalDevice, device);
	if (swapchain == nullptr)
	{
		log_error("Failed to create swapchain!");
		return -1;
	}

	// Command buffers
	log_trace("Command buffers");
	const uint32_t commandBufferCount = vtek::swapchain_get_length(swapchain);
	vtek::CommandBufferCreateInfo commandBufferInfo{};
	commandBufferInfo.isSecondary = false;
	std::vector<vtek::CommandBuffer*> commandBuffers = vtek::command_buffer_create(
		&commandBufferInfo, commandBufferCount, graphicsCommandPool, device);
	if (commandBuffers.empty())
	{
		log_error("Failed to create command buffer!");
		return -1;
	}
	if (commandBufferCount != commandBuffers.size())
	{
		log_error("Number of command buffers created not same as number asked!");
		return -1;
	}

	// Shader
	const char* shaderdirstr = "../shaders/dynamic_circle/";
	vtek::Directory* shaderdir = vtek::directory_open(shaderdirstr);
	if (shaderdir == nullptr)
	{
		log_error("Failed to open shader directory!");
		return -1;
	}
	vtek::GraphicsShaderInfo shaderInfo{};
	shaderInfo.vertex = true;
	shaderInfo.fragment = true;
	vtek::GraphicsShader* shader =
		vtek::graphics_shader_load_spirv(&shaderInfo, shaderdir, device);
	if (shader == nullptr)
	{
		log_error("Failed to load graphics shader!");
		return -1;
	}

	// Vertex buffer
	log_trace("Vertex buffer");
	vtek::BufferInfo bufferInfo{};
	bufferInfo.size = sizeof(glm::vec2) * kVertMax;
	bufferInfo.requireHostVisibleStorage = true; // NOTE: Easy for now.
	//bufferInfo.disallowInternalStagingBuffer = true;
	//bufferInfo.requireDedicatedAllocation = true;
	bufferInfo.writePolicy = vtek::BufferWritePolicy::overwrite_often;
	bufferInfo.usageFlags
		= vtek::BufferUsageFlag::transfer_dst
		| vtek::BufferUsageFlag::vertex_buffer;
	vtek::Buffer* buffer = vtek::buffer_create(&bufferInfo, device);
	if (buffer == nullptr)
	{
		log_error("Failed to create vertex buffer!");
		return -1;
	}
	if (!fill_buffer(buffer, device))
	{
		log_error("Failed to fill vertex buffer!");
		return -1;
	}



	// Cleanup
	vtek::device_wait_idle(device);

	vtek::buffer_destroy(buffer);
	vtek::graphics_shader_destroy(shader, device);
	vtek::swapchain_destroy(swapchain, device);
	vtek::command_pool_destroy(graphicsCommandPool, device);
	vtek::device_destroy(device);
	vtek::physical_device_release(physicalDevice);
	vtek::window_surface_destroy(surface, instance);
	vtek::instance_destroy(instance);
	vtek::window_destroy(window);

	log_info("Program Success!");
	vtek::terminate();

	return 0;
}
