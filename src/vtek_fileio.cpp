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



/* hashing of fileio structs (for memory pools) */
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



/* global definitions: memory pools and directory locations */
struct MemoryPool
{
	// TODO: This is really unoptimal! Can we use std::map or something else?
	//std::vector<vtek::Directory> directories;
	std::unordered_map<uint64_t, vtek::Directory> directories;
	std::unordered_map<uint64_t, vtek::File> files;

	// Id for indexing into the pools
	uint64_t dir_id {0};
	uint64_t file_id {0};

	// Mutex for thread-safe access to the pool
	std::mutex m;
};

struct DirectoryLocations
{
	// Where was the consuming application run from
	fs::path workingDirectory {};
	// Where is the consuming application located
	fs::path executableDirectory {};
};

static MemoryPool* sMemoryPool = nullptr;
static DirectoryLocations* sDirLocations = nullptr;



/* find directory locations */
// NOTE: More answers may be found here:
// https://stackoverflow.com/questions/1528298/get-path-of-executable
// https://stackoverflow.com/questions/143174/how-do-i-get-the-directory-that-a-program-is-running-from
static fs::path find_executable_directory()
{
	try
	{
		auto exec_path = fs::canonical("/proc/self/exe");
		return exec_path.parent_path();
	}
	catch (...)
	{
		vtek_log_fatal("fileio: 'find_executable_path()' throwed!");
		vtek_log_fatal("--> The application might not run!");
		return {};
	}
}



/* initialization */
#include "impl/vtek_init.hpp"

bool vtek::initialize_fileio()
{
	vtek_log_trace("vtek::initialize_fileio()");
	sMemoryPool = new MemoryPool();

	sDirLocations = new DirectoryLocations();
	sDirLocations->workingDirectory = fs::current_path();
	sDirLocations->executableDirectory = find_executable_directory();

	vtek_log_info("Working directory: {}{}",
	              sDirLocations->workingDirectory.c_str(),
	              fs::path::preferred_separator);
	vtek_log_info("Executable directory: {}{}",
	              sDirLocations->executableDirectory.c_str(),
	              fs::path::preferred_separator);

	return true;
}

void vtek::terminate_fileio()
{
	vtek_log_trace("vtek::terminate_fileio()");
	// TODO: We could check if some memory was not cleaned up.
	delete sMemoryPool;
	delete sDirLocations;
}


/* helper functions */
static std::ios_base::openmode read_file_mode_flags(vtek::FileModeFlags flags)
{
	std::ios_base::openmode openmode = static_cast<std::ios_base::openmode>(0);

	if (flags & vtek::FileModeFlag::read)
	{
		if (flags & vtek::FileModeFlag::write)
		{
			vtek_log_error("FileModeFlags cannot be both read and write!");
			return static_cast<std::ios_base::openmode>(0);
		}

		openmode |= std::ios::in;
	}
	else if (flags & vtek::FileModeFlag::write)
	{
		if ((flags & vtek::FileModeFlag::trunc) && (flags & vtek::FileModeFlag::append))
		{
			vtek_log_error("FileModeFlags cannot be both trunc and append!");
			return static_cast<std::ios_base::openmode>(0);
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

	return openmode;
}



/* fileio interface */
char vtek::get_path_separator()
{
	return fs::path::preferred_separator;
}

vtek::Directory* vtek::directory_open(std::string_view path)
{
	// If path ends with a trailing separator, remove it
	if (path.ends_with(fs::path::preferred_separator))
	{
		path.remove_suffix(1);
	}

	auto p = fs::path(path);
	if (!fs::exists(p))
	{
		vtek_log_error("Cannot open non-existing directory \"{}\"!", path);
		return nullptr;
	}

	std::lock_guard<std::mutex> lock(sMemoryPool->m);
	const uint64_t id = sMemoryPool->dir_id++;

	auto [it, insert] = sMemoryPool->directories.insert({ id, {id, p} });
	if (!insert)
	{
		vtek_log_debug("Failed to insert new Directory into std::unordered_map!");
		vtek_log_error("Failed to open directory!");
		return nullptr;
	}

	vtek_log_trace("Opened directory \"{}\"", p.c_str());
	return &(it->second);
}

vtek::Directory* vtek::subdirectory_open(
	const vtek::Directory* dir, std::string_view subdir)
{
	vtek_log_fatal("vtek::subdirectory_open(dir, str) - not implemented!");
	return nullptr;
}

vtek::Directory* vtek::subdirectory_open(
	const vtek::Directory* dir, std::string_view subdir1, std::string_view subdir2)
{
	vtek_log_fatal("vtek::subdirectory_open(dir, str, str) - not implemented!");
	return nullptr;
}

void vtek::directory_close(vtek::Directory* dir)
{
	vtek_log_fatal("vtek::directory_close - not implemented!");
}

std::string_view vtek::directory_get_path(const vtek::Directory* dir)
{
	return dir->handle.c_str();
}

std::string vtek::directory_get_path(
	const vtek::Directory* dir, std::string_view filename)
{
	auto path = dir->handle/filename;
	return std::string{path};
}

std::string vtek::directory_get_absolute_path(const vtek::Directory* dir)
{
	std::error_code ec; // Added so fs::absolute will not throw!
	auto path = fs::absolute(dir->handle);
	return path.native(); // Hopefully RVO
}

std::string vtek::directory_get_absolute_path(
	const vtek::Directory* dir, std::string_view filename)
{

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
	std::ios_base::openmode openmode = read_file_mode_flags(flags);
	if (openmode == static_cast<std::ios_base::openmode>(0))
	{
		vtek_log_error(
			"--> cannot open file \"{}{}{}\"", dir->handle.c_str(),
			fs::path::preferred_separator, filename.data());
		return nullptr;
	}

	// Mutex-lock the fileio module
	std::lock_guard<std::mutex> lock(sMemoryPool->m);

	std::error_code ec; // Added so fs::<...> will not throw!

	// The path exists
	auto path = dir->handle/filename;
	if (!fs::exists(path, ec)) { return nullptr; }

	// The file is a regular file
	auto entry = fs::directory_entry(path, ec);
	if (!entry.is_regular_file(ec)) return nullptr;

	// Allocate the file
	const uint64_t id = sMemoryPool->file_id++;
	auto [it, insert] = sMemoryPool->files.insert({ id, vtek::File{} });
	if (!insert)
	{
		vtek_log_debug("Failed to insert new Directory into std::unordered_map!");
		vtek_log_error("Failed to open directory!");
		return nullptr;
	}
	vtek::File* file = &it->second;
	file->id = id;

	// Open the file
	file->handle.open(path.c_str(), openmode);
	if (!file->handle.is_open())
	{
		file->handle.clear();
		sMemoryPool->files.erase(it);
		return nullptr;
	}
	file->size = static_cast<uint64_t>(fs::file_size(path));

	return file;
}

void vtek::file_close(vtek::File* file)
{
	file->handle.close();
	file->handle.clear();
	file->size = 0;

	auto it = sMemoryPool->files.find(file->id);
	if (it == sMemoryPool->files.end())
	{
		vtek_log_debug("Failed to find file with id in std::unordered_map!");
		vtek_log_debug("--> cannot deallocate file!");
	}

	sMemoryPool->files.erase(it);
}

bool vtek::file_read_into_buffer(vtek::File* file, std::vector<char>& buffer)
{
	buffer.resize(file->size);
	file->handle.seekg(0);
	file->handle.read(buffer.data(), file->size);
	file->handle.seekg(0);
	return true;
}

bool vtek::file_read_line_accum(
	vtek::File* file, std::vector<char>& accumBuffer, std::vector<char>& line)
{
	vtek_log_fatal("vtek::file_read_line_accum - not implemented!");
	return false;
}
