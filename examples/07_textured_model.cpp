#include <vtek/vtek.hpp>
#include <iostream>

/* global data */
vtek::ApplicationWindow* gWindow = nullptr;
uint32_t gFramebufferWidth = 0U;
uint32_t gFramebufferHeight = 0U;
vtek::KeyboardMap gKeyboardMap;
vtek::Camera* gCamera = nullptr;
vtek::Uniform_m4_v4 gCameraUniform;
vtek::FloatClamp<0.01f, 10.0f> gCameraExposure = 1.05f;
// TODO: Implement pipeline derivation!
// TODO: Depth testing switch by depth function as dynamic pipeline state!
bool gRenderWireframe = false;
vtek::CullMode gCullMode = vtek::CullMode::none; // default: back-face culling


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
		case KeyboardKey::p:
			gRenderWireframe ^= true; // XOR-toggle
			break;
		case KeyboardKey::b:
			gCullMode = (gCullMode == vtek::CullMode::back)
				? vtek::CullMode::none : vtek::CullMode::back;
			break;
		case KeyboardKey::o: // TODO: Temporary to find good start orientation!
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
	constexpr float moveSpeed = 0.05f;
	constexpr float rotateSpeed = 0.025f;

	// left / right
	if (gKeyboardMap.get_key(KeyboardKey::a)) {
		vtek::camera_move_right(gCamera, moveSpeed);
	}
	else if (gKeyboardMap.get_key(KeyboardKey::d)) {
		vtek::camera_move_left(gCamera, moveSpeed);
	}

	// forward / backward
	if (gKeyboardMap.get_key(KeyboardKey::w)) {
		vtek::camera_move_forward(gCamera, moveSpeed);
	}
	else if (gKeyboardMap.get_key(KeyboardKey::s)) {
		vtek::camera_move_backward(gCamera, moveSpeed);
	}

	// up / down
	if (gKeyboardMap.get_key(KeyboardKey::space)) {
		vtek::camera_move_up(gCamera, moveSpeed);
	}
	else if (gKeyboardMap.get_key(KeyboardKey::c)) {
		vtek::camera_move_down(gCamera, moveSpeed);
	}

	// rotate
	if (gKeyboardMap.get_key(KeyboardKey::q)) {
		vtek::camera_roll_left_radians(gCamera, rotateSpeed);
	}
	else if (gKeyboardMap.get_key(KeyboardKey::e)) {
		vtek::camera_roll_right_radians(gCamera, rotateSpeed);
	}

	// Adjust camera exposure
	if (gKeyboardMap.get_key(KeyboardKey::numpad_add)) {
		gCameraExposure += 0.01f;
	}
	else if (gKeyboardMap.get_key(KeyboardKey::numpad_subtract)) {
		gCameraExposure -= 0.01f;
	}
}

/* helper functions */
bool update_camera_uniform(
	vtek::Buffer* buffer, vtek::Device* device)
{
	// Wait until the device is finished with all operations
	vtek::device_wait_idle(device);

	// Packed data
	gCameraUniform.m4 = *(vtek::camera_get_projection_matrix(gCamera));
	gCameraUniform.m4 *= *(vtek::camera_get_view_matrix(gCamera));
	gCameraUniform.v4 = glm::vec4(
		vtek::camera_get_position(gCamera), gCameraExposure.get());

	// Update uniform buffer
	vtek::BufferRegion region{
		.offset = 0,
		.size = gCameraUniform.size()
	};
	if (!vtek::buffer_write_data(buffer, &gCameraUniform, &region, device))
	{
		log_error("Failed to write data to the uniform buffer!");
		return false;
	}

	return true;
}

bool update_descriptor_set(
	vtek::DescriptorSet* set, vtek::Buffer* buffer, vtek::Image2D* image,
	vtek::Sampler* sampler, vtek::Device* device)
{
	// Update descriptor set
	if (!vtek::descriptor_set_bind_uniform_buffer(
		    set, 0, buffer, gCameraUniform.type()))
	{
		log_error("Failed to add uniform buffer to the descriptor set!");
		return false;
	}
	// TODO: We might not need to update the sampler since it hasn't changed!! ?
	if (!vtek::descriptor_set_bind_combined_image2d_sampler(
		    set, 1, sampler, image, vtek::ImageLayout::shader_readonly_optimal))
	{
		log_error("Failed to add combined image sampler to the descriptor set!");
		return false;
	}
	vtek::descriptor_set_update(set, device);

	return true;
}

bool recordCommandBuffer(
	vtek::CommandBuffer* commandBuffer, vtek::GraphicsPipeline* pipeline,
	vtek::Swapchain* swapchain, uint32_t imageIndex,
	vtek::Model* model, vtek::DescriptorSet* descriptorSet)
{
	VkPipeline pipl = vtek::graphics_pipeline_get_handle(pipeline);
	VkPipelineLayout pipLayout = vtek::graphics_pipeline_get_layout(pipeline);
	VkCommandBuffer cmdBuf = vtek::command_buffer_get_handle(commandBuffer);

	if (!vtek::command_buffer_begin(commandBuffer))
	{
		log_error("Failed to begin command buffer {} recording!", imageIndex);
		return false;
	}

	glm::vec3 clearColor(0.15f, 0.15f, 0.15f);
	vtek::swapchain_dynamic_rendering_begin(
		swapchain, imageIndex, commandBuffer, clearColor);

	// TODO: Can we make all this shorter and better?

	vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipl);

	// Dynamic state
	vkCmdSetCullMode(cmdBuf, vtek::get_cull_mode(gCullMode));

	// Bind stuff for model
	VkBuffer buffers[3] = {
		vtek::buffer_get_handle(vtek::model_get_vertex_buffer(model)),
		vtek::buffer_get_handle(vtek::model_get_normal_buffer(model)),
		vtek::buffer_get_handle(vtek::model_get_texcoord_buffer(model))
	};
	VkDeviceSize offsets[3] = { 0, 0, 0 };
	vkCmdBindVertexBuffers(cmdBuf, 0, 3, buffers, offsets);

	VkDescriptorSet descrSet = vtek::descriptor_set_get_handle(descriptorSet);
	vkCmdBindDescriptorSets(
		cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipLayout, 0, 1,
		&descrSet, 0, nullptr); // NOTE: Dynamic offset unused

	// Push constant for the model, ie. transformation matrix
	vtek::PushConstant_m4 pc{};
	pc.m1 = glm::mat4(1.0f); // unit matrix
	pc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // TODO: Hide away! (?)
	pc.cmdPush(cmdBuf, pipLayout);

	// Draw the model
	uint32_t numVertices = vtek::model_get_num_vertices(model);
	vkCmdDraw(cmdBuf, numVertices, 1, 0, 0);

	vtek::swapchain_dynamic_rendering_end(swapchain, imageIndex, commandBuffer);

	if (!vtek::command_buffer_end(commandBuffer))
	{
		log_error("Failed to end command buffer {} recording!", imageIndex);
		return false;
	}

	return true;
}


/* main */
int main()
{
	// TODO: Too much confusion btw. application title, window title,
	// and instance application title. Eliminate!

	// TODO: Consider renaming `vtek::InitInfo::applicationTitle` to
	// shortAppTitle, or similar.

	// TODO: Create a directory relative to "Executable directory" for
	// asset loading! This will make sure that the application may be
	// run from any directory!
	// TODO: And propagate this change to all prior example programs!

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
	vtek::WindowCreateInfo windowInfo{};
	windowInfo.title = "vtek example 07: Drawing a textured model";
	windowInfo.resizeable = false;
	windowInfo.cursorDisabled = true;
	windowInfo.width = 1024;
	windowInfo.height = 1024;
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
	swapchainInfo.depthBuffer = vtek::SwapchainDepthBuffer::single_shared;
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
	// TODO: Bug in camera!
	// glm::vec3 camPos {-2.0968692f, -2.5813563f, -1.4253441f}; // {8.0f, 0.0f, 0.0f};
	// glm::vec3 camFront {0.5990349f, 0.7475561f, 0.28690946f}; // {-1.0f, 0.0f, 0.0f};
	// glm::vec3 camUp {-0.18743359f, -0.21744627f, 0.95790696f}; // {0.0f, 0.0f, 1.0f};
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
	const char* shaderdirstr = "../shaders/07_textured_model/";
	// TODO: `directory_open` might check if the directory is relative,
	// and in such case open it from executable directory instead of root! (?)
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
	const char* modeldirstr = "../models/viking_room/";
	// TODO: `directory_open` might check if the directory is relative,
	// and in such case open it from executable directory instead of root! (?)
	vtek::Directory* modeldir = vtek::directory_open(modeldirstr);
	if (modeldir == nullptr)
	{
		log_error("Failed to open model directory!");
		return -1;
	}
	vtek::ModelInfo modelInfo{};
	modelInfo.keepVertexDataInMemory = false; // Preferred usage
	modelInfo.loadNormals = true;
	modelInfo.loadTextureCoordinates = true;
	modelInfo.flipUVs = true;
	vtek::Model* model = vtek::model_load_obj(
		&modelInfo, modeldir, "viking_room.obj", device);
	if (model == nullptr)
	{
		log_error("Failed to load obj model!");
		return -1;
	}

	// Model diffuse texture
	vtek::Image2DLoadInfo textureInfo{};
	textureInfo.loadSRGB = true;
	textureInfo.forceAlphaPremultiply = false; // TODO: Would be cool to test!
	textureInfo.createMipmaps = false; // TODO: We DEFINITELY want this!
	textureInfo.baseMipLevel = 0;
	vtek::Image2D* texture = vtek::image2d_load(
		&textureInfo, modeldir, "viking_room.png", device);
	if (texture == nullptr)
	{
		log_error("Failed to load texture for model!");
		return -1;
	}

	// Texture sampler
	vtek::SamplerInfo samplerInfo{};
	samplerInfo.addressMode = vtek::SamplerAddressMode::clamp_to_edge;
	samplerInfo.anisotropicFiltering = false; // NOTE: Don't really need this yet
	samplerInfo.minFilter = vtek::SamplerFilterMode::linear;
	samplerInfo.magFilter = vtek::SamplerFilterMode::linear;
	vtek::Sampler* sampler = vtek::sampler_create(&samplerInfo, device);
	if (sampler == nullptr)
	{
		log_error("Failed to create texture sampler!");
		return -1;
	}

	// Descriptor pool
	vtek::DescriptorPoolInfo descriptorPoolInfo{};
	descriptorPoolInfo.allowUpdateAfterBind = false;
	descriptorPoolInfo.descriptorTypes = {
		{ vtek::DescriptorType::uniform_buffer, 1 }, // NOTE: Leave place for light
		{ vtek::DescriptorType::combined_image_sampler, 1 }};
	vtek::DescriptorPool* descriptorPool =
		vtek::descriptor_pool_create(&descriptorPoolInfo, device);
	if (descriptorPool == nullptr)
	{
		log_error("Failed to create descriptor pool!");
		return -1;
	}

	// Descriptor set layout
	vtek::DescriptorSetLayoutInfo descriptorLayoutInfo{};
	vtek::DescriptorLayoutBinding descriptorBindingCamera{};
	descriptorBindingCamera.type = vtek::DescriptorType::uniform_buffer;
	descriptorBindingCamera.binding = 0;
	descriptorBindingCamera.shaderStages
		= vtek::ShaderStage::vertex
		| vtek::ShaderStage::fragment;
	descriptorBindingCamera.updateAfterBind = false;
	vtek::DescriptorLayoutBinding descriptorBindingSampler{};
	descriptorBindingSampler.type = vtek::DescriptorType::combined_image_sampler;
	descriptorBindingSampler.binding = 1;
	descriptorBindingSampler.shaderStages = vtek::ShaderStage::fragment;
	descriptorBindingSampler.updateAfterBind = false;
	descriptorLayoutInfo.bindings.emplace_back(descriptorBindingCamera);
	descriptorLayoutInfo.bindings.emplace_back(descriptorBindingSampler);
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
		log_error("Failed to create descriptor set (Camera)!");
		return -1;
	}

	// Uniform buffer (Camera transform)
	vtek::BufferInfo uniformBufferInfoCamera{};
	uniformBufferInfoCamera.size = gCameraUniform.size();
	uniformBufferInfoCamera.requireHostVisibleStorage = true;
	uniformBufferInfoCamera.writePolicy = vtek::BufferWritePolicy::overwrite_often;
	uniformBufferInfoCamera.usageFlags
		= vtek::BufferUsageFlag::transfer_dst
		| vtek::BufferUsageFlag::uniform_buffer;
	vtek::Buffer* uniformBufferCamera =
		vtek::buffer_create(&uniformBufferInfoCamera, device);
	if (uniformBufferCamera == nullptr)
	{
		log_error("Failed to create uniform buffer!");
		return -1;
	}
	if (!update_camera_uniform(uniformBufferCamera, device))
	{
		log_error("Failed to fill uniform buffer!");
		return -1;
	}
	if (!update_descriptor_set(descriptorSet, uniformBufferCamera, texture, sampler, device))
	{
		log_error("Failed to update descriptor set!");
		return -1;
	}

	// Graphics pipeline
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
	vtek::MultisampleState multisampling{};
	vtek::DepthStencilState depthStencil{};
	depthStencil.depthTestEnable = true;
	depthStencil.depthWriteEnable = true;
	vtek::ColorBlendState colorBlending{};
	colorBlending.attachments.emplace_back(
		vtek::ColorBlendAttachment::GetDefault());
	vtek::PipelineRendering pipelineRendering{};
	pipelineRendering.colorAttachmentFormats.push_back(
		vtek::swapchain_get_image_format(swapchain));
	pipelineRendering.depthAttachmentFormat =
		vtek::swapchain_get_depth_image_format(swapchain);

	vtek::GraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.renderPassType = vtek::RenderPassType::dynamic;
	pipelineInfo.renderPass = nullptr; // Nice!
	pipelineInfo.pipelineRendering = &pipelineRendering;
	pipelineInfo.shader = shader;
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
		= vtek::PipelineDynamicState::cull_mode;
		//| vtek::PipelineDynamicState::depth_compare_op;
	pipelineInfo.descriptorSetLayouts.push_back(descriptorSetLayout);
	pipelineInfo.pushConstantType = vtek::PushConstantType::mat4;
	// TODO: Push constant m4_v4, for transform _and_ vertex color?
	pipelineInfo.pushConstantShaderStages = vtek::ShaderStageGraphics::vertex;

	vtek::GraphicsPipeline* pipeline =
		vtek::graphics_pipeline_create(&pipelineInfo, device);
	if (pipeline == nullptr)
	{
		log_error("Failed to create graphics pipeline!");
		return -1;
	}
	// Modify polygon mode and create another pipeline
	rasterizer.polygonMode = vtek::PolygonMode::line;
	vtek::GraphicsPipeline* pipelineWireframe =
		vtek::graphics_pipeline_create(&pipelineInfo, device);
	if (pipelineWireframe == nullptr)
	{
		log_error("Failed to create graphics pipeline (Wireframe)!");
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
	vtek::descriptor_set_destroy(descriptorSet);
	vtek::descriptor_set_layout_destroy(descriptorSetLayout, device);
	vtek::descriptor_pool_destroy(descriptorPool, device);
	vtek::sampler_destroy(sampler, device);
	vtek::image2d_destroy(texture, device);
	vtek::model_destroy(model, device);
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
