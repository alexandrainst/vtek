#include "vtek_vulkan.pch"
#include "vtek_image_load.hpp"

#include "vtek_fileio.hpp"
#include "vtek_logging.hpp"

// External
#include "stb_image.h"

using IFType = vtek::ImageFileType;


/* utility functions */
vtek::ImageFileType vtek::get_image_type(const std::string_view filename)
{
	const std::string_view extv = vtek::filename_get_extension(filename);
	if (extv.empty()) return IFType::empty;

	std::string ext(extv);
	for (auto& c : ext)
	{
		c = std::tolower(c);
	}

	if (ext == "jpg" || ext == "jpeg") return IFType::jpg;
	if (ext == "png") return IFType::png;
	if (ext == "tga") return IFType::tga;
	if (ext == "bmp") return IFType::bmp;
	if (ext == "hdr") return IFType::hdr;

	return IFType::unsupported;
}



/* image loading */
bool vtek::image_load(
	const vtek::Directory* directory, std::string_view filename,
	const ImageLoadInfo* info, ImageLoadData* outData)
{
	const std::string path = vtek::directory_get_path(directory, filename);

	if (!vtek::file_exists(directory, filename))
	{
		vtek_log_error("No image found in path {}", path);
		return false;
	}

	vtek::ImageFileType filetype = vtek::get_image_type(filename);
	switch (filetype)
	{
	case IFType::empty:
		vtek_log_error(
			"Image file extension is empty (filename={})!", filename);
		return false;
	case IFType::unsupported:
		vtek_log_error(
			"Unsupported image file extension (filename={})!", filename);
		return false;

	case IFType::jpg: break;
	case IFType::png: break;
	case IFType::tga: break;
	case IFType::bmp: break;
	case IFType::hdr: break;
	default:
		vtek_log_error("vtek::image2d_load(): Unrecognized ImageFileType enum!");
		return false;
	}

	switch (info->desiredChannels)
	{
	case 0:
		vtek_log_error("Cannot load image with 0 channels specified!");
		return false;
	}

	int desiredChannels = STBI_rgb_alpha; // TODO: Specify somehow!
	int width, height, channels;

	outData->data = stbi_load(
		path.c_str(), &width, &height, &channels, desiredChannels);
	if (outData->data == nullptr)
	{
		vtek_log_error("Failed to read image file!");
		return false;
	}

	// TODO: Probably assert that width, height are non-negative!
	if (width < 0 || height < 0 || channels < 0)
	{
		vtek_log_fatal(
			"stbi_load return negative values for width|height|channels {}",
			"-- cannot continue with image loading!");
		vtek::image_load_data_destroy(outData);
		return false;
	}
	outData->width = static_cast<uint32_t>(width);
	outData->height = static_cast<uint32_t>(height);
	outData->channels = static_cast<uint32_t>(channels);

	return true;
}

void vtek::image_load_data_destroy(vtek::ImageLoadData* loadData)
{
	stbi_image_free(loadData->data);
	*loadData = {}; // zero-initialize
}

uint64_t vtek::image_load_data_get_size(const vtek::ImageLoadData* loadData)
{
	return loadData->bitsPerChannel
		* loadData->channels
		* loadData->width
		* loadData->height;
}
