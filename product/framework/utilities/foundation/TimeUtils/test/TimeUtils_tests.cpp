#include <catch2/catch_test_macros.hpp>

// Public headers intentionally compile without relying on an include order.
#include <ucf/utilities/TimeUtils/TimeUtils.h>
#include <ucf/utilities/TimeUtils/CivilTime.h>
#include <ucf/utilities/TimeUtils/Clock.h>
#include <ucf/utilities/TimeUtils/Instant.h>
#include <ucf/utilities/TimeUtils/TimeResult.h>
#include <ucf/utilities/TimeUtils/TimeZone.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <limits>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

using namespace std::chrono_literals;
using ucf::utilities::AmbiguousTimePolicy;
using ucf::utilities::IClock;
using ucf::utilities::Instant;
using ucf::utilities::LocalDate;
using ucf::utilities::LocalDateTime;
using ucf::utilities::LocalTime;
using ucf::utilities::LocalTimeResolvePolicy;
using ucf::utilities::NonexistentTimePolicy;
using ucf::utilities::SystemClock;
using ucf::utilities::TimeErrorCode;
using ucf::utilities::TimeResult;
using ucf::utilities::TimeUtils;
using ucf::utilities::TimeZone;
using ucf::utilities::TimeZoneKind;
using ucf::utilities::UtcOffset;

template<typename T>
T requireValue(TimeResult<T> result)
{
    if (!result)
    {
        throw std::runtime_error{result.error().diagnostic};
    }
    return std::move(result).value();
}

LocalDate date(int year, unsigned month, unsigned day)
{
    return requireValue(LocalDate::create(year, month, day));
}

LocalTime time(
    unsigned hour,
    unsigned minute,
    unsigned second,
    unsigned millisecond = 0)
{
    return requireValue(LocalTime::create(hour, minute, second, millisecond));
}

LocalDateTime dateTime(
    int year,
    unsigned month,
    unsigned day,
    unsigned hour,
    unsigned minute,
    unsigned second,
    unsigned millisecond = 0)
{
    return LocalDateTime{
        date(year, month, day),
        time(hour, minute, second, millisecond)};
}

class FixedClock final : public IClock
{
public:
    explicit FixedClock(Instant value)
        : mValue{value}
    {
    }

    Instant now() const noexcept override
    {
        return mValue;
    }

private:
    Instant mValue;
};

} // namespace

TEST_CASE("Civil values are valid by construction", "[TimeUtils][Civil]")
{
    const auto leapDay = LocalDate::create(2024, 2, 29);
    REQUIRE(leapDay);
    REQUIRE(leapDay.value().weekday() == 4);
    REQUIRE(leapDay.value().dayOfYear() == 60);
    REQUIRE(leapDay.value().toIsoString() == "2024-02-29");

    const auto invalidDate = LocalDate::create(2023, 2, 29);
    REQUIRE_FALSE(invalidDate);
    REQUIRE(invalidDate.error().code == TimeErrorCode::InvalidDate);
    const auto invalidYear = LocalDate::create(40000, 1, 1);
    REQUIRE_FALSE(invalidYear);
    REQUIRE(invalidYear.error().code == TimeErrorCode::OutOfRange);
    REQUIRE_FALSE(LocalDate::create(2026, 257, 1));
    REQUIRE_FALSE(LocalDate::create(2026, 1, 257));

    REQUIRE_FALSE(LocalTime::create(24, 0, 0));
    REQUIRE_FALSE(LocalTime::create(0, 60, 0));
    REQUIRE_FALSE(LocalTime::create(0, 0, 60));
    REQUIRE_FALSE(LocalTime::create(0, 0, 0, 1000));

    const auto finalMillisecond = LocalTime::create(23, 59, 59, 999);
    REQUIRE(finalMillisecond);
    REQUIRE(finalMillisecond.value().toIsoString() == "23:59:59.999");
}

TEST_CASE("Civil ISO representations round-trip", "[TimeUtils][Civil]")
{
    const auto parsedDate = LocalDate::parseIso("2026-05-21");
    REQUIRE(parsedDate);
    REQUIRE(parsedDate.value() == date(2026, 5, 21));
    REQUIRE_FALSE(LocalDate::parseIso("2026-5-21"));

    const auto parsedTime = LocalTime::parseIso("10:30:45.12");
    REQUIRE(parsedTime);
    REQUIRE(parsedTime.value() == time(10, 30, 45, 120));
    REQUIRE_FALSE(LocalTime::parseIso("10:30:45.1234"));

    const auto parsed = LocalDateTime::parseIso("2026-05-21T10:30:45.007");
    REQUIRE(parsed);
    REQUIRE(parsed.value() == dateTime(2026, 5, 21, 10, 30, 45, 7));
    REQUIRE(parsed.value().toIsoString() == "2026-05-21T10:30:45.007");

    const auto expandedPositive = date(10000, 1, 2);
    const auto expandedNegative = date(-1, 12, 31);
    REQUIRE(expandedPositive.toIsoString() == "+10000-01-02");
    REQUIRE(expandedNegative.toIsoString() == "-0001-12-31");
    REQUIRE(LocalDate::parseIso(expandedPositive.toIsoString()).value()
            == expandedPositive);
    REQUIRE(LocalDate::parseIso(expandedNegative.toIsoString()).value()
            == expandedNegative);
    const LocalDateTime expandedDateTime{
        expandedPositive,
        time(10, 30, 45, 7)};
    REQUIRE(LocalDateTime::parseIso(
                expandedPositive.toIsoString() + "T10:30:45.007").value()
            == expandedDateTime);
}

TEST_CASE("LocalDate arithmetic retains calendar invariants", "[TimeUtils][Civil]")
{
    const auto base = date(2026, 5, 21);
    REQUIRE(base + std::chrono::days{11} == date(2026, 6, 1));
    REQUIRE((base + std::chrono::days{10}) - base == std::chrono::days{10});

    const auto maximum = date(static_cast<int>(std::chrono::year::max()), 12, 31);
    const auto minimum = date(static_cast<int>(std::chrono::year::min()), 1, 1);
    REQUIRE_THROWS_AS(maximum + std::chrono::days{1}, std::out_of_range);
    REQUIRE_THROWS_AS(minimum - std::chrono::days{1}, std::out_of_range);
}

TEST_CASE("Instant parses RFC 3339 UTC and numeric offsets", "[TimeUtils][Instant]")
{
    const auto utc = Instant::parseRfc3339("2025-05-21T16:10:45.123Z");
    const auto positiveOffset = Instant::parseRfc3339(
        "2025-05-22T00:10:45.123+08:00");
    const auto negativeOffset = Instant::parseRfc3339(
        "2025-05-21T10:40:45.123-05:30");
    REQUIRE(utc);
    REQUIRE(positiveOffset);
    REQUIRE(negativeOffset);
    REQUIRE(utc.value().toUnixMilliseconds() == 1747843845123LL);
    REQUIRE(positiveOffset.value() == utc.value());
    REQUIRE(negativeOffset.value() == utc.value());
    REQUIRE(utc.value().toRfc3339().value()
            == "2025-05-21T16:10:45.123Z");

    REQUIRE(Instant::parseRfc3339("2026-05-21T10:30:00.5Z")
                .value().toRfc3339().value()
            == "2026-05-21T10:30:00.500Z");
    REQUIRE_FALSE(Instant::parseRfc3339("2026-05-21T10:30:00"));
    REQUIRE_FALSE(Instant::parseRfc3339("2026-02-30T10:30:00Z"));
    REQUIRE_FALSE(Instant::parseRfc3339("2026-05-21T10:30:60Z"));
    REQUIRE_FALSE(Instant::parseRfc3339("2026-05-21T10:30:00.1234Z"));
    REQUIRE_FALSE(Instant::parseRfc3339("2026-05-21T10:30:00+18:01"));
}

TEST_CASE("Instant floors negative Unix seconds", "[TimeUtils][Instant]")
{
    REQUIRE(Instant::fromUnixMilliseconds(-1).toUnixSeconds() == -1);
    REQUIRE(Instant::fromUnixMilliseconds(-999).toUnixSeconds() == -1);
    REQUIRE(Instant::fromUnixMilliseconds(-1000).toUnixSeconds() == -1);
    REQUIRE(Instant::fromUnixMilliseconds(-1001).toUnixSeconds() == -2);
    REQUIRE(Instant::fromUnixMilliseconds(-1).toRfc3339().value()
            == "1969-12-31T23:59:59.999Z");
}

TEST_CASE("Instant validates Unix, time_t, and arithmetic ranges", "[TimeUtils][Instant]")
{
    constexpr int64_t minimumSeconds = std::numeric_limits<int64_t>::min() / 1000;
    constexpr int64_t maximumSeconds = std::numeric_limits<int64_t>::max() / 1000;
    REQUIRE(Instant::fromUnixSeconds(minimumSeconds));
    REQUIRE(Instant::fromUnixSeconds(maximumSeconds));
    REQUIRE_FALSE(Instant::fromUnixSeconds(minimumSeconds - 1));
    REQUIRE_FALSE(Instant::fromUnixSeconds(maximumSeconds + 1));

    const std::time_t value = static_cast<std::time_t>(1747843845);
    const auto fromTime = Instant::fromTimeT(value);
    REQUIRE(fromTime);
    REQUIRE(fromTime.value().toTimeT().value() == value);

    const auto maximum = Instant::fromUnixMilliseconds(std::numeric_limits<int64_t>::max());
    const auto minimum = Instant::fromUnixMilliseconds(std::numeric_limits<int64_t>::min());
    REQUIRE_THROWS_AS(maximum + 1ms, std::overflow_error);
    REQUIRE_THROWS_AS(minimum - 1ms, std::overflow_error);
    REQUIRE_THROWS_AS(maximum - minimum, std::overflow_error);
    REQUIRE_FALSE(maximum.toRfc3339());
    REQUIRE_FALSE(TimeUtils::toZonedDateTime(maximum, TimeZone::utc()));
}

TEST_CASE("UTC offsets and fixed zones are bounded values", "[TimeUtils][TimeZone]")
{
    REQUIRE(UtcOffset::utc().toString() == "+00:00");
    REQUIRE(UtcOffset::fromHoursMinutes(8, 0).value().toString() == "+08:00");
    REQUIRE(UtcOffset::fromHoursMinutes(-5, 30).value().toString() == "-05:30");
    REQUIRE(UtcOffset::fromHoursMinutes(0, -30).value().toString() == "-00:30");
    REQUIRE_FALSE(UtcOffset::fromHoursMinutes(18, 1));
    REQUIRE_FALSE(UtcOffset::fromSeconds(18 * 3600 + 1));

    const auto fixed = TimeZone::fixedOffset(
        UtcOffset::fromHoursMinutes(8, 0).value());
    REQUIRE(fixed.kind() == TimeZoneKind::FixedOffset);
    REQUIRE(fixed.id() == "+08:00");
    REQUIRE(fixed.configuredOffset().value().totalSeconds() == 8 * 3600);
}

TEST_CASE("UTC and fixed offset conversions round-trip", "[TimeUtils][TimeZone]")
{
    const auto instant = Instant::parseRfc3339("2025-05-21T16:10:45.123Z").value();
    const auto utc = TimeUtils::toZonedDateTime(instant, TimeZone::utc());
    REQUIRE(utc);
    REQUIRE(utc.value().localDateTime()
            == dateTime(2025, 5, 21, 16, 10, 45, 123));
    REQUIRE(utc.value().abbreviation() == "UTC");
    REQUIRE_FALSE(utc.value().isDaylightSavingTime());
    REQUIRE(TimeUtils::toInstant(
                utc.value().localDateTime(),
                utc.value().timeZone()).value()
            == instant);

    const auto plusEight = TimeZone::fixedOffset(
        UtcOffset::fromHoursMinutes(8, 0).value());
    const auto east = TimeUtils::toZonedDateTime(instant, plusEight);
    REQUIRE(east);
    REQUIRE(east.value().localDateTime()
            == dateTime(2025, 5, 22, 0, 10, 45, 123));
    REQUIRE(TimeUtils::toInstant(east.value().localDateTime(), plusEight).value()
            == instant);

    const auto minusFiveThirty = TimeZone::fixedOffset(
        UtcOffset::fromHoursMinutes(-5, 30).value());
    const auto west = TimeUtils::toZonedDateTime(instant, minusFiveThirty);
    REQUIRE(west);
    REQUIRE(west.value().localDateTime()
            == dateTime(2025, 5, 21, 10, 40, 45, 123));
    REQUIRE(TimeUtils::toInstant(
                west.value().localDateTime(),
                minusFiveThirty).value()
            == instant);
}

TEST_CASE("One Instant can map to different calendar dates", "[TimeUtils][TimeZone]")
{
    const auto instant = Instant::parseRfc3339("2025-05-21T01:00:00Z").value();
    const auto west = TimeZone::fixedOffset(
        UtcOffset::fromHoursMinutes(-5, 0).value());
    REQUIRE(TimeUtils::toZonedDateTime(instant, TimeZone::utc())
                .value().localDateTime().date()
            == date(2025, 5, 21));
    REQUIRE(TimeUtils::toZonedDateTime(instant, west)
                .value().localDateTime().date()
            == date(2025, 5, 20));
}

TEST_CASE("Named timezone capability is explicit", "[TimeUtils][TimeZone]")
{
    const auto abbreviation = TimeUtils::findTimeZone("EST");
    REQUIRE_FALSE(abbreviation);
    REQUIRE(abbreviation.error().code == TimeErrorCode::UnknownTimeZone);

    const auto newYork = TimeUtils::findTimeZone("America/New_York");
    if (!TimeUtils::supportsNamedTimeZones())
    {
        REQUIRE_FALSE(newYork);
        REQUIRE(newYork.error().code == TimeErrorCode::TimeZoneNotSupported);
        REQUIRE_FALSE(TimeUtils::systemTimeZone());
        REQUIRE_FALSE(TimeUtils::timeZoneDatabaseVersion());
    }
    else
    {
        REQUIRE(newYork);
        REQUIRE(newYork.value().kind() == TimeZoneKind::Named);
        REQUIRE(TimeUtils::timeZoneDatabaseVersion());
    }
}

TEST_CASE("DST resolution policies handle gaps and overlaps", "[TimeUtils][TimeZone][DST]")
{
    if (!TimeUtils::supportsNamedTimeZones())
    {
        SUCCEED("standard library has no C++20 tzdb support");
        return;
    }

    const auto zone = TimeUtils::findTimeZone("America/New_York");
    REQUIRE(zone);

    const auto nonexistent = dateTime(2024, 3, 10, 2, 30, 0);
    const auto rejectedGap = TimeUtils::toInstant(nonexistent, zone.value());
    REQUIRE_FALSE(rejectedGap);
    REQUIRE(rejectedGap.error().code == TimeErrorCode::NonexistentLocalTime);

    LocalTimeResolvePolicy forward;
    forward.nonexistent = NonexistentTimePolicy::ShiftForward;
    LocalTimeResolvePolicy backward;
    backward.nonexistent = NonexistentTimePolicy::ShiftBackward;
    const auto shiftedForward = TimeUtils::toInstant(nonexistent, zone.value(), forward);
    const auto shiftedBackward = TimeUtils::toInstant(nonexistent, zone.value(), backward);
    REQUIRE(shiftedForward);
    REQUIRE(shiftedBackward);
    REQUIRE(TimeUtils::toZonedDateTime(shiftedForward.value(), zone.value())
                .value().localDateTime()
            == dateTime(2024, 3, 10, 3, 30, 0));
    REQUIRE(TimeUtils::toZonedDateTime(shiftedBackward.value(), zone.value())
                .value().localDateTime()
            == dateTime(2024, 3, 10, 1, 30, 0));

    const auto ambiguous = dateTime(2024, 11, 3, 1, 30, 0);
    const auto rejectedOverlap = TimeUtils::toInstant(ambiguous, zone.value());
    REQUIRE_FALSE(rejectedOverlap);
    REQUIRE(rejectedOverlap.error().code == TimeErrorCode::AmbiguousLocalTime);

    LocalTimeResolvePolicy earlier;
    earlier.ambiguous = AmbiguousTimePolicy::Earlier;
    LocalTimeResolvePolicy later;
    later.ambiguous = AmbiguousTimePolicy::Later;
    const auto earlierInstant = TimeUtils::toInstant(ambiguous, zone.value(), earlier);
    const auto laterInstant = TimeUtils::toInstant(ambiguous, zone.value(), later);
    REQUIRE(earlierInstant);
    REQUIRE(laterInstant);
    REQUIRE(laterInstant.value() - earlierInstant.value() == 1h);
}

TEST_CASE("Portable pattern parsing and formatting are strict", "[TimeUtils][Format]")
{
    const auto value = dateTime(2026, 5, 21, 10, 30, 45, 7);
    REQUIRE(TimeUtils::format(value, "%Y-%m-%d %H:%M:%S.%f").value()
            == "2026-05-21 10:30:45.007");
    REQUIRE(TimeUtils::format(value, "%%Z").value() == "%Z");
    REQUIRE_FALSE(TimeUtils::format(value, "%Y-%m-%d %z"));
    REQUIRE_FALSE(TimeUtils::format(value, "%Y-%m-%d %Z"));
    REQUIRE_FALSE(TimeUtils::format(value, "%c"));

    const auto parsed = TimeUtils::parseLocalDateTime(
        "12 2026-05-21 10:30:45",
        "%f %Y-%m-%d %H:%M:%S");
    REQUIRE(parsed);
    REQUIRE(parsed.value() == dateTime(2026, 5, 21, 10, 30, 45, 120));
    REQUIRE_FALSE(TimeUtils::parseLocalDateTime(
        "2026-05-21 10:30:45 trailing",
        "%Y-%m-%d %H:%M:%S"));
    REQUIRE_FALSE(TimeUtils::parseLocalDateTime(
        "2026-05-21 UTC",
        "%Y-%m-%d %Z"));
    REQUIRE_FALSE(TimeUtils::parseLocalDateTime(
        "2026-05-21 10:30:45 1 2",
        "%Y-%m-%d %H:%M:%S %f %f"));

    const auto fixed = TimeZone::fixedOffset(
        UtcOffset::fromHoursMinutes(8, 0).value());
    const auto zoned = TimeUtils::toZonedDateTime(
        Instant::parseRfc3339("2025-05-21T16:10:45.123Z").value(),
        fixed);
    REQUIRE(zoned);
    REQUIRE(TimeUtils::format(zoned.value(), "%Y-%m-%d %H:%M %z %Z").value()
            == "2025-05-22 00:10 +08:00 +08:00");
}

TEST_CASE("Duration formatting preserves negative signs and large hours", "[TimeUtils][Duration]")
{
    REQUIRE(TimeUtils::formatHoursMinutesSeconds(3661s) == "01:01:01");
    REQUIRE(TimeUtils::formatHoursMinutesSeconds(-3661s) == "-01:01:01");
    REQUIRE(TimeUtils::formatMinutesSeconds(125s) == "02:05");
    REQUIRE(TimeUtils::formatMinutesSeconds(-65s) == "-01:05");
    REQUIRE(TimeUtils::formatDuration(3661050ms) == "01:01:01.050");
    REQUIRE(TimeUtils::formatDuration(-1500ms) == "-00:00:01.500");
    REQUIRE(TimeUtils::formatDurationHuman(0ms) == "0 ms");
    REQUIRE(TimeUtils::formatDurationHuman(500ms) == "500 ms");
    REQUIRE(TimeUtils::formatDurationHuman(-500ms) == "-500 ms");
    REQUIRE(TimeUtils::formatDurationHuman(-1500ms) == "-1s 500ms");
    REQUIRE(TimeUtils::formatHoursMinutesSeconds(100h) == "100:00:00");

    const auto minimumMilliseconds = std::chrono::milliseconds{
        std::numeric_limits<std::chrono::milliseconds::rep>::min()};
    REQUIRE_FALSE(TimeUtils::formatDuration(minimumMilliseconds).empty());
    REQUIRE_FALSE(TimeUtils::formatDurationHuman(minimumMilliseconds).empty());
}

TEST_CASE("Fixed-zone operations are deterministic under concurrency", "[TimeUtils][Threading]")
{
    const auto zone = TimeZone::fixedOffset(
        UtcOffset::fromHoursMinutes(8, 0).value());
    const auto instant = Instant::parseRfc3339("2025-05-21T16:10:45.123Z").value();
    constexpr std::size_t threadCount = 8;
    constexpr int iterationCount = 300;
    std::atomic<bool> consistent{true};
    std::vector<std::thread> threads;
    for (std::size_t index = 0; index < threadCount; ++index)
    {
        threads.emplace_back([&]
        {
            for (int iteration = 0; iteration < iterationCount; ++iteration)
            {
                const auto formatted = TimeUtils::format(
                    instant,
                    zone,
                    "%Y-%m-%dT%H:%M:%S.%f%z");
                if (!formatted
                    || formatted.value() != "2025-05-22T00:10:45.123+08:00")
                {
                    consistent.store(false, std::memory_order_relaxed);
                    return;
                }
            }
        });
    }
    for (auto& thread : threads)
    {
        thread.join();
    }
    REQUIRE(consistent.load(std::memory_order_relaxed));
}

TEST_CASE("Clock abstraction is injectable without global mutable state", "[TimeUtils][Clock]")
{
    const auto fixedValue = Instant::fromUnixMilliseconds(1234);
    const FixedClock fixed{fixedValue};
    REQUIRE(fixed.now() == fixedValue);

    const auto before = Instant::fromUnixMilliseconds(
        std::chrono::floor<std::chrono::milliseconds>(
            std::chrono::system_clock::now()).time_since_epoch().count());
    const auto observed = SystemClock::instance().now();
    const auto after = Instant::fromUnixMilliseconds(
        std::chrono::floor<std::chrono::milliseconds>(
            std::chrono::system_clock::now()).time_since_epoch().count());
    REQUIRE(observed >= before);
    REQUIRE(observed <= after);
    REQUIRE(TimeUtils::now() >= before);
}
