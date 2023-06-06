#pragma once


namespace vtek
{
	// TODO: This is an idea!
	enum class BufferUpdatePolicy
	{
		// Buffer will never be updated once written to.
		never,

		// Buffer may be written to, but not often (ie. not every frame or similar)
		infrequently,

		// Buffer is updated very often, perhaps every frame.
		frequently
	};

}
