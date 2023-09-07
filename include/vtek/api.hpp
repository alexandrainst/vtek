#pragma once


/* Check compiler version to define proper linkage of shared libraries. */
#if defined(_MSC_VER)
	// Microsoft
	#define VTEK_EXPORT __declspec(dllexport)
	#define VTEK_IMPORT __declspec(dllimport)
#elif defined(__GNUC__) && __GNUC__ >= 4
	// GCC -- only viable for '__GNUC__ >= 4'
	#define VTEK_EXPORT __attribute__ ((visibility ("default")))
	#define VTEK_IMPORT __attribute__ ((visibility ("default")))
#else
	#define VTEK_EXPORT
	#define VTEK_IMPORT
	#error Unknown dynamic link import/export semantics in api.hpp (vtek)
#endif


/* Define common api for use across engine objects and functions. */
#if defined(VTEK_SHARED_LIB)
	#define VTEK_API VTEK_EXPORT
#else
	#define VTEK_API VTEK_IMPORT
#endif


/* Define inlining of functions (taken from Travis!) */
#if defined(_MSC_VER)
	#define VTEK_INLINE __forceinline
	#define VTEK_NOINLINE __declspec(noinline)
#elif defined(__GNUC__)
	#define VTEK_INLINE __attribute((always_inline))
	#define VTEK_NOINLINE
#else
	#define VTEK_INLINE
	#define VTEK_NOINLINE
	#error Unknown function inlining semantics in api.hpp (vtek)
#endif
