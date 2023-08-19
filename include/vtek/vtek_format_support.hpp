#pragma once

#include <cstdint>

#include "vtek_object_handles.hpp"


namespace vtek
{
	class FormatSupport
	{
	public:
		FormatSupport(const Device* device);

		inline bool sRGB_channel_1() { return mSRGB & 0x01; }
		inline bool sRGB_channel_2() { return mSRGB & 0x02; }
		inline bool sRGB_channel_3() { return mSRGB & 0x04; }

	private:
		uint8_t mSRGB {0U};
	};

	// NOTE: Implemented in src/imgutils/vtek_image_formats.cpp
};
