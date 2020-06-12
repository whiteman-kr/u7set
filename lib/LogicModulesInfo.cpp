#include "LogicModulesInfo.h"

// -----------------------------------------------------------------------------------
//
// LogicModulesInfo implementation
//
// -----------------------------------------------------------------------------------

const QString LogicModulesInfo::ELEM_LOGIC_MODULES("LogicModules");
const QString LogicModulesInfo::ELEM_LOGIC_MODULE("LogicModule");

const QString LogicModulesInfo::ELEM_LAN_CONTROLLERS("LanControllers");
const QString LogicModulesInfo::ELEM_LAN_CONTROLLER("LanController");

const QString LogicModulesInfo::ATTR_COUNT("Count");
const QString LogicModulesInfo::ATTR_ID("ID");

const QString LogicModulesInfo::ATTR_EQUIPMENT_ID("EquipmentID");
const QString LogicModulesInfo::ATTR_CAPTION("Caption");

const QString LogicModulesInfo::ATTR_SUBSYSTEM_ID("SubsystemID");
const QString LogicModulesInfo::ATTR_LM_NUMBER("LMNumber");
const QString LogicModulesInfo::ATTR_SUBSYSTEM_CHANNEL("SubsystemChannel");

const QString LogicModulesInfo::ATTR_MODULE_FAMILY("ModuleFamily");
const QString LogicModulesInfo::ATTR_MODULE_VERSION("ModuleVersion");

const QString LogicModulesInfo::ATTR_PRESET_NAME("PresetName");
const QString LogicModulesInfo::ATTR_LM_DESCRIPTION_FILE("LmDescriptionFile");

const QString LogicModulesInfo::ATTR_DATA_ID("DataID");
const QString LogicModulesInfo::ATTR_HEX_DATA_ID("HexDataID");


bool LogicModulesInfo::load(const QString& fileName, QString* errMsg)
{
	if (errMsg == nullptr)
	{
		Q_ASSERT(false);
		return false;
	}

	QFile file(fileName);

	if(file.open(QIODevice::ReadOnly) == false)
	{
		*errMsg = QString("File open error");
		return false;
	}

	QByteArray xmlData = file.readAll();

	if (xmlData.size() != QFileInfo(file).size())
	{
		*errMsg = QString("File read error");
		return false;
	}

	file.close();

	return load(xmlData, errMsg);
}

bool LogicModulesInfo::load(const QByteArray& xmlData, QString* errMsg)
{
	if (errMsg == nullptr)
	{
		Q_ASSERT(false);
		return false;
	}

	QDomDocument xmlDoc;

	QString parsingError;
	int errLine = 0;
	int errColumn = 0;

	bool result = xmlDoc.setContent(xmlData, false, &parsingError, &errLine, &errColumn);

	if (result == false)
	{
		*errMsg = QString("%1, line %2, column %3").arg(parsingError).arg(errLine).arg(errColumn);
		return false;
	}

	logicModulesInfo.clear();

	QDomElement logicModulesElem = xmlDoc.documentElement();

	if (logicModulesElem.isNull() == true || logicModulesElem.tagName() != LogicModulesInfo::ELEM_LOGIC_MODULES)
	{
		*errMsg = DomXmlHelper::errElementNotFound(LogicModulesInfo::ELEM_LOGIC_MODULES);
		return false;
	}

	int lmsCount = 0;

	result = DomXmlHelper::getIntAttribute(logicModulesElem, LogicModulesInfo::ATTR_COUNT, &lmsCount, errMsg);

	if (result == false)
	{
		return false;
	}

	QDomNodeList lmsNodes = logicModulesElem.elementsByTagName(LogicModulesInfo::ELEM_LOGIC_MODULE);

	if (lmsNodes.count() != lmsCount)
	{
		*errMsg = QString("File corruption! Count of LogicModule nodes is not equal to LogicModules Count attribute value");
		return false;
	}

	logicModulesInfo.resize(lmsCount);

	for(int i = 0; i < lmsCount; i++)
	{
		bool res = load(&logicModulesInfo[i], lmsNodes.item(i), errMsg);

		if (res == false)
		{
			return false;
		}
	}

	return true;
}

bool LogicModulesInfo::load(LogicModuleInfo* lmi, const QDomNode& node, QString* errMsg)
{
	if (lmi == nullptr)
	{
		Q_ASSERT(false);
		return false;
	}

	if (node.isElement() == false || node.nodeName() != LogicModulesInfo::ELEM_LOGIC_MODULE)
	{
		*errMsg = DomXmlHelper::errElementNotFound(LogicModulesInfo::ELEM_LOGIC_MODULE);
		return false;
	}

	QDomElement elem = node.toElement();

	bool result = true;

	result &= DomXmlHelper::getStringAttribute(elem, ATTR_EQUIPMENT_ID, &lmi->equipmentID, errMsg);
	result &= DomXmlHelper::getStringAttribute(elem, ATTR_CAPTION, &lmi->caption, errMsg);

	result &= DomXmlHelper::getStringAttribute(elem, ATTR_SUBSYSTEM_ID, &lmi->subsystemID, errMsg);
	result &= DomXmlHelper::getIntAttribute(elem, ATTR_LM_NUMBER, &lmi->lmNumber, errMsg);
	result &= DomXmlHelper::getStringAttribute(elem, ATTR_SUBSYSTEM_CHANNEL, &lmi->subsystemChannel, errMsg);

	result &= DomXmlHelper::getStringAttribute(elem, ATTR_MODULE_FAMILY, &lmi->moduleFamily, errMsg);
	result &= DomXmlHelper::getIntAttribute(elem, ATTR_MODULE_VERSION, &lmi->moduleVersion, errMsg);

	result &= DomXmlHelper::getStringAttribute(elem, ATTR_PRESET_NAME, &lmi->presetName, errMsg);
	result &= DomXmlHelper::getStringAttribute(elem, ATTR_LM_DESCRIPTION_FILE, &lmi->lmDescriptionFile, errMsg);

	return result;
}


#ifdef IS_BUILDER

	// -----------------------------------------------------------------------------------
	//
	// LogicModulesInfoWriter implementation
	//
	// -----------------------------------------------------------------------------------

	bool LogicModulesInfoWriter::fill(const QVector<Builder::ModuleLogicCompiler*>& moduleCompilers)
	{
		bool result = true;

		int lmsCount = moduleCompilers.size();

		logicModulesInfo.resize(lmsCount);

		for(int i = 0; i < lmsCount; i++)
		{
			Builder::ModuleLogicCompiler* mc = moduleCompilers[i];

			TEST_PTR_CONTINUE(mc);

			result &= fill(&logicModulesInfo[i], *mc);
		}

		return result;
	}

	void LogicModulesInfoWriter::save(QByteArray* xmlFileData) const
	{
		TEST_PTR_RETURN(xmlFileData);

		XmlWriteHelper xml(xmlFileData);

		xml.setAutoFormatting(true);
		xml.writeStartDocument();

		{
			xml.writeStartElement(LogicModulesInfo::ELEM_LOGIC_MODULES);

			xml.writeIntAttribute(LogicModulesInfo::ATTR_COUNT, static_cast<int>(logicModulesInfo.size()));

			for(const LogicModuleInfo& lmInfo : logicModulesInfo)
			{
				save(lmInfo, xml);
			}

			xml.writeEndElement();
		}

		xml.writeEndDocument();
	}


	bool LogicModulesInfoWriter::fill(LogicModuleInfo* lmInfo, Builder::ModuleLogicCompiler& mc)
	{
		TEST_PTR_RETURN_FALSE(lmInfo);

		bool result = true;

		std::shared_ptr<Hardware::DeviceModule> lmModule = mc.getLmSharedPtr();

		TEST_PTR_RETURN_FALSE(lmModule);

		lmInfo->equipmentID = lmModule->equipmentIdTemplate();
		lmInfo->caption = lmModule->caption();

		result &= DeviceHelper::getStrProperty(lmModule.get(), ATTR_SUBSYSTEM_ID, &lmInfo->subsystemID, mc.log());
		result &= DeviceHelper::getIntProperty(lmModule.get(), ATTR_LM_NUMBER, &lmInfo->lmNumber, mc.log());
		result &= DeviceHelper::getStrProperty(lmModule.get(), ATTR_SUBSYSTEM_CHANNEL, &lmInfo->subsystemChannel, mc.log());

		result &= DeviceHelper::getStrProperty(lmModule.get(), ATTR_MODULE_FAMILY, &lmInfo->moduleFamily, mc.log());
		result &= DeviceHelper::getIntProperty(lmModule.get(), ATTR_MODULE_VERSION, &lmInfo->moduleVersion, mc.log());

		lmInfo->presetName = lmModule->presetName();
		result &= DeviceHelper::getStrProperty(lmModule.get(), ATTR_LM_DESCRIPTION_FILE, &lmInfo->lmDescriptionFile, mc.log());

		return result;
	}

	bool LogicModulesInfoWriter::save(const LogicModuleInfo& lmInfo, XmlWriteHelper& xml) const
	{
		xml.writeStartElement(ELEM_LOGIC_MODULE);

		xml.writeStringAttribute(ATTR_EQUIPMENT_ID, lmInfo.equipmentID);
		xml.writeStringAttribute(ATTR_CAPTION, lmInfo.caption);

		xml.writeStringAttribute(ATTR_SUBSYSTEM_ID, lmInfo.subsystemID);
		xml.writeIntAttribute(ATTR_LM_NUMBER, lmInfo.lmNumber);
		xml.writeStringAttribute(ATTR_SUBSYSTEM_CHANNEL, lmInfo.subsystemChannel);

		xml.writeStringAttribute(ATTR_MODULE_FAMILY, lmInfo.moduleFamily);
		xml.writeIntAttribute(ATTR_MODULE_VERSION, lmInfo.moduleVersion);

		xml.writeStringAttribute(ATTR_PRESET_NAME, lmInfo.presetName);
		xml.writeStringAttribute(ATTR_LM_DESCRIPTION_FILE, lmInfo.lmDescriptionFile);

		xml.writeStartElement(ELEM_LAN_CONTROLLERS);
		xml.writeIntAttribute(ATTR_COUNT, static_cast<int>(lmInfo.lanControllers.size()));

		for(const LanControllerInfo& lci : lmInfo.lanControllers)
		{
			save(lci, xml);
		}

		xml.writeEndElement();	//		/ELEM_LAN_CONTROLLERS

		xml.writeEndElement();	//	/ELEM_LOGIC_MODULE

		return true;
	}

	bool LogicModulesInfoWriter::save(const LanControllerInfo& lci, XmlWriteHelper& xml) const
	{
		xml.writeStartElement(ELEM_LAN_CONTROLLER);

		xml.writeEndElement();	//		/ELEM_LAN_CONTROLLER

		return true;
	}

#endif


/*
void testConnInfoLoad()
{
	ConnectionsInfo ci;

	QString err;

	bool res = ci.load(QString("d:/temp/connections-debug/build/common/connections.xml"), &err);

	if (res == false)
	{
		qDebug() << err;
	}
	else
	{
		qDebug() << "OK";
	}
}*/
