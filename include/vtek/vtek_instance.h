#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>

#include "vtek_vulkan_version.h"


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
		VulkanVersion applicationVersion {0};

		// Extension settings
		std::vector<const char*> requiredExtensions {}; // Prerequisites, e.g. provided by window library
		bool enableRayTracingExtension {false};

		// Validation settings
		bool enableValidationLayers {false};
		InstanceValidationSettings validationSettings {};
	};


	struct Instance; // opaque handle

	Instance* instance_create(InstanceCreateInfo* info);
	void instance_destroy(Instance* instance);

	VkInstance instance_get_handle(const Instance* instance);
	bool instance_get_raytracing_enabled(const Instance* instance);

	bool instance_get_validation_enabled(const Instance* instance);
	const std::vector<const char*>& instance_get_validation_layer_names(const Instance* instance);
}
