#include "LogicModulesInfo.h"
#include "../lib/ConstStrings.h"
#include "../UtilsLib/DomXmlHelper.h"


// -----------------------------------------------------------------------------------
//
// LogicModulesInfo implementation
//
// -----------------------------------------------------------------------------------

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

	if (logicModulesElem.isNull() == true || logicModulesElem.tagName() != XmlElement::LOGIC_MODULES)
	{
		*errMsg = DomXmlHelper::errElementNotFound(XmlElement::LOGIC_MODULES);
		return false;
	}

	int lmsCount = 0;

	result = DomXmlHelper::getIntAttribute(logicModulesElem, XmlAttribute::COUNT, &lmsCount, errMsg);

	if (result == false)
	{
		return false;
	}

	QDomNodeList lmsNodes = logicModulesElem.elementsByTagName(XmlElement::LOGIC_MODULE);

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

std::optional<::LogicModuleInfo> LogicModulesInfo::get(QString equipmentId) const
{
	std::optional<::LogicModuleInfo> result;

	auto fit = std::find_if(logicModulesInfo.begin(),
							logicModulesInfo.end(),
							[&equipmentId](const ::LogicModuleInfo& lmi)
							{
								return lmi.equipmentID == equipmentId;
							});

	if (fit != logicModulesInfo.end())
	{
		result = *fit;
	}

	return result;
}

bool LogicModulesInfo::load(::LogicModuleInfo* lmi, const QDomNode& lmNode, QString* errMsg)
{
	if (lmi == nullptr)
	{
		Q_ASSERT(false);
		return false;
	}

	if (lmNode.isElement() == false || lmNode.nodeName() != XmlElement::LOGIC_MODULE)
	{
		*errMsg = DomXmlHelper::errElementNotFound(XmlElement::LOGIC_MODULE);
		return false;
	}

	QDomElement lmElem = lmNode.toElement();

	bool result = true;

	result &= DomXmlHelper::getStringAttribute(lmElem, EquipmentPropNames::EQUIPMENT_ID, &lmi->equipmentID, errMsg);
	result &= DomXmlHelper::getStringAttribute(lmElem, EquipmentPropNames::CAPTION, &lmi->caption, errMsg);

	result &= DomXmlHelper::getStringAttribute(lmElem, EquipmentPropNames::SUBSYSTEM_ID, &lmi->subsystemID, errMsg);
	result &= DomXmlHelper::getIntAttribute(lmElem, EquipmentPropNames::SUBSYSTEM_KEY, &lmi->subsystemKey, errMsg);
	result &= DomXmlHelper::getIntAttribute(lmElem, EquipmentPropNames::LM_NUMBER, &lmi->lmNumber, errMsg);
	result &= DomXmlHelper::getStringAttribute(lmElem, EquipmentPropNames::SUBSYSTEM_CHANNEL, &lmi->subsystemChannel, errMsg);

	QString uniqueIdStr;

	result &= DomXmlHelper::getStringAttribute(lmElem, EquipmentPropNames::LM_UNIQUE_ID, &uniqueIdStr, errMsg);

	bool ok = false;

	lmi->lmUniqueID = uniqueIdStr.toULongLong(&ok, 16);

	if (ok == false)
	{
		*errMsg = DomXmlHelper::errAttributeParsing(lmElem, EquipmentPropNames::LM_UNIQUE_ID);
		return false;
	}

	result &= DomXmlHelper::getBoolAttribute(lmElem, EquipmentPropNames::APP_DATA_ENABLE, &lmi->appDataEnable, errMsg);
	result &= DomXmlHelper::getIntAttribute(lmElem, EquipmentPropNames::APP_DATA_SIZE_BYTES, &lmi->appDataSizeBytes, errMsg);
	result &= DomXmlHelper::getUInt32Attribute(lmElem, EquipmentPropNames::RUP_APP_DATA_UID, &lmi->rupAppDataUID, errMsg);

	result &= DomXmlHelper::getBoolAttribute(lmElem, EquipmentPropNames::DIAG_DATA_ENABLE, &lmi->diagDataEnable, errMsg);
	result &= DomXmlHelper::getIntAttribute(lmElem, EquipmentPropNames::DIAG_DATA_SIZE_BYTES, &lmi->diagDataSizeBytes, errMsg);
	result &= DomXmlHelper::getUInt32Attribute(lmElem, EquipmentPropNames::RUP_DIAG_DATA_UID, &lmi->rupDiagDataUID, errMsg);

	result &= DomXmlHelper::getStringAttribute(lmElem, EquipmentPropNames::MODULE_FAMILY, &lmi->moduleFamily, errMsg);

	QString strModuleFamilyID;

	result &= DomXmlHelper::getStringAttribute(lmElem, EquipmentPropNames::MODULE_FAMILY_ID, &strModuleFamilyID, errMsg);

	ok = false;

	lmi->moduleFamilyID = strModuleFamilyID.toInt(&ok, 0);

	if (ok == false)
	{
		*errMsg = DomXmlHelper::errAttributeParsing(lmElem, EquipmentPropNames::MODULE_FAMILY_ID);
		return false;
	}

	result &= DomXmlHelper::getIntAttribute(lmElem, EquipmentPropNames::MODULE_VERSION, &lmi->moduleVersion, errMsg);

	result &= DomXmlHelper::getStringAttribute(lmElem, EquipmentPropNames::PRESET_NAME, &lmi->presetName, errMsg);
	result &= DomXmlHelper::getStringAttribute(lmElem, EquipmentPropNames::LM_DESCRIPTION_FILE, &lmi->lmDescriptionFile, errMsg);

	if (result == false)
	{
		return false;
	}

	QDomNodeList lanControllersNodes = lmElem.elementsByTagName(XmlElement::LAN_CONTROLLERS);

	if (lanControllersNodes.count() != 1)
	{
		*errMsg = DomXmlHelper::errElementNotFound(XmlElement::LAN_CONTROLLERS);
		return false;
	}

	QDomElement lanControllersElem = lanControllersNodes.at(0).toElement();

	if (lanControllersElem.isNull() == true)
	{
		*errMsg = DomXmlHelper::errElementNotFound(XmlElement::LAN_CONTROLLERS);
		return false;
	}

	lmi->lanControllers.readFromXml(lanControllersNodes.at(0), errMsg);

	return result;
}

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
