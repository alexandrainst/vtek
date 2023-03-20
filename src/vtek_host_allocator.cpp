#include "impl/vtek_host_allocator.h"
#include "vtek_logging.h"


/* allocator storage */
struct HostAllocatorStorage
{
	std::vector<vtek::IHostAllocator*> allocators;
};
static HostAllocatorStorage* sStorage;


/* interface */
bool vtek::host_allocator_initialize()
{
	sStorage = new HostAllocatorStorage();

	return true;
}

void vtek::host_allocator_destroy()
{
	if (sStorage == nullptr) { return; }

	for (auto* a : sStorage->allocators)
	{
		if (a->GetNumAllocations() > 0)
		{
			vtek_log_warn("Host allocator \"{}\" had unfreed objects!", a->GetTitle());
		}
	}

	sStorage->allocators.clear();
	sStorage = nullptr;
}

void vtek::host_allocator_register_allocator(vtek::IHostAllocator* allocator)
{
	sStorage->allocators.push_back(allocator);
}
