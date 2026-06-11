#include <catch2/catch_test_macros.hpp>

#include "ucf/Utilities/TimeUtils/Instant.h"
#include "ucf/Utilities/TimeUtils/LocalDate.h"
#include "ucf/Utilities/TimeUtils/LocalDateTime.h"
#include "ucf/Utilities/TimeUtils/TimeUtils.h"

#include <chrono>
#include <thread>


// =================== LocalDate ===================

TEST_CASE("LocalDate - construct and accessors", "[LocalDate]")
{
    ucf::utilities::LocalDate d{2026, 5, 21};
    REQUIRE(d.isValid());
    REQUIRE(d.year() == 2026);
    REQUIRE(d.month() == 5);
    REQUIRE(d.day() == 21);
    REQUIRE(d.toString() == "2026-05-21");
}

TEST_CASE("LocalDate - invalid date", "[LocalDate]")
{
    ucf::utilities::LocalDate bad{2026, 2, 30};
    REQUIRE_FALSE(bad.isValid());
}

TEST_CASE("LocalDate::parse", "[LocalDate][Parse]")
{
    REQUIRE(ucf::utilities::LocalDate::parse("2026-05-21").has_value());
    REQUIRE(ucf::utilities::LocalDate::parse("2026-05-21")->toString() == "2026-05-21");
    REQUIRE_FALSE(ucf::utilities::LocalDate::parse("2026-5-21").has_value());     // strict width
    REQUIRE_FALSE(ucf::utilities::LocalDate::parse("2026/05/21").has_value());    // wrong separator
    REQUIRE_FALSE(ucf::utilities::LocalDate::parse("2026-13-01").has_value());    // bad month
    REQUIRE_FALSE(ucf::utilities::LocalDate::parse("").has_value());
    REQUIRE_FALSE(ucf::utilities::LocalDate::parse("abcd-ef-gh").has_value());
}

TEST_CASE("LocalDate - weekday and dayOfYear", "[LocalDate]")
{
    // 2026-05-21 is a Thursday → ISO weekday 4.
    ucf::utilities::LocalDate d{2026, 5, 21};
    REQUIRE(d.weekday() == 4);
    // Jan 1 + 31 + 28 + 31 + 30 + 21 = 141.
    REQUIRE(d.dayOfYear() == 141);

    ucf::utilities::LocalDate jan1{2026, 1, 1};
    REQUIRE(jan1.dayOfYear() == 1);
}

TEST_CASE("LocalDate - arithmetic", "[LocalDate]")
{
    ucf::utilities::LocalDate base{2026, 5, 21};
    auto next = base + std::chrono::days{10};
    REQUIRE(next == ucf::utilities::LocalDate{2026, 5, 31});

    auto rollover = base + std::chrono::days{11};
    REQUIRE(rollover == ucf::utilities::LocalDate{2026, 6, 1});

    REQUIRE((next - base) == std::chrono::days{10});
    REQUIRE((base - next) == std::chrono::days{-10});
}

TEST_CASE("LocalDate - ordering", "[LocalDate]")
{
    ucf::utilities::LocalDate a{2026, 5, 21};
    ucf::utilities::LocalDate b{2026, 5, 22};
    ucf::utilities::LocalDate c{2026, 6, 1};

    REQUIRE(a < b);
    REQUIRE(b < c);
    REQUIRE(a < c);
    REQUIRE(c > a);
    REQUIRE_FALSE(a == b);
}

// =================== LocalDateTime ===================

TEST_CASE("LocalDateTime - construct and accessors", "[LocalDateTime]")
{
    ucf::utilities::LocalDateTime dt{ucf::utilities::LocalDate{2026, 5, 21}, 10, 30, 45, 123};
    REQUIRE(dt.date() == ucf::utilities::LocalDate{2026, 5, 21});
    REQUIRE(dt.hour() == 10);
    REQUIRE(dt.minute() == 30);
    REQUIRE(dt.second() == 45);
    REQUIRE(dt.millisecond() == 123);
}

TEST_CASE("LocalDateTime - format including %f", "[LocalDateTime][Format]")
{
    ucf::utilities::LocalDateTime dt{ucf::utilities::LocalDate{2026, 5, 21}, 10, 30, 45, 7};
    REQUIRE(dt.format("%Y-%m-%d %H:%M:%S") == "2026-05-21 10:30:45");
    REQUIRE(dt.format("%Y-%m-%d %H:%M:%S.%f") == "2026-05-21 10:30:45.007");
}

TEST_CASE("LocalDateTime - ordering", "[LocalDateTime][Compare]")
{
    ucf::utilities::LocalDateTime a{ucf::utilities::LocalDate{2026, 5, 21}, 10, 0, 0, 0};
    ucf::utilities::LocalDateTime b{ucf::utilities::LocalDate{2026, 5, 21}, 10, 0, 0, 1};
    ucf::utilities::LocalDateTime c{ucf::utilities::LocalDate{2026, 5, 21}, 10, 0, 1, 0};

    REQUIRE(a < b);
    REQUIRE(b < c);
}

// =================== Instant ===================

TEST_CASE("Instant - unix milliseconds roundtrip", "[Instant]")
{
    auto t = ucf::utilities::Instant::fromUnixMilliseconds(1747843845123LL);
    REQUIRE(t.toUnixMilliseconds() == 1747843845123LL);
    REQUIRE(t.toUnixSeconds() == 1747843845LL);
}

TEST_CASE("Instant - ISO 8601 roundtrip", "[Instant][ISO8601]")
{
    auto t = ucf::utilities::Instant::fromUnixMilliseconds(1747843845123LL);
    auto s = t.toISO8601();
    REQUIRE(s == "2025-05-21T16:10:45.123Z");

    auto back = ucf::utilities::Instant::parseISO8601(s);
    REQUIRE(back.has_value());
    REQUIRE(*back == t);
}

TEST_CASE("Instant::parseISO8601 - acceptance", "[Instant][ISO8601]")
{
    REQUIRE(ucf::utilities::Instant::parseISO8601("2026-05-21T10:30:00Z").has_value());
    REQUIRE(ucf::utilities::Instant::parseISO8601("2026-05-21T10:30:00.5Z").has_value());
    REQUIRE(ucf::utilities::Instant::parseISO8601("2026-05-21T10:30:00.123Z").has_value());

    REQUIRE_FALSE(ucf::utilities::Instant::parseISO8601("2026-05-21T10:30:00").has_value());
    REQUIRE_FALSE(ucf::utilities::Instant::parseISO8601("2026-05-21 10:30:00Z").has_value());
    REQUIRE_FALSE(ucf::utilities::Instant::parseISO8601("2026/05/21T10:30:00Z").has_value());
    REQUIRE_FALSE(ucf::utilities::Instant::parseISO8601("2026-05-21T10:30:00.1234Z").has_value());
    REQUIRE_FALSE(ucf::utilities::Instant::parseISO8601("").has_value());
}

TEST_CASE("Instant - ordering and arithmetic", "[Instant][Compare]")
{
    auto a = ucf::utilities::Instant::fromUnixMilliseconds(1000);
    auto b = ucf::utilities::Instant::fromUnixMilliseconds(2500);

    REQUIRE(a < b);
    REQUIRE(b - a == std::chrono::milliseconds{1500});
    REQUIRE((a + std::chrono::milliseconds{1500}) == b);
    REQUIRE((b - std::chrono::milliseconds{1500}) == a);
}

TEST_CASE("Instant - now monotonicity within a thread", "[Instant]")
{
    auto a = ucf::utilities::Instant::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    auto b = ucf::utilities::Instant::now();
    REQUIRE(b >= a);
}

TEST_CASE("Instant - toUTCDateTime", "[Instant]")
{
    auto t = ucf::utilities::Instant::fromUnixMilliseconds(1747843845123LL);
    auto dt = t.toUTCDateTime();
    REQUIRE(dt.date() == ucf::utilities::LocalDate{2025, 5, 21});
    REQUIRE(dt.hour() == 16);
    REQUIRE(dt.minute() == 10);
    REQUIRE(dt.second() == 45);
    REQUIRE(dt.millisecond() == 123);
}

// =================== TimeUtils helpers ===================

TEST_CASE("TimeUtils::formatSecondsToHMS", "[TimeUtils][Duration]")
{
    REQUIRE(ucf::utilities::TimeUtils::formatSecondsToHMS(0) == "00:00:00");
    REQUIRE(ucf::utilities::TimeUtils::formatSecondsToHMS(3661) == "01:01:01");
    REQUIRE(ucf::utilities::TimeUtils::formatSecondsToHMS(-5) == "00:00:00");
}

TEST_CASE("TimeUtils::formatSecondsToMS", "[TimeUtils][Duration]")
{
    REQUIRE(ucf::utilities::TimeUtils::formatSecondsToMS(0) == "00:00");
    REQUIRE(ucf::utilities::TimeUtils::formatSecondsToMS(125) == "02:05");
    REQUIRE(ucf::utilities::TimeUtils::formatSecondsToMS(-1) == "00:00");
}

TEST_CASE("TimeUtils::formatDuration", "[TimeUtils][Duration]")
{
    REQUIRE(ucf::utilities::TimeUtils::formatDuration(std::chrono::milliseconds(0)) == "00:00:00.000");
    REQUIRE(ucf::utilities::TimeUtils::formatDuration(std::chrono::milliseconds(123)) == "00:00:00.123");
    REQUIRE(ucf::utilities::TimeUtils::formatDuration(std::chrono::milliseconds(1000)) == "00:00:01.000");
    REQUIRE(ucf::utilities::TimeUtils::formatDuration(std::chrono::milliseconds(3661050)) == "01:01:01.050");
    REQUIRE(ucf::utilities::TimeUtils::formatDuration(-std::chrono::milliseconds(100)) == "00:00:00.000");
}

TEST_CASE("TimeUtils::formatDurationHuman", "[TimeUtils][Duration]")
{
    REQUIRE(ucf::utilities::TimeUtils::formatDurationHuman(std::chrono::milliseconds(0)) == "0 ms");
    REQUIRE(ucf::utilities::TimeUtils::formatDurationHuman(std::chrono::milliseconds(500)) == "500 ms");
    REQUIRE(ucf::utilities::TimeUtils::formatDurationHuman(std::chrono::milliseconds(1000)) == "1s");
    REQUIRE(ucf::utilities::TimeUtils::formatDurationHuman(std::chrono::milliseconds(125000)) == "2m 5s");
    REQUIRE(ucf::utilities::TimeUtils::formatDurationHuman(std::chrono::milliseconds(3661000)) == "1h 1m 1s");
    REQUIRE(ucf::utilities::TimeUtils::formatDurationHuman(-std::chrono::milliseconds(100)) == "0 ms");
}

TEST_CASE("TimeUtils::formatUTCTime - fixed pattern", "[TimeUtils][Format]")
{
    std::time_t t = static_cast<std::time_t>(1747843845);  // 2025-05-21 16:10:45 UTC
    REQUIRE(ucf::utilities::TimeUtils::formatUTCTime(t, "%Y-%m-%d %H:%M:%S") == "2025-05-21 16:10:45");
    REQUIRE(ucf::utilities::TimeUtils::formatUTCTime(t, "").empty());
}

TEST_CASE("TimeUtils - current time round-trip consistency", "[TimeUtils]")
{
    auto ms = ucf::utilities::TimeUtils::getCurrentUTCMilliseconds();
    auto sec = ucf::utilities::TimeUtils::getCurrentUTCSeconds();
    REQUIRE(ms >= sec * 1000);
    REQUIRE(ms <  sec * 1000 + 5000);   // wall-clock skew tolerance
}
