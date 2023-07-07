#pragma once

#include <cstdint>

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
	// ========================== //
	// === Push constant enum === //
	// ========================== //
	enum class PushConstantType
	{
		none, // ie. a pipeline/shader program uses no push constants.
		vec3,
		mat4
	};


	template<uint32_t Size, PushConstantType Type>
	struct PushConstant
	{
		// NOTE: Size must always be a multiple of 4.
		static_assert(Size % 4 == 0);

		constexpr uint32_t size() { return Size; }
		static constexpr uint32_t static_size() { return Size; }

		constexpr PushConstantType type() { return Type; }
		virtual void* data() = 0;

		void cmdPush(VkCommandBuffer buf, VkPipelineLayout layout)
		{
			vkCmdPushConstants(buf, layout, stageFlags, 0, Size, data());
		}

		VkShaderStageFlags stageFlags {0U};
	};


	// =========================== //
	// === Push constant types === //
	// =========================== //
	struct PushConstant_v3 : public PushConstant<
		sizeof(glm::vec3), PushConstantType::vec3>
	{
		void* data() override { return static_cast<void*>(&v1); }
		glm::vec3 v1 {0.0f};
	};

	struct PushConstant_m4 : public PushConstant<
		sizeof(glm::mat4), PushConstantType::mat4>
	{
		void* data() override { return static_cast<void*>(&m1); }
		glm::mat4 m1 {0.0f};
	};


	// ============================= //
	// === Push constant helpers === //
	// ============================= //
	inline constexpr uint32_t push_constant_size(PushConstantType pct)
	{
		switch (pct)
		{
		case PushConstantType::none: return 0;
		case PushConstantType::vec3: return PushConstant_v3::static_size();
		case PushConstantType::mat4: return PushConstant_m4::static_size();
		default:
			return 0;
		}
	}
}
