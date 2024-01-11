#include "vtek_vulkan.pch"
#define VTEK_DISABLE_LOGGING
#include <vtek/vtek.hpp>
#include <iostream>

#include <boost/ut.hpp>
using namespace boost::ut;

void test_shader_stage_flags_graphics()
{
	vtek::EnumBitmask<vtek::ShaderStageGraphics> mask;
	mask.add_flag(vtek::ShaderStageGraphics::vertex);
	mask.add_flag(vtek::ShaderStageGraphics::tessellation_control);
	mask.add_flag(vtek::ShaderStageGraphics::tessellation_eval);
	mask.add_flag(vtek::ShaderStageGraphics::geometry);
	mask.add_flag(vtek::ShaderStageGraphics::fragment);

	VkShaderStageFlags flags = vtek::get_shader_stage_flags_graphics(mask);
	expect(flags & VK_SHADER_STAGE_VERTEX_BIT) << "no vertex bit!";
	expect(flags & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)
		<< "no tessellation control bit!";
	expect(flags & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
		<< "no tessellation eval bit!";
	expect(flags & VK_SHADER_STAGE_GEOMETRY_BIT) << "no geometry bit!";
	expect(flags & VK_SHADER_STAGE_FRAGMENT_BIT) << "no fragment bit!";
}

int main()
{
	"shader_tests"_test = []{
		test_shader_stage_flags_graphics();
	};
}
