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

	// vertex data
	std::vector<Vertex2D> vertices = {
		{{-0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}}
	};

	// vertex buffer
	vtek::BufferInfo bufferInfo{};
	bufferInfo.size = sizeof(Vertex2D) * vertices.size();
	bufferInfo.usage = vtek::BufferUsage::overwrite_once;
	vtek::Buffer* buffer = vtek::buffer_create(&bufferInfo);
	if (buffer == nullptr)
	{
		log_error("Failed to create vertex buffer!");
		return -1;
	}

	return 0;
}
