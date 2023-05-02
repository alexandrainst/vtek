#pragma once

#include <vulkan/vulkan.h>


namespace vtek
{
	// ============================= //
	// === Opaque vulkan handles === //
	// ============================= //
	struct ApplicationWindow;
	struct CommandBuffer;
	struct CommandPool;
	struct Device;
	struct GraphicsPipeline;
	struct GraphicsShader;
	struct Instance;
	struct PhysicalDevice;
	struct Queue;
	struct RenderPass;
	struct Swapchain;
	// TODO: struct SwapchainFramebuffers;



	// ========================= //
	// === Useful data types === //
	// ========================= //
	// TODO: For increased compilation speed place these somewhere else.
	class FloatRange
	{
	public:
		FloatRange() : fmin(0.0f), fmax(0.0f) {}
		FloatRange(float f1, float f2) {
			fmin = (f1 < f2) ? f1 : f2;
			fmax = (f1 > f2) ? f1 : f2;
		}
		float min() const { return fmin; }
		float max() const { return fmax; }
	private:
		float fmin, fmax;
	};


	template<float Min, float Max>
	class FloatClamp
	{
	public:
		FloatClamp(float _val) {
			val = (_val < Min) ? Min : (_val > Max) ? Max : _val;
		}
		float get() const { return val; }
	private:
		float val;
	};

	class VulkanBool
	{
	public:
		inline VulkanBool(bool _b) : b(_b) {}
		inline VulkanBool& operator=(bool _b) { b = _b; return *this; }
		VkBool32 get() const { return static_cast<VkBool32>(b); }
	private:
		bool b;
	};
}
