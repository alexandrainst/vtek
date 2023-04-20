#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "vtek_input.hpp"
#include "vtek_instance.hpp"


namespace vtek
{
	struct WindowCreateInfo
	{
		const char* title {"sample-app"};
		uint32_t width {512};
		uint32_t height {512};
		bool fullscreen {false};

		bool cursorDisabled {false};
	};

	struct ApplicationWindow; // opaque handle


	ApplicationWindow* window_create(const WindowCreateInfo* info);
	void window_destroy(ApplicationWindow* window);

	void window_poll_events();
	void window_get_framebuffer_size(ApplicationWindow* window, int* width, int* height);
	bool window_get_should_close(ApplicationWindow* window);
	void window_set_should_close(ApplicationWindow* window, bool shouldClose);

	// surface for the application window, needed for Vulkan
	VkSurfaceKHR window_create_surface(ApplicationWindow* window, Instance* instance);
	void window_surface_destroy(VkSurfaceKHR surface, Instance* instance);

	// event handling
	typedef std::function<void(KeyboardKey,InputAction)> tKeyCallback;
	typedef std::function<void(MouseButton,InputAction)> tMouseButtonCallback;
	typedef std::function<void(double,double)> tMouseMoveCallback;
	typedef std::function<void(double, double)> tMouseScrollCallback;

	void window_set_key_handler(ApplicationWindow* window, tKeyCallback fn);
	void window_set_mouse_button_handler(ApplicationWindow* window, tMouseButtonCallback fn);
	void window_set_mouse_move_handler(ApplicationWindow* window, tMouseMoveCallback fn);
	void window_set_mouse_scroll_handler(ApplicationWindow* window, tMouseScrollCallback fn);
}
