#include "vtek_fileio.hpp"

#include "vtek_logging.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;


/* struct implementations */
struct vtek::Directory
{
	// NOTE: This must be present, otherwise problems occur!
	// TODO: Is this error also present in vtek?
	int dummyMember;
	fs::path handle;
};

struct vtek::File
{
	uint64_t size;
	std::fstream handle;
	// TODO: Store a filename? `std::string filename;`, which is _not_ the entire path!
};


/* memory pools */
struct MemoryPool
{
	// TODO: This is really unoptimal! Can we use std::map or something else?
	std::vector<vtek::Directory> directories;
	std::vector<vtek::File> files;
};
static MemoryPool* sMemoryPool = nullptr;


/* initialization */
#include "impl/vtek_init.hpp"

bool vtek::initialize_fileio()
{
	vtek_log_trace("vtek::initialize_fileio()");
	sMemoryPool = new MemoryPool();

	return true;
}

void vtek::terminate_fileio()
{
	vtek_log_trace("vtek::terminate_fileio()");
	// TODO: We could check if some memory was not cleaned up.
	delete sMemoryPool;
}


/* fileio interface */
vtek::Directory* vtek::directory_open(const char* path)
{
	vtek_log_error("vtek::directory_open - not implemented!");
	return nullptr;
}

vtek::Directory* vtek::subdirectory_open(const vtek::Directory* dir, const char* subdir)
{
	vtek_log_error("vtek::subdirectory_open(dir, str) - not implemented!");
	return nullptr;
}

vtek::Directory* vtek::subdirectory_open(
	const vtek::Directory* dir, const char* subdir1, const char* subdir2)
{
	vtek_log_error("vtek::subdirectory_open(dir, str, str) - not implemented!");
	return nullptr;
}

void vtek::directory_close(vtek::Directory* dir)
{
	vtek_log_error("vtek::directory_close - not implemented!");
}
