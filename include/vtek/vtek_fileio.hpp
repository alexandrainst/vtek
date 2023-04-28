#pragma once

#include <string>
#include <string_view>
#include <vector>


namespace vtek
{
	// Opaque handles
	struct Directory;
	struct File;


	// ========================= //
	// === Utility functions === //
	// ========================= //
	std::string_view get_filename_suffix(std::string_view filename);
	char get_path_separator();


	// =========================== //
	// === Directory functions === //
	// =========================== //

	// Attemps to open a directory, specified by `path`. Will return
	// `nullptr` if the directory does not exist or if it cannot be accessed.
	// The directory must be closed when no longer needed by calling
	// `directory_close` - this will free all memory resources acquired
	// by this function.
	Directory* directory_open(std::string_view dir);

	// Attemps to open a subdirectory inside a given directory.
	// Returns nullptr if the subdirectory `subdir` does not exist.
	// The sub-directory must be closed when no longer needed by calling
	// `directory_close` - this will free all memory resources acquired
	// by this function.
	Directory* subdirectory_open(const Directory* dir, std::string_view subdir);

	// Attempts to open the canonical path "dir/subdir1/subdir2", where
	// `dir` is a valid directory already opened. Returns `nullptr` if one
	// of the subdirectories don't exist or cannot be accessed.
	// The sub-directory must be closed when no longer needed by calling
	// `directory_close` - this will free all memory resources acquired
	// by this function.
	Directory* subdirectory_open(
		const Directory* dir, std::string_view subdir1, std::string_view subdir2);

	// Will close the directory and free all memory that was allocated
	// when opening it.
	void directory_close(Directory* dir);

	// Get the directory name, as was provided when opening the directory.
	std::string_view directory_get_path(const Directory* dir);

	// Get the directory name, as was provided when opening the directory,
	// and appended `filename`.
	std::string directory_get_path(const Directory* dir, std::string_view filename);

	// If a directory was created as a relative path (relative to the executing
	// directory, or the location of the binary), then this function will
	// convert that directory's path to an absolute, fully-qualified path.
	std::string directory_get_absolute_path(const Directory* dir);

	// If a directory was created as a relative path (relative to the executing
	// directory, or the location of the binary), then this function will
	// convert that directory's path to an absolute, fully-qualified path.
	// In addition, it will append `filename` and return a platform-specific
	// path with proper path separator placement.
	// This function may be used for tools that require a fully-qualified path.
	std::string directory_get_absolute_path(
		const Directory* dir, std::string_view filename);


	// ====================== //
	// === File functions === //
	// ====================== //

	// Check if a given file exists within a previously opened directory.
	bool file_exists(const Directory* dir, std::string_view filename);

	// Various flags for controlling how a file should be opened. These
	// may be combined into bitmasks, though not all combinations are valid:
	// (read)
	// --> Opens the file for reading binary contents.
	// --> May not be used with write, trunc, or append.
	// (write)
	// --> Opens the file for writing contents.
	// --> May not be used with read.
	// --> May be used with either trunc or append, but not both.
	// --> If neither trunc nor append is specified, it defaults to trunc.
	// (trunc)
	// --> Truncates (destroys) the file contents and resets the file pointer.
	// --> Can only be used together with write.
	// (append)
	// --> Appends file contents on writes and preserves existing contents.
	// --> Can only be used together with write.
	// --> If the file is empty this will have the same effect as trunc.
	// (binary)
	// --> Specifies the file contents to be binary, ie. no character encoding
	//     to respect.
	// --> May be used with all other flags.
	struct FileModeFlag
	{
		enum flags : unsigned int
		{
			read     = 0x0001u,
			write    = 0x0002u,
			trunc    = 0x0004u,
			append   = 0x0008u,
			binary   = 0x0010u
		};
	};
	using FileModeFlags = unsigned int;

	// Attemps to open a file inside specified directory, where the usage
	// must be specified with the `flags` parameter described above.
	// Returns nullptr if flags are invalid, if the application has no rights
	// to read/write the file, or if the flags are not allowed by the underlying
	// platform's file system - ie. opening a file that does not exist.
	File* file_open(
		const Directory* dir, std::string_view filename, FileModeFlags flags);

	// Closes the file and releases all resources acquired when opening it.
	// This function may flush the file or finish any previously given writes
	// in a manner specified by the platform.
	void file_close(File* file);

	// Returns the size of a file, in bytes.
	uint64_t file_get_size_bytes(File* file);

	// Read the entire contents of the file, and copy it into `buffer`.
	bool file_read_into_buffer(File* file, std::vector<char>& buffer);

	// Read the file, line by line, until reaching EOF. The accumulated result
	// is stored in `accumBuffer`, while each consecutive read replaces the
	// contents of `line` with the next line.
	// Returns true if a line was read, and false otherwise (ie. EOF).
	bool file_read_line_accum(
		File* file, std::vector<char>& accumBuffer, std::vector<char>& line);
}
