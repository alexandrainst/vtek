#include "vtek_fileio.hpp"

#include "vtek_logging.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <unordered_map>
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


/* hashing of fileio structs */
namespace std
{
	template<> struct hash<vtek::Directory>
	{
		std::size_t operator()(const vtek::Directory& v) const
		{
			return hash<const char*>()(v.handle.c_str());
		}
	};
}



/* memory pools */
struct MemoryPool
{
	// TODO: This is really unoptimal! Can we use std::map or something else?
	//std::vector<vtek::Directory> directories;
	std::unordered_map<const char*, vtek::Directory> directories;
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
char vtek::get_path_separator()
{
	return fs::path::preferred_separator;
}

vtek::Directory* vtek::directory_open(std::string_view path)
{
	auto p = fs::path(path);
	if (!fs::exists(p)) return nullptr;

	auto [it, insert] = sMemoryPool->directories.insert({ p.c_str(), vtek::Directory{0, p} });
	if (!insert)
	{
		vtek_log_debug("Failed to insert new Directory into std::unordered_map!");
		vtek_log_error("Failed to open directory!");
		return nullptr;
	}

	return &(it->second);
}

vtek::Directory* vtek::subdirectory_open(
	const vtek::Directory* dir, std::string_view subdir)
{
	vtek_log_error("vtek::subdirectory_open(dir, str) - not implemented!");
	return nullptr;
}

vtek::Directory* vtek::subdirectory_open(
	const vtek::Directory* dir, std::string_view subdir1, std::string_view subdir2)
{
	vtek_log_error("vtek::subdirectory_open(dir, str, str) - not implemented!");
	return nullptr;
}

void vtek::directory_close(vtek::Directory* dir)
{
	vtek_log_error("vtek::directory_close - not implemented!");
}

std::string_view vtek::directory_get_name(const vtek::Directory* dir)
{
	return dir->handle.c_str();
}



bool vtek::file_exists(const vtek::Directory* dir, std::string_view filename)
{
	std::error_code ec; // Added so fs::exists will not throw!
	auto p = dir->handle/filename;
	return fs::exists(p, ec);
}
