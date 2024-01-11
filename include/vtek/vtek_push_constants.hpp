#pragma once

#include <cstdint>

#include "vtek_glm_includes.hpp"
#include "vtek_types.hpp"


namespace vtek
{
	// ========================== //
	// === Push constant enum === //
	// ========================== //
	// NOTE: Certain types are NOT recommended, such as mat3. It might, or might
	// not, work depending on driver/hardware/etc. Best to just avoid them.
	enum class PushConstantType
	{
		none, // ie. a pipeline/shader program uses no push constants.
		vec3,
		mat4
	};


	struct IPushConstant
	{
		virtual uint32_t size() = 0;
		virtual void* data() = 0;
	};

	template<uint32_t Size, PushConstantType Type>
	struct PushConstant : public IPushConstant
	{
		// NOTE: Size must always be a multiple of 4.
		static_assert(Size % 4 == 0);
		static_assert(Type != PushConstantType::none); // TODO: Is this reasonable?

		uint32_t size() override { return Size; }
		static constexpr uint32_t static_size() { return Size; }

		constexpr PushConstantType type() { return Type; }

		// /*
		void cmdPush(VkCommandBuffer buf, VkPipelineLayout layout)
		{
			vkCmdPushConstants(buf, layout, stageFlags, 0, Size, data());
		}

		VkShaderStageFlags stageFlags {0U};
		// */
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
		alignas(16) glm::mat4 m1 {1.0f};
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
