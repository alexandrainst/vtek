#include "vtek_fileio.hpp"

#include "vtek_logging.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;


/* struct implementations */
struct vtek::Directory
{
	// NOTE: This must be present, otherwise problems occur!
	// TODO: Is this error also present in vtek?
	uint64_t id;
	fs::path handle;
};

struct vtek::File
{
	uint64_t id;
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
			return hash<uint64_t>()(v.id);
		}
	};
}



/* memory pools */
struct MemoryPool
{
	// TODO: This is really unoptimal! Can we use std::map or something else?
	//std::vector<vtek::Directory> directories;
	std::unordered_map<uint64_t, vtek::Directory> directories;
	std::vector<vtek::File> files;

	// Id for indexing into the pools
	uint64_t dir_id {0};
	uint64_t file_id {0};

	// Mutex for thread-safe access to the pool
	std::mutex m;
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


/* helper functions */
static auto read_file_mode_flags(vtek::FileModeFlags flags)
{
	auto openmode = 0;

	if (flags & vtek::FileModeFlag::read)
	{
		if (flags & vtek::FileModeFlag::write)
		{
			vtek_log_error("FileModeFlags cannot be both read and write!");
			return 0;
		}

		openmode |= std::ios::in;
	}
	else if (flags & vtek::FileModeFlag::write)
	{
		if ((flags & vtek::FileModeFlag::trunc) && (flags & vtek::FileModeFlag::append))
		{
			vtek_log_error("FileModeFlags cannot be both trunc and append!");
			return 0;
		}
		else if (flags & vtek::FileModeFlag::trunc)
		{
			openmode |= std::ios::trunc;
		}
		else if (flags & vtek::FileModeFlag::append)
		{
			openmode |= std::ios::app;
		}

		openmode |= std::ios::out;
	}

	if (flags & vtek::FileModeFlag::binary)
	{
		openmode |= std::ios::binary;
	}

	// Always seek to end, so we can efficiently obtain the size of the file
	openmode |= std::ios::ate;

	return openmode;
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

	std::lock_guard<std::mutex> lock(sMemoryPool->m);
	const uint64_t id = sMemoryPool->dir_id++;

	auto [it, insert] = sMemoryPool->directories.insert({ id, /*vtek::Directory*/{0, p} });
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

vtek::File* vtek::file_open(
	const vtek::Directory* dir, std::string_view filename, vtek::FileModeFlags flags)
{
	auto openmode = read_file_mode_flags(flags);
	if (openmode == 0)
	{
		vtek_log_error(
			"--> cannot open file \"{}{}{}\"", dir->handle.c_str(),
			fs::path::preferred_separator, filename.data());
		return nullptr;
	}

	// Mutex-lock the fileio module
	std::lock_guard<std::mutex> lock(sMemoryPool->m);

	std::error_code ec; // Added so fs::exists will not throw!

	// The path exists
	auto path = dir->handle/filename;
	if (!fs::exists(path, ec)) { return nullptr; }

	// The file is a regular file
	auto entry = fs::directory_entry(path, ec);
	if (!entry.is_regular_file(ec)) return nullptr;

	

	return nullptr;
}
