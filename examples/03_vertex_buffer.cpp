#include <vtek/vtek.hpp>
#include <iostream>
#include <vector>

// global data
vtek::ApplicationWindow* window = nullptr;


void keyCallback(vtek::KeyboardKey key, vtek::InputAction action)
{
	if ((action == vtek::InputAction::release) &&
	    (key == vtek::KeyboardKey::escape))
	{
		vtek::window_set_should_close(window, true);
	}
}


struct Vertex2D
{
	glm::vec2 position;
	glm::vec3 color;
};


int main()
{
	// Initialize vtek
	vtek::InitInfo initInfo{};
	initInfo.disableLogging = false;
	initInfo.applicationTitle = "03_vertex_buffer";
	initInfo.useGLFW = true;
	initInfo.loadShadersFromGLSL = false;
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

	// vertex data
	std::vector<Vertex2D> vertices = {
		{{-0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}}
	};

	// vertex buffer
	vtek::BufferInfo bufferInfo{};
	bufferInfo.size = sizeof(Vertex2D) * vertices.size();
	//bufferInfo.requireHostVisibleStorage = true;
	//bufferInfo.disallowInternalStagingBuffer = true;
	//bufferInfo.requireDedicatedAllocation = true;
	bufferInfo.writePolicy = vtek::BufferWritePolicy::write_once;
	bufferInfo.usageFlags
		= vtek::BufferUsageFlag::transfer_dst
		| vtek::BufferUsageFlag::vertex_buffer;
	vtek::Buffer* buffer = vtek::buffer_create(&bufferInfo, device);
	if (buffer == nullptr)
	{
		log_error("Failed to create vertex buffer!");
		return -1;
	}

	if (!vtek::buffer_write_data(
		    buffer, vertices.data(), vertices.size()*sizeof(Vertex2D), device))
	{
		log_error("Failed to write data to vertex buffer!");
		return -1;
	}




	log_info("Program Success!");
	return 0;
}
