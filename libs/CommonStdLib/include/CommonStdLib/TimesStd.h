#pragma once

#include <cstdint>
//#include <chrono> // Please use chrono from precompiled headers in your project!

// Time literals converts to ms
//
constexpr int64_t operator""_ms(unsigned long long int value)
{
	return value;
}

constexpr int64_t operator""_sec(unsigned long long int value)
{
	return value * 1000;
}

constexpr int64_t operator""_min(unsigned long long int value)
{
	return value * 60 * 1000;
}

constexpr int64_t operator""_hour(unsigned long long int value)
{
	return value * 3600 * 1000;
}

constexpr int64_t operator""_hours(unsigned long long int value)
{
	return value * 3600 * 1000;
}

constexpr int64_t operator""_day(unsigned long long int value)
{
	return value * 24 * 3600 * 1000;
}

[[nodiscard]] inline int64_t currentMSecsSinceEpoch()
{
	using namespace std::chrono;

	const auto now = system_clock::now();
	const auto ms = time_point_cast<milliseconds>(now).time_since_epoch();
	return ms.count(); // UTC time
}

#define currentMSecsUTC currentMSecsSinceEpoch

[[nodiscard]] inline int64_t currentMSecsLocal()
{
	using namespace std::chrono;

	const auto now = system_clock::now();
	const auto utcMs = duration_cast<milliseconds>(now.time_since_epoch()).count();

	const auto* zone = current_zone();
	const auto info = zone->get_info(now);

	const int64_t offsetMs = static_cast<int64_t>(info.offset.count()) * 1000;
	return utcMs + offsetMs; // Local time
}
