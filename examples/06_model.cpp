#include <vtek/vtek.hpp>
#include <iostream>

/* global data */
vtek::ApplicationWindow* gWindow = nullptr;
uint32_t gFramebufferWidth = 0U;
uint32_t gFramebufferHeight = 0U;
vtek::KeyboardMap gKeyboardMap;
vtek::Camera* gCamera = nullptr;
vtek::Uniform_m4 gUniform;


/* Input handling */
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

		// NOTE: Un-comment for checking correct angular displacement.
		// case KeyboardKey::q:
		// 	vtek::camera_roll_left_radians(gCamera, glm::pi<float>() * 0.25f);
		// 	break;
		// case KeyboardKey::e:
		// 	vtek::camera_roll_right_radians(gCamera, glm::pi<float>() * 0.25f);
		// 	break;

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


/* Helper functions */
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
	gUniform.m4 = *(vtek::camera_get_projection_matrix(gCamera));
	gUniform.m4 *= *(vtek::camera_get_view_matrix(gCamera));

	// Update uniform buffer
	vtek::BufferRegion region{
		.offset = 0,
		.size = gUniform.size()
	};
	if (!vtek::buffer_write_data(buffer, &gUniform, &region, device))
	{
		log_error("Failed to write data to the uniform buffer!");
		return false;
	}

	// Update descriptor set
	if (!vtek::descriptor_set_bind_uniform_buffer(
		    set, 0, buffer, gUniform.type()))
	{
		log_error("Failed to add uniform buffer to the descriptor set!");
		return false;
	}
	vtek::descriptor_set_update(set, device);

	return true;
}

bool record_command_buffers(
	vtek::GraphicsPipeline* pipeline,
	std::vector<vtek::CommandBuffer*> commandBuffers, vtek::Swapchain* swapchain,
	vtek::Buffer* vertexBuffer, vtek::DescriptorSet* descriptorSet)
{
	VkPipeline pipl = vtek::graphics_pipeline_get_handle(pipeline);
	VkPipelineLayout pipLayout = vtek::graphics_pipeline_get_layout(pipeline);
	uint32_t commandBufferCount = commandBuffers.size();

	for (uint32_t i = 0; i < commandBufferCount; i++)
	{
		vtek::CommandBuffer* commandBuffer = commandBuffers[i];
		VkCommandBuffer cmdBuf = vtek::command_buffer_get_handle(commandBuffer);

		if (!vtek::command_buffer_begin(commandBuffer))
		{
			log_error("Failed to begin command buffer {} recording!", i);
			return false;
		}

		glm::vec3 clearColor(0.3f, 0.3f, 0.3f);
		// TODO: Ensure the swapchain has a depth buffer (or several!)
		vtek::swapchain_dynamic_rendering_begin(
			swapchain, i, commandBuffer, clearColor);

		vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipl);

		// TODO: Bind vertex buffer for model
		VkBuffer buffers[1] = { vtek::buffer_get_handle(vertexBuffer) };
		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(cmdBuf, 0, 1, buffers, offsets);

		// TODO: Bind descriptor set for the pipeline:
		// TODO: m4 push constant, m4 uniform
		VkDescriptorSet descriptorSets[1] = {
			vtek::descriptor_set_get_handle(descriptorSet)
		};
		vkCmdBindDescriptorSets(
			cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipLayout, 0, 1,
			descriptorSets, 0, nullptr);

		// Push constant for the model, ie. transformation matrix
		vtek::PushConstant_m4 pc{};
		pc.m1 = glm::mat4(1.0f); // unit matrix
		pc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // TODO: Hide away! (?)
		pc.cmdPush(cmdBuf, pipLayout);

		// Draw the model
		vkCmdDraw(cmdBuf, gCubeVertices.size(), 1, 0, 0);

		vtek::swapchain_dynamic_rendering_end(swapchain, i, commandBuffer);

		if (!vtek::command_buffer_end(commandBuffer))
		{
			log_error("Failed to end command buffer {} recording!", i);
			return false;
		}
	}

	return true;
}

// TODO: Remove when done.
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





int main()
{
	// Initialize vtek
	vtek::InitInfo initInfo{};
	initInfo.disableLogging = false;
	initInfo.applicationTitle = "06_model";
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
	windowInfo.title = "vtek example 06: Model loading";
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
	instanceInfo.applicationName = "06_model";
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
	vtek::window_get_framebuffer_size(
		gWindow, &gFramebufferWidth, &gFramebufferHeight);

	// Swapchain
	vtek::SwapchainInfo swapchainInfo{};
	swapchainInfo.vsync = true;
	swapchainInfo.prioritizeLowLatency = false;
	swapchainInfo.framebufferWidth = gFramebufferWidth;
	swapchainInfo.framebufferHeight = gFramebufferHeight;
	swapchainInfo.createDepthBuffers = true; // TODO: Yes, we want this!
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

	// Camera
	gCamera = vtek::camera_create();
	glm::vec3 camPos {8.0f, 0.0f, 0.0f};
	glm::vec3 camFront {-1.0f, 0.0f, 0.0f};
	glm::vec3 camUp {0.0f, 0.0f, 1.0f};
	vtek::camera_set_lookat(gCamera, camPos, camFront, camUp);
	vtek::camera_set_window_size(gCamera, gFramebufferWidth, gFramebufferHeight);
	vtek::camera_set_perspective_frustrum(gCamera, 45.0f, 0.1f, 100.0f);
	vtek::camera_update(gCamera);
	// TODO: Maybe for this application, use FPS-game style camera instead?
	// TODO: It's also a good opportunity to test if the camera supports it properly

	// Shader
	const char* shaderdirstr = "../shaders/06_simple_model/";
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

	// Model
	const char* modeldirstr = "../models/";
	vtek::Directory* modeldir = vtek::directory_open(modeldirstr);
	if (modeldir == nullptr)
	{
		log_error("Failed to open model directory!");
		return -1;
	}
	vtek::Model* model = vtek::model_load_obj(modeldir, "armored_car.obj", device);
	if (model == nullptr)
	{
		log_error("Failed to load obj model!");
		return -1;
	}

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

	// Descriptor pool
	vtek::DescriptorPoolInfo descriptorPoolInfo{};
	descriptorPoolInfo.allowUpdateAfterBind = true;
	descriptorPoolInfo.descriptorTypes.push_back(
		{ vtek::DescriptorType::uniform_buffer, 2 }); // NOTE: Leave place for light
	vtek::DescriptorPool* descriptorPool =
		vtek::descriptor_pool_create(&descriptorPoolInfo, device);
	if (descriptorPool == nullptr)
	{
		log_error("Failed to create descriptor pool!");
		return -1;
	}

	// Descriptor set layout (Camera transform)
	vtek::DescriptorSetLayoutInfo descriptorLayoutInfoCamera{};
	vtek::DescriptorLayoutBinding descriptorBindingCamera{};
	descriptorBindingCamera.type = vtek::DescriptorType::uniform_buffer;
	descriptorBindingCamera.binding = 0;
	descriptorBindingCamera.shaderStages = vtek::ShaderStage::vertex;
	descriptorBindingCamera.updateAfterBind = true;
	descriptorLayoutInfoCamera.bindings.emplace_back(descriptorBindingCamera);
	vtek::DescriptorSetLayout* descriptorSetLayoutCamera =
		vtek::descriptor_set_layout_create(&descriptorLayoutInfoCamera, device);
	if (descriptorSetLayoutCamera == nullptr)
	{
		log_error("Failed to create descriptor set layout (Camera)!");
		return -1;
	}

	// Descriptor set layout (Point light)
	vtek::DescriptorSetLayoutInfo descriptorLayoutInfoLight{};
	vtek::DescriptorLayoutBinding descriptorBindingLight{};
	descriptorBindingLight.type = vtek::DescriptorType::uniform_buffer;
	descriptorBindingLight.binding = 1;
	descriptorBindingLight.shaderStages = vtek::ShaderStage::fragment;
	descriptorBindingLight.updateAfterBind = true;
	descriptorLayoutInfoLight.bindings.emplace_back(descriptorBindingLight);
	vtek::DescriptorSetLayout* descriptorSetLayoutLight =
		vtek::descriptor_set_layout_create(&descriptorLayoutInfoLight, device);
	if (descriptorSetLayoutLight == nullptr)
	{
		log_error("Failed to create descriptor set layout (Light)!");
		return -1;
	}

	// Descriptor set (Camera transform)
	// TODO: descriptor_pool_alloc_set(pool, layout, device) - ??
	// TODO: This is similar to the improved command buffer interface!
	vtek::DescriptorSet* descriptorSetCamera = vtek::descriptor_set_create(
		descriptorPool, descriptorSetLayoutCamera, device);
	if (descriptorSetCamera == nullptr)
	{
		log_error("Failed to create descriptor set (Camera)!");
		return -1;
	}

	// Descriptor set (Point light)
	// TODO: descriptor_pool_alloc_set(pool, layout, device) - ??
	// TODO: This is similar to the improved command buffer interface!
	vtek::DescriptorSet* descriptorSetLight = vtek::descriptor_set_create(
		descriptorPool, descriptorSetLayoutLight, device);
	if (descriptorSetLight == nullptr)
	{
		log_error("Failed to create descriptor set (Light)!");
		return -1;
	}

	// Graphics pipeline
	vtek::ViewportState viewport{
		.viewportRegion = {
			.offset = {0U, 0U},
			.extent = {windowSize.x, windowSize.y}
		},
	};
	vtek::VertexBufferBindings bindings{};
	bindings.add_buffer(
		vtek::VertexAttributeType::vec3, vtek::VertexInputRate::per_vertex);
	// TODO: Also add vertex normal
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
	graphicsPipelineInfo.descriptorSetLayouts.push_back(descriptorSetLayoutCamera);
	graphicsPipelineInfo.descriptorSetLayouts.push_back(descriptorSetLayoutLight);
	graphicsPipelineInfo.pushConstantType = vtek::PushConstantType::mat4;
	graphicsPipelineInfo.pushConstantShaderStages =
		vtek::ShaderStageGraphics::vertex;

	vtek::GraphicsPipeline* graphicsPipeline = vtek::graphics_pipeline_create(
		&graphicsPipelineInfo, device);
	if (graphicsPipeline == nullptr)
	{
		log_error("Failed to create graphics pipeline!");
		return -1;
	}

	// Command buffer recording
	if (!record_command_buffers(
		    graphicsPipeline, commandBuffers, swapchain, vertexBuffer, descriptorSet))
	{
		log_error("Failed to record command buffers!");
		return -1;
	}





	// Cleanup
	vtek::device_wait_idle(device);



	vtek::model_destroy(model); // TODO: Device handle
	vtek::graphics_shader_destroy(shader, device);
	vtek::camera_destroy(gCamera);
	vtek::command_pool_free_buffers(graphicsCommandPool, commandBuffers, device);
	vtek::command_pool_destroy(graphicsCommandPool, device);
	vtek::swapchain_destroy(swapchain, device);
	vtek::device_destroy(device);
	vtek::physical_device_release(physicalDevice);
	vtek::window_surface_destroy(surface, instance);
	vtek::instance_destroy(instance);
	vtek::window_destroy(gWindow);

	log_info("Program Success!");
	vtek::terminate();

	return 0;
}
