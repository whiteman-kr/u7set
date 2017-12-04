#include "LmDescription.h"
#include "DeviceObject.h"

LmDescription::LmDescription(QObject *parent)
	: QObject(parent)
{

}

LmDescription::~LmDescription()
{

}

bool LmDescription::load(const QByteArray& file, QString* errorMessage)
{
	if (errorMessage == nullptr)
	{
		assert(false);
		return false;
	}

	if (file.isEmpty() == true)
	{
		*errorMessage = tr("Input LogicModule description file is empty.");
		return false;
	}

	QDomDocument doc;
	int parseErrorLine = -1;
	int parseErrorColumn = -1;

	bool ok = doc.setContent(file, false, errorMessage, &parseErrorLine, &parseErrorColumn);
	if (ok == false)
	{
		errorMessage->append(tr(" Error line %1, column %2").arg(parseErrorLine).arg(parseErrorColumn));
		return false;
	}

	return load(doc, errorMessage);
}

bool LmDescription::load(const QString& file, QString* errorMessage)
{
	if (errorMessage == nullptr)
	{
		assert(false);
		return false;
	}

	if (file.isEmpty() == true)
	{
		*errorMessage = tr("Input LogicModule description file is empty.");
		return false;
	}

	QDomDocument doc;
	int parseErrorLine = -1;
	int parseErrorColumn = -1;

	bool ok = doc.setContent(file, errorMessage, &parseErrorLine, &parseErrorColumn);
	if (ok == false)
	{
		errorMessage->append(tr(" Error line %1, column %2").arg(parseErrorLine).arg(parseErrorColumn));
		return false;
	}

	return load(doc, errorMessage);
}

bool LmDescription::load(QDomDocument doc, QString* errorMessage)
{
	if (errorMessage == nullptr)
	{
		assert(false);
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
		errorMessage->append(tr("Cant't find root element <LogicModule>."));
		return false;
	}

	// Attribute DescriptionNumber
	//
	QString s = logicModuleElement.attribute("DescriptionNumber");

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
    m_configurationScriptFile = logicModuleElement.attribute("ConfigurationScriptFile");

    if (m_configurationScriptFile.isEmpty() == true)
    {
        errorMessage->append(tr("Cant't find attribute ConfigurationScriptFile"));
        return false;
    }

    // Attribute Version
    //
    m_version = logicModuleElement.attribute("Version");

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

	// <AFBImplementation> -- Loading Application Functional Components
	//
	{
		QDomNodeList afbcElementList = logicModuleElement.elementsByTagName("AFBImplementation");

		if (afbcElementList.size() != 1)
		{
			errorMessage->append(tr("Expected one element <AFBImplementation>"));
			return false;
		}

		QDomElement afbcElement = afbcElementList.at(0).toElement();

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
		QDomNodeList afbsElementList = logicModuleElement.elementsByTagName("AFBL");
		if (afbsElementList.size() != 1)
		{
			errorMessage->append(tr("Expected one element <AFBL>"));
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

bool LmDescription::loadAfbComponents(const QDomElement& element, QString* errorMessage)
{
	assert(element.tagName() == "AFBImplementation");

	if (errorMessage == nullptr)
	{
		assert(errorMessage);
		return false;
	}

	// Enumerate <AFBComponent>
	//
	m_afbComponents.clear();

	QDomNodeList afbNodeList = element.elementsByTagName("AFBComponent");

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
	assert(element.tagName() == "AFBL");

	if (errorMessage == nullptr)
	{
		assert(errorMessage);
		return false;
	}

	// Enumerate <AFB>
	//
	QDomNodeList afbNodeList = element.elementsByTagName("AFB");

	m_afbs.clear();
	m_afbs.reserve(afbNodeList.size());

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

		m_afbs.push_back(afb);
	}

	// Set AFB Components to AFbElement
	//
	for (std::shared_ptr<Afb::AfbElement> afb : m_afbs)
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
		logicModule->isFSCConfigurationModule() == false)
	{
		assert(logicModule);
		assert(logicModule->isFSCConfigurationModule());
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
		errorMessage->append(tr("Cant't find root element <LogicModule>."));
		return false;
	}

	// <FlashMemory>
	QDomNodeList elements = logicModuleElement.elementsByTagName(QLatin1String("FlashMemory"));

	if (elements.size() != 1)
	{
		*errorMessage = "Expected one <FlashMemory> section";
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
	m_appLogicFrameSize = getSectionUintValue(QLatin1String("AppLogicFrameSize"), errorMessage);
	m_appLogicUartId = getSectionUintDefaultValue(QLatin1String("AppLogicUartID"), 0);
	m_appLogicWriteBitstream = getSectionBoolDefaultValue(QLatin1String("AppLogicWriteBitstream"), false);

	m_configFrameCount = getSectionUintValue(QLatin1String("ConfigFrameCount"), errorMessage);
	m_configFrameSize = getSectionUintValue(QLatin1String("ConfigFrameSize"), errorMessage);
	m_configUartId = getSectionUintDefaultValue(QLatin1String("ConfigUartID"), 0);
	m_configWriteBitstream = getSectionBoolDefaultValue(QLatin1String("ConfigWriteBitstream"), false);

	m_tuningFrameCount = getSectionUintValue(QLatin1String("TuningFrameCount"), errorMessage);
	m_tuningFrameSize = getSectionUintValue(QLatin1String("TuningFrameSize"), errorMessage);
	m_tuningUartId = getSectionUintDefaultValue(QLatin1String("TuningUartID"), 0);
	m_tuningWriteBitstream = getSectionBoolDefaultValue(QLatin1String("TuningWriteBitstream"), false);

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
		errorMessage->append(tr("Cant't find root element <LogicModule>."));
		return false;
	}

	// <Memory>
	//
	QDomNodeList elements = logicModuleElement.elementsByTagName(QLatin1String("Memory"));

	if (elements.size() != 1)
	{
		*errorMessage = "Expected one <Memory> section";
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

	m_tuningDataOffset = getSectionUintValue(QLatin1String("TuningDataOffset"), errorMessage);
	m_tuningDataSize = getSectionUintValue(QLatin1String("TuningDataSize"), errorMessage);

	m_txDiagDataOffset = getSectionUintValue(QLatin1String("TxDiagDataOffset"), errorMessage);
	m_txDiagDataSize = getSectionUintValue(QLatin1String("TxDiagDataSize"), errorMessage);

	return errorMessage->isEmpty();
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
		errorMessage->append(tr("Cant't find root element <LogicModule>."));
		return false;
	}

	// <LogicUnit>
	//
	QDomNodeList elements = logicModuleElement.elementsByTagName(QLatin1String("LogicUnit"));

	if (elements.size() != 1)
	{
		*errorMessage = "Expected one <LogicUnit> section";
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
		errorMessage->append(tr("Cant't find root element <LogicModule>."));
		return false;
	}

	// <OptoInterface>
	//
	QDomNodeList elements = logicModuleElement.elementsByTagName(QLatin1String("OptoInterface"));

	if (elements.size() != 1)
	{
		*errorMessage = "Expected one <OptoInterface> section";
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

	// Getting data
	//
	errorMessage->clear();	// Just in case

	m_optoPortCount = getSectionUintValue(QLatin1String("OptoPortCount"), errorMessage);
	m_optoPortAppDataOffset= getSectionUintValue(QLatin1String("OptoPortAppDataOffset"), errorMessage);
	m_optoPortAppDataSize = getSectionUintValue(QLatin1String("OptoPortAppDataSize"), errorMessage);
	m_optoInterfaceDataOffset = getSectionUintValue(QLatin1String("OptoInterfaceDataOffset"), errorMessage);
	m_optoPortDataSize = getSectionUintValue(QLatin1String("OptoPortDataSize"), errorMessage);

	return errorMessage->isEmpty();
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

const std::vector<std::shared_ptr<Afb::AfbElement>>& LmDescription::afbs() const
{
	return m_afbs;
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

const std::map<int, std::shared_ptr<Afb::AfbComponent>>& LmDescription::afbComponents() const
{
	return m_afbComponents;
}

