#pragma once

#include <vulkan/vulkan.h>
#include <optional>
#include <vector>

#include "vtek_descriptor_type.hpp"
#include "vtek_object_handles.hpp"


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
	void descriptor_pool_reset(DescriptorPool* pool, Device* device);


	// ======================= //
	// === Descriptor sets === //
	// ======================= //
	DescriptorSet* descriptor_pool_alloc_set(
		DescriptorPool* pool, DescriptorSetLayout* layout, Device* device);

	std::vector<DescriptorSet*> descriptor_pool_alloc_sets(
		DescriptorPool* pool, DescriptorSetLayout* layout,
		uint32_t numSets, Device* device);

	void descriptor_pool_free_set(
		DescriptorPool* pool, DescriptorSet* set, Device* device);

	void descriptor_pool_free_sets(
		DescriptorPool* pool, std::vector<DescriptorSet*>& sets, Device* device);
}
