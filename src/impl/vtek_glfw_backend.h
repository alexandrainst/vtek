#pragma once

#include <vector>


namespace vtek
{
	bool glfw_backend_initialize();
	void glfw_backend_terminate();
	void glfw_backend_get_required_instance_extensions(std::vector<const char*>& list);
}
