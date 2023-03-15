#include "vtek_main.h"
#include "vtek_host_allocator.h"

#include "FmDebug.h" // TODO: Write our own logger.


bool vtek::initialize(const vtek::InitInfo* info)
{
	if (!vtek::host_allocator_initialize())
	{
		Fundament::FmDebugging().logCritical(
			"Failed to initialize vtek host allocator!", __FILE__, __LINE__);
		return false;
	}

	return true;
}

void vtek::terminate()
{

}
