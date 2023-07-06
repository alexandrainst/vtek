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


namespace vtek
{
	struct Camera; // opaque handle

	Camera* camera_create();
	void camera_destroy(Camera* camera);

	void camera_set_orientation_degrees(Camera* camera, float rightAngle, float upAngle);
	void camera_set_orientation_radians(Camera* camera, float rightAngle, float upAngle);

	void camera_mouse_move_input(double x, double y);


	const glm::mat4* camera_get_view_matrix(Camera* camera);
	const glm::mat4& camera_get_projection_matrix(Camera* camera);
}
