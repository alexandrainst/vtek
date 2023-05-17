#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "vtek_input.hpp"
#include "vtek_vulkan_handles.hpp"


namespace vtek
{
	struct WindowCreateInfo
	{
		const char* title {"sample-app"};
		uint32_t width {512};
		uint32_t height {512};
		bool fullscreen {false};
		bool maximized {false};

		bool cursorDisabled {false};
		bool resizeable {false};
		bool decorated {true};
	};


	// Create/destroy window
	ApplicationWindow* window_create(const WindowCreateInfo* info);
	void window_destroy(ApplicationWindow* window);

	// surface for the application window, needed for Vulkan
	VkSurfaceKHR window_create_surface(ApplicationWindow* window, Instance* instance);
	void window_surface_destroy(VkSurfaceKHR surface, Instance* instance);


	// =============================== //
	// === Basic window management === //
	// =============================== //
	void window_poll_events();
	void window_get_framebuffer_size(
		ApplicationWindow* window, uint32_t* width, uint32_t* height);
	bool window_get_should_close(ApplicationWindow* window);
	void window_set_should_close(ApplicationWindow* window, bool shouldClose);

	// Wait while the window is minimized, ie. has a framebuffer of size 0.
	// This should be used as a guard for when a swapchain is "out-of-date", and
	// wait until the window is no longer minimized before re-creating the swapchain.
	void window_wait_while_minimized(ApplicationWindow* window);

	bool window_is_resizing(ApplicationWindow* window);

	// Wait while the window is being resized. This should be used as a guard
	// inside the rendering loop of an application, so the swapchain will _likely_
	// only be resized when the user has finished resizing the window, and NOT
	// every frame (which would be expensive!).
	// NOTE: Current implementation sleeps for 100 milliseconds in-between while
	// checking a condition.
	void window_wait_while_resizing(ApplicationWindow* window);


	// ========================= //
	// === Window attributes === //
	// ========================= //
	// Obtain the ratio between the current DPI and the platform's default DPI,
	// important for scaling text and UI elements. This ensures that the pixel
	// dimensions of text and UI, scaled by these values, will appear at a
	// reasonable size on other machines and monitors, regardless of their DPI
	// and scaling settings.
	void window_get_content_scale(ApplicationWindow* window, float* scaleX, float* scaleY);


	// ====================== //
	// === Event handling === //
	// ====================== //
	typedef std::function<void(KeyboardKey,InputAction)> tKeyCallback;
	typedef std::function<void(MouseButton,InputAction)> tMouseButtonCallback;
	typedef std::function<void(double,double)> tMouseMoveCallback;
	typedef std::function<void(double, double)> tMouseScrollCallback;
	typedef std::function<void(void)> tFramebufferResizeCallback;

	void window_set_key_handler(
		ApplicationWindow* window, tKeyCallback fn);
	void window_set_mouse_button_handler(
		ApplicationWindow* window, tMouseButtonCallback fn);
	void window_set_mouse_move_handler(
		ApplicationWindow* window, tMouseMoveCallback fn);
	void window_set_mouse_scroll_handler(
		ApplicationWindow* window, tMouseScrollCallback fn);

	void window_set_framebuffer_resize_handler(
		ApplicationWindow* window, tFramebufferResizeCallback fn);
}
