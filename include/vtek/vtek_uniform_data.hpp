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
		vec3,
		mat4,

		point_light
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

	struct Uniform_m4
	{
		alignas(16) glm::mat4 m4;

		constexpr uint64_t size();
		inline constexpr UniformBufferType type()
		{
			return UniformBufferType::mat4;
		}
	};

	struct Uniform_PointLight
	{
		alignas(16) glm::vec4 positionFalloff; // {x,y,z}: position; TODO: w: falloff ?
		alignas(16) glm::vec4 colorIntensity; // {x,y,z}: color; w: intensity

		constexpr uint64_t size();
		inline constexpr UniformBufferType type()
		{
			return UniformBufferType::point_light;
		}
	};


	// ========================== //
	// === Member definitions === //
	// ========================== //
	constexpr uint64_t Uniform_v3::size() { return sizeof(Uniform_v3); }
	constexpr uint64_t Uniform_m4::size() { return sizeof(Uniform_m4); }
	constexpr uint64_t Uniform_PointLight::size() {
		return sizeof(Uniform_PointLight);
	}


	// ===================== //
	// === Memory checks === //
	// ===================== //
	static_assert(sizeof(Uniform_v3) == 16);
	static_assert(alignof(Uniform_v3) == 16);
	static_assert(offsetof(Uniform_v3, v3) == 0);

	static_assert(sizeof(Uniform_m4) == 64);
	static_assert(alignof(Uniform_m4) == 16);
	static_assert(offsetof(Uniform_m4, m4) == 0);

	static_assert(sizeof(Uniform_PointLight) == 32);
	static_assert(alignof(Uniform_PointLight) == 16);
	static_assert(offsetof(Uniform_PointLight, positionFalloff) == 0);
	static_assert(offsetof(Uniform_PointLight, colorIntensity) == 16);
}
