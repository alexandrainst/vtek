// standard
#include <algorithm>
#include <cstring>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

// vtek
#include "impl/vtek_host_allocator.hpp"
#include "vtek_logging.hpp"
#include "vtek_physical_device.hpp"


/* struct implementation */
struct vtek::PhysicalDevice
{
	uint64_t id {VTEK_INVALID_ID};
	VkPhysicalDevice vulkanHandle { VK_NULL_HANDLE };
	VkPhysicalDeviceProperties properties {};

	// TODO: Delete this! It can always be queried for, and we ONLY care about what to enable
	//       later during device creation, which is covered by `requiredFeatures`.
	VkPhysicalDeviceFeatures features {}; // TODO: We probably don't want this but instead `featureSupport`!

	VkPhysicalDeviceMemoryProperties memoryProperties {};

	// NOTE: Features required to be supported, used during device creation
	VkPhysicalDeviceFeatures requiredFeatures {};

	// NOTE: Extensions required to be supported, used during device creation
	std::vector<const char*> requiredExtensions {};

	vtek::PhysicalDeviceQueueSupport queueSupport {};
	vtek::PhysicalDeviceExtensionSupport extensionSupport {};
	std::vector<std::string> supportedExtensions {}; // TODO: Can we replace this with `requiredExtensions`?
};


/* host allocator */
static vtek::HostAllocator<vtek::PhysicalDevice> sAllocator("vtek_physical_device");


/* helper functions */
static void get_queue_family_support(vtek::PhysicalDevice* physicalDevice, VkSurfaceKHR surface)
{
	VkPhysicalDevice device = physicalDevice->vulkanHandle;
	vtek::PhysicalDeviceQueueSupport* queueSupport = &(physicalDevice->queueSupport);

	// reset support status
	*queueSupport = {};

	// get queue family properties
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
	std::vector<VkQueueFamilyProperties> families(count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

	// status
	bool graphicsFound = false;
	bool transferFound = false;
	bool computeFound = false;

	for (uint32_t i = 0; i < families.size(); i++)
	{
		// queue family with graphics support
		// NOTE: transfer support is implicitly guaranteed by the specification
		if (!graphicsFound && (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			queueSupport->graphics = true;
			queueSupport->graphicsIndex = i;
			queueSupport->graphicsMaxCount = families[i].queueCount;

			// Check if the graphics queue family supports timestamp queries.
			// This is strictly needed to be able to measure rendering time
			// during command buffer recording.
			// The valid range for the count is 36..64 bits, or a value of 0,
			// indicating no support for timestamps.
			uint32_t timestampValidBits = families[i].timestampValidBits;
			queueSupport->graphicsTimestampQuery = (timestampValidBits > 0);

			if (families[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
			{
				queueSupport->graphicsHasSparseBinding = true;
			}

			// We prefer to have graphics and presentation in the same queue, so
			// even if a separate presentation queue has been found already, we
			// overwrite it here.
			if ((surface != VK_NULL_HANDLE))
			{
				VkBool32 present = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present);
				if (present)
				{
					queueSupport->present = true;
					queueSupport->graphicsHasPresent = true;
				}
			}

			// compute support in the graphics queue
			if (families[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				queueSupport->compute = true;
				queueSupport->graphicsHasCompute = true;
			}

			graphicsFound = true;
		}

		// separate transfer queue (some GPUs have this)
		// If transfer support was already found in either graphics or compute queue,
		// we overwrite it here since separate transfer queue means more throughput.
		if (!transferFound && (families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) &&
		    !(families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
		    !(families[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
		{
			queueSupport->hasSeparateTransferFamily = true;
			queueSupport->separateTransferIndex = i;
			queueSupport->separateTransferMaxCount = families[i].queueCount;

			uint32_t timestampValidBits = families[i].timestampValidBits;
			queueSupport->separateTransferTimestampQuery = (timestampValidBits > 0);

			if (families[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
			{
				queueSupport->separateTransferHasSparseBinding = true;
			}

			transferFound = true;
		}

		// separate compute queue
		if (!computeFound && (families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) &&
		    !(families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
		{
			queueSupport->compute = true;
			queueSupport->hasSeparateComputeFamily = true;
			queueSupport->separateComputeIndex = i;

			uint32_t timestampValidBits = families[i].timestampValidBits;
			queueSupport->separateComputeTimestampQuery = (timestampValidBits > 0);

			if (families[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
			{
				queueSupport->separateComputeHasSparseBinding = true;
			}

			VkBool32 present = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present);
			if (present)
			{
				queueSupport->separateComputeHasPresent = true;
			}

			computeFound = true;
		}
	} // for-loop
}



static bool has_required_properties()
{
	// TODO: We could query for stuff here..
	return true;
}

static bool has_required_extension_support(
	const vtek::PhysicalDeviceInfo* info, vtek::PhysicalDevice* physicalDevice)
{
	VkPhysicalDevice device = physicalDevice->vulkanHandle;
	vtek::PhysicalDeviceExtensionSupport* support = &physicalDevice->extensionSupport;

	// All required extensions must, if they are supported, be added to this vector!
	// This vector is used during (logical) device creation.
	std::vector<const char*>& requiredExtRef = physicalDevice->requiredExtensions;
	requiredExtRef.clear();

	// Enumerate extensions supported by the physical device
	uint32_t extCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, nullptr);
	std::vector<VkExtensionProperties> properties(extCount);
	vkEnumerateDeviceExtensionProperties(
		device, nullptr, &extCount, properties.data());
	physicalDevice->supportedExtensions.clear();
	physicalDevice->supportedExtensions.reserve(extCount);
	for (auto prop : properties)
	{
		physicalDevice->supportedExtensions.emplace_back(prop.extensionName);
	}

	// Lambda magic... {^_^}
	auto my_strcmp = [](auto& str1, auto& str2) { return std::strcmp(str1, str2) == 0; };
	auto my_find_if = [p=properties, my_strcmp](const char* ext){
		return std::find_if(p.begin(), p.end(), [ext, my_strcmp](auto& prop){
			return my_strcmp(ext, prop.extensionName); }) != p.end();
	};

	// check for swapchain support
	if (info->requirePresentQueue && info->requireSwapchainSupport)
	{
		bool swapchain = my_find_if(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		support->swapchain = swapchain;
		if (!swapchain)
		{
			vtek_log_error("Swapchain extension not supported!");
			return false;
		}
		requiredExtRef.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	// ray-tracing
	if (info->requireRaytracingSupport)
	{
		// NOTE: For NVidia GPUs we can check for the NV raytracing extension
		// NOTE: Is is best to just pick the KHR extension, if available?
		// NOTE: The KHR extension is likely more widely support, so check for that first.
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_ray_tracing_pipeline.html
		bool hasPipelineExt = my_find_if(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
		if (!hasPipelineExt)
		{
			vtek_log_error("Ray tracing pipeline extension not supported!");
			return false;
		}
		requiredExtRef.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);

		// NOTE: SpirV 1.4 extension is needed for compiling ray-tracing shaders
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VK_KHR_spirv_1_4.html
		bool hasSpirv14Ext = my_find_if(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
		if (!hasSpirv14Ext)
		{
			vtek_log_error("Spir-V 1.4 extension not supported!");
			return false;
		}
		requiredExtRef.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);

		bool hasAccelExt = my_find_if(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		if (!hasAccelExt)
		{
			vtek_log_error("Acceleration structure extension not supported!");
			return false;
		}
		requiredExtRef.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);

		bool hasRayQueryExt = my_find_if(VK_KHR_RAY_QUERY_EXTENSION_NAME);
		if (!hasRayQueryExt)
		{
			vtek_log_error("Ray query extension not supported!");
			return false;
		}
		requiredExtRef.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);

		// TODO: vTek 1.0 code base had a check for this particular extension (do we need it?):
		// - VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME
		// NOTE: This is also dependent upon available Vulkan instance version, so likely not needed!

		// TODO: Check for dynamic rendering extension of Vulkan version < 1.3!

		support->raytracing = true;
	}

	// NEXT: More extension checks may be added here..

	return true;
}

static bool has_required_features(
	const vtek::PhysicalDeviceInfo* info, vtek::PhysicalDevice* physicalDevice)
{
	VkPhysicalDevice device = physicalDevice->vulkanHandle;
	const VkPhysicalDeviceFeatures* required = &info->requiredFeatures;
	VkPhysicalDeviceFeatures supported{};
	vkGetPhysicalDeviceFeatures(device, &supported);

	bool support = true;

	if (required->robustBufferAccess                             && !supported.robustBufferAccess) {
		vtek_log_error("robustBufferAccess feature required but not supported"); support = false;
	}
	if (required->fullDrawIndexUint32                            && !supported.fullDrawIndexUint32) {
		vtek_log_error("fullDrawIndexUint32 feature required but not supported"); support = false;
	}
	if (required->imageCubeArray                                 && !supported.imageCubeArray) {
		vtek_log_error("imageCubeArray feature required but not supported"); support = false;
	}
	if (required->independentBlend                               && !supported.independentBlend) {
		vtek_log_error("independentBlend feature required but not supported"); support = false;
	}
	if (required->geometryShader                                 && !supported.geometryShader) {
		vtek_log_error("geometryShader feature required but not supported"); support = false;
	}
	if (required->tessellationShader                             && !supported.tessellationShader) {
		vtek_log_error("tessellationShader feature required but not supported"); support = false;
	}
	if (required->sampleRateShading                              && !supported.sampleRateShading) {
		vtek_log_error("sampleRateShading feature required but not supported"); support = false;
	}
	if (required->dualSrcBlend                                   && !supported.dualSrcBlend) {
		vtek_log_error("dualSrcBlend feature required but not supported"); support = false;
	}
	if (required->logicOp                                        && !supported.logicOp) {
		vtek_log_error("logicOp feature required but not supported"); support = false;
	}
	if (required->multiDrawIndirect                              && !supported.multiDrawIndirect) {
		vtek_log_error("multiDrawIndirect feature required but not supported"); support = false;
	}
	if (required->drawIndirectFirstInstance                      && !supported.drawIndirectFirstInstance) {
		vtek_log_error("drawIndirectFirstInstance feature required but not supported"); support = false;
	}
	if (required->depthClamp                                     && !supported.depthClamp) {
		vtek_log_error("depthClamp feature required but not supported"); support = false;
	}
	if (required->depthBiasClamp                                 && !supported.depthBiasClamp) {
		vtek_log_error("depthBiasClamp feature required but not supported"); support = false;
	}
	if (required->fillModeNonSolid                               && !supported.fillModeNonSolid) {
		vtek_log_error("fillModeNonSolid feature required but not supported"); support = false;
	}
	if (required->depthBounds                                    && !supported.depthBounds) {
		vtek_log_error("depthBounds feature required but not supported"); support = false;
	}
	if (required->wideLines                                      && !supported.wideLines) {
		vtek_log_error("wideLines feature required but not supported"); support = false;
	}
	if (required->largePoints                                    && !supported.largePoints) {
		vtek_log_error("largePoints feature required but not supported"); support = false;
	}
	if (required->alphaToOne                                     && !supported.alphaToOne) {
		vtek_log_error("alphaToOne feature required but not supported"); support = false;
	}
	if (required->multiViewport                                  && !supported.multiViewport) {
		vtek_log_error("multiViewport feature required but not supported"); support = false;
	}
	if (required->samplerAnisotropy                              && !supported.samplerAnisotropy) {
		vtek_log_error("samplerAnisotropy feature required but not supported"); support = false;
	}
	if (required->textureCompressionETC2                         && !supported.textureCompressionETC2) {
		vtek_log_error("textureCompressionETC2 feature required but not supported"); support = false;
	}
	if (required->textureCompressionASTC_LDR                     && !supported.textureCompressionASTC_LDR) {
		vtek_log_error("textureCompressionASTC_LDR feature required but not supported"); support = false;
	}
	if (required->textureCompressionBC                           && !supported.textureCompressionBC) {
		vtek_log_error("textureCompressionBC feature required but not supported"); support = false;
	}
	if (required->occlusionQueryPrecise                          && !supported.occlusionQueryPrecise) {
		vtek_log_error("occlusionQueryPrecise feature required but not supported"); support = false;
	}
	if (required->pipelineStatisticsQuery                        && !supported.pipelineStatisticsQuery) {
		vtek_log_error("pipelineStatisticsQuery feature required but not supported"); support = false;
	}
	if (required->vertexPipelineStoresAndAtomics                 && !supported.vertexPipelineStoresAndAtomics) {
		vtek_log_error("vertexPipelineStoresAndAtomics feature required but not supported"); support = false;
	}
	if (required->fragmentStoresAndAtomics                       && !supported.fragmentStoresAndAtomics) {
		vtek_log_error("fragmentStoresAndAtomics feature required but not supported"); support = false;
	}
	if (required->shaderTessellationAndGeometryPointSize         && !supported.shaderTessellationAndGeometryPointSize) {
		vtek_log_error("shaderTessellationAndGeometryPointSize feature required but not supported"); support = false;
	}
	if (required->shaderImageGatherExtended                      && !supported.shaderImageGatherExtended) {
		vtek_log_error("shaderImageGatherExtended feature required but not supported"); support = false;
	}
	if (required->shaderStorageImageExtendedFormats              && !supported.shaderStorageImageExtendedFormats) {
		vtek_log_error("shaderStorageImageExtendedFormats feature required but not supported"); support = false;
	}
	if (required->shaderStorageImageMultisample                  && !supported.shaderStorageImageMultisample) {
		vtek_log_error("shaderStorageImageMultisample feature required but not supported"); support = false;
	}
	if (required->shaderStorageImageReadWithoutFormat            && !supported.shaderStorageImageReadWithoutFormat) {
		vtek_log_error("shaderStorageImageReadWithoutFormat feature required but not supported"); support = false;
	}
	if (required->shaderStorageImageWriteWithoutFormat           && !supported.shaderStorageImageWriteWithoutFormat) {
		vtek_log_error("shaderStorageImageWriteWithoutFormat feature required but not supported"); support = false;
	}
	if (required->shaderUniformBufferArrayDynamicIndexing        && !supported.shaderUniformBufferArrayDynamicIndexing) {
		vtek_log_error("shaderUniformBufferArrayDynamicIndexing feature required but not supported"); support = false;
	}
	if (required->shaderSampledImageArrayDynamicIndexing         && !supported.shaderSampledImageArrayDynamicIndexing) {
		vtek_log_error("shaderSampledImageArrayDynamicIndexing feature required but not supported"); support = false;
	}
	if (required->shaderStorageBufferArrayDynamicIndexing        && !supported.shaderStorageBufferArrayDynamicIndexing) {
		vtek_log_error("shaderStorageBufferArrayDynamicIndexing feature required but not supported"); support = false;
	}
	if (required->shaderStorageImageArrayDynamicIndexing         && !supported.shaderStorageImageArrayDynamicIndexing) {
		vtek_log_error("shaderStorageImageArrayDynamicIndexing feature required but not supported"); support = false;
	}
	if (required->shaderClipDistance                             && !supported.shaderClipDistance) {
		vtek_log_error("shaderClipDistance feature required but not supported"); support = false;
	}
	if (required->shaderCullDistance                             && !supported.shaderCullDistance) {
		vtek_log_error("shaderCullDistance feature required but not supported"); support = false;
	}
	if (required->shaderFloat64                                  && !supported.shaderFloat64) {
		vtek_log_error("shaderFloat64 feature required but not supported"); support = false;
	}
	if (required->shaderInt64                                    && !supported.shaderInt64) {
		vtek_log_error("shaderInt64 feature required but not supported"); support = false;
	}
	if (required->shaderInt16                                    && !supported.shaderInt16) {
		vtek_log_error("shaderInt16 feature required but not supported"); support = false;
	}
	if (required->shaderResourceResidency                        && !supported.shaderResourceResidency) {
		vtek_log_error("shaderResourceResidency feature required but not supported"); support = false;
	}
	if (required->shaderResourceMinLod                           && !supported.shaderResourceMinLod) {
		vtek_log_error("shaderResourceMinLod feature required but not supported"); support = false;
	}
	if (required->sparseBinding                                  && !supported.sparseBinding) {
		vtek_log_error("sparseBinding feature required but not supported"); support = false;
	}
	if (required->sparseResidencyBuffer                          && !supported.sparseResidencyBuffer) {
		vtek_log_error("sparseResidencyBuffer feature required but not supported"); support = false;
	}
	if (required->sparseResidencyImage2D                         && !supported.sparseResidencyImage2D) {
		vtek_log_error("sparseResidencyImage2D feature required but not supported"); support = false;
	}
	if (required->sparseResidencyImage3D                         && !supported.sparseResidencyImage3D) {
		vtek_log_error("sparseResidencyImage3D feature required but not supported"); support = false;
	}
	if (required->sparseResidency2Samples                        && !supported.sparseResidency2Samples) {
		vtek_log_error("sparseResidency2Samples feature required but not supported"); support = false;
	}
	if (required->sparseResidency4Samples                        && !supported.sparseResidency4Samples) {
		vtek_log_error("sparseResidency4Samples feature required but not supported"); support = false;
	}
	if (required->sparseResidency8Samples                        && !supported.sparseResidency8Samples) {
		vtek_log_error("sparseResidency8Samples feature required but not supported"); support = false;
	}
	if (required->sparseResidency16Samples                       && !supported.sparseResidency16Samples) {
		vtek_log_error("sparseResidency16Samples feature required but not supported"); support = false;
	}
	if (required->sparseResidencyAliased                         && !supported.sparseResidencyAliased) {
		vtek_log_error("sparseResidencyAliased feature required but not supported"); support = false;
	}
	if (required->variableMultisampleRate                        && !supported.variableMultisampleRate) {
		vtek_log_error("variableMultisampleRate feature required but not supported"); support = false;
	}
	if (required->inheritedQueries                               && !supported.inheritedQueries) {
		vtek_log_error("inheritedQueries feature required but not supported"); support = false;
	}

	return support;
}



static bool check_device_suitability(
	const vtek::PhysicalDeviceInfo* info, vtek::PhysicalDevice* device, VkSurfaceKHR surface)
{
	// queue support
	get_queue_family_support(device, surface);
	bool queueFamilySupport = true;

	vtek::PhysicalDeviceQueueSupport* queueSupport = &(device->queueSupport);
	if (info->requireGraphicsQueue && !queueSupport->graphics)
	{
		vtek_log_error("Graphics queue required but not supported");
		queueFamilySupport = false;
	}
	if (info->requirePresentQueue && !queueSupport->present)
	{
		vtek_log_error("Presentation queue required but not supported");
		queueFamilySupport = false;
	}
	if (info->requireComputeQueue && !queueSupport->compute)
	{
		vtek_log_error("Compute queue required but not supported");
		queueFamilySupport = false;
	}

	// setting fields
	queueSupport->graphicsRequired = info->requireGraphicsQueue;
	queueSupport->presentRequired = info->requirePresentQueue;
	queueSupport->computeRequired = info->requireComputeQueue;

	const VkPhysicalDeviceFeatures* features = &(info->requiredFeatures);
	bool sparseBindingRequired
		= features->sparseBinding
		| features->sparseResidencyImage2D
		| features->sparseResidencyImage3D
		| features->sparseResidencyBuffer
		| features->shaderResourceResidency;
	bool sparseBindingQueueSupport
		= queueSupport->graphicsHasSparseBinding
		| queueSupport->separateTransferHasSparseBinding
		| queueSupport->separateComputeHasSparseBinding;

	if (sparseBindingRequired && !sparseBindingQueueSupport)
	{
		// TODO: sparse binding required but not supported
		queueFamilySupport = false;
	}

	// extension support
	bool extensionSupport = has_required_extension_support(info, device);

	// features support
	bool featureSupport = has_required_features(info, device);

	// properties support
	bool propertiesSupport = has_required_properties();

	// NEXT: Could also check memory properties
	return queueFamilySupport && extensionSupport && featureSupport && propertiesSupport;
}




// helper struct for picking a physical device
struct PhysicalDeviceChoice
{
	VkPhysicalDevice handle {VK_NULL_HANDLE};
	VkPhysicalDeviceProperties properties {};
};

vtek::PhysicalDevice* vtek::physical_device_pick(
	const PhysicalDeviceInfo* info, const Instance* instance)
{
	return vtek::physical_device_pick(info, instance, VK_NULL_HANDLE); // no presentation support desired
}

vtek::PhysicalDevice* vtek::physical_device_pick(
	const PhysicalDeviceInfo* info, const vtek::Instance* instance, VkSurfaceKHR surface)
{
	vtek_log_trace("physical_device_pick()");
	VkInstance inst = vtek::instance_get_handle(instance);

	// Enumerate physical devices
	uint32_t deviceCount = 0u;
	vkEnumeratePhysicalDevices(inst, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		vtek_log_error("Failed to find GPUs with Vulkan support!");
		return nullptr;
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(inst, &deviceCount, devices.data());

	// Heuristic device sort, ie. discrete devices first.
	std::vector<std::pair<unsigned char, PhysicalDeviceChoice>> weightedDevices;
	for (const auto& device : devices)
	{
		unsigned char weight = 255;
		PhysicalDeviceChoice choice{};
		choice.handle = device;
		vkGetPhysicalDeviceProperties(device, &choice.properties);

		switch (choice.properties.deviceType)
		{
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			weight = 1;
			break;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			weight = 0;
			break;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			weight = 4;
			break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			weight = 3;
			break;

		case VK_PHYSICAL_DEVICE_TYPE_OTHER:
		default:
			weight = 254;
			break;
		}

		weightedDevices.emplace_back(std::pair<unsigned char, PhysicalDeviceChoice>(weight, std::move(choice)));
	}

	std::sort(
		weightedDevices.begin(), weightedDevices.end(), [](auto& p1, auto& p2) { return p1.first < p2.first; });

	// Allocate
	auto [id, physicalDevice] = sAllocator.alloc();
	if (physicalDevice == nullptr)
	{
		vtek_log_fatal("Failed to allocate physical device!");
		return nullptr;
	}
	physicalDevice->id = id;

	// Run through them in sorted order and check if the physical device supports:
	// extensions, features, property coverage, and if so - pick it.
	for (const auto& weightedDevice : weightedDevices)
	{
		PhysicalDeviceChoice& choice = const_cast<PhysicalDeviceChoice&>(weightedDevice.second);
		physicalDevice->vulkanHandle = choice.handle;
		physicalDevice->properties = choice.properties;
		physicalDevice->requiredFeatures = info->requiredFeatures;

		// NOTE: For Vulkan >= 1.2 we can check for more, including driver properties
		// TODO: Figure out what to do with this information
#if defined(VK_API_VERSION_1_2)
		// Struct provided by Vulkan 1.2
		VkPhysicalDeviceDriverProperties driverProperties {};
		driverProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
		driverProperties.pNext = nullptr;

		// Struct provided by Vulkan 1.1
		VkPhysicalDeviceProperties2 properties2 {};
		properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		properties2.pNext = &driverProperties;

		// Provided by VK_VERSION_1_1
		// Provided by VK_KHR_get_physical_device_properties2
		vkGetPhysicalDeviceProperties2(physicalDevice->vulkanHandle, &properties2);
#endif

		// Get supported Vulkan features
		vkGetPhysicalDeviceFeatures(physicalDevice->vulkanHandle, &physicalDevice->features);

		bool isSuitable = check_device_suitability(info, physicalDevice, surface);
		if (isSuitable)
		{
			// NOTE: If driverName == "NVIDIA" we can probably use NV extensions
			vtek_log_info("Picked physical device[{}]: {}",
			              driverProperties.driverName, choice.properties.deviceName);
			// Now that we found a suitable device, query the rest of the necessary data
			// Get Vulkan features, properties, and memory properties
			// TODO: Do we need memory properties?
			// TODO: We could probably do this before checking suitability
			vkGetPhysicalDeviceMemoryProperties(physicalDevice->vulkanHandle, &physicalDevice->memoryProperties);

			return physicalDevice;
		}
	}

	// FAIL: We didn't manage to find a suitable physical device!
	vtek_log_error("Failed to find a suitable GPU!");
	return nullptr;
}

void vtek::physical_device_release(vtek::PhysicalDevice* physicalDevice)
{
	if (physicalDevice == nullptr) return;

	physicalDevice->vulkanHandle = VK_NULL_HANDLE;
	physicalDevice->supportedExtensions.clear();
	physicalDevice->requiredExtensions.clear();

	sAllocator.free(physicalDevice->id);
	physicalDevice->id = VTEK_INVALID_ID;
}



/* query functions */
VkPhysicalDevice vtek::physical_device_get_handle(const vtek::PhysicalDevice* physicalDevice)
{
	return physicalDevice->vulkanHandle;
}

const VkPhysicalDeviceProperties* vtek::physical_device_get_properties(
	const vtek::PhysicalDevice* physicalDevice)
{
	return &physicalDevice->properties;
}

const VkPhysicalDeviceMemoryProperties* vtek::physical_device_get_memory_properties(
	const vtek::PhysicalDevice* physicalDevice)
{
	return &physicalDevice->memoryProperties;
}

const VkPhysicalDeviceFeatures* vtek::physical_device_get_required_features(
	const vtek::PhysicalDevice* physicalDevice)
{
	return &physicalDevice->requiredFeatures;
}

const std::vector<const char*>& vtek::physical_device_get_required_extensions(
	const vtek::PhysicalDevice* physicalDevice)
{
	return physicalDevice->requiredExtensions;
}

const vtek::PhysicalDeviceQueueSupport* vtek::physical_device_get_queue_support(
	const vtek::PhysicalDevice* physicalDevice)
{
	return &physicalDevice->queueSupport;
}

const vtek::PhysicalDeviceExtensionSupport* vtek::physical_device_get_extension_support(
	const vtek::PhysicalDevice* physicalDevice)
{
	return &physicalDevice->extensionSupport;
}
