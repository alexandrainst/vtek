#include "vtek_vertex_data.h"
#include "vtek_logging.h"

// ================================ //
// === Vertex type descriptions === //
// ================================ //
// TODO: Could use a host allocator here to not store unneeded descriptions?
const vtek::BindingDescription& binding_desc_vertex_p2(bool instanced)
{
	static const vtek::BindingDescription desc = {
		.binding = 0,
		.stride = sizeof(vtek::Vertex_p2),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
	};
	static const vtek::BindingDescription desc_inst = {
		.binding = 0,
		.stride = sizeof(vtek::Vertex_p2),
		.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE
	};

	return (instanced) ? desc : desc_inst;
}


// =========================== //
// === Interface functions === //
// =========================== //
const vtek::BindingDescription& vtek::vertex_binding_description(
	vtek::VertexType vt, bool instanced)
{
	static const vtek::BindingDescription defaultDesc = {};

	switch (vt)
	{
	case vtek::VertexType::vec2:
		return binding_desc_vertex_p2(instanced);
	default:
		vtek_log_error("vtek_vertex_data.cpp: Invalid vertex type!");
		return defaultDesc;
	}
}

const vtek::AttributeDescriptions& vtek::vertex_attribute_descriptions(
	vtek::VertexType vt)
{
	static const vtek::AttributeDescriptions defaultDescs = {};

	switch (vt)
	{
	default:
		return defaultDescs;
	}
}
