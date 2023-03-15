// Internal header file, do not include.

#pragma once

namespace vtek
{
	struct Queue
	{
		VkQueue vulkanHandle {VK_NULL_HANDLE};
		uint32_t familyIndex {UINT32_MAX}; // or UINT_MAX, or std::numeric_limits<uint32_t>::max_value()
		VkQueueFlags queueFlags {0}; // VK_QUEUE_TRANSFER_BIT etc.
		bool presentSupport {false};
	};
}
