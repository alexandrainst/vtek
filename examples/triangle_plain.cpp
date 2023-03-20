#include <vtek/vtek.h>


int main()
{
	// Initialize vtek
	vtek::InitInfo initInfo{};
	initInfo.applicationTitle = "exTrianglePlain";
	initInfo.useGLFW = true;
	vtek::initialize(&initInfo);

	// Create window
	vtek::WindowCreateInfo windowInfo{};
	windowInfo.windowTitle = "triangle_plain";
	vtek::ApplicationWindow* window = vtek::glfw_window_create(&windowInfo);
	if (window == nullptr)
	{
		log_error("Failed to create window!");
		return -1;
	}

	// surface
	VkSurfaceKHR surface = vtek::glfw_window_get_surface(window);



	// Cleanup
	vtek::glfw_window_destroy(window);
	vtek::terminate();

	return 0;
}
