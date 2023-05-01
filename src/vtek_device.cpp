// standard
#include <algorithm>
#include <map>
#include <optional>
#include <set>
#include <vector>

// vtek
#include "vtek_device.hpp"

#include "impl/vtek_host_allocator.hpp"
#include "impl/vtek_init.hpp"
#include "vtek_instance.hpp"
#include "vtek_logging.hpp"
#include "vtek_physical_device.hpp"


/* queue implementation */
#include "impl/vtek_queue_struct.hpp"


/* struct implementation */
struct vtek::Device
{
	uint64_t id {VTEK_INVALID_ID};
	VkDevice vulkanHandle {VK_NULL_HANDLE};
	vtek::VulkanVersion vulkanVersion {1, 0, 0}; // TODO: Compiles?

	VkPhysicalDeviceFeatures enabledFeatures {};
	vtek::DeviceExtensions enabledExtensions {};

	// TODO: We really **do** need a queue allocator. Example:
	// Say 1 transfer queue == graphics queue, then destroying graphics queue
	// would be highly ambiguous!
	// OKAY: This should be done!
	vtek::HostAllocator<vtek::Queue>* queueAllocator {nullptr};
	vtek::Queue graphicsQueue {};
	vtek::Queue presentQueue {};
	std::vector<vtek::Queue> transferQueues {};
	std::vector<vtek::Queue> computeQueues {};
};


/* host allocator */
// TODO: Because of the high memory requirement, perhaps this particular allocator
//       should store a pointer to a vtek::Device instead, as an optimization.
// OKAY: This should be done!
static vtek::HostAllocator<vtek::Device> sAllocator("vtek_device");


/* helper functions */
// NOTE: We could use a different set of queue priorities.
static const uint32_t gMaxQueueCount = 16;
static const float gQueuePriorities[gMaxQueueCount] = {
	1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
};

// Used for fetching queues after device creation
struct QueueDescription
{
	uint32_t familyIndex {0};
	uint32_t familyMaxCount {0};
	uint32_t queueCount {0};
	VkQueueFlags queueFlags {0};
	bool present {false}; // TODO: Needed ?
};

enum class PresentPlacement
{
	none, graphics_queue, compute_family
};
enum class TransferPlacement
{
	graphics_queue, graphics_family, separate_family, compute_family
};
enum class ComputePlacement
{
	none, graphics_queue, graphics_family, separate_family
};

struct QueueFamilySelections
{
	std::optional<QueueDescription> graphics {std::nullopt};

	PresentPlacement presentPlacement {PresentPlacement::none};
	std::optional<QueueDescription> present {std::nullopt};

	TransferPlacement transferPlacement {TransferPlacement::graphics_queue};
	std::optional<QueueDescription> transfer {std::nullopt};

	ComputePlacement computePlacement {ComputePlacement::none};
	std::optional<QueueDescription> compute {std::nullopt};
};



static bool create_queue_infos(
	const vtek::DeviceCreateInfo* info,
	const vtek::PhysicalDevice* physicalDevice,
	std::vector<VkDeviceQueueCreateInfo>& createInfos,
	QueueFamilySelections* queueSelections)
{
	const vtek::PhysicalDeviceQueueSupport* support =
		vtek::physical_device_get_queue_support(physicalDevice);

	bool separateCompute = support->hasSeparateComputeFamily && info->preferSeparateComputeQueue;
	bool doGraphics = support->graphics && support->graphicsRequired;
	bool doPresent = support->present && support->presentRequired;
	bool doCompute = support->compute && support->computeRequired;
	bool graphicsWithCompute = support->graphics && support->graphicsHasCompute;

	// === QUEUE SELECTIONS ===
	*queueSelections = {}; // reset
	QueueDescription graphicsDescription{};
	QueueDescription computeDescription{};

	// === QUEUE CREATE INFOS === //

	// graphics queue family
	if (doGraphics)
	{
		uint32_t minNum = 1;
		const uint32_t maxNum = std::min(gMaxQueueCount, support->graphicsMaxCount);
		VkDeviceQueueCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		createInfo.pNext = nullptr;
		// If flags are non-zero, then the queues must be fetched after
		// device creation with `vkGetDeviceQueue2` instead of `vkGetDeviceQueue`.
		createInfo.flags = 0;
		// Always 1 graphics queue (recommended by most sources)
		createInfo.queueCount = 1;
		// We can assign priority to queues to influence the scheduling of
		// command buffer execution, using floating-point numbers btw. 0-1.
		// This is required even if there is only a single queue.
		createInfo.pQueuePriorities = gQueuePriorities;
		createInfo.queueFamilyIndex = support->graphicsIndex;

		graphicsDescription.queueFlags = VK_QUEUE_GRAPHICS_BIT;

		// If the physical device doesn't have a separate transfer family, but we
		// still wish for graphics and transfer operations to be two different queues.
		if (!support->hasSeparateTransferFamily && info->preferSeparateTransferQueue)
		{
			createInfo.queueCount += std::max(1U, info->numTransferQueues); // perhaps the user specified 0
			if ((minNum + 1) <= support->graphicsMaxCount) { minNum++; }

			graphicsDescription.queueFlags |= VK_QUEUE_TRANSFER_BIT;
			queueSelections->transferPlacement = TransferPlacement::graphics_family;
		}
		else if (!info->preferSeparateTransferQueue)
		{
			graphicsDescription.queueFlags |= VK_QUEUE_TRANSFER_BIT;
			queueSelections->transferPlacement = TransferPlacement::graphics_queue;
		}

		// If the physical device doesn't have a separate compute family, but we
		// will wish for compute and graphics operations to be two different queues.
		if (doCompute && !support->hasSeparateComputeFamily && info->preferSeparateComputeQueue)
		{
			createInfo.queueCount += std::max(1U, info->numComputeQueues); // perhaps the user specified 0
			if ((minNum + 1) <= support->graphicsMaxCount) { minNum++; }

			graphicsDescription.queueFlags |= VK_QUEUE_COMPUTE_BIT;
			queueSelections->computePlacement = ComputePlacement::graphics_family;
		}
		else if (doCompute && !info->preferSeparateComputeQueue)
		{
			graphicsDescription.queueFlags |= VK_QUEUE_COMPUTE_BIT;
			queueSelections->computePlacement = ComputePlacement::graphics_queue;
		}

		// We default to presentation inside the graphics queue, _unless_ there's a
		// separate compute family which has present and which has been marked as the
		// preferred place for transfer operations.
		bool presentHere = (info->preferPresentInComputeQueue)
			? !(support->hasSeparateComputeFamily && support->separateComputeHasPresent)
			: true;
		if (doPresent && support->graphicsHasPresent && presentHere)
		{
			queueSelections->presentPlacement = PresentPlacement::graphics_queue;
			graphicsDescription.present = true;
		}

		if (support->graphicsHasSparseBinding)
		{
			graphicsDescription.queueFlags |= VK_QUEUE_SPARSE_BINDING_BIT;
		}

		createInfo.queueCount = std::clamp(createInfo.queueCount, minNum, maxNum);
		createInfos.emplace_back(createInfo);

		// So we can later retrieve the EXACT amount of queues created.
		graphicsDescription.familyIndex = createInfo.queueFamilyIndex;
		graphicsDescription.familyMaxCount = maxNum;
		graphicsDescription.queueCount = createInfo.queueCount;

		queueSelections->graphics = std::move(graphicsDescription);
	}

	// separate compute queue
	if (doCompute && separateCompute)
	{
		const uint32_t maxNum = std::min(support->separateComputeMaxCount, gMaxQueueCount);
		VkDeviceQueueCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.queueFamilyIndex = support->separateComputeIndex;
		createInfo.queueCount = std::max(1U, info->numComputeQueues); // perhaps the user specified 0
		createInfo.pQueuePriorities = gQueuePriorities;

		computeDescription.queueFlags = VK_QUEUE_COMPUTE_BIT;

		// We prefer presentation to be inside the compute queue
		if (doPresent && info->preferPresentInComputeQueue && support->separateComputeHasPresent)
		{
			createInfo.queueCount++;

			queueSelections->presentPlacement = PresentPlacement::compute_family;
			computeDescription.present = true;
		}

		if (!support->hasSeparateTransferFamily && !doGraphics)
		{
			createInfo.queueCount += std::max(1U, info->numTransferQueues); // perhaps the user specified 0

			computeDescription.queueFlags |= VK_QUEUE_TRANSFER_BIT;
			queueSelections->transferPlacement = TransferPlacement::compute_family;
		}

		createInfo.queueCount = std::clamp(info->numComputeQueues, 1U, maxNum);
		createInfos.emplace_back(createInfo);

		computeDescription.familyIndex = support->separateComputeIndex;
		computeDescription.familyMaxCount = maxNum;
		computeDescription.queueCount = createInfo.queueCount;
		queueSelections->compute = std::move(computeDescription);
	}

	// compute, not dedicated, but part of the "graphics" queue.
	// This should trigger when graphic support was not required but the graphics family supports compute.
	if (doCompute && graphicsWithCompute && !support->graphicsRequired && !support->hasSeparateComputeFamily)
	{
		const uint32_t maxNum = std::min(support->graphicsMaxCount, gMaxQueueCount);
		VkDeviceQueueCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.queueFamilyIndex = support->graphicsIndex;
		createInfo.queueCount = std::max(1U, info->numComputeQueues); // perhaps the user specified 0
		createInfo.pQueuePriorities = gQueuePriorities;

		computeDescription.queueFlags = VK_QUEUE_COMPUTE_BIT;

		// transfer queue same as this family, but since compute will potentially have
		// multiple queues (by design choice) we add them on top here.
		if (!support->hasSeparateTransferFamily)
		{
			createInfo.queueCount += std::max(1U, info->numTransferQueues); // perhaps the user specified 0
			computeDescription.queueFlags |= VK_QUEUE_TRANSFER_BIT;

			// Technically speaking it's the graphics family, but since graphics is not enabled
			// we associate transfer with the family used for compute.
			queueSelections->transferPlacement = TransferPlacement::compute_family;
		}

		if (support->graphicsHasSparseBinding)
		{
			computeDescription.queueFlags |= VK_QUEUE_SPARSE_BINDING_BIT;
		}

		if (doPresent && support->graphicsHasPresent)
		{
			createInfo.queueCount++;

			computeDescription.present = true;
			queueSelections->presentPlacement = PresentPlacement::compute_family;
		}

		createInfo.queueCount = std::clamp(createInfo.queueCount, 1U, maxNum);
		createInfos.emplace_back(createInfo);

		computeDescription.familyIndex = support->graphicsIndex;
		computeDescription.familyMaxCount = maxNum;
		computeDescription.queueCount = createInfo.queueCount;
		queueSelections->compute = std::move(computeDescription);
	}

	// possibly a separate transfer queue that's different from graphics/compute queues
	if (support->hasSeparateTransferFamily && info->preferSeparateTransferQueue)
	{
		const uint32_t maxNum = std::min(gMaxQueueCount, support->separateTransferMaxCount);
		VkDeviceQueueCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.queueFamilyIndex = support->separateTransferIndex;
		createInfo.queueCount = std::clamp(info->numTransferQueues, 1U, maxNum);
		createInfo.pQueuePriorities = gQueuePriorities;

		createInfos.emplace_back(createInfo);

		QueueDescription transferDescription{};
		transferDescription.familyIndex = support->separateTransferIndex;
		transferDescription.familyMaxCount = maxNum;
		transferDescription.queueCount = createInfo.queueCount;
		transferDescription.queueFlags = VK_QUEUE_TRANSFER_BIT;

		if (support->separateTransferHasSparseBinding)
		{
			transferDescription.queueFlags |= VK_QUEUE_SPARSE_BINDING_BIT;
		}

		queueSelections->transferPlacement = TransferPlacement::separate_family;
		queueSelections->transfer = std::move(transferDescription);
	}

	// TODO: We could (should?) have some post-condition guards here..

	return true;
}



// Call this function _after_ device creation to create the queues
static void create_device_queues(
	vtek::Device* device, const vtek::DeviceCreateInfo* info,
	const QueueFamilySelections* selections)
{
	VkDevice handle = device->vulkanHandle;

	if (selections->graphics.has_value())
	{
		const QueueDescription& description = selections->graphics.value();
		vtek::Queue* queue = &device->graphicsQueue;
		const uint32_t maxNum = description.familyMaxCount;
		const uint32_t num = description.queueCount;
		const uint32_t familyIndex = description.familyIndex;

		// get graphics queue
		vkGetDeviceQueue(handle, familyIndex, 0, &queue->vulkanHandle);
		queue->familyIndex = familyIndex;
		queue->queueFlags = description.queueFlags;

		// set inline present queue
		if (selections->presentPlacement == PresentPlacement::graphics_queue)
		{
			device->presentQueue = *queue;
		}

		// set inline compute queue
		if (selections->computePlacement == ComputePlacement::graphics_queue)
		{
			device->computeQueues.emplace_back(*queue);
		}

		// set inline transfer queue
		if (selections->transferPlacement == TransferPlacement::graphics_queue)
		{
			device->transferQueues.emplace_back(*queue);
		}

		// compute and transfer queues are separate but both inside graphics family.
		// So we need to check the number of available queues and distribute them evenly.
		if (selections->transferPlacement == TransferPlacement::graphics_family &&
		    selections->computePlacement == ComputePlacement::graphics_family)
		{
			// base case
			if (num == 1)
			{
				device->computeQueues.emplace_back(*queue);
				device->transferQueues.emplace_back(*queue);
			}
			else if (num == 2)
			{
				vtek::Queue tcQueue{};
				vkGetDeviceQueue(handle, familyIndex, 1, &tcQueue.vulkanHandle);
				tcQueue.familyIndex = familyIndex;
				tcQueue.queueFlags = description.queueFlags;
				tcQueue.presentSupport = false;

				device->transferQueues.emplace_back(tcQueue);
				device->computeQueues.emplace_back(tcQueue);
			}
			else
			{
				uint32_t numComputeQueues = std::min(1U, info->numComputeQueues);
				uint32_t numTransferQueues = std::min(1U, info->numTransferQueues);

				// We just assign every second queue to either compute/transfer
				// NOTE: We _could_ improve this, but it's probably not worth it.
				for (uint32_t i = 1; i < num; i++)
				{
					vtek::Queue queue{};
					vkGetDeviceQueue(handle, familyIndex, i, &queue.vulkanHandle);
					queue.familyIndex = familyIndex;
					queue.queueFlags = description.queueFlags;
					queue.presentSupport = false;

					if ((i % 2 == 0) && (numTransferQueues > 0))
					{
						device->transferQueues.emplace_back(queue);
						numTransferQueues--;
					}
					else
					{
						device->computeQueues.emplace_back(queue);
						numComputeQueues--;
					}
				}
			}
		}
		else if (selections->transferPlacement == TransferPlacement::graphics_family)
		{
			if (maxNum == 1)
			{
				device->transferQueues.emplace_back(*queue);
			}
			else
			{
				for (uint32_t i = 1; i < description.queueCount; i++)
				{
					vtek::Queue tQueue{};
					vkGetDeviceQueue(handle, familyIndex, i, &tQueue.vulkanHandle);
					tQueue.familyIndex = familyIndex;
					tQueue.queueFlags = description.queueFlags;
					tQueue.presentSupport = false;

					device->transferQueues.emplace_back(tQueue);
				}
			}
		}
		else if (selections->computePlacement == ComputePlacement::graphics_family)
		{
			if (maxNum == 1)
			{
				device->computeQueues.emplace_back(*queue);
			}
			else
			{
				for (uint32_t i = 1; i < description.queueCount; i++)
				{
					vtek::Queue tQueue{};
					vkGetDeviceQueue(handle, familyIndex, i, &tQueue.vulkanHandle);
					tQueue.familyIndex = familyIndex;
					tQueue.queueFlags = description.queueFlags;
					tQueue.presentSupport = false;

					device->computeQueues.emplace_back(tQueue);
				}
			}
		}
	} // graphics family

	if (selections->transfer.has_value())
	{
		const QueueDescription& description = selections->transfer.value();

		for (uint32_t i = 0; i < description.queueCount; i++)
		{
			// Error. This should never happen, but better be safe.
			if (i >= description.familyMaxCount)
			{
				vtek_log_error("i >= description.familyMaxCount, cannot create enough separate transfer queues!");
				break;
			}

			vtek::Queue queue{};
			vkGetDeviceQueue(handle, description.familyIndex, i, &queue.vulkanHandle);
			queue.familyIndex = description.familyIndex;
			queue.queueFlags = description.queueFlags;
			queue.presentSupport = false;

			device->transferQueues.emplace_back(queue);
		}
	} // transfer family

	if (selections->compute.has_value())
	{
		const QueueDescription& description = selections->compute.value();
		const uint32_t num = description.queueCount;
		const uint32_t familyIndex = description.familyIndex;

		uint32_t numComputeQueues = std::min(1U, info->numComputeQueues);
		uint32_t numTransferQueues = 0U;
		bool presentQueue = selections->presentPlacement == PresentPlacement::compute_family;

		if (selections->transferPlacement == TransferPlacement::compute_family)
		{
			numTransferQueues = info->numTransferQueues;
		}

		if (num == 1)
		{
			vtek::Queue queue{};
			vkGetDeviceQueue(handle, familyIndex, 0, &queue.vulkanHandle);
			queue.familyIndex = familyIndex;
			queue.queueFlags = description.queueFlags;
			queue.presentSupport = presentQueue;

			device->computeQueues.emplace_back(queue);
			device->transferQueues.emplace_back(queue);

			if (presentQueue) { device->presentQueue = queue; }
		}
		else if (num == 2)
		{
			// 1 queue dedicated for transfers, 1 for compute(+present?). Just a design choice!
			// NOTE: We could alternatively decide {compute/present, transfer},
			//       but that's more application specific.
			vtek::Queue tQueue{};
			vkGetDeviceQueue(handle, familyIndex, 0, &tQueue.vulkanHandle);
			tQueue.familyIndex = familyIndex;
			tQueue.queueFlags = description.queueFlags;
			tQueue.presentSupport = false;

			device->transferQueues.emplace_back(tQueue);

			vtek::Queue cpQueue{};
			vkGetDeviceQueue(handle, familyIndex, 1, &cpQueue.vulkanHandle);
			cpQueue.familyIndex = familyIndex;
			cpQueue.queueFlags = description.queueFlags;
			cpQueue.presentSupport = presentQueue;

			device->computeQueues.emplace_back(cpQueue);
			if (presentQueue) { device->presentQueue = cpQueue; }
		}
		else
		{
			uint32_t i = 0;
			if (presentQueue)
			{
				vtek::Queue pQueue{};
				vkGetDeviceQueue(handle, familyIndex, 0, &pQueue.vulkanHandle);
				pQueue.familyIndex = familyIndex;
				pQueue.queueFlags = description.queueFlags;
				pQueue.presentSupport = true;
				device->presentQueue = pQueue; // TODO: std::move ?
				i++;
			}

			// We just assign every second queue to either graphics/transfer
			// NOTE: We _could_ improve this, but it's probably not worth it.
			for (; i < num; i++)
			{
				vtek::Queue queue{};
				vkGetDeviceQueue(handle, familyIndex, i, &queue.vulkanHandle);
				queue.familyIndex = familyIndex;
				queue.queueFlags = description.queueFlags;
				queue.presentSupport = false;

				if ((i % 2 == 0) && (numTransferQueues > 0))
				{
					device->transferQueues.emplace_back(queue);
					numTransferQueues--;
				}
				else
				{
					device->computeQueues.emplace_back(queue);
					numComputeQueues--;
				}
			}
		}
	} // compute family
}

static void set_extensions_enabled(
	vtek::Device* device, const vtek::PhysicalDevice* physicalDevice)
{
	auto support = vtek::physical_device_get_extension_support(physicalDevice);
	device->enabledExtensions.swapchain = support->swapchain;
	device->enabledExtensions.dynamicRendering = support->dynamicRendering;
}



/* device interface */
vtek::Device* vtek::device_create(
	const vtek::DeviceCreateInfo* info, const vtek::Instance* instance,
	const vtek::PhysicalDevice* physicalDevice)
{
	VkPhysicalDevice physDev = vtek::physical_device_get_handle(physicalDevice);

	// Allocate device
	auto[id, device] = sAllocator.alloc();
	if (device == nullptr)
	{
		vtek_log_error("Failed to allocate (logical) device!");
		return nullptr;
	}
	device->id = id;

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	QueueFamilySelections queueSelections{};
	if (!create_queue_infos(info, physicalDevice, queueCreateInfos, &queueSelections))
	{
		vtek_log_error("Failed to get device queue infos! Device creation cannot proceed.");
		return nullptr;
	}

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = vtek::physical_device_get_required_features(physicalDevice);

	// Device extensions - support queried for during physical device pick.
	// Now we just enable them!
	const std::vector<const char*>& requiredExtensions =
		vtek::physical_device_get_required_extensions(physicalDevice);
	createInfo.enabledExtensionCount = requiredExtensions.size();
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();

	// Fetch supported extensions (different from those _required_)
	auto supportedExtensions = vtek::physical_device_get_extension_support(physicalDevice);

	// Add dynamic rendering
	VkPhysicalDeviceDynamicRenderingFeatures dynRenderInfo{};
	if (supportedExtensions->dynamicRendering)
	{
		dynRenderInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
		dynRenderInfo.pNext = nullptr;
		dynRenderInfo.dynamicRendering = VK_TRUE;

		createInfo.pNext = &dynRenderInfo;
	};

	//
	// NOTE: Dealing with legacy vTek stuff here (So left for future, but out-commented)!
	//
	// REVIEW: This should be done when picking physical device, IF at all!
	// TODO: How best to approach this?
	// if (info->enableMaintenance1Extension)
	// {
	// 	// TODO: If we are using Vulkan 1.1 or later, then this extension is automatically included.
	// 	enabledDeviceExtensions.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
	// }


	// Bindless texture support----------------------------------------
	if (info->enableBindlessTextureSupport)
	{
		vtek_log_error("Bindless texture support untested/not implemented");
		/*VkPhysicalDeviceFeatures2 physicalDeviceFeatures2 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
		  vkGetPhysicalDeviceFeatures2(physicalDevice_, &physicalDeviceFeatures2);
		  VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT, nullptr };
		  createInfo.pNext = &physicalDeviceFeatures2;
		  indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
		  indexingFeatures.runtimeDescriptorArray = VK_TRUE;
		  physicalDeviceFeatures2.pNext = &indexingFeatures;*/
		return nullptr;
	}

	// Validation layers
	if (vtek::instance_get_validation_enabled(instance))
	{
		const std::vector<const char*>& layers = vtek::instance_get_validation_layer_names(instance);
		createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
		createInfo.ppEnabledLayerNames = layers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physDev, &createInfo, nullptr, &device->vulkanHandle) != VK_SUCCESS)
	{
		vtek_log_error("Failed to create logical device!");
		return nullptr;
	}

	// Retrieve device queues
	device->queueAllocator = new vtek::HostAllocator<vtek::Queue>("vtek_device_queues");
	create_device_queues(device, info, &queueSelections);

	// Set extensions as enabled
	set_extensions_enabled(device, physicalDevice);

	// Set features enabled, which is what was _required_ when picking physical device
	device->enabledFeatures = *(vtek::physical_device_get_required_features(physicalDevice));

	// If GLSL shader loading was enabled, build resource limits for shader compilation
	if (vtek::is_glsl_shader_loading_enabled())
	{
		vtek::build_glslang_resource_limits(physicalDevice);
	}

	// TODO: Optional info logging that a (logical) device was created.
	return device;
}



void vtek::device_destroy(Device* device)
{
	if (device == nullptr || device->vulkanHandle == VK_NULL_HANDLE) return;

	vkDestroyDevice(device->vulkanHandle, nullptr);
	device->vulkanHandle = VK_NULL_HANDLE;

	// TODO: Implement proper cleanup routine inside the vtek::queue_destroy function!
	if (device->queueAllocator != nullptr)
	{
		device->graphicsQueue = {};
		device->presentQueue = {};
		device->transferQueues.clear();
		device->computeQueues.clear();

		delete device->queueAllocator;
	}
	device->queueAllocator = nullptr;

	sAllocator.free(device->id);
	device->id = VTEK_INVALID_ID;
}



/* query functions */
VkDevice vtek::device_get_handle(const vtek::Device* device)
{
	return device->vulkanHandle;
}

const vtek::VulkanVersion* vtek::device_get_vulkan_version(const vtek::Device* device)
{
	return &device->vulkanVersion;
}

const vtek::DeviceExtensions* vtek::device_get_enabled_extensions(const vtek::Device* device)
{
	return &device->enabledExtensions;
}

const VkPhysicalDeviceFeatures* vtek::device_get_enabled_features(const vtek::Device* device)
{
	return &device->enabledFeatures;
}

vtek::Queue* vtek::device_get_graphics_queue(vtek::Device* device)
{
	return (device->graphicsQueue.vulkanHandle == VK_NULL_HANDLE) ? nullptr : &device->graphicsQueue;
}

vtek::Queue* vtek::device_get_present_queue(vtek::Device* device)
{
	return (device->presentQueue.vulkanHandle == VK_NULL_HANDLE) ? nullptr : &device->presentQueue;
}

std::vector<vtek::Queue*> vtek::device_get_transfer_queues(vtek::Device* device)
{
	std::vector<vtek::Queue*> res;
	for (vtek::Queue& q : device->transferQueues) { res.emplace_back(&q); }
	return res;
}

std::vector<vtek::Queue*> vtek::device_get_compute_queues(vtek::Device* device)
{
	// TODO: When queues are stored as pointers from a queue allocator, this can be greatly simplified!
	std::vector<vtek::Queue*> res;
	for (vtek::Queue& q : device->computeQueues) { res.emplace_back(&q); }
	return res;
}

bool vtek::device_get_graphics_present_same_family(const vtek::Device* device)
{
	return device->graphicsQueue.familyIndex == device->presentQueue.familyIndex;
}

void vtek::device_wait_idle(vtek::Device* device)
{
	vkDeviceWaitIdle(device->vulkanHandle);
}
