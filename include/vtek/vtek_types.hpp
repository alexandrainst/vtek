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


	// This class is used to clamp a value within a certain range. Templated
	// so that instantiations may efficiently be created for various types
	// that support comparison operations.
	// When this object is constructed, given a value, it will automatically
	// clamp that value inside its templated boundaries, to ensure complete
	// run-time safety with no explicit bounds checking in applications.
	template<class Type, Type Min, Type Max, Type Default>
	class ValueClamp
	{
	public:
		inline ValueClamp() { Assign(Default); }
		inline ValueClamp(Type _val) { Assign(_val); }
		inline ValueClamp& operator= (Type _val) { Assign(_val); return *this; }
		inline ValueClamp& operator= (const ValueClamp& _vc) {
			Assign(_vc.val);
			return *this;
		}
		inline ValueClamp& operator+= (Type _val) {
			Assign(val + _val);
			return *this;
		}
		inline ValueClamp& operator-= (Type _val) {
			Assign(val - _val);
			return *this;
		}
		inline const Type& operator()() const { return val; }
		inline constexpr const Type& get() const { return val; }

		inline static constexpr Type min() { return Min; }
		inline static constexpr Type max() { return Max; }

	private:
		constexpr void Assign(Type _val) {
			val = (_val < Min) ? Min : (_val > Max) ? Max : _val;
		}
		Type val;
	};

	// Template specializations for `ValueClamp`:
	template<float Min, float Max>
	using FloatClamp = ValueClamp<float, Min, Max, 0.0f>;

	template<class Type, Type Min, Type Max>
	requires std::is_integral_v<Type>
	using IntClamp = ValueClamp<Type, Min, Max, Type{0}>;


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

	// NOTE: The addition of `std::is_convertible` is a hack necessary because
	// the bitwise OR-operator defined at the end of this file would otherwise
	// override the default-implementations of bitwise OR for regular enum
	// types, which then breaks compilation.
	// NOTE: This would be trivially fixed by upgrading to C++23 and then using
	// `std::is_scoped_enum` instead when defining this concept.
	template<typename Enum>
	concept enum_type = std::is_unsigned_v<std::underlying_type_t<Enum>> &&
		!std::is_convertible_v<Enum, uint32_t>;

	// This class can be used to perform bitwise operations on the values
	// inside an enumeration, and removes the need for filling up the code
	// base with calls to `static_cast`. It provides a clean interface with
	// the member functions `get()`, `has_flag()`, and `clear()`.
	template<enum_type Enum>
	class EnumBitmask
	{
	public:
		using Type = std::underlying_type_t<Enum>;

		inline EnumBitmask() {}
		inline constexpr EnumBitmask(Type _mask) : mask(_mask) {}

		inline Type get() const { return mask; }
		inline void add_flag(Enum e) { mask |= static_cast<Type>(e); }
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


// Bitwise OR-operation which creates an `EnumBitmask`:
template<vtek::enum_type Enum>
inline constexpr vtek::EnumBitmask<Enum> operator| (Enum s1, Enum s2)
{
	uint32_t flags = static_cast<uint32_t>(s1) | static_cast<uint32_t>(s2);
	return {flags};
}


// Increment(++) operators for `IntClamp`:
template<class Type, Type Min, Type Max>
vtek::IntClamp<Type, Min, Max>& operator++(vtek::IntClamp<Type, Min, Max>& ic)
{
	ic = ic.get() + 1; return ic;
}

template<class Type, Type Min, Type Max>
vtek::IntClamp<Type, Min, Max>& operator++(
	vtek::IntClamp<Type, Min, Max>& ic, auto)
{
	ic = ic.get() + 1; return ic;
}

// Decrement(--) operators for `IntClamp`:
template<class Type, Type Min, Type Max>
vtek::IntClamp<Type, Min, Max>& operator--(vtek::IntClamp<Type, Min, Max>& ic)
{
	if constexpr(std::is_unsigned_v<Type>) { // guard against overflow
		if (ic.get() == static_cast<Type>(0U)) { return ic; }
	}
	ic = ic.get() - 1; return ic;
}

template<class Type, Type Min, Type Max>
vtek::IntClamp<Type, Min, Max>& operator--(
	vtek::IntClamp<Type, Min, Max>& ic, auto)
{
	ic = ic.get() - 1; return ic;
}
