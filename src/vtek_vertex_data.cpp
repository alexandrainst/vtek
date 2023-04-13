#include "vtek_vertex_data.h"

static const vtek::BindingDescription vtek::Vertex_p2::bindingDescription(
	bool instanced)
{
	vtek::BindingDescription desc{};
	desc.binding = 0;
	desc.stride = sizeof(vtek::Vertex_p2);
	desc.inputRate = (instanced)
		? VK_VERTEX_INPUT_RATE_INSTANCE
		: VK_VERTEX_INPUT_RATE_VERTEX;

	return desc;
}
