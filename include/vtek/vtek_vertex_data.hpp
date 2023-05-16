#pragma once

#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

// TODO: Provide centralized GLM header?
#define GLM_FORCE_RADIANS
// The perspective projection matric generated by GLM will use the OpenGL
// depth range of (-1.0, 1.0) by default. We need it to use the Vulkan
// range of (0.0, 1.0) instead, which is forced by using this definition.
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
// The hash functions `std::hash<glm::_types_>` are defined in the gtx/
// folder which means that it's technically still an experimental
// extension to GLM. Therefore, we need to define experimental use.
// It means that the API could change with a new version of GLM in the
// future, but in practice the API is very stable.
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>


namespace vtek
{
	// =================== //
	// === Vertex enum === //
	// =================== //
	// Type of a single vertex attribute, as if provided by a single vertex
	// buffer. When multiple vertex attributes are packed together in the
	// same vertex buffer, a list of these should be provided to the
	// `VertexDescription` class when specifying the bindings.
	enum class VertexAttribute
	{
		uvec2,
		ivec2,
		vec2,
		vec3,
		vec4
	};

	enum class PackedVertexAttribute
	{

	};


	// ====================================== //
	// === Binding/attribute descriptions === //
	// ====================================== //
	class VertexBufferBinding
	{

	};

	class VertexBufferBindings
	{
	public:
		using BindingList = std::vector<VkVertexInputBindingDescription>;
		using AttributeList = std::vector<VkVertexInputAttributeDescription>;

		inline const BindingList& GetBindingDescriptions() { return mBindDesc; }
		inline const AttributeList& GetAttributeDescriptions() { return mAttrDesc; }

		// With `instancedVertexArray`, the vertex shader only updates the
		// contents of the vertex attribute on a per-instance basis. This
		// allows for instanced rendering without an external buffer containing
		// the per-instance transforms (like UBO array of SSBO). Instanced
		// rendering may be done without settings this flag, in which case
		// a UBO array or an SSBO should be bound.
		void add_attribute(VertexType vt, bool instancedVertexArray = false);

		void add_attributes(std::vector<VertexType> vts);

	private:
		uint32_t mNumAttrBindings {0};
		uint32_t mAttrOffset {0};
		BindingList mBindDesc;
		AttributeList mAttrDesc;
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
		static constexpr VertexType kVertexType = VertexType::vec2;
	};
}


// Hashing, necessary for certain operations such as using vertex types as
// keys in an std::unordered_map, needed by model loading..
// REVIEW: Place in model loading file instead?
namespace std
{
	template<> struct hash<vtek::Vertex_p2>
	{
		std::size_t operator()(const vtek::Vertex_p2& v) const
		{
			return hash<glm::vec2>()(v.pos);
		}
	};
}
