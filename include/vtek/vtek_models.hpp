#pragma once

#include <string_view>

#include "vtek_fileio.hpp"


namespace vtek
{
	enum class ModelType
	{

	};

	struct Model; // opaque handle

	Model* model_load_obj(Directory* directory, std::string_view filename);

	void model_destroy(Model* model);
}
