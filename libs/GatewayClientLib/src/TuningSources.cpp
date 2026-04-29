#include "TuningSources.hpp"

#include <pugixml/pugixml.hpp>

#include <algorithm>
#include <bit>
#include <charconv>
#include <cstdint>
#include <ranges>


namespace
{
	bool tryParseHexUint32(std::string_view text, uint32_t& value)
	{
		if (text.empty())
		{
			return false;
		}

		if (text.starts_with("0x") || text.starts_with("0X"))
		{
			text.remove_prefix(2);
		}

		auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value, 16);
		return ec == std::errc{} && ptr == text.data() + text.size();
	}


	GatewayClientLib::TuningSource parseDataSource(const pugi::xml_node& dataSource, std::vector<std::string>& errors)
	{
		assert(dataSource.name() == std::string_view{"DataSource"});

		GatewayClientLib::TuningSource tuningSource;
		tuningSource.moduleEquipmentId = dataSource.attribute("ModuleEquipmentID").as_string();
		tuningSource.moduleCaption = dataSource.attribute("Caption").as_string();
		tuningSource.subsystemId = dataSource.attribute("SubsystemID").as_string();

		std::string channelStr = dataSource.attribute("SubsystemChannel").as_string();
		if (channelStr.size() != 1 || channelStr[0] < 'A' || channelStr[0] > 'D')
		{
			errors.push_back(std::string{"DataSource '"} + tuningSource.moduleEquipmentId + "': invalid SubsystemChannel value '" +
							 channelStr + "'. Expected 'A', 'B', 'C' or 'D'.");
		}
		else
		{
			static_assert(static_cast<int>(GatewayClientLib::Channel::A) == 0);
			static_assert(static_cast<int>(GatewayClientLib::Channel::B) == 1);
			static_assert(static_cast<int>(GatewayClientLib::Channel::C) == 2);
			static_assert(static_cast<int>(GatewayClientLib::Channel::D) == 3);

			tuningSource.channel = static_cast<GatewayClientLib::Channel>(channelStr[0] - 'A');
		}

		return tuningSource;
	}

	void parseSignals(const pugi::xml_node& node,
					  GatewayClientLib::SignalType signalType,
					  GatewayClientLib::TuningSource& targetTuningSource,
					  std::vector<std::string>& errors)
	{
		if (not node)
		{
			errors.push_back("Missing element '" + std::string{node.name()} + "'");
			return;
		}

		for (const auto& signalNode : node.children("Signal"))
		{
			std::string appSignalId = signalNode.attribute("AppSignalID").as_string();
			auto hash = Radiy::calcHash(appSignalId);

			auto readDiscreteTuningValue = [&signalNode](const char* hexAttributeName)
			{
				uint32_t rawValue = 0;
				const bool ok = tryParseHexUint32(signalNode.attribute(hexAttributeName).as_string(), rawValue);
				assert(ok);
				return static_cast<double>(rawValue == 0 ? 0 : 1);
			};

			auto readSignedTuningValue = [&signalNode](const char* hexAttributeName)
			{
				uint32_t rawValue = 0;
				const bool ok = tryParseHexUint32(signalNode.attribute(hexAttributeName).as_string(), rawValue);
				assert(ok);
				return static_cast<double>(std::bit_cast<int32_t>(rawValue));
			};

			auto readFloatingTuningValue = [&signalNode](const char* hexAttributeName)
			{
				uint32_t rawValue = 0;
				const bool ok = tryParseHexUint32(signalNode.attribute(hexAttributeName).as_string(), rawValue);
				assert(ok);
				return static_cast<double>(std::bit_cast<float>(rawValue));
			};


			GatewayClientLib::GwAppSignalParam sp{};

			sp.hash = hash;
			std::snprintf(sp.appSignalId, sizeof(sp.appSignalId), "%s", appSignalId.c_str());
			std::snprintf(sp.customSignalId, sizeof(sp.customSignalId), "%s", signalNode.attribute("CustomAppSignalID").as_string());
			std::snprintf(sp.caption, sizeof(sp.caption), "%s", signalNode.attribute("Caption").as_string());
			std::snprintf(sp.equipmentId, sizeof(sp.equipmentId), "%s", signalNode.attribute("EquipmentID").as_string());
			std::snprintf(sp.lmEquipmentId, sizeof(sp.lmEquipmentId), "%s", signalNode.attribute("EquipmentID").as_string());
			std::snprintf(sp.units, sizeof(sp.units), "%s", signalNode.attribute("Unit").as_string());

			// Tags in file are comma separated, but GwAppSignalParam expects space separated, so replace ',' with ' '.
			//
			std::snprintf(sp.tags, sizeof(sp.tags), "%s", signalNode.attribute("Tags").as_string());
			std::replace(std::begin(sp.tags), std::end(sp.tags), ',', ' ');

			sp.channel = targetTuningSource.channel;
			sp.inOutType = GatewayClientLib::InOutType::Internal;
			sp.type = signalType;
			sp.decimalPlaces = static_cast<uint8_t>(signalNode.attribute("DecimalPlaces").as_uint(0));
			sp.tuning = true;

			// TO DO: Read valid range and tuning bounds as hex values from "xxxxHex" attribute.
			// sending to the gateway.
			//
			switch (signalType)
			{
			case GatewayClientLib::SignalType::Discrete:
				{
					sp.lowValidRange = signalNode.attribute("LowEngineeringUnits").as_int();
					sp.highValidRange = signalNode.attribute("HighEngineeringUnits").as_int();
					sp.tuningDefaultValue = readDiscreteTuningValue("TuningDefaultValueHex");
                 sp.tuningLowBound = readDiscreteTuningValue("TuningLowBoundHex");
					sp.tuningHighBound = readDiscreteTuningValue("TuningHighBoundHex");
				}
				break;

			case GatewayClientLib::SignalType::SignedInt32:
				{
					sp.lowValidRange = signalNode.attribute("LowEngineeringUnits").as_int();
					sp.highValidRange = signalNode.attribute("HighEngineeringUnits").as_int();
					sp.tuningDefaultValue = readSignedTuningValue("TuningDefaultValueHex");
					sp.tuningLowBound = readSignedTuningValue("TuningLowBoundHex");
					sp.tuningHighBound = readSignedTuningValue("TuningHighBoundHex");
				}
				break;

			case GatewayClientLib::SignalType::Float32:
				{
					sp.lowValidRange = signalNode.attribute("LowEngineeringUnits").as_double();
					sp.highValidRange = signalNode.attribute("HighEngineeringUnits").as_double();
					sp.tuningDefaultValue = readFloatingTuningValue("TuningDefaultValueHex");
					sp.tuningLowBound = readFloatingTuningValue("TuningLowBoundHex");
					sp.tuningHighBound = readFloatingTuningValue("TuningHighBoundHex");
				}
				break;
			};

			targetTuningSource.signalIds.push_back(appSignalId);
			targetTuningSource.signals.emplace(hash, sp);
		}

		return;
	}
} // namespace

namespace GatewayClientLib
{
	ParseTuningSourceXmlResult parseTuningSourcesXml(std::span<const std::byte> xmlContent)
	{
		ParseTuningSourceXmlResult result;

		pugi::xml_document doc;
		pugi::xml_parse_result parseResult = doc.load_buffer(xmlContent.data(), xmlContent.size());
		if (parseResult == false)
		{
			result.errors.push_back(std::string{"XML parse error: "} + parseResult.description());
			return result;
		}

		const pugi::xml_node contentNode = doc.child("Content");
		if (not contentNode)
		{
			result.errors.push_back("Missing root element 'Content'");
			return result;
		}

		// Getting project and build info
		//
		{
			auto buildInfoNode = contentNode.child("BuildInfo");
			if (buildInfoNode.empty() == true)
			{
				result.errors.push_back("Missing element 'Content/BuildInfo'");
				return result;
			}

			result.project.name = buildInfoNode.attribute("Project").as_string();
			result.project.buildNo = buildInfoNode.attribute("ID").as_int(0);
			result.project.buildDate = buildInfoNode.attribute("Date").as_string();
			result.project.buildUser = buildInfoNode.attribute("User").as_string();
		}

		// Getting tuning sources
		//
		const auto dataSourcesNode = contentNode.child("DataSources");
		if (not dataSourcesNode)
		{
			result.errors.push_back("Missing element 'Content/DataSources'");
			return result;
		}

		// Skip all not "Default" profiles.
		//
		auto defaultProfileFilter = [](const pugi::xml_node& node)
		{
			return std::string_view{node.attribute("Profile").as_string()} == "Default";
		};

		auto defaultDataSource = dataSourcesNode.children("DataSource") | std::views::filter(defaultProfileFilter);

		result.tuningSources.reserve(std::ranges::distance(defaultDataSource));

		for (const auto dataSourceNode : defaultDataSource)
		{
			auto& current = result.tuningSources.emplace_back(parseDataSource(dataSourceNode, result.errors));

			// Signals - DataSource/TuningData/*Signals/Signal
			//
			auto tuningDataNode = dataSourceNode.child("TuningData");
			if (not tuningDataNode)
			{
				result.errors.push_back("Missing element 'DataSource/TuningData'");
				continue;
			}

			const size_t signalCount = tuningDataNode.attribute("SignalsCount").as_uint();
			current.signalIds.reserve(signalCount);
			current.signals.reserve(signalCount);

			parseSignals(tuningDataNode.child("AnalogFloatSignals"), SignalType::Float32, current, result.errors);
			parseSignals(tuningDataNode.child("AnalogInt32Signals"), SignalType::SignedInt32, current, result.errors);
			parseSignals(tuningDataNode.child("DiscreteSignals"), SignalType::Discrete, current, result.errors);
		}

		return result;
	}
} // namespace GatewayClientLib
