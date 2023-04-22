#include <vtek/vtek.hpp>
#include <iostream>

// global data
vtek::ApplicationWindow* window = nullptr;


void keyCallback(vtek::KeyboardKey key, vtek::InputAction action)
{
	std::cout << "key callback\n";
	using vtek::KeyboardKey;
	using vtek::InputAction;

	if (action == InputAction::press)
	{

	}
	else if (action == InputAction::release)
	{
		switch (key)
		{
		case KeyboardKey::escape:
			vtek::window_set_should_close(window, true);
			break;
		default:
			break;
		}
	}
}

// TODO: Provide function parameters (likely a _lot_) :
// window, device, swapchain, frameSync, --renderPass--, --framebuffers--, commandBuffers
// --> (window, device, swapchain, frameSync, commandBuffers) <---
void recreateSwapchain()
{
	// 1) window minimization guard
	// 2) device wait idle
	// 3) recreate swapchain
	// 4) reset frame sync
	// 5) -- rebuild render pass --
	// 6) -- recreate swapchain framebuffers --
	// 7) re-record command buffers
}


int main()
{
	// Initialize vtek
	vtek::InitInfo initInfo{};
	initInfo.applicationTitle = "triangle_plain";
	initInfo.useGLFW = true;
	if (!vtek::initialize(&initInfo))
	{
		std::cerr << "Failed to initialize vtek!" << std::endl;
		return -1;
	}

	// Create window
	vtek::WindowCreateInfo windowInfo{};
	windowInfo.title = "triangle_plain";
	windowInfo.width = 500;
	windowInfo.height = 500;
	// TODO: Set resizeable to false for now!
	window = vtek::window_create(&windowInfo);
	if (window == nullptr)
	{
		log_error("Failed to create window!");
		return -1;
	}
	vtek::window_set_key_handler(window, keyCallback);

	// Vulkan instance
	vtek::InstanceCreateInfo instanceInfo{};
	instanceInfo.applicationName = "triangle_plain";
	instanceInfo.applicationVersion = vtek::VulkanVersion(1, 0, 0);
	instanceInfo.enableValidationLayers = true;
	auto instance = vtek::instance_create(&instanceInfo);
	if (instance == nullptr)
	{
		log_error("Failed to create Vulkan instance!");
		return -1;
	}

	// Surface
	VkSurfaceKHR surface = vtek::window_create_surface(window, instance);
	if (surface == VK_NULL_HANDLE)
	{
		log_error("Failed to create Vulkan window surface!");
		return -1;
	}

	// Physical device
	vtek::PhysicalDeviceInfo physicalDeviceInfo{};
	physicalDeviceInfo.requireGraphicsQueue = true;
	physicalDeviceInfo.requirePresentQueue = true;
	physicalDeviceInfo.requireSwapchainSupport = true;
	vtek::PhysicalDevice* physicalDevice = vtek::physical_device_pick(&physicalDeviceInfo, instance, surface);
	if (physicalDevice == nullptr)
	{
		log_error("Failed to pick physical device!");
		return -1;
	}

	// Device
	vtek::LogicalDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.enableSwapchainExtension = true; // TODO: We probably don't want to do this here!
	vtek::Device* device = vtek::device_create(&deviceCreateInfo, instance, physicalDevice);
	if (device == nullptr)
	{
		log_error("Failed to create device!");
		return -1;
	}

	// Graphics queue
	vtek::Queue* graphicsQueue = vtek::device_get_graphics_queue(device);
	if (graphicsQueue == nullptr)
	{
		log_error("Failed to get graphics queue!");
		return -1;
	}

	// Graphics command pool
	vtek::CommandPoolCreateInfo commandPoolCreateInfo{};
	vtek::CommandPool* graphicsCommandPool = vtek::command_pool_create(
		&commandPoolCreateInfo, device, graphicsQueue);
	if (graphicsCommandPool == nullptr)
	{
		log_error("Failed to create graphics command pool!");
		return -1;
	}

	// Swapchain
	vtek::SwapchainCreateInfo swapchainCreateInfo{};
	swapchainCreateInfo.vsync = true;
	swapchainCreateInfo.prioritizeLowLatency = false;
	vtek::window_get_framebuffer_size(
		window, &swapchainCreateInfo.framebufferWidth, &swapchainCreateInfo.framebufferHeight);
	vtek::Swapchain* swapchain = vtek::swapchain_create(&swapchainCreateInfo, surface, physicalDevice, device);
	if (swapchain == nullptr)
	{
		log_error("Failed to create swapchain!");
		return -1;
	}

	// Vulkan render pass
	// DONE: We use dynamic rendering

	// Vulkan swapchain framebuffers (after render pass ?)
	// DONE: We use dynamic rendering

	// Vulkan framebuffers
	// DONE: We use dynamic rendering

	// Vulkan graphics pipeline
	vtek::GraphicsShader* shader = nullptr;// = vtek::graphics_shader_load(...);

	const uint32_t width = swapchainCreateInfo.framebufferWidth;
	const uint32_t height = swapchainCreateInfo.framebufferHeight;
	vtek::ViewportState viewport{
		.viewportRegion = {
			.offset = {0U, 0U},
			.extent = {width, height}
		},
	};
	vtek::RasterizationState rasterizer{};
	vtek::MultisampleState multisampling{};
	vtek::DepthStencilState depthStencil{}; // No depth testing!
	vtek::ColorBlendState colorBlending{};
	colorBlending.attachments.emplace_back(
		vtek::ColorBlendAttachment::GetBlendingDisabled());
	vtek::PipelineRendering pipelineRendering{};
	pipelineRendering.colorAttachmentFormats.push_back(
		vtek::swapchain_get_image_format(swapchain));

	vtek::GraphicsPipelineCreateInfo graphicsPipelineInfo{
		.renderPassType = vtek::RenderPassType::dynamic,
		.renderPass = nullptr, // Nice!
		.pipelineRendering = &pipelineRendering,
		.shader = shader,
		.vertexType = vtek::VertexType::vec2,
		.instancedRendering = false,
		.primitiveTopology = vtek::PrimitiveTopology::triangle_list,
		.enablePrimitiveRestart = false,
		.viewportState = &viewport,
		.rasterizationState = &rasterizer,
		.multisampleState = &multisampling,
		.depthStencilState = &depthStencil,
		.colorBlendState = &colorBlending,
		.dynamicStateFlags = 0U //vtek::PipelineDynamicState::viewport;
	};
	vtek::GraphicsPipeline* graphicsPipeline = vtek::graphics_pipeline_create(
		&graphicsPipelineInfo, device);
	if (graphicsPipeline == nullptr)
	{
		log_error("Failed to create graphics pipeline!");
		return -1;
	}

	// REVIEW: geometry ?

	// Command buffers
	const uint32_t commandBufferCount = vtek::swapchain_get_length(swapchain);
	vtek::CommandBufferCreateInfo commandBufferInfo{};
	commandBufferInfo.isSecondary = false;
	std::vector<vtek::CommandBuffer*> commandBuffers = vtek::command_buffer_create(
		&commandBufferInfo, commandBufferCount, graphicsCommandPool, device);
	if (commandBuffers.empty())
	{
		log_error("Failed to create command buffer!");
		return -1;
	}
	if (commandBufferCount != commandBuffers.size())
	{
		log_error("Number of command buffers created not same as number asked!");
		return -1;
	}

	// Command buffer recording
	for (uint32_t i = 0; i < commandBufferCount; i++)
	{
		vtek::CommandBuffer* commandBuffer = commandBuffers[i];
		VkCommandBuffer cmdBuf = vtek::command_buffer_get_handle(commandBuffer);

		if (!vtek::command_buffer_begin(commandBuffer))
		{
			log_error("Failed to begin command buffer {} recording!", i);
			return -1;
		}

		// Transition from whatever (probably present src) to color attachment
		VkImageMemoryBarrier beginBarrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.image = vtek::swapchain_get_image(swapchain, i),
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		vkCmdPipelineBarrier(
			cmdBuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr,
			0, nullptr, 1, &beginBarrier);

		VkRenderingAttachmentInfo colorAttachmentInfo{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO, // _KHR ??
			.imageView = vtek::swapchain_get_image_view(swapchain, i),
			.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = { .color = { .float32 = {1.0f, 0.0f, 0.0f, 1.0f} } },
		};

		VkRenderingInfo renderingInfo{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO, // _KHR ??
			.renderArea = { .offset = {0U, 0U}, .extent = {width, height} },
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachmentInfo
		};
		vkCmdBeginRendering(cmdBuf, &renderingInfo);

		// draw calls here
		// TODO: Can we do this without a bound vertex buffer?
		// vkCmdDraw(cmdBuf, ...)

		// End dynamic rendering
		vkCmdEndRendering(cmdBuf);

		// Transition from color attachment to present src
		// PROG: We can extract function from this pipeline barrier (vtek::dynamic_rendering_insert_barrier(), etc.).
		VkImageMemoryBarrier endBarrier{};
		endBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		endBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		endBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		endBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		endBarrier.image = vtek::swapchain_get_image(swapchain, i);
		endBarrier.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		};

		vkCmdPipelineBarrier(
			cmdBuf, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &endBarrier);

		if (!vtek::command_buffer_end(commandBuffer))
		{
			log_error("Failed to end command buffer {} recording!", i);
			return -1;
		}
	}

	// Error tolerance
	int errors = 10;

	while (vtek::window_get_should_close(window) && errors > 0)
	{
		// React to incoming input events
		vtek::window_poll_events();

		// TODO: Check if framebuffer has been resized.

		// To avoid excessive GPU work we wait until we may begin the frame
		if (!vtek::swapchain_wait_begin_frame(swapchain, device)) { errors--; continue; }

		// Acquire the next available image in the swapchain
		uint32_t frameIndex;
		vtek::swapchain_acquire_next_image(swapchain, &frameIndex);

		// Wait until any previous operations are finished using this image, for either read or write.
		// NOTE: We can do command buffer recording or other operations before calling this function.
		vtek::swapchain_wait_image_ready(swapchain, frameIndex); // TODO: if (...)

		// Submit the current command buffer for execution on the graphics queue
		vtek::SubmitInfo submitInfo{};
		vtek::swapchain_fill_queue_submit_info(swapchain, &submitInfo);
		vtek::queue_submit(graphicsQueue, commandBuffers[frameIndex], &submitInfo);

		// Wait for command buffer to finish execution, and present frame to screen.
		vtek::swapchain_wait_end_frame(swapchain, frameIndex);
	}



	// Cleanup
	vtek::swapchain_destroy(swapchain, device);
	vtek::command_pool_destroy(graphicsCommandPool, device);
	vtek::device_destroy(device);
	vtek::physical_device_release(physicalDevice);
	vtek::window_surface_destroy(surface, instance);
	vtek::instance_destroy(instance);
	vtek::window_destroy(window);

	log_debug("All went well!");
	vtek::terminate();

	return 0;
}
