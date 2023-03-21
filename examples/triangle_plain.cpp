#include <vtek/vtek.h>
#include <iostream>


int main()
{
	std::cout << "a\n";
	// Initialize vtek
	vtek::InitInfo initInfo{};
	initInfo.applicationTitle = "triangle_plain";
	initInfo.useGLFW = true;
	std::cout << "b\n";
	vtek::initialize(&initInfo);
	std::cout << "c\n";

	// // Create window
	// vtek::WindowCreateInfo windowInfo{};
	// windowInfo.title = "triangle_plain";
	// windowInfo.width = 500;
	// windowInfo.height = 500;
	// vtek::ApplicationWindow* window = vtek::window_create(&windowInfo);
	// if (window == nullptr)
	// {
	// 	log_error("Failed to create window!");
	// 	return -1;
	// }

	// // Vulkan instance
	// vtek::InstanceCreateInfo instanceInfo{};
	// instanceInfo.applicationName = "triangle_plain";
	// instanceInfo.applicationVersion = vtek::VulkanVersion(1, 0, 0);
	// instanceInfo.enableValidationLayers = true;
	// auto instance = vtek::instance_create(&instanceInfo);
	// if (instance == nullptr)
	// {
	// 	log_error("Failed to create Vulkan instance!");
	// 	return -1;
	// }

	// // Surface
	// VkSurfaceKHR surface = vtek::window_create_surface(window, instance);
	// if (surface == VK_NULL_HANDLE)
	// {
	// 	log_error("Failed to create Vulkan window surface!");
	// 	return -1;
	// }


	// // Cleanup
	// log_debug("All went well!");

	// vtek::window_destroy(window);
	// vtek::terminate();

	return 0;
}
