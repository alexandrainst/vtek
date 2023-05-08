#include <vtek/vtek.hpp>
#include <iostream>

// global data
vtek::ApplicationWindow* window = nullptr;

// helper functions
void keyCallback(vtek::KeyboardKey key, vtek::InputAction action)
{
	if (action == vtek::InputAction::press)
	{

	}
	else if (action == vtek::InputAction::release)
	{
		switch (key)
		{
		case vtek::KeyboardKey::escape:
			vtek::window_set_should_close(window, true);
			break;
		default:
			break;
		}
	}
}

void framebufferResizedCallback()
{

}

void recreateSwapchain(vtek::Device* device, vtek::Swapchain* swapchain)
{
	// 1) window minimization guard
	vtek::window_wait_while_minimized(window);

	// 2) device wait idle
	vtek::device_wait_idle(device);

	// 3) recreate swapchain
	vtek::swapchain_recreate(swapchain);
}

bool recordCommandBuffer(
	vtek::CommandBuffer* buffer, vtek::GraphicsPipeline* graphicsPipeline,
	VkImage swapchainImage, VkImageView swapchainImageView, uint32_t queueIndex)
{
	VkCommandBuffer cmdBuf = vtek::command_buffer_get_handle(commandBuffer);
	VkPipeline pipl = vtek::graphics_pipeline_get_handle(graphicsPipeline);

	if (!vtek::command_buffer_begin(commandBuffer))
	{
		log_error("Failed to begin command buffer recording!", i);
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
	renderingInfo.renderArea = { 0U, 0U, width, height };
	renderingInfo.layerCount = 1;
	renderingInfo.viewMask = 0;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments = &colorAttachmentInfo;
	renderingInfo.pDepthAttachment = nullptr;
	renderingInfo.pStencilAttachment = nullptr;

	vkCmdBeginRendering(cmdBuf, &renderingInfo);

	vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipl);

	// TODO: Viewport dynamic state

	// TODO: Scissor dynamic state

	// TODO: Movement offset push constants

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

	// Create window
	vtek::WindowCreateInfo windowInfo{};
	windowInfo.title = "02triangle_plain";
	windowInfo.width = 500;
	windowInfo.height = 500;
	windowInfo.maximized = false;
	windowInfo.resizeable = true;
	windowInfo.decorated = true;
	windowInfo.cursorDisabled = false;
	window = vtek::window_create(&windowInfo);
	if (window == nullptr)
	{
		log_error("Failed to create window!");
		return -1;
	}
	vtek::window_set_key_handler(window, keyCallback);

	// Vulkan instance
	vtek::InstanceCreateInfo instanceInfo{};
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
		window, &swapchainCreateInfo.framebufferWidth,
		&swapchainCreateInfo.framebufferHeight);
	vtek::Swapchain* swapchain = vtek::swapchain_create(
		&swapchainCreateInfo, surface, physicalDevice, device);
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
	const uint32_t width = swapchainCreateInfo.framebufferWidth;
	const uint32_t height = swapchainCreateInfo.framebufferHeight;
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

	vtek::GraphicsPipelineCreateInfo graphicsPipelineInfo{
		.renderPassType = vtek::RenderPassType::dynamic,
		.renderPass = nullptr, // Nice!
		.pipelineRendering = &pipelineRendering,
		.shader = shader,
		.vertexInputType = vtek::VertexType::vec2,
		.instancedRendering = false,
		.primitiveTopology = vtek::PrimitiveTopology::triangle_list,
		.enablePrimitiveRestart = false,
		.viewportState = &viewport,
		.rasterizationState = &rasterizer,
		.multisampleState = &multisampling,
		.depthStencilState = &depthStencil,
		.colorBlendState = &colorBlending,
		.dynamicStateFlags = 0U
	};
	// graphicsPipelineInfo.dynamicStateFlags |= vtek::PipelineDynamicState::viewport;
	// graphicsPipelineInfo.dynamicStateFlags |= vtek::PipelineDynamicState::scissor;



	return 0;
}
