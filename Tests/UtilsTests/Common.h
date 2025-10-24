#pragma once

#include <random>

#include "../../OnlineLib/CircularLogger.h"

extern std::shared_ptr<CircularLogger> logger;
extern QString buildPath;
extern QString profileName;

extern QByteArray appDataService_configurationXml;

inline quint64 randomUint64()
{
	static thread_local std::mt19937_64 gen{ std::random_device{}() };

	std::uniform_int_distribution<quint64> dist(0, UINT64_MAX);

	return dist(gen);
}

inline quint32 randomUint32()
{
	static thread_local std::mt19937 gen{ std::random_device{}() };

	std::uniform_int_distribution<quint32> dist(0, UINT32_MAX);

	return dist(gen);
}
