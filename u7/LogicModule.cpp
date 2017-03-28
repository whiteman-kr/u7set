#include "LogicModule.h"

LogicModule::LogicModule(QObject *parent)
	: QObject(parent)
{

}

LogicModule::~LogicModule()
{

}

bool LogicModule::load(const QByteArray& file, QString* errorMessage)
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

bool LogicModule::load(const QString& file, QString* errorMessage)
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

bool LogicModule::load(QDomDocument doc, QString* errorMessage)
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

bool LogicModule::loadAfbComponents(const QDomElement& element, QString* errorMessage)
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


bool LogicModule::loadAfbs(const QDomElement& element, QString* errorMessage)
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

void LogicModule::dump() const
{
	qDebug() << "LogicModule Description:";

	qDebug() << "\tDescriptionNumber: " << m_descriptionNumber;

	return;
}

bool LogicModule::FlashMemory::load(const QDomDocument& document, QString* errorMessage)
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

	// Getting data
	//

	errorMessage->clear();	// Just in case

	appLogicFrameCount = getSectionUintValue(QLatin1String("AppLogicFrameCount"), errorMessage);
	appLogicFrameSize= getSectionUintValue(QLatin1String("AppLogicFrameSize"), errorMessage);
	configFrameCount = getSectionUintValue(QLatin1String("ConfigFrameCount"), errorMessage);
	configFrameSize = getSectionUintValue(QLatin1String("ConfigFrameSize"), errorMessage);
	tuningFrameCount = getSectionUintValue(QLatin1String("TuningFrameCount"), errorMessage);
	tuningFrameSize = getSectionUintValue(QLatin1String("TuningFrameSize"), errorMessage);

	return errorMessage->isEmpty();
}

bool LogicModule::Memory::load(const QDomDocument& document, QString* errorMessage)
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

	appDataOffset = getSectionUintValue(QLatin1String("AppDataOffset"), errorMessage);
	appDataSize= getSectionUintValue(QLatin1String("AppDataSize"), errorMessage);

	appLogicBitDataOffset = getSectionUintValue(QLatin1String("AppLogicBitDataOffset"), errorMessage);
	appLogicBitDataSize = getSectionUintValue(QLatin1String("AppLogicBitDataSize"), errorMessage);

	appLogicWordDataOffset = getSectionUintValue(QLatin1String("AppLogicWordDataOffset"), errorMessage);
	appLogicWordDataSize = getSectionUintValue(QLatin1String("AppLogicWordDataSize"), errorMessage);

	moduleDataOffset = getSectionUintValue(QLatin1String("ModuleDataOffset"), errorMessage);
	moduleDataSize = getSectionUintValue(QLatin1String("ModuleDataSize"), errorMessage);

	tuningDataOffset = getSectionUintValue(QLatin1String("TuningDataOffset"), errorMessage);
	tuningDataSize = getSectionUintValue(QLatin1String("TuningDataSize"), errorMessage);

	txDiagDataOffset = getSectionUintValue(QLatin1String("TxDiagDataOffset"), errorMessage);
	txDiagDataSize = getSectionUintValue(QLatin1String("TxDiagDataSize"), errorMessage);

	return errorMessage->isEmpty();
}

bool LogicModule::LogicUnit::load(const QDomDocument& document, QString* errorMessage)
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

	alpPhaseTime = getSectionUintValue(QLatin1String("ALPPhaseTime"), errorMessage);
	clockFrequency= getSectionUintValue(QLatin1String("ClockFrequency"), errorMessage);
	cycleDuration = getSectionUintValue(QLatin1String("CycleDuration"), errorMessage);
	idrPhaseTime = getSectionUintValue(QLatin1String("IDRPhaseTime"), errorMessage);

	return errorMessage->isEmpty();
}

bool LogicModule::OptoInterface::load(const QDomDocument& document, QString* errorMessage)
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

	optoPortCount = getSectionUintValue(QLatin1String("OptoPortCount"), errorMessage);
	optoPortAppDataOffset= getSectionUintValue(QLatin1String("OptoPortAppDataOffset"), errorMessage);
	optoPortAppDataSize = getSectionUintValue(QLatin1String("OptoPortAppDataSize"), errorMessage);
	optoInterfaceDataOffset = getSectionUintValue(QLatin1String("OptoInterfaceDataOffset"), errorMessage);
	optoPortDataSize = getSectionUintValue(QLatin1String("OptoPortDataSize"), errorMessage);

	return errorMessage->isEmpty();
}


int LogicModule::descriptionNumber() const
{
	return m_descriptionNumber;
}

const LogicModule::FlashMemory& LogicModule::flashMemory() const
{
	return m_flashMemory;
}

const LogicModule::Memory& LogicModule::memory() const
{
	return m_memory;
}

const LogicModule::LogicUnit& LogicModule::logicUnit() const
{
	return m_logicUnit;
}

const LogicModule::OptoInterface& LogicModule::optoInterface() const
{
	return m_optoInterface;
}

const std::vector<std::shared_ptr<Afb::AfbElement>>& LogicModule::afbs() const
{
	return m_afbs;
}

std::shared_ptr<Afb::AfbComponent> LogicModule::component(int opCode) const
{
	auto it = m_afbComponents.find(opCode);

	if (it == m_afbComponents.end())
	{
		return nullptr;
	}

	return it->second;
}

