#include "LogicModulesInfoWriter.h"
#include "../lib/ConstStrings.h"


// -----------------------------------------------------------------------------------
//
// LogicModulesInfoWriter implementation
//
// -----------------------------------------------------------------------------------

LogicModulesInfoWriter::LogicModulesInfoWriter(const Builder::Context& context) :
	m_context(context)
{
}

bool LogicModulesInfoWriter::fill()
{
	TEST_PTR_RETURN_FALSE(log());

	bool result = true;

	int lmsCount = static_cast<int>(m_context.m_lmModules.size());

	logicModulesInfo.resize(lmsCount);

	for(int i = 0; i < lmsCount; i++)
	{
		const Hardware::DeviceModule* lmModule = m_context.m_lmModules[i];

		TEST_PTR_CONTINUE(lmModule);

		result &= fill(lmModule, &logicModulesInfo[i]);
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
		xml.writeStartElement(XmlElement::LOGIC_MODULES);

		xml.writeIntAttribute(XmlAttribute::COUNT, static_cast<int>(logicModulesInfo.size()));

		for(const LogicModuleInfo& lmInfo : logicModulesInfo)
		{
			save(lmInfo, xml);
		}

		xml.writeEndElement();	//		/XmlElement::LOGIC_MODULES
	}

	xml.writeEndDocument();
}

bool LogicModulesInfoWriter::fill(const Hardware::DeviceModule* lmModule, LogicModuleInfo* lmInfo)
{
	TEST_PTR_RETURN_FALSE(lmInfo);

	bool result = true;

	lmInfo->equipmentID = lmModule->equipmentIdTemplate();
	lmInfo->caption = lmModule->caption();

	result &= DeviceHelper::getStrProperty(lmModule, EquipmentPropNames::SUBSYSTEM_ID, &lmInfo->subsystemID, log());

	TEST_PTR_RETURN_FALSE(m_context.m_subsystems);

	lmInfo->subsystemKey = m_context.m_subsystems->subsystemKey(lmInfo->subsystemID);

	auto p = m_context.m_lmsUniqueIDs.find(lmInfo->equipmentID);

	if (p != m_context.m_lmsUniqueIDs.end())
	{
		lmInfo->lmUniqueID = p->second;
	}
	else
	{
		Q_ASSERT(false);
		LOG_INTERNAL_ERROR_MSG(log(), QString("UniqueID isn't found fro LM %1").arg(lmInfo->equipmentID));
		return false;
	}

	result &= DeviceHelper::getIntProperty(lmModule, EquipmentPropNames::LM_NUMBER, &lmInfo->lmNumber, log());
	result &= DeviceHelper::getStrProperty(lmModule, EquipmentPropNames::SUBSYSTEM_CHANNEL, &lmInfo->subsystemChannel, log());

	result &= DeviceHelper::getStrProperty(lmModule, EquipmentPropNames::MODULE_FAMILY, &lmInfo->moduleFamily, log());
	lmInfo->moduleFamilyID = static_cast<int>(lmModule->moduleFamily());
	result &= DeviceHelper::getIntProperty(lmModule, EquipmentPropNames::MODULE_VERSION, &lmInfo->moduleVersion, log());

	lmInfo->presetName = lmModule->presetName();
	result &= DeviceHelper::getStrProperty(lmModule, EquipmentPropNames::LM_DESCRIPTION_FILE, &lmInfo->lmDescriptionFile, log());

	lmInfo->lanControllers.clear();

	std::shared_ptr<LmDescription> lmDescription = m_context.m_lmDescriptions->get(lmModule);

	TEST_PTR_RETURN_FALSE(lmDescription);

	int lanControllersCount = lmDescription->lan().lanControllerCount();

	lmInfo->lanControllers.resize(lanControllersCount);

	lmInfo->lanControllers.setRupVersion(lmDescription->lan().m_rupVersion);
	lmInfo->lanControllers.setFotipVersion(lmDescription->lan().m_fotipVersion);

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

		result &= LanControllerInfoHelper::getInfo(	*lmModule, lc.m_type, lc.m_place,
													m_context, false, &lci, log());

		if (lci.isAppDataEnabled() == true)
		{
			lmInfo->appDataEnable = true;

			if (lmInfo->appDataSizeBytes != 0)
			{
				Q_ASSERT(lmInfo->appDataSizeBytes == lci.appDataSizeBytes);
			}

			lmInfo->appDataSizeBytes = lci.appDataSizeBytes;

			if (lmInfo->appDataUID != 0)
			{
				Q_ASSERT(lmInfo->appDataUID == lci.appDataUID);
			}

			lmInfo->appDataUID = lci.appDataUID;
		}

		if (lci.isDiagDataEnabled() == true)
		{
			lmInfo->diagDataEnable = true;

			if (lmInfo->diagDataSizeBytes != 0)
			{
				Q_ASSERT(lmInfo->diagDataSizeBytes == lci.diagDataSizeBytes);
			}

			lmInfo->diagDataSizeBytes = lci.diagDataSizeBytes;

			if (lmInfo->diagDataUID != 0)
			{
				Q_ASSERT(lmInfo->diagDataUID == lci.diagDataUID);
			}

			lmInfo->diagDataUID = lci.diagDataUID;
		}
	}

	return result;
}

bool LogicModulesInfoWriter::save(const LogicModuleInfo& lmInfo, XmlWriteHelper& xml) const
{
	xml.writeStartElement(XmlElement::LOGIC_MODULE);

	xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, lmInfo.equipmentID);
	xml.writeStringAttribute(EquipmentPropNames::CAPTION, lmInfo.caption);

	xml.writeStringAttribute(EquipmentPropNames::SUBSYSTEM_ID, lmInfo.subsystemID);
	xml.writeIntAttribute(EquipmentPropNames::SUBSYSTEM_KEY, lmInfo.subsystemKey);
	xml.writeIntAttribute(EquipmentPropNames::LM_NUMBER, lmInfo.lmNumber);
	xml.writeStringAttribute(EquipmentPropNames::SUBSYSTEM_CHANNEL, lmInfo.subsystemChannel);

	xml.writeUInt64Attribute(EquipmentPropNames::LM_UNIQUE_ID, lmInfo.lmUniqueID, true);

	xml.writeBoolAttribute(EquipmentPropNames::APP_DATA_ENABLE, lmInfo.appDataEnable);
	xml.writeIntAttribute(EquipmentPropNames::APP_DATA_SIZE_BYTES, lmInfo.appDataSizeBytes);
	xml.writeUInt32Attribute(EquipmentPropNames::APP_DATA_UID, lmInfo.appDataUID, true);

	xml.writeBoolAttribute(EquipmentPropNames::DIAG_DATA_ENABLE, lmInfo.diagDataEnable);
	xml.writeIntAttribute(EquipmentPropNames::DIAG_DATA_SIZE_BYTES, lmInfo.diagDataSizeBytes);
	xml.writeUInt32Attribute(EquipmentPropNames::DIAG_DATA_UID, lmInfo.diagDataUID, true);

	xml.writeStringAttribute(EquipmentPropNames::MODULE_FAMILY, lmInfo.moduleFamily);
	xml.writeIntAttribute(EquipmentPropNames::MODULE_FAMILY_ID, lmInfo.moduleFamilyID, true);
	xml.writeIntAttribute(EquipmentPropNames::MODULE_VERSION, lmInfo.moduleVersion);

	xml.writeStringAttribute(EquipmentPropNames::PRESET_NAME, lmInfo.presetName);
	xml.writeStringAttribute(EquipmentPropNames::LM_DESCRIPTION_FILE, lmInfo.lmDescriptionFile);

	//

	lmInfo.lanControllers.writeToXml(xml);

	//

	xml.writeEndElement();	//	/XmlElement::LOGIC_MODULE

	return true;
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
