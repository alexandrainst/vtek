// glfw
// NOTE: GLFW must be included before the Vulkan header, so we place it first.
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std
#include <chrono>
#include <thread>
#include <unordered_map>
#include <vector>

// vtek
#include "vtek_application_window.hpp"

#include "vtek_input.hpp"
#include "vtek_instance.hpp"
#include "vtek_logging.hpp"


/* global constants */
constexpr int kMinWindowWidth = 100;
constexpr int kMinWindowHeight = 100;


/* struct implementation */
struct vtek::ApplicationWindow
{
	GLFWwindow* glfwHandle {nullptr};
	uint32_t framebufferWidth {0U};
	uint32_t framebufferHeight {0U};

	tKeyCallback fKeyCallback;
	tMouseButtonCallback fMouseButtonCallback;
	tMouseMoveCallback fMouseMoveCallback;
	tMouseScrollCallback fMouseScrollCallback;

	bool frameBufferResized {false};
	bool isMinimized {false};
};



// Event mapper: Fetch a complete window context from an opaque GLFW handle.
// Used for delegating input events and other window-related events, such as
// minimizing/maximizing the window, resizing the framebuffer, etc.
static std::unordered_map<GLFWwindow*, vtek::ApplicationWindow*>* spEventMapper = nullptr;


/* implementation of GLFW backend */
#include "vtek_main.hpp"
#include "impl/vtek_glfw_backend.hpp"

static std::vector<std::string> sRequiredInstanceExtensions {};

bool vtek::glfw_backend_initialize()
{
	if (!glfwInit()) { return false; }

	// Get required instance extensions from GLFW
	uint32_t extensionCount = 0;
	const char** extensions;
	extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
	if (extensions == nullptr)
	{
		if (!glfwVulkanSupported())
		{
			vtek_log_error("Vulkan is not supported on this machine, {}"
			               "or GLFW backend could find a loader and an ICD.");
		}
		else
		{
			vtek_log_error(
				"Creating a Vulkan-compatible surface is not supported {}"
				"by GLFW on this machine!");
			vtek_log_error(
				"However, Vulkan may still be used for off-screen rendering {}",
				"and compute work. You may disable GLFW backend and try again.");
		}
		return false;
	}

	for (uint32_t i = 0; i < extensionCount; i++)
	{
		sRequiredInstanceExtensions.push_back(extensions[i]);
	}

	// Create the event mapper
	spEventMapper = new std::unordered_map<GLFWwindow*, vtek::ApplicationWindow*>();

	return true;
}

void vtek::glfw_backend_terminate()
{
	if (spEventMapper->size() > 0)
	{
		vtek_log_debug("vtek::glfw_backend_terminate: {}",
		               "Not all windows were properly destroyed.");
	}
	delete spEventMapper;
	spEventMapper = nullptr;

	sRequiredInstanceExtensions.clear();

	glfwTerminate();
}

void vtek::glfw_backend_get_required_instance_extensions(
	std::vector<const char*>& list)
{
	// This function may be safely called even when GLFW is not used
	if (!vtek::vtek_context_get_glfw_enabled()) { return; }

	for (auto& s : sRequiredInstanceExtensions)
	{
		list.push_back(s.c_str());
	}
}


/* helper functions */
static vtek::KeyboardKey get_key_from_glfw(int key)
{
	using vtek::KeyboardKey;

	switch (key)
	{
	case GLFW_KEY_UNKNOWN:       return KeyboardKey::unknown; //-1

	case GLFW_KEY_SPACE:         return KeyboardKey::space; //32
	case GLFW_KEY_APOSTROPHE:    return KeyboardKey::apostrophe; //39 /* ' */
	case GLFW_KEY_COMMA:         return KeyboardKey::comma; // 44 /* , */
	case GLFW_KEY_MINUS:         return KeyboardKey::minus; // 45 /* - */
	case GLFW_KEY_PERIOD:        return KeyboardKey::period; // 46 /* . */
	case GLFW_KEY_SLASH:         return KeyboardKey::slash; // 47 /* / */

	case GLFW_KEY_0:             return KeyboardKey::num_0; // 48
	case GLFW_KEY_1:             return KeyboardKey::num_1; // 49
	case GLFW_KEY_2:             return KeyboardKey::num_2; // 50
	case GLFW_KEY_3:             return KeyboardKey::num_3; // 51
	case GLFW_KEY_4:             return KeyboardKey::num_4; // 52
	case GLFW_KEY_5:             return KeyboardKey::num_5; // 53
	case GLFW_KEY_6:             return KeyboardKey::num_6; // 54
	case GLFW_KEY_7:             return KeyboardKey::num_7; // 55
	case GLFW_KEY_8:             return KeyboardKey::num_8; // 56
	case GLFW_KEY_9:             return KeyboardKey::num_9; // 57
	case GLFW_KEY_SEMICOLON:     return KeyboardKey::semicolon; // 59 /* ; */
	case GLFW_KEY_EQUAL:         return KeyboardKey::equal; // 61 /* = */
	case GLFW_KEY_A:             return KeyboardKey::a; // 65
	case GLFW_KEY_B:             return KeyboardKey::b; // 66
	case GLFW_KEY_C:             return KeyboardKey::c; // 67
	case GLFW_KEY_D:             return KeyboardKey::d; // 68
	case GLFW_KEY_E:             return KeyboardKey::e; // 69
	case GLFW_KEY_F:             return KeyboardKey::f; // 70
	case GLFW_KEY_G:             return KeyboardKey::g; // 71
	case GLFW_KEY_H:             return KeyboardKey::h; // 72
	case GLFW_KEY_I:             return KeyboardKey::i; // 73
	case GLFW_KEY_J:             return KeyboardKey::j; // 74
	case GLFW_KEY_K:             return KeyboardKey::k; // 75
	case GLFW_KEY_L:             return KeyboardKey::l; // 76
	case GLFW_KEY_M:             return KeyboardKey::m; // 77
	case GLFW_KEY_N:             return KeyboardKey::n; // 78
	case GLFW_KEY_O:             return KeyboardKey::o; // 79
	case GLFW_KEY_P:             return KeyboardKey::p; // 80
	case GLFW_KEY_Q:             return KeyboardKey::q; // 81
	case GLFW_KEY_R:             return KeyboardKey::r; // 82
	case GLFW_KEY_S:             return KeyboardKey::s; // 83
	case GLFW_KEY_T:             return KeyboardKey::t; // 84
	case GLFW_KEY_U:             return KeyboardKey::u; // 85
	case GLFW_KEY_V:             return KeyboardKey::v; // 86
	case GLFW_KEY_W:             return KeyboardKey::w; // 87
	case GLFW_KEY_X:             return KeyboardKey::x; // 88
	case GLFW_KEY_Y:             return KeyboardKey::y; // 89
	case GLFW_KEY_Z:             return KeyboardKey::z; // 90
	case GLFW_KEY_LEFT_BRACKET:  return KeyboardKey::left_bracket; // 91 /* [ */
	case GLFW_KEY_BACKSLASH:     return KeyboardKey::backslash; // 92 /* \ */
	case GLFW_KEY_RIGHT_BRACKET: return KeyboardKey::right_bracket;   // 93 /* ] */
	case GLFW_KEY_GRAVE_ACCENT:  return KeyboardKey::grave_accent;  // 96 /* ` */
	case GLFW_KEY_WORLD_1:       return KeyboardKey::world_1; // 161 /* non-US #1 */
	case GLFW_KEY_WORLD_2:       return KeyboardKey::world_2; // 162 /* non-US #2 */
	case GLFW_KEY_ESCAPE:        return KeyboardKey::escape; // 256
	case GLFW_KEY_ENTER:         return KeyboardKey::enter;// 257
	case GLFW_KEY_TAB:           return KeyboardKey::tab; // 258
	case GLFW_KEY_BACKSPACE:     return KeyboardKey::backspace; // 259

	case GLFW_KEY_INSERT:        return KeyboardKey::insert; // 260
	case GLFW_KEY_DELETE:        return KeyboardKey::del; // 261
	case GLFW_KEY_RIGHT:         return KeyboardKey::right; // 262
	case GLFW_KEY_LEFT:          return KeyboardKey::left; // 263
	case GLFW_KEY_DOWN:          return KeyboardKey::down; // 264
	case GLFW_KEY_UP:            return KeyboardKey::up; // 265
	case GLFW_KEY_PAGE_UP:       return KeyboardKey::page_up; // 266
	case GLFW_KEY_PAGE_DOWN:     return KeyboardKey::page_down; // 267
	case GLFW_KEY_HOME:          return KeyboardKey::home; // 268
	case GLFW_KEY_END:           return KeyboardKey::end; // 269
	case GLFW_KEY_CAPS_LOCK:     return KeyboardKey::caps_lock; // 280
	case GLFW_KEY_SCROLL_LOCK:   return KeyboardKey::scroll_lock; // 281
	case GLFW_KEY_NUM_LOCK:      return KeyboardKey::num_lock; // 282
	case GLFW_KEY_PRINT_SCREEN:  return KeyboardKey::print_screen; // 283
	case GLFW_KEY_PAUSE:         return KeyboardKey::pause_break; // 284

	case GLFW_KEY_F1:            return KeyboardKey::f1; // 290
	case GLFW_KEY_F2:            return KeyboardKey::f2; // 291
	case GLFW_KEY_F3:            return KeyboardKey::f3; // 292
	case GLFW_KEY_F4:            return KeyboardKey::f4; // 293
	case GLFW_KEY_F5:            return KeyboardKey::f5; // 294
	case GLFW_KEY_F6:            return KeyboardKey::f6; // 295
	case GLFW_KEY_F7:            return KeyboardKey::f7; // 296
	case GLFW_KEY_F8:            return KeyboardKey::f8; // 297
	case GLFW_KEY_F9:            return KeyboardKey::f9; // 298
	case GLFW_KEY_F10:           return KeyboardKey::f10; // 299
	case GLFW_KEY_F11:           return KeyboardKey::f11; // 300
	case GLFW_KEY_F12:           return KeyboardKey::f12; // 301

	case GLFW_KEY_KP_0:          return KeyboardKey::numpad_0; // 320
	case GLFW_KEY_KP_1:          return KeyboardKey::numpad_1; // 321
	case GLFW_KEY_KP_2:          return KeyboardKey::numpad_2; // 322
	case GLFW_KEY_KP_3:          return KeyboardKey::numpad_3; // 323
	case GLFW_KEY_KP_4:          return KeyboardKey::numpad_4; // 324
	case GLFW_KEY_KP_5:          return KeyboardKey::numpad_5; // 325
	case GLFW_KEY_KP_6:          return KeyboardKey::numpad_6; // 326
	case GLFW_KEY_KP_7:          return KeyboardKey::numpad_7; // 327
	case GLFW_KEY_KP_8:          return KeyboardKey::numpad_8; // 328
	case GLFW_KEY_KP_9:          return KeyboardKey::numpad_9; // 329
	case GLFW_KEY_KP_DECIMAL:    return KeyboardKey::numpad_decimal; // 330
	case GLFW_KEY_KP_DIVIDE:     return KeyboardKey::numpad_divide; // 331
	case GLFW_KEY_KP_MULTIPLY:   return KeyboardKey::numpad_multiply; // 332
	case GLFW_KEY_KP_SUBTRACT:   return KeyboardKey::numpad_subtract; // 333
	case GLFW_KEY_KP_ADD:        return KeyboardKey::numpad_add; // 334
	case GLFW_KEY_KP_ENTER:      return KeyboardKey::numpad_enter; // 335
	case GLFW_KEY_KP_EQUAL:      return KeyboardKey::numpad_equal; // 336

	case GLFW_KEY_LEFT_SHIFT:    return KeyboardKey::left_shift; // 340
	case GLFW_KEY_LEFT_CONTROL:  return KeyboardKey::left_control; // 341
	case GLFW_KEY_LEFT_ALT:      return KeyboardKey::left_alt; // 342
	case GLFW_KEY_LEFT_SUPER:    return KeyboardKey::left_super; // 343
	case GLFW_KEY_RIGHT_SHIFT:   return KeyboardKey::right_shift; // 344
	case GLFW_KEY_RIGHT_CONTROL: return KeyboardKey::right_control; // 345
	case GLFW_KEY_RIGHT_ALT:     return KeyboardKey::right_alt; // 346
	case GLFW_KEY_RIGHT_SUPER:   return KeyboardKey::right_super; // 347

	// These two are defined as the same, that is - last is defined as menu!
	//case GLFW_KEY_LAST:   // 348
	case GLFW_KEY_MENU:          return KeyboardKey::menu;

	default:
		return KeyboardKey::unknown;
	}
}

static vtek::InputAction get_input_action_from_glfw(int action)
{
	using vtek::InputAction;

	switch (action)
	{
	case GLFW_PRESS:   return InputAction::press;
	case GLFW_RELEASE: return InputAction::release;
	case GLFW_REPEAT:  return InputAction::repeat;
	default:
		return InputAction::ignore;
	}
}

static vtek::MouseButton get_mouse_button_from_glfw(int button)
{
	switch (button)
	{
	case GLFW_MOUSE_BUTTON_LEFT:   return vtek::MouseButton::left;
	case GLFW_MOUSE_BUTTON_MIDDLE: return vtek::MouseButton::middle;
	case GLFW_MOUSE_BUTTON_RIGHT:  return vtek::MouseButton::right;
	default:
		return vtek::MouseButton::ignore;
	}
}

static void default_key_callback(vtek::KeyboardKey, vtek::InputAction) {}
static void default_mouse_button_callback(vtek::MouseButton, vtek::InputAction) {}
static void default_mouse_move_callback(double, double) {}
static void default_mouse_scroll_callback(double, double) {}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	auto appWindow = (*spEventMapper)[window];
	// TODO: VTEK_ASSERT(appWindow != nullptr);

	vtek::KeyboardKey vKey = get_key_from_glfw(key);
	vtek::InputAction vAction = get_input_action_from_glfw(action);

	// By storing only a pointer to the context, and _not_ the callback itself,
	// we can change the key callback at runtime.
	// NOTE: This will be less efficient!
	// REVIEW: We may simply force upon the application to never change the callback?
	// context->windowInterface.fKey(vKey, vAction);
	appWindow->fKeyCallback(vKey, vAction);
}

static void mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	auto appWindow = (*spEventMapper)[window];
	// TODO: VTEK_ASSERT(appWindow != nullptr);

	appWindow->fMouseMoveCallback(xpos, ypos);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	auto appWindow = (*spEventMapper)[window];
	// TODO: VTEK_ASSERT(appWindow != nullptr);

	vtek::MouseButton vButton = get_mouse_button_from_glfw(button);
	vtek::InputAction vAction = get_input_action_from_glfw(action);

	appWindow->fMouseButtonCallback(vButton, vAction);
}

static void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	auto appWindow = (*spEventMapper)[window];
	// TODO: VTEK_ASSERT(appWindow != nullptr);

	appWindow->fMouseScrollCallback(xoffset, yoffset);
}

static void framebuffer_resize_callback(GLFWwindow* window, int width, int height)
{
	auto appWindow = (*spEventMapper)[window];
	// TODO: VTEK_ASSERT(appWindow != nullptr);

	appWindow->frameBufferResized = true;
}

static void window_minimize_callback(GLFWwindow* window, int iconified)
{
	auto appWindow = (*spEventMapper)[window];
	// TODO: VTEK_ASSERT(appWindow != nullptr);

	if (iconified)
	{
		// The window was iconified (ie. minimized)
		vtek_log_debug("window_minimize_callback: minimize");
		appWindow->isMinimized = true;
	}
	else
	{
		// The window was restored
		vtek_log_debug("window_minimize_callback: restore");
		appWindow->isMinimized = false;
	}
}

static void set_window_hints(const vtek::WindowInfo* info)
{
	// Always disable GL API with Vulkan
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	// The swap interval indicates how many frames to wait until swapping the
	// buffers, commonly known as vsync. By default, the swap interval is zero,
	// meaning buffer swapping will occur immediately. On fast machines, many
	// of those frames will never be seen, as the screen is still only updated
	// typically 60-75 times per second, so this wastes a lot of CPU and GPU
	// cycles. Also, because the buffers will be swapped in the middle the
	// screen update, leading to screen tearing. For these reasons, applications
	// will typically want to set the swap interval to one. It can be set to
	// higher values, but this is usually not recommended, because of the input
	// latency it leads to.
	glfwSwapInterval(1);

	if (info->fullscreen)
	{
		// RESIZE: Always disabled for fullscreen windows
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	}
	else
	{
		// DECORATE: Default is GLFW_TRUE
		glfwWindowHint(GLFW_DECORATED, (info->decorated) ? GLFW_TRUE : GLFW_FALSE);

		// RESIZE: Ignored for undecorated or fullscreen windows
		if (info->decorated)
		{
			glfwWindowHint(GLFW_RESIZABLE, (info->resizeable) ? GL_TRUE : GL_FALSE);
		}

		// MAXIMIZED: Default is GLFW_FALSE
		glfwWindowHint(GLFW_MAXIMIZED, (info->maximized) ? GLFW_TRUE : GLFW_FALSE);
	}
}

static void configure_window(GLFWwindow* window, const vtek::WindowInfo* info)
{
	// Raw mouse motion is not affected by the scaling and acceleration applied
	// to the motion of the desktop cursor, hence more suitable for controlling
	// a 3D camera.
	if (info->cursorDisabled)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		if (glfwRawMouseMotionSupported())
		{
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}
	}

	// Define window size limits, such that when a window is resized its size
	// will never be too small. We use `GLFW_DONT_CARE` for maximum width and
	// height.
	if (!info->fullscreen)
	{
		glfwSetWindowSizeLimits(
			window, kMinWindowWidth, kMinWindowHeight,
			GLFW_DONT_CARE, GLFW_DONT_CARE);
	}
}

static GLFWwindow* create_fullscreen_window(const vtek::WindowInfo* info)
{
	// Quoting GLFW docs:
	// "Unless you have a way for the user to choose a specific monitor, it is
	// recommended that you pick the primary monitor."
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	if (monitor == nullptr)
	{
		vtek_log_error("Failed to call glfwGetPrimaryMonitor -- {}",
		               "cannot create GLFW fullscreen window!");
		return nullptr;
	}

	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	if (mode == nullptr)
	{
		vtek_log_error("Failed to call glfwGetVideoMode -- {}",
		               "cannot create GLFW fullscreen window!");
		return nullptr;
	}

	return glfwCreateWindow(
		mode->width, mode->height, info->title, monitor, nullptr);
}



/* interface */
vtek::ApplicationWindow* vtek::window_create(const vtek::WindowInfo* info)
{
	auto appWindow = new vtek::ApplicationWindow;

	// Set hints for how GLFW should create the window
	set_window_hints(info);

	if (info->fullscreen) {
		appWindow->glfwHandle = create_fullscreen_window(info);
	}
	else {
		appWindow->glfwHandle = glfwCreateWindow(
			info->width, info->height, info->title, NULL, NULL);
	}
	if (appWindow->glfwHandle == nullptr)
	{
		vtek_log_error("Failed to create GLFW window!");
		delete appWindow;
		return nullptr;
	}

	// Configure the window after it has been created
	configure_window(appWindow->glfwHandle, info);

	// Get framebuffer size, needed when creating a swapchain. Description below:
	//
	// Window resolutions are measured in virtual screen coordinates,
	// but swap image sizes are measured in pixels. These do not always
	// correspond, e.g. on high DPI displays such as Apple's Retina.
	// So after the window is created, we can query GLFW for the
	// framebuffer size, which is always in pixels, and use this value
	// to determine an appropriate swap image size.
	int width, height;
	glfwGetFramebufferSize(appWindow->glfwHandle, &width, &height);
	if (width < 0 || height < 0)
	{
		vtek_log_error("Invalid framebuffer dimensions retrieved from GLFW!");
		vtek_log_error("--> Cannot create application window.");
		delete appWindow;
		return nullptr;
	}
	appWindow->framebufferWidth = static_cast<uint32_t>(width);
	appWindow->framebufferHeight = static_cast<uint32_t>(height);

	// Add window to the event mapper
	(*spEventMapper)[appWindow->glfwHandle] = appWindow;

	// Setup default event handlers
	appWindow->fKeyCallback = default_key_callback;
	appWindow->fMouseButtonCallback = default_mouse_button_callback;
	appWindow->fMouseMoveCallback = default_mouse_move_callback;
	appWindow->fMouseScrollCallback = default_mouse_scroll_callback;

	glfwSetKeyCallback(appWindow->glfwHandle, key_callback);
	glfwSetMouseButtonCallback(appWindow->glfwHandle, mouse_button_callback);
	glfwSetCursorPosCallback(appWindow->glfwHandle, mouse_move_callback);
	glfwSetScrollCallback(appWindow->glfwHandle, mouse_scroll_callback);

	if (!info->fullscreen && info->decorated && info->resizeable)
	{
		glfwSetFramebufferSizeCallback(
			appWindow->glfwHandle, framebuffer_resize_callback);
	}
	glfwSetWindowIconifyCallback(appWindow->glfwHandle, window_minimize_callback);

	// NEXT: Might use other callbacks: maximized, on_close, lose_focus, etc.

	return appWindow;
}

void vtek::window_destroy(vtek::ApplicationWindow* window)
{
	if (window == nullptr) { return; }

	auto erased = spEventMapper->erase(window->glfwHandle);
	if (erased == 0)
	{
		vtek_log_debug(
			"vtek::window_destroy(): {}",
			"Window was not registered in the event mapper - cannot remove!");
	}

	if (window->glfwHandle != nullptr)
	{
		glfwDestroyWindow(window->glfwHandle);
	}

	delete window;
}

VkSurfaceKHR vtek::window_create_surface(
	vtek::ApplicationWindow* window, vtek::Instance* instance)
{
	VkInstance inst = vtek::instance_get_handle(instance);

	VkSurfaceKHR surface = VK_NULL_HANDLE;
	if (glfwCreateWindowSurface(inst, window->glfwHandle, nullptr, &surface) != VK_SUCCESS)
	{
		vtek_log_error("Failed to create GLFW window surface!");
		return VK_NULL_HANDLE;
	}

	return surface;
}

void vtek::window_surface_destroy(VkSurfaceKHR surface, vtek::Instance* instance)
{
	if (surface == VK_NULL_HANDLE) { return; }

	VkInstance inst = vtek::instance_get_handle(instance);
	vkDestroySurfaceKHR(inst, surface, nullptr);
}

void vtek::window_poll_events()
{
	glfwPollEvents();
}

void vtek::window_get_framebuffer_size(
	vtek::ApplicationWindow* window, uint32_t* width, uint32_t* height)
{
	int w, h;
	glfwGetFramebufferSize(window->glfwHandle, &w, &h);
	window->framebufferWidth = static_cast<uint32_t>(w);
	window->framebufferHeight = static_cast<uint32_t>(h);
	*width = window->framebufferWidth;
	*height = window->framebufferHeight;
}

bool vtek::window_get_should_close(vtek::ApplicationWindow* window)
{
	return !glfwWindowShouldClose(window->glfwHandle);
}

void vtek::window_set_should_close(vtek::ApplicationWindow* window, bool shouldClose)
{
	glfwSetWindowShouldClose(window->glfwHandle, shouldClose ? GLFW_TRUE : GLFW_FALSE);
}

void vtek::window_wait_while_minimized(vtek::ApplicationWindow* window)
{
	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(window->glfwHandle, &width, &height);
	while(width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window->glfwHandle, &width, &height);
		glfwWaitEvents();
	}
	if (width < 0 || height < 0)
	{
		vtek_log_error("Invalid framebuffer dimensions retrieved from GLFW!");
		vtek_log_error("--> Cannot properly wait while minimized.");
		window->framebufferWidth = 0U;
		window->framebufferHeight = 0U;
		return;
	}

	window->framebufferWidth = static_cast<uint32_t>(width);
	window->framebufferHeight = static_cast<uint32_t>(height);
}

bool vtek::window_is_resizing(vtek::ApplicationWindow* window)
{
	return window->frameBufferResized;
}

void vtek::window_wait_while_resizing(vtek::ApplicationWindow* window)
{
	while (window->frameBufferResized)
	{
		window->frameBufferResized = false;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

void vtek::window_get_content_scale(
	vtek::ApplicationWindow* window, float* scaleX, float* scaleY)
{
	glfwGetWindowContentScale(window->glfwHandle, scaleX, scaleY);
}



void vtek::window_set_key_handler(
	vtek::ApplicationWindow* window, vtek::tKeyCallback fn)
{
	window->fKeyCallback = fn;
}

void vtek::window_set_mouse_button_handler(
	vtek::ApplicationWindow* window, vtek::tMouseButtonCallback fn)
{
	window->fMouseButtonCallback = fn;
}

void vtek::window_set_mouse_move_handler(
	vtek::ApplicationWindow* window, vtek::tMouseMoveCallback fn)
{
	window->fMouseMoveCallback = fn;
}

void vtek::window_set_mouse_scroll_handler(
	vtek::ApplicationWindow* window, vtek::tMouseScrollCallback fn)
{
	window->fMouseScrollCallback = fn;
}
