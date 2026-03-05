#pragma once

#include <AdsGatewayLib/GwHash.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace TestSettings
{
	inline std::string Address = "127.0.0.1";
	inline uint16_t Port = 5566;

	struct ProjectSignal
	{
		std::string appSignalId;
		Radiy::Hash hash;
		double expectedValue;
	};

	inline std::vector<ProjectSignal> projectSignals;
} // namespace TestSettings