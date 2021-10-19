#include "LanControllerInfo.h"

bool LanControllerInfo::isProvideTuning(E::LanControllerType lanControllerType)
{
	return (static_cast<int>(lanControllerType) & static_cast<int>(E::LanControllerType::Tuning)) != 0;
}

bool LanControllerInfo::isProvideAppData(E::LanControllerType lanControllerType)
{
	return (static_cast<int>(lanControllerType) & static_cast<int>(E::LanControllerType::AppData)) != 0;
}

bool LanControllerInfo::isProvideDiagData(E::LanControllerType lanControllerType)
{
	return (static_cast<int>(lanControllerType) & static_cast<int>(E::LanControllerType::DiagData)) != 0;
}

bool LanControllerInfo::isProvideTuning() const
{
	return isProvideTuning(lanControllerType);
}

bool LanControllerInfo::isProvideAppData() const
{
	return isProvideAppData(lanControllerType);
}

bool LanControllerInfo::isProvideDiagData() const
{
	return isProvideDiagData(lanControllerType);
}

bool LanControllerInfo::writeToXml(XmlWriteHelper& xml) const
{
	xml.writeStartElement(XmlElement::LAN_CONTROLLER);

	xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, equipmentID);
	xml.writeIntAttribute(EquipmentPropNames::CONTROLLER_NO, controllerNo);
	xml.writeStringAttribute(EquipmentPropNames::LAN_CONTROLLER_TYPE,
							 E::valueToString<E::LanControllerType>(lanControllerType));

	//

	xml.writeStartElement(XmlElement::TUNING_PARAMS);

	if (isProvideTuning() == true)
	{
		xml.writeBoolAttribute(EquipmentPropNames::TUNING_ENABLE, tuningEnable);
		xml.writeStringAttribute(EquipmentPropNames::TUNING_IP, tuningIP);
		xml.writeIntAttribute(EquipmentPropNames::TUNING_PORT, tuningPort);
		xml.writeStringAttribute(EquipmentPropNames::TUNING_SERVICE_ID, tuningServiceID);
		xml.writeStringAttribute(EquipmentPropNames::TUNING_SERVICE_IP, tuningServiceIP);
		xml.writeIntAttribute(EquipmentPropNames::TUNING_SERVICE_PORT, tuningServicePort);
		xml.writeStringAttribute(EquipmentPropNames::TUNING_SERVICE_NETMASK, tuningServiceNetmask);
		xml.writeUInt64Attribute(EquipmentPropNames::TUNING_DATA_UID, tuningDataUID, true);

		xml.writeStringElement(XmlElement::TUNING_ASSOCIATED_SIGNALS, tuningAssociatedSignals.join(Separator::COMMA));
	}

	xml.writeEndElement();	//	/XmlElement::TUNING_PARAMS

	//

	xml.writeStartElement(XmlElement::APP_DATA_PARAMS);

	if (isProvideAppData() == true)
	{
		xml.writeBoolAttribute(EquipmentPropNames::APP_DATA_ENABLE, appDataEnable);
		xml.writeStringAttribute(EquipmentPropNames::APP_DATA_IP, appDataIP);
		xml.writeIntAttribute(EquipmentPropNames::APP_DATA_PORT, appDataPort);
		xml.writeStringAttribute(EquipmentPropNames::APP_DATA_SERVICE_ID, appDataServiceID);
		xml.writeStringAttribute(EquipmentPropNames::APP_DATA_SERVICE_IP, appDataServiceIP);
		xml.writeIntAttribute(EquipmentPropNames::APP_DATA_SERVICE_PORT, appDataServicePort);
		xml.writeStringAttribute(EquipmentPropNames::APP_DATA_SERVICE_NETMASK, appDataServiceNetmask);
		xml.writeIntAttribute(EquipmentPropNames::APP_DATA_SIZE_BYTES, appDataSizeBytes);
		xml.writeUInt32Attribute(EquipmentPropNames::APP_DATA_UID, appDataUID, false);
		xml.writeUInt32Attribute(EquipmentPropNames::HEX_APP_DATA_UID, appDataUID, true);
		xml.writeIntAttribute(EquipmentPropNames::APP_DATA_FRAMES_QUANTITY, appDataFramesQuantity);
		xml.writeIntAttribute(EquipmentPropNames::OVERRIDE_APP_DATA_WORD_COUNT, overrideAppDataWordCount);

		xml.writeStringElement(XmlElement::APP_DATA_ASSOCIATED_SIGNALS, appDataAssociatedSignals.join(Separator::COMMA));
	}

	xml.writeEndElement();	//	/XmlElement::APP_DATA_PARAMS

	//

	xml.writeStartElement(XmlElement::DIAG_DATA_PARAMS);

	if (isProvideDiagData() == true)
	{
		xml.writeBoolAttribute(EquipmentPropNames::DIAG_DATA_ENABLE, diagDataEnable);
		xml.writeStringAttribute(EquipmentPropNames::DIAG_DATA_IP, diagDataIP);
		xml.writeIntAttribute(EquipmentPropNames::DIAG_DATA_PORT, diagDataPort);
		xml.writeStringAttribute(EquipmentPropNames::DIAG_DATA_SERVICE_ID, diagDataServiceID);
		xml.writeStringAttribute(EquipmentPropNames::DIAG_DATA_SERVICE_IP, diagDataServiceIP);
		xml.writeIntAttribute(EquipmentPropNames::DIAG_DATA_SERVICE_PORT, diagDataServicePort);
		xml.writeStringAttribute(EquipmentPropNames::DIAG_DATA_SERVICE_NETMASK, diagDataServiceNetmask);
		xml.writeIntAttribute(EquipmentPropNames::DIAG_DATA_SIZE_BYTES, diagDataSizeBytes);
		xml.writeUInt32Attribute(EquipmentPropNames::DIAG_DATA_UID, diagDataUID, false);
		xml.writeUInt32Attribute(EquipmentPropNames::HEX_DIAG_DATA_UID, diagDataUID, true);
		xml.writeIntAttribute(EquipmentPropNames::DIAG_DATA_FRAMES_QUANTITY, diagDataFramesQuantity);
		xml.writeIntAttribute(EquipmentPropNames::OVERRIDE_DIAG_DATA_WORD_COUNT, overrideDiagDataWordCount);

		xml.writeStringElement(XmlElement::DIAG_DATA_ASSOCIATED_SIGNALS, diagDataAssociatedSignals.join(Separator::COMMA));
	}

	xml.writeEndElement();	//	/XmlElement::DIAG_DATA_PARAMS

	//

	xml.writeEndElement();	//	/XmlElement::LAN_CONTROLLER

	return true;
}

bool LanControllerInfo::readFromXml(XmlReadHelper& xml)
{
	if (xml.name() != XmlElement::LAN_CONTROLLER)
	{
		return false;
	}

	bool result = true;

	result &= xml.readStringAttribute(EquipmentPropNames::EQUIPMENT_ID, &equipmentID);
	result &= xml.readIntAttribute(EquipmentPropNames::CONTROLLER_NO, &controllerNo);

	result &= xml.readEnumAttribute(EquipmentPropNames::LAN_CONTROLLER_TYPE, &lanControllerType);

	//

	if (xml.findElement(XmlElement::TUNING_PARAMS) == false)
	{
		return false;
	}

	if (isProvideTuning() == true)
	{
		result &= xml.readBoolAttribute(EquipmentPropNames::TUNING_ENABLE, &tuningEnable);
		result &= xml.readStringAttribute(EquipmentPropNames::TUNING_IP, &tuningIP);
		result &= xml.readIntAttribute(EquipmentPropNames::TUNING_PORT, &tuningPort);
		result &= xml.readStringAttribute(EquipmentPropNames::TUNING_SERVICE_ID, &tuningServiceID);
		result &= xml.readStringAttribute(EquipmentPropNames::TUNING_SERVICE_IP, &tuningServiceIP);
		result &= xml.readIntAttribute(EquipmentPropNames::TUNING_SERVICE_PORT, &tuningServicePort);
		result &= xml.readStringAttribute(EquipmentPropNames::TUNING_SERVICE_NETMASK, &tuningServiceNetmask);
		result &= xml.readUInt64Attribute(EquipmentPropNames::TUNING_DATA_UID, &tuningDataUID);

		QString tuningSignals;

		result &= xml.readStringElement(XmlElement::TUNING_ASSOCIATED_SIGNALS, &tuningSignals);

		tuningAssociatedSignals = tuningSignals.split(Separator::COMMA, Qt::SkipEmptyParts);

		RETURN_IF_FALSE(result);
	}

	//

	if (xml.findElement(XmlElement::APP_DATA_PARAMS) == false)
	{
		return false;
	}

	if (isProvideAppData() == true)
	{
		result &= xml.readBoolAttribute(EquipmentPropNames::APP_DATA_ENABLE, &appDataEnable);
		result &= xml.readStringAttribute(EquipmentPropNames::APP_DATA_IP, &appDataIP);
		result &= xml.readIntAttribute(EquipmentPropNames::APP_DATA_PORT, &appDataPort);
		result &= xml.readStringAttribute(EquipmentPropNames::APP_DATA_SERVICE_ID, &appDataServiceID);
		result &= xml.readStringAttribute(EquipmentPropNames::APP_DATA_SERVICE_IP, &appDataServiceIP);
		result &= xml.readIntAttribute(EquipmentPropNames::APP_DATA_SERVICE_PORT, &appDataServicePort);
		result &= xml.readStringAttribute(EquipmentPropNames::APP_DATA_SERVICE_NETMASK, &appDataServiceNetmask);
		result &= xml.readIntAttribute(EquipmentPropNames::APP_DATA_SIZE_BYTES, &appDataSizeBytes);
		result &= xml.readUInt32Attribute(EquipmentPropNames::APP_DATA_UID, &appDataUID);
		result &= xml.readIntAttribute(EquipmentPropNames::APP_DATA_FRAMES_QUANTITY, &appDataFramesQuantity);
		result &= xml.readIntAttribute(EquipmentPropNames::OVERRIDE_APP_DATA_WORD_COUNT, &overrideAppDataWordCount);

		QString appSignals;

		result &= xml.readStringElement(XmlElement::APP_DATA_ASSOCIATED_SIGNALS, &appSignals);

		appDataAssociatedSignals = appSignals.split(Separator::COMMA, Qt::SkipEmptyParts);

		RETURN_IF_FALSE(result);
	}

	//

	if (xml.findElement(XmlElement::DIAG_DATA_PARAMS) == false)
	{
		return false;
	}

	if (isProvideDiagData() == true)
	{
		result &= xml.readBoolAttribute(EquipmentPropNames::DIAG_DATA_ENABLE, &diagDataEnable);
		result &= xml.readStringAttribute(EquipmentPropNames::DIAG_DATA_IP, &diagDataIP);
		result &= xml.readIntAttribute(EquipmentPropNames::DIAG_DATA_PORT, &diagDataPort);
		result &= xml.readStringAttribute(EquipmentPropNames::DIAG_DATA_SERVICE_ID, &diagDataServiceID);
		result &= xml.readStringAttribute(EquipmentPropNames::DIAG_DATA_SERVICE_IP, &diagDataServiceIP);
		result &= xml.readIntAttribute(EquipmentPropNames::DIAG_DATA_SERVICE_PORT, &diagDataServicePort);
		result &= xml.readStringAttribute(EquipmentPropNames::DIAG_DATA_SERVICE_NETMASK, &diagDataServiceNetmask);
		result &= xml.readIntAttribute(EquipmentPropNames::DIAG_DATA_SIZE_BYTES, &diagDataSizeBytes);
		result &= xml.readUInt32Attribute(EquipmentPropNames::DIAG_DATA_UID, &diagDataUID);
		result &= xml.readIntAttribute(EquipmentPropNames::DIAG_DATA_FRAMES_QUANTITY, &diagDataFramesQuantity);
		result &= xml.readIntAttribute(EquipmentPropNames::OVERRIDE_DIAG_DATA_WORD_COUNT, &overrideDiagDataWordCount);

		QString diagSignals;

		result &= xml.readStringElement(XmlElement::DIAG_DATA_ASSOCIATED_SIGNALS, &diagSignals);

		diagDataAssociatedSignals = diagSignals.split(Separator::COMMA, Qt::SkipEmptyParts);
	}

	return result;
}


bool LanControllerInfo::readFromXml(const QDomNode& lanControllerNode, QString* errMsg)
{
	TEST_PTR_RETURN_FALSE(errMsg);

	if (lanControllerNode.isElement() == false || lanControllerNode.nodeName() != XmlElement::LAN_CONTROLLER)
	{
		*errMsg = DomXmlHelper::errElementNotFound(XmlElement::LOGIC_MODULE);
		return false;
	}

	bool result = true;

	QDomElement lcElem = lanControllerNode.toElement();

	result &= DomXmlHelper::getStringAttribute(lcElem, EquipmentPropNames::EQUIPMENT_ID, &equipmentID, errMsg);
	result &= DomXmlHelper::getIntAttribute(lcElem, EquipmentPropNames::CONTROLLER_NO, &controllerNo, errMsg);

	QString lanControllerTypeStr;

	result &= DomXmlHelper::getStringAttribute(lcElem, EquipmentPropNames::LAN_CONTROLLER_TYPE, &lanControllerTypeStr, errMsg);

	bool ok = false;

	lanControllerType = E::stringToValue<E::LanControllerType>(lanControllerTypeStr, &ok);

	if (ok == false)
	{
		*errMsg = "File corruption! Can't convert LanControllerType to E::LanControllerType value";
		return false;
	}

	RETURN_IF_FALSE(result);

	//

	QDomElement tuningElem;

	result &= DomXmlHelper::getSingleChildElement(lcElem, XmlElement::TUNING_PARAMS, &tuningElem, errMsg);

	RETURN_IF_FALSE(result);

	if (isProvideTuning() == true)
	{
		result &= DomXmlHelper::getBoolAttribute(tuningElem, EquipmentPropNames::TUNING_ENABLE, &tuningEnable, errMsg);
		result &= DomXmlHelper::getStringAttribute(tuningElem, EquipmentPropNames::TUNING_IP, &tuningIP, errMsg);
		result &= DomXmlHelper::getIntAttribute(tuningElem, EquipmentPropNames::TUNING_PORT, &tuningPort, errMsg);
		result &= DomXmlHelper::getStringAttribute(tuningElem, EquipmentPropNames::TUNING_SERVICE_ID, &tuningServiceID, errMsg);
		result &= DomXmlHelper::getStringAttribute(tuningElem, EquipmentPropNames::TUNING_SERVICE_IP, &tuningServiceIP, errMsg);
		result &= DomXmlHelper::getIntAttribute(tuningElem, EquipmentPropNames::TUNING_SERVICE_PORT, &tuningServicePort, errMsg);
		result &= DomXmlHelper::getStringAttribute(tuningElem, EquipmentPropNames::TUNING_SERVICE_NETMASK, &tuningServiceNetmask, errMsg);
		result &= DomXmlHelper::getUInt64Attribute(tuningElem, EquipmentPropNames::TUNING_DATA_UID, &tuningDataUID, errMsg);

		QDomElement tuningSignals;

		result &= DomXmlHelper::getSingleChildElement(tuningElem,
													  XmlElement::TUNING_ASSOCIATED_SIGNALS,
													  &tuningSignals,
													  errMsg);

		tuningAssociatedSignals = tuningSignals.text().split(Separator::COMMA, Qt::SkipEmptyParts);

		RETURN_IF_FALSE(result);
	}

	//

	QDomElement appDataElem;

	result &= DomXmlHelper::getSingleChildElement(lcElem, XmlElement::APP_DATA_PARAMS, &appDataElem, errMsg);

	RETURN_IF_FALSE(result);

	if (isProvideAppData() == true)
	{
		result &= DomXmlHelper::getBoolAttribute(appDataElem, EquipmentPropNames::APP_DATA_ENABLE, &appDataEnable, errMsg);
		result &= DomXmlHelper::getStringAttribute(appDataElem, EquipmentPropNames::APP_DATA_IP, &appDataIP, errMsg);
		result &= DomXmlHelper::getIntAttribute(appDataElem, EquipmentPropNames::APP_DATA_PORT, &appDataPort, errMsg);
		result &= DomXmlHelper::getStringAttribute(appDataElem, EquipmentPropNames::APP_DATA_SERVICE_ID, &appDataServiceID, errMsg);
		result &= DomXmlHelper::getStringAttribute(appDataElem, EquipmentPropNames::APP_DATA_SERVICE_IP, &appDataServiceIP, errMsg);
		result &= DomXmlHelper::getIntAttribute(appDataElem, EquipmentPropNames::APP_DATA_SERVICE_PORT, &appDataServicePort, errMsg);
		result &= DomXmlHelper::getStringAttribute(appDataElem, EquipmentPropNames::APP_DATA_SERVICE_NETMASK, &appDataServiceNetmask, errMsg);
		result &= DomXmlHelper::getIntAttribute(appDataElem, EquipmentPropNames::APP_DATA_SIZE_BYTES, &appDataSizeBytes, errMsg);
		result &= DomXmlHelper::getUInt32Attribute(appDataElem, EquipmentPropNames::APP_DATA_UID, &appDataUID, errMsg);
		result &= DomXmlHelper::getIntAttribute(appDataElem, EquipmentPropNames::APP_DATA_FRAMES_QUANTITY, &appDataFramesQuantity, errMsg);
		result &= DomXmlHelper::getIntAttribute(appDataElem, EquipmentPropNames::OVERRIDE_APP_DATA_WORD_COUNT, &overrideAppDataWordCount, errMsg);

		QDomElement appSignals;

		result &= DomXmlHelper::getSingleChildElement(appDataElem,
													  XmlElement::APP_DATA_ASSOCIATED_SIGNALS,
													  &appSignals,
													  errMsg);

		appDataAssociatedSignals = appSignals.text().split(Separator::COMMA, Qt::SkipEmptyParts);

		RETURN_IF_FALSE(result);
	}

	//

	QDomElement diagDataElem;

	result &= DomXmlHelper::getSingleChildElement(lcElem, XmlElement::DIAG_DATA_PARAMS, &diagDataElem, errMsg);

	RETURN_IF_FALSE(result);

	if (isProvideDiagData() == true)
	{
		result &= DomXmlHelper::getBoolAttribute(diagDataElem, EquipmentPropNames::DIAG_DATA_ENABLE, &diagDataEnable, errMsg);
		result &= DomXmlHelper::getStringAttribute(diagDataElem, EquipmentPropNames::DIAG_DATA_IP, &diagDataIP, errMsg);
		result &= DomXmlHelper::getIntAttribute(diagDataElem, EquipmentPropNames::DIAG_DATA_PORT, &diagDataPort, errMsg);
		result &= DomXmlHelper::getStringAttribute(diagDataElem, EquipmentPropNames::DIAG_DATA_SERVICE_ID, &diagDataServiceID, errMsg);
		result &= DomXmlHelper::getStringAttribute(diagDataElem, EquipmentPropNames::DIAG_DATA_SERVICE_IP, &diagDataServiceIP, errMsg);
		result &= DomXmlHelper::getIntAttribute(diagDataElem, EquipmentPropNames::DIAG_DATA_SERVICE_PORT, &diagDataServicePort, errMsg);
		result &= DomXmlHelper::getStringAttribute(diagDataElem, EquipmentPropNames::DIAG_DATA_SERVICE_NETMASK, &diagDataServiceNetmask, errMsg);
		result &= DomXmlHelper::getIntAttribute(diagDataElem, EquipmentPropNames::DIAG_DATA_SIZE_BYTES, &diagDataSizeBytes, errMsg);
		result &= DomXmlHelper::getUInt32Attribute(diagDataElem, EquipmentPropNames::DIAG_DATA_UID, &diagDataUID, errMsg);
		result &= DomXmlHelper::getIntAttribute(diagDataElem, EquipmentPropNames::DIAG_DATA_FRAMES_QUANTITY, &diagDataFramesQuantity, errMsg);
		result &= DomXmlHelper::getIntAttribute(diagDataElem, EquipmentPropNames::OVERRIDE_DIAG_DATA_WORD_COUNT, &overrideDiagDataWordCount, errMsg);

		QDomElement diagSignals;

		result &= DomXmlHelper::getSingleChildElement(diagDataElem,
													  XmlElement::DIAG_DATA_ASSOCIATED_SIGNALS,
													  &diagSignals,
													  errMsg);

		diagDataAssociatedSignals = diagSignals.text().split(Separator::COMMA, Qt::SkipEmptyParts);

		RETURN_IF_FALSE(result);
	}

	return result;
}

void LanControllerInfo::saveToProto(Network::LanControllerInfo* proto, bool includeSignals) const
{
	TEST_PTR_RETURN(proto);

	proto->set_equipmentid(equipmentID.toStdString());
	proto->set_controllerno(controllerNo);
	proto->set_lancontrollertype(TO_INT(lanControllerType));

	//

	proto->set_tuningenable(tuningEnable);
	proto->set_tuningip(tuningIP.toStdString());
	proto->set_tuningport(tuningPort);
	proto->set_tuningserviceid(tuningServiceID.toStdString());
	proto->set_tuningserviceip(tuningServiceIP.toStdString());
	proto->set_tuningserviceport(tuningServicePort);
	proto->set_tuningservicenetmask(tuningServiceNetmask.toStdString());
	proto->set_tuningdatauid(tuningDataUID);

	if (includeSignals == true)
	{
		proto->set_tuningassociatedsignals(tuningAssociatedSignals.join(Separator::COMMA).toStdString());
	}
	else
	{
		proto->release_tuningassociatedsignals();
	}

	//

	proto->set_appdataenable(appDataEnable);
	proto->set_appdataip(appDataIP.toStdString());
	proto->set_appdataport(appDataPort);
	proto->set_appdataserviceid(appDataServiceID.toStdString());
	proto->set_appdataserviceip(appDataServiceIP.toStdString());
	proto->set_appdataserviceport(appDataServicePort);
	proto->set_appdataservicenetmask(appDataServiceNetmask.toStdString());
	proto->set_appdatauid(appDataUID);
	proto->set_appdatasizebytes(appDataSizeBytes);
	proto->set_appdataframesquantity(appDataFramesQuantity);
	proto->set_overrideappdatawordcount(overrideAppDataWordCount);

	if (includeSignals == true)
	{
		proto->set_appdataassociatedsignals(appDataAssociatedSignals.join(Separator::COMMA).toStdString());
	}
	else
	{
		proto->release_appdataassociatedsignals();
	}

	//

	proto->set_diagdataenable(diagDataEnable);
	proto->set_diagdataip(diagDataIP.toStdString());
	proto->set_diagdataport(diagDataPort);
	proto->set_diagdataserviceid(diagDataServiceID.toStdString());
	proto->set_diagdataserviceip(diagDataServiceIP.toStdString());
	proto->set_diagdataserviceport(diagDataServicePort);
	proto->set_diagdataservicenetmask(diagDataServiceNetmask.toStdString());
	proto->set_diagdatauid(diagDataUID);
	proto->set_diagdatasizebytes(diagDataSizeBytes);
	proto->set_diagdataframesquantity(diagDataFramesQuantity);
	proto->set_overridediagdatawordcount(overrideDiagDataWordCount);

	if (includeSignals == true)
	{
		proto->set_diagdataassociatedsignals(diagDataAssociatedSignals.join(Separator::COMMA).toStdString());
	}
	else
	{
		proto->release_diagdataassociatedsignals();
	}
}

void LanControllerInfo::loadFromProto(const Network::LanControllerInfo& proto)
{
	equipmentID = QString::fromStdString(proto.equipmentid());
	controllerNo = proto.controllerno();
	lanControllerType = static_cast<E::LanControllerType>(proto.lancontrollertype());

	//

	tuningEnable = proto.tuningenable();
	tuningIP = QString::fromStdString(proto.tuningip());
	tuningPort = proto.tuningport();
	tuningServiceID = QString::fromStdString(proto.tuningserviceid());
	tuningServiceIP = QString::fromStdString(proto.tuningserviceip());
	tuningServicePort = proto.tuningserviceport();
	tuningServiceNetmask = QString::fromStdString(proto.tuningservicenetmask());
	tuningDataUID = proto.tuningdatauid();
	tuningAssociatedSignals = QString::fromStdString(proto.tuningassociatedsignals()).split(Separator::COMMA, Qt::SkipEmptyParts);

	//

	appDataEnable = proto.appdataenable();
	appDataIP = QString::fromStdString(proto.appdataip());
	appDataPort = proto.appdataport();
	appDataServiceID = QString::fromStdString(proto.appdataserviceid());
	appDataServiceIP = QString::fromStdString(proto.appdataserviceip());
	appDataServicePort = proto.appdataserviceport();
	appDataServiceNetmask = QString::fromStdString(proto.appdataservicenetmask());
	appDataUID = proto.appdatauid();
	appDataSizeBytes = proto.appdatasizebytes();
	appDataFramesQuantity = proto.appdataframesquantity();
	overrideAppDataWordCount = proto.overrideappdatawordcount();
	appDataAssociatedSignals = QString::fromStdString(proto.appdataassociatedsignals()).split(Separator::COMMA, Qt::SkipEmptyParts);

	//

	diagDataEnable = proto.diagdataenable();
	diagDataIP = QString::fromStdString(proto.diagdataip());
	diagDataPort = proto.diagdataport();
	diagDataServiceID = QString::fromStdString(proto.diagdataserviceid());
	diagDataServiceIP = QString::fromStdString(proto.diagdataserviceip());
	diagDataServicePort = proto.diagdataserviceport();
	diagDataServiceNetmask = QString::fromStdString(proto.diagdataservicenetmask());
	diagDataUID = proto.diagdatauid();
	diagDataSizeBytes = proto.diagdatasizebytes();
	diagDataFramesQuantity = proto.diagdataframesquantity();
	overrideDiagDataWordCount = proto.overridediagdatawordcount();
	diagDataAssociatedSignals = QString::fromStdString(proto.diagdataassociatedsignals()).split(Separator::COMMA, Qt::SkipEmptyParts);
}



