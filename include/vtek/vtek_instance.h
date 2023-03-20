#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>

#include "vtek_glfw_window.h"


namespace vtek
{
	struct InstanceValidationSettings
	{
		bool debugSeverityVerbose {false};
		bool debugSeverityInfo {false};
		bool debugSeverityWarning {true};
		bool debugSeverityError {true};

		bool debugTypeGeneral {true};
		bool debugTypeValidation {true};
		bool debugTypePerformance {true};
	};

	struct InstanceCreateInfo
	{
		// General settings
		const char* applicationName {""};
		uint32_t applicationVersion {0U};
		const char* engineName {""};
		uint32_t engineVersion {0U};

		// Extension settings
		std::vector<const char*> requiredExtensions {}; // Prerequisites, e.g. provided by window library
		bool enableRayTracingExtension {false};

		// Validation settings
		bool enableValidationLayers {false};
		InstanceValidationSettings validationSettings {};
	};


	struct Instance; // opaque handle

	Instance* instance_create(InstanceCreateInfo* info);
	Instance* instance_create(InstanceCreateInfo* info, ApplicationWindow* window);
	void instance_destroy(Instance* instance);

	VkInstance instance_get_handle(const Instance* instance);
	bool instance_get_raytracing_enabled(const Instance* instance);

	bool instance_get_validation_enabled(const Instance* instance);
	const std::vector<const char*>& instance_get_validation_layer_names(const Instance* instance);
}
