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
// To rotate a point p t degrees in this axis:
// f(p) = q * p * q^{-1}

#pragma once

#include "vtek_glm_includes.hpp"

#include <cstdint>


namespace vtek
{
	struct Camera; // opaque handle

	// TODO: Info struct?
	Camera* camera_create();
	void camera_destroy(Camera* camera);

	// ======================== //
	// === Camera Behaviour === //
	// ======================== //
	void camera_set_window_size(Camera* camera, uint32_t width, uint32_t height);

	void camera_set_perspective_frustrum(
		Camera* camera, float fov_degrees, float near, float far);

	// Default behaviour: No restriction on pitch, and roll is enabled.
	void camera_set_lookat(
		Camera* camera, glm::vec3 pos, glm::vec3 front, glm::vec3 up);

	// The camera will perform in orbiting mode, instead of maintaining a particular
	// view direction.
	// TODO: This should probably disable movement and translation.
	void camera_set_lookat_orbit(
		Camera* camera, glm::vec3 orbitPoint, glm::vec3 eulerAngles);

	// Alternative behaviour: Restrict pitch, disable roll, and specify up vector
	// for an FPS-game style camera.
	void camera_set_lookat_fps(
		Camera* camera, glm::vec3 pos, glm::vec3 front, glm::vec3 up);

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

	// Call this function every frame so that processed user input
	// may reorient the camera.
	void camera_update(Camera* camera);

	void camera_move_left(Camera* camera, float distance);
	void camera_move_right(Camera* camera, float distance);
	void camera_move_up(Camera* camera, float distance);
	void camera_move_down(Camera* camera, float distance);
	void camera_move_forward(Camera* camera, float distance);
	void camera_move_backward(Camera* camera, float distance);

	void camera_translate(Camera* camera, glm::vec3 offset);

	void camera_roll_left_radians(Camera* camera, float angle);
	void camera_roll_right_radians(Camera* camera, float angle);

	void camera_on_mouse_move(Camera* camera, double x, double y);

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
}
