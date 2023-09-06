// Internal header file, do not include.

#pragma once

#include <deque>
#include <vector>


namespace vtek
{
	struct DescriptorSet
	{
		VkDescriptorSet vulkanHandle {VK_NULL_HANDLE};

		std::deque<VkDescriptorBufferInfo> bufferInfos;
		std::deque<VkDescriptorImageInfo> imageInfos;
		std::vector<VkWriteDescriptorSet> writeDescriptors;
	};
}
