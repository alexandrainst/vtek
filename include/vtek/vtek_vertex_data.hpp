#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

#include "vtek_glm_includes.hpp"


namespace vtek
{
	// =================== //
	// === Vertex enum === //
	// =================== //
	// Type of a single vertex attribute, as if provided by a single vertex
	// buffer. When multiple vertex attributes are packed together in the
	// same vertex buffer, a list of these should be provided to the
	// `VertexDescription` class when specifying the bindings.
	enum class VertexAttributeType
	{
		uvec2,
		ivec2,
		vec2,
		vec3,
		vec4
	};

	// With a per-instance input rate, the vertex shader only updates the
	// contents of the vertex attribute on a per-instance basis. This
	// allows for instanced rendering without an external buffer containing
	// the per-instance transforms (like UBO array of SSBO). Instanced
	// rendering may be done without settings this flag, in which case
	// a UBO array or an SSBO should be bound.
	enum class VertexInputRate
	{
		per_vertex, per_instance
	};


	// ====================================== //
	// === Binding/attribute descriptions === //
	// ====================================== //
	class VertexBufferBindings
	{
	public:
		using VAT = VertexAttributeType;
		using VIR = VertexInputRate;

		void add_buffer(VAT vt, VIR rate);
		void add_buffer(VAT vt1, VAT vt2, VIR rate);
		void add_buffer(VAT vt1, VAT vt2, VAT vt3, VIR rate);
		void add_buffer(VAT vt1, VAT vt2, VAT vt3, VAT vt4, VIR rate);

		using BindingList = std::vector<VkVertexInputBindingDescription>;
		using AttributeList = std::vector<VkVertexInputAttributeDescription>;

		inline const BindingList& GetBindingDescriptions() { return mBindDesc; }
		inline const AttributeList& GetAttributeDescriptions() { return mAttrDesc; }

	private:
		BindingList mBindDesc {};
		AttributeList mAttrDesc {};
		uint32_t mBindCount {0};
		uint32_t mLocCount {0};
	};


	// ==================== //
	// === Vertex types === //
	// ==================== //
	struct Vertex_v2
	{
		// data
		glm::vec2 pos;

		// descriptions, only static access
		// TODO: Does this work?
		static constexpr VertexAttributeType kVertexType =
			VertexAttributeType::vec2;
	};
}


// Hashing, necessary for certain operations such as using vertex types as
// keys in an std::unordered_map, needed by model loading..
// REVIEW: Place in model loading file instead?
namespace std
{
	template<> struct hash<vtek::Vertex_v2>
	{
		std::size_t operator()(const vtek::Vertex_v2& v) const
		{
			return hash<glm::vec2>()(v.pos);
		}
	};
}
