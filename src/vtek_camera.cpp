#include "vtek_vulkan.pch"
#include "vtek_camera.hpp"


/* struct implementation */
struct vtek::Camera
{
	float rightAngle {0.0f};
	float upAngle {0.0f};

	glm::mat4 viewMatrix {1.0f};
};



void vtek::camera_set_orientation_degrees(
	vtek::Camera* camera, float rightAngle, float upAngle)
{
	camera->rightAngle = glm::radians(rightAngle);
	camera->upAngle = glm::radians(upAngle);
}

void vtek::camera_set_orientation_radians(
	vtek::Camera* camera, float rightAngle,float upAngle)
{
	camera->rightAngle = rightAngle;
	camera->upAngle = upAngle;
}

const glm::mat4* vtek::camera_get_view_matrix(vtek::Camera* camera)
{
	glm::quat q = glm::angleAxis(-camera->upAngle, glm::vec3(1.0f, 0.0f, 0.0f));
	q *= glm::angleAxis(camera->rightAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	camera->viewMatrix = glm::mat4_cast(q);

	return camera->viewMatrix;
}

void procedure()
{
	// glm::quatLookAt(View, Up); // TODO: Perhaps this can also be used.

	// To make a camera we typically need 3 vectors: Position, View, and Up.
	// For FPS camera we're only going to consider rotation the view vector.
	// With quaternions we can rotate a vector around an arbitrary axis very easily.
	// 1) Turn our View vector into a quaternion
	// 2) Define a rotation quaternion
	// 3) Apply the rotation quaternion to the view quaternion to make the rotation.

	glm::vec3 Position;
	glm::vec3 View; // aka. 'forward'
	glm::vec3 Up;

	glm::quat viewQuaternion = glm::quat(0.0f, View);


}

void test()
{
	// GLM_GTC_quaternion module:
	// https://glm.g-truc.net/0.9.4/api/a00153.html#gaa53e0e8933e176c6207720433fb8dd2b

	glm::quat q;
	glm::normalize(q); // -> quat; does q / glm::length(q)
	glm::inverse(q); // -> quat
	glm::conjugate(q); // -> quat

	glm::eulerAngles(q); // vec3; returns Euler angles {x,y,z} = {pitch, yaw, roll}.

	glm::length(q); // -> float

	// According to tutorial, and as mentioned in header file, this will rotate p:
	glm::vec3 p;
	glm::vec3 p_prime = q * p * glm::inverse(q);


	// Quaternion from Euler angles,
	// where angle is a glm::vec3 containing pitch, yaw, roll respectively:
	glm::quat myquaternion = glm::quat(glm::vec3(angle.x, angle.y, angle.z));
}
