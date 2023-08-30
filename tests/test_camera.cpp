#include <vtek/vtek.hpp>
#include <boost/ut.hpp>
#include <random>

using namespace boost::ut;

bool test_create_camera()
{
	vtek::Camera* cam = vtek::camera_create();
	bool ret = cam != nullptr;
	vtek::camera_destroy(cam);
	return ret;
}

bool vec3_eq(const glm::vec3& v1, const glm::vec3& v2)
{
	constexpr float eps = 0.000001f;
	auto feq = [](float f1, float f2){ return glm::abs(f1-f2) < eps; };
	return feq(v1.x, v2.x) && feq(v1.y, v2.y) && feq(v1.z, v2.z);
}

bool test_camera_default_lookat()
{
	bool success = true;

	vtek::Camera* cam = vtek::camera_create();
	glm::vec3 pos   {0.0f, 0.0f, 0.0f};
	glm::vec3 front {1.0f, 0.0f, 0.0f};
	glm::vec3 up    {0.0f, 0.0f, 1.0f};
	vtek::camera_set_lookat(cam, pos, front, up);

	glm::vec3 cpos   = vtek::camera_get_position(cam);
	glm::vec3 cfront = vtek::camera_get_front(cam);
	glm::vec3 cup    = vtek::camera_get_up(cam);

	"default_lookat"_test = [&pos, &cpos, &success]{
		expect(success = vec3_eq(pos, cpos));
	};

	return success;
}

bool test_create_camera_at_position(glm::vec3 pos)
{
	//vtek::Camera* cam = vtek::camera_create();
	return false;
}

int main()
{
	// For randomized testing
	std::random_device rdev;
	std::default_random_engine re(rdev());
	constexpr int kNumPosTests = 100;

	"camera_tests"_test = []{
		expect(test_create_camera()) << "Camera was nullptr!";
		expect(test_camera_default_lookat());
	};
}
