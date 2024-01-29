#include <vtek/vtek.hpp>
#include <iostream>

// global data
vtek::ApplicationWindow* gWindow = nullptr;
uint32_t gFramebufferWidth = 0U;
uint32_t gFramebufferHeight = 0U;
glm::vec2 gMoveOffset = glm::vec2(0.0f, 0.0f);
float gRotateAngle = 0.0f;
const float gMoveSpeed = 0.02f;
const float gRotateSpeed = glm::two_pi<float>() * 0.005f;
vtek::KeyboardMap gKeyboardMap;

// helper functions
void keyCallback(vtek::KeyboardKey key, vtek::InputAction action)
{
	using vtek::KeyboardKey;

	if (action == vtek::InputAction::press)
	{
		switch (key)
		{
		case KeyboardKey::escape:
			vtek::window_set_should_close(gWindow, true);
			break;
		default:
			gKeyboardMap.press_key(key);
			break;
		}
	}
	else if (action == vtek::InputAction::release)
	{
		gKeyboardMap.release_key(key);
	}
}

void update_movement()
{
	using vtek::KeyboardKey;

	// left / right
	if (gKeyboardMap.get_key(KeyboardKey::left)) {
		gMoveOffset.x -= gMoveSpeed;
	}
	else if (gKeyboardMap.get_key(KeyboardKey::right)) {
		gMoveOffset.x += gMoveSpeed;
	}

	// up / down
	if (gKeyboardMap.get_key(KeyboardKey::up)) {
		gMoveOffset.y -= gMoveSpeed;
	}
	else if (gKeyboardMap.get_key(KeyboardKey::down)) {
		gMoveOffset.y += gMoveSpeed;
	}

	// rotate
	if (gKeyboardMap.get_key(KeyboardKey::z)) {
		gRotateAngle += gRotateSpeed;
	}
	else if (gKeyboardMap.get_key(KeyboardKey::x)) {
		gRotateAngle -= gRotateSpeed;
	}
}

bool recreateSwapchain(
	vtek::Device* device, vtek::Swapchain* swapchain, VkSurfaceKHR surface)
{
	// 1) window minimization guard
	vtek::window_wait_while_minimized(gWindow);

	// 2) device wait idle
	vtek::device_wait_idle(device);

	// 3) recreate swapchain
	vtek::window_get_framebuffer_size(
		gWindow, &gFramebufferWidth, &gFramebufferHeight);

	if (!vtek::swapchain_recreate(
		    swapchain, device, surface, gFramebufferWidth, gFramebufferHeight))
	{
		log_error("Failed to recreate swapchain!");
		return false;
	}

	return true;
}

bool recordCommandBuffer(
	vtek::CommandBuffer* commandBuffer, vtek::GraphicsPipeline* graphicsPipeline,
	VkImage swapchainImage, VkImageView swapchainImageView, uint32_t queueIndex)
{
	VkCommandBuffer cmdBuf = vtek::command_buffer_get_handle(commandBuffer);
	VkPipeline pipl = vtek::graphics_pipeline_get_handle(graphicsPipeline);
	VkPipelineLayout pipLayout = vtek::graphics_pipeline_get_layout(graphicsPipeline);

	vtek::CommandBufferBeginInfo beginInfo{};
	if (!vtek::command_buffer_begin(commandBuffer, &beginInfo))
	{
		log_error("Failed to begin command buffer recording!");
		return false;
	}

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
		.image = swapchainImage,
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
		.imageView = swapchainImageView,
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
	renderingInfo.renderArea = { 0U, 0U, gFramebufferWidth, gFramebufferHeight };
	renderingInfo.layerCount = 1;
	renderingInfo.viewMask = 0;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachmentInfo;
	renderingInfo.pDepthAttachment = nullptr;
	renderingInfo.pStencilAttachment = nullptr;

	vkCmdBeginRendering(cmdBuf, &renderingInfo);

	vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipl);

	// Viewport dynamic state
	VkViewport viewport{};
	viewport.x = 0.0f; // upper-left corner
	viewport.y = 0.0f; // upper-left corner
	viewport.width = (float)gFramebufferWidth;
	viewport.height = (float)gFramebufferHeight;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 0.0f;
	vkCmdSetViewport(cmdBuf, 0, 1, &viewport);

	// Scissor dynamic state
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = { gFramebufferWidth, gFramebufferHeight };
	vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

	// Movement offset push constants
	vtek::PushConstant_v3 pc{};
	pc.v1 = glm::vec3(gMoveOffset.x, gMoveOffset.y, gRotateAngle);
	vtek::cmd_push_constant_graphics(
		commandBuffer, graphicsPipeline, &pc,
		vtek::ShaderStageGraphics::vertex | vtek::ShaderStageGraphics::fragment);

	vkCmdDraw(cmdBuf, 3, 1, 0, 0);

	vkCmdEndRendering(cmdBuf);

	// Transition from color attachment to present src
	// PROG: We can extract function from this pipeline barrier
	//       (vtek::dynamic_rendering_insert_barrier(), etc.).
	VkImageMemoryBarrier endBarrier{};
	endBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	endBarrier.pNext = nullptr;
	endBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	endBarrier.dstAccessMask = 0;
	endBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	endBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	endBarrier.srcQueueFamilyIndex = queueIndex;
	endBarrier.dstQueueFamilyIndex = queueIndex;
	endBarrier.image = swapchainImage,
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
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr,
		0, nullptr, 1, &endBarrier);

	if (!vtek::command_buffer_end(commandBuffer))
	{
		log_error("Failed to end command buffer recording!");
		return false;
	}

	return true;
}



int main()
{
	// Initialize vtek
	vtek::InitInfo initInfo{};
	initInfo.disableLogging = false;
	initInfo.applicationTitle = "triangle_move";
	initInfo.useGLFW = true;
	initInfo.loadShadersFromGLSL = false;
	if (!vtek::initialize(&initInfo))
	{
		std::cerr << "Failed to initialize vtek!" << std::endl;
		return -1;
	}

	// Keyboard map
	gKeyboardMap.reset();

	// Create window
	vtek::WindowInfo windowInfo{};
	windowInfo.title = "02triangle_plain";
	windowInfo.width = 500;
	windowInfo.height = 500;
	windowInfo.maximized = false;
	windowInfo.resizeable = true;
	windowInfo.decorated = true;
	windowInfo.cursorDisabled = false;
	gWindow = vtek::window_create(&windowInfo);
	if (gWindow == nullptr)
	{
		log_error("Failed to create window!");
		return -1;
	}
	vtek::window_get_framebuffer_size(
		gWindow, &gFramebufferWidth, &gFramebufferHeight);
	vtek::window_set_key_handler(gWindow, keyCallback);

	// Vulkan instance
	vtek::InstanceInfo instanceInfo{};
	instanceInfo.applicationName = "triangle_move";
	instanceInfo.applicationVersion = vtek::VulkanVersion(1, 0, 0);
	instanceInfo.enableValidationLayers = true;
	auto instance = vtek::instance_create(&instanceInfo);
	if (instance == nullptr)
	{
		log_error("Failed to create Vulkan instance!");
		return -1;
	}

	// Surface
	VkSurfaceKHR surface = vtek::window_create_surface(gWindow, instance);
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
	vtek::DeviceInfo deviceInfo{};
	vtek::Device* device = vtek::device_create(
		&deviceInfo, instance, physicalDevice);
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
	commandPoolInfo.allowIndividualBufferReset = true;
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
	swapchainInfo.framebufferWidth = gFramebufferWidth;
	swapchainInfo.framebufferHeight = gFramebufferHeight;
	vtek::Swapchain* swapchain = vtek::swapchain_create(
		&swapchainInfo, surface, physicalDevice, device);
	if (swapchain == nullptr)
	{
		log_error("Failed to create swapchain!");
		return -1;
	}

	// Shader
	const char* shaderdirstr = "../shaders/simple_triangle_move/";
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

	// Vulkan graphics pipeline
	const uint32_t width = swapchainInfo.framebufferWidth;
	const uint32_t height = swapchainInfo.framebufferHeight;
	vtek::ViewportState viewport{};

	vtek::RasterizationState rasterizer{};
	vtek::MultisampleState multisampling{};
	vtek::DepthStencilState depthStencil{}; // No depth testing!
	vtek::ColorBlendState colorBlending{};
	colorBlending.attachments.emplace_back(
		vtek::ColorBlendAttachment::GetDefault());
	vtek::PipelineRendering pipelineRendering{};
	pipelineRendering.colorAttachmentFormats.push_back(
		vtek::swapchain_get_image_format(swapchain));

	vtek::GraphicsPipelineInfo graphicsPipelineInfo{};
	graphicsPipelineInfo.renderPassType = vtek::RenderPassType::dynamic;
	graphicsPipelineInfo.renderPass = nullptr; // Nice!
	graphicsPipelineInfo.pipelineRendering = &pipelineRendering;
	graphicsPipelineInfo.shader = shader;
	graphicsPipelineInfo.primitiveTopology = vtek::PrimitiveTopology::triangle_list;
	graphicsPipelineInfo.enablePrimitiveRestart = false;
	graphicsPipelineInfo.viewportState = &viewport;
	graphicsPipelineInfo.rasterizationState = &rasterizer;
	graphicsPipelineInfo.multisampleState = &multisampling;
	graphicsPipelineInfo.depthStencilState = &depthStencil;
	graphicsPipelineInfo.colorBlendState = &colorBlending;
	graphicsPipelineInfo.dynamicStateFlags
		= vtek::PipelineDynamicState::viewport
		| vtek::PipelineDynamicState::scissor;
	graphicsPipelineInfo.pushConstantType = vtek::PushConstantType::vec3;
	graphicsPipelineInfo.pushConstantShaderStages
		= vtek::ShaderStageGraphics::vertex
		| vtek::ShaderStageGraphics::fragment;

	vtek::GraphicsPipeline* graphicsPipeline = vtek::graphics_pipeline_create(
		&graphicsPipelineInfo, device);
	if (graphicsPipeline == nullptr)
	{
		log_error("Failed to create graphics pipeline!");
		return -1;
	}

	// Command buffers
	const uint32_t commandBufferCount = vtek::swapchain_get_length(swapchain);
	std::vector<vtek::CommandBuffer*> commandBuffers =
		vtek::command_pool_alloc_buffers(
			graphicsCommandPool, vtek::CommandBufferUsage::primary,
			commandBufferCount, device);
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

	// Error tolerance
	int errors = 10;

	while (vtek::window_get_should_close(gWindow) && errors > 0)
	{
		// React to incoming input events
		vtek::window_poll_events();
		update_movement();

		// Check if framebuffer has been resized.
		if (vtek::window_is_resizing(gWindow))
		{
			vtek::window_wait_while_resizing(gWindow);

			if (!recreateSwapchain(device, swapchain, surface))
			{
				log_error("Failed to re-create swapchain!");
				errors = 0;
				continue;
			}
		}

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
			errors = 0;
			continue;
		}

		// Acquire the next available image in the swapchain
		uint32_t frameIndex;
		auto acquireStatus = vtek::swapchain_acquire_next_image(
			swapchain, device, &frameIndex);
		if (acquireStatus == vtek::SwapchainStatus::outofdate)
		{
			log_debug("Failed to acquire image - swapchain outofdate!");
			continue;
		}
		else if (acquireStatus == vtek::SwapchainStatus::error)
		{
			log_error("Failed to acquire image - swapchain error!");
			errors = 0;
			continue;
		}

		// Record command buffer for next frame
		vtek::CommandBuffer* commandBuffer = commandBuffers[frameIndex];
		bool record = recordCommandBuffer(
			commandBuffer, graphicsPipeline,
			vtek::swapchain_get_image(swapchain, frameIndex),
			vtek::swapchain_get_image_view(swapchain, frameIndex),
			vtek::queue_get_family_index(graphicsQueue));
		if (!record)
		{
			log_error("Failed to record command buffer!");
			errors--; continue;
		}

		// Wait until any previous operations are finished using this image.
		auto readyStatus = vtek::swapchain_wait_image_ready(
			swapchain, device, frameIndex);
		if (readyStatus == vtek::SwapchainStatus::timeout)
		{
			log_error("Failed to wait image ready - swapchain timeout!");
			// TODO: Probably log an error and then run the loop again.
			continue;
		}
		else if (readyStatus == vtek::SwapchainStatus::error)
		{
			log_error("Failed to wait image ready - swapchain error!");
			errors = 0;
			continue;
		}

		// Submit the current command buffer for execution on the graphics queue
		vtek::SubmitInfo submitInfo{};
		vtek::swapchain_fill_queue_submit_info(swapchain, &submitInfo);
		if (!vtek::queue_submit(
			    graphicsQueue, commandBuffers[frameIndex], &submitInfo))
		{
			log_error("Failed to submit to queue!");
			errors = 0;
			continue;
		}

		// Wait for command buffer to finish execution, and present frame to screen.
		auto presentStatus = vtek::swapchain_present_frame(swapchain, frameIndex);
		if (presentStatus == vtek::SwapchainStatus::outofdate)
		{
			log_debug("Failed to present frame - swapchain outofdate!");
			continue;
		}
		else if (presentStatus == vtek::SwapchainStatus::error)
		{
			log_error("Failed to present frame - swapchain error!");
			errors = 0;
			continue;
		}
	}


	// Cleanup
	vtek::device_wait_idle(device);

	vtek::graphics_pipeline_destroy(graphicsPipeline, device);
	vtek::graphics_shader_destroy(shader, device);
	vtek::swapchain_destroy(swapchain, device);
	vtek::command_pool_free_buffers(graphicsCommandPool, commandBuffers, device);
	vtek::command_pool_destroy(graphicsCommandPool, device);
	vtek::device_destroy(device);
	vtek::physical_device_release(physicalDevice);
	vtek::window_surface_destroy(surface, instance);
	vtek::instance_destroy(instance);
	vtek::window_destroy(gWindow);
	gWindow = nullptr;

	log_debug("All went well!");
	vtek::terminate();
	return 0;
}
