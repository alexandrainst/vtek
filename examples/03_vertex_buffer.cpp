#include <vtek/vtek.hpp>
#include <iostream>
#include <vector>

// global data
vtek::ApplicationWindow* window = nullptr;


void keyCallback(vtek::KeyboardKey key, vtek::InputAction action)
{
	if ((action == vtek::InputAction::release) &&
	    (key == vtek::KeyboardKey::escape))
	{
		vtek::window_set_should_close(window, true);
	}
}


struct Vertex2D
{
	glm::vec2 position;
	glm::vec3 color;
};


int main()
{
	// Initialize vtek
	vtek::InitInfo initInfo{};
	initInfo.disableLogging = false;
	initInfo.applicationTitle = "03_vertex_buffer";
	initInfo.useGLFW = true;
	if (!vtek::initialize(&initInfo))
	{
		std::cerr << "Failed to initialize vtek!" << std::endl;
		return -1;
	}

	// Create window
	vtek::WindowCreateInfo windowInfo{};
	windowInfo.title = "vtek example 03: Vertex buffer";
	window = vtek::window_create(&windowInfo);
	if (window == nullptr)
	{
		log_error("Failed to create window!");
		return -1;
	}
	vtek::window_set_key_handler(window, keyCallback);

	// Vulkan instance
	vtek::InstanceCreateInfo instanceInfo{};
	instanceInfo.applicationName = "vertex_buffer";
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
	physicalDeviceInfo.requireDynamicRendering = true;
	vtek::PhysicalDevice* physicalDevice = vtek::physical_device_pick(
		&physicalDeviceInfo, instance, surface);
	if (physicalDevice == nullptr)
	{
		log_error("Failed to pick physical device!");
		return -1;
	}

	// Device
	vtek::DeviceCreateInfo deviceCreateInfo{};
	// TODO: Require independent transfer queue, because we can!
	// TODO: -- also to test the vertex buffer transfer!
	vtek::Device* device = vtek::device_create(
		&deviceCreateInfo, instance, physicalDevice);
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
	vtek::CommandPoolInfo commandPoolInfo{};
	vtek::CommandPool* graphicsCommandPool = vtek::command_pool_create(
		&commandPoolInfo, device, graphicsQueue);
	if (graphicsCommandPool == nullptr)
	{
		log_error("Failed to create graphics command pool!");
		return -1;
	}

	// Swapchain
	vtek::SwapchainInfo swapchainInfo{};
	swapchainInfo.vsync = true;
	swapchainInfo.prioritizeLowLatency = false;
	vtek::window_get_framebuffer_size(
		window, &swapchainInfo.framebufferWidth, &swapchainInfo.framebufferHeight);
	vtek::Swapchain* swapchain =
		vtek::swapchain_create(&swapchainInfo, surface, physicalDevice, device);
	if (swapchain == nullptr)
	{
		log_error("Failed to create swapchain!");
		return -1;
	}

	// Shader
	const char* shaderdirstr = "../shaders/simple_vertex/";
	vtek::Directory* shaderdir = vtek::directory_open(shaderdirstr);
	if (shaderdir == nullptr)
	{
		log_error("Failed to open shader directory!");
		return -1;
	}
	vtek::GraphicsShaderInfo shaderInfo{};
	shaderInfo.vertex = true;
	shaderInfo.fragment = true;
	vtek::GraphicsShader* shader =
		vtek::graphics_shader_load_spirv(&shaderInfo, shaderdir, device);
	if (shader == nullptr)
	{
		log_error("Failed to load graphics shader!");
		return -1;
	}

	// vertex data
	std::vector<Vertex2D> vertices = {
		{{-0.5f, 0.5f}, {1.0f, 1.0f, 0.0f}},
		{{0.0f, -0.75f}, {1.0f, 0.7f, 0.4f}},
		{{0.75f, 0.3f}, {0.9f, 0.9f, 0.6f}}
	};

	// Vertex buffer
	log_trace("Vertex buffer");
	vtek::BufferInfo bufferInfo{};
	bufferInfo.size = sizeof(Vertex2D) * vertices.size();
	bufferInfo.requireHostVisibleStorage = true; // NOTE: Easy for now.
	//bufferInfo.disallowInternalStagingBuffer = true;
	//bufferInfo.requireDedicatedAllocation = true;
	bufferInfo.writePolicy = vtek::BufferWritePolicy::write_once;
	bufferInfo.usageFlags
		= vtek::BufferUsageFlag::transfer_dst
		| vtek::BufferUsageFlag::vertex_buffer;
	vtek::Buffer* buffer = vtek::buffer_create(&bufferInfo, device);
	if (buffer == nullptr)
	{
		log_error("Failed to create vertex buffer!");
		return -1;
	}
	vtek::BufferRegion vertexRegion{};
	if (!vtek::buffer_write_data(buffer, vertices.data(), &vertexRegion, device))
	{
		log_error("Failed to write data to vertex buffer!");
		return -1;
	}

	// Vulkan graphics pipeline
	log_trace("Vulkan graphics pipeline");
	const uint32_t width = swapchainInfo.framebufferWidth;
	const uint32_t height = swapchainInfo.framebufferHeight;
	vtek::ViewportState viewport{
		.viewportRegion = {
			.offset = {0U, 0U},
			.extent = {width, height}
		},
	};
	vtek::VertexBufferBindings bindings{};
	bindings.add_buffer(
		vtek::VertexAttributeType::vec2,
		vtek::VertexAttributeType::vec3,
		vtek::VertexInputRate::per_vertex);
	vtek::RasterizationState rasterizer{};
	vtek::MultisampleState multisampling{};
	vtek::DepthStencilState depthStencil{}; // No depth testing!
	vtek::ColorBlendState colorBlending{};
	colorBlending.attachments.emplace_back(
		vtek::ColorBlendAttachment::GetDefault());
	vtek::PipelineRendering pipelineRendering{};
	pipelineRendering.colorAttachmentFormats.push_back(
		vtek::swapchain_get_image_format(swapchain));

	vtek::GraphicsPipelineCreateInfo graphicsPipelineInfo{};
	graphicsPipelineInfo.renderPassType = vtek::RenderPassType::dynamic;
	graphicsPipelineInfo.renderPass = nullptr; // Nice!
	graphicsPipelineInfo.pipelineRendering = &pipelineRendering;
	graphicsPipelineInfo.shader = shader;
	// TODO: Rename to `vertexBufferBindings`
	graphicsPipelineInfo.vertexInputBindings = &bindings;
	graphicsPipelineInfo.primitiveTopology = vtek::PrimitiveTopology::triangle_list;
	graphicsPipelineInfo.enablePrimitiveRestart = false;
	graphicsPipelineInfo.viewportState = &viewport;
	graphicsPipelineInfo.rasterizationState = &rasterizer;
	graphicsPipelineInfo.multisampleState = &multisampling;
	graphicsPipelineInfo.depthStencilState = &depthStencil;
	graphicsPipelineInfo.colorBlendState = &colorBlending;

	vtek::GraphicsPipeline* graphicsPipeline = vtek::graphics_pipeline_create(
		&graphicsPipelineInfo, device);
	if (graphicsPipeline == nullptr)
	{
		log_error("Failed to create graphics pipeline!");
		return -1;
	}

	// Command buffers
	log_trace("Command buffers");
	// TODO: Can we do with only a single command buffer?
	// TODO: -- because I kinda wanna try!
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
	VkPipeline pipl = vtek::graphics_pipeline_get_handle(graphicsPipeline);
	uint32_t queueIndex = vtek::queue_get_family_index(graphicsQueue);
	for (uint32_t i = 0; i < commandBufferCount; i++)
	{
		vtek::CommandBuffer* commandBuffer = commandBuffers[i];
		VkCommandBuffer cmdBuf = vtek::command_buffer_get_handle(commandBuffer);

		if (!vtek::command_buffer_begin(commandBuffer))
		{
			log_error("Failed to begin command buffer {} recording!", i);
			return -1;
		}

		// NEXT: Cleanup a bit
		//VkImage image = vtek::swapchain_get_image(swapchain, i);
		//VkImageView imageView = vtek::swapchain_get_image_view(swapchain, i);
		//vtek::swapchain_barrier_prerendering(i);
		//render...
		//vtek::swapchain_barrier_postrendering(i);

		// Transition from whatever (probably present src) to color attachment
		VkImageMemoryBarrier beginBarrier{
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext = nullptr,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.srcQueueFamilyIndex = queueIndex,
			.dstQueueFamilyIndex = queueIndex,
			.image = vtek::swapchain_get_image(swapchain, i),
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};
		// TODO: Also use barrier for depth/stencil image!

		vkCmdPipelineBarrier(
			cmdBuf, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr,
			0, nullptr, 1, &beginBarrier);

		VkRenderingAttachmentInfo colorAttachmentInfo{
			.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO, // _KHR ??
			.pNext = nullptr,
			.imageView = vtek::swapchain_get_image_view(swapchain, i),
			.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			.resolveMode = VK_RESOLVE_MODE_NONE,
			.resolveImageView = VK_NULL_HANDLE,
			.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = { .color = { .float32 = {0.3f, 0.3f, 0.3f, 1.0f} } },
		};

		VkRenderingInfo renderingInfo{};
		renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
		renderingInfo.pNext = nullptr;
		renderingInfo.flags = 0;
		renderingInfo.renderArea = { 0U, 0U, width, height };
		renderingInfo.layerCount = 1;
		renderingInfo.viewMask = 0;
		renderingInfo.colorAttachmentCount = 1;
		renderingInfo.pColorAttachments = &colorAttachmentInfo;
		renderingInfo.pDepthAttachment = nullptr;
		renderingInfo.pStencilAttachment = nullptr;

		vkCmdBeginRendering(cmdBuf, &renderingInfo);

		vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipl);

		// Bind vertex buffer
		VkBuffer buffers[1] = { vtek::buffer_get_handle(buffer) };
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(cmdBuf, 0, 1, buffers, offsets);

		// draw calls here
		vkCmdDraw(cmdBuf, vertices.size(), 1, 0, 0);

		// End dynamic rendering
		vkCmdEndRendering(cmdBuf);

		// Transition from color attachment to present src
		// PROG: We can extract function from this pipeline barrier (vtek::dynamic_rendering_insert_barrier(), etc.).
		VkImageMemoryBarrier endBarrier{};
		endBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		endBarrier.pNext = nullptr;
		endBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		endBarrier.dstAccessMask = 0;
		endBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		endBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		endBarrier.srcQueueFamilyIndex = queueIndex;
		endBarrier.dstQueueFamilyIndex = queueIndex;
		endBarrier.image = vtek::swapchain_get_image(swapchain, i);
		endBarrier.subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		};
		// TODO: Also use barrier for depth/stencil image!

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

		// To avoid excessive GPU work we wait until we may begin the frame
		auto beginStatus = vtek::swapchain_wait_begin_frame(swapchain, device);
		if (beginStatus == vtek::SwapchainStatus::timeout)
		{
			log_error("Failed to wait begin frame - swapchain timeout!");
			// TODO: Probably log an error and then run the loop again.
			errors--; continue;
		}
		else if (beginStatus == vtek::SwapchainStatus::error)
		{
			log_error("Failed to wait begin frame - swapchain error!");
			// NEXT: Terminate application.
		}

		// Acquire the next available image in the swapchain
		uint32_t frameIndex;
		auto acquireStatus = vtek::swapchain_acquire_next_image(swapchain, device, &frameIndex);
		if (acquireStatus == vtek::SwapchainStatus::outofdate)
		{
			log_error("Failed to acquire image - swapchain outofdate!");
			// TODO: Rebuild swapchain
			// NOTE: Swapchain _may_ indeed change length!
		}
		else if (acquireStatus == vtek::SwapchainStatus::error)
		{
			log_error("Failed to acquire image - swapchain error!");
			// NEXT: Terminate application.
		}

		// Wait until any previous operations are finished using this image, for either read or write.
		// NOTE: We can do command buffer recording or other operations before calling this function.
		auto readyStatus = vtek::swapchain_wait_image_ready(swapchain, device, frameIndex);
		if (readyStatus == vtek::SwapchainStatus::timeout)
		{
			log_error("Failed to wait image ready - swapchain timeout!");
			// TODO: Probably log an error and then run the loop again.
		}
		else if (readyStatus == vtek::SwapchainStatus::error)
		{
			log_error("Failed to wait image ready - swapchain error!");
			// NEXT: Terminate application.
		}

		// Submit the current command buffer for execution on the graphics queue
		vtek::SubmitInfo submitInfo{};
		vtek::swapchain_fill_queue_submit_info(swapchain, &submitInfo);
		if (!vtek::queue_submit(graphicsQueue, commandBuffers[frameIndex], &submitInfo))
		{
			log_error("Failed to submit to queue!");
			// TODO: This is an error.
		}

		// Wait for command buffer to finish execution, and present frame to screen.
		auto presentStatus = vtek::swapchain_present_frame(swapchain, frameIndex);
		if (presentStatus == vtek::SwapchainStatus::outofdate)
		{
			log_error("Failed to present frame - swapchain outofdate!");
			// TODO: Rebuild swapchain
			// NOTE: Swapchain _may_ indeed change length!
		}
		else if (presentStatus == vtek::SwapchainStatus::error)
		{
			log_error("Failed to present frame - swapchain error!");
			// NEXT: Terminate application.
		}
	}


	// Cleanup
	vtek::device_wait_idle(device);

	vtek::graphics_pipeline_destroy(graphicsPipeline, device);
	vtek::buffer_destroy(buffer);
	vtek::graphics_shader_destroy(shader, device);
	vtek::swapchain_destroy(swapchain, device);
	vtek::command_pool_destroy(graphicsCommandPool, device);
	vtek::device_destroy(device);
	vtek::physical_device_release(physicalDevice);
	vtek::window_surface_destroy(surface, instance);
	vtek::instance_destroy(instance);
	vtek::window_destroy(window);

	log_info("Program Success!");
	vtek::terminate();

	return 0;
}
