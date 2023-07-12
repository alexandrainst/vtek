#include "vtek_vulkan.pch"
#include "vtek_camera.hpp"

#include "vtek_logging.hpp"


/* struct implementation */
struct vtek::Camera
{
	float rightAngle {0.0f};
	float upAngle {0.0f};
	float rollAngle {0.0f};
	float mouseSensitivity {0.001f};
	float lastX {0.0f};
	float lastY {0.0f};

	bool constrainPitch {false};
	float pitchUpConstain {glm::half_pi<float>()};
	float pitchDownConstrain {glm::half_pi<float>()};

	// viewing frustrum
	glm::vec2 windowSize {500.0, 500.0};
	float fov = glm::radians(45.0f);
	float near {0.1f}; // NOTE: This should never be 0!
	float far = {100.0f};

	glm::mat4 viewMatrix {1.0f};
	glm::mat4 projectionMatrix {1.0f};

	glm::vec3 position {0.0f, 0.0f, 0.0f};
	glm::vec3 front {1.0f, 0.0f, 0.0f};
	glm::vec3 up {0.0f, 0.0f, 1.0f};
	glm::vec3 right {0.0f, 1.0f, 0.0f};

	glm::quat orientation {0.0f, 1.0f, 0.0f, 0.0f};
};



/* interface */
vtek::Camera* vtek::camera_create()
{
	return new vtek::Camera;
}

void vtek::camera_destroy(vtek::Camera* camera)
{
	if (camera == nullptr) return;

	delete camera;
}



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
	camera->fov = glm::radians(fov_degrees);
	camera->near = near;
	camera->far = far;

	camera->projectionMatrix = glm::perspectiveFov(
		camera->fov, camera->windowSize.x, camera->windowSize.y,
		camera->near, camera->far);
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
		glm::vec3 alterntiveUp =
			(glm::abs(glm::dot(front, {1, 0, 0})) > 0.9999f)
			? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);

		camera->orientation = glm::quatLookAt(front, alterntiveUp);
	}
	else
	{
		camera->orientation = glm::quatLookAt(front, up);
	}

	camera->orientation = glm::normalize(camera->orientation);
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
	glm::quat& q = camera->orientation;
	camera->viewMatrix = glm::translate(glm::mat4_cast(q), -camera->position);

	camera->front = glm::rotate(glm::conjugate(q), glm::vec3(0.0f, 0.0f, -1.0f));
	camera->up = glm::rotate(glm::conjugate(q), glm::vec3(0.0f, 1.0f, 0.0f));
	camera->right = glm::normalize(glm::cross(camera->up, camera->front));
}

void vtek::camera_move_left(vtek::Camera* camera, float distance)
{
	camera->position -= camera->right * distance;
}

void vtek::camera_move_right(vtek::Camera* camera, float distance)
{
	camera->position += camera->right * distance;
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

void vtek::camera_roll_left_radians(vtek::Camera* camera, float angle)
{
	angle *= 0.5f;
	float cos = glm::cos(angle);
	float sin = glm::sin(angle);
	glm::quat rotor = glm::normalize(glm::quat(cos, sin*glm::vec3(0, 0, -1)));

	camera->orientation = glm::normalize(rotor * camera->orientation);
}

void vtek::camera_roll_right_radians(vtek::Camera* camera, float angle)
{
	angle *= -0.5f;
	float cos = glm::cos(angle);
	float sin = glm::sin(angle);
	glm::quat rotor = glm::normalize(glm::quat(cos, sin*glm::vec3(0, 0, -1)));

	camera->orientation = glm::normalize(rotor * camera->orientation);
}

void vtek::camera_on_mouse_move(vtek::Camera* camera, double x, double y)
{
	static bool first = true;
	if (first)
	{
		camera->lastX = x;
		camera->lastY = y;
		first = false;
	}

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
	glm::quat xRotor = glm::normalize(glm::quat(xCos, xSin*glm::vec3(0, 1, 0)));
	camera->orientation = glm::normalize(xRotor * camera->orientation);

	float yCos = glm::cos(-yOffset);
	float ySin = glm::sin(-yOffset);
	glm::quat yRotor = glm::normalize(glm::quat(yCos, ySin*glm::vec3(1, 0, 0)));
	camera->orientation = glm::normalize(yRotor * camera->orientation);
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
