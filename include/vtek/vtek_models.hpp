#pragma once

#include <string_view>

#include "vtek_fileio.hpp"
#include "vtek_object_handles.hpp"


namespace vtek
{
	enum class ModelType
	{

	};

	struct ModelInfo
	{
		// If for some reason the vertex data should reside in main memory
		// after loading a model, enable this value.
		bool keepVertexDataInMemory {false};
	};

	// Loads an obj model from disk and buffer its vertex data to GPU memory.
	Model* model_load_obj(
		const ModelInfo* info, Directory* directory, std::string_view filename,
		Device* device);

	void model_destroy(Model* model);

	Buffer* model_get_buffer_handle(Model* model);
}