#include <vtek/vtek.hpp>
#include <iostream>

/* global data */
vtek::ApplicationWindow* gWindow = nullptr;
uint32_t gFramebufferWidth = 0U;
uint32_t gFramebufferHeight = 0U;
vtek::KeyboardMap gKeyboardMap;
vtek::Camera* gCamera = nullptr;
vtek::Uniform_m4_v4 gCameraUniform;
// TODO: Implement pipeline derivation!
// TODO: Depth testing switch by depth function as dynamic pipeline state!
bool gRenderWireframe = false;
vtek::CullMode gCullMode = vtek::CullMode::back; // default: back-face culling


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
}

/* helper functions */
bool update_camera_uniform(
	vtek::DescriptorSet* set, vtek::Buffer* buffer, vtek::Device* device)
{
	// Wait until the device is finished with all operations
	vtek::device_wait_idle(device);

	// Packed data
	gCameraUniform.m4 = *(vtek::camera_get_projection_matrix(gCamera));
	gCameraUniform.m4 *= *(vtek::camera_get_view_matrix(gCamera));
	gCameraUniform.v4 = glm::vec4(vtek::camera_get_position(gCamera), 0.0f);

	// Update uniform buffer
	vtek::BufferRegion region{
		.offset = 0,
		.size = gCameraUniform.size()
	};
	if (!vtek::buffer_write_data(buffer, &gCameraUniform, &region, device))
	{
		log_error("Failed to write data to the uniform buffer (Camera)!");
		return false;
	}

	// Update descriptor set
	if (!vtek::descriptor_set_bind_uniform_buffer(
		    set, 0, buffer, gCameraUniform.type()))
	{
		log_error("Failed to add uniform buffer to the descriptor set (Camera)!");
		return false;
	}
	vtek::descriptor_set_update(set, device);

	return true;
}

bool recordCommandBuffer(
	vtek::CommandBuffer* commandBuffer, vtek::GraphicsPipeline* pipeline,
	vtek::Swapchain* swapchain, uint32_t imageIndex,
	vtek::Model* model, const vtek::DescriptorSet* descriptorSets[2])
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
	VkBuffer buffers[2] = {
		vtek::buffer_get_handle(vtek::model_get_vertex_buffer(model)),
		vtek::buffer_get_handle(vtek::model_get_normal_buffer(model))
	};
	VkDeviceSize offsets[2] = { 0, 0 };
	vkCmdBindVertexBuffers(cmdBuf, 0, 2, buffers, offsets);

	VkDescriptorSet descriptorSetHandles[2] = {
		vtek::descriptor_set_get_handle(descriptorSets[0]),
		vtek::descriptor_set_get_handle(descriptorSets[1]),
	};
	vkCmdBindDescriptorSets(
		cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipLayout, 0, 2,
		descriptorSetHandles, 0, nullptr); // NOTE: Dynamic offset unused

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

	// Descriptor pool
	vtek::DescriptorPoolInfo descriptorPoolInfo{};
	descriptorPoolInfo.allowUpdateAfterBind = false;
	descriptorPoolInfo.descriptorTypes.push_back(
		{ vtek::DescriptorType::uniform_buffer, 1 }); // NOTE: Leave place for light
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
	descriptorBindingCamera.updateAfterBind = false;
	descriptorLayoutInfoCamera.bindings.emplace_back(descriptorBindingCamera);
	vtek::DescriptorSetLayout* descriptorSetLayoutCamera =
		vtek::descriptor_set_layout_create(&descriptorLayoutInfoCamera, device);
	if (descriptorSetLayoutCamera == nullptr)
	{
		log_error("Failed to create descriptor set layout (Camera)!");
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
		log_error("Failed to create uniform buffer (Camera)!");
		return -1;
	}
	if (!update_camera_uniform(descriptorSetCamera, uniformBufferCamera, device))
	{
		log_error("Failed to fill uniform buffer (Camera)!");
		return -1;
	}

	// Graphics pipeline
	vtek::ViewportState viewport{
		.viewportRegion = {
			.offset = {0U, 0U},
			.extent = {gFramebufferWidth, gFramebufferHeight}
		},
	};
	vtek::VertexBufferBindings bindings{};
	bindings.add_buffer(
		vtek::VertexAttributeType::vec3, vtek::VertexInputRate::per_vertex); // vertex
	bindings.add_buffer(
		vtek::VertexAttributeType::vec3, vtek::VertexInputRate::per_vertex); // normal
	vtek::RasterizationState rasterizer{};
	rasterizer.cullMode = vtek::CullMode::back; // enable back-face culling
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
		= vtek::PipelineDynamicState::cull_mode
		| vtek::PipelineDynamicState::depth_compare_op;
	pipelineInfo.descriptorSetLayouts.push_back(descriptorSetLayoutCamera);
	//pipelineInfo.descriptorSetLayouts.push_back(descriptorSetLayoutLight);
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










	return 0;
}
