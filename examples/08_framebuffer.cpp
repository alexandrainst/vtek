#include <vtek/vtek.hpp>
#include <iostream>

/* global data */
vtek::ApplicationWindow* gWindow = nullptr;
uint32_t gFramebufferWidth = 0U;
uint32_t gFramebufferHeight = 0U;
vtek::KeyboardMap gKeyboardMap;
glm::vec2 gMoveOffset {0.0f, 0.0f};

/* input handling */
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
	constexpr float moveSpeed = 0.05f;

	// left / right
	if (gKeyboardMap.get_key(KeyboardKey::left)) {
		gMoveOffset.x -= moveSpeed;
	}
	else if (gKeyboardMap.get_key(KeyboardKey::right)) {
		gMoveOffset.x += moveSpeed;
	}

	// up / down
	if (gKeyboardMap.get_key(KeyboardKey::up)) {
		gMoveOffset.y -= moveSpeed;
	}
	else if (gKeyboardMap.get_key(KeyboardKey::down)) {
		gMoveOffset.y += moveSpeed;
	}
}

bool update_uniform(vtek::Buffer* buffer, vtek::Device* device)
{
	vtek::Uniform_v2 uniform {.v2 = gMoveOffset};
	vtek::BufferRegion region {.offset = 0, .size = uniform.size()};

	if (!vtek::buffer_write_data(buffer, &uniform, &region, device))
	{
		log_error("Failed to write data to the uniform buffer!");
		return false;
	}

	return true;
}

struct RenderingInfo
{
	std::vector<vtek::CommandBuffer*> commandBuffers;
	std::vector<vtek::Framebuffer*> framebuffers;
	vtek::GraphicsPipeline* mainPipeline {nullptr};
	vtek::GraphicsPipeline* quadPipeline {nullptr};
	vtek::Swapchain* swapchain {nullptr};
	glm::uvec2 windowSize {0U, 0U};
};

bool recordCommandBuffer(
	RenderingInfo* info, uint32_t imageIndex,
	vtek::Model* model, vtek::DescriptorSet* descriptorSet)
{
	auto commandBuffer = info->commandBuffers[imageIndex];
	auto framebuffer = info->framebuffers[imageIndex];

	// 1) begin recording on the command buffer
	vtek::CommandBufferBeginInfo beginInfo{};
	beginInfo.oneTimeSubmit = true;
	if (!vtek::command_buffer_begin(commandBuffer, &beginInfo))
	{
		log_error("Failed to begin command buffer {} recording!", imageIndex);
		return false;
	}

	// 2) begin dynamic rendering on the framebuffer
	if (!vtek::framebuffer_dynrender_begin(framebuffer, commandBuffer))
	{
		log_error("Failed to begin dynamic rendering on framebuffer!");
		return false;
	}

	// 3) bind pipeline and set dynamic states
	vtek::cmd_bind_graphics_pipeline(commandBuffer, info->mainPipeline);

	vtek::cmd_set_viewport_scissor(commandBuffer, info->windowSize, {0.0f, 0.0f});

	// 4) bind and render each model, including descriptor sets and push constants

	// 5) end dynamic rendering on the framebuffer
	vtek::framebuffer_dynrender_end(framebuffer, commandBuffer);

	// 6) begin dynamic rendering on the swapchain
	glm::vec3 clearColor(0.15f, 0.15f, 0.15f);
	vtek::swapchain_dynamic_rendering_begin(
		info->swapchain, imageIndex, commandBuffer, clearColor);

	// 7) bind framebuffer targets for reading (descriptor sets?)
	vtek::PushConstant_m4 pc{};
	pc.m1 = glm::mat4(1.0f);
	vtek::EnumBitmask<vtek::ShaderStageGraphics> pcStages =
		vtek::ShaderStageGraphics::vertex;
	vtek::cmd_push_constant_graphics(
		commandBuffer, info->mainPipeline, &pc, pcStages);

	// 8) draw fullscreen quad

	// 9) end dynamic rendering on the swapchain
	vtek::swapchain_dynamic_rendering_end(
		info->swapchain, imageIndex, commandBuffer);

	// 10) end recording on the command buffer
	if (!vtek::command_buffer_end(commandBuffer))
	{
		log_error("Failed to end command buffer {} recording!", imageIndex);
		return false;
	}

	return true;
}



int main()
{
	// Initialize vtek
	vtek::InitInfo initInfo{};
	initInfo.disableLogging = false;
	initInfo.applicationTitle = "07_textured_model";
	initInfo.useGLFW = true;
	if (!vtek::initialize(&initInfo))
	{
		std::cerr << "Failed to initialize vtek!" << std::endl;
		return -1;
	}

	// Keyboard map
	gKeyboardMap.reset();

	// Create window
	vtek::WindowInfo windowInfo{};
	windowInfo.title = "vtek example 07: Drawing a textured model";
	windowInfo.resizeable = false;
	windowInfo.cursorDisabled = true;
	windowInfo.width = 1024;
	windowInfo.height = 1024;
	windowInfo.fullscreen = false;
	gWindow = vtek::window_create(&windowInfo);
	if (gWindow == nullptr)
	{
		log_error("Failed to create window!");
		return -1;
	}
	vtek::window_set_key_handler(gWindow, key_callback);

	// Vulkan instance
	vtek::InstanceInfo instanceInfo{};
	instanceInfo.applicationName = "07_textured_model";
	instanceInfo.applicationVersion = vtek::VulkanVersion(1, 0, 0); // TODO: vtek::AppVersion
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
	physicalDeviceInfo.requiredFeatures.depthBounds = true;
	physicalDeviceInfo.requiredFeatures.fillModeNonSolid = true;
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

	// Window size, for swapchain, camera, and other things.
	glm::uvec2 windowSize;
	vtek::window_get_framebuffer_size(gWindow, &windowSize.x, &windowSize.y);
	gFramebufferWidth = windowSize.x;
	gFramebufferHeight = windowSize.y;

	// Swapchain
	vtek::SwapchainInfo swapchainInfo{};
	swapchainInfo.vsync = true;
	swapchainInfo.prioritizeLowLatency = false;
	swapchainInfo.framebufferWidth = gFramebufferWidth;
	swapchainInfo.framebufferHeight = gFramebufferHeight;
	// NOTE: Depth buffering is handled by the framebuffer!
	swapchainInfo.depthBuffer = vtek::SwapchainDepthBuffer::none;
	vtek::Swapchain* swapchain = vtek::swapchain_create(
		&swapchainInfo, surface, physicalDevice, device);
	if (swapchain == nullptr)
	{
		log_error("Failed to create swapchain!");
		return -1;
	}

	// Framebuffer attachment formats
	std::vector<vtek::Format> prioritizedColorFormats = {
		vtek::Format::r8g8b8_unorm,
		vtek::Format::r8g8b8a8_unorm
	};
	vtek::FormatInfo framebufferFormatInfo{};
	framebufferFormatInfo.tiling = vtek::ImageTiling::optimal;
	framebufferFormatInfo.features
		= vtek::FormatFeature::sampled_image
		| vtek::FormatFeature::color_attachment_blend; // TODO: With/without blend?
	vtek::SupportedFormat colorFormat;
	if (!vtek::SupportedFormat::FindFormat(
		    &framebufferFormatInfo, prioritizedColorFormats,
		    device, colorFormat))
	{
		log_error("Failed to find a supported framebuffer color format!");
		return -1;
	}
	std::vector<vtek::Format> prioritizedDepthStencilFormats = {
		vtek::Format::d24_unorm_s8_uint,
		vtek::Format::d32_sfloat,
		vtek::Format::d32_sfloat_s8_uint
	};
	framebufferFormatInfo.features
		= vtek::FormatFeature::depth_stencil_attachment;
	vtek::SupportedFormat depthStencilFormat;
	if (!vtek::SupportedFormat::FindFormat(
		    &framebufferFormatInfo, prioritizedDepthStencilFormats,
		    device, depthStencilFormat))
	{
		log_error("Failed to find a supported framebuffer depth/stencil format!");
		return -1;
	}

	// Framebuffers
	vtek::FramebufferAttachmentInfo colorAttachment{};
	colorAttachment.supportedFormat = colorFormat;
	colorAttachment.clearValue.setColorFloat(0.2f, 0.2f, 0.2f, 1.0f);
	vtek::FramebufferAttachmentInfo depthStencilAttachment{};
	depthStencilAttachment.supportedFormat = depthStencilFormat;
	depthStencilAttachment.clearValue.setDepthStencil();
	vtek::FramebufferInfo framebufferInfo{};
	framebufferInfo.colorAttachments.emplace_back(colorAttachment);
	framebufferInfo.depthStencilAttachment = depthStencilAttachment;
	framebufferInfo.useDepthStencil = true;
	framebufferInfo.resolution = windowSize;
	 // TODO: When multisampling implemented in vtek, try it out?
	framebufferInfo.multisampling = vtek::MultisampleType::none;
	framebufferInfo.renderPass = nullptr;
	framebufferInfo.useDynamicRendering = true;

	// Multiple framebuffers, same number as max frames in flight!
	std::vector<vtek::Framebuffer*> framebuffers = vtek::framebuffer_create(
		&framebufferInfo, vtek::kMaxFramesInFlight, device);
	if (framebuffers.empty() || framebuffers.size() != vtek::kMaxFramesInFlight)
	{
		log_error("Failed to create framebuffer!");
		return -1;
	}
	for (auto fb : framebuffers)
	{
		if (!vtek::framebuffer_dynamic_rendering_only(fb)) {
			log_error("Framebuffer not properly created for dynamic rendering!");
			return -1;
		}
		for (vtek::Format fmt : vtek::framebuffer_get_color_formats(fb)) {
			if (fmt == vtek::Format::undefined)
			{
				log_error("Framebuffer color attachment has undefined format!");
				return -1;
			}
		}
		if (vtek::framebuffer_get_depth_stencil_format(fb) ==
		    vtek::Format::undefined) {
			log_error("Framebuffer depth/stencil attachment has undefined format!");
			return -1;
		}
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

	// Shader for main render pass
	const char* shaderdirstrMain = "../shaders/08_framebuffer/main/";
	vtek::Directory* shaderdirMain = vtek::directory_open(shaderdirstrMain);
	if (shaderdirMain == nullptr)
	{
		log_error("Failed to open shader directory (main)!");
		return -1;
	}
	vtek::GraphicsShaderInfo shaderInfo{};
	shaderInfo.vertex = true;
	shaderInfo.fragment = true;
	vtek::GraphicsShader* shaderMain =
		vtek::graphics_shader_load_spirv(&shaderInfo, shaderdirMain, device);
	if (shaderMain == nullptr)
	{
		log_error("Failed to load graphics shader (main)!");
		return -1;
	}

	// Shader for fullscreen quad
	const char* shaderdirstrQuad = "../shaders/08_framebuffer/quad/";
	vtek::Directory* shaderdirQuad = vtek::directory_open(shaderdirstrQuad);
	if (shaderdirQuad == nullptr)
	{
		log_error("Failed to open shader directory (quad)!");
		return -1;
	}
	vtek::GraphicsShader* shaderQuad =
		vtek::graphics_shader_load_spirv(&shaderInfo, shaderdirQuad, device);
	if (shaderQuad == nullptr)
	{
		log_error("Failed to load graphics shader (quad)!");
		return -1;
	}

	// Descriptor pool
	vtek::DescriptorPoolInfo descriptorPoolInfo{};
	descriptorPoolInfo.allowIndividualFree = false; // TODO: Needed?
	descriptorPoolInfo.allowUpdateAfterBind = false;
	descriptorPoolInfo.descriptorTypes = {
		{ vtek::DescriptorType::combined_image_sampler, 8 },
		{ vtek::DescriptorType::sampled_image, 8 } // TODO: ?
	};
	vtek::DescriptorPool* descriptorPool = vtek::descriptor_pool_create(
		&descriptorPoolInfo, device);
	if (descriptorPool == nullptr)
	{
		log_error("Failed to create descriptor pool!");
		return -1;
	}

	// Command buffers
	// TODO: Length same as max frames in flight instead of swapchain length?
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

	// Descriptor set (main)
	// TODO: Multiple sets "alloc_sets", for each swapchain image?
	// NOTE: Count should be for kMaxFramesInFlight !!
	vtek::DescriptorSet* descriptorSetLayoutMain =
		vtek::descriptor_pool_alloc_set();

	// Descriptor set (quad)
	// TODO: Multiple sets "alloc_sets", for each swapchain image?
	// NOTE: Count should be for kMaxFramesInFlight !!

	// Graphics pipeline for main render pass
	vtek::ViewportState viewport{
		.viewportRegion = {
			.offset = {0U, 0U},
			.extent = {gFramebufferWidth, gFramebufferHeight}
		},
	};
	// Buffer bindings: Vertex, Normal, TexCoord
	vtek::VertexBufferBindings bindings{};
	bindings.add_buffer(
		vtek::VertexAttributeType::vec3, vtek::VertexInputRate::per_vertex);
	bindings.add_buffer(
		vtek::VertexAttributeType::vec3, vtek::VertexInputRate::per_vertex);
	bindings.add_buffer(
		vtek::VertexAttributeType::vec2, vtek::VertexInputRate::per_vertex);
	vtek::RasterizationState rasterizer{};
	rasterizer.cullMode = vtek::CullMode::none; // enable back-face culling
	rasterizer.polygonMode = vtek::PolygonMode::fill;
	rasterizer.frontFace = vtek::FrontFace::clockwise;
	vtek::MultisampleState multisampling{};
	vtek::DepthStencilState depthStencil{};
	depthStencil.depthTestEnable = true;
	depthStencil.depthWriteEnable = true;
	vtek::ColorBlendState colorBlending{};
	colorBlending.attachments.emplace_back(
		vtek::ColorBlendAttachment::GetDefault());
	vtek::PipelineRendering pipelineRendering{};
	pipelineRendering.colorAttachmentFormats =
		vtek::framebuffer_get_color_formats(framebuffers[0]);
	pipelineRendering.depthAttachmentFormat =
		vtek::framebuffer_get_depth_stencil_format(framebuffers[0]);

	vtek::GraphicsPipelineInfo pipelineInfo{};
	pipelineInfo.renderPassType = vtek::RenderPassType::dynamic;
	pipelineInfo.renderPass = nullptr; // Nice!
	pipelineInfo.pipelineRendering = &pipelineRendering;
	pipelineInfo.shader = shaderMain;
	// TODO: Rename to `vertexBufferBindings`
	pipelineInfo.vertexInputBindings = &bindings;
	pipelineInfo.primitiveTopology =
		vtek::PrimitiveTopology::triangle_list;
	pipelineInfo.enablePrimitiveRestart = false;
	pipelineInfo.viewportState = &viewport;
	pipelineInfo.rasterizationState = &rasterizer;
	pipelineInfo.multisampleState = &multisampling;
	pipelineInfo.depthStencilState = &depthStencil;
	pipelineInfo.colorBlendState = &colorBlending;
	pipelineInfo.dynamicStateFlags
		= vtek::PipelineDynamicState::viewport
		| vtek::PipelineDynamicState::scissor;
	// TODO: Create the descriptor set!
	pipelineInfo.descriptorSetLayouts.push_back(descriptorSetLayoutMain);
	pipelineInfo.pushConstantType = vtek::PushConstantType::mat4;
	pipelineInfo.pushConstantShaderStages = vtek::ShaderStageGraphics::vertex;

	vtek::GraphicsPipeline* mainPipeline =
		vtek::graphics_pipeline_create(&pipelineInfo, device);
	if (mainPipeline == nullptr)
	{
		log_error("Failed to create graphics pipeline (main)!");
		return -1;
	}

	// Graphics pipeline for fullscreen quad
	vtek::VertexBufferBindings quadBindings{};
	quadBindings.add_buffer(
		vtek::VertexAttributeType::vec2, vtek::VertexAttributeType::vec2,
		vtek::VertexInputRate::per_vertex);
	depthStencil.depthTestEnable = false;
	depthStencil.depthWriteEnable = false;
	pipelineRendering.colorAttachmentFormats.push_back(
		vtek::swapchain_get_image_format(swapchain));
	pipelineRendering.depthAttachmentFormat = vtek::Format::undefined;
	pipelineInfo.shader = shaderQuad;
	pipelineInfo.vertexInputBindings = &quadBindings;
	pipelineInfo.dynamicStateFlags = {}; // reset
	// TODO: Create the descriptor set!
	pipelineInfo.descriptorSetLayout.push_back(descriptorSetLayoutQuad);
	pipelineInfo.pushConstantType = vtek::PushConstantType::none;
	pipelineInfo.pushConstantShaderStages = {}; // reset
	vtek::GraphicsPipeline* quadPipeline =
		vtek::graphics_pipeline_create(&pipelineInfo, device);
	if (quadPipeline == nullptr)
	{
		log_error("Failed to create graphics pipeline (quad)!");
		return -1;
	}



	// Error tolerance
	int errors = 10;

	while (vtek::window_get_should_close(gWindow) && errors > 0)
	{
		// React to incoming input events
		vtek::window_poll_events();

		// To avoid excessive GPU work we wait until we may begin the frame
		auto beginStatus = vtek::swapchain_wait_begin_frame(swapchain, device);
		if (beginStatus == vtek::SwapchainStatus::timeout)
		{
			log_error("Failed to wait begin frame - swapchain timeout!");
			errors--; continue;
		}
		else if (beginStatus == vtek::SwapchainStatus::error)
		{
			log_error("Failed to wait begin frame - swapchain error!");
			errors = -1; continue;
		}

		// Acquire the next available image in the swapchain
		uint32_t imageIndex;
		auto acquireStatus =
			vtek::swapchain_acquire_next_image(swapchain, device, &imageIndex);
		if (acquireStatus == vtek::SwapchainStatus::outofdate)
		{
			log_error("Failed to acquire image - swapchain outofdate!");
			// TODO: Rebuild swapchain
			// NOTE: Swapchain _may_ indeed change length!
		}
		else if (acquireStatus == vtek::SwapchainStatus::error)
		{
			log_error("Failed to acquire image - swapchain error!");
			errors = -1; continue;
		}

		// Successfully acquired image, now update uniforms before rendering
		// TODO: Update after bind no longer needed!
		update_movement();
		vtek::camera_update(gCamera);
		if (!update_camera_uniform(uniformBufferCamera, device))
		{
			log_error("Failed to update uniform buffer (Camera)!");
			return -1;
		}
		// if (gLightChanged)
		// {
		// 	if (!update_light_uniform(descriptorSetLight, uniformBufferLight, device))
		// 	{
		// 		log_error("Failed to update uniform buffer (Light)!");
		// 		return -1;
		// 	}
		// 	gLightChanged = false;
		// }

		// Record command buffer for next frame
		vtek::CommandBuffer* commandBuffer = commandBuffers[imageIndex];
		if (!vtek::command_pool_reset_buffer(graphicsCommandPool, commandBuffer))
		{
			log_error("Failed to reset command buffer prior to re-recording it!");
			errors--; continue;
		}
		vtek::GraphicsPipeline* nextPipeline =
			(gRenderWireframe) ? pipelineWireframe : pipeline;
		bool record = recordCommandBuffer(
			commandBuffer, nextPipeline, swapchain, imageIndex, model,
			descriptorSet);
		if (!record)
		{
			log_error("Failed to record command buffer!");
			errors--; continue;
		}

		// Wait until any previous operations are finished using this image, for either read or write.
		// NOTE: We can do command buffer recording or other operations before calling this function.
		auto readyStatus =
			vtek::swapchain_wait_image_ready(swapchain, device, imageIndex);
		if (readyStatus == vtek::SwapchainStatus::timeout)
		{
			log_error("Failed to wait image ready - swapchain timeout!");
			errors--; continue;
		}
		else if (readyStatus == vtek::SwapchainStatus::error)
		{
			log_error("Failed to wait image ready - swapchain error!");
			errors = -1; continue;
		}

		// Submit the current command buffer for execution on the graphics queue
		vtek::SubmitInfo submitInfo{};
		vtek::swapchain_fill_queue_submit_info(swapchain, &submitInfo);
		if (!vtek::queue_submit(
			    graphicsQueue, commandBuffers[imageIndex], &submitInfo))
		{
			log_error("Failed to submit to queue!");
			errors--; continue;
		}

		// Wait for command buffer to finish execution, and present frame to screen.
		auto presentStatus = vtek::swapchain_present_frame(swapchain, imageIndex);
		if (presentStatus == vtek::SwapchainStatus::outofdate)
		{
			log_error("Failed to present frame - swapchain outofdate!");
			// TODO: Rebuild swapchain
			// NOTE: Swapchain _may_ indeed change length!
		}
		else if (presentStatus == vtek::SwapchainStatus::error)
		{
			log_error("Failed to present frame - swapchain error!");
			errors = -1; continue;
		}

		// NOTE: This may be adjusted based on machine CPU speed.. (no timer created yet!)
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}



	// Cleanup
	vtek::device_wait_idle(device);

	vtek::graphics_pipeline_destroy(pipelineWireframe, device);
	vtek::graphics_pipeline_destroy(pipeline, device);
	vtek::buffer_destroy(uniformBufferCamera);
	vtek::descriptor_set_layout_destroy(descriptorSetLayout, device);
	vtek::descriptor_pool_destroy(descriptorPool, device);
	vtek::sampler_destroy(sampler, device);
	vtek::image2d_destroy(texture, device);
	vtek::model_destroy(model, device);
	vtek::graphics_shader_destroy(shader, device);
	vtek::camera_destroy(gCamera);
	vtek::command_pool_free_buffers(graphicsCommandPool, commandBuffers, device);
	vtek::command_pool_destroy(graphicsCommandPool, device);
	vtek::framebuffer_destroy(framebuffers, device);
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
