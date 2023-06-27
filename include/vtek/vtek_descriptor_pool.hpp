#pragma once

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

#include "vtek_vulkan_handles.hpp"
#include "vtek_descriptor_type.hpp"


namespace vtek
{
	struct DescriptorPoolType
	{
		DescriptorType type;
		uint32_t count {0};
	};

	struct DescriptorPoolInfo
	{
		// Specifies that descriptor sets created from this pool can return
		// their individual allocations to the pool.
		// If false, then only the entire pool may be reset, freeing all
		// allocated descriptor sets.
		bool allowIndividualFree {false};

		bool allowUpdateAfterBind {false};

		// A collection of different types of descriptors that may be bound
		// in descriptor sets allocated from this pool.
		std::vector<DescriptorPoolType> descriptorTypes;
	};


	DescriptorPool* descriptor_pool_create(
		DescriptorPoolInfo* info, Device* device);
	void descriptor_pool_destroy(DescriptorPool* pool, Device* device);

	VkDescriptorPool descriptor_pool_get_handle(DescriptorPool* pool);

	bool descriptor_pool_individual_free(DescriptorPool* pool);
	bool descriptor_pool_update_after_bind(DescriptorPool* pool);

	// Frees all allocated descriptor sets from the pool.
	// Before calling this function, make sure that no descriptor sets allocated
	// from this pool are still in use.
	void descriptor_pool_reset(DescriptorPool* pool);

	// TODO: This is the same problem as with command pools and command buffers:
	// Descriptor sets are ALLOCATED, not created, from a descriptor pool.
	// It does not really make sense to "destroy" a descriptor set, and it is
	// CERTAINLY NOT a descriptor set's responsibility to free itself!

	void descriptor_pool_free_set(DescriptorPool* pool);
}
