#pragma once

#include <cstdint>


namespace vtek
{
	// =================== //
	// === Input types === //
	// =================== //
	enum class InputAction { press, release, repeat, ignore };

	enum class KeyboardKey : uint32_t {
		unknown,
		space, apostrophe, comma, minus, period, slash,
		num_0, num_1, num_2, num_3, num_4, num_5, num_6, num_7, num_8, num_9,
		semicolon, equal,
		a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z,
		left_bracket, backslash, right_bracket, grave_accent, world_1, world_2,
		escape, enter, tab, backspace, insert, del, right, left, down, up,
		page_up, page_down, home, end, caps_lock, scroll_lock, num_lock, print_screen, pause_break,
		f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11, f12,
		numpad_0, numpad_1, numpad_2, numpad_3, numpad_4, numpad_5, numpad_6, numpad_7, numpad_8, numpad_9,
		numpad_decimal, numpad_divide, numpad_multiply, numpad_subtract, numpad_add,
		numpad_enter, numpad_equal,
		left_shift, left_control, left_alt, left_super, right_shift, right_control, right_alt, right_super,
		menu
	};

	enum class MouseButton { left, middle, right, ignore };


	// =================== //
	// === Keyboard map == //
	// =================== //
	class KeyboardMap
	{
	public:
		static constexpr int kSize = 128;

		inline void press_key(KeyboardKey key) {
			map[static_cast<uint32_t>(key)] = true;
		}
		inline void release_key(KeyboardKey key) {
			map[static_cast<uint32_t>(key)] = false;
		}
		inline bool get_key(KeyboardKey key) {
			return map[static_cast<uint32_t>(key)];
		}
		inline void reset() {
			for (int i = 0; i < kSize; i++) { map[i] = false; }
		}
	private:
		bool map[kSize];
	};
}
