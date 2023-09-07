#include "vtek_vulkan.pch"
#include "vtek_vertex_data.hpp"
#include "vtek_logging.hpp"


// ========================= //
// === Utility functions === //
// ========================= //
using VAT = vtek::VertexAttributeType;
using VIR = vtek::VertexInputRate;

static uint32_t get_vertex_size(VAT vt)
{
	switch (vt)
	{
	case VAT::uvec2: return sizeof(glm::uvec2);
	case VAT::ivec2: return sizeof(glm::ivec2);
	case VAT::vec2:  return sizeof(glm::vec2);
	case VAT::vec3:  return sizeof(glm::vec3);
	case VAT::vec4:  return sizeof(glm::vec4);
	default:
		vtek_log_error(
			"vtek_vertex_data.cpp: Invalid vertex type to calculate offset!");
		return 0U;
	}
}

static VkVertexInputRate get_vertex_input_rate(VIR rate)
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
	return (rate == VIR::per_instance)
		? VK_VERTEX_INPUT_RATE_INSTANCE
		: VK_VERTEX_INPUT_RATE_VERTEX;
}

static VkFormat get_vertex_format(VAT vt)
{
	switch (vt)
	{
	case VAT::uvec2: return VK_FORMAT_R32G32_UINT;
	case VAT::ivec2: return VK_FORMAT_R32G32_SINT;
	case VAT::vec2:  return VK_FORMAT_R32G32_SFLOAT;
	case VAT::vec3:  return VK_FORMAT_R32G32B32_SFLOAT;
	case VAT::vec4:  return VK_FORMAT_R32G32B32A32_SFLOAT;
	default:
		vtek_log_error(
			"vtek_vertex_data.cpp: Invalid vertex type to calculate format!");
		return VK_FORMAT_UNDEFINED;
	}
}


// ============================== //
// === Vertex buffer bindings === //
// ============================== //
void vtek::VertexBufferBindings::add_buffer(VAT vt, VIR rate)
{
	VkVertexInputAttributeDescription attrDesc{};
	attrDesc.binding = mBindCount;
	attrDesc.location = mLocCount;
	attrDesc.format = get_vertex_format(vt);
	attrDesc.offset = 0;
	mAttrDesc.emplace_back(attrDesc);

	VkVertexInputBindingDescription bindDesc{};
	bindDesc.binding = mBindCount;
	bindDesc.stride = get_vertex_size(vt);
	bindDesc.inputRate = get_vertex_input_rate(rate);
	mBindDesc.emplace_back(bindDesc);

	mBindCount++;
	mLocCount++;
}

void vtek::VertexBufferBindings::add_buffer(VAT vt1, VAT vt2, VIR rate)
{
	VkVertexInputAttributeDescription attrDesc1{};
	attrDesc1.binding = mBindCount;
	attrDesc1.location = mLocCount;
	attrDesc1.format = get_vertex_format(vt1);
	attrDesc1.offset = 0;
	mAttrDesc.emplace_back(attrDesc1);
	mLocCount++;

	VkVertexInputAttributeDescription attrDesc2{};
	attrDesc2.binding = mBindCount;
	attrDesc2.location = mLocCount;
	attrDesc2.format = get_vertex_format(vt2);
	attrDesc2.offset = get_vertex_size(vt1);
	mAttrDesc.emplace_back(attrDesc2);
	mLocCount++;

	VkVertexInputBindingDescription bindDesc{};
	bindDesc.binding = mBindCount;
	bindDesc.stride = get_vertex_size(vt1) + get_vertex_size(vt2);
	bindDesc.inputRate = get_vertex_input_rate(rate);
	mBindDesc.emplace_back(bindDesc);
	mBindCount++;
}

void vtek::VertexBufferBindings::add_buffer(VAT vt1, VAT vt2, VAT vt3, VIR rate)
{
	vtek_log_error(
		"vtek_vertex_data.cpp: add_buffer(vt1, vt2, vt3, rate): Not implemented!");
}

void vtek::VertexBufferBindings::add_buffer(
	VAT vt1, VAT vt2, VAT vt3, VAT vt4, VIR rate)
{
	vtek_log_error(
		"vtek_vertex_data.cpp: add_buffer(vt1, vt2, vt3, vt4, rate): {}",
		"Not implemented!");
}
