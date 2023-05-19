#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <map>

#include "vtek_logging.hpp"

#define VTEK_INVALID_ID (UINT64_MAX)


namespace vtek
{
	class IHostAllocator
	{
	public:
		IHostAllocator(const std::string& title) : mTitle(title) {}
		IHostAllocator(const std::string&& title) : mTitle(title) {}
		inline virtual ~IHostAllocator() {}

		virtual int GetNumAllocations() const = 0;
		inline std::string GetTitle() const { return mTitle; }

		inline void PrintInvalidFree(uint64_t id)
		{
			vtek_log_error("HostAllocator[{}] couldn't free object with id={}", mTitle, id);
		}

	private:
		const std::string mTitle;
	};


	void host_allocator_register_allocator(IHostAllocator* allocator);
	void host_allocator_check_all_freed();


	template<typename T>
	class HostAllocator : public IHostAllocator
	{
	public:
		HostAllocator(const std::string&& title)
		: IHostAllocator(std::forward<const std::string&&>(title)), mNext(0UL) {
			host_allocator_register_allocator(this);
		}

		inline int GetNumAllocations() const override { return mPool.size(); }
		inline std::pair<uint64_t, T*> alloc()
		{
			uint64_t id = mNext;
			auto [it, inserted] = mPool.insert({id, T{}});
			if (inserted)
			{
				mNext++;
				T* ptr = &(*it).second;
				return { id, ptr };
			}

			return { 0UL, nullptr };
		}
		inline void free(uint64_t id)
		{
			auto numRemoved = mPool.erase(id);
			if (numRemoved == 0)
			{
				PrintInvalidFree(id);
			}
		}
		inline void clear()
		{
			mPool.clear();
		}

	private:
		std::map<uint64_t, T> mPool;
		unsigned long int mNext;
	};
}
