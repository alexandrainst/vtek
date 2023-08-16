#include "vtek_vulkan.pch"
#include "vtek_image_load.hpp"

#include "vtek_fileio.hpp"
#include "vtek_logging.hpp"

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



/* image loading helpers */
// TODO: We very much want this! :
static bool image_load_jpg(
	const std::string& path, const vtek::ImageLoadInfo* info,
	vtek::ImageLoadData* outData)
{
	vtek_log_debug("vtek_image_load.cpp: image_load_jpg(): Not implemented!");
	return false;
}

static bool image_load_png(
	const std::string& path, const vtek::ImageLoadInfo* info,
	vtek::ImageLoadData* outData)
{
	bool image16 = false; // image contains 16-bit data
	bool imagef = false;  // image has a floating-point format

	if (stbi_is_16_bit(path.c_str())) { image16 = true; }

	vtek_log_debug("vtek_image_load.cpp: image_load_png(): Not implemented!");
	return false;
}

static bool image_load_tga(
	const std::string& path, const vtek::ImageLoadInfo* info,
	vtek::ImageLoadData* outData)
{
	vtek_log_debug("vtek_image_load.cpp: image_load_tga(): Not implemented!");
	return false;
}

static bool image_load_bmp(
	const std::string& path, const vtek::ImageLoadInfo* info,
	vtek::ImageLoadData* outData)
{
	vtek_log_debug("vtek_image_load.cpp: image_load_bmp(): Not implemented!");
	return false;
}

static bool image_load_hdr(
	const std::string& path, const vtek::ImageLoadInfo* info,
	vtek::ImageLoadData* outData)
{
	bool hdr = false;
	if (stbi_is_hdr(path.c_str())) { hdr = true; }

	vtek_log_debug("vtek_image_load.cpp: image_load_hdr(): Not implemented!");
	return false;
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

	// Query image info without having to decode the entire image first.
	// This includes width, height, channels, bit-format, and layout.
	int x,y,n,ok;
	ok = stbi_info(path.c_str(), &x, &y, &n);
	if (!ok)
	{
		vtek_log_error("Image format is unsupported!");
		return false;
	}

	int width, height, channels;

	outData->data = stbi_load(
		path.c_str(), &width, &height, &channels, desiredChannels);
	if (outData->data == nullptr || channels <= 0)
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
	return loadData->bitsPerChannel
		* loadData->channels
		* loadData->width
		* loadData->height;
}
