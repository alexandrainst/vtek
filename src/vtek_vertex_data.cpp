#include "vtek_vertex_data.hpp"
#include "vtek_logging.hpp"

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

// ============================= //
// === Alternative interface === //
// ============================= //
static const vtek::BindingDescription& bind_desc_empty()
{

}
static const vtek::BindingDescription& bind_desc_vec2()
{
	// A vertex binding describes at which rate to load data from memory
	// throughout the vertices. It specifies the number of bytes between
	// data entries and whether to move to the next data entry after each
	// vertex or after each instance.
	// The `binding` parameter specifies the index of the binding in the
	// array of bindings.
	// The `stride` parameter specifies the number of bytes from one entry
	// to the next.
	// The `inputRate` parameter can have two values:
	// - VK_VERTEX_INPUT_RATE_VERTEX:
	//     Move to the next data entry after each vertex.
	// - VK_VERTEX_INPUT_RATE_INSTANCE:
	//     Move to the next data entry after each instance.
	VkVertexInputBindingDescription desc{};
	desc.binding = ; // ?
	desc.stride = sizeof(vtek::Vertex_v2); // ?
}

const vtek::BindingDescription& vtek::BindingDescription::get(
	vtek::VertexType vt, bool instancedVertexArray)
{
	switch (vt)
	{
	case vtek::VertexType::empty: return bind_desc_empty();
	}
}

void vtek::BindingDescription::add_binding(
	vtek::VertexType vt, bool instancedVertexArray)
{
	
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
