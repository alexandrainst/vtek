#include <vtek/vtek.hpp>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

// global data
vtek::ApplicationWindow* gWindow = nullptr;
uint32_t gFramebufferWidth = 0U;
uint32_t gFramebufferHeight = 0U;
vtek::KeyboardMap gKeyboardMap;
vtek::Camera* gCamera = nullptr;
vtek::Uniform_m4 uniform;

std::vector<float> gCubeVertices = {
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



// helper functions
void key_callback(vtek::KeyboardKey key, vtek::InputAction action)
{
	using vtek::KeyboardKey;

	if (action == vtek::InputAction::press)
	{
		switch (key)
		{
		case KeyboardKey::escape:
			vtek::window_set_should_close(gWindow, true);
			break;
		case KeyboardKey::p:
			{
				glm::vec3 pos = vtek::camera_get_position(gCamera);
				glm::vec3 front = vtek::camera_get_front(gCamera);
				glm::vec3 up = vtek::camera_get_up(gCamera);

				log_debug("[Camera] pos=({},{},{}), front=({},{},{}), up=({},{},{})",
				          pos.x, pos.y, pos.z, front.x, front.y, front.z, up.x, up.y, up.z);
			}
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

void mouse_move_callback(double x, double y)
{
	vtek::camera_on_mouse_move(gCamera, x, y);
}

void update_movement()
{
	using vtek::KeyboardKey;

	// left / right
	if (gKeyboardMap.get_key(KeyboardKey::a)) {
		vtek::camera_move_right(gCamera, 0.1f);
	}
	else if (gKeyboardMap.get_key(KeyboardKey::d)) {
		vtek::camera_move_left(gCamera, 0.1f);
	}

	// forward / backward
	if (gKeyboardMap.get_key(KeyboardKey::w)) {
		vtek::camera_move_forward(gCamera, 0.1f);
	}
	else if (gKeyboardMap.get_key(KeyboardKey::s)) {
		vtek::camera_move_backward(gCamera, 0.1f);
	}

	// up / down
	if (gKeyboardMap.get_key(KeyboardKey::space)) {
		vtek::camera_move_up(gCamera, 0.1f);
	}
	else if (gKeyboardMap.get_key(KeyboardKey::c)) {
		vtek::camera_move_down(gCamera, 0.1f);
	}

	// rotate
	if (gKeyboardMap.get_key(KeyboardKey::q)) {
		vtek::camera_roll_left_radians(gCamera, 0.05f);
	}
	else if (gKeyboardMap.get_key(KeyboardKey::e)) {
		vtek::camera_roll_right_radians(gCamera, 0.05f);
	}
}


vtek::Buffer* create_vertex_buffer(vtek::Device* device)
{
	vtek::BufferInfo info{};
	info.size = sizeof(float) * gCubeVertices.size();
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
	if (!vtek::buffer_write_data(buffer, gCubeVertices.data(), &region, device))
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
	uniform.m4 = *(vtek::camera_get_projection_matrix(gCamera));
	uniform.m4 *= *(vtek::camera_get_view_matrix(gCamera));

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
		vkCmdDraw(cmdBuf, gCubeVertices.size(), 1, 0, 0);

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
	windowInfo.cursorDisabled = true;
	gWindow = vtek::window_create(&windowInfo);
	if (gWindow == nullptr)
	{
		log_error("Failed to create window!");
		return -1;
	}
	vtek::window_set_key_handler(gWindow, key_callback);
	vtek::window_set_mouse_move_handler(gWindow, mouse_move_callback);

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

	// Window size, for swapchain, camera, and other things.
	glm::uvec2 windowSize;
	vtek::window_get_framebuffer_size(gWindow, &windowSize.x, &windowSize.y);

	// Swapchain
	vtek::SwapchainInfo swapchainInfo{};
	swapchainInfo.vsync = true;
	swapchainInfo.prioritizeLowLatency = false;
	swapchainInfo.framebufferWidth = windowSize.x;
	swapchainInfo.framebufferHeight = windowSize.y;
	vtek::Swapchain* swapchain = vtek::swapchain_create(
		&swapchainInfo, surface, physicalDevice, device);
	if (swapchain == nullptr)
	{
		log_error("Failed to create swapchain!");
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

	// Command buffers
	const uint32_t commandBufferCount = vtek::swapchain_get_length(swapchain);
	std::vector<vtek::CommandBuffer*> commandBuffers =
		vtek::command_pool_alloc_buffers(
			graphicsCommandPool, vtek::CommandBufferUsage::primary,
			commandBufferCount, device);
	if (commandBuffers.empty())
	{
		log_error("Failed to create command buffers!");
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
	gCamera = vtek::camera_create();
	//vtek::camera_set_z_up(gCamera);
	//vtek::camera_set_position(gCamera, glm::vec3(-3.0f, 0.0f, 0.0f));

	//vtek::camera_set_up(gCamera, glm::vec3(0.0f, 0.0f, 1.0f));
	//vtek::camera_set_front(gCamera, glm::vec3(1.0f, 0.0f, 0.0f));
	//vtek::camera_set_orientation_degrees(gCamera, 180.0f, 0.0f);
	glm::vec3 camPos {0.0f, 0.0f, 0.0f};
	glm::vec3 camFront {1.0f, 0.0f, 0.0f};
	glm::vec3 camUp {0.0f, 0.0f, 1.0f};
	vtek::camera_set_lookat(gCamera, camPos, camFront, camUp);

	vtek::camera_set_window_size(gCamera, windowSize.x, windowSize.y);
	vtek::camera_set_perspective_frustrum(gCamera, 45.0f, 0.1f, 100.0f);
	vtek::camera_update(gCamera);

	// Uniform buffer
	vtek::BufferInfo uniformBufferInfo{};
	uniformBufferInfo.size = uniform.size();
	uniformBufferInfo.requireHostVisibleStorage = true;
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

	// Vulkan graphics pipeline
	vtek::ViewportState viewport{
		.viewportRegion = {
			.offset = {0U, 0U},
			.extent = {windowSize.x, windowSize.y}
		},
	};
	vtek::VertexBufferBindings bindings{};
	bindings.add_buffer(
		vtek::VertexAttributeType::vec3, vtek::VertexInputRate::per_vertex);
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
	graphicsPipelineInfo.primitiveTopology =
		vtek::PrimitiveTopology::triangle_list;
	graphicsPipelineInfo.enablePrimitiveRestart = false;
	graphicsPipelineInfo.viewportState = &viewport;
	graphicsPipelineInfo.rasterizationState = &rasterizer;
	graphicsPipelineInfo.multisampleState = &multisampling;
	graphicsPipelineInfo.depthStencilState = &depthStencil;
	graphicsPipelineInfo.colorBlendState = &colorBlending;
	graphicsPipelineInfo.descriptorSetLayouts.push_back(descriptorSetLayout);

	// TODO: Push constant for the model matrix

	vtek::GraphicsPipeline* graphicsPipeline = vtek::graphics_pipeline_create(
		&graphicsPipelineInfo, device);
	if (graphicsPipeline == nullptr)
	{
		log_error("Failed to create graphics pipeline!");
		return -1;
	}

	// Command buffer recording
	if (!record_command_buffers(
		    graphicsPipeline, graphicsQueue, commandBuffers,
		    swapchain, vertexBuffer, descriptorSet))
	{
		log_error("Failed to record command buffers!");
		return -1;
	}




	// Error tolerance
	int errors = 10;

	while (vtek::window_get_should_close(gWindow) && errors > 0)
	{
		// React to incoming input events
		vtek::window_poll_events();

		update_movement();
		vtek::camera_update(gCamera);
		if (!update_uniform_buffer(descriptorSet, uniformBuffer, device))
		{
			log_error("Failed to update uniform buffer!");
			return -1;
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
			// NEXT: Terminate application.
		}

		// Acquire the next available image in the swapchain
		uint32_t frameIndex;
		auto acquireStatus =
			vtek::swapchain_acquire_next_image(swapchain, device, &frameIndex);
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
		auto readyStatus =
			vtek::swapchain_wait_image_ready(swapchain, device, frameIndex);
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
		if (!vtek::queue_submit(
			    graphicsQueue, commandBuffers[frameIndex], &submitInfo))
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

		// NOTE: This may be adjusted based on machine CPU speed.. (no timer created yet!)
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}




	// Cleanup
	vtek::device_wait_idle(device);

	vtek::graphics_pipeline_destroy(graphicsPipeline, device);
	vtek::buffer_destroy(uniformBuffer);
	vtek::camera_destroy(gCamera);
	vtek::buffer_destroy(vertexBuffer);
	vtek::descriptor_set_destroy(descriptorSet);
	vtek::descriptor_set_layout_destroy(descriptorSetLayout, device);
	vtek::descriptor_pool_destroy(descriptorPool, device);
	vtek::graphics_shader_destroy(shader, device);
	vtek::swapchain_destroy(swapchain, device);
	vtek::command_pool_free_buffers(graphicsCommandPool, commandBuffers, device);
	vtek::command_pool_destroy(graphicsCommandPool, device);
	vtek::device_destroy(device);
	vtek::physical_device_release(physicalDevice);
	vtek::window_surface_destroy(surface, instance);
	vtek::instance_destroy(instance);
	vtek::window_destroy(gWindow);

	log_info("Program Success!");
	vtek::terminate();

	return 0;
}
