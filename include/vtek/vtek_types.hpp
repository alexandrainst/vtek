#pragma once

#include <vulkan/vulkan.h>
#include <type_traits>

#include "vtek_glm_includes.hpp"


namespace vtek
{
	// ========================= //
	// === Useful data types === //
	// ========================= //
	// This class can be used to capture a range, e.g. a depth range
	// which is required for viewport state during pipeline creation.
	// The range may be specified at runtime.
	template<class T>
	requires std::is_arithmetic_v<T>
	class ValueRange
	{
	public:
		ValueRange() : mmin(T{0}), mmax(T{0}) {}
		ValueRange(T _min, T _max)
			: mmin(std::min(_min, _max)), mmax(std::max(_min, _max)) {}
		ValueRange(const ValueRange<T>& _vr)
			: mmin(_vr.mmin), mmax(_vr.mmax) {}

		inline ValueRange<T>& operator= (const glm::tvec2<T>& vec) {
			mmin = std::min(vec.x, vec.y);
			mmax = std::max(vec.x, vec.y);
			return *this;
		}
		inline ValueRange<T>& operator= (const ValueRange<T>& _vr) {
			mmin = _vr.mmin;
			mmax = _vr.mmax;
			return *this;
		}

		T min() const { return mmin; }
		T max() const { return mmax; }
		inline T clamp(T _val) const { return glm::clamp(_val, mmin, mmax); }

	private:
		T mmin;
		T mmax;
	};

	// ValueRange specializations
	using FloatRange = ValueRange<float>;


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
		inline ValueClamp(const ValueClamp& _vc) { Assign(_vc.val); }

		inline ValueClamp& operator= (Type _val) {
			Assign(_val);
			return *this;
		}
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

		inline Type operator& (Enum e) {
			return mask & static_cast<Type>(e);
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
