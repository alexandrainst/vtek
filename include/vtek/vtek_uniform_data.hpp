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
		mat4_vec4,

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
		static constexpr uint64_t static_size();
		inline constexpr UniformBufferType type()
		{
			return UniformBufferType::vec3;
		}
	};

	struct Uniform_m4
	{
		alignas(16) glm::mat4 m4;

		constexpr uint64_t size();
		static constexpr uint64_t static_size();
		inline constexpr UniformBufferType type()
		{
			return UniformBufferType::mat4;
		}
	};

	struct Uniform_m4_v4
	{
		alignas(16) glm::mat4 m4;
		alignas(16) glm::vec4 v4;

		constexpr uint64_t size();
		static constexpr uint64_t static_size();
		inline constexpr UniformBufferType type()
		{
			return UniformBufferType::mat4_vec4;
		}
	};

	struct Uniform_PointLight
	{
		alignas(16) glm::vec4 positionFalloff; // {x,y,z}: position; TODO: w: falloff ?
		alignas(16) glm::vec4 colorIntensity; // {x,y,z}: color; w: intensity

		constexpr uint64_t size();
		static constexpr uint64_t static_size();
		inline constexpr UniformBufferType type()
		{
			return UniformBufferType::point_light;
		}
	};



	// ========================== //
	// === Member definitions === //
	// ========================== //
	constexpr uint64_t Uniform_v3::static_size() { return sizeof(Uniform_v3); }
	constexpr uint64_t Uniform_m4::static_size() { return sizeof(Uniform_m4); }
	constexpr uint64_t Uniform_m4_v4::static_size() { return sizeof(Uniform_m4_v4); }
	constexpr uint64_t Uniform_PointLight::static_size() {
		return sizeof(Uniform_PointLight);
	}

	constexpr uint64_t vtek::Uniform_v3::size() { return static_size(); }
	constexpr uint64_t vtek::Uniform_m4::size() { return static_size(); }
	constexpr uint64_t vtek::Uniform_m4_v4::size() { return static_size(); }
	constexpr uint64_t vtek::Uniform_PointLight::size() { return static_size(); }
}
