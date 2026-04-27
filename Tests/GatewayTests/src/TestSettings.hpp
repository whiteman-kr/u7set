#pragma once

#include <GatewayClientLib/AdsGwProtocol.hpp>
#include <GatewayClientLib/GwHash.hpp>
#include <GatewayClientLib/TuningGwProtocol.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace AdsTestSettings
{
	inline std::string Address = "127.0.0.1";
	inline uint16_t Port = GatewayClientLib::ADS_GW_PORT;

	struct ProjectSignal
	{
		std::string appSignalId;
		Radiy::Hash hash;
		double expectedValue;
	};

	inline std::vector<ProjectSignal> projectSignals;
} // namespace AdsTestSettings

namespace TuningTestSettings
{
	inline std::string Address = "127.0.0.1";
	inline uint16_t Port = GatewayClientLib::TUNING_GW_PORT;

	struct ProjectSignal
	{
		std::string appSignalId;
		Radiy::Hash hash;
		double expectedValue;
	};

	inline std::vector<ProjectSignal> projectSignals;
} // namespace TuningTestSettings