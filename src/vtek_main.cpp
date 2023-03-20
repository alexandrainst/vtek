#include "vtek_main.h"
#include "vtek_host_allocator.h"
#include "vtek_logging.h"


bool vtek::initialize(const vtek::InitInfo* info)
{
	if (!vtek::host_allocator_initialize())
	{
		vtek_log_fatal("Failed to initialize vtek host allocator!");
		return false;
	}

	return true;
}

void vtek::terminate()
{

}
