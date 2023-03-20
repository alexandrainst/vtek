#pragma once


namespace vtek
{
	struct WindowCreateInfo
	{

	};

	struct ApplicationWindow; // opaque handle


	ApplicationWindow* glfw_window_create(const WindowCreateInfo* info);
}
