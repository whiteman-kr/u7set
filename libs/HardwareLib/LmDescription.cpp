#ifndef HARDWARE_LIB_DOMAIN
	#error Do not include this file in the project! Link HardwareLib instead.
#endif

#include "../UtilsLib/DomXmlHelper.h"
#include <HardwareLib/DataProtocols.h>
#include <HardwareLib/DeviceModule.h>
#include <HardwareLib/LmDescription.h>
#include <HardwareLib/PropertyNames.h>

#include <algorithm>
#include <expected>
#include <stdexcept>
#include <type_traits>

namespace
{
	template<class>
	inline constexpr bool always_false_v = false;

	// Func for getting data from some xml section
	//
	template<typename T>
	std::expected<T, QString> getSectionValue(const QDomElement& element, QLatin1StringView section)
	{
		QDomNodeList nl = element.elementsByTagName(section);
		if (nl.size() != 1)
		{
			return std::unexpected(QString{"Expected one %1 section in element %2."}.arg(section).arg(element.tagName()));
		}

		QString nodeText = nl.at(0).toElement().text();

		if constexpr (std::is_same_v<T, quint32>)
		{
			bool convertOk = false;
			T value = nodeText.toUInt(&convertOk);

			if (convertOk == false)
			{
				return std::unexpected(QString{"Cannot convert value '%1', element: %2."}.arg(nodeText).arg(element.tagName()));
			}
			else
			{
				return value;
			}
		}
		else if constexpr (std::is_same_v<T, bool>)
		{
			return nodeText.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;
		}
		else if constexpr (std::is_same_v<T, QString>)
		{
			return nodeText;
		}
		else
		{
			static_assert(always_false_v<T>, "Unsupported type.");
		}
	};

	bool sectionExists(QDomElement element, QLatin1String section)
	{
		QDomNodeList nl = element.elementsByTagName(section);
		if (nl.size() != 1)
		{
			return false;
		}

		return true;
	};
} // namespace


bool LmCommand::loadFromXml(const QDomElement& element, QString* errorMessage)
{
	if (errorMessage == nullptr || element.isNull() == true || element.tagName() != QLatin1String("Command"))
	{
		assert(errorMessage);
		assert(element.isNull() == false);
		assert(element.tagName() == QLatin1String("Command"));
		return false;
	}

	// Caption
	//
	if (DomXmlHelper::getStringAttribute(element, "Caption", &caption, errorMessage) == false)
	{
		return false;
	}

	int intValue = 0;

	// Code
	//
	if (DomXmlHelper::getIntAttribute(element, "Code", &intValue, errorMessage, 16) == false)
	{
		return false;
	}

	code = static_cast<quint16>(intValue);

	// CodeMask
	//
	if (DomXmlHelper::getIntAttribute(element, "CodeMask", &intValue, errorMessage, 16) == false)
	{
		return false;
	}

	codeMask = static_cast<quint16>(intValue);

	Q_ASSERT((code & (!codeMask)) == 0);

	// SimulationFunc
	//
	if (DomXmlHelper::getStringAttribute(element, "SimulationFunc", &simulationFunc, errorMessage) == false)
	{
		return false;
	}

	// ParseFunc
	//
	if (DomXmlHelper::getStringAttribute(element, "ParseFunc", &parseFunc, errorMessage) == false)
	{
		return false;
	}

	// Description
	//
	if (DomXmlHelper::getStringAttribute(element, "Description", &description, errorMessage) == false)
	{
		return false;
	}

	// CodeSize
	//
	if (DomXmlHelper::getIntAttribute(element, "CodeSize", &codeSize, errorMessage, 10) == false)
	{
		return false;
	}

	// ReadTime
	//
	if (DomXmlHelper::getIntAttribute(element, "ReadTime", &readTime, errorMessage, 10) == false)
	{
		return false;
	}

	// WaitFbExecution
	//
	if (DomXmlHelper::getBoolAttribute(element, "WaitFbExecution", &waitFbExecution, errorMessage) == false)
	{
		return false;
	}

	// ConstRuntime
	//
	if (DomXmlHelper::getIntAttributeIfExists(element, "ConstRuntime", LmCommand::UNDEFINED_PARAM, &constRuntime, errorMessage, 10) ==
		false)
	{
		return false;
	}

	// WriteToBitMemRuntime
	//
	if (DomXmlHelper::getIntAttributeIfExists(element,
											  "WriteToBitMemRuntime",
											  LmCommand::UNDEFINED_PARAM,
											  &writeToBitMemRuntime,
											  errorMessage,
											  10) == false)
	{
		return false;
	}

	// WriteToWordMemRuntime
	//
	if (DomXmlHelper::getIntAttributeIfExists(element,
											  "WriteToWordMemRuntime",
											  LmCommand::UNDEFINED_PARAM,
											  &writeToWordMemRuntime,
											  errorMessage,
											  10) == false)
	{
		return false;
	}

	// PreFbReadWordTime
	//
	if (DomXmlHelper::getIntAttributeIfExists(element,
											  "PreFbReadWordTime",
											  LmCommand::UNDEFINED_PARAM,
											  &preFbReadWordTime,
											  errorMessage,
											  10) == false)
	{
		return false;
	}

	// PostFbReadWordTime
	//
	if (DomXmlHelper::getIntAttributeIfExists(element,
											  "PostFbReadWordTime",
											  LmCommand::UNDEFINED_PARAM,
											  &postFbReadWordTime,
											  errorMessage,
											  10) == false)
	{
		return false;
	}

	// PreFbReadBitTime
	//
	if (DomXmlHelper::getIntAttributeIfExists(element,
											  "PreFbReadBitTime",
											  LmCommand::UNDEFINED_PARAM,
											  &preFbReadBitTime,
											  errorMessage,
											  10) == false)
	{
		return false;
	}

	// PostFbReadBitTime
	//
	if (DomXmlHelper::getIntAttributeIfExists(element,
											  "PostFbReadBitTime",
											  LmCommand::UNDEFINED_PARAM,
											  &postFbReadBitTime,
											  errorMessage,
											  10) == false)
	{
		return false;
	}

	// CheckFunc
	//
	if (DomXmlHelper::getStringAttribute(element, "CheckFunc", &checkFunc, errorMessage) == false)
	{
		return false;
	}

	Q_ASSERT(checkFunc.isEmpty() == false);

	// GetMnemoFunc
	//
	if (DomXmlHelper::getStringAttribute(element, "GetMnemoFunc", &getMnemoFunc, errorMessage) == false)
	{
		return false;
	}

	Q_ASSERT(getMnemoFunc.isEmpty() == false);

	// CalcExecTimeFunc
	//
	if (DomXmlHelper::getStringAttribute(element, "CalcExecTimeFunc", &calcExecTimeFunc, errorMessage) == false)
	{
		return false;
	}

	Q_ASSERT(calcExecTimeFunc.isEmpty() == false);

	return true;
}

LmDescription::LmDescription(QObject* parent) :
	QObject(parent)
{
}

LmDescription::LmDescription(const LmDescription& that)
{
	*this = that;
	return;
}

LmDescription& LmDescription::operator=(const LmDescription& src)
{
	if (&src == this)
	{
		return *this;
	}

	m_name = src.m_name;
	m_descriptionNumber = src.m_descriptionNumber;
	m_configurationScriptFile = src.m_configurationScriptFile;
	m_version = src.m_version;

	m_flashMemory = src.m_flashMemory;
	m_memory = src.m_memory;
	m_logicUnit = src.m_logicUnit;
	m_optoInterface = src.m_optoInterface;
	m_lan = src.m_lan;
	m_other = src.m_other;
	m_dataConfiguration = src.m_dataConfiguration;

	// LmCommands
	//
	m_commands = src.m_commands;
	m_logicUnitCommandsVersion = src.m_logicUnitCommandsVersion;

	m_bitAccAvailable = src.m_bitAccAvailable;

	// AFBs
	//
	m_checkAfbVersions = src.m_checkAfbVersions;
	m_checkAfbVersionsOffset = src.m_checkAfbVersionsOffset;

	m_afbComponents.clear();
	for (const auto& p : src.m_afbComponents)
	{
		std::shared_ptr<Afb::AfbComponent> afbComponentCopy = std::make_shared<Afb::AfbComponent>(*p.second.get());
		m_afbComponents.insert({p.first, std::move(afbComponentCopy)});
	}

	m_afbElements.clear();
	m_afbElements.reserve(src.m_afbElements.size());
	for (const std::shared_ptr<Afb::AfbElement>& afb : src.m_afbElements)
	{
		std::shared_ptr<Afb::AfbElement> afbCopy = std::make_shared<Afb::AfbElement>(*afb.get());
		m_afbElements.push_back(std::move(afbCopy));
	}

	return *this;
}

LmDescription::~LmDescription() = default;

bool LmDescription::load(const QByteArray& xml, QString* errorMessage)
{
	if (errorMessage == nullptr)
	{
		assert(errorMessage);
		return false;
	}

	if (xml.isEmpty() == true)
	{
		*errorMessage = tr("Input LogicModule description file is empty.");
		return false;
	}

	QDomDocument doc;

	QDomDocument::ParseResult pr = doc.setContent(xml);

	if (pr.errorMessage.isEmpty() == false)
	{
		errorMessage->append(tr(" Error %1, line %2, column %3").arg(pr.errorMessage).arg(pr.errorLine).arg(pr.errorColumn));
		return false;
	}

	return load(doc, errorMessage);
}

bool LmDescription::load(const QString& xml, QString* errorMessage)
{
	if (errorMessage == nullptr)
	{
		assert(errorMessage);
		return false;
	}

	if (xml.isEmpty() == true)
	{
		*errorMessage = tr("Input LogicModule description file is empty.");
		return false;
	}

	QDomDocument doc;

	QDomDocument::ParseResult pr = doc.setContent(xml);

	if (pr.errorMessage.isEmpty() == false)
	{
		errorMessage->append(tr(" Error %1, line %2, column %3").arg(pr.errorMessage).arg(pr.errorLine).arg(pr.errorColumn));
		return false;
	}

	return load(doc, errorMessage);
}

bool LmDescription::load(const QDomDocument& doc, QString* errorMessage)
{
	if (errorMessage == nullptr)
	{
		assert(errorMessage);
		return false;
	}

	if (doc.isNull() == true)
	{
		*errorMessage = tr("Input LogicModule description file is empty.");
		return false;
	}

	// Get root element -- <LogicModule>
	//
	QDomElement logicModuleElement = doc.documentElement();

	if (logicModuleElement.isNull() == true || logicModuleElement.tagName() != QLatin1String("LogicModule"))
	{
		errorMessage->append(tr("Cant't find root element LogicModule."));
		return false;
	}

	// Attribute Name
	//
	m_name = logicModuleElement.attribute(QLatin1String("Name"));

	// Attribute DescriptionNumber
	//
	QString s = logicModuleElement.attribute(QLatin1String("DescriptionNumber"));
	if (s.isEmpty() == true)
	{
		errorMessage->append(tr("Cant't find attribute DescriptionNumber"));
		return false;
	}

	bool ok = false;
	m_descriptionNumber = s.toInt(&ok);
	if (ok == false)
	{
		errorMessage->append(tr("Attribute DescriptionNumber has wrong format (integer is expected)"));
		return false;
	}

	// Attribute ConfigurationScriptFile
	//
	m_configurationScriptFile = logicModuleElement.attribute(QLatin1String("ConfigurationScriptFile"));
	if (m_configurationScriptFile.isEmpty() == true)
	{
		errorMessage->append(tr("Cant't find attribute ConfigurationScriptFile"));
		return false;
	}

	// Attribute Version
	//
	m_version = logicModuleElement.attribute(QLatin1String("Version"));
	if (m_version.isEmpty() == true)
	{
		errorMessage->append(tr("Cant't find attribute Version"));
		return false;
	}

	// <FlashMemory> -> m_flashMemory
	//
	ok = m_flashMemory.load(doc, errorMessage);
	if (ok == false)
	{
		return false;
	}

	// <Memory> -> m_memory
	//
	ok = m_memory.load(doc, errorMessage);
	if (ok == false)
	{
		return false;
	}

	// <LogicUnit> -> m_logicUnit
	//
	ok = m_logicUnit.load(doc, errorMessage);
	if (ok == false)
	{
		return false;
	}

	// <OptoInterface> -> m_optoInterface
	//
	ok = m_optoInterface.load(doc, errorMessage);
	if (ok == false)
	{
		return false;
	}

	// <LanInterfaces> -> m_lanInterface
	//
	ok = m_lan.load(doc, errorMessage);
	if (ok == false)
	{
		return false;
	}

	// <Other> -> m_other
	//
	ok = m_other.load(doc, errorMessage);
	if (ok == false)
	{
		return false;
	}

	// DataConfiguration -> m_dataConfiguration
	//
	ok = m_dataConfiguration.load(doc, errorMessage);
	m_dataConfiguration.dataConfigurationOffset = m_memory.m_dataConfigurationOffset;
	m_dataConfiguration.dataConfigurationSize = m_memory.m_dataConfigurationSize;

	if (ok == false)
	{
		return false;
	}

	// <LogicUnitCommnads> -- Loading logic unit commands
	//
	{
		QDomNodeList commandElementList = logicModuleElement.elementsByTagName(QLatin1String("LogicUnitCommnads"));

		if (commandElementList.size() != 1)
		{
			errorMessage->append(tr("Expected one element LogicUnitCommnads"));
			return false;
		}

		QDomElement element = commandElementList.at(0).toElement();

		ok = loadCommands(element, errorMessage);
		if (ok == false)
		{
			// ErrorMessage is set in loadCommands
			//
			return false;
		}
	} // </LogicUnitCommnads>

	// <AFBImplementation> -- Loading Application Functional Components
	//
	{
		// --
		//
		QDomNodeList afbcElementList = logicModuleElement.elementsByTagName(QLatin1String("AFBImplementation"));

		if (afbcElementList.size() != 1)
		{
			errorMessage->append(tr("Expected one element AFBImplementation"));
			return false;
		}

		QDomElement afbcElement = afbcElementList.at(0).toElement();

		// Check Afb Versions
		//
		m_checkAfbVersions = afbcElement.attribute(QLatin1String("CheckAfbVersions")).compare("true", Qt::CaseInsensitive) == 0;
		m_checkAfbVersionsOffset = afbcElement.attribute(QLatin1String("CheckAfbVersionsOffset")).toInt();

		// --
		//
		ok = loadAfbComponents(afbcElement, errorMessage);
		if (ok == false)
		{
			// ErrorMessage is set in loadAfbComponents
			//
			return false;
		}
	}

	// <AFBL> -- Loading Application Functional Block Library
	//
	{
		QDomNodeList afbsElementList = logicModuleElement.elementsByTagName(QLatin1String("AFBL"));
		if (afbsElementList.size() != 1)
		{
			errorMessage->append(tr("Expected one element AFBL"));
			return false;
		}

		QDomElement afbsElement = afbsElementList.at(0).toElement();

		ok = loadAfbs(afbsElement, errorMessage);
		if (ok == false)
		{
			// ErrorMessage is set in loadAfbs
			//
			return false;
		}
	}

	// --
	//

	return true;
}

void LmDescription::clear()
{
	*this = LmDescription();
}

bool LmDescription::loadCommands(const QDomElement& element, QString* errorMessage)
{
	assert(element.tagName() == QLatin1String("LogicUnitCommnads"));

	if (errorMessage == nullptr)
	{
		assert(errorMessage);
		return false;
	}

	if (DomXmlHelper::getIntAttribute(element, "Version", &m_logicUnitCommandsVersion, errorMessage) == false)
	{
		return false;
	}

	m_commands.clear();

	// Parse command list
	//
	QDomNodeList nodeList = element.elementsByTagName(QLatin1String("Command"));
	for (int i = 0; i < nodeList.size(); i++)
	{
		QDomNode node = nodeList.at(i);

		if (node.isNull() == true || node.isElement() == false)
		{
			*errorMessage = tr("Loading LogicUnitCommnads list error. Some nodes are null or not XML element.");
			return false;
		}

		QDomElement commandElement = node.toElement();

		LmCommand lmCommand;
		bool ok = lmCommand.loadFromXml(commandElement, errorMessage);
		if (ok == false)
		{
			return false;
		}

		// Check command code uniqueness
		//
		if (m_commands.count(lmCommand.code) != 0)
		{
			*errorMessage = tr("Loading LM commands error. Duplicate command code %1.").arg(lmCommand.code);
			return false;
		}

		m_commands.insert({lmCommand.code, lmCommand});
	}

	return true;
}

bool LmDescription::loadAfbComponents(const QDomElement& element, QString* errorMessage)
{
	assert(element.tagName() == QLatin1String("AFBImplementation"));

	if (errorMessage == nullptr)
	{
		assert(errorMessage);
		return false;
	}

	// Enumerate <AFBComponent>
	//
	m_afbComponents.clear();

	QDomNodeList afbNodeList = element.elementsByTagName(QLatin1String("AFBComponent"));

	for (int i = 0; i < afbNodeList.size(); i++)
	{
		QDomNode afbNode = afbNodeList.at(i);

		if (afbNode.isNull() == true || afbNode.isElement() == false)
		{
			*errorMessage = tr("Loading AFB components list error. Some nodes are null or not XML element.");
			return false;
		}

		QDomElement afbElement = afbNode.toElement();

		std::shared_ptr<Afb::AfbComponent> afbc = std::make_shared<Afb::AfbComponent>();

		bool ok = afbc->loadFromXml(afbElement, errorMessage);
		if (ok == false)
		{
			return false;
		}

		if (m_afbComponents.count(afbc->opCode()) != 0)
		{
			*errorMessage = tr("Loading AFB components list error. Duplicate AFB Component OpCode (%1).").arg(afbc->opCode());
			return false;
		}

		m_afbComponents[afbc->opCode()] = afbc;
	}

	return true;
}


bool LmDescription::loadAfbs(const QDomElement& element, QString* errorMessage)
{
	assert(element.tagName() == QLatin1String("AFBL"));

	if (errorMessage == nullptr)
	{
		assert(errorMessage);
		return false;
	}

	// Enumerate <AFB>
	//
	QDomNodeList afbNodeList = element.elementsByTagName(QLatin1String("AFB"));

	m_afbElements.clear();
	m_afbElements.reserve(afbNodeList.size());

	for (int i = 0; i < afbNodeList.size(); i++)
	{
		QDomNode afbNode = afbNodeList.at(i);

		if (afbNode.isNull() == true || afbNode.isElement() == false)
		{
			*errorMessage = tr("Loading AFB list error. Some nodes are null or not XML element.");
			return false;
		}

		QDomElement afbElement = afbNode.toElement();

		std::shared_ptr<Afb::AfbElement> afb = std::make_shared<Afb::AfbElement>();

		bool ok = afb->loadFromXml(afbElement, errorMessage);
		if (ok == false)
		{
			return false;
		}

		m_afbElements.push_back(afb);
	}

	// Set AFB Components to AFbElement
	//
	for (std::shared_ptr<Afb::AfbElement> afb : m_afbElements)
	{
		int opCode = afb->opCode();

		auto foundCompIt = m_afbComponents.find(opCode);
		if (foundCompIt == m_afbComponents.end())
		{
			*errorMessage = tr("Loading AFB list error. AFB %1 has unknown OpCode %2.").arg(afb->strID()).arg(afb->opCode());
			return false;
		}

		std::shared_ptr<Afb::AfbComponent> afbc = foundCompIt->second;
		if (afbc == nullptr)
		{
			assert(afbc);
			return false;
		}

		afb->setComponent(afbc);
	}

	return true;
}

QString LmDescription::lmDescriptionFile(const Hardware::DeviceModule* logicModule)
{
	if (logicModule == nullptr || (logicModule->isFSCConfigurationModule() == false && logicModule->isVdu() == false))
	{
		assert(logicModule);
		assert(logicModule->isFSCConfigurationModule() || logicModule->isVdu());

		return QString();
	}

	auto lmDescriptionFileProp = logicModule->propertyByCaption(Hardware::PropertyNames::lmDescriptionFile);
	if (lmDescriptionFileProp == nullptr)
	{
		assert(lmDescriptionFileProp);
		return QString();
	}

	QString lmDescriptionFile = lmDescriptionFileProp->value().toString();
	return lmDescriptionFile;
}

void LmDescription::dump() const
{
	qDebug() << "LogicModule Description:";

	qDebug() << "\tDescriptionNumber: " << m_descriptionNumber;

	return;
}

bool LmDescription::FlashMemory::load(const QDomDocument& document, QString* errorMessage)
{
	if (errorMessage == nullptr)
	{
		assert(errorMessage);
		return false;
	}

	if (document.isNull() == true)
	{
		assert(document.isNull() == false);
		*errorMessage = "XML documnet is null";
		return false;
	}

	// <LogicModule>
	//
	QDomElement logicModuleElement = document.documentElement();

	if (logicModuleElement.isNull() == true || logicModuleElement.tagName() != QLatin1String("LogicModule"))
	{
		errorMessage->append(tr("Cant't find root element LogicModule."));
		return false;
	}

	// <FlashMemory>
	QDomNodeList elements = logicModuleElement.elementsByTagName(QLatin1String("FlashMemory"));

	if (elements.size() != 1)
	{
		*errorMessage = "Expected one FlashMemory section";
		return false;
	}

	QDomElement element = elements.at(0).toElement();

	*this = FlashMemory();

	// Getting data
	//
	try
	{
		m_appLogicFrameCount = getSectionValue<quint32>(element, QLatin1String("AppLogicFrameCount")).value();
		m_appLogicFramePayload = getSectionValue<quint32>(element, QLatin1String("AppLogicFramePayload")).value();
		m_appLogicFrameSize = getSectionValue<quint32>(element, QLatin1String("AppLogicFrameSize")).value();
		m_appLogicUartId = getSectionValue<quint32>(element, QLatin1String("AppLogicUartID")).value_or(0);
		m_appLogicWriteBitstream = getSectionValue<bool>(element, QLatin1String("AppLogicWriteBitstream")).value_or(false);

		m_configFrameCount = getSectionValue<quint32>(element, QLatin1String("ConfigFrameCount")).value();
		m_configFramePayload = getSectionValue<quint32>(element, QLatin1String("ConfigFramePayload")).value();
		m_configFrameSize = getSectionValue<quint32>(element, QLatin1String("ConfigFrameSize")).value();
		m_configUartId = getSectionValue<quint32>(element, QLatin1String("ConfigUartID")).value_or(0);
		m_configWriteBitstream = getSectionValue<bool>(element, QLatin1String("ConfigWriteBitstream")).value_or(false);

		m_tuningFrameCount = getSectionValue<quint32>(element, QLatin1String("TuningFrameCount")).value();
		m_tuningFramePayload = getSectionValue<quint32>(element, QLatin1String("TuningFramePayload")).value();
		m_tuningFrameSize = getSectionValue<quint32>(element, QLatin1String("TuningFrameSize")).value();
		m_tuningUartId = getSectionValue<quint32>(element, QLatin1String("TuningUartID")).value_or(0);
		m_tuningWriteBitstream = getSectionValue<bool>(element, QLatin1String("TuningWriteBitstream")).value_or(false);

		m_maxConfigurationCount = getSectionValue<quint32>(element, QLatin1String("MaxConfigurationCount")).value();
		m_singleConfigFirstFrame = getSectionValue<quint32>(element, QLatin1String("SingleConfigFirstFrame")).value();
		m_singleConfigFrameCount = getSectionValue<quint32>(element, QLatin1String("SingleConfigFrameCount")).value();
		m_singleConfigUniqueIdOffset = getSectionValue<quint32>(element, QLatin1String("SingleConfigUniqueIDOffset")).value();
	}
	catch (std::bad_expected_access<QString>& e)
	{
		*errorMessage = e.error();
		return false;
	}

	errorMessage->clear(); // Just in case, no error happened.
	return true;
}

bool LmDescription::Memory::load(const QDomDocument& document, QString* errorMessage)
{
	if (errorMessage == nullptr)
	{
		assert(errorMessage);
		return false;
	}

	if (document.isNull() == true)
	{
		assert(document.isNull() == false);
		*errorMessage = "XML documnet is null";
		return false;
	}

	// <LogicModule>
	//
	QDomElement logicModuleElement = document.documentElement();

	if (logicModuleElement.isNull() == true || logicModuleElement.tagName() != QLatin1String("LogicModule"))
	{
		errorMessage->append(tr("Cant't find root element LogicModule."));
		return false;
	}

	// <Memory>
	//
	QDomNodeList elements = logicModuleElement.elementsByTagName(QLatin1String("Memory"));

	if (elements.size() != 1)
	{
		*errorMessage = "Expected one Memory section";
		return false;
	}

	QDomElement element = elements.at(0).toElement();

	*this = Memory();

	// Getting data
	//
	try
	{
		m_codeMemorySize = getSectionValue<quint32>(element, QLatin1String("CodeMemorySize")).value();

		m_appMemorySize = getSectionValue<quint32>(element, QLatin1String("AppMemorySize")).value();

		m_appDataOffset = getSectionValue<quint32>(element, QLatin1String("AppDataOffset")).value();
		m_appDataSize = getSectionValue<quint32>(element, QLatin1String("AppDataSize")).value();

		m_appLogicBitDataOffset = getSectionValue<quint32>(element, QLatin1String("AppLogicBitDataOffset")).value();
		m_appLogicBitDataSize = getSectionValue<quint32>(element, QLatin1String("AppLogicBitDataSize")).value();

		m_appLogicWordDataOffset = getSectionValue<quint32>(element, QLatin1String("AppLogicWordDataOffset")).value();
		m_appLogicWordDataSize = getSectionValue<quint32>(element, QLatin1String("AppLogicWordDataSize")).value();

		m_moduleDataOffset = getSectionValue<quint32>(element, QLatin1String("ModuleDataOffset")).value();
		m_moduleDataSize = getSectionValue<quint32>(element, QLatin1String("ModuleDataSize")).value();
		m_moduleCount = getSectionValue<quint32>(element, QLatin1String("ModuleCount")).value();

		m_tuningDataOffset = getSectionValue<quint32>(element, QLatin1String("TuningDataOffset")).value();
		m_tuningDataSize = getSectionValue<quint32>(element, QLatin1String("TuningDataSize")).value();

		m_tuningDataFrameCount = getSectionValue<quint32>(element, QLatin1String("TuningDataFrameCount")).value();
		m_tuningDataFramePayload = getSectionValue<quint32>(element, QLatin1String("TuningDataFramePayload")).value();
		m_tuningDataFrameSize = getSectionValue<quint32>(element, QLatin1String("TuningDataFrameSize")).value();

		m_txDiagDataOffset = getSectionValue<quint32>(element, QLatin1String("TxDiagDataOffset")).value();
		m_txDiagDataSize = getSectionValue<quint32>(element, QLatin1String("TxDiagDataSize")).value();

		m_dataConfigurationOffset = getSectionValue<quint32>(element, QLatin1String("DataConfigurationOffset")).value_or(0xFFFFFFFF);
		m_dataConfigurationSize = getSectionValue<quint32>(element, QLatin1String("DataConfigurationSize")).value_or(0xFFFFFFFF);
	}
	catch (std::bad_expected_access<QString>& e)
	{
		*errorMessage = e.error();
		return false;
	}

	errorMessage->clear(); // Just in case
	return true;
}

bool LmDescription::Memory::isAppLogicBitData(quint32 address) const
{
	return address >= m_appLogicBitDataOffset && address < (m_appLogicBitDataOffset + m_appLogicBitDataSize);
}

bool LmDescription::Memory::isAppLogicWordData(quint32 address) const
{
	return address >= m_appLogicWordDataOffset && address < (m_appLogicWordDataOffset + m_appLogicWordDataSize);
}

bool LmDescription::LogicUnit::load(const QDomDocument& document, QString* errorMessage)
{
	if (errorMessage == nullptr)
	{
		assert(errorMessage);
		return false;
	}

	if (document.isNull() == true)
	{
		assert(document.isNull() == false);
		*errorMessage = "XML documnet is null";
		return false;
	}

	// <LogicModule>
	//
	QDomElement logicModuleElement = document.documentElement();

	if (logicModuleElement.isNull() == true || logicModuleElement.tagName() != QLatin1String("LogicModule"))
	{
		errorMessage->append(tr("Cant't find root element LogicModule."));
		return false;
	}

	// <LogicUnit>
	//
	QDomNodeList elements = logicModuleElement.elementsByTagName(QLatin1String("LogicUnit"));

	if (elements.size() != 1)
	{
		*errorMessage = "Expected one LogicUnit section";
		return false;
	}

	QDomElement element = elements.at(0).toElement();

	*this = LogicUnit();

	// Getting data
	//

	try
	{
		m_alpPhaseTime = getSectionValue<quint32>(element, QLatin1String("ALPPhaseTime")).value();
		m_clockFrequency = getSectionValue<quint32>(element, QLatin1String("ClockFrequency")).value();
		m_cycleDuration = getSectionValue<quint32>(element, QLatin1String("CycleDuration")).value();
		m_idrPhaseTime = getSectionValue<quint32>(element, QLatin1String("IDRPhaseTime")).value();
	}
	catch (std::bad_expected_access<QString>& e)
	{
		*errorMessage = e.error();
		return false;
	}

	errorMessage->clear(); // Just in case
	return true;
}

double LmDescription::LogicUnit::clockTimeSecs() const
{
	if (m_clockFrequency == 0)
	{
		Q_ASSERT(false);
		return 0;
	}

	return 1.0 / static_cast<double>(m_clockFrequency);
}

int LmDescription::LogicUnit::idrPhaseClocks() const
{
	return static_cast<int>(m_idrPhaseTime / (clockTimeSecs() * 1000000.0));
}

int LmDescription::LogicUnit::alpPhaseClocks() const
{
	return static_cast<int>(m_alpPhaseTime / (clockTimeSecs() * 1000000.0));
}

bool LmDescription::OptoInterface::load(const QDomDocument& document, QString* errorMessage)
{
	if (errorMessage == nullptr)
	{
		assert(errorMessage);
		return false;
	}

	if (document.isNull() == true)
	{
		assert(document.isNull() == false);
		*errorMessage = "XML documnet is null";
		return false;
	}

	// <LogicModule>
	//
	QDomElement logicModuleElement = document.documentElement();

	if (logicModuleElement.isNull() == true || logicModuleElement.tagName() != QLatin1String("LogicModule"))
	{
		errorMessage->append(tr("Cant't find root element LogicModule."));
		return false;
	}

	// <OptoInterface>
	//
	QDomNodeList elements = logicModuleElement.elementsByTagName(QLatin1String("OptoInterface"));

	if (elements.size() != 1)
	{
		*errorMessage = "Expected one OptoInterface section";
		return false;
	}

	QDomElement element = elements.at(0).toElement();

	*this = OptoInterface();

	// Getting data
	//
	try
	{
		m_optoPortCount = getSectionValue<quint32>(element, QLatin1String("OptoPortCount")).value();
		m_optoPortAppDataOffset = getSectionValue<quint32>(element, QLatin1String("OptoPortAppDataOffset")).value();
		m_optoPortAppDataSize = getSectionValue<quint32>(element, QLatin1String("OptoPortAppDataSize")).value();
		m_optoInterfaceDataOffset = getSectionValue<quint32>(element, QLatin1String("OptoInterfaceDataOffset")).value();
		m_optoPortDataSize = getSectionValue<quint32>(element, QLatin1String("OptoPortDataSize")).value();
		m_sharedBuffer = getSectionValue<bool>(element, QLatin1String("SharedBuffer")).value_or(false);
	}
	catch (std::bad_expected_access<QString>& e)
	{
		*errorMessage = e.error();
		return false;
	}

	errorMessage->clear(); // Just in case
	return true;
}

bool LmDescription::LanController::isProvideTuning() const
{
	return (static_cast<int>(m_type) & static_cast<int>(E::LanControllerType::Tuning)) != 0;
}

bool LmDescription::LanController::isProvideAppData() const
{
	return (static_cast<int>(m_type) & static_cast<int>(E::LanControllerType::AppData)) != 0;
}

bool LmDescription::LanController::isProvideDiagData() const
{
	return (static_cast<int>(m_type) & static_cast<int>(E::LanControllerType::DiagData)) != 0;
}

int LmDescription::Lan::lanControllerCount() const
{
	return static_cast<int>(m_lanControllers.size());
}

E::LanControllerType LmDescription::Lan::lanControllerType(int index, bool* ok) const
{
	if (index < 0 || index >= lanControllerCount())
	{
		Q_ASSERT(false);
		if (ok != nullptr)
		{
			*ok = false;
		}

		return E::LanControllerType::Unknown;
	}

	if (ok != nullptr)
	{
		*ok = true;
	}
	return m_lanControllers[index].m_type;
}

int LmDescription::Lan::lanControllerPlace(int index, bool* ok) const
{
	if (index < 0 || index >= lanControllerCount())
	{
		Q_ASSERT(false);
		if (ok != nullptr)
		{
			*ok = false;
		}
		return -1;
	}

	if (ok != nullptr)
	{
		*ok = true;
	}
	return m_lanControllers[index].m_place;
}

int LmDescription::Lan::lanControllerConfigVersion(int index, bool* ok) const
{
	if (index < 0 || index >= lanControllerCount())
	{
		Q_ASSERT(false);
		if (ok != nullptr)
		{
			*ok = false;
		}
		return -1;
	}

	if (ok != nullptr)
	{
		*ok = true;
	}
	return m_lanControllers[index].m_configVersion;
}

LmDescription::LanController LmDescription::Lan::lanController(int index, bool* ok) const
{
	if (index < 0 || index >= lanControllerCount())
	{
		Q_ASSERT(false);

		if (ok != nullptr)
		{
			*ok = false;
		}
		return LanController();
	}

	if (ok != nullptr)
	{
		*ok = true;
	}

	return m_lanControllers[index];
}


bool LmDescription::Lan::load(const QDomDocument& document, QString* errorMessage)
{
	if (errorMessage == nullptr)
	{
		assert(errorMessage);
		return false;
	}

	errorMessage->clear();

	if (document.isNull() == true)
	{
		assert(document.isNull() == false);
		*errorMessage = "XML documnet is null";
		return false;
	}

	// <LogicModule>
	//
	QDomElement logicModuleElement = document.documentElement();

	if (logicModuleElement.isNull() == true || logicModuleElement.tagName() != QLatin1String("LogicModule"))
	{
		errorMessage->append(tr("Cant't find root element LogicModule."));
		return false;
	}

	// <LanInterface>
	//
	QDomNodeList elements = logicModuleElement.elementsByTagName(QLatin1String("Lan"));

	if (elements.size() != 1)
	{
		*errorMessage = "Expected one Lan section";
		return false;
	}

	QDomElement element = elements.at(0).toElement();

	*this = Lan();

	// Read LAN version
	//
	{
		const int defaultRupVersion = Rup::V5;
		const int defaultFotipVersion = Fotip::V2;

		bool ok = false;

		// RupVersion
		//
		if (element.hasAttribute(QLatin1String("RupVersion")) == true)
		{
			m_rupVersion = element.attribute("RupVersion").toInt(&ok);
			if (ok == false)
			{
				errorMessage->append(tr("Cant't read attribute RupVersion in Lan section."));
				return false;
			}
		}
		else
		{
			m_rupVersion = defaultRupVersion; // Default value
		}

		// FotipVersion
		//
		if (element.hasAttribute(QLatin1String("FotipVersion")) == true)
		{
			m_fotipVersion = element.attribute("FotipVersion").toInt(&ok);
			if (ok == false)
			{
				errorMessage->append(tr("Cant't read attribute FotipVersion in Lan section."));
				return false;
			}
		}
		else
		{
			m_fotipVersion = defaultFotipVersion; // Default value
		}
	}

	// Read LAN Controllers
	//
	try
	{
		QDomNodeList controllers = element.elementsByTagName(QLatin1String("LanController"));

		int count = controllers.count();
		for (int i = 0; i < count; i++)
		{
			QDomNode node = controllers.at(i);

			LanController li;

			QString typeStr = getSectionValue<QString>(node.toElement(), QLatin1String("Type")).value();

			bool ok = false;
			li.m_type = E::stringToValue<E::LanControllerType>(typeStr, &ok);
			if (ok == false)
			{
				*errorMessage = QString("Unknown LAN controller type: '%1'.").arg(typeStr);
				return false;
			}

			li.m_place = getSectionValue<quint32>(node.toElement(), QLatin1String("Place")).value();

			if (sectionExists(node.toElement(), QLatin1String("ConfigVersion")) == true)
			{
				li.m_configVersion = getSectionValue<quint32>(node.toElement(), QLatin1String("ConfigVersion")).value();
			}
			else
			{
				li.m_configVersion = li.m_type == E::LanControllerType::TuningAndAppAndDiagData ?
										 1 :
										 0; // Default value for config version based on the type
			}

			m_lanControllers.push_back(li);
		}
	}
	catch (std::bad_expected_access<QString>& e)
	{
		*errorMessage = e.error();
		return false;
	}

	return true;
}

bool LmDescription::Other::load(const QDomDocument& document, QString* errorMessage)
{
	if (errorMessage == nullptr)
	{
		assert(errorMessage);
		return false;
	}

	errorMessage->clear();

	if (document.isNull() == true)
	{
		assert(document.isNull() == false);
		*errorMessage = "XML documnet is null";
		return false;
	}

	// <LogicModule>
	//
	QDomElement logicModuleElement = document.documentElement();

	if (logicModuleElement.isNull() == true || logicModuleElement.tagName() != QLatin1String("LogicModule"))
	{
		errorMessage->append(tr("Cant't find root element LogicModule."));
		return false;
	}

	// <Other>
	//
	QDomNodeList elements = logicModuleElement.elementsByTagName(QLatin1String("Other"));

	if (elements.size() != 1)
	{
		*errorMessage = "Expected one section Other";
		return false;
	}

	QDomElement element = elements.at(0).toElement();

	*this = Other{};

	// Getting data
	//
	try
	{
		ocmTxDataSizeLimit = getSectionValue<quint32>(element, QLatin1String("OcmTxDataSizeLimit")).value();
		ocmRxDataSizeLimit = getSectionValue<quint32>(element, QLatin1String("OcmRxDataSizeLimit")).value();
	}
	catch (std::bad_expected_access<QString>& e)
	{
		*errorMessage = e.error();
		return false;
	}

	return true;
}

std::optional<LmDescription::DataConfigurationParam> LmDescription::DataConfiguration::param(const QString& id) const
{
	auto it = std::find_if(params.begin(),
						   params.end(),
						   [&id](const DataConfigurationParam& param)
						   {
							   return param.id == id;
						   });
	if (it != params.end())
	{
		return *it;
	}

	return std::nullopt;
}

bool LmDescription::DataConfiguration::load(const QDomDocument& document, QString* errorMessage)
{
	*this = {};

	if (errorMessage == nullptr)
	{
		assert(errorMessage);
		return false;
	}

	errorMessage->clear();

	if (document.isNull() == true)
	{
		assert(document.isNull() == false);
		*errorMessage = "XML document is null";
		return false;
	}

	// <LogicModule>
	//
	QDomElement logicModuleElement = document.documentElement();
	if (logicModuleElement.isNull() == true || logicModuleElement.tagName() != QLatin1String("LogicModule"))
	{
		errorMessage->append(tr("Cant't find root element LogicModule."));
		return false;
	}

	// <DataConfiguration>
	//
	QDomNodeList elements = logicModuleElement.elementsByTagName(QLatin1String("DataConfiguration"));
	if (elements.size() == 0)
	{
		// DataConfiguration is an optional section, now it present only in ACM.
		//
		return true;
	}

	if (elements.size() > 1)
	{
		*errorMessage = "Too many 'DataConfiguration' sections: expected 0 or 1";
		return false;
	}

	// Getting data
	//
	try
	{
		QDomElement dataConfigurationElement = elements.at(0).toElement();
		QDomNodeList confParamElements = dataConfigurationElement.elementsByTagName("ConfParam");

		for (auto sz = confParamElements.size(), index = 0; index < sz; index++)
		{
			bool convertOk = false;
			QDomElement element = confParamElements.at(index).toElement();
			DataConfigurationParam param{};

			param.id = element.attribute(QLatin1String("ID"));
			if (param.id.isEmpty() == true)
			{
				throw std::runtime_error{"Parse ConfParam error, no ID attribute."};
			}

			param.offset = element.attribute(QLatin1String("Offset")).toUInt(&convertOk);
			if (convertOk == false)
			{
				throw std::runtime_error{"Parse ConfParam error, cannot convert attribute Offset to uint32."};
			}

			param.sizeBits = element.attribute(QLatin1String("SizeBits")).toUInt(&convertOk);
			if (convertOk == false)
			{
				throw std::runtime_error{"Parse ConfParam error, cannot convert attribute SizeBits to uint32."};
			}

			if (auto er = E::stringToValue<E::DataFormat>(element.attribute(QLatin1String("DataFormat"))); //
				er.second == false)
			{
				throw std::runtime_error{QString{"Cannot convert '%1' to enum E::DataFormat."}
											 .arg(element.attribute(QLatin1String("DataFormat")))
											 .toStdString()};
			}
			else
			{
				param.format = er.first;
			}

			params.push_back(param);
		}
	}
	catch (std::bad_expected_access<QString>& e)
	{
		*errorMessage = e.error();
		return false;
	}
	catch (std::runtime_error& e)
	{
		*errorMessage = e.what();
		return false;
	}

	return true;
}

QString LmDescription::name() const
{
	return m_name;
}

int LmDescription::descriptionNumber() const
{
	return m_descriptionNumber;
}

const QString& LmDescription::configurationStringFile() const
{
	return m_configurationScriptFile;
}

QString LmDescription::jsConfigurationStringFile() const
{
	return m_configurationScriptFile;
}

const QString& LmDescription::version() const
{
	return m_version;
}

const LmDescription::FlashMemory& LmDescription::flashMemory() const
{
	return m_flashMemory;
}

const LmDescription::Memory& LmDescription::memory() const
{
	return m_memory;
}

const LmDescription::LogicUnit& LmDescription::logicUnit() const
{
	return m_logicUnit;
}

const LmDescription::OptoInterface& LmDescription::optoInterface() const
{
	return m_optoInterface;
}

const LmDescription::Lan& LmDescription::lan() const
{
	return m_lan;
}

const LmDescription::Other& LmDescription::other() const
{
	return m_other;
}

const LmDescription::DataConfiguration& LmDescription::dataConfiguration() const
{
	return m_dataConfiguration;
}

int LmDescription::jsLanControllerType(int index)
{
	return static_cast<int>(m_lan.lanControllerType(index));
}

int LmDescription::jsLanControllerPlace(int index)
{
	return static_cast<int>(m_lan.lanControllerPlace(index));
}

int LmDescription::jsLanControllerConfigVersion(int index)
{
	return static_cast<int>(m_lan.lanControllerConfigVersion(index));
}

bool LmDescription::checkAfbVersions() const
{
	return m_checkAfbVersions;
}

quint32 LmDescription::checkAfbVersionsOffset(bool absoluteValue) const
{
	return m_checkAfbVersionsOffset + (absoluteValue ? m_memory.m_appDataOffset : 0);
}

const std::vector<std::shared_ptr<Afb::AfbElement>>& LmDescription::afbElements() const
{
	return m_afbElements;
}

std::vector<std::shared_ptr<Afb::AfbElement>> LmDescription::afbElements(int opCode) const
{
	std::vector<std::shared_ptr<Afb::AfbElement>> elements;

	for (auto& elem : m_afbElements)
	{
		if (elem->opCode() == opCode)
		{
			elements.push_back(elem);
		}
	}

	return elements;
}

std::vector<std::shared_ptr<Afb::AfbElement>> LmDescription::afbElements(const QString& componentCaption) const
{
	std::shared_ptr<Afb::AfbComponent> afbComp = component(componentCaption);

	if (afbComp == nullptr)
	{
		return std::vector<std::shared_ptr<Afb::AfbElement>>();
	}

	return afbElements(afbComp->opCode());
}

const std::shared_ptr<Afb::AfbElement> LmDescription::afbElement(const QString& elementCaption) const
{
	for (auto& elem : m_afbElements)
	{
		if (elem->caption() == elementCaption)
		{
			return elem;
		}
	}

	return nullptr;
}

std::shared_ptr<Afb::AfbComponent> LmDescription::component(int opCode) const
{
	auto it = m_afbComponents.find(opCode);
	if (it == m_afbComponents.end())
	{
		return nullptr;
	}

	return it->second;
}

std::shared_ptr<Afb::AfbComponent> LmDescription::component(const QString& caption) const
{
	for (auto& afbComponent : m_afbComponents)
	{
		if (afbComponent.second == nullptr)
		{
			Q_ASSERT(afbComponent.second != nullptr);
			continue;
		}

		if (afbComponent.second->caption() == caption)
		{
			return afbComponent.second;
		}
	}

	return nullptr;
}

const std::map<int, std::shared_ptr<Afb::AfbComponent>>& LmDescription::afbComponents() const
{
	return m_afbComponents;
}

LmCommand LmDescription::command(int commandCode) const
{
	auto it = m_commands.find(commandCode);
	if (it != m_commands.end())
	{
		return it->second;
	}
	else
	{
		return LmCommand();
	}
}

const LmCommand* LmDescription::commandPtr(int commandCode) const
{
	auto it = m_commands.find(commandCode);

	if (it != m_commands.end())
	{
		return &it->second;
	}
	else
	{
		return nullptr;
	}
}

const std::map<int, LmCommand>& LmDescription::commands() const
{
	return m_commands;
}

std::vector<LmCommand> LmDescription::commandsAsVector() const
{
	std::vector<LmCommand> result;
	result.reserve(m_commands.size());

	for (auto p : m_commands)
	{
		result.push_back(p.second);
	}

	return result;
}

int LmDescription::logicUnitCommandsVersion() const
{
	return m_logicUnitCommandsVersion;
}

bool LmDescription::isCommandsAvailable(const std::vector<LmCommandCode>& commandsCodes) const
{
	for (const LmCommandCode cmd : commandsCodes)
	{
		if (commandPtr(cmd) == nullptr)
		{
			return false;
		}
	}

	return true;
}

bool LmDescription::isBitAccAvailable() const
{
	if (m_bitAccAvailable.has_value() == false)
	{
		static const std::vector<LmCommandCode> bitAccCommands = {
			LmCommand::RESET,
			LmCommand::SET,
			LmCommand::OR,
			LmCommand::AND,
			LmCommand::NOT,
			LmCommand::LSHIFT0,
			LmCommand::LSHIFT1,
			LmCommand::MOV_ADDR_ACC,
			LmCommand::MOV_ACC_ADDR,
			LmCommand::MOVC_ACC,
			LmCommand::MOVB_ACC_ADDR,
			LmCommand::MOVB_ADDR_ACC,
		};

		m_bitAccAvailable = isCommandsAvailable(bitAccCommands);
	}

	return m_bitAccAvailable.value();
}
