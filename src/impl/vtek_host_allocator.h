#pragma once

#include <string>
#include <utility>
#include <vector>

#include <iostream>


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
		: IHostAllocator(std::forward<const std::string&&>(title)) {
			host_allocator_register_allocator(this);
		}

		inline int GetNumAllocations() const override { return mPool.size(); }
		inline T* alloc()
		{
			mPool.push_back(T{});
			return &mPool.back();
		}
		inline void free(T* ptr)
		{
			if (ptr == nullptr) return;
			for (auto& it : mPool)
			{
				if (&it == ptr) { std::cout << "HostAllocator<T>::free(): FOUND\n"; }
			}
		}
		inline void clear()
		{
			mPool.clear();
		}

	private:
		std::vector<T> mPool;
	};
}
