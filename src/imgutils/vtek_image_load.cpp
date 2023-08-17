#include "vtek_image_load.hpp"

#include "vtek_fileio.hpp"
#include "vtek_logging.hpp"

// Standard
#include <cstdio>

// External
// Define to let stbi_failure_reason return slightly more use-friendly messages.
#define STBI_FAILURE_USERMSG
// TODO: This message is taken from documentation of stb image:
// "If compiling for Windows and you wish to use Unicode filenames, compile with
//     #define STBI_WINDOWS_UTF8
// and pass utf8-encoded filenames. Call stbi_convert_wchar_to_utf8 to convert
// Windows wchar_t filenames to utf8.
// TODO: Figure out, if this is something we want!
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
	int desiredChannels = 0;

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
	case IFType::png: desiredChannels = STBI_rgb_alpha; break;
	case IFType::tga: break;
	case IFType::bmp: break;
	case IFType::hdr: break;
	default:
		vtek_log_error("vtek::image2d_load(): Unrecognized ImageFileType enum!");
		return false;
	}

	const char* const cpath = path.c_str();
	int width, height, channels;

	// Use a FILE* instead of opening the file several times
	std::FILE* fd = std::fopen(cpath, "r");
	if (fd == nullptr)
	{
		vtek_log_error("Failed to open image file!");
		return false;
	}

	// Conditionally load floating-point data
	if (stbi_is_hdr_from_file(fd))
	{
		outData->fdata = stbi_loadf_from_file(
			fd, &width, &height, &channels, 0);
	}
	else if (stbi_is_16_bit_from_file(fd))
	{
		// outData->data16 = stbi_load_from_file_16(
		// 	fd, &width, &height, &channels, 0);
		vtek_log_debug(
			"vtek_image_load.cpp: 16-bit image loading is not implemented!");
		std::fclose(fd);
		return false;
	}
	else
	{
		outData->data = stbi_load_from_file(
			fd, &width, &height, &channels, desiredChannels);
	}

	std::fclose(fd);

	if (outData->data == channels <= 0) // TODO: Proper check!
	{
		vtek_log_error("Failed to read image file: {}", stbi_failure_reason());
		return false;
	}

	// TODO: Probably assert that width, height are non-negative!
	if (width <= 0 || height <= 0 || channels <= 0)
	{
		vtek_log_fatal(
			"stbi_load returned invalid image data: (w={},h={},c={}) {}",
			width, height, channels, "-- cannot continue with image loading!");
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
	uint32_t channelSize =
		(loadData->data != nullptr) ? 1 :
		(loadData->data16 != nullptr) ? 2 :
		(loadData->fdata != nullptr) ? 32 :
		0;

	return channelSize * loadData->channels * loadData->width * loadData->height;
}
