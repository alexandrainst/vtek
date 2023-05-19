#pragma once

#include <vulkan/vulkan.h>
#include <type_traits>


namespace vtek
{
	// ========================= //
	// === Useful data types === //
	// ========================= //
	// This class can be used to capture a range, e.g. a depth range
	// which is required for viewport state during pipeline creation.
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

	// This class is used to clamp a value within a certain range, e.g.
	// when a queue's priority must be a number between 0 and 1, this
	// class can be templated as <0.0f, 1.0f>, and will then automatically
	// clamp its value inside this range.
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

	// Short wrapper for the conversion from C++ `bool` to an untyped
	// raw integer alias `VkBool32`, which many Vulkan functions and structs
	// expect. In such regard it's a type safe alternative to writing
	// `VK_TRUE` and `VK_FALSE`.
	class VulkanBool
	{
	public:
		inline VulkanBool(bool _b) : b(_b) {}
		inline VulkanBool& operator=(bool _b) { b = _b; return *this; }
		VkBool32 get() const { return static_cast<VkBool32>(b); }
	private:
		bool b;
	};

	// This class can be used to perform bitwise operations on the values
	// inside an enumeration, and removes the need for filling up the code
	// base with calls to `static_cast`. It provides a clean interface with
	// the member functions `get()`, `has_flag()`, and `clear()`.
	template<typename Enum>
	requires std::is_unsigned_v<std::underlying_type_t<Enum>>
	class EnumBitmask
	{
	public:
		using Type = std::underlying_type_t<Enum>;

		inline EnumBitmask() {}
		inline constexpr EnumBitmask(Type _mask) : mask(_mask) {}

		inline Type get() const { return mask; }
		inline bool has_flag(Enum e) const { return mask & static_cast<Type>(e); }
		inline bool empty() const { return mask == Type{0}; }
		inline void clear() { mask = {Type{0}}; }

		inline EnumBitmask& operator= (Enum e) {
			mask = static_cast<Type>(e);
			return *this;
		}
		inline EnumBitmask& operator|= (Enum e) {
			mask |= static_cast<Type>(e);
			return *this;
		}

		inline EnumBitmask operator| (Enum e) {
			return EnumBitmask{ mask | static_cast<Type>(e) };
		}

	private:
		Type mask {Type{0}};
	};
}
