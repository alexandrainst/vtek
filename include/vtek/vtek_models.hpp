#pragma once

#include <string_view>

#include "vtek_fileio.hpp"
#include "vtek_vulkan_handles.hpp"


namespace vtek
{
	enum class ModelType
	{

	};

	struct ModelInfo
	{

	};

	Model* model_load_obj(
		Directory* directory, std::string_view filename, Device* device);

	void model_destroy(Model* model);
}
