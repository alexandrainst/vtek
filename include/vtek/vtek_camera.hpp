// Notes regarding quaternions:
//
// Demo application:
// https://eater.net/quaternions/video/intro

// A quaternion has a real number and 3 imaginary numbers.
// Quaternions are written in the form:
// q = s + xi + yj + zk
// It can also be written as an ordered pair:
// q = [s, v]   ,   s \in R, v \in R^3

// We can construct a rotor, that is a quaternion where i,j,k
// defines an orthonormal basis, and which may be used to rotate
// an arbitrary quaternion with 3 degrees of freedom.
//
// This method is inspired from using complex numbers to rotate
// a point in 2D, by defining the complex number as a rotor of the form:
// c = cos(t) + sin(t) i
// When used on a point p:
// p' = c * p
// This will rotate p t degrees counter-clockwise.
//
// Similarly, the quaternion rotor can be written in the form:
// q = cos(t) + sin(t) * (xi + yj + zk)
// The axis of rotation is defined by x,y,z; the angle of rotation by t.
// To rotate a point p t degrees around this axis:
// f(p) = q * p * q^{-1}

#pragma once

#include "vtek_glm_includes.hpp"
#include "vtek_object_handles.hpp"
#include "vtek_types.hpp"


namespace vtek
{
	enum class CameraHandedness
	{
		left_handed, right_handed
	};

	enum class CameraProjection
	{
		perspective, orthographic
	};

	enum class CameraMode
	{
		// A freeform camera has no restrictions on pitch/roll, and may perform
		// arbitrary rotations within 3 degrees of freedom.
		freeform,

		// A first-person shooter (FPS) camera has roll disabled, pitch clamped
		// in [-89, 89] degrees, and a universal "up" axis/direction.
		fps,

		// An fps-style camera with the additional restriction that all movement
		// is restricted to the horizontal plane, ie. changes in pitch does not
		// influence movement direction. The "horizontal" plane is specified as
		// the 2D plane which has the provided "upAxis" as normal.
		fps_grounded,

		// A freeform-orbiting camera has its view direction focused on a single
		// point-of-interest, with no restrictions on yaw/pitch/roll.
		orbit_free,

		// An fps-orbiting camera has the restrictions of an FPS-style camera,
		// while also being in orbiting mode. This is suitable when roll is
		// undesired.
		orbit_fps
	};


	// The closest allowed distance between the camera location and its near
	// clipping plane. Any lower (or negative) values will be clamped.
	constexpr float kNearClippingPlaneMin = 0.1f;

	// For clamping camera field-of-view (fov) to a sensible value [10,180].
	using FovClamp = FloatClamp<10.0f, 135.0f>;
	using FovClampRadians = FloatClamp<glm::radians(10.0f), glm::radians(135.0f)>;


	struct CameraSensitivityInfo
	{
		float mouseMoveSpeed {0.001f};
		float mouseScrollSpeed {0.1f};
	};

	struct CameraProjectionInfo
	{
		// Fields are documented in `CameraInfo`.
		CameraProjection projection {CameraProjection::perspective};

		// Size of the viewport, in pixels.
		glm::uvec2 viewportSize {0, 0};

		// Camera's near and far clip planes.
		glm::vec2 clipPlanes {0.1f, 100.0f};

		// Camera's field of view (FOV). Applies only to perspective projection.
		// 45 is considered the canonical value.
		FovClamp fovDegrees {45.0f};

		// TODO: Could alternatively provide float fov and an explicit clamp range!
		// float fovDegrees {45.0f};
		// glm::vec2 fovClampDegrees {10.0f, 135.0f};

		// Instead of explicit FOV, calculate it from the viewport aspect ratio.
		// This will result in the scene getting stretched according to viewport
		// dimensions, so that objects will always take up the same amount of
		// screen percentage.
		// NOTE: For quadratic viewports, fov=45, for larger widths smaller.
		bool fovFromAspectRatio {false};

		// As an alternative to explicitly providing a field of view, a sensor
		// width and a lens focal length may be provided instead, both in mm.
		// This will internally convert to FOV using this formula:
		// fov = 2 * atan(sensor_width / (2*focal_length)).
		// NOTE: Default parameters provided will result in an FOV of 45 degrees.
		bool useFocalLength {false};
		float focalLengthMm {12.071067823f};
		float sensorWidthMm {10.0f};
	};

	struct CameraOrbitInfo
	{
		// Initial distance between the camera and the orbiting point.
		float orbitDistance {1.0f};

		// Permitted distance range between camera and orbiting point.
		vtek::FloatRange orbitRange {vtek::FloatRange(0.1f, 100.0f)};
	};

	struct CameraFpsInfo
	{
		// Specifies range in which to clamp the camera pitch. No matter the
		// provided values, min and max will always be the [-89,89] range,
		// with min being the down angle and max being the up angle.
		vtek::FloatRange pitchRange {vtek::FloatRange(-89.0f, 89.0f)};
	};

	struct CameraInfo
	{
		// The camera mode, ie. the type of camera to simulate.
		CameraMode mode {CameraMode::freeform};

		// In order to suit the needs of various rendering platforms, we may
		// specify the handedness of the world space coordinate system in which
		// the camera is placed. This does not change the internal behaviour of
		// Vulkan, and no change in shaders is necessary.
		CameraHandedness worldSpaceHandedness {CameraHandedness::right_handed};

		// Starting position of the camera. For orbiting cameras, this reflects
		// the point-of-interest being looked at, instead of the actual camera
		// location which is computed automatically.
		glm::vec3 position {0.0f, 0.0f, 0.0f};

		// Front-facing direction of the camera.
		glm::vec3 front {1.0f, 0.0f, 0.0f};

		// Up-vector for the camera rotation. For fps-style cameras, this vector
		// is used as the "up"-axis (need not be axis-aligned).
		glm::vec3 up {0.0f, 0.0f, 1.0f};

		// Sensitivity info for how camera reacts to input. When not provided,
		// default values will be applied.
		CameraSensitivityInfo* sensitivityInfo {nullptr};

		// Camera projection, ie. internal matrix. When not provided, default
		// values will be applied.
		CameraProjectionInfo* projectionInfo {nullptr};

		// When camera mode is set to orbit, orbiting behaviour will be applied
		// from this struct, unless not provided in which case defaults will be
		// applied. Ignored for non-orbiting cameras.
		CameraOrbitInfo* orbitInfo {nullptr};
	};

	Camera* camera_create(const CameraInfo* info);
	void camera_destroy(Camera* camera);

	// NEXT: Improved interface

	// =================== //
	// === Camera mode === //
	// =================== //



	void camera_set_mode_freeform(Camera* camera, glm::vec3 up, glm::vec3 front);
	void camera_set_mode_fps(Camera* camera, glm::vec3 upAxis, glm::vec3 front);
	void camera_set_mode_fps_grounded(
		Camera* camera, glm::vec3 upAxis, glm::vec3 front);
	void camera_set_mode_orbit_free(
		Camera* camera, glm::vec3 front, glm::vec3 up, float distance);
	void camera_set_mode_orbit_fps(
		Camera* camera, glm::vec3 upAxis, glm::vec3 up, float distance);

	// Takes an extra parameter for clamping the pitch in [p,q], where both p and
	// q must be in [-89,89], and p < q.
	void camera_set_mode_fps(
		Camera* camera, glm::vec3 upAxis, glm::vec3 front,
		glm::vec2 pitchClampDegrees);


	// ===================== //
	// === Camera matrix === //
	// ===================== //

	// TODO: Not implemented!
	void camera_change_projection(Camera* camera, const CameraProjectionInfo* info);

	// TODO: Not implemented!
	// TODO: Specify interface?
	void camera_change_mode(Camera* camera, CameraMode mode);



	// ===================== //
	// === Camera update === //
	// ===================== //

	// Call this function every frame so that processed user input
	// may reorient the camera.
	void camera_update(Camera* camera);










	// NEXT: Old interface

	// ======================== //
	// === Camera behaviour === //
	// ======================== //

	void camera_set_constrain_pitch(
		Camera* camera, bool restrict, float angleUpDegrees, float angleDownDegrees);

	void camera_set_orientation_degrees(
		Camera* camera, float yaw, float pitch, float roll);
	void camera_set_orientation_radians(
		Camera* camera, float yaw, float pitch, float roll);

	// ========================== //
	// === Camera adjustments === //
	// ========================== //
	void camera_set_position(Camera* camera, glm::vec3 position);

	// A low value is recommended (0.001 is the default).
	void camera_set_mouse_sensitivity(Camera* camera, float sensitivity);

	// ================== //
	// === User input === //
	// ================== //
	void camera_move_left(Camera* camera, float distance);
	void camera_move_right(Camera* camera, float distance);
	void camera_move_up(Camera* camera, float distance);
	void camera_move_down(Camera* camera, float distance);
	void camera_move_forward(Camera* camera, float distance);
	void camera_move_backward(Camera* camera, float distance);

	void camera_translate(Camera* camera, glm::vec3 offset);

	void camera_on_mouse_move(Camera* camera, double x, double y);
	void camera_on_mouse_scroll(Camera* camera, double x, double y);

	// ======================= //
	// === Angular offsets === //
	// ======================= //
	void camera_roll_left_radians(Camera* camera, float angle);
	void camera_roll_right_radians(Camera* camera, float angle);

	void camera_pitch_up_radians(Camera* camera, float angle);
	void camera_pitch_down_radians(Camera* camera, float angle);

	void camera_bank_left_radians(Camera* camera, float angle);
	void camera_bank_right_radians(Camera* camera, float angle);

	// ======================= //
	// === Retrieve values === //
	// ======================= //
	const glm::mat4* camera_get_view_matrix(Camera* camera);
	const glm::mat4* camera_get_projection_matrix(Camera* camera);

	glm::quat camera_get_orientation_quat(Camera* camera);
	glm::vec3 camera_get_orientation_euler(Camera* camera);

	glm::vec3 camera_get_position(Camera* camera);
	glm::vec3 camera_get_front(Camera* camera);
	glm::vec3 camera_get_up(Camera* camera);

	CameraMode camera_get_mode(Camera* camera);
}
