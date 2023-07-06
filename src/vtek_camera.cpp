#include "vtek_vulkan.pch"
#include "vtek_camera.hpp"

#include "vtek_logging.hpp"


/* struct implementation */
struct vtek::Camera
{
	float rightAngle {0.0f};
	float upAngle {0.0f};
	float rollAngle {0.0f};
	float mouseSensitivity {0.01f};

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

	glm::vec3 position;
	glm::vec3 view;
	glm::vec3 up;

	float yaw {0.0f};
	float pitch {0.0f};

	glm::quat orientation {0.0f, 1.0f, 0.0f, 0.0f};
};



/* interface */
vtek::Camera* vtek::camera_create()
{
	return new vtek::Camera;
}

void vtek::camera_destroy(vtek::Camera* camera)
{
	delete camera;
}

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

void vtek::camera_set_position(vtek::Camera* camera, glm::vec3 position)
{
	camera->position = position;
}

void vtek::camera_set_y_up(vtek::Camera* camera)
{
	vtek_log_error("vtek::camera_set_y_up(): Not implemented!");
}

void vtek::camera_set_z_up(vtek::Camera* camera)
{
	vtek_log_error("vtek::camera_set_z_up(): Not implemented!");
}

void vtek::camera_set_orientation_degrees(
	vtek::Camera* camera, float rightAngle, float upAngle)
{
	camera->rightAngle = glm::radians(rightAngle);
	camera->upAngle = glm::radians(upAngle);
}

void vtek::camera_set_orientation_radians(
	vtek::Camera* camera, float rightAngle, float upAngle)
{
	camera->rightAngle = rightAngle;
	camera->upAngle = upAngle;
}

const glm::mat4* vtek::camera_get_view_matrix(vtek::Camera* camera)
{
	return &camera->viewMatrix;
}

const glm::mat4* vtek::camera_get_projection_matrix(vtek::Camera* camera)
{
	return &camera->projectionMatrix;
}



void vtek::camera_on_mouse_move(vtek::Camera* camera, double x, double y)
{
	x *= camera->mouseSensitivity;
	y *= camera->mouseSensitivity;

	camera->rightAngle += x;
	camera->upAngle += y;

	if (camera->constrainPitch)
	{
		camera->upAngle = glm::clamp(
			camera->upAngle,            // value
			camera->pitchDownConstrain, // min
			camera->pitchUpConstain);   // max
	}
}

void vtek::camera_update(vtek::Camera* camera)
{
	glm::quat q = glm::angleAxis(-camera->upAngle, glm::vec3(1.0f, 0.0f, 0.0f));
	q *= glm::angleAxis(camera->rightAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	camera->orientation = q;
	camera->viewMatrix = glm::mat4_cast(q);
}





/*
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

	// We need a vector to rotate about, and the angle to rotate by.
	glm::quat rotationQuaternion =

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
	// NOTE: Since q is (hopefully) a unit-quaternion, we can equally write:
	glm::vec3 p_prime = q * p * glm::conjugate(q); // <-- NOTE: We should use this!


	// Quaternion from Euler angles,
	// where angle is a glm::vec3 containing pitch, yaw, roll respectively:
	glm::quat myquaternion = glm::quat(glm::vec3(angle.x, angle.y, angle.z));
}




static void construction()
{
	// 1) Create a quaternion to represent the initial orientation of the camera.
	// This quaternion will store the camera's rotation.

	// 2) Initialize the camera position, and other relevant fields such as FOV, aspect, etc.

	// 3) Capture keyboard inputs to move the camera.

	// 4) Capture mouse input and convert X and Y into yaw and pitch values.

	// 5) Update the quaternion representing the camera's rotation based on received input.

	// 6) Construct a transformation matrix from the quaternion representing the camera's rotation.

	// 7) Combine the transformation matrix with the camera's position to form the camera's view matrix.

	// 8) The view matrix transforms the world coordinates into camera-relative coordinates.

	// A) Ensure that the camera quaternion is kept normalized to maintain its mathematic properties.
}




void vtek::camera_update(vtek::Camera* camera)
{
	// 1) SetViewByMouse
	// 2) RotateCamera
	// 3) gluLookAt
	camera->viewMatrix = glm::lookAt(camera->view, camera->position, camera->up);


}



void Camera::SetViewByMouse(void)
{
	// the coordinates of our mouse coordinates
	int MouseX, MouseY;
	// the middle of the screen in the x direction
	int MiddleX = SCREENWIDTH/2;
	// the middle of the screen in the y direction
	int MiddleY = SCREENHEIGHT/2;
	// vector that describes mouseposition - center
	Vector MouseDirection(0, 0, 0);
	// static variable to store the rotation about the x-axis, since
	// we want to limit how far up or down we can look.
	// We don't need to cap the rotation about the y-axis as we
	// want to be able to turn around 360 degrees
	static double CurrentRotationAboutX = 0.0;
	// The maximum angle we can look up or down, in radians
	double maxAngle = 1;
	// This function gets the position of the mouse
	SDL_GetMouseState(&MouseX, &MouseY);
	// if the mouse hasn't moved, return without doing
	// anything to our view
	if((MouseX == MiddleX) && (MouseY == MiddleY)) return;
	// otherwise move the mouse back to the middle of the screen
	SDL_WarpMouse(MiddleX, MiddleY);
	// get the distance and direction the mouse moved in x (in
	// pixels). We can't use the actual number of pixels in radians,
	// as only six pixels would cause a full 360 degree rotation.
	// So we use a mousesensitivity variable that can be changed to
	// vary how many radians we want to turn in the x-direction for
	// a given mouse movement distance
	// We have to remember that positive rotation is counter-clockwise.
	// Moving the mouse down is a negative rotation about the x axis
	// Moving the mouse right is a negative rotation about the y axis
	MouseDirection.x = (MiddleX - MouseX)/MouseSensitivity;
	MouseDirection.y = (MiddleY - MouseY)/MouseSensitivity;
	CurrentRotationX += MouseDirection.y;
	// We don't want to rotate up more than one radian, so we cap it.
	if(CurrentRotationX > 1) { CurrentRotationX = 1; return; }
	// We don't want to rotate down more than one radian, so we cap it.
	if(CurrentRotationX < -1) { CurrentRotationX = -1; return; } else {
		// get the axis to rotate around the x-axis.
		Vector Axis = CrossProduct(View - Position, Up);
		// To be able to use the quaternion conjugate, the axis to
		// rotate around must be normalized.
		Axis = Normalize(Axis);
		// Rotate around the y axis
		RotateCamera(MouseDirection.y, Axis.x, Axis.y, Axis.z);
		// Rotate around the x axis
		RotateCamera(MouseDirection.x, 0, 1, 0);
	}
}

void RotateCamera(double Angle, double x, double y, double z)
{
	quaternion temp, quat_view, result;
	temp.x = x * sin(Angle/2);
	temp.y = y * sin(Angle/2);
	temp.z = z * sin(Angle/2);
	temp.w = cos(Angle/2);
	quat_view.x = View.x;
	quat_view.y = View.y;
	quat_view.z = View.z;
	quat_view.w = 0;
	result = mult(mult(temp, quat_view), conjugate(temp));
	View.x = result.x;
	View.y = result.y;
	View.z = result.z;
}
*/
