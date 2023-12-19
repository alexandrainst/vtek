#include "vtek_vulkan.pch"
#include "vtek_camera.hpp"

#include "vtek_logging.hpp"


/* classes which control camera sub-logic */
class CameraProjectionBehaviour
{
public:
	virtual ~CameraProjectionBehaviour() {}
	virtual void CreateProjectionMatrix(vtek::Camera* camera) = 0;
	virtual void OnMouseScroll(vtek::Camera* camera, double x, double y) = 0;
	virtual void CameraMoveForward(vtek::Camera* camera, float dist) = 0;
	virtual void CameraMoveBackward(vtek::Camera* camera, float dist) = 0;
};

class CameraModeBehaviour
{
public:
	virtual ~CameraModeBehaviour() {}
	virtual void Init(vtek::Camera* camera, const vtek::CameraModeInfo* info) = 0;
	virtual void OnMouseMove(vtek::Camera* camera, double x, double y) = 0;
	virtual void CameraRoll(vtek::Camera* camera, float angle) = 0;
	virtual void Update(vtek::Camera* camera) = 0;
};



/* struct implementation */
struct vtek::Camera
{
	// mouse settings
	bool initialMove {true};
	float mouseSensitivity {0.001f};
	float mouseScrollSpeed {0.1f};
	float lastX {0.0f};
	float lastY {0.0f};
	glm::vec2 mouseScroll {0.0f, 0.0f};

	// Euler-angle constraints
	bool constrainPitch {false}; // TODO: This goes away.
	float pitchUpConstain {glm::half_pi<float>()}; // TODO: This goes away.
	float pitchDownConstrain {glm::half_pi<float>()}; // TODO: This goes away.
	glm::vec2 pitchClamp {glm::radians(-89.0f), glm::radians(89.0f)}; // OKAY: This instead.

	// camera vectors
	glm::vec3 position {0.0f, 0.0f, 0.0f};
	glm::vec3 orbitPoint {}; // TODO: Necessary?
	glm::vec3 front {1.0f, 0.0f, 0.0f};
	glm::vec3 up {0.0f, 0.0f, 1.0f};
	glm::vec3 right {0.0f, 1.0f, 0.0f};

	// orientation quaternion and camera matrices
	glm::quat orientation {0.0f, 1.0f, 0.0f, 0.0f};
	glm::mat4 viewMatrix {1.0f};
	glm::mat4 projectionMatrix {1.0f};

	// camera lens parameters
	vtek::FovClampRadians fov {glm::radians(45.0f)};

	// camera mode
	vtek::CameraMode mode {vtek::CameraMode::freeform};

	// external parameters
	glm::vec2 viewportSize {0.0f, 0.0f};
	glm::vec2 clippingPlanes {0.1f, 100.0f}; // { front, back }

	// Camera sub-logic behaviour
	CameraProjectionBehaviour* projectionBehaviour {nullptr};
	CameraModeBehaviour* modeBehaviour {nullptr};
};



/* Implementation of camera projection behaviour */
// TODO: Might do something like this to solve the handedness problem:
// class PerspectiveBehaviourLH : public CameraProjectionBehaviour {};
// class PerspectiveBehaviourRH : public CameraProjectionBehaviour {};

class PerspectiveBehaviour :  public CameraProjectionBehaviour
{
public:
	PerspectiveBehaviour(bool fovScroll)
	{
		this.fovScroll = fovScroll;
	}
	void CreateProjectionMatrix(vtek::Camera* camera) override
	{
		float fov = camera->fov.get();
		float w = camera->viewportSize.x;
		float h = camera->viewportSize.y;
		float near = camera->clippingPlanes.x;
		float far = camera->clippingPlanes.y;
		camera->projectionMatrix = glm::perspectiveFov(fov, w, h, near, far);

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
	void OnMouseScroll(vtek::Camera* camera, double x, double y) override
	{
		// NEXT: Convert this to static behaviour!
		if (fovScroll)
		{
			camera->fov -= y * camera->mouseScrollSpeed;
			CreateProjectionMatrix(camera);
		}
	}
	void CameraMoveForward(vtek::Camera* camera, float dist) override
	{
		camera->position += camera->front * dist;
	}
	void CameraMoveBackward(vtek::Camera* camera, float dist) override
	{
		camera->position -= camera->front * dist;
	}
private:
	bool fovScroll;
};

class OrthographicBehaviour : public CameraProjectionBehaviour
{
	void CreateProjectionMatrix(vtek::Camera* camera) override
	{
		float fov = camera->fov.get() + zoomOffset;
		float near = camera->clippingPlanes.x;
		float far = camera->clippingPlanes.y;
		float w = camera->viewportSize.x;
		float h = camera->viewportSize.y;
		float aspect = h / w;
		float width = 2.0f * (fov / glm::radians(45.0f));
		float height = 2.0f * aspect * (fov / glm::radians(45.0f));
		camera->projectionMatrix =
			glm::ortho(-width, width, -height, height, near, far);

		glm::mat4 correction(
			glm::vec4(1.0f,  0.0f, 0.0f, 0.0f),
			glm::vec4(0.0f, -1.0f, 0.0f, 0.0f),
			glm::vec4(0.0f,  0.0f, 0.5f, 0.0f),
			glm::vec4(0.0f,  0.0f, 0.5f, 1.0f));
		camera->projectionMatrix = correction * camera->projectionMatrix;
	}
	void OnMouseScroll(vtek::Camera* camera, double x, double y) override
	{
		camera->fov -= y * camera->mouseScrollSpeed;
		CreateProjectionMatrix(camera);
	}
	void CameraMoveForward(vtek::Camera* camera, float dist) override
	{
		zoomOffset -= dist * camera->mouseScrollSpeed * 2.0f;
		CreateProjectionMatrix(camera);
	}
	void CameraMoveBackward(vtek::Camera* camera, float dist) override
	{
		zoomOffset += dist * camera->mouseScrollSpeed * 2.0f;
		CreateProjectionMatrix(camera);
	}
private:
	float zoomOffset {0.0f};
};



/* Implementation of camera modes */
class FreeformModeBehaviour : public CameraModeBehaviour
{
	void Init(vtek::Camera* camera, const vtek::CameraModeInfo* info) override
	{

	}
	void OnMouseMove(vtek::Camera* camera, double x, double y) override
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
	void CameraRoll(vtek::Camera* camera, float angle) override
	{
		angle *= 0.5f;
		float cos = glm::cos(angle);
		float sin = glm::sin(angle);
		glm::quat rotor = glm::quat(cos, sin*glm::vec3(0, 0, -1));

		camera->orientation = glm::normalize(rotor * camera->orientation);
	};
	void Update(vtek::Camera* camera) override
	{
		const glm::quat& q = camera->orientation;
		camera->viewMatrix = glm::translate(glm::mat4_cast(q), camera->position);

		camera->front = glm::rotate(glm::conjugate(q), glm::vec3(0.0f, 0.0f, 1.0f));
		camera->up = glm::rotate(glm::conjugate(q), glm::vec3(0.0f, 1.0f, 0.0f));
		camera->right = glm::normalize(glm::cross(-camera->up, camera->front));
	}
};

class FpsModeBehaviour : public CameraModeBehaviour
{
	void Init(vtek::Camera* camera, const vtek::CameraModeInfo* info) override
	{

	}
	void OnMouseMove(vtek::Camera* camera, double x, double y) override
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
	void CameraRoll(vtek::Camera* camera, float angle) override {};
	void Update(vtek::Camera* camera) override
	{
		// float angle = glm::pi<float>();
		// auto newQuat = glm::angleAxis(angle, glm::vec3(0.0f, 0.0f, 1.0f)); // ?
	}
};

class FpsGroundedModeBehaviour : public CameraModeBehaviour
{
	void Init(vtek::Camera* camera, const vtek::CameraModeInfo* info) override
	{

	}
	void OnMouseMove(vtek::Camera* camera, double x, double y) override
	{

	}
	void CameraRoll(vtek::Camera* camera, float angle) override {};
	void Update(vtek::Camera* camera) override
	{

	}
};

class OrbitFreeModeBehaviour : public FreeformModeBehaviour
{
	void Init(vtek::Camera* camera, const vtek::CameraModeInfo* info) override
	{
		vtek_log_debug("OrbitFreeModeBehaviour::Init()");
		orbitDistance = info->orbitDistance;
		orbitDistanceClamp = info->orbitDistanceClamp;
		orbitPoint = camera->position + (camera->front * orbitDistance);
	}
	void Update(vtek::Camera* camera) override
	{
		orbitPoint = camera->position + (camera->front * orbitDistance);

		const glm::quat& q = camera->orientation;

		camera->front = glm::rotate(glm::conjugate(q), glm::vec3(0.0f, 0.0f, 1.0f));
		camera->up = glm::rotate(glm::conjugate(q), glm::vec3(0.0f, 1.0f, 0.0f));
		camera->right = glm::normalize(glm::cross(-camera->up, camera->front));

		camera->position = orbitPoint - (camera->front * orbitDistance);

		camera->viewMatrix = glm::translate(glm::mat4_cast(q), camera->position);
	}

private:
	float orbitDistance {1.0f};
	glm::vec2 orbitDistanceClamp {0.1f, 100.0f};
	glm::vec3 orbitPoint;
};

class OrbitFpsModeBehaviour : public CameraModeBehaviour
{
	void Init(vtek::Camera* camera, const vtek::CameraModeInfo* info) override
	{

	}
	void OnMouseMove(vtek::Camera* camera, double x, double y) override
	{

	}
	void CameraRoll(vtek::Camera* camera, float angle) override {};
	void Update(vtek::Camera* camera) override
	{

	}
};



/* set camera mode */
static void set_default_lookat(vtek::Camera* camera, glm::vec3 up, glm::vec3 front)
{
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
}

static void set_fps_lookat(vtek::Camera* camera, glm::vec3 upAxis, glm::vec3 front)
{
	// Check if length is valid; Also deals with NaN
	float frontLength = glm::length(front);
	if (frontLength <= 0.0001f)
	{
		camera->orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		return;
	}

	front /= frontLength; // normalize front vector

	// upAxis and front are nearly parallel, so find a better front vector
	if (glm::abs(glm::dot(upAxis, front)) > 0.9999f)
	{
		front = (glm::abs(glm::dot(upAxis, {0, 0, 1})) > 0.9999f)
			? glm::vec3(1, 0, 0) : glm::vec3(0, 0, 1);
	}

	camera->orientation = glm::conjugate(glm::quatLookAt(-front, upAxis));
}



/* interface */
vtek::Camera* vtek::camera_create(const vtek::CameraInfo* info)
{
	CameraModeInfo* modeInfo = info->modeInfo;
	CameraProjectionInfo* projInfo = info->projectionInfo;

	if (modeInfo == nullptr || projInfo == nullptr)
	{
		vtek_log_error(
			"vtek::camera_create(): {}",
			"Both modeInfo and projectionInfo must be provided!");
		return nullptr;
	}
	if (projInfo->viewportSize.x * projInfo->viewportSize.y == 0)
	{
		vtek_log_error("vtek::camera_create(): Viewport size must be set!");
		return nullptr;
	}
	auto camera = new vtek::Camera;

	// TODO: Accommodate for camera handedness.

	camera->position = info->position;
	camera->mode = modeInfo->mode;
	camera->viewportSize.x = static_cast<float>(projInfo->viewportSize.x);
	camera->viewportSize.y = static_cast<float>(projInfo->viewportSize.y);

	float near = glm::max(vtek::kNearClippingPlaneMin, projInfo->clipPlanes.x);
	float far = projInfo->clipPlanes.y;
	if (!(far > near))
	{
		vtek_log_warn(
			"Camera's far-clipping plane has a lower value than near -- {}",
			"far will be set to 'near + 100'!");
		far = near + 100.0f;
	}
	camera->clippingPlanes = { near, far };

	// If mouse scroll wheel will zoom in/out by modifying the camera's FOV.
	// This behaviour is disabled for orbiting cameras.
	bool fovScroll = true;

	// Camera orientation, ie. external matrix
	switch (modeInfo->mode)
	{
	case vtek::CameraMode::freeform:
		camera->modeBehaviour = new FreeformModeBehaviour();
		break;
	case vtek::CameraMode::fps:
		camera->modeBehaviour = new FpsModeBehaviour();
		break;
	case vtek::CameraMode::fps_grounded:
		camera->modeBehaviour = new FpsGroundedModeBehaviour();
		break;
	case vtek::CameraMode::orbit_free:
		camera->modeBehaviour = new OrbitFreeModeBehaviour();
		fovScroll = false;
		break;
	case vtek::CameraMode::orbit_fps:
		camera->modeBehaviour = new OrbitFpsModeBehaviour();
		fovScroll = false;
		break;
	default:
		vtek_log_error("vtek::camera_create(): Invalid camera mode!");
		delete camera;
		return nullptr;
	}
	set_default_lookat(camera, modeInfo->up, modeInfo->front); // TODO: Custom behaviour!
	camera->modeBehaviour->Init(camera, modeInfo);

	// Camera projection, ie. internal matrix
	if (projInfo->useFocalLength)
	{
		float frac = projInfo->sensorWidthMm / (2.0f * projInfo->focalLengthMm);
		camera->fov = 2.0f * glm::atan(frac);
	}
	else if (projInfo->fovFromAspectRatio)
	{
		float aspect = camera->viewportSize.y / camera->viewportSize.x;
		// TODO: Consider using `aspect * glm::radians(info->fovDegrees)` instead.
		camera->fov = aspect * glm::quarter_pi<float>();
	}
	else
	{
		camera->fov = glm::radians(projInfo->fovDegrees.get());
	}
	camera->projectionBehaviour =
		(projInfo->projection == vtek::CameraProjection::orthographic)
		? (CameraProjectionBehaviour*) (new OrthographicBehaviour())
		: (CameraProjectionBehaviour*) (new PerspectiveBehaviour(fovScroll));
	camera->projectionBehaviour->CreateProjectionMatrix(camera);

	return camera;
}

void vtek::camera_destroy(vtek::Camera* camera)
{
	if (camera == nullptr) return;

	delete camera->modeBehaviour;
	delete camera->projectionBehaviour;
	delete camera;
}


// ===================== //
// === Camera matrix === //
// ===================== //
void vtek::camera_change_projection(
	vtek::Camera* camera, const vtek::CameraProjectionInfo* info)
{

}

void vtek::camera_change_mode(
	vtek::Camera* camera, const vtek::CameraModeInfo* info)
{

}





// ===================== //
// === Camera update === //
// ===================== //
void vtek::camera_update(vtek::Camera* camera)
{
	camera->modeBehaviour->Update(camera);
}











// NEXT: Old interface

// ========================== //
// === Camera Orientation === //
// ========================== //

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
	camera->projectionBehaviour->CameraMoveForward(camera, distance);
}

void vtek::camera_move_backward(vtek::Camera* camera, float distance)
{
	camera->projectionBehaviour->CameraMoveBackward(camera, distance);
}

void vtek::camera_translate(vtek::Camera* camera, glm::vec3 offset)
{
	camera->position += offset;
}

void vtek::camera_on_mouse_move(vtek::Camera* camera, double x, double y)
{
	// TODO: Can we optimize this?
	if (camera->initialMove)
	{
		camera->initialMove = false;
		camera->lastX = x;
		camera->lastY = y;
	}
	camera->modeBehaviour->OnMouseMove(camera, x, y);
}

void vtek::camera_on_mouse_scroll(Camera* camera, double x, double y)
{
	camera->mouseScroll += glm::vec2(x, y) * camera->mouseScrollSpeed;
	camera->projectionBehaviour->OnMouseScroll(camera, x, y);
}



// ======================= //
// === Angular offsets === //
// ======================= //
void vtek::camera_roll_left_radians(vtek::Camera* camera, float angle)
{
	camera->modeBehaviour->CameraRoll(camera, angle);
}

void vtek::camera_roll_right_radians(vtek::Camera* camera, float angle)
{
	camera->modeBehaviour->CameraRoll(camera, -angle);
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
