// glfw
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// vtek
#include "vtek_glfw_window.h"
#include "vtek_logging.h"


/* struct implementation */
struct vtek::ApplicationWindow
{
	GLFWwindow* window {nullptr};
};



/* helper functions */
static bool sGlfwInitialized = false;

static bool initialize_glfw()
{
	return (!glfwInit()) ? false : true;
}



/* interface */
vtek::ApplicationWindow* vtek::window_create(const vtek::WindowCreateInfo* info)
{
	if (!sGlfwInitialized && !initialize_glfw())
	{
		vtek_log_error("Failed to initialize GLFW!");
		return nullptr;
	}


	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwSwapInterval(1);

	GLFWwindow* window = glfwCreateWindow(info->width, info->height, info->title, NULL, NULL);
	if (window == nullptr)
	{
		vtek_log_error("Failed to create GLFW window!");
		return nullptr;
	}


	return nullptr;
}

void vtek::window_destroy(vtek::ApplicationWindow* window)
{
	if (window == nullptr) { return; }

	if (window->glfwHandle != nullptr)
	{
		glfwDestroyWindow
	}
}
