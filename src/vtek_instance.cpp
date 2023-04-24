// standard
#include <algorithm>
#include <cstring>
#include <vulkan/vk_enum_string_helper.h> // string_VkObjectType(VkObjectType input_value)

// vtek
#include "impl/vtek_glfw_backend.hpp"
#include "impl/vtek_host_allocator.hpp"
#include "version.hpp"
#include "vtek_instance.hpp"
#include "vtek_logging.hpp"
#include "vtek_vulkan_version.hpp"


/* Validation layers, more can be added if desired */
static const std::vector<const char*> sValidationLayers = {
	"VK_LAYER_KHRONOS_validation"
};


/* struct implementation */
struct vtek::Instance
{
	uint64_t id {VTEK_INVALID_ID};
	VkInstance vulkanHandle {VK_NULL_HANDLE};
	vtek::VulkanVersion vulkanVersion {0};

	VkDebugUtilsMessengerEXT debugMessenger {VK_NULL_HANDLE};
	bool validationLayersEnabled {false};
	bool rayTracingExtensionEnabled {false};
};


/* host allocator */
static vtek::HostAllocator<vtek::Instance> sAllocator("instance");


/* helper functions */
static bool checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	bool allLayersFound = true;
	for (const char* layer : sValidationLayers)
	{
		bool found = false;
		for (const auto& properties : availableLayers)
		{
			if (std::strcmp(layer, properties.layerName) == 0)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			vtek_log_error("Cannot find validation layer: {}", layer);
			allLayersFound = false;
		}
	}
	return allLayersFound;
}

static bool checkInstanceExtensionSupport(const std::vector<const char*>& extensions)
{
	bool allSupported = true;

	uint32_t count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

	std::vector<VkExtensionProperties> properties(count);
	vkEnumerateInstanceExtensionProperties(nullptr, &count, properties.data());

	for (const char* ext : extensions)
	{
		auto it = std::find_if(
			properties.begin(), properties.end(),
			[ext](auto& prop) {
				return std::strcmp(ext, prop.extensionName) == 0;
			});

		if (it == properties.end())
		{
			vtek_log_error("Required instance extension is not available: {}", ext);
			allSupported = false;
		}
	}

	return allSupported;
}

static uint32_t get_vulkan_instance_version()
{
	// TODO: Check documentation for VkApplicationInfo - it answers how to handle instance version.
	auto vkEnumerateInstanceVersion =
		reinterpret_cast<PFN_vkEnumerateInstanceVersion>(vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion"));
	if (vkEnumerateInstanceVersion == nullptr)
	{
		// NOTE: This is a Vulkan 1.0 implementation!
		// TODO: As long as the instance supports at least Vulkan 1.1, an application can use different
		//       versions of Vulkan with an instance than it does with a device or physical device.
		return VK_API_VERSION_1_0;
	}

	uint32_t apiVersion = 0;
	VkResult result = vkEnumerateInstanceVersion(&apiVersion);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to enumerate instance version! Cannot create an instance.");
		return VK_API_VERSION_1_0;
	}

	uint32_t major = VK_API_VERSION_MAJOR(apiVersion);
	uint32_t minor = VK_API_VERSION_MINOR(apiVersion);
	uint32_t patch = VK_API_VERSION_PATCH(apiVersion);

	// NOTE: The packed `apiVersion` also contains a variant, which will always be 0 except for
	// non-standard Vulkan implementations (Perhaps MoltenVk included?).
	vtek_log_info("Supported Vulkan Instance version: {}.{}.{} ({})",
	              major, minor, patch, apiVersion);

#if defined(VK_API_VERSION_1_3)
	if (major >= 1 && minor >= 3) { return VK_API_VERSION_1_3; }
#endif

#if defined(VK_API_VERSION_1_2)
	if (major >= 1 && minor >= 2) { return VK_API_VERSION_1_2; }
#endif

#if defined(VK_API_VERSION_1_1)
	if (major >= 1 && minor >= 2) { return VK_API_VERSION_1_1; }
#endif

	return VK_API_VERSION_1_0;
}

static uint32_t get_vtek_vulkan_version()
{
	// These values are defined in the local header `version.h`, which is
	// also the appropriate place to update them when/as needed.
	vtek::VulkanVersion v(
		VTEK_VERSION_MAJOR,
		VTEK_VERSION_MINOR,
		VTEK_VERSION_PATCH);

	return v.apiVersion();
}

static VkResult createVulkanDebugMessenger(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	// Since the function which returns a `VkDebugUtilsMessengerEXT` object
	// is an extension function, and thus not automatically loaded, we look
	// up its address here.
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkCreateDebugUtilsMessengerEXT");

	if (func == nullptr) { return VK_ERROR_EXTENSION_NOT_PRESENT; }

	// If debug function was found, call it.
	VkResult result = func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	return result;
}

static void destroyVulkanDebugMessenger(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != nullptr) { func(instance, debugMessenger, pAllocator); }
}

static VKAPI_ATTR VkBool32 VKAPI_CALL vulkanDebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT severity,
	VkDebugUtilsMessageTypeFlagsEXT /*type*/,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* /*pUserData*/)
{
	// `pCallbackData` refers to a struct containing message details,
	// and has several members, the most important being:
	// - pMessage: The debug message as a C-string
	// - pObjects: Array of Vulkan object handles related to the message
	// - objectCount: Number of objects in array

	// `pUserData` parameter contains a pointer that was specified during
	// setup of the callback, and may point to custom user-defined data.

	// In Vulkan, these flags are set up such as to enable comparison operators.
	switch (severity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
		vtek_log_debug("Vulkan[verbose]: {}", pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
		vtek_log_debug("Vulkan[info]: {}", pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		vtek_log_warn("Vulkan[warning]: {}", pCallbackData->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		vtek_log_error("Vulkan[error]: {}", pCallbackData->pMessage);
		break;
	default:
		vtek_log_error("Vulkan[unrecognized]: {}", pCallbackData->pMessage);
		break;
	}

	if (pCallbackData->pObjects != VK_NULL_HANDLE)
	{
		// handle: pCallbackData->pObjects->objectHandle
		// type: pCallbackData->pObjects->objectType
		// REVIEW: We could print something extra, although this information is not very useful,
		// except for looking up for a proper identifier in a hash table.
	}

	return VK_FALSE;
}




/* instance functions */
vtek::Instance* vtek::instance_create(vtek::InstanceCreateInfo* info)
{
	if (info->enableValidationLayers && !checkValidationLayerSupport())
	{
		vtek_log_error("Unsupported validation layer(s)");
		return nullptr;
	}

	auto [id, instance] = sAllocator.alloc();
	if (instance == nullptr)
	{
		vtek_log_fatal("Failed to allocate instance!");
		return nullptr;
	}
	instance->id = id;

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = info->applicationName;
	appInfo.applicationVersion = info->applicationVersion.apiVersion();
	// NOTE: Here we extract version from vtek itself
	appInfo.pEngineName = "vtek";
	appInfo.engineVersion = get_vtek_vulkan_version();

	uint32_t supportedInstanceVersion = get_vulkan_instance_version();
	instance->vulkanVersion = vtek::VulkanVersion(supportedInstanceVersion);
	appInfo.apiVersion = supportedInstanceVersion;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Fill out the extension list with extensions provided by GLFW.
	// If GLFW is not used (ie. has not been queried for during vtek initialization),
	// then this function will do nothing.
	vtek::glfw_backend_get_required_instance_extensions(info->requiredExtensions);

	if (info->enableValidationLayers)
	{
		info->requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	if (info->enableRayTracingExtension)
	{
		// TODO: Why is this extension needed for ray tracing?
		info->requiredExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

		instance->rayTracingExtensionEnabled = true;
	}
	createInfo.enabledExtensionCount = static_cast<uint32_t>(info->requiredExtensions.size());
	createInfo.ppEnabledExtensionNames = info->requiredExtensions.data();

	if (info->enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(sValidationLayers.size());
		createInfo.ppEnabledLayerNames = sValidationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	// Check for instance extension support
	if (!checkInstanceExtensionSupport(info->requiredExtensions))
	{
		vtek_log_error("Not all required instance extensions are available!");
		return nullptr;
	}

	// add debug messenger to creation struct
	// Best-practices validation layer. Included in
	// `VK_LAYER_KHRONOS_VALIDATION` but must be explicitly enabled at
	// CreateInstance time.
	// VkValidationFeatureEnableEXT validationEnables[] = {
	// 	VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT
	// };
	// VkValidationFeaturesEXT validationFeatures = {};
	// validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
	// validationFeatures.enabledValidationFeatureCount = 1;
	// validationFeatures.pEnabledValidationFeatures = validationEnables;
	// Add a debug messenger so we can debug create/destroy instance.
	// We place the `debugCreateInfo` struct outside the if-statement so it
	// is not automatically destroyed before the call to `vkCreateInstance`.
	// By attaching it to `createInfo.pNext` it will be automatically used
	// during `vkCreateInstance` and `vkDestroyInstance`, and cleaned up
	// after that.
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (info->enableValidationLayers)
	{
		debugCreateInfo = {};
		debugCreateInfo.sType =
			VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

		debugCreateInfo.messageSeverity = 0;
		if (info->validationSettings.debugSeverityVerbose) {
			debugCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
		}
		if (info->validationSettings.debugSeverityInfo) {
			debugCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
		}
		if (info->validationSettings.debugSeverityWarning) {
			debugCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		}
		if (info->validationSettings.debugSeverityError) {
			debugCreateInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		}

		debugCreateInfo.messageType = 0;
		if (info->validationSettings.debugTypeGeneral) {
			debugCreateInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
		}
		if (info->validationSettings.debugTypeValidation) {
			debugCreateInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		}
		if (info->validationSettings.debugTypePerformance) {
			debugCreateInfo.messageType |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		}

		debugCreateInfo.pfnUserCallback = vulkanDebugCallback;
		// TODO: Validation layer error, apparently, when pNext != NULL
		//debugCreateInfo.pNext = &validationFeatures;
		debugCreateInfo.pNext = nullptr;
		createInfo.pNext = &debugCreateInfo;
	}
	else
	{
		createInfo.pNext = nullptr;
	}

	// Create instance
	// NOTE: SHOULD determine available Vulkan version before calling this, as it otherwise
	// may return VK_ERROR_INCOMPATIBLE_DRIVER.
	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance->vulkanHandle);
	if (result != VK_SUCCESS)
	{
		vtek_log_error("Failed to create Vulkan instance!");
		return nullptr;
	}

	// Setup debug messenger
	if (info->enableValidationLayers)
	{
		debugCreateInfo.pUserData = nullptr; // optional

		VkResult debugResult = createVulkanDebugMessenger(
			instance->vulkanHandle, &debugCreateInfo, nullptr, &instance->debugMessenger);
		if (debugResult != VK_SUCCESS)
		{
			vtek_log_error("Failed to setup Vulkan debug messenger!");
			vtek_log_warn("Validation layers will not be used, Vulkan might still work.");
		}
	}

	// Return success
	instance->validationLayersEnabled = info->enableValidationLayers;
	return instance;
}


void vtek::instance_destroy(vtek::Instance* instance)
{
	if (instance == nullptr) { return; }

	if (instance->vulkanHandle != VK_NULL_HANDLE)
	{
		if (instance->validationLayersEnabled && instance->debugMessenger != VK_NULL_HANDLE)
		{
			destroyVulkanDebugMessenger(
				instance->vulkanHandle, instance->debugMessenger, nullptr);
		}

		vkDestroyInstance(instance->vulkanHandle, nullptr);
		instance->debugMessenger = VK_NULL_HANDLE;
		instance->vulkanHandle = VK_NULL_HANDLE;
	}

	sAllocator.free(instance->id);
	instance->id = VTEK_INVALID_ID;
}



/* query functions */
VkInstance vtek::instance_get_handle(const vtek::Instance* instance)
{
	return instance->vulkanHandle;
}

bool vtek::instance_get_raytracing_enabled(const vtek::Instance* instance)
{
	return instance->rayTracingExtensionEnabled;
}

bool vtek::instance_get_validation_enabled(const vtek::Instance* instance)
{
	return instance->validationLayersEnabled;
}

const std::vector<const char*>& vtek::instance_get_validation_layer_names(const vtek::Instance*)
{
	return sValidationLayers;
}
