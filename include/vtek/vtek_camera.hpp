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

	void camera_set_window_size(Camera* camera, uint32_t width, uint32_t height);

	void camera_set_perspective_frustrum(
		Camera* camera, float fov_degrees, float near, float far);

	void camera_set_position(Camera* camera, glm::vec3 position);
	void camera_set_front(Camera* camera, glm::vec3 front);
	void camera_set_up(Camera* camera, glm::vec3 up);

	void camera_set_orientation_degrees(Camera* camera, float rightAngle, float upAngle);
	void camera_set_orientation_radians(Camera* camera, float rightAngle, float upAngle);

	void camera_set_constrain_pitch(
		Camera* camera, bool restrict, float angleUpDegrees, float angleDownDegrees);

	void camera_set_y_up(Camera* camera);
	void camera_set_z_up(Camera* camera);

	void camera_set_perspective_frustrum(
		Camera* camera, float fov_degrees, float near, float far);

	/*
	enum class MovementDirection
	{
		left, right, forward, backward, up, down
	};

	// TODO: Perhaps this is too simplified, because what for movement spped,
	// which might be individual for all movement directions.
	void camera_add_movement(Camera* camera, EnumBitmask<MovementDirection> moveDir);
	*/

	void camera_move_left(Camera* camera, float distance);
	void camera_move_right(Camera* camera, float distance);
	void camera_move_up(Camera* camera, float distance);
	void camera_move_down(Camera* camera, float distance);
	void camera_move_forward(Camera* camera, float distance);
	void camera_move_backward(Camera* camera, float distance);

	void camera_roll_left_radians(Camera* camera, float angle);
	void camera_roll_right_radians(Camera* camera, float angle);

	void camera_on_mouse_move(Camera* camera, double x, double y);

	void camera_update(Camera* camera);

	const glm::mat4* camera_get_view_matrix(Camera* camera);
	const glm::mat4* camera_get_projection_matrix(Camera* camera);

	glm::quat camera_get_orientation(Camera* camera);
}
