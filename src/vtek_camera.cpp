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

	glm::vec3 globalUp{0.0f, 0.0f, 1.0f}; // z-up is default

	glm::vec3 position {0.0f, 0.0f, 0.0f};
	glm::vec3 front {1.0f, 0.0f, 0.0f};
	glm::vec3 up {0.0f, 0.0f, 1.0f};
	glm::vec3 right {0.0f, 1.0f, 0.0f}; // TODO: left-handed or right-handed?

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
	if (camera == nullptr) return;

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

void vtek::camera_set_front(vtek::Camera* camera, glm::vec3 front)
{
	camera->front = glm::normalize(front);
	camera->orientation = glm::quat(0.0f, camera->front);
}

void vtek::camera_set_up(vtek::Camera* camera, glm::vec3 up)
{
	camera->up = glm::normalize(up);
}

void vtek::camera_set_y_up(vtek::Camera* camera)
{
	camera->globalUp = {0.0f, 1.0f, 0.0f};
}

void vtek::camera_set_z_up(vtek::Camera* camera)
{
	camera->globalUp = {0.0f, 0.0f, 1.0f};
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
	float xOffset = x - camera->lastX;
	float yOffset = y - camera->lastY;
	camera->lastX = x;
	camera->lastY = y;

	xOffset *= camera->mouseSensitivity;
	yOffset *= camera->mouseSensitivity;

	camera->rightAngle += xOffset;
	camera->upAngle += yOffset;

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
	// https://stackoverflow.com/questions/52413464/look-at-quaternion-using-up-vector

	// your code from before
	glm::vec3 F = glm::normalize(camera->front);
	glm::vec3 R = glm::normalize(glm::cross(F, camera->up));
	glm::vec3 U = glm::cross(R, F);

	camera->front = F;
	camera->right = R;
	camera->up = U;

	// note that R needed to be re-normalized
	// since F and worldUp are not necessary perpendicular
	// so must remove the sin(angle) factor of the cross-product
	// same not true for U because dot(R, F) = 0

	// adapted source
	glm::quat& q = camera->orientation;
	double trace = R.x + U.y + F.z;
	if (trace > 0.0) {
		double s = 0.5 / sqrt(trace + 1.0);
		q.w = 0.25 / s;
		q.x = (U.z - F.y) * s;
		q.y = (F.x - R.z) * s;
		q.z = (R.y - U.x) * s;
	} else {
		if (R.x > U.y && R.x > F.z) {
			double s = 2.0 * sqrt(1.0 + R.x - U.y - F.z);
			q.w = (U.z - F.y) / s;
			q.x = 0.25 * s;
			q.y = (U.x + R.y) / s;
			q.z = (F.x + R.z) / s;
		} else if (U.y > F.z) {
			double s = 2.0 * sqrt(1.0 + U.y - R.x - F.z);
			q.w = (F.x - R.z) / s;
			q.x = (U.x + R.y) / s;
			q.y = 0.25 * s;
			q.z = (F.y + U.z) / s;
		} else {
			double s = 2.0 * sqrt(1.0 + F.z - R.x - U.y);
			q.w = (R.y - U.x) / s;
			q.x = (F.x + R.z) / s;
			q.y = (F.y + U.z) / s;
			q.z = 0.25 * s;
		}
	}

	q = glm::normalize(q);
	camera->viewMatrix = glm::translate(glm::mat4_cast(q), -camera->position);

}



/*
void vtek::camera_update(vtek::Camera* camera)
{
	// camera roll
	// glm::quat temp()

	// glm::quat q = glm::angleAxis(-camera->upAngle, glm::vec3(1.0f, 0.0f, 0.0f));
	// q *= glm::angleAxis(camera->rightAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::quat& q = camera->orientation;
	q = glm::normalize(q);
	// camera->orientation = q;
	camera->viewMatrix = glm::translate(glm::mat4_cast(q), -camera->position);

	// camera->front.x = 2.0f*(q.x*q.z - q.w*q.y);
	// camera->front.y = 2.0f*(q.y*q.z + q.w*q.x);
	// camera->front.z = 1.0f - 2.0f*(q.x*q.x + q.y*q.y);
	// camera->up = glm::vec3(
	// 	2.0f * (q.x * q.y + q.w * q.z),
	// 	1.0f - 2.0f * (q.x * q.x + q.z * q.z),
	// 	2.0f * (q.y * q.z - q.w * q.x));

	// TODO: if (camera->rollEnabled) { }
	//camera->up = glm::rotate(camera->up, camera->rollAngle, camera->front);
	// camera->viewMatrix = glm::translate(glm::mat4_cast(q), -camera->position);
	// camera->viewMatrix = glm::rotate(camera->viewMatrix, camera->rollAngle, camera->front);


	// TODO: This doesn't affect the view matrix !
	// TODO: Rotate the orientation quaternion directly !

	camera->right = glm::normalize(glm::cross(camera->up, camera->front));
}
*/



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
	camera->position -= camera->front * distance;
}

void vtek::camera_move_backward(vtek::Camera* camera, float distance)
{
	camera->position += camera->front * distance;
}

void vtek::camera_roll_left_radians(vtek::Camera* camera, float angle)
{
	camera->rollAngle -= angle;
	if (camera->rollAngle > glm::two_pi<float>())
	{
		camera->rollAngle -= glm::two_pi<float>();
	}

	camera->up = glm::rotate(camera->up, angle, camera->front);
}

void vtek::camera_roll_right_radians(vtek::Camera* camera, float angle)
{
	camera->rollAngle += angle;
	if (camera->rollAngle > glm::two_pi<float>())
	{
		camera->rollAngle += glm::two_pi<float>();
	}

	camera->up = glm::rotate(camera->up, -angle, camera->front);
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
	glm::quat rotationQuaternion = glm::rotate(viewQuaternion, angle, View);

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
