#include <vtek/vtek.hpp>
#include <iostream>

/* global data */
vtek::ApplicationWindow* gWindow = nullptr;
uint32_t gFramebufferWidth = 0U;
uint32_t gFramebufferHeight = 0U;
vtek::KeyboardMap gKeyboardMap;
glm::vec2 gMoveOffset {0.0f, 0.0f};
vtek::Camera* gCamera = nullptr;
vtek::Uniform_m4 gCameraUniform;
uint32_t gNumFramesInFlight = 0U;

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

void mouse_move_callback(double x, double y)
{
	vtek::camera_on_mouse_move(gCamera, x, y);
}

void mouse_scroll_callback(double x, double y)
{
	vtek::camera_on_mouse_scroll(gCamera, x, y);
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
bool update_camera_uniform(vtek::Buffer* buffer, vtek::Device* device)
{
	// Wait until the device is finished with all operations
	vtek::device_wait_idle(device);

	// Packed data
	gCameraUniform.m4 = *(vtek::camera_get_projection_matrix(gCamera));
	gCameraUniform.m4 *= *(vtek::camera_get_view_matrix(gCamera));

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

bool update_descriptor_sets_main(
	std::vector<vtek::DescriptorSet*>& sets,
	std::vector<vtek::Buffer*> uniformBuffers,
	vtek::Image2D* image, vtek::Sampler* sampler,
	vtek::Device* device)
{
	for (uint32_t i = 0; i < sets.size(); i++)
	{
		// descriptor for camera uniform
		if (!vtek::descriptor_set_bind_uniform_buffer(
			    sets[i], 0, uniformBuffers[i], gCameraUniform.type()))
		{
			log_error(
				"Failed to add camera uniform buffer to the descriptor set!");
			return false;
		}

		// Texture for model albedo color
		if (!vtek::descriptor_set_bind_combined_image2d_sampler(
			    sets[i], 1, sampler, image,
			    vtek::ImageLayout::shader_readonly_optimal))
		{
			log_error("Failed to add image sampler uniform to descriptor set!");
			return false;
		}

		vtek::descriptor_set_update(sets[i], device);
	}

	return true;
}

bool update_descriptor_sets_quad(
	std::vector<vtek::DescriptorSet*>& sets,
	std::vector<vtek::Framebuffer*>& framebuffers,
	vtek::Sampler* sampler, vtek::Device* device)
{
	for (uint32_t i = 0; i < sets.size(); i++)
	{
		std::vector<vtek::Image2D*> fbImages =
			vtek::framebuffer_get_color_images(framebuffers[i]);

		// NOTE: We only care about fbImages[0] since there is only 1 attachment!
		if (!vtek::descriptor_set_bind_combined_image2d_sampler(
			    sets[i], 0, sampler, fbImages[0],
			    vtek::ImageLayout::shader_readonly_optimal))
		{
			log_error(
				"Failed to add framebuffer image sampler to descriptor set!");
			return false;
		}
		vtek::descriptor_set_update(sets[i], device);
	}

	return true;
}

struct RenderingInfo
{
	vtek::CommandBuffer* commandBuffer;
	vtek::Framebuffer* framebuffer;
	vtek::GraphicsPipeline* mainPipeline {nullptr};
	vtek::GraphicsPipeline* quadPipeline {nullptr};
	vtek::DescriptorSet* mainDescriptorSet {nullptr};
	vtek::DescriptorSet* quadDescriptorSet {nullptr};
	vtek::Swapchain* swapchain {nullptr};
	glm::uvec2 windowSize {0U, 0U};
	vtek::Buffer* quadBuffer {nullptr};
};

bool recordCommandBuffer(
	RenderingInfo* info, uint32_t imageIndex, vtek::Model* model)
{
	auto commandBuffer = info->commandBuffer;
	auto framebuffer = info->framebuffer;

	// 1) begin recording on the command buffer
	vtek::CommandBufferBeginInfo beginInfo{};
	beginInfo.oneTimeSubmit = true;
	if (!vtek::command_buffer_begin(commandBuffer, &beginInfo))
	{
		log_error("Failed to begin command buffer {} recording!", imageIndex);
		return false;
	}

	// 2) begin dynamic rendering on the framebuffer
	if (!vtek::framebuffer_dynamic_rendering_begin(framebuffer, commandBuffer))
	{
		log_error("Failed to begin dynamic rendering on framebuffer!");
		return false;
	}

	// 3) bind pipeline and set dynamic states
	vtek::cmd_bind_graphics_pipeline(commandBuffer, info->mainPipeline);
	vtek::cmd_set_viewport_scissor(commandBuffer, info->windowSize);

	// Bind stuff for model
	VkCommandBuffer cmdBuf = vtek::command_buffer_get_handle(commandBuffer);
	VkBuffer buffers[2] = {
		vtek::buffer_get_handle(vtek::model_get_vertex_buffer(model)),
		vtek::buffer_get_handle(vtek::model_get_texcoord_buffer(model))
	};
	VkDeviceSize offsets[2] = { 0, 0 };
	vkCmdBindVertexBuffers(cmdBuf, 0, 2, buffers, offsets);

	// 4) bind and render model, including descriptor sets and push constants
	vtek::cmd_bind_descriptor_set_graphics(
		commandBuffer, info->mainPipeline, info->mainDescriptorSet);
	vtek::PushConstant_m4 pc{};
	pc.m1 = glm::mat4(1.0f); // unit matrix
	vtek::cmd_push_constant_graphics(
		commandBuffer, info->mainPipeline, &pc, vtek::ShaderStageGraphics::vertex);
	uint32_t numVertices = vtek::model_get_num_vertices(model);
	vtek::cmd_draw_vertices(commandBuffer, numVertices);

	// 5) end dynamic rendering on the framebuffer
	vtek::framebuffer_dynamic_rendering_end(framebuffer, commandBuffer);

	// 6) begin dynamic rendering on the swapchain
	glm::vec3 clearColor(0.0f, 0.0f, 0.0f);
	vtek::swapchain_dynamic_rendering_begin(
		info->swapchain, imageIndex, commandBuffer, clearColor);

	// 7) bind the quad pipeline
	vtek::cmd_bind_graphics_pipeline(commandBuffer, info->quadPipeline);

	// 8) bind framebuffer targets for reading (descriptor sets?)
	vtek::cmd_bind_descriptor_set_graphics(
		commandBuffer, info->quadPipeline, info->quadDescriptorSet);

	// 9) draw fullscreen quad
	vtek::cmd_bind_vertex_buffer(commandBuffer, info->quadBuffer, 0);
	vtek::cmd_draw_vertices(commandBuffer, 6);

	// 10) end dynamic rendering on the swapchain
	vtek::swapchain_dynamic_rendering_end(
		info->swapchain, imageIndex, commandBuffer);

	// 11) end recording on the command buffer
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
	initInfo.applicationTitle = "08_framebuffer";
	initInfo.useGLFW = true;
	initInfo.loadShadersFromGLSL = true;
	if (!vtek::initialize(&initInfo))
	{
		std::cerr << "Failed to initialize vtek!" << std::endl;
		return -1;
	}

	// Keyboard map
	gKeyboardMap.reset();

	// Create window
	vtek::WindowInfo windowInfo{};
	windowInfo.title = "vtek example 08: Deferred rendering with framebuffer";
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
	vtek::window_set_mouse_move_handler(gWindow, mouse_move_callback);
	vtek::window_set_mouse_scroll_handler(gWindow, mouse_scroll_callback);

	// Vulkan instance
	vtek::InstanceInfo instanceInfo{};
	instanceInfo.applicationName = "08_framebuffer";
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
	gNumFramesInFlight = vtek::kMaxFramesInFlight;

	// Framebuffer attachment formats
	std::vector<vtek::Format> prioritizedColorFormats = {
		vtek::Format::r8g8b8_unorm,
		//vtek::Format::r8g8b8a8_unorm
		vtek::Format::b8g8r8a8_unorm
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
	if (colorFormat.get() == vtek::Format::r8g8b8_unorm)
	{
		log_debug("color format: vtek::Format::r8g8b8_unorm");
	}
	else if (colorFormat.get() == vtek::Format::b8g8r8a8_unorm)
	{
		log_debug("color format: vtek::Format::b8g8r8a8_unorm");
	}
	else
	{
		log_debug("INVALID color format!");
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
	colorAttachment.clearValue.setColorFloat(0.5f, 0.16f, 0.14f, 1.0f);
	vtek::FramebufferAttachmentInfo depthStencilAttachment{};
	depthStencilAttachment.supportedFormat = depthStencilFormat;
	depthStencilAttachment.clearValue.setDepthStencil(1.0f, 0);
	vtek::FramebufferInfo framebufferInfo{};
	framebufferInfo.colorAttachments.emplace_back(colorAttachment);
	framebufferInfo.depthStencilAttachment = depthStencilAttachment;
	framebufferInfo.depthStencil = vtek::DepthStencilMode::depth;
	framebufferInfo.resolution = windowSize;
	 // TODO: When multisampling implemented in vtek, try it out?
	framebufferInfo.multisampling = vtek::MultisampleType::none;
	framebufferInfo.renderPass = nullptr;
	framebufferInfo.useDynamicRendering = true;

	// Multiple framebuffers, same number as max frames in flight!
	std::vector<vtek::Framebuffer*> framebuffers = vtek::framebuffer_create(
		&framebufferInfo, gNumFramesInFlight, device);
	if (framebuffers.empty() || framebuffers.size() != gNumFramesInFlight)
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

	// Command buffers
	std::vector<vtek::CommandBuffer*> commandBuffers =
		vtek::command_pool_alloc_buffers(
			graphicsCommandPool, vtek::CommandBufferUsage::primary,
			gNumFramesInFlight, device);
	if (commandBuffers.empty())
	{
		log_error("Failed to create command buffers!");
		return -1;
	}
	if (commandBuffers.size() != gNumFramesInFlight)
	{
		log_error("Number of command buffers created not same as number asked!");
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
		vtek::graphics_shader_load_glsl(&shaderInfo, shaderdirMain, device);
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
		vtek::graphics_shader_load_glsl(&shaderInfo, shaderdirQuad, device);
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
		{ vtek::DescriptorType::sampled_image, 8 }, // TODO: ?
		{ vtek::DescriptorType::uniform_buffer, 8 }
	};
	vtek::DescriptorPool* descriptorPool = vtek::descriptor_pool_create(
		&descriptorPoolInfo, device);
	if (descriptorPool == nullptr)
	{
		log_error("Failed to create descriptor pool!");
		return -1;
	}

	// Descriptor set layout (main)
	vtek::DescriptorLayoutBinding descriptorBindingCamera{};
	descriptorBindingCamera.type = vtek::DescriptorType::uniform_buffer;
	descriptorBindingCamera.binding = 0;
	descriptorBindingCamera.shaderStages = vtek::ShaderStage::vertex;
	descriptorBindingCamera.updateAfterBind = false;
	vtek::DescriptorLayoutBinding descriptorBindingSampler{};
	descriptorBindingSampler.type = vtek::DescriptorType::combined_image_sampler;
	descriptorBindingSampler.binding = 1;
	descriptorBindingSampler.shaderStages = vtek::ShaderStage::fragment;
	descriptorBindingSampler.updateAfterBind = false;
	vtek::DescriptorSetLayoutInfo descriptorLayoutInfo{};
	descriptorLayoutInfo.bindings.emplace_back(descriptorBindingCamera);
	descriptorLayoutInfo.bindings.emplace_back(descriptorBindingSampler);
	vtek::DescriptorSetLayout* descriptorSetLayoutMain =
		vtek::descriptor_set_layout_create(&descriptorLayoutInfo, device);
	if (descriptorSetLayoutMain == nullptr)
	{
		log_error("Failed to create descriptor set layout (main)!");
		return -1;
	}

	// Descriptor set layout (quad)
	vtek::DescriptorLayoutBinding descriptorBindingGbuffer{};
	descriptorBindingGbuffer.type = vtek::DescriptorType::combined_image_sampler;
	descriptorBindingGbuffer.binding = 0;
	descriptorBindingGbuffer.shaderStages = vtek::ShaderStage::fragment;
	descriptorBindingGbuffer.updateAfterBind = false;
	descriptorLayoutInfo.bindings = { descriptorBindingGbuffer };
	vtek::DescriptorSetLayout* descriptorSetLayoutQuad =
		vtek::descriptor_set_layout_create(&descriptorLayoutInfo, device);
	if (descriptorSetLayoutQuad == nullptr)
	{
		log_error("Failed to create descriptor set layout (quad)!");
		return -1;
	}

	// Descriptor sets (main)
	std::vector<vtek::DescriptorSet*> descriptorSetsMain =
		vtek::descriptor_pool_alloc_sets(
			descriptorPool, descriptorSetLayoutMain, gNumFramesInFlight, device);
	if (descriptorSetsMain.empty() ||
	    descriptorSetsMain.size() != gNumFramesInFlight)
	{
		log_error("Failed to create descriptor sets (main)!");
		return -1;
	}

	// Descriptor set (quad)
	std::vector<vtek::DescriptorSet*> descriptorSetsQuad =
		vtek::descriptor_pool_alloc_sets(
			descriptorPool, descriptorSetLayoutQuad, gNumFramesInFlight, device);
	if (descriptorSetsQuad.empty() ||
	    descriptorSetsQuad.size() != gNumFramesInFlight)
	{
		log_error("Failed to create descriptor sets (quad)!");
		return -1;
	}

	// Camera
	vtek::CameraProjectionInfo cameraProjInfo{};
	cameraProjInfo.projection = vtek::CameraProjection::perspective;
	cameraProjInfo.viewportSize = windowSize;
	cameraProjInfo.clipPlanes = { 0.1f, 100.0f };
	cameraProjInfo.fovDegrees = 45.0f;
	vtek::CameraInfo cameraInfo{};
	cameraInfo.mode = vtek::CameraMode::freeform;
	cameraInfo.worldSpaceHandedness = vtek::CameraHandedness::right_handed;
	cameraInfo.position = glm::vec3(-2.0968692f, -2.5813563f, -1.4253441f);
	cameraInfo.front = {0.5990349f, 0.7475561f, 0.28690946f};
	cameraInfo.up = {-0.18743359f, -0.21744627f, 0.95790696f};
	cameraInfo.projectionInfo = &cameraProjInfo;
	gCamera = vtek::camera_create(&cameraInfo);
	if (gCamera == nullptr)
	{
		log_error("Failed to create camera!");
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
	modelInfo.loadNormals = false;
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

	// Uniform buffer (Camera transform)
	// TODO: We need multiple uniform buffers, one for each frame-in-flight!
	vtek::BufferInfo uniformBufferInfoCamera{};
	uniformBufferInfoCamera.size = gCameraUniform.size();
	uniformBufferInfoCamera.requireHostVisibleStorage = true;
	uniformBufferInfoCamera.writePolicy = vtek::BufferWritePolicy::overwrite_often;
	uniformBufferInfoCamera.usageFlags
		= vtek::BufferUsageFlag::transfer_dst
		| vtek::BufferUsageFlag::uniform_buffer;
	std::vector<vtek::Buffer*> uniformBuffersCamera =
		vtek::buffer_create(&uniformBufferInfoCamera, gNumFramesInFlight, device);
	if (uniformBuffersCamera.size() != gNumFramesInFlight)
	{
		log_error("Failed to create uniform buffers (camera)!");
		return -1;
	}
	for (uint32_t i = 0; i < gNumFramesInFlight; i++)
	{
		if (!update_camera_uniform(uniformBuffersCamera[i], device))
		{
			log_error("Failed to fill uniform buffer!");
			return -1;
		}
	}

	// Update descriptor sets
	if (!update_descriptor_sets_main(
		    descriptorSetsMain, uniformBuffersCamera, texture, sampler, device))
	{
		log_error("Failed to update descriptor sets (main)!");
		return -1;
	}
	if (!update_descriptor_sets_quad(
		    descriptorSetsQuad, framebuffers, sampler, device))
	{
		log_error("Failed to update descriptor sets (quad)!");
		return -1;
	}

	// Vertex data for fullscreen quad
	std::vector<float> quadVertices = {
		-1.0f, -1.0f, 0.0f, 0.0f, // top-left
		1.0f, -1.0f, 1.0f, 0.0f,  // top-right
		-1.0f, 1.0f, 0.0f, 1.0f,  // bottom-left

		-1.0f, 1.0f, 0.0f, 1.0f,  // bottom-left
		1.0f, -1.0f, 1.0f, 0.0f,  // top-right
		1.0f, 1.0f, 1.0f, 1.0f    // bottom-right
	};

	// Vertex buffer for fullscreen quad
	vtek::BufferInfo quadBufferInfo{};
	quadBufferInfo.size = quadVertices.size() * sizeof(float);
	quadBufferInfo.requireHostVisibleStorage = false;
	quadBufferInfo.writePolicy = vtek::BufferWritePolicy::write_once;
	quadBufferInfo.usageFlags
		= vtek::BufferUsageFlag::transfer_dst
		| vtek::BufferUsageFlag::vertex_buffer;
	vtek::Buffer* quadBuffer = vtek::buffer_create(&quadBufferInfo, device);
	if (quadBuffer == nullptr)
	{
		log_error("Failed to create quad vertex buffer!");
		return -1;
	}
	vtek::BufferRegion quadBufferRegion{
		.offset = 0,
		.size = quadBufferInfo.size
	};
	if (!buffer_write_data(
		    quadBuffer, quadVertices.data(), &quadBufferRegion, device))
	{
		log_error("Failed to write quad data to vertex buffer!");
		return -1;
	}

	// Graphics pipeline for main render pass
	vtek::ViewportState viewport{
		.viewportRegion = {
			.offset = {0U, 0U},
			.extent = {gFramebufferWidth, gFramebufferHeight}
		},
	};
	// Buffer bindings: Vertex, TexCoord
	vtek::VertexBufferBindings bindings{};
	bindings.add_buffer(
		vtek::VertexAttributeType::vec3, vtek::VertexInputRate::per_vertex);
	bindings.add_buffer(
		vtek::VertexAttributeType::vec2, vtek::VertexInputRate::per_vertex);
	vtek::RasterizationState rasterizer{};
	rasterizer.cullMode = vtek::CullMode::back; // no back-face culling
	rasterizer.polygonMode = vtek::PolygonMode::fill;
	rasterizer.frontFace = vtek::FrontFace::clockwise;
	vtek::MultisampleState multisampling{};
	vtek::DepthStencilState depthStencil{};
	depthStencil.depthTestEnable = true;
	depthStencil.depthWriteEnable = true;
	depthStencil.depthCompareOp = vtek::DepthCompareOp::less_equal;
	depthStencil.stencilTestEnable = false;
	vtek::ColorBlendState colorBlending{};
	colorBlending.attachments.emplace_back(
		vtek::ColorBlendAttachment::GetDefault());
	vtek::PipelineRendering pipelineRendering{};
	pipelineRendering.colorAttachmentFormats =
		vtek::framebuffer_get_color_formats(framebuffers[0]);
	pipelineRendering.depthStencilAttachmentFormat =
		vtek::framebuffer_get_depth_stencil_format(framebuffers[0]);

	vtek::GraphicsPipelineInfo pipelineInfo{};
	pipelineInfo.renderPassType = vtek::RenderPassType::dynamic;
	pipelineInfo.renderPass = nullptr;
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
	pipelineInfo.descriptorSetLayouts = { descriptorSetLayoutMain };
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
	rasterizer.cullMode = vtek::CullMode::front;
	pipelineRendering.colorAttachmentFormats = {
		vtek::swapchain_get_image_format(swapchain)
	};
	pipelineRendering.depthStencilAttachmentFormat = vtek::Format::undefined;
	pipelineInfo.shader = shaderQuad;
	pipelineInfo.vertexInputBindings = &quadBindings;
	pipelineInfo.dynamicStateFlags = {}; // reset
	pipelineInfo.descriptorSetLayouts = { descriptorSetLayoutQuad };
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
		// TODO: Should it be renamed `frameIndex`?
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
		update_movement();
		vtek::camera_update(gCamera);
		if (!update_camera_uniform(uniformBuffersCamera[imageIndex], device))
		{
			log_error("Failed to update uniform buffer (Camera)!");
			return -1;
		}

		// Record command buffer for next frame
		vtek::CommandBuffer* commandBuffer = commandBuffers[imageIndex];
		if (!vtek::command_pool_reset_buffer(graphicsCommandPool, commandBuffer))
		{
			log_error("Failed to reset command buffer prior to re-recording it!");
			errors--; continue;
		}
		RenderingInfo renderingInfo{};
		renderingInfo.commandBuffer = commandBuffer;
		renderingInfo.framebuffer = framebuffers[imageIndex];
		renderingInfo.mainPipeline = mainPipeline;
		renderingInfo.quadPipeline = quadPipeline;
		renderingInfo.mainDescriptorSet = descriptorSetsMain[imageIndex];
		renderingInfo.quadDescriptorSet = descriptorSetsQuad[imageIndex];
		renderingInfo.swapchain = swapchain;
		renderingInfo.windowSize = windowSize;
		renderingInfo.quadBuffer = quadBuffer;
		bool record = recordCommandBuffer(&renderingInfo, imageIndex, model);
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

	vtek::graphics_pipeline_destroy(quadPipeline, device);
	vtek::graphics_pipeline_destroy(mainPipeline, device);
	vtek::buffer_destroy(quadBuffer);
	vtek::buffer_destroy(uniformBuffersCamera);
	vtek::sampler_destroy(sampler, device);
	vtek::image2d_destroy(texture, device);
	vtek::model_destroy(model, device);
	vtek::camera_destroy(gCamera);
	vtek::descriptor_set_layout_destroy(descriptorSetLayoutQuad, device);
	vtek::descriptor_set_layout_destroy(descriptorSetLayoutMain, device);
	vtek::descriptor_pool_destroy(descriptorPool, device);
	// vtek::directory_close(shaderdirMain); // TODO: Not implemented
	// vtek::directory_close(shaderdirQuad); // TODO: Not implemented
	vtek::graphics_shader_destroy(shaderQuad, device);
	vtek::graphics_shader_destroy(shaderMain, device);
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
