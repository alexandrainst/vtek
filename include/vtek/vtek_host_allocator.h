#pragma once

#include <string>
#include <utility>
#include <vector>


namespace vtek
{
	class IHostAllocator
	{
	public:
		IHostAllocator(const std::string& title) : mTitle(title) {}
		IHostAllocator(const std::string&& title) : mTitle(title) {}
		inline virtual ~IHostAllocator() {}

		virtual int GetNumAllocations() = 0;
		std::string GetTitle();
	private:
		const std::string mTitle;
	};

	template<typename T>
	class HostAllocator : public IHostAllocator
	{
	public:
		HostAllocator(const std::string&& title)
		: IHostAllocator(std::forward<const std::string&&>(title)) {}

		inline int GetNumAllocations() override { return mPool.size(); }
		inline T* alloc()
		{
			mPool.push_back(T{});
			return &mPool.back();
		}
		inline void free(T* ptr)
		{
			if (ptr == nullptr) return;
		}
		inline void clear()
		{
			mPool.clear();
		}

	private:
		std::vector<T> mPool;
	};


	bool host_allocator_initialize();
	void host_allocator_destroy();

	void host_allocator_register_allocator(IHostAllocator* allocator);
}
