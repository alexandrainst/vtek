#pragma once

#include <cstdint>
#include <string_view>

#include "vtek_object_handles.hpp"


namespace vtek
{
	// ========================= //
	// === Utility functions === //
	// ========================= //
	enum class ImageFileType
	{
		empty, unsupported, jpg, png, tga, bmp, hdr
	};

	ImageFileType get_image_type(const std::string_view filename);

	// ===================== //
	// === Image loading === //
	// ===================== //
	struct ImageLoadData
	{
		uint8_t* data {nullptr};
		uint32_t width {0};
		uint32_t height {0};
		uint8_t channels {0};
		uint8_t bitsPerChannel {0};
	};

	struct ImageLoadInfo
	{
		int desiredChannels {0};
	};
	// TODO: ImageLoadInfoPNG, ImageLoadInfoJPG, ... ?

	bool image_load(
		const vtek::Directory* directory, std::string_view filename,
		const ImageLoadInfo* info, ImageLoadData* outData);
	void image_load_data_destroy(ImageLoadData* loadData);

	uint64_t image_load_data_get_size(const ImageLoadData* loadData);
}
