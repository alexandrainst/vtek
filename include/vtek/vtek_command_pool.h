#include <vulkan/vulkan.h>
#include <memory>

#include "vtek_device.h"
#include "vtek_queue.h"


namespace vtek
{
	struct CommandPoolCreateInfo
	{
		// Allow individual command buffers to be re-recorded individually.
		// Without this flag, only the entire pool may be reset at once.
		bool allowIndividualBufferReset {false};

		// If command buffers are rerecorded often this flag may be set to
		// hint the driver at a more suitable allocation behaviour.
		bool hintRerecordOften {false}; // TODO: Better name?
	};

	struct CommandPool; // opaque handle



	CommandPool* command_pool_create(
		const CommandPoolCreateInfo* info, const Device* device, const Queue* queue);

	void command_pool_destroy(CommandPool* commandPool);
}
