#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>
#include <string>
#include <vector>

#include "vtek_instance.h"


namespace vtek
{
	struct WindowCreateInfo
	{
		const char* title {"sample-app"};
		uint32_t width {512};
		uint32_t height {512};
		bool fullscreen {false};
	};

	struct ApplicationWindow; // opaque handle


	ApplicationWindow* window_create(const WindowCreateInfo* info);
	void window_destroy(ApplicationWindow* window);

	// surface for the application window, needed for Vulkan
	VkSurfaceKHR window_create_surface(ApplicationWindow* window, Instance* instance);
	void window_surface_destroy();

	void window_set_key_handler(ApplicationWindow* window);

	void window_get_framebuffer_size(ApplicationWindow* window, int* width, int* height);
	const std::vector<std::string>& window_get_required_instance_extensions(ApplicationWindow* window);
}
