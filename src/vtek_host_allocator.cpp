#include "impl/vtek_host_allocator.hpp"
#include "vtek_logging.hpp"

#include <algorithm>
#include <vector>


/* allocator storage */
static std::vector<vtek::IHostAllocator*> sAllocators = {};


/* interface */
void vtek::host_allocator_check_all_freed()
{
	vtek_log_debug("Freeing {} allocators..", sAllocators.size());
	for (auto* a : sAllocators)
	{
		vtek_log_debug("Freeing allocator: {}", a->GetTitle());
		if (a->GetNumAllocations() > 0)
		{
			vtek_log_warn("Host allocator \"{}\" had unfreed objects!", a->GetTitle());
		}
	}

	sAllocators.clear();
	sAllocators = {};
}

void vtek::host_allocator_register_allocator(vtek::IHostAllocator* allocator)
{
	vtek_log_debug("host_allocator_register_allocator {}", allocator->GetTitle());
	sAllocators.push_back(allocator);
}

void vtek::host_allocator_unregister_allocator(vtek::IHostAllocator* allocator)
{
	if (auto it = std::find(sAllocators.begin(), sAllocators.end(), allocator);
	    it != sAllocators.end())
	{
		sAllocators.erase(it);
	}
}
