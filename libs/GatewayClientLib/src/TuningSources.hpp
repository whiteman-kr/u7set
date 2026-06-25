#pragma once

#include <GatewayClientLib/GwClient.hpp>
#include <GatewayClientLib/GwHash.hpp>

#include <span>
#include <string>
#include <unordered_map>
#include <vector>


namespace GatewayClientLib
{
	// TuningSources.xml: Content/BuildInfo
	//
	struct Project
	{
		std::string name;        // [@Project]
		int buildNo{};           // [@ID]
		std::string buildDate{}; // [@Date]
		std::string buildUser{}; // [@User]
	};

	// TuningSources.xml: Content/DataSources/DataSource
	//
	struct TuningSource
	{
		std::string moduleEquipmentId;                             // [@ModuleEquipmentID]
		std::string moduleCaption;                                 // [@Caption]
		std::string subsystemId;                                   // [@SubsystemID]
		Channel channel = Channel::A;                              // [@SubsystemChannel]

		std::vector<std::string> signalIds;                        // TuningData/*Signals/Signal
		std::unordered_map<Radiy::Hash, GwAppSignalParam> signals; // TuningData/*Signals/Signal
	};

	// Result of parsing TuningSources.xml
	//
	struct ParseTuningSourceXmlResult
	{
		Project project;                         // Content/BuildInfo
		std::vector<TuningSource> tuningSources; // Content/DataSources/DataSource

		std::vector<std::string> errors;
	};

	// Parsing function for TuningSources.xml content. Returns a struct containing the parsed project info, tuning sources and any errors
	// encountered during parsing.
	//
	[[nodiscard]] ParseTuningSourceXmlResult parseTuningSourcesXml(std::span<const std::byte> xmlContent);

} // namespace GatewayClientLib