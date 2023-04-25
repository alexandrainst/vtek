//
// Provide initialization routines for internal vtek use.
// The functions provided here are implemented by their respective
// source files, e.g.:
// vtek_init_fileio -> vtek_fileio.cpp
//

#pragma once


namespace vtek
{
	/* logging */
	// TODO: Probably initialize the logger here?

	/* fileio module */
	bool initialize_fileio();
	void terminate_fileio();

	/* allocators for basic Vulkan types */
	// TODO: We could place initialization functions here and make everyting better.

}
