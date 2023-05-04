#pragma once

#include <vulkan/vulkan.h>
#include <type_traits>


namespace vtek
{
	// ========================= //
	// === Useful data types === //
	// ========================= //
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

	template<typename Enum>
	requires std::is_unsigned_v<std::underlying_type_t<Enum>>
	class EnumBitflag
	{
	public:
		using Type = std::underlying_type_t<Enum>;

		inline EnumBitflag() {}
		inline EnumBitflag(Type _flag) : flag(_flag) {}

		inline Type get() { return flag; }
		inline bool has_flag(Enum e) { return flag & static_cast<Type>(e); }
		inline void clear() { flag = {Type{0}}; }

		inline EnumBitflag& operator= (Enum e) { flag = static_cast<Type>(e); }
		inline EnumBitflag& operator|= (Enum e) {
			flag |= static_cast<Type>(e);
			return *this;
		}

	private:
		Type flag {Type{0}};
	};
}
