#include "vtek_vulkan.pch"
#define VTEK_DISABLE_LOGGING
#include <vtek/vtek.hpp>
#include <random>
#include <iostream>

//
// vtek uses ut for unit testing. Description on github:
// https://github.com/boost-ext/ut
//
// Define what ut needs
std::ostream& operator<<(std::ostream& s, const glm::vec3& v)
{
	return s << '[' << v.x << ',' << v.y << ',' << v.z << ']';
}
#include <boost/ut.hpp>

using namespace boost::ut;

bool vec3_eq(const glm::vec3& v1, const glm::vec3& v2)
{
	constexpr float eps = glm::epsilon<float>();
	auto feq = [=](float f1, float f2){ return glm::abs(f1-f2) < eps; };
	return feq(v1.x, v2.x) && feq(v1.y, v2.y) && feq(v1.z, v2.z);
}



bool test_create_camera()
{
	vtek::Camera* cam = vtek::camera_create();
	bool ret = cam != nullptr;
	vtek::camera_destroy(cam);
	return ret;
}

void test_camera_default_lookat()
{
	vtek::Camera* cam = vtek::camera_create();
	glm::vec3 pos   {0.0f, 0.0f, 0.0f};
	glm::vec3 front {1.0f, 0.0f, 0.0f};
	glm::vec3 up    {0.0f, 0.0f, 1.0f};
	vtek::camera_set_lookat(cam, pos, front, up);

	"default_lookat"_test = [&pos, &front, &up, &cam]{
		glm::vec3 cpos = vtek::camera_get_position(cam);
		expect(vec3_eq(pos, cpos)) << "invalid position: " << pos << " != " << cpos;

		glm::vec3 cfront = vtek::camera_get_front(cam);
		expect(vec3_eq(front, cfront)) << "invalid front: " << front << " != " << cfront;

		glm::vec3 cup = vtek::camera_get_up(cam);
		expect(vec3_eq(up, cup)) << "invalid up: " << up << " != " << cup;
	};
}

void test_camera_custom_lookat(glm::vec3 front, glm::vec3 up)
{
	vtek::Camera* cam = vtek::camera_create();
	glm::vec3 pos   {0.0f, 0.0f, 0.0f};
	vtek::camera_set_lookat(cam, pos, front, up);

	glm::vec3 cpos = vtek::camera_get_position(cam);
	expect(vec3_eq(pos, cpos)) << "invalid position: " << pos << " != " << cpos;

	glm::vec3 cfront = vtek::camera_get_front(cam);
	expect(vec3_eq(front, cfront)) << "invalid front: " << front << " != " << cfront;

	glm::vec3 cup = vtek::camera_get_up(cam);
	expect(vec3_eq(up, cup)) << "invalid up: " << up << " != " << cup;
}



int main()
{
	// For randomized testing
	std::random_device rdev;
	std::default_random_engine re(rdev());
	constexpr int kNumPosTests = 100;

	"camera_tests"_test = []{
		expect(eq(test_create_camera(), "Camera was nullptr!"_b) >> fatal);
		test_camera_default_lookat();

		"custom_lookat"_test = [=] {
			test_camera_custom_lookat({-1.0f, 0.0f, 0.0f},{0.0f, 0.0f, 1.0f});
		// 	test_camera_custom_lookat({0.0f, 1.0f, 0.0f},{0.0f, 0.0f, 1.0f});
		// 	test_camera_custom_lookat({0.0f, -1.0f, 0.0f},{0.0f, 0.0f, 1.0f});
		};

		// Randomized parameters
		// for (int i = 0; i < kNumPosTests; i++)
		// {
		// }
	};
}
