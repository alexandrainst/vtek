// glfw
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std
#include <vector>

// vtek
#include "impl/vtek_host_allocator.h"
#include "vtek_application_window.h"
#include "vtek_input.h"
#include "vtek_logging.h"
#include "vtek_main.h"


/* struct implementation */
struct vtek::ApplicationWindow
{
	uint64_t id {0UL};
	GLFWwindow* glfwHandle {nullptr};
	int framebufferWidth {0};
	int framebufferHeight {0};

	tKeyCallback fKeyCallback;
	tMouseButtonCallback fMouseButtonCallback;
	tMouseMoveCallback fMouseMoveCallback;
	tMouseScrollCallback fMouseScrollCallback;
};


/* host allocator */
static vtek::HostAllocator<vtek::ApplicationWindow> sAllocator("application_window");


/* implementation of GLFW backend */
#include "impl/vtek_glfw_backend.h"

static std::vector<std::string> sRequiredInstanceExtensions {};

bool vtek::glfw_backend_initialize()
{
	if (!glfwInit()) { return false; }

	// Get required instance extensions from GLFW
	uint32_t extensionCount = 0;
	const char** extensions;
	extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
	for (uint32_t i = 0; i < extensionCount; i++)
	{
		sRequiredInstanceExtensions.push_back(extensions[i]);
	}

	return true;
}

void vtek::glfw_backend_terminate()
{
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
static vtek::Key get_key_from_glfw(int key)
{
	switch (glfw_key)
	{
	case GLFW_KEY_UNKNOWN:       return vtek::unknown; //-1

	case GLFW_KEY_SPACE:         return vtek::space; //32
	case GLFW_KEY_APOSTROPHE:    return vtek::apostrophe; //39 /* ' */
	case GLFW_KEY_COMMA:         return vtek::comma; // 44 /* , */
	case GLFW_KEY_MINUS:         return vtek::minus; // 45 /* - */
	case GLFW_KEY_PERIOD:        return vtek::period; // 46 /* . */
	case GLFW_KEY_SLASH:         return vtek::slash; // 47 /* / */

	case GLFW_KEY_0:             return vtek::num_0; // 48
	case GLFW_KEY_1:             return vtek::num_1; // 49
	case GLFW_KEY_2:             return vtek::num_2; // 50
	case GLFW_KEY_3:             return vtek::num_3; // 51
	case GLFW_KEY_4:             return vtek::num_4; // 52
	case GLFW_KEY_5:             return vtek::num_5; // 53
	case GLFW_KEY_6:             return vtek::num_6; // 54
	case GLFW_KEY_7:             return vtek::num_7; // 55
	case GLFW_KEY_8:             return vtek::num_8; // 56
	case GLFW_KEY_9:             return vtek::num_9; // 57
	case GLFW_KEY_SEMICOLON:     return vtek::semicolon; // 59 /* ; */
	case GLFW_KEY_EQUAL:         return vtek::equal; // 61 /* = */
	case GLFW_KEY_A:             return vtek::a; // 65
	case GLFW_KEY_B:             return vtek::b; // 66
	case GLFW_KEY_C:             return vtek::c; // 67
	case GLFW_KEY_D:             return vtek::d; // 68
	case GLFW_KEY_E:             return vtek::e; // 69
	case GLFW_KEY_F:             return vtek::f; // 70
	case GLFW_KEY_G:             return vtek::g; // 71
	case GLFW_KEY_H:             return vtek::h; // 72
	case GLFW_KEY_I:             return vtek::i; // 73
	case GLFW_KEY_J:             return vtek::j; // 74
	case GLFW_KEY_K:             return vtek::k; // 75
	case GLFW_KEY_L:             return vtek::l; // 76
	case GLFW_KEY_M:             return vtek::m; // 77
	case GLFW_KEY_N:             return vtek::n; // 78
	case GLFW_KEY_O:             return vtek::o; // 79
	case GLFW_KEY_P:             return vtek::p; // 80
	case GLFW_KEY_Q:             return vtek::q; // 81
	case GLFW_KEY_R:             return vtek::r; // 82
	case GLFW_KEY_S:             return vtek::s; // 83
	case GLFW_KEY_T:             return vtek::t; // 84
	case GLFW_KEY_U:             return vtek::u; // 85
	case GLFW_KEY_V:             return vtek::v; // 86
	case GLFW_KEY_W:             return vtek::w; // 87
	case GLFW_KEY_X:             return vtek::x; // 88
	case GLFW_KEY_Y:             return vtek::y; // 89
	case GLFW_KEY_Z:             return vtek::z; // 90
	case GLFW_KEY_LEFT_BRACKET:  return vtek::left_bracket; // 91 /* [ */
	case GLFW_KEY_BACKSLASH:     return vtek::backslash; // 92 /* \ */
	case GLFW_KEY_RIGHT_BRACKET: return vtek::right_bracket;   // 93 /* ] */
	case GLFW_KEY_GRAVE_ACCENT:  return vtek::grave_accent;  // 96 /* ` */
	case GLFW_KEY_WORLD_1:       return vtek::world_1; // 161 /* non-US #1 */
	case GLFW_KEY_WORLD_2:       return vtek::world_2; // 162 /* non-US #2 */
	case GLFW_KEY_ESCAPE:        return vtek::escape; // 256
	case GLFW_KEY_ENTER:         return vtek::enter;// 257
	case GLFW_KEY_TAB:           return vtek::tab; // 258
	case GLFW_KEY_BACKSPACE:     return vtek::backspace; // 259

	case GLFW_KEY_INSERT:        return vtek::insert; // 260
	case GLFW_KEY_DELETE:        return vtek::del; // 261
	case GLFW_KEY_RIGHT:         return vtek::right; // 262
	case GLFW_KEY_LEFT:          return vtek::left; // 263
	case GLFW_KEY_DOWN:          return vtek::down; // 264
	case GLFW_KEY_UP:            return vtek::up; // 265
	case GLFW_KEY_PAGE_UP:       return vtek::page_up; // 266
	case GLFW_KEY_PAGE_DOWN:     return vtek::page_down; // 267
	case GLFW_KEY_HOME:          return vtek::home; // 268
	case GLFW_KEY_END:           return vtek::end; // 269
	case GLFW_KEY_CAPS_LOCK:     return vtek::caps_lock; // 280
	case GLFW_KEY_SCROLL_LOCK:   return vtek::scroll_lock; // 281
	case GLFW_KEY_NUM_LOCK:      return vtek::num_lock; // 282
	case GLFW_KEY_PRINT_SCREEN:  return vtek::print_screen; // 283
	case GLFW_KEY_PAUSE:         return vtek::pause_break; // 284

	case GLFW_KEY_F1:            return vtek::f1; // 290
	case GLFW_KEY_F2:            return vtek::f2; // 291
	case GLFW_KEY_F3:            return vtek::f3; // 292
	case GLFW_KEY_F4:            return vtek::f4; // 293
	case GLFW_KEY_F5:            return vtek::f5; // 294
	case GLFW_KEY_F6:            return vtek::f6; // 295
	case GLFW_KEY_F7:            return vtek::f7; // 296
	case GLFW_KEY_F8:            return vtek::f8; // 297
	case GLFW_KEY_F9:            return vtek::f9; // 298
	case GLFW_KEY_F10:           return vtek::f10; // 299
	case GLFW_KEY_F11:           return vtek::f11; // 300
	case GLFW_KEY_F12:           return vtek::f12; // 301

	case GLFW_KEY_KP_0:          return vtek::numpad_0; // 320
	case GLFW_KEY_KP_1:          return vtek::numpad_1; // 321
	case GLFW_KEY_KP_2:          return vtek::numpad_2; // 322
	case GLFW_KEY_KP_3:          return vtek::numpad_3; // 323
	case GLFW_KEY_KP_4:          return vtek::numpad_4; // 324
	case GLFW_KEY_KP_5:          return vtek::numpad_5; // 325
	case GLFW_KEY_KP_6:          return vtek::numpad_6; // 326
	case GLFW_KEY_KP_7:          return vtek::numpad_7; // 327
	case GLFW_KEY_KP_8:          return vtek::numpad_8; // 328
	case GLFW_KEY_KP_9:          return vtek::numpad_9; // 329
	case GLFW_KEY_KP_DECIMAL:    return vtek::numpad_decimal; // 330
	case GLFW_KEY_KP_DIVIDE:     return vtek::numpad_divide; // 331
	case GLFW_KEY_KP_MULTIPLY:   return vtek::numpad_multiply; // 332
	case GLFW_KEY_KP_SUBTRACT:   return vtek::numpad_subtract; // 333
	case GLFW_KEY_KP_ADD:        return vtek::numpad_add; // 334
	case GLFW_KEY_KP_ENTER:      return vtek::numpad_enter; // 335
	case GLFW_KEY_KP_EQUAL:      return vtek::numpad_equal; // 336

	case GLFW_KEY_LEFT_SHIFT:    return vtek::left_shift; // 340
	case GLFW_KEY_LEFT_CONTROL:  return vtek::left_control; // 341
	case GLFW_KEY_LEFT_ALT:      return vtek::left_alt; // 342
	case GLFW_KEY_LEFT_SUPER:    return vtek::left_super; // 343
	case GLFW_KEY_RIGHT_SHIFT:   return vtek::right_shift; // 344
	case GLFW_KEY_RIGHT_CONTROL: return vtek::right_control; // 345
	case GLFW_KEY_RIGHT_ALT:     return vtek::right_alt; // 346
	case GLFW_KEY_RIGHT_SUPER:   return vtek::right_super; // 347

	// These two are defined as the same, that is - last is defined as menu!
	//case GLFW_KEY_LAST:   // 348
	case GLFW_KEY_MENU:          return vtek::menu;

	default:
		return vtek::unknown;
	}
}

static vtek::KeyAction get_key_action_from_glfw(int action)
{
	switch (action)
	{
	case GLFW_PRESS: return vtek::press;
	}
}

static void default_key_callback(vtek::Key, vtek::KeyAction) {}
static void default_mouse_button_callback(vtek::MouseButton, vtek::MouseAction) {}
static void default_mouse_move_callback(double, double) {}
static void default_mouse_scroll_callback(double, double) {}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// TODO: Multiple windows??
	static const vtek::ApplicationWindow* appWindow =
		static_cast<vtek::ApplicationWindow*>(glfwGetWindowUserPointer(window));
	ASSERT_MSG(context != nullptr,
	           "key_callback: error fetching GLFW user pointer!");

	vtek::Key vKey = get_key_from_glfw(key);
	vtek::KeyAction vAction = get_key_action_from_glfw(action);

	// By storing only a pointer to the context, and _not_ the callback itself,
	// we can change the key callback at runtime.
	// NOTE: This will be less efficient!
	// REVIEW: We may simply force upon the application to never change the callback?
	context->windowInterface.fKey(vKey, vAction);
}

static void mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	static const vtek::ApplicationWindow* appWindow =
		static_cast<vtek::ApplicationWindow*>(glfwGetWindowUserPointer(window));
	ASSERT_MSG(context != nullptr,
	           "mouse_move_callback: error fetching GLFW user pointer!");

	context->windowInterface.fMouseMove(xpos, ypos);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	static const vtek::ApplicationWindow* appWindow =
		static_cast<vtek::ApplicationWindow*>(glfwGetWindowUserPointer(window));
	ASSERT_MSG(context != nullptr,
	           "mouse_button_callback: error fetching GLFW user pointer!");

	// yggdrasil::input::MouseButtonType button,
	// yggdrasil::input::InputActionType action
	input::MouseButtonType bt = graphics::get_glfw_mouse_button(button);
	input::MouseActionType at = graphics::get_glfw_mouse_action(action);

	context->windowInterface.fMouseButton(bt, at);
}

static void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	static const vtek::ApplicationWindow* appWindow =
		static_cast<vtek::ApplicationWindow*>(glfwGetWindowUserPointer(window));
	ASSERT_MSG(context != nullptr,
	           "mouse_scroll_callback: error fetching GLFW user pointer!");

	context->windowInterface.fMouseScroll(xoffset, yoffset);
}



/* interface */
vtek::ApplicationWindow* vtek::window_create(const vtek::WindowCreateInfo* info)
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwSwapInterval(1);

	// Allocate window
	auto [id, appWindow] = sAllocator.alloc();
	if (appWindow == nullptr)
	{
		vtek_log_fatal("Failed to allocate application window!");
		return nullptr;
	}
	appWindow->id = id;

	appWindow->glfwHandle = glfwCreateWindow(
		info->width, info->height, info->title, NULL, NULL);
	if (appWindow->glfwHandle == nullptr)
	{
		vtek_log_error("Failed to create GLFW window!");
		return nullptr;
	}

	// Get framebuffer size, needed when creating a swapchain. Description below:
	//
	// Window resolutions are measured in virtual screen coordinates,
	// but swap image sizes are measured in pixels. These do not always
	// correspond, e.g. on high DPI displays such as Apple's Retina.
	// So after the window is created, we can query GLFW for the
	// framebuffer size, which is always in pixels, and use this value
	// to determine an appropriate swap image size.
	glfwGetFramebufferSize(
		appWindow->glfwHandle, &appWindow->framebufferWidth, &appWindow->framebufferHeight);

	// Setup default event handlers
	window->fKeyCallback = default_key_handler;
	window->fMouseButtonCallback = default_mouse_button_callback;
	window->fMouseMoveCallback = default_mouse_move_callback;
	window->fMouseScrollCallback = default_mouse_scroll_callback;

	glfwSetWindowUserPointer(appWindow->glfwHandle, static_cast<void*>(appWindow));

	glfwSetKeyCallback(appWindow->glfwHandle, key_callback);
	glfwSetMouseButtonCallback(appWindow->glfwHandle, mouse_button_callback);
	glfwSetCursorPosCallback(appWindow->glfwHandle, mouse_move_callback);
	glfwSetScrollCallback(appWindow->glfwHandle, mouse_scroll_callback);

	// TODO: Do this properly.
	// Raw mouse motion is not affected by the scaling and acceleration applied
	// to the motion of the desktop cursor, hence more suitable for controlling
	// a 3D camera.
	bool cursorDisabled = (glfwGetInputMode(appWindow->glfwHandle, GLFW_CURSOR))
		== GLFW_CURSOR_DISABLED;
	if (cursorDisabled && glfwRawMouseMotionSupported())
	{
		glfwSetInputMode(appWindow->glfwHandle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	}

	return appWindow;
}

void vtek::window_destroy(vtek::ApplicationWindow* window)
{
	if (window == nullptr) { return; }

	if (window->glfwHandle != nullptr)
	{
		glfwDestroyWindow(window->glfwHandle);
	}

	sAllocator.free(window->id);
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

void vtek::window_get_framebuffer_size(vtek::ApplicationWindow* window, int* width, int* height)
{
	*width = window->framebufferWidth;
	*height = window->framebufferHeight;
}
