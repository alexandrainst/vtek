#include "impl/vtek_host_allocator.hpp"
#include "vtek_logging.hpp"


/* allocator storage */
static std::vector<vtek::IHostAllocator*> sAllocators = {};


/* interface */
void vtek::host_allocator_check_all_freed()
{
	for (auto* a : sAllocators)
	{
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
	sAllocators.push_back(allocator);
}
