#pragma once

#include <vulkan/vulkan.h>
#include "vtek_device.hpp"


namespace vtek
{
	struct FramebufferCreateInfo
	{

	};

	struct Framebuffer; // opaque handle


	Framebuffer* framebuffer_create(
		const FramebufferCreateInfo* info, Device* device);
	void framebuffer_destroy(Framebuffer* framebuffer);

	// TODO: What is actually needed/desired?
	std::vector<Framebuffer*> framebuffer_create_multiple();
	std::vector<Framebuffer*> framebuffer_create_for_swapchain();
}
