#include <blaze/color.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_random.hpp>

TEST_CASE("Color", "[color]") {
	auto gen = GENERATE(take(10, chunk(4, random(0, 255))));

	SECTION("Basic construction") {
		// the constructor with alpha omitted should default to the maximum alpha value
		auto color = Blaze::Color(gen[0], gen[1], gen[2]);
		REQUIRE(color.r == gen[0]);
		REQUIRE(color.g == gen[1]);
		REQUIRE(color.b == gen[2]);
		REQUIRE(color.a == 255);

		color = Blaze::Color(gen[0], gen[1], gen[2], gen[3]);
		REQUIRE(color.r == gen[0]);
		REQUIRE(color.g == gen[1]);
		REQUIRE(color.b == gen[2]);
		REQUIRE(color.a == gen[3]);
	}
}
