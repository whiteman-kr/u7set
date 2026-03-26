#include "TuningSources.hpp"

#include <pugixml/pugixml.hpp>

#include <charconv>
#include <system_error>
#include <unordered_set>

namespace
{
	//std::string trim(std::string_view value)
	//{
	//	const auto begin = value.find_first_not_of(" \t\r\n");
	//	if (begin == std::string_view::npos)
	//	{
	//		return {};
	//	}

	//	const auto end = value.find_last_not_of(" \t\r\n");
	//	return std::string{value.substr(begin, end - begin + 1)};
	//}

	//void addError(std::vector<std::string>& errors, std::string message)
	//{
	//	errors.push_back(std::move(message));
	//}

	//std::string getRequiredAttribute(
	//	const pugi::xml_node& node,
	//	const char* attributeName,
	//	std::vector<std::string>& errors,
	//	std::string_view context)
	//{
	//	const auto attribute = node.attribute(attributeName);
	//	if (!attribute)
	//	{
	//		addError(errors, std::string{context} + ": missing attribute '" + attributeName + "'");
	//		return {};
	//	}

	//	return attribute.as_string();
	//}

	//std::optional<uint16_t> parseUint16(
	//	const pugi::xml_node& node,
	//	const char* attributeName,
	//	std::vector<std::string>& errors,
	//	std::string_view context)
	//{
	//	const auto attribute = node.attribute(attributeName);
	//	if (!attribute)
	//	{
	//		return std::nullopt;
	//	}

	//	uint16_t value = 0;
	//	const std::string_view text{attribute.value()};
	//	const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
	//	if (ec != std::errc{} || ptr != text.data() + text.size())
	//	{
	//		addError(errors, std::string{context} + ": invalid uint16 attribute '" + attributeName + "' value '" +
	//			std::string{text} + "'");
	//		return std::nullopt;
	//	}

	//	return value;
	//}

	//std::optional<double> parseDouble(
	//	const pugi::xml_node& node,
	//	const char* attributeName,
	//	std::vector<std::string>& errors,
	//	std::string_view context)
	//{
	//	const auto attribute = node.attribute(attributeName);
	//	if (!attribute)
	//	{
	//		return std::nullopt;
	//	}

	//	double value = 0.0;
	//	const std::string_view text{attribute.value()};
	//	const auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), value);
	//	if (ec != std::errc{} || ptr != text.data() + text.size())
	//	{
	//		addError(errors, std::string{context} + ": invalid numeric attribute '" + attributeName + "' value '" +
	//			std::string{text} + "'");
	//		return std::nullopt;
	//	}

	//	return value;
	//}

	//std::vector<std::string> parseTuningSignalIds(std::string_view text)
	//{
	//	std::vector<std::string> result;
	//	size_t begin = 0;

	//	while (begin < text.size())
	//	{
	//		const auto end = text.find(',', begin);
	//		const auto token = trim(text.substr(begin, end == std::string_view::npos ? text.size() - begin : end - begin));
	//		if (!token.empty())
	//		{
	//			result.push_back(token);
	//		}

	//		if (end == std::string_view::npos)
	//		{
	//			break;
	//		}

	//		begin = end + 1;
	//	}

	//	return result;
	//}

	//pugi::xml_node findTuningParamsNode(const pugi::xml_node& dataSource)
	//{
	//	const auto lanControllers = dataSource.child("LanControllers");
	//	for (const auto lanController : lanControllers.children("LanController"))
	//	{
	//		const auto type = std::string_view{lanController.attribute("LanControllerType").as_string()};
	//		if (type == "Tuning")
	//		{
	//			return lanController.child("TuningParams");
	//		}
	//	}

	//	return {};
	//}

	//void parseSignalGroup(
	//	const pugi::xml_node& signalGroup,
	//	std::string_view groupType,
	//	std::vector<GatewayClientLib::TuningSignal>& signals,
	//	std::vector<std::string>& errors,
	//	std::string_view moduleEquipmentId)
	//{
	//	for (const auto signalNode : signalGroup.children("Signal"))
	//	{
	//		GatewayClientLib::TuningSignal signal;
	//		signal.type = signalNode.attribute("Type").as_string(groupType.data());
	//		signal.tuningValueType = signalNode.attribute("TuningValueTypeStr").as_string();
	//		signal.customAppSignalId = signalNode.attribute("CustomAppSignalID").as_string();
	//		signal.caption = signalNode.attribute("Caption").as_string();
	//		signal.equipmentId = signalNode.attribute("EquipmentID").as_string();
	//		signal.enableTuning = signalNode.attribute("EnableTuning").as_bool(false);

	//		const auto context = std::string{"DataSource '"} + std::string{moduleEquipmentId} + "' Signal";
	//		signal.appSignalId = getRequiredAttribute(signalNode, "AppSignalID", errors, context);
	//		if (signal.appSignalId.empty())
	//		{
	//			continue;
	//		}

	//		signal.hash = Radiy::calcHash(signal.appSignalId);
	//		signal.tuningDefaultValue = parseDouble(signalNode, "TuningDefaultValue", errors, context);
	//		signal.tuningLowBound = parseDouble(signalNode, "TuningLowBound", errors, context);
	//		signal.tuningHighBound = parseDouble(signalNode, "TuningHighBound", errors, context);

	//		signals.push_back(std::move(signal));
	//	}
	//}

	//GatewayClientLib::TuningSource parseDataSource(const pugi::xml_node& dataSource, std::vector<std::string>& errors)
	//{
	//	GatewayClientLib::TuningSource tuningSource;
	//	tuningSource.moduleEquipmentId = dataSource.attribute("ModuleEquipmentID").as_string();
	//	tuningSource.profile = dataSource.attribute("Profile").as_string();
	//	tuningSource.caption = dataSource.attribute("Caption").as_string();

	//	const auto tuningContext = std::string{"DataSource '"} + tuningSource.moduleEquipmentId + "'";
	//	const auto tuningParams = findTuningParamsNode(dataSource);
	//	if (!tuningParams)
	//	{
	//		addError(errors, tuningContext + ": missing LanControllers/LanController[@LanControllerType='Tuning']/TuningParams");
	//	}
	//	else
	//	{
	//		tuningSource.tuningEnabled = tuningParams.attribute("TuningEnable").as_bool(false);
	//		tuningSource.tuningIp = tuningParams.attribute("TuningIP").as_string();
	//		tuningSource.tuningServiceId = tuningParams.attribute("TuningServiceID").as_string();
	//		tuningSource.tuningServiceIp = tuningParams.attribute("TuningServiceIP").as_string();
	//		tuningSource.tuningServiceNetmask = tuningParams.attribute("TuningServiceNetmask").as_string();

	//		if (const auto port = parseUint16(tuningParams, "TuningPort", errors, tuningContext))
	//		{
	//			tuningSource.tuningPort = *port;
	//		}

	//		if (const auto servicePort = parseUint16(tuningParams, "TuningServicePort", errors, tuningContext))
	//		{
	//			tuningSource.tuningServicePort = *servicePort;
	//		}
	//	}

	//	const auto lanControllers = dataSource.child("LanControllers");
	//	for (const auto lanController : lanControllers.children("LanController"))
	//	{
	//		const auto type = std::string_view{lanController.attribute("LanControllerType").as_string()};
	//		if (type == "Tuning")
	//		{
	//			tuningSource.lanEquipmentId = lanController.attribute("EquipmentID").as_string();
	//			break;
	//		}
	//	}

	//	tuningSource.tuningSignalIds = parseTuningSignalIds(dataSource.child("TuningSignals").text().as_string());

	//	const auto tuningData = dataSource.child("TuningData");
	//	if (tuningData)
	//	{
	//		parseSignalGroup(tuningData.child("AnalogFloatSignals"), "AnalogFloat", tuningSource.signals, errors, tuningSource.moduleEquipmentId);
	//		parseSignalGroup(tuningData.child("AnalogInt32Signals"), "AnalogInt32", tuningSource.signals, errors, tuningSource.moduleEquipmentId);
	//		parseSignalGroup(tuningData.child("DiscreteSignals"), "Discrete", tuningSource.signals, errors, tuningSource.moduleEquipmentId);
	//	}

	//	return tuningSource;
	//}
} // namespace

namespace GatewayClientLib
{
	//ParseTuningSourceXmlResult parseTuningSourcesXml(std::string_view xmlContent)
	//{
	//	ParseTuningSourceXmlResult result;

		//pugi::xml_document document;
		//const auto parseResult = document.load_buffer(xmlContent.data(), xmlContent.size(), pugi::parse_default, pugi::encoding_utf8);
		//if (!parseResult)
		//{
		//	result.errors.push_back(std::string{"XML parse error: "} + parseResult.description());
		//	return result;
		//}

		//const auto content = document.child("Content");
		//if (!content)
		//{
		//	result.errors.push_back("Missing root element 'Content'");
		//	return result;
		//}

		//const auto dataSourcesNode = content.child("DataSources");
		//if (!dataSourcesNode)
		//{
		//	result.errors.push_back("Missing element 'Content/DataSources'");
		//	return result;
		//}

		//std::unordered_set<std::string> modulesWithDefaultProfile;
		//for (const auto dataSource : dataSourcesNode.children("DataSource"))
		//{
		//	const auto moduleEquipmentId = std::string{dataSource.attribute("ModuleEquipmentID").as_string()};
		//	const auto profile = std::string_view{dataSource.attribute("Profile").as_string()};
		//	if (!moduleEquipmentId.empty() && profile == "Default")
		//	{
		//		modulesWithDefaultProfile.insert(moduleEquipmentId);
		//	}
		//}

		//std::unordered_set<std::string> addedModules;
		//for (const auto dataSource : dataSourcesNode.children("DataSource"))
		//{
		//	const auto moduleEquipmentId = getRequiredAttribute(
		//		dataSource,
		//		"ModuleEquipmentID",
		//		result.errors,
		//		"Content/DataSources/DataSource");
		//	if (moduleEquipmentId.empty())
		//	{
		//		continue;
		//	}

		//	const auto profile = std::string_view{dataSource.attribute("Profile").as_string()};
		//	const bool hasDefaultProfile = modulesWithDefaultProfile.contains(moduleEquipmentId);
		//	const bool isDefaultProfile = profile == "Default";

		//	if ((hasDefaultProfile && !isDefaultProfile) || addedModules.contains(moduleEquipmentId))
		//	{
		//		continue;
		//	}

		//	result.tuningSources.push_back(parseDataSource(dataSource, result.errors));
		//	addedModules.insert(moduleEquipmentId);
		//}

	//	return result;
	//}
} // namespace GatewayClientLib
