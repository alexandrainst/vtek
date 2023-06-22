#pragma once

#include <cstdint>

#include "vtek_glm_includes.hpp"


namespace vtek
{
	// ==================== //
	// === Uniform enum === //
	// ==================== //
	enum class UniformBufferType
	{
		vec3
	};

	uint64_t get_uniform_buffer_size(UniformBufferType type);

	// TODO: Consider different types of uniforms? (layouts?)


	// ===================== //
	// === Uniform types === //
	// ===================== //
	struct Uniform_v3
	{
		alignas(16) glm::vec3 v3;

		constexpr uint64_t size();
		inline constexpr UniformBufferType type()
		{
			return UniformBufferType::vec3;
		}
	};


	// ========================== //
	// === Member definitions === //
	// ========================== //
	constexpr uint64_t Uniform_v3::size() { return sizeof(Uniform_v3); }


	// ===================== //
	// === Memory checks === //
	// ===================== //
	static_assert(sizeof(Uniform_v3) == 16);
	static_assert(alignof(Uniform_v3) == 16);
	static_assert(offsetof(Uniform_v3, v3) == 0);
}
