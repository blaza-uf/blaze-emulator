#pragma once

#include <cstdint>

namespace Blaze {
	struct Color {
		std::uint8_t r;
		std::uint8_t g;
		std::uint8_t b;
		std::uint8_t a;

		constexpr Color(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255): // NOLINT
			r(r), g(g), b(b), a(a) {};
	};
} // namespace Blaze
