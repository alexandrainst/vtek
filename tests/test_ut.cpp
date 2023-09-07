#include <vtek/vtek.hpp>
#include <boost/ut.hpp>

vtek::Instance* test_create_instance()
{
	vtek::InstanceInfo info{};
	info.applicationName = "vtek_unit_test";
	info.enableValidationLayers = true;
	return vtek::instance_create(&info);
}

vtek::PhysicalDevice* test_pick_physical_device(const vtek::Instance* instance)
{
	vtek::PhysicalDeviceInfo info{};
	info.requireGraphicsQueue = true;
	info.requirePresentQueue = true;
	info.requireSwapchainSupport = true;
	info.requireDynamicRendering = true;
	return vtek::physical_device_pick(&info, instance);
}

vtek::Device* test_create_device(
	const vtek::Instance* instance, const vtek::PhysicalDevice* physicalDevice)
{
	vtek::DeviceInfo info{};
	return vtek::device_create(&info, instance, physicalDevice);
}

/*
void test_vulkan()
{
	"vulkan_graphics_setup"_test = [] {
		auto inst = test_create_instance();
		(expect(inst != nullptr) << "Vulkan instance is nullptr!") >> fatal;

		auto physDev = test_pick_physical_device(inst);
		expect(physDev != nullptr) << "Vulkan physical device pick is nullptr!";

		auto dev = test_create_device(inst, physDev);
		expect(dev != nullptr) << "Vulkan device is nullptr!";
	};
}

auto main(int argc, const char** argv) -> int
{
	"empty"_test         = []{};
	"single"_test        = []{};
	"many"_test          = []{
		"equal"_test     = []{};
		"not equal"_test = []{};
	};

	"don't run"_test = [] {
		expect(42_i == 43) << "should not fire!";
	};

	cfg<override> = {.tag = {"nightly"}};

	return ut_main(argc, argv);
}
*/

constexpr auto sum(auto... args) { return (args + ...); }

int main() {
	using namespace boost::ut;

	"sum"_test = [] {
		expect(sum(0) == 0_i);
		expect(sum(1, 2) == 3_i);
		expect(sum(1, 2) > 0_i and 41_i == sum(40, 2));
	};
}
