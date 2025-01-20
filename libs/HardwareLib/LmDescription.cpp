#ifndef HARDWARE_LIB_DOMAIN
#error Do not include this file in the project! Link HardwareLib instead.
#endif

#include "../UtilsLib/DomXmlHelper.h"
#include <HardwareLib/LmDescription.h>
#include <HardwareLib/DataProtocols.h>
#include <HardwareLib/DeviceModule.h>
#include <HardwareLib/PropertyNames.h>


bool LmCommand::loadFromXml(const QDomElement& element, QString* errorMessage)
{
	if (errorMessage == nullptr ||
		element.isNull() == true ||
		element.tagName() != QLatin1String("Command"))
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
	if (DomXmlHelper::getIntAttributeIfExists(element, "ConstRuntime", LmCommand::UNDEFINED_PARAM,
											  &constRuntime, errorMessage, 10) == false)
	{
		return false;
	}

	// WriteToBitMemRuntime
	//
	if (DomXmlHelper::getIntAttributeIfExists(element, "WriteToBitMemRuntime", LmCommand::UNDEFINED_PARAM,
											  &writeToBitMemRuntime, errorMessage, 10) == false)
	{
		return false;
	}

	// WriteToWordMemRuntime
	//
	if (DomXmlHelper::getIntAttributeIfExists(element, "WriteToWordMemRuntime", LmCommand::UNDEFINED_PARAM,
											  &writeToWordMemRuntime, errorMessage, 10) == false)
	{
		return false;
	}

	// PreFbReadWordTime
	//
	if (DomXmlHelper::getIntAttributeIfExists(element, "PreFbReadWordTime", LmCommand::UNDEFINED_PARAM,
											  &preFbReadWordTime, errorMessage, 10) == false)
	{
		return false;
	}

	// PostFbReadWordTime
	//
	if (DomXmlHelper::getIntAttributeIfExists(element, "PostFbReadWordTime", LmCommand::UNDEFINED_PARAM,
											  &postFbReadWordTime, errorMessage, 10) == false)
	{
		return false;
	}

	// PreFbReadBitTime
	//
	if (DomXmlHelper::getIntAttributeIfExists(element, "PreFbReadBitTime", LmCommand::UNDEFINED_PARAM,
											  &preFbReadBitTime, errorMessage, 10) == false)
	{
		return false;
	}

	// PostFbReadBitTime
	//
	if (DomXmlHelper::getIntAttributeIfExists(element, "PostFbReadBitTime", LmCommand::UNDEFINED_PARAM,
											  &postFbReadBitTime, errorMessage, 10) == false)
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

LmDescription::LmDescription(QObject *parent)
	: QObject(parent)
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
		m_afbComponents.insert({p.first, afbComponentCopy});
	}

	m_afbElements.clear();
	m_afbElements.reserve(src.m_afbElements.size());
	for (std::shared_ptr<Afb::AfbElement> afb : src.m_afbElements)
	{
		std::shared_ptr<Afb::AfbElement> afbCopy = std::make_shared<Afb::AfbElement>(*afb.get());
		m_afbElements.push_back(afbCopy);
	}

	return *this;
}

LmDescription::~LmDescription()
{
}

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
		errorMessage->append(tr(" Error %1, line %2, column %3").
								arg(pr.errorMessage).arg(pr.errorLine).arg(pr.errorColumn));
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
		errorMessage->append(tr(" Error %1, line %2, column %3").
								arg(pr.errorMessage).arg(pr.errorLine).arg(pr.errorColumn));
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

	if (logicModuleElement.isNull() == true ||
		logicModuleElement.tagName() != QLatin1String("LogicModule"))
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

		if (node.isNull() == true ||
			node.isElement() == false)
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

		if (afbNode.isNull() == true ||
			afbNode.isElement() == false)
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

		if (afbNode.isNull() == true ||
			afbNode.isElement() == false)
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
	if (logicModule == nullptr ||
		(logicModule->isFSCConfigurationModule() == false && logicModule->isVdu() == false))
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

	if (logicModuleElement.isNull() == true ||
		logicModuleElement.tagName() != QLatin1String("LogicModule"))
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

	// Func for gettiong data from some xml section
	//
	auto getSectionUintValue =
		[&element](QLatin1String section, QString* errorMessage) -> quint32
		{
			QDomNodeList nl = element.elementsByTagName(section);
			if (nl.size() != 1)
			{
				*errorMessage = QString("Expected one %1 section.").arg(section);
				return 0xFFFFFFFF;
			}

			QString nodeText = nl.at(0).toElement().text();
			return nodeText.toUInt();
		};

	auto getSectionUintDefaultValue =
		[&element](QLatin1String section, quint32 defaultValue) -> quint32
		{
			QDomNodeList nl = element.elementsByTagName(section);
			if (nl.size() != 1)
			{
				return defaultValue;
			}

			QString nodeText = nl.at(0).toElement().text();
			return nodeText.toUInt();
		};

	auto getSectionBoolDefaultValue =
		[&element](QLatin1String section, bool defaultValue) -> bool
		{
			QDomNodeList nl = element.elementsByTagName(section);
			if (nl.size() != 1)
			{
				return defaultValue;
			}

			QString nodeText = nl.at(0).toElement().text();
			return nodeText.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;
		};

	// Getting data
	//

	errorMessage->clear();	// Just in case

	m_appLogicFrameCount = getSectionUintValue(QLatin1String("AppLogicFrameCount"), errorMessage);
	m_appLogicFramePayload = getSectionUintValue(QLatin1String("AppLogicFramePayload"), errorMessage);
	m_appLogicFrameSize = getSectionUintValue(QLatin1String("AppLogicFrameSize"), errorMessage);
	m_appLogicUartId = getSectionUintDefaultValue(QLatin1String("AppLogicUartID"), 0);
	m_appLogicWriteBitstream = getSectionBoolDefaultValue(QLatin1String("AppLogicWriteBitstream"), false);

	m_configFrameCount = getSectionUintValue(QLatin1String("ConfigFrameCount"), errorMessage);
	m_configFramePayload = getSectionUintValue(QLatin1String("ConfigFramePayload"), errorMessage);
	m_configFrameSize = getSectionUintValue(QLatin1String("ConfigFrameSize"), errorMessage);
	m_configUartId = getSectionUintDefaultValue(QLatin1String("ConfigUartID"), 0);
	m_configWriteBitstream = getSectionBoolDefaultValue(QLatin1String("ConfigWriteBitstream"), false);

	m_tuningFrameCount = getSectionUintValue(QLatin1String("TuningFrameCount"), errorMessage);
	m_tuningFramePayload = getSectionUintValue(QLatin1String("TuningFramePayload"), errorMessage);
	m_tuningFrameSize = getSectionUintValue(QLatin1String("TuningFrameSize"), errorMessage);
	m_tuningUartId = getSectionUintDefaultValue(QLatin1String("TuningUartID"), 0);
	m_tuningWriteBitstream = getSectionBoolDefaultValue(QLatin1String("TuningWriteBitstream"), false);

	m_maxConfigurationCount = getSectionUintValue(QLatin1String("MaxConfigurationCount"), errorMessage);
	m_singleConfigFirstFrame = getSectionUintValue(QLatin1String("SingleConfigFirstFrame"), errorMessage);
	m_singleConfigFrameCount = getSectionUintValue(QLatin1String("SingleConfigFrameCount"), errorMessage);
	m_singleConfigUniqueIdOffset = getSectionUintValue(QLatin1String("SingleConfigUniqueIDOffset"), errorMessage);

	return errorMessage->isEmpty();
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

	if (logicModuleElement.isNull() == true ||
		logicModuleElement.tagName() != QLatin1String("LogicModule"))
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

	// Func for gettiong data from some xml section
	//
	auto getSectionUintValue =
		[&element](QLatin1String section, QString* errorMessage) -> quint32
		{
			QDomNodeList nl = element.elementsByTagName(section);

			if (nl.size() != 1)
			{
				*errorMessage = QString("Expected one %1 section.").arg(section);
				return 0xFFFFFFFF;
			}

			QString nodeText = nl.at(0).toElement().text();
			return nodeText.toUInt();
		};

	// Getting data
	//
	errorMessage->clear();	// Just in case

	m_codeMemorySize = getSectionUintValue(QLatin1String("CodeMemorySize"), errorMessage);

	m_appMemorySize = getSectionUintValue(QLatin1String("AppMemorySize"), errorMessage);

	m_appDataOffset = getSectionUintValue(QLatin1String("AppDataOffset"), errorMessage);
	m_appDataSize= getSectionUintValue(QLatin1String("AppDataSize"), errorMessage);

	m_appLogicBitDataOffset = getSectionUintValue(QLatin1String("AppLogicBitDataOffset"), errorMessage);
	m_appLogicBitDataSize = getSectionUintValue(QLatin1String("AppLogicBitDataSize"), errorMessage);

	m_appLogicWordDataOffset = getSectionUintValue(QLatin1String("AppLogicWordDataOffset"), errorMessage);
	m_appLogicWordDataSize = getSectionUintValue(QLatin1String("AppLogicWordDataSize"), errorMessage);

	m_moduleDataOffset = getSectionUintValue(QLatin1String("ModuleDataOffset"), errorMessage);
	m_moduleDataSize = getSectionUintValue(QLatin1String("ModuleDataSize"), errorMessage);
	m_moduleCount = getSectionUintValue(QLatin1String("ModuleCount"), errorMessage);

	m_tuningDataOffset = getSectionUintValue(QLatin1String("TuningDataOffset"), errorMessage);
	m_tuningDataSize = getSectionUintValue(QLatin1String("TuningDataSize"), errorMessage);

	m_tuningDataFrameCount = getSectionUintValue(QLatin1String("TuningDataFrameCount"), errorMessage);
	m_tuningDataFramePayload = getSectionUintValue(QLatin1String("TuningDataFramePayload"), errorMessage);
	m_tuningDataFrameSize = getSectionUintValue(QLatin1String("TuningDataFrameSize"), errorMessage);

	m_txDiagDataOffset = getSectionUintValue(QLatin1String("TxDiagDataOffset"), errorMessage);
	m_txDiagDataSize = getSectionUintValue(QLatin1String("TxDiagDataSize"), errorMessage);

	return errorMessage->isEmpty();
}

bool LmDescription::Memory::isAppLogicBitData(quint32 address) const
{
	return address >= m_appLogicBitDataOffset &&
	       address < (m_appLogicBitDataOffset + m_appLogicBitDataSize);
}

bool LmDescription::Memory::isAppLogicWordData(quint32 address) const
{
	return address >= m_appLogicWordDataOffset &&
	       address < (m_appLogicWordDataOffset + m_appLogicWordDataSize);
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

	if (logicModuleElement.isNull() == true ||
		logicModuleElement.tagName() != QLatin1String("LogicModule"))
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

	// Func for gettiong data from some xml section
	//
	auto getSectionUintValue =
		[&element](QLatin1String section, QString* errorMessage) -> quint32
		{
			QDomNodeList nl = element.elementsByTagName(section);

			if (nl.size() != 1)
			{
				*errorMessage = QString("Expected one %1 section.").arg(section);
				return 0xFFFFFFFF;
			}

			QString nodeText = nl.at(0).toElement().text();
			return nodeText.toUInt();
		};

	// Getting data
	//
	errorMessage->clear();	// Just in case

	m_alpPhaseTime = getSectionUintValue(QLatin1String("ALPPhaseTime"), errorMessage);
	m_clockFrequency= getSectionUintValue(QLatin1String("ClockFrequency"), errorMessage);
	m_cycleDuration = getSectionUintValue(QLatin1String("CycleDuration"), errorMessage);
	m_idrPhaseTime = getSectionUintValue(QLatin1String("IDRPhaseTime"), errorMessage);

	return errorMessage->isEmpty();
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

	if (logicModuleElement.isNull() == true ||
		logicModuleElement.tagName() != QLatin1String("LogicModule"))
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

	// Func for gettiong data from some xml section
	//
	auto getSectionUintValue =
		[&element](QLatin1String section, QString* errorMessage) -> quint32
		{
			QDomNodeList nl = element.elementsByTagName(section);

			if (nl.size() != 1)
			{
				*errorMessage = QString("Expected one %1 section.").arg(section);
				return 0xFFFFFFFF;
			}

			QString nodeText = nl.at(0).toElement().text();
			return nodeText.toUInt();
		};
	auto getSectionBoolDefaultValue =
		[&element](QLatin1String section, bool defaultValue) -> bool
		{
			QDomNodeList nl = element.elementsByTagName(section);
			if (nl.size() != 1)
			{
				return defaultValue;
			}

			QString nodeText = nl.at(0).toElement().text();
			return nodeText.compare(QLatin1String("true"), Qt::CaseInsensitive) == 0;
		};

	// Getting data
	//
	errorMessage->clear();	// Just in case

	m_optoPortCount = getSectionUintValue(QLatin1String("OptoPortCount"), errorMessage);
	m_optoPortAppDataOffset= getSectionUintValue(QLatin1String("OptoPortAppDataOffset"), errorMessage);
	m_optoPortAppDataSize = getSectionUintValue(QLatin1String("OptoPortAppDataSize"), errorMessage);
	m_optoInterfaceDataOffset = getSectionUintValue(QLatin1String("OptoInterfaceDataOffset"), errorMessage);
	m_optoPortDataSize = getSectionUintValue(QLatin1String("OptoPortDataSize"), errorMessage);
	m_sharedBuffer = getSectionBoolDefaultValue(QLatin1String("SharedBuffer"), false);

	return errorMessage->isEmpty();
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

	if (document.isNull() == true)
	{
		assert(document.isNull() == false);
		*errorMessage = "XML documnet is null";
		return false;
	}

	// <LogicModule>
	//
	QDomElement logicModuleElement = document.documentElement();

	if (logicModuleElement.isNull() == true ||
		logicModuleElement.tagName() != QLatin1String("LogicModule"))
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

	errorMessage->clear();	// Just in case

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
			m_rupVersion = defaultRupVersion;	// Default value
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
			m_fotipVersion = defaultFotipVersion;	// Default value
		}
	}

	// Func for gettiong data from some xml section
	//
	auto getSectionUintValue =
		[](QDomElement element, QLatin1String section, QString* errorMessage) -> quint32
		{
			QDomNodeList nl = element.elementsByTagName(section);

			if (nl.size() != 1)
			{
				*errorMessage = QString("Expected one %1 section.").arg(section);
				return 0xFFFFFFFF;
			}

			QString nodeText = nl.at(0).toElement().text();
			return nodeText.toUInt();
		};
	auto getSectionStringValue =
			[](QDomElement element, QLatin1String section, QString* errorMessage) -> QString
			{
				QDomNodeList nl = element.elementsByTagName(section);

				if (nl.size() != 1)
				{
					*errorMessage = QString("Expected one %1 section.").arg(section);
					return "";
				}

				QString nodeText = nl.at(0).toElement().text();
				return nodeText;
			};

	// Read LAN Controllers
	//

	QDomNodeList controllers = element.elementsByTagName(QLatin1String("LanController"));

	int count = controllers.count();
	for (int i = 0; i < count; i++)
	{
		QDomNode node = controllers.at(i);

		LanController li;

		QString typeStr = getSectionStringValue(node.toElement(), QLatin1String("Type"), errorMessage);

		bool ok = false;
		li.m_type = E::stringToValue<E::LanControllerType>(typeStr, &ok);
		if (ok == false)
		{
			*errorMessage = QString("Unknown LAN controller type: '%1'.").arg(typeStr);
			break;
		}

		li.m_place = getSectionUintValue(node.toElement(), QLatin1String("Place"), errorMessage);

		m_lanControllers.push_back(li);
	}

	return errorMessage->isEmpty();
}

bool LmDescription::Other::load(const QDomDocument& document, QString* errorMessage)
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

	if (logicModuleElement.isNull() == true ||
		logicModuleElement.tagName() != QLatin1String("LogicModule"))
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

	// Func for gettiong data from some xml section
	//
	auto getSectionUintValue =
		[&element](QLatin1String section, QString* errorMessage) -> quint32
		{
			QDomNodeList nl = element.elementsByTagName(section);

			if (nl.size() != 1)
			{
				*errorMessage = QString("Expected one %1 section.").arg(section);
				return 0xFFFFFFFF;
			}

			QString nodeText = nl.at(0).toElement().text();
			return nodeText.toUInt();
		};

	// Getting data
	//
	ocmTxDataSizeLimit = getSectionUintValue(QLatin1String("OcmTxDataSizeLimit"), errorMessage);
	ocmRxDataSizeLimit = getSectionUintValue(QLatin1String("OcmRxDataSizeLimit"), errorMessage);

	return errorMessage->isEmpty();
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

int LmDescription::jsLanControllerType(int index)
{
	return static_cast<int>(m_lan.lanControllerType(index));
}

int LmDescription::jsLanControllerPlace(int index)
{
	return static_cast<int>(m_lan.lanControllerPlace(index));
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

	for(auto& elem : m_afbElements)
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
	for(auto& elem : m_afbElements)
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
	for(auto& afbComponent : m_afbComponents)
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
	if (it !=m_commands.end())
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

	if (it !=m_commands.end())
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
	for(const LmCommandCode cmd : commandsCodes)
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
		static const std::vector<LmCommandCode> bitAccCommands =
		{
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


