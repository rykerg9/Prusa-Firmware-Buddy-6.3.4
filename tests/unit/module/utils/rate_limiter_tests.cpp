#include <catch2/catch.hpp>
#include <utils/timing/rate_limiter.hpp>

TEST_CASE("RateLimiter::uint8_t") {
    // First of all, check that math still works
    CHECK(1 + 1 == 2);

    RateLimiter<uint8_t> limiter(50);

    // No previous event - cooldown should be zero in all cases
    CHECK(limiter.remaining_cooldown(0) == 0);
    CHECK(limiter.remaining_cooldown(5) == 0);
    CHECK(limiter.remaining_cooldown(127) == 0);

    // First check should always return true
    CHECK(limiter.check(1));
    CHECK(limiter.remaining_cooldown(1) == 50);
    CHECK(limiter.remaining_cooldown(50) == 1);
    CHECK(limiter.remaining_cooldown(51) == 0);
    CHECK(limiter.remaining_cooldown(52) == 0);

    // Just on the verge - should work again
    CHECK(limiter.check(51));
    CHECK(limiter.remaining_cooldown(51) == 50);
    CHECK(limiter.remaining_cooldown(71) == 30);

    // Nope
    CHECK(!limiter.check(52));
    CHECK(!limiter.check(100));

    CHECK(limiter.check(101));

    // Check wrapping
    CHECK(limiter.check(250));
    CHECK(limiter.remaining_cooldown(250) == 50);
    CHECK(limiter.remaining_cooldown(255) == 45);
    CHECK(limiter.remaining_cooldown(0) == 44);
    CHECK(limiter.remaining_cooldown(43) == 1);
    CHECK(limiter.remaining_cooldown(44) == 0);

    CHECK(!limiter.check(43));
    CHECK(limiter.check(44));

    // Check that math still works after doing our tests
    // One can never be too sure
    CHECK(2 * 2 == 4);
}
