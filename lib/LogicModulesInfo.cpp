#include "LogicModulesInfo.h"
#include "ConstStrings.h"

// -----------------------------------------------------------------------------------
//
// LogicModulesInfo implementation
//
// -----------------------------------------------------------------------------------

const QString LogicModulesInfo::ELEM_LOGIC_MODULES("LogicModules");
const QString LogicModulesInfo::ELEM_LOGIC_MODULE("LogicModule");

const QString LogicModulesInfo::ELEM_LAN_CONTROLLERS("LanControllers");
const QString LogicModulesInfo::ELEM_LAN_CONTROLLER("LanController");

const QString LogicModulesInfo::ELEM_TUNING_PARAMS("TuningParams");
const QString LogicModulesInfo::ELEM_APP_DATA_PARAMS("AppDataParams");
const QString LogicModulesInfo::ELEM_DIAG_DATA_PARAMS("DiagDataParams");

const QString LogicModulesInfo::ATTR_TUNING_PROVIDED("TuningProvided");
const QString LogicModulesInfo::ATTR_APP_DATA_PROVIDED("AppDataProvided");
const QString LogicModulesInfo::ATTR_DIAG_DATA_PROVIDED("DiagDataProvided");


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

	if (logicModulesElem.isNull() == true || logicModulesElem.tagName() != ELEM_LOGIC_MODULES)
	{
		*errMsg = DomXmlHelper::errElementNotFound(ELEM_LOGIC_MODULES);
		return false;
	}

	int lmsCount = 0;

	result = DomXmlHelper::getIntAttribute(logicModulesElem, XmlAttribute::COUNT, &lmsCount, errMsg);

	if (result == false)
	{
		return false;
	}

	QDomNodeList lmsNodes = logicModulesElem.elementsByTagName(ELEM_LOGIC_MODULE);

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

bool LogicModulesInfo::load(::LogicModuleInfo* lmi, const QDomNode& lmNode, QString* errMsg)
{
	if (lmi == nullptr)
	{
		Q_ASSERT(false);
		return false;
	}

	if (lmNode.isElement() == false || lmNode.nodeName() != LogicModulesInfo::ELEM_LOGIC_MODULE)
	{
		*errMsg = DomXmlHelper::errElementNotFound(LogicModulesInfo::ELEM_LOGIC_MODULE);
		return false;
	}

	QDomElement lmElem = lmNode.toElement();

	bool result = true;

	result &= DomXmlHelper::getStringAttribute(lmElem, EquipmentPropNames::EQUIPMENT_ID, &lmi->equipmentID, errMsg);
	result &= DomXmlHelper::getStringAttribute(lmElem, EquipmentPropNames::CAPTION, &lmi->caption, errMsg);

	result &= DomXmlHelper::getStringAttribute(lmElem, EquipmentPropNames::SUBSYSTEM_ID, &lmi->subsystemID, errMsg);
	result &= DomXmlHelper::getIntAttribute(lmElem, EquipmentPropNames::LM_NUMBER, &lmi->lmNumber, errMsg);
	result &= DomXmlHelper::getStringAttribute(lmElem, EquipmentPropNames::SUBSYSTEM_CHANNEL, &lmi->subsystemChannel, errMsg);

	result &= DomXmlHelper::getIntAttribute(lmElem, EquipmentPropNames::APP_DATA_SIZE_BYTES, &lmi->appDataSizeBytes, errMsg);
	result &= DomXmlHelper::getIntAttribute(lmElem, EquipmentPropNames::DIAG_DATA_SIZE_BYTES, &lmi->diagDataSizeBytes, errMsg);

	result &= DomXmlHelper::getStringAttribute(lmElem, EquipmentPropNames::MODULE_FAMILY, &lmi->moduleFamily, errMsg);
	result &= DomXmlHelper::getIntAttribute(lmElem, EquipmentPropNames::MODULE_VERSION, &lmi->moduleVersion, errMsg);

	result &= DomXmlHelper::getStringAttribute(lmElem, EquipmentPropNames::PRESET_NAME, &lmi->presetName, errMsg);
	result &= DomXmlHelper::getStringAttribute(lmElem, EquipmentPropNames::LM_DESCRIPTION_FILE, &lmi->lmDescriptionFile, errMsg);

	if (result == false)
	{
		return false;
	}

	QDomNodeList lanControllersNodes = lmElem.elementsByTagName(ELEM_LAN_CONTROLLERS);

	if (lanControllersNodes.count() != 1)
	{
		*errMsg = DomXmlHelper::errElementNotFound(ELEM_LAN_CONTROLLERS);
		return false;
	}

	QDomElement lanControllersElem = lanControllersNodes.at(0).toElement();

	if (lanControllersElem.isNull() == true)
	{
		*errMsg = DomXmlHelper::errElementNotFound(ELEM_LAN_CONTROLLERS);
		return false;
	}

	int lanControllersCount = 0;

	result &= DomXmlHelper::getIntAttribute(lanControllersElem, XmlAttribute::COUNT, &lanControllersCount, errMsg);

	QDomNodeList lanControllerNodes = lmElem.elementsByTagName(ELEM_LAN_CONTROLLER);

	if (lanControllerNodes.count() != lanControllersCount)
	{
		*errMsg = QString("File corruption! Count of LanController nodes is not equal to LanControllers Count attribute value");
		return false;
	}

	lmi->lanControllers.resize(lanControllersCount);

	for(int i = 0; i < lanControllersCount; i++)
	{
		QDomNode lanControllerNode = lanControllerNodes.at(i);

		result &= load(&lmi->lanControllers[i], lanControllerNode, errMsg);

		if (result == false)
		{
			break;
		}
	}

	return result;
}

bool LogicModulesInfo::load(LanControllerInfo* lci, const QDomNode& lanControllerNode, QString* errMsg)
{
	if (lci == nullptr)
	{
		Q_ASSERT(false);
		return false;
	}

	if (lanControllerNode.isElement() == false || lanControllerNode.nodeName() != LogicModulesInfo::ELEM_LAN_CONTROLLER)
	{
		*errMsg = DomXmlHelper::errElementNotFound(LogicModulesInfo::ELEM_LOGIC_MODULE);
		return false;
	}

	bool result = true;

	QDomElement lcElem = lanControllerNode.toElement();

	result &= DomXmlHelper::getStringAttribute(lcElem, EquipmentPropNames::EQUIPMENT_ID, &lci->equipmentID, errMsg);
	result &= DomXmlHelper::getIntAttribute(lcElem, EquipmentPropNames::CONTROLLER_NO, &lci->controllerNo, errMsg);

	QString lanControllerTypeStr;

	result &= DomXmlHelper::getStringAttribute(lcElem, EquipmentPropNames::LAN_CONTROLLER_TYPE, &lanControllerTypeStr, errMsg);

	bool ok = false;

	lci->lanControllerType = E::stringToValue<E::LanControllerType>(lanControllerTypeStr, &ok);

	if (ok == false)
	{
		*errMsg = "File corruption! Can't convert LanControllerType to E::LanControllerType value";
		return false;
	}

	result &= DomXmlHelper::getBoolAttribute(lcElem, ATTR_TUNING_PROVIDED, &lci->tuningProvided, errMsg);
	result &= DomXmlHelper::getBoolAttribute(lcElem, ATTR_APP_DATA_PROVIDED, &lci->appDataProvided, errMsg);
	result &= DomXmlHelper::getBoolAttribute(lcElem, ATTR_DIAG_DATA_PROVIDED, &lci->diagDataProvided, errMsg);

	if (result == false)
	{
		return false;
	}

	if (lci->tuningProvided == true)
	{
		QDomElement tuningElem;

		result &= DomXmlHelper::getSingleChildElement(lcElem, ELEM_TUNING_PARAMS, &tuningElem, errMsg);

		if (result == false)
		{
			return false;
		}

		result &= DomXmlHelper::getBoolAttribute(tuningElem, EquipmentPropNames::TUNING_ENABLE, &lci->tuningEnable, errMsg);
		result &= DomXmlHelper::getStringAttribute(tuningElem, EquipmentPropNames::TUNING_IP, &lci->tuningIP, errMsg);
		result &= DomXmlHelper::getIntAttribute(tuningElem, EquipmentPropNames::TUNING_PORT, &lci->tuningPort, errMsg);
		result &= DomXmlHelper::getStringAttribute(tuningElem, EquipmentPropNames::TUNING_SERVICE_ID, &lci->tuningServiceID, errMsg);
		result &= DomXmlHelper::getStringAttribute(tuningElem, EquipmentPropNames::TUNING_SERVICE_IP, &lci->tuningServiceIP, errMsg);
		result &= DomXmlHelper::getIntAttribute(tuningElem, EquipmentPropNames::TUNING_SERVICE_PORT, &lci->tuningServicePort, errMsg);
		result &= DomXmlHelper::getStringAttribute(tuningElem, EquipmentPropNames::TUNING_SERVICE_NETMASK, &lci->tuningServiceNetmask, errMsg);

		if (result == false)
		{
			return false;
		}
	}

	if (lci->appDataProvided == true)
	{
		QDomElement appDataElem;

		result &= DomXmlHelper::getSingleChildElement(lcElem, ELEM_APP_DATA_PARAMS, &appDataElem, errMsg);

		if (result == false)
		{
			return false;
		}

		result &= DomXmlHelper::getBoolAttribute(appDataElem, EquipmentPropNames::APP_DATA_ENABLE, &lci->appDataEnable, errMsg);
		result &= DomXmlHelper::getStringAttribute(appDataElem, EquipmentPropNames::APP_DATA_IP, &lci->appDataIP, errMsg);
		result &= DomXmlHelper::getIntAttribute(appDataElem, EquipmentPropNames::APP_DATA_PORT, &lci->appDataPort, errMsg);
		result &= DomXmlHelper::getStringAttribute(appDataElem, EquipmentPropNames::APP_DATA_SERVICE_ID, &lci->appDataServiceID, errMsg);
		result &= DomXmlHelper::getStringAttribute(appDataElem, EquipmentPropNames::APP_DATA_SERVICE_IP, &lci->appDataServiceIP, errMsg);
		result &= DomXmlHelper::getIntAttribute(appDataElem, EquipmentPropNames::APP_DATA_SERVICE_PORT, &lci->appDataServicePort, errMsg);
		result &= DomXmlHelper::getStringAttribute(appDataElem, EquipmentPropNames::APP_DATA_SERVICE_NETMASK, &lci->appDataServiceNetmask, errMsg);
		result &= DomXmlHelper::getIntAttribute(appDataElem, EquipmentPropNames::APP_DATA_SIZE_BYTES, &lci->appDataSizeBytes, errMsg);
		result &= DomXmlHelper::getUInt32Attribute(appDataElem, EquipmentPropNames::APP_DATA_UID, &lci->appDataUID, errMsg);
		result &= DomXmlHelper::getIntAttribute(appDataElem, EquipmentPropNames::APP_DATA_FRAMES_QUANTITY, &lci->appDataFramesQuantity, errMsg);
		result &= DomXmlHelper::getIntAttribute(appDataElem, EquipmentPropNames::OVERRIDE_APP_DATA_WORD_COUNT, &lci->overrideAppDataWordCount, errMsg);

		if (result == false)
		{
			return false;
		}
	}

	if (lci->diagDataProvided == true)
	{
		QDomElement diagDataElem;

		result &= DomXmlHelper::getSingleChildElement(lcElem, ELEM_DIAG_DATA_PARAMS, &diagDataElem, errMsg);

		if (result == false)
		{
			return false;
		}

		result &= DomXmlHelper::getBoolAttribute(diagDataElem, EquipmentPropNames::DIAG_DATA_ENABLE, &lci->diagDataEnable, errMsg);
		result &= DomXmlHelper::getStringAttribute(diagDataElem, EquipmentPropNames::DIAG_DATA_IP, &lci->diagDataIP, errMsg);
		result &= DomXmlHelper::getIntAttribute(diagDataElem, EquipmentPropNames::DIAG_DATA_PORT, &lci->diagDataPort, errMsg);
		result &= DomXmlHelper::getStringAttribute(diagDataElem, EquipmentPropNames::DIAG_DATA_SERVICE_ID, &lci->diagDataServiceID, errMsg);
		result &= DomXmlHelper::getStringAttribute(diagDataElem, EquipmentPropNames::DIAG_DATA_SERVICE_IP, &lci->diagDataServiceIP, errMsg);
		result &= DomXmlHelper::getIntAttribute(diagDataElem, EquipmentPropNames::DIAG_DATA_SERVICE_PORT, &lci->diagDataServicePort, errMsg);
		result &= DomXmlHelper::getStringAttribute(diagDataElem, EquipmentPropNames::DIAG_DATA_SERVICE_NETMASK, &lci->diagDataServiceNetmask, errMsg);
		result &= DomXmlHelper::getIntAttribute(diagDataElem, EquipmentPropNames::DIAG_DATA_SIZE_BYTES, &lci->diagDataSizeBytes, errMsg);
		result &= DomXmlHelper::getUInt32Attribute(diagDataElem, EquipmentPropNames::DIAG_DATA_UID, &lci->diagDataUID, errMsg);
		result &= DomXmlHelper::getIntAttribute(diagDataElem, EquipmentPropNames::DIAG_DATA_FRAMES_QUANTITY, &lci->diagDataFramesQuantity, errMsg);
		result &= DomXmlHelper::getIntAttribute(diagDataElem, EquipmentPropNames::OVERRIDE_DIAG_DATA_WORD_COUNT, &lci->overrideDiagDataWordCount, errMsg);

		if (result == false)
		{
			return false;
		}
	}

	return result;
}

#ifdef IS_BUILDER

	// -----------------------------------------------------------------------------------
	//
	// LogicModulesInfoWriter implementation
	//
	// -----------------------------------------------------------------------------------

	LogicModulesInfoWriter::LogicModulesInfoWriter(	const QVector<Builder::ModuleLogicCompiler *>& moduleCompilers,
													const Hardware::EquipmentSet& equipmentSet) :
		m_moduleCompilers(moduleCompilers),
		m_equipmentSet(equipmentSet)
	{
	}

	bool LogicModulesInfoWriter::fill()
	{
		bool result = true;

		int lmsCount = m_moduleCompilers.size();

		logicModulesInfo.resize(lmsCount);

		for(int i = 0; i < lmsCount; i++)
		{
			Builder::ModuleLogicCompiler* mc = m_moduleCompilers[i];

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

			xml.writeIntAttribute(XmlAttribute::COUNT, static_cast<int>(logicModulesInfo.size()));

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

		result &= DeviceHelper::getStrProperty(lmModule.get(), EquipmentPropNames::SUBSYSTEM_ID, &lmInfo->subsystemID, mc.log());
		result &= DeviceHelper::getIntProperty(lmModule.get(), EquipmentPropNames::LM_NUMBER, &lmInfo->lmNumber, mc.log());
		result &= DeviceHelper::getStrProperty(lmModule.get(), EquipmentPropNames::SUBSYSTEM_CHANNEL, &lmInfo->subsystemChannel, mc.log());

		result &= DeviceHelper::getStrProperty(lmModule.get(), EquipmentPropNames::MODULE_FAMILY, &lmInfo->moduleFamily, mc.log());
		result &= DeviceHelper::getIntProperty(lmModule.get(), EquipmentPropNames::MODULE_VERSION, &lmInfo->moduleVersion, mc.log());

		lmInfo->presetName = lmModule->presetName();
		result &= DeviceHelper::getStrProperty(lmModule.get(), EquipmentPropNames::LM_DESCRIPTION_FILE, &lmInfo->lmDescriptionFile, mc.log());

		lmInfo->lanControllers.clear();

		std::shared_ptr<LmDescription> lmDescription = mc.getLmDescription();

		TEST_PTR_RETURN_FALSE(lmDescription);

		int lanControllersCount = lmDescription->lan().lanControllerCount();

		lmInfo->lanControllers.resize(lanControllersCount);

		for(int i = 0; i < lanControllersCount; i++)
		{
			bool ok = false;

			LmDescription::LanController lc = lmDescription->lan().lanController(i, &ok);

			if (ok == false)
			{
				result = false;
				continue;
			}

			LanControllerInfo& lci = lmInfo->lanControllers[i];

			result &= LanControllerInfoHelper::getInfo(	*lmModule.get(), lc.m_place, lc.m_type, &lci,
														m_equipmentSet, mc.log());

			if (lci.appDataProvided == true && lci.appDataEnable == true)
			{
				if (lmInfo->appDataSizeBytes != 0)
				{
					Q_ASSERT(lmInfo->appDataSizeBytes == lci.appDataSizeBytes);
				}

				lmInfo->appDataSizeBytes = lci.appDataSizeBytes;
			}

			if (lci.diagDataProvided == true && lci.diagDataEnable == true)
			{
				if (lmInfo->diagDataSizeBytes != 0)
				{
					Q_ASSERT(lmInfo->diagDataSizeBytes == lci.diagDataSizeBytes);
				}

				lmInfo->diagDataSizeBytes = lci.diagDataSizeBytes;
			}
		}

		return result;
	}

	bool LogicModulesInfoWriter::save(const LogicModuleInfo& lmInfo, XmlWriteHelper& xml) const
	{
		xml.writeStartElement(ELEM_LOGIC_MODULE);

		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, lmInfo.equipmentID);
		xml.writeStringAttribute(EquipmentPropNames::CAPTION, lmInfo.caption);

		xml.writeStringAttribute(EquipmentPropNames::SUBSYSTEM_ID, lmInfo.subsystemID);
		xml.writeIntAttribute(EquipmentPropNames::LM_NUMBER, lmInfo.lmNumber);
		xml.writeStringAttribute(EquipmentPropNames::SUBSYSTEM_CHANNEL, lmInfo.subsystemChannel);

		xml.writeIntAttribute(EquipmentPropNames::APP_DATA_SIZE_BYTES, lmInfo.appDataSizeBytes);
		xml.writeIntAttribute(EquipmentPropNames::DIAG_DATA_SIZE_BYTES, lmInfo.diagDataSizeBytes);

		xml.writeStringAttribute(EquipmentPropNames::MODULE_FAMILY, lmInfo.moduleFamily);
		xml.writeIntAttribute(EquipmentPropNames::MODULE_VERSION, lmInfo.moduleVersion);

		xml.writeStringAttribute(EquipmentPropNames::PRESET_NAME, lmInfo.presetName);
		xml.writeStringAttribute(EquipmentPropNames::LM_DESCRIPTION_FILE, lmInfo.lmDescriptionFile);

		xml.writeStartElement(ELEM_LAN_CONTROLLERS);
		xml.writeIntAttribute(XmlAttribute::COUNT, static_cast<int>(lmInfo.lanControllers.size()));

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

		xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, lci.equipmentID);
		xml.writeIntAttribute(EquipmentPropNames::CONTROLLER_NO, lci.controllerNo);
		xml.writeStringAttribute(EquipmentPropNames::LAN_CONTROLLER_TYPE,
								 E::valueToString<E::LanControllerType>(lci.lanControllerType));
		xml.writeBoolAttribute(ATTR_TUNING_PROVIDED, lci.tuningProvided);
		xml.writeBoolAttribute(ATTR_APP_DATA_PROVIDED, lci.appDataProvided);
		xml.writeBoolAttribute(ATTR_DIAG_DATA_PROVIDED, lci.diagDataProvided);

		if (lci.tuningProvided == true)
		{
			xml.writeStartElement(ELEM_TUNING_PARAMS);

			xml.writeBoolAttribute(EquipmentPropNames::TUNING_ENABLE, lci.tuningEnable);
			xml.writeStringAttribute(EquipmentPropNames::TUNING_IP, lci.tuningIP);
			xml.writeIntAttribute(EquipmentPropNames::TUNING_PORT, lci.tuningPort);
			xml.writeStringAttribute(EquipmentPropNames::TUNING_SERVICE_ID, lci.tuningServiceID);
			xml.writeStringAttribute(EquipmentPropNames::TUNING_SERVICE_IP, lci.tuningServiceIP);
			xml.writeIntAttribute(EquipmentPropNames::TUNING_SERVICE_PORT, lci.tuningServicePort);
			xml.writeStringAttribute(EquipmentPropNames::TUNING_SERVICE_NETMASK, lci.tuningServiceNetmask);

			xml.writeEndElement();	//	/ELEM_TUNING_PARAMS
		}

		if (lci.appDataProvided == true)
		{
			xml.writeStartElement(ELEM_APP_DATA_PARAMS);

			xml.writeBoolAttribute(EquipmentPropNames::APP_DATA_ENABLE, lci.appDataEnable);
			xml.writeStringAttribute(EquipmentPropNames::APP_DATA_IP, lci.appDataIP);
			xml.writeIntAttribute(EquipmentPropNames::APP_DATA_PORT, lci.appDataPort);
			xml.writeStringAttribute(EquipmentPropNames::APP_DATA_SERVICE_ID, lci.appDataServiceID);
			xml.writeStringAttribute(EquipmentPropNames::APP_DATA_SERVICE_IP, lci.appDataServiceIP);
			xml.writeIntAttribute(EquipmentPropNames::APP_DATA_SERVICE_PORT, lci.appDataServicePort);
			xml.writeStringAttribute(EquipmentPropNames::APP_DATA_SERVICE_NETMASK, lci.appDataServiceNetmask);
			xml.writeIntAttribute(EquipmentPropNames::APP_DATA_SIZE_BYTES, lci.appDataSizeBytes);
			xml.writeUInt32Attribute(EquipmentPropNames::APP_DATA_UID, lci.appDataUID, false);
			xml.writeUInt32Attribute(EquipmentPropNames::HEX_APP_DATA_UID, lci.appDataUID, true);
			xml.writeIntAttribute(EquipmentPropNames::APP_DATA_FRAMES_QUANTITY, lci.appDataFramesQuantity);
			xml.writeIntAttribute(EquipmentPropNames::OVERRIDE_APP_DATA_WORD_COUNT, lci.overrideAppDataWordCount);

			xml.writeEndElement();	//	/ELEM_APP_DATA_PARAMS
		}

		if (lci.diagDataProvided == true)
		{
			xml.writeStartElement(ELEM_DIAG_DATA_PARAMS);

			xml.writeBoolAttribute(EquipmentPropNames::DIAG_DATA_ENABLE, lci.diagDataEnable);
			xml.writeStringAttribute(EquipmentPropNames::DIAG_DATA_IP, lci.diagDataIP);
			xml.writeIntAttribute(EquipmentPropNames::DIAG_DATA_PORT, lci.diagDataPort);
			xml.writeStringAttribute(EquipmentPropNames::DIAG_DATA_SERVICE_ID, lci.diagDataServiceID);
			xml.writeStringAttribute(EquipmentPropNames::DIAG_DATA_SERVICE_IP, lci.diagDataServiceIP);
			xml.writeIntAttribute(EquipmentPropNames::DIAG_DATA_SERVICE_PORT, lci.diagDataServicePort);
			xml.writeStringAttribute(EquipmentPropNames::DIAG_DATA_SERVICE_NETMASK, lci.diagDataServiceNetmask);
			xml.writeIntAttribute(EquipmentPropNames::DIAG_DATA_SIZE_BYTES, lci.diagDataSizeBytes);
			xml.writeUInt32Attribute(EquipmentPropNames::DIAG_DATA_UID, lci.diagDataUID, false);
			xml.writeUInt32Attribute(EquipmentPropNames::HEX_DIAG_DATA_UID, lci.diagDataUID, true);
			xml.writeIntAttribute(EquipmentPropNames::DIAG_DATA_FRAMES_QUANTITY, lci.diagDataFramesQuantity);
			xml.writeIntAttribute(EquipmentPropNames::OVERRIDE_DIAG_DATA_WORD_COUNT, lci.overrideDiagDataWordCount);

			xml.writeEndElement();	//	/ELEM_DIAG_DATA_PARAMS
		}

		xml.writeEndElement();	//	/ELEM_LAN_CONTROLLER

		return true;
	}
#endif


/*
void testLogicModulesInfoLoad()
{
	LogicModulesInfo lmi;

	QString err;

	bool res = lmi.load(QString("d:/temp/compiler_tests/build/common/LogicModules.xml"), &err);

	if (res == false)
	{
		qDebug() << err;
	}
	else
	{
		qDebug() << "OK";
	}
}*/
