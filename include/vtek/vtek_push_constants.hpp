#pragma once

#include <cstdint>


namespace vtek
{
	// ========================== //
	// === Push constant enum === //
	// ========================== //
	enum class PushConstantType
	{
		none, // ie. a pipeline/shader program uses no push constants.
		vec3
	};

	const uint32_t push_constant_size(PushConstantType pct);


	template<uint32_t Size, PushConstantType Type, class Derived>
	struct PushConstant
	{
		// NOTE: Size must always be a multiple of 4.
		static_assert(Size % 4 == 0);

		constexpr uint32_t size() { return Size; }
		constexpr PushConstantType type() { return Type; }

		void cmdPush(VkCommandBuffer buf, VkPipelineLayout layout)
		{
			vkCmdPushConstants(buf, layout, stageFlags, 0, Size,
			                   (void*)(static_cast<Derived*>(this)));
		}

		VkShaderStageFlags stageFlags {0U};
	};


	// =========================== //
	// === Push constant types === //
	// =========================== //
	struct PushConstant_v3 : public PushConstant<
		sizeof(glm::vec3), PushConstantType::vec3, PushConstant_v3>
	{
		glm::vec3 v1 {0.0f};
	};
}
