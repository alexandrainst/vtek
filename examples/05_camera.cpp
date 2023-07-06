#include <vtek/vtek.hpp>
#include <iostream>
#include <vector>

// global data
vtek::ApplicationWindow* gWindow = nullptr;
uint32_t gFramebufferWidth = 0U;
uint32_t gFramebufferHeight = 0U;
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


vtek::Buffer* create_vertex_buffer(vtek::Device* device)
{
	std::vector<float> cubeVertices = {
		// front
		-0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f, 0.5f, -0.5f,  0.5f, 0.5f, -0.5f, -0.5f,
		// left
		-0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f, -0.5f,
		// back
		0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  0.5f, -0.5f, 0.5f,  0.5f,
		0.5f, 0.5f, -0.5f, -0.5f, 0.5f,  0.5f, -0.5f, 0.5f, -0.5f,
		// right
		0.5f, -0.5f, -0.5f, 0.5f, -0.5f,  0.5f, 0.5f,  0.5f,  0.5f,
		0.5f, -0.5f, -0.5f, 0.5f,  0.5f,  0.5f, 0.5f,  0.5f, -0.5f,
		// top
		-0.5f, -0.5f, 0.5f, -0.5f,  0.5f, 0.5f, 0.5f,  0.5f, 0.5f,
		-0.5f, -0.5f, 0.5f, 0.5f,  0.5f, 0.5f, 0.5f, -0.5f, 0.5f,
		// bottom
		-0.5f,  0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f,  0.5f, -0.5f,
	};

	vtek::BufferInfo info{};
	info.size = sizeof(float) * cubeVertices.size();
	info.writePolicy = vtek::BufferWritePolicy::write_once;

	using BUFlag = vtek::BufferUsageFlag;
	info.usageFlags = BUFlag::transfer_dst | BUFlag::vertex_buffer;

	vtek::Buffer* buffer = vtek::buffer_create(&info, device);
	if (buffer == nullptr)
	{
		log_error("Failed to create vertex buffer!");
		return nullptr;
	}

	vtek::BufferRegion region {
		.offset = 0,
		.size = info.size
	};
	if (!vtek::buffer_write_data(buffer, cubeVertices.data(), &region, device))
	{
		log_error("Failed to write data to vertex buffer!");
		vtek::buffer_destroy(buffer);
		return nullptr;
	}

	return buffer;
}

bool update_uniform_buffer(
	vtek::DescriptorSet* set, vtek::Buffer* buffer, vtek::Device* device)
{
	// Wait until the device is finished with all operations
	// NOTE: This is needed because even with "updateAfterBind" enabled,
	// it is still not valid to update a bound descriptor set if the command
	// buffer is pending execution. Exempt to this rule is only updating
	// descriptors inside the set which are not used by the command buffer,
	// but that is a subtle nuance and likely not needed for most purposes.
	vtek::device_wait_idle(device);

	// Packed data
	uniform.v3 = {circleCenter.x, circleCenter.y, circleRadius};

	// Update uniform buffer
	vtek::BufferRegion region{
		.offset = 0,
		.size = uniform.size()
	};
	if (!vtek::buffer_write_data(buffer, &uniform, &region, device))
	{
		log_error("Failed to write data to the uniform buffer!");
		return false;
	}

	// Update descriptor set
	if (!vtek::descriptor_set_bind_uniform_buffer(
		    set, 0, buffer, uniform.type()))
	{
		log_error("Failed to add uniform buffer to the descriptor set!");
		return false;
	}
	vtek::descriptor_set_update(set, device);

	return true;
}




bool record_command_buffers(
	vtek::GraphicsPipeline* pipeline, vtek::Queue* queue,
	std::vector<vtek::CommandBuffer*> commandBuffers, vtek::Swapchain* swapchain,
	vtek::Buffer* vertexBuffer, vtek::DescriptorSet* descriptorSet)
{
	VkPipeline pipl = vtek::graphics_pipeline_get_handle(pipeline);
	VkPipelineLayout pipLayout = vtek::graphics_pipeline_get_layout(pipeline);
	uint32_t queueIndex = vtek::queue_get_family_index(queue);
	uint32_t commandBufferCount = commandBuffers.size();
	VkExtent2D swapchainExtent = vtek::swapchain_get_image_extent(swapchain);
	const uint32_t width = swapchainExtent.width;
	const uint32_t height = swapchainExtent.height;

	for (uint32_t i = 0; i < commandBufferCount; i++)
	{
		vtek::CommandBuffer* commandBuffer = commandBuffers[i];
		VkCommandBuffer cmdBuf = vtek::command_buffer_get_handle(commandBuffer);

		if (!vtek::command_buffer_begin(commandBuffer))
		{
			log_error("Failed to begin command buffer {} recording!", i);
			return false;
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
		VkBuffer buffers[1] = { vtek::buffer_get_handle(vertexBuffer) };
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(cmdBuf, 0, 1, buffers, offsets);

		// Bind descriptor set
		VkDescriptorSet descriptorSets[1] = {
			vtek::descriptor_set_get_handle(descriptorSet)
		};
		vkCmdBindDescriptorSets(
			cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipLayout, 0, 1,
			descriptorSets, 0, nullptr);

		// TODO: Push constant for model matrix ?

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
			return false;
		}
	}

	return true;
}




int main()
{
	// Initialize vtek
	vtek::InitInfo initInfo{};
	initInfo.disableLogging = false;
	initInfo.applicationTitle = "05_camera";
	initInfo.useGLFW = true;
	if (!vtek::initialize(&initInfo))
	{
		std::cerr << "Failed to initialize vtek!" << std::endl;
		return -1;
	}

	// Keyboard map
	gKeyboardMap.reset();

	// Create window
	vtek::WindowCreateInfo windowInfo{};
	windowInfo.title = "vtek example 05: Camera";
	windowInfo.resizeable = false;
	window = vtek::window_create(&windowInfo);
	if (window == nullptr)
	{
		log_error("Failed to create window!");
		return -1;
	}
	vtek::window_set_key_handler(window, key_callback);

	// Vulkan instance
	vtek::InstanceCreateInfo instanceInfo{};
	instanceInfo.applicationName = "camera";
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
	// Allows us to change the uniform without re-recording the command buffers:
	physicalDeviceInfo.updateAfterBindFeatures
		= vtek::UpdateAfterBindFeature::uniform_buffer;
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
	vtek::window_get_framebuffer_size(
		window, &swapchainInfo.framebufferWidth, &swapchainInfo.framebufferHeight);
	vtek::Swapchain* swapchain = vtek::swapchain_create(
		&swapchainInfo, surface, physicalDevice, device);
	if (swapchain == nullptr)
	{
		log_error("Failed to create swapchain!");
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

	// Shader
	const char* shaderdirstr = "../shaders/simple_camera/";
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

	// Descriptor pool
	vtek::DescriptorPoolInfo descriptorPoolInfo{};
	descriptorPoolInfo.allowUpdateAfterBind = true;
	descriptorPoolInfo.descriptorTypes.push_back(
		{ vtek::DescriptorType::uniform_buffer, 1 });
	vtek::DescriptorPool* descriptorPool =
		vtek::descriptor_pool_create(&descriptorPoolInfo, device);
	if (descriptorPool == nullptr)
	{
		log_error("Failed to create descriptor pool!");
		return -1;
	}

	// Descriptor set layout
	vtek::DescriptorSetLayoutInfo descriptorLayoutInfo{};
	vtek::DescriptorLayoutBinding descriptorBinding{};
	descriptorBinding.type = vtek::DescriptorType::uniform_buffer;
	descriptorBinding.binding = 0;
	descriptorBinding.shaderStages = vtek::ShaderStage::vertex;
	descriptorBinding.updateAfterBind = true;
	descriptorLayoutInfo.bindings.emplace_back(descriptorBinding);
	vtek::DescriptorSetLayout* descriptorSetLayout =
		vtek::descriptor_set_layout_create(&descriptorLayoutInfo, device);
	if (descriptorSetLayout == nullptr)
	{
		log_error("Failed to create descriptor set layout!");
		return -1;
	}

	// Descriptor set
	// TODO: descriptor_pool_alloc_set(pool, layout, device) - ??
	// TODO: This is similar to the improved command buffer interface!
	vtek::DescriptorSet* descriptorSet = vtek::descriptor_set_create(
		descriptorPool, descriptorSetLayout, device);
	if (descriptorSet == nullptr)
	{
		log_error("Failed to create descriptor set!");
		return -1;
	}

	// Vertex buffer
	vtek::Buffer* vertexBuffer = create_vertex_buffer(device);
	if (vertexBuffer == nullptr)
	{
		return -1;
	}

	// Camera

	// Uniform buffer
	vtek::BufferInfo uniformBufferInfo{};
	uniformBufferInfo.size = uniform.size();
	uniformBufferInfo.requireHostVisibleStorage = true; // TODO: Test without!
	uniformBufferInfo.disallowInternalStagingBuffer = true;
	uniformBufferInfo.writePolicy = vtek::BufferWritePolicy::overwrite_often;
	uniformBufferInfo.usageFlags
		= vtek::BufferUsageFlag::transfer_dst
		| vtek::BufferUsageFlag::uniform_buffer;
	vtek::Buffer* uniformBuffer = vtek::buffer_create(&uniformBufferInfo, device);
	if (uniformBuffer == nullptr)
	{
		log_error("Failed to create uniform buffer!");
		return -1;
	}
	if (!update_uniform_buffer(descriptorSet, uniformBuffer, device))
	{
		log_error("Failed to fill uniform buffer!");
		return -1;
	}






	return 0;
}
