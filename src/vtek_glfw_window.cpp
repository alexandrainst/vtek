// glfw
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std
#include <vector>

// vtek
#include "vtek_glfw_window.h"
#include "vtek_logging.h"


/* struct implementation */
struct vtek::ApplicationWindow
{
	GLFWwindow* glfwHandle {nullptr};
	int framebufferWidth {0};
	int framebufferHeight {0};
};



/* implementation of GLFW backend */
#include "impl/vtek_glfw_backend.h"

static std::vector<std::string> sRequiredInstanceExtensions {};
static bool sGlfwEnabled {false};

bool vtek::glfw_backend_initialize()
{
	if (!glfwInit()) { return false; }

	// Get required instance extensions from GLFW
	glfwMakeContextCurrent(window);
	uint32_t extensionCount = 0;
	const char** extensions;
	extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
	for (uint32_t i = 0; i < extensionCount; i++)
	{
		sRequiredInstanceExtensions.push_back(extensions[i]);
	}

	return true;
}

void vtek::glfw_backend_terminate()
{
	glfwTerminate();
}

bool vtek::glfw_backend_is_enabled()
{

}



/* helper functions */



/* interface */
vtek::ApplicationWindow* vtek::window_create(const vtek::WindowCreateInfo* info)
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwSwapInterval(1);

	GLFWwindow* window = glfwCreateWindow(info->width, info->height, info->title, NULL, NULL);
	if (window == nullptr)
	{
		vtek_log_error("Failed to create GLFW window!");
		return nullptr;
	}

	// Allocate window
	vtek::ApplicationWindow* appWindow = new vtek::ApplicationWindow();

	// Get framebuffer size, needed when creating a swapchain. Description below:
	//
	// Window resolutions are measured in virtual screen coordinates,
	// but swap image sizes are measured in pixels. These do not always
	// correspond, e.g. on high DPI displays such as Apple's Retina.
	// So after the window is created, we can query GLFW for the
	// framebuffer size, which is always in pixels, and use this value
	// to determine an appropriate swap image size.
	glfwGetFramebufferSize(window, &appWindow->framebufferWidth, &appWindow->framebufferHeight);

	return appWindow;
}

void vtek::window_destroy(vtek::ApplicationWindow* window)
{
	if (window == nullptr) { return; }

	if (window->glfwHandle != nullptr)
	{
		glfwDestroyWindow(window->glfwHandle);
	}
}
