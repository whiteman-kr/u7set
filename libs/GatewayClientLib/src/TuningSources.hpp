#pragma once

#include <GatewayClientLib/GwHash.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace GatewayClientLib
{
	//struct TuningSignal
	//{
	//	std::string appSignalId;
	//	Radiy::Hash hash = Radiy::UNDEFINED_HASH;
	//	std::string customAppSignalId;
	//	std::string caption;
	//	std::string equipmentId;
	//	std::string type;
	//	std::string tuningValueType;
	//	
	//	double tuningDefaultValue{};
	//	double tuningLowBound{};
	//	double tuningHighBound{};
	//};

	//struct TuningSource
	//{
	//	std::string moduleEquipmentId;
	//	std::string profile;
	//	std::string caption;
	//	std::string lanEquipmentId;
	//	bool tuningEnabled = false;
	//	std::string tuningIp;
	//	uint16_t tuningPort = 0;
	//	std::string tuningServiceId;
	//	std::string tuningServiceIp;
	//	uint16_t tuningServicePort = 0;
	//	std::string tuningServiceNetmask;
	//	std::vector<std::string> tuningSignalIds;
	//	std::vector<TuningSignal> signals;
	//};

	//struct ParseTuningSourceXmlResult
	//{
	//	std::vector<TuningSource> tuningSources;
	//	std::vector<std::string> errors;
	//};

	//[[nodiscard]] ParseTuningSourceXmlResult parseTuningSourcesXml(std::string_view xmlContent);
} // namespace GatewayClientLib
