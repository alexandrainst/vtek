#include "vtek_vulkan.pch"
#include "vtek_camera.hpp"

#include "vtek_logging.hpp"

#include <functional>

// Function declarations for implementing the camera modes
using tMouseMove = std::function<void(vtek::Camera*,double,double)>;
using tRoll = std::function<void(vtek::Camera*,float)>;


/* struct implementation */
struct vtek::Camera
{
	float mouseSensitivity {0.001f};
	float lastX {0.0f};
	float lastY {0.0f};

	bool constrainPitch {false}; // TODO: This goes away.
	float pitchUpConstain {glm::half_pi<float>()}; // TODO: This goes away.
	float pitchDownConstrain {glm::half_pi<float>()}; // TODO: This goes away.

	// viewing frustrum
	glm::vec2 windowSize {500.0, 500.0}; // TODO: This goes away.
	float fov {glm::radians(45.0f)};
	float near {vtek::kNearClippingPlaneMin};
	float far {100.0f};

	glm::mat4 viewMatrix {1.0f};
	glm::mat4 projectionMatrix {1.0f};

	glm::vec3 position {0.0f, 0.0f, 0.0f};
	glm::vec3 front {1.0f, 0.0f, 0.0f};
	glm::vec3 up {0.0f, 0.0f, 1.0f};
	glm::vec3 right {0.0f, 1.0f, 0.0f};

	glm::quat orientation {0.0f, 1.0f, 0.0f, 0.0f};

	// camera mode
	vtek::CameraMode mode {vtek::CameraMode::freeform};
	tMouseMove fMouseMove {};
	tRoll fRoll {};
};



/* camera modes */
static void on_mouse_move_none(vtek::Camera* camera, double x, double y) {}
static void roll_none(vtek::Camera*, float) {}

static void on_mouse_move_freeform(vtek::Camera* camera, double x, double y);
static void on_mouse_move_fps(vtek::Camera* camera, double x, double y);
static void on_mouse_move_fps_grounded(vtek::Camera* camera, double x, double y);
static void on_mouse_move_orbit_free(vtek::Camera* camera, double x, double y);
static void on_mouse_move_orbit_fps(vtek::Camera* camera, double x, double y);

static void on_mouse_move_initial(vtek::Camera* camera, double x, double y)
{
	camera->lastX = x;
	camera->lastY = y;

	switch (camera->mode)
	{
	case vtek::CameraMode::undefined:
		vtek_log_warn(
			"No mode has been specified for camera -- {}",
			"a default freeform orientation will be applied!");
		return;
	case vtek::CameraMode::freeform:
		camera->fMouseMove = on_mouse_move_freeform;
		on_mouse_move_freeform(camera, x, y);
		return;
	case vtek::CameraMode::fps:
		camera->fMouseMove = on_mouse_move_fps;
		on_mouse_move_fps(camera, x, y);
		return;
	case vtek::CameraMode::fps_grounded:
		camera->fMouseMove = on_mouse_move_fps_grounded;
		on_mouse_move_fps_grounded(camera, x, y);
		return;
	case vtek::CameraMode::orbit_free:
		camera->fMouseMove = on_mouse_move_orbit_free;
		on_mouse_move_orbit_free(camera, x, y);
		return;
	case vtek::CameraMode::orbit_fps:
		camera->fMouseMove = on_mouse_move_orbit_fps;
		on_mouse_move_orbit_fps(camera, x, y);
		return;

	default:
		vtek_log_error(
			"vtek_camera.cpp: on_mouse_move_initial: Invalid mode enum!");
		camera->fMouseMove = on_mouse_move_none;
		return;
	}
}

static void on_mouse_move_freeform(vtek::Camera* camera, double x, double y)
{
	float xOffset = x - camera->lastX;
	float yOffset = y - camera->lastY;
	camera->lastX = x;
	camera->lastY = y;

	xOffset *= camera->mouseSensitivity;
	yOffset *= camera->mouseSensitivity;

	float xCos = glm::cos(xOffset);
	float xSin = glm::sin(xOffset);
	glm::quat xRotor = glm::quat(xCos, xSin*glm::vec3(0, 1, 0));
	camera->orientation = glm::normalize(xRotor * camera->orientation);

	float yCos = glm::cos(yOffset);
	float ySin = glm::sin(yOffset);
	glm::quat yRotor = glm::quat(yCos, ySin*glm::vec3(1, 0, 0));
	camera->orientation = glm::normalize(yRotor * camera->orientation);
}

static void on_mouse_move_fps(vtek::Camera* camera, double x, double y)
{
	float xOffset = x - camera->lastX;
	float yOffset = y - camera->lastY;
	camera->lastX = x;
	camera->lastY = y;

	xOffset *= camera->mouseSensitivity;
	yOffset *= camera->mouseSensitivity;

	// TODO: Constrain pitch here
	if (camera->constrainPitch)
	{

	}

	// TODO: Perhaps also check if roll is disabled?
	// if(camera->disableRoll) { }

	float xCos = glm::cos(xOffset);
	float xSin = glm::sin(xOffset);
	glm::quat xRotor = glm::quat(xCos, xSin*glm::vec3(0, 1, 0));
	camera->orientation = glm::normalize(xRotor * camera->orientation);

	float yCos = glm::cos(yOffset);
	float ySin = glm::sin(yOffset);
	glm::quat yRotor = glm::quat(yCos, ySin*glm::vec3(1, 0, 0));
	camera->orientation = glm::normalize(yRotor * camera->orientation);
}

static void on_mouse_move_orbit(vtek::Camera* camera, double x, double y)
{

}

static void roll_freeform(vtek::Camera* camera, float angle)
{
	angle *= 0.5f;
	float cos = glm::cos(angle);
	float sin = glm::sin(angle);
	glm::quat rotor = glm::quat(cos, sin*glm::vec3(0, 0, -1));

	camera->orientation = glm::normalize(rotor * camera->orientation);
}



static void roll_orbit(vtek::Camera* camera, float angle)
{

}


static void set_mode_undefined(vtek::Camera* camera)
{
	camera->fMouseMove = []{}; // TODO: Possible?
}

static void set_mode_freeform(vtek::Camera* camera)
{
	camera->fMouseMove = on_mouse_move_freeform;
	camera->fRoll = roll_freeform;
}

static void set_mode_fps(vtek::Camera* camera)
{
	camera->fMouseMove = on_mouse_move_fps;
	camera->fRoll = roll_none;
}

static void set_mode_fps_grounded(vtek::Camera* camera)
{

}

static void set_mode_orbit_free(vtek::Camera* camera)
{

}

static void set_mode_orbit_fps(vtek::Camera* camera)
{

}



/* interface */
vtek::Camera* vtek::camera_create(const vtek::CameraInfo* info)
{
	auto camera = new vtek::Camera;

	// camera mode
	set_type_undefined(camera);

	return camera;
}

void vtek::camera_destroy(vtek::Camera* camera)
{
	if (camera == nullptr) return;

	delete camera;
}


// NEXT: Improved interface

void vtek::camera_set_perspective(
	vtek::Camera* camera, glm::uvec2 windowSize, float near, float far)
{

}

void vtek::camera_set_perspective(
	vtek::Camera* camera, glm::uvec2 windowSize, float near, float far,
	float fovDegrees)
{

}













// NEXT: Old interface

// ========================== //
// === Camera Orientation === //
// ========================== //
void vtek::camera_set_window_size(
	vtek::Camera* camera, uint32_t width, uint32_t height)
{
	camera->windowSize = glm::vec2((float)width, (float)height);

	camera->projectionMatrix = glm::perspectiveFov(
		camera->fov, camera->windowSize.x, camera->windowSize.y,
		camera->near, camera->far);
}

void vtek::camera_set_perspective_frustrum(
	vtek::Camera* camera, float fov_degrees, float near, float far)
{
	// TODO: Clamp the fov?
	camera->fov = glm::radians(fov_degrees);
	camera->near = (near < 0.1) ? 0.1 : near;
	camera->far = glm::max(camera->near, far);

	camera->projectionMatrix = glm::perspectiveFov(
		camera->fov, camera->windowSize.x, camera->windowSize.y,
		camera->near, camera->far);

	// Vulkan-trick because GLM was written for OpenGL, and Vulkan uses
	// a right-handed coordinate system instead. Without this correction,
	// geometry will be y-inverted in screen space, and the coordinate space
	// will be left-handed. Described at:
	// https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/
	glm::mat4 correction(
		glm::vec4(1.0f,  0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, -1.0f, 0.0f, 0.0f),
		glm::vec4(0.0f,  0.0f, 0.5f, 0.0f),
		glm::vec4(0.0f,  0.0f, 0.5f, 1.0f));
	camera->projectionMatrix = correction * camera->projectionMatrix;
}

void vtek::camera_set_lookat(
	vtek::Camera* camera, glm::vec3 pos, glm::vec3 front, glm::vec3 up)
{
	camera->position = pos;
	// NOTE: front = lookTo - lookFrom, perhaps, for orbitation camera!

	// Check if length is valid; Also deals with NaN
	float frontLength = glm::length(front);
	if (frontLength <= 0.0001f)
	{
		camera->orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		return;
	}

	front /= frontLength; // normalize front vector

	// Check if front and up vectors are (nearly) parallel
	if (glm::abs(glm::dot(front, up)) > 0.9999f)
	{
		// Find an alternative up vector, which is not (nearly) parallel to front
		glm::vec3 alternativeUp =
			(glm::abs(glm::dot(front, {1, 0, 0})) > 0.9999f)
			? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);

		camera->orientation =  glm::conjugate(glm::quatLookAt(-front, alternativeUp));
	}
	else
	{
		// "Because the rotation given by quatLookAt(target-eye, up) is the inverse
		// of the rotational part of lookAt(eye, center, up)"...
		// So we inverse (ie. conjugate on unit quaternion!) to get the correct
		// rotation. Described here:
		// https://github.com/g-truc/glm/pull/659
		camera->orientation = glm::conjugate(glm::quatLookAt(-front, up));
	}

	set_class_freeform(camera);
	vtek::camera_update(camera);
}

void vtek::camera_set_lookat_orbit(
	vtek::Camera* camera, glm::vec3 orbitPoint, glm::vec3 eulerAngles)
{
	vtek_log_error("vtek::camera_set_lookat_orbit(): Not implemented!");
}

void vtek::camera_set_constrain_pitch(
	vtek::Camera* camera, bool restrict, float angleUpDegrees, float angleDownDegrees)
{
	vtek_log_error("vtek::camera_set_constrain_pitch(): Not implemented!");
}

void vtek::camera_set_orientation_degrees(
	vtek::Camera* camera, float yaw, float pitch, float roll)
{
	vtek_log_error("vtek::camera_set_orientation_degrees(): Not implemented!");
}

void vtek::camera_set_orientation_radians(
	vtek::Camera* camera, float yaw, float pitch, float roll)
{
	vtek_log_error("vtek::camera_set_orientation_radians(): Not implemented!");
}



// ========================== //
// === Camera adjustments === //
// ========================== //
void vtek::camera_set_position(vtek::Camera* camera, glm::vec3 position)
{
	camera->position = position;
}

void vtek::camera_set_mouse_sensitivity(vtek::Camera* camera, float sensitivity)
{
	camera->mouseSensitivity = sensitivity;
}



// ================== //
// === User input === //
// ================== //
void vtek::camera_update(vtek::Camera* camera)
{
	const glm::quat& q = camera->orientation;
	camera->viewMatrix = glm::translate(glm::mat4_cast(q), camera->position);

	camera->front = glm::rotate(glm::conjugate(q), glm::vec3(0.0f, 0.0f, 1.0f));
	camera->up = glm::rotate(glm::conjugate(q), glm::vec3(0.0f, 1.0f, 0.0f));
	camera->right = glm::normalize(glm::cross(-camera->up, camera->front));
}

void vtek::camera_move_left(vtek::Camera* camera, float distance)
{
	camera->position += camera->right * distance;
}

void vtek::camera_move_right(vtek::Camera* camera, float distance)
{
	camera->position -= camera->right * distance;
}

void vtek::camera_move_up(vtek::Camera* camera, float distance)
{
	camera->position -= camera->up * distance;
}

void vtek::camera_move_down(vtek::Camera* camera, float distance)
{
	camera->position += camera->up * distance;
}

void vtek::camera_move_forward(vtek::Camera* camera, float distance)
{
	camera->position += camera->front * distance;
}

void vtek::camera_move_backward(vtek::Camera* camera, float distance)
{
	camera->position -= camera->front * distance;
}

void vtek::camera_translate(vtek::Camera* camera, glm::vec3 offset)
{
	camera->position += offset;
}

void vtek::camera_on_mouse_move(vtek::Camera* camera, double x, double y)
{
	camera->fMouseMove(camera, x, y);
}



// ======================= //
// === Angular offsets === //
// ======================= //
void vtek::camera_roll_left_radians(vtek::Camera* camera, float angle)
{
	camera->fRoll(camera, angle);
}

void vtek::camera_roll_right_radians(vtek::Camera* camera, float angle)
{
	camera->fRoll(camera, -angle);
}

void vtek::camera_pitch_up_radians(vtek::Camera* camera, float angle)
{

}

void vtek::camera_pitch_down_radians(vtek::Camera* camera, float angle)
{

}

void vtek::camera_bank_left_radians(vtek::Camera* camera, float angle)
{

}

void vtek::camera_bank_right_radians(vtek::Camera* camera, float angle)
{

}



// ======================= //
// === Retrieve values === //
// ======================= //
const glm::mat4* vtek::camera_get_view_matrix(vtek::Camera* camera)
{
	return &camera->viewMatrix;
}

const glm::mat4* vtek::camera_get_projection_matrix(vtek::Camera* camera)
{
	return &camera->projectionMatrix;
}

glm::quat vtek::camera_get_orientation_quat(vtek::Camera* camera)
{
	return camera->orientation;
}

glm::vec3 vtek::camera_get_orientation_euler(vtek::Camera* camera)
{
	return glm::eulerAngles(camera->orientation);
}

glm::vec3 vtek::camera_get_position(vtek::Camera* camera)
{
	return camera->position;
}

glm::vec3 vtek::camera_get_front(vtek::Camera* camera)
{
	return camera->front;
}

glm::vec3 vtek::camera_get_up(vtek::Camera* camera)
{
	return camera->up;
}
