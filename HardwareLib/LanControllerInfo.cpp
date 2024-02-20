#include "LanControllerInfo.h"
#include "../UtilsLib/DomXmlHelper.h"
#include <QDomNode>
// -----------------------------------------------------------------------------
//
// LanControllerInfo struct implementation
//
// -----------------------------------------------------------------------------

bool LanControllerInfo::isValid() const
{
	return equipmentID.isEmpty() == false;
}

HostAddressPort LanControllerInfo::appDataHostAddressPort() const
{
	return HostAddressPort(appDataIP, appDataPort);
}

HostAddressPort LanControllerInfo::tuningHostAddressPort() const
{
	return HostAddressPort(tuningIP, tuningPort);
}

HostAddressPort LanControllerInfo::diagDataHostAddressPort() const
{
	return HostAddressPort(diagDataIP, diagDataPort);
}

quint32 LanControllerInfo::appDataIP32() const
{
	return QHostAddress(appDataIP).toIPv4Address();
}

quint32 LanControllerInfo::tuningIP32() const
{
	return QHostAddress(tuningIP).toIPv4Address();
}

quint32 LanControllerInfo::diagDataIP32() const
{
	return QHostAddress(diagDataIP).toIPv4Address();
}

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

bool LanControllerInfo::isTuningEnabled() const
{
	return isProvideTuning() && tuningEnable;
}

bool LanControllerInfo::isAppDataEnabled() const
{
	return isProvideAppData() && appDataEnable;
}

bool LanControllerInfo::isDiagDataEnabled() const
{
	return isProvideDiagData() && diagDataEnable;
}

bool LanControllerInfo::isEnabled(E::LanControllerType lanCtrlType) const
{
	switch(lanCtrlType)
	{
	case E::LanControllerType::AppData:
		return isAppDataEnabled();

	case E::LanControllerType::DiagData:
		return isDiagDataEnabled();

	case E::LanControllerType::Tuning:
		return isTuningEnabled();

	case E::LanControllerType::Unknown:
	case E::LanControllerType::AppAndDiagData:
	case E::LanControllerType::TuningAndAppAndDiagData:
	default:
		// lanCtrlType - no mixed types allowed
		Q_ASSERT(false);
	}

	return false;
}


void LanControllerInfo::writeToXml(XmlWriteHelper& xml) const
{
	xml.writeStartElement(XmlElement::LAN_CONTROLLER);

	xml.writeStringAttribute(EquipmentPropNames::EQUIPMENT_ID, equipmentID);
	xml.writeIntAttribute(EquipmentPropNames::CONTROLLER_NO, controllerNo);
	xml.writeEnumKeyAttribute(EquipmentPropNames::LAN_CONTROLLER_TYPE, lanControllerType);

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
		xml.writeUInt32Attribute(EquipmentPropNames::RUP_APP_DATA_UID, rupAppDataUID, false);
		xml.writeUInt32Attribute(EquipmentPropNames::HEX_RUP_APP_DATA_UID, rupAppDataUID, true);
		xml.writeIntAttribute(EquipmentPropNames::APP_DATA_FRAMES_QUANTITY, appDataFramesQuantity);
		xml.writeIntAttribute(EquipmentPropNames::OVERRIDE_APP_DATA_WORD_COUNT, overrideAppDataWordCount);
	}

	xml.writeEndElement();	//	/XmlElement::APP_DATA_PARAMS

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

		xml.writeUInt32Attribute(EquipmentPropNames::RUP_TUNING_DATA_UID, rupTuningDataUID, false);
		xml.writeUInt32Attribute(EquipmentPropNames::HEX_RUP_TUNING_DATA_UID, rupTuningDataUID, true);

		xml.writeUInt64Attribute(EquipmentPropNames::FOTIP_TUNING_DATA_UID, fotipTuningDataUID);
		xml.writeUInt64Attribute(EquipmentPropNames::HEX_FOTIP_TUNING_DATA_UID, fotipTuningDataUID, true);
	}

	xml.writeEndElement();	//	/XmlElement::TUNING_PARAMS

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
		xml.writeUInt32Attribute(EquipmentPropNames::RUP_DIAG_DATA_UID, rupDiagDataUID, false);
		xml.writeUInt32Attribute(EquipmentPropNames::HEX_RUP_DIAG_DATA_UID, rupDiagDataUID, true);
		xml.writeIntAttribute(EquipmentPropNames::DIAG_DATA_FRAMES_QUANTITY, diagDataFramesQuantity);
		xml.writeIntAttribute(EquipmentPropNames::OVERRIDE_DIAG_DATA_WORD_COUNT, overrideDiagDataWordCount);
	}

	xml.writeEndElement();	//	/XmlElement::DIAG_DATA_PARAMS

	//

	xml.writeEndElement();	//	/XmlElement::LAN_CONTROLLER
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
	result &= xml.readEnumKeyAttribute(EquipmentPropNames::LAN_CONTROLLER_TYPE, &lanControllerType);

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
		result &= xml.readUInt32Attribute(EquipmentPropNames::RUP_APP_DATA_UID, &rupAppDataUID);
		result &= xml.readIntAttribute(EquipmentPropNames::APP_DATA_FRAMES_QUANTITY, &appDataFramesQuantity);
		result &= xml.readIntAttribute(EquipmentPropNames::OVERRIDE_APP_DATA_WORD_COUNT, &overrideAppDataWordCount);

		RETURN_IF_FALSE(result);
	}

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

		result &= xml.readUInt32Attribute(EquipmentPropNames::RUP_TUNING_DATA_UID, &rupTuningDataUID);
		result &= xml.readUInt64Attribute(EquipmentPropNames::FOTIP_TUNING_DATA_UID, &fotipTuningDataUID);

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
		result &= xml.readUInt32Attribute(EquipmentPropNames::RUP_DIAG_DATA_UID, &rupDiagDataUID);
		result &= xml.readIntAttribute(EquipmentPropNames::DIAG_DATA_FRAMES_QUANTITY, &diagDataFramesQuantity);
		result &= xml.readIntAttribute(EquipmentPropNames::OVERRIDE_DIAG_DATA_WORD_COUNT, &overrideDiagDataWordCount);
	}

	return result;
}


bool LanControllerInfo::readFromXml(const QDomNode& lanControllerNode, QString* errMsg)
{
	TEST_PTR_RETURN_FALSE(errMsg);

	if (lanControllerNode.isElement() == false || lanControllerNode.nodeName() != XmlElement::LAN_CONTROLLER)
	{
		*errMsg = DomXmlHelper::errElementNotFound(XmlElement::LAN_CONTROLLER);
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
		result &= DomXmlHelper::getUInt32Attribute(appDataElem, EquipmentPropNames::RUP_APP_DATA_UID, &rupAppDataUID, errMsg);
		result &= DomXmlHelper::getIntAttribute(appDataElem, EquipmentPropNames::APP_DATA_FRAMES_QUANTITY, &appDataFramesQuantity, errMsg);
		result &= DomXmlHelper::getIntAttribute(appDataElem, EquipmentPropNames::OVERRIDE_APP_DATA_WORD_COUNT, &overrideAppDataWordCount, errMsg);

		RETURN_IF_FALSE(result);
	}

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

		result &= DomXmlHelper::getUInt32Attribute(tuningElem, EquipmentPropNames::RUP_TUNING_DATA_UID, &rupTuningDataUID, errMsg);
		result &= DomXmlHelper::getUInt64Attribute(tuningElem, EquipmentPropNames::FOTIP_TUNING_DATA_UID, &fotipTuningDataUID, errMsg);

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
		result &= DomXmlHelper::getUInt32Attribute(diagDataElem, EquipmentPropNames::RUP_DIAG_DATA_UID, &rupDiagDataUID, errMsg);
		result &= DomXmlHelper::getIntAttribute(diagDataElem, EquipmentPropNames::DIAG_DATA_FRAMES_QUANTITY, &diagDataFramesQuantity, errMsg);
		result &= DomXmlHelper::getIntAttribute(diagDataElem, EquipmentPropNames::OVERRIDE_DIAG_DATA_WORD_COUNT, &overrideDiagDataWordCount, errMsg);

		RETURN_IF_FALSE(result);
	}

	return result;
}

void LanControllerInfo::saveToProto(Network::LanControllerInfo* proto) const
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
	proto->set_ruptuningdatauid(rupTuningDataUID);
	proto->set_fotiptuningdatauid(fotipTuningDataUID);

	//

	proto->set_appdataenable(appDataEnable);
	proto->set_appdataip(appDataIP.toStdString());
	proto->set_appdataport(appDataPort);
	proto->set_appdataserviceid(appDataServiceID.toStdString());
	proto->set_appdataserviceip(appDataServiceIP.toStdString());
	proto->set_appdataserviceport(appDataServicePort);
	proto->set_appdataservicenetmask(appDataServiceNetmask.toStdString());
	proto->set_rupappdatauid(rupAppDataUID);
	proto->set_appdatasizebytes(appDataSizeBytes);
	proto->set_appdataframesquantity(appDataFramesQuantity);
	proto->set_overrideappdatawordcount(overrideAppDataWordCount);

	//

	proto->set_diagdataenable(diagDataEnable);
	proto->set_diagdataip(diagDataIP.toStdString());
	proto->set_diagdataport(diagDataPort);
	proto->set_diagdataserviceid(diagDataServiceID.toStdString());
	proto->set_diagdataserviceip(diagDataServiceIP.toStdString());
	proto->set_diagdataserviceport(diagDataServicePort);
	proto->set_diagdataservicenetmask(diagDataServiceNetmask.toStdString());
	proto->set_rupdiagdatauid(rupDiagDataUID);
	proto->set_diagdatasizebytes(diagDataSizeBytes);
	proto->set_diagdataframesquantity(diagDataFramesQuantity);
	proto->set_overridediagdatawordcount(overrideDiagDataWordCount);
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
	rupTuningDataUID = proto.ruptuningdatauid();
	fotipTuningDataUID = proto.fotiptuningdatauid();

	//

	appDataEnable = proto.appdataenable();
	appDataIP = QString::fromStdString(proto.appdataip());
	appDataPort = proto.appdataport();
	appDataServiceID = QString::fromStdString(proto.appdataserviceid());
	appDataServiceIP = QString::fromStdString(proto.appdataserviceip());
	appDataServicePort = proto.appdataserviceport();
	appDataServiceNetmask = QString::fromStdString(proto.appdataservicenetmask());
	rupAppDataUID = proto.rupappdatauid();
	appDataSizeBytes = proto.appdatasizebytes();
	appDataFramesQuantity = proto.appdataframesquantity();
	overrideAppDataWordCount = proto.overrideappdatawordcount();

	//

	diagDataEnable = proto.diagdataenable();
	diagDataIP = QString::fromStdString(proto.diagdataip());
	diagDataPort = proto.diagdataport();
	diagDataServiceID = QString::fromStdString(proto.diagdataserviceid());
	diagDataServiceIP = QString::fromStdString(proto.diagdataserviceip());
	diagDataServicePort = proto.diagdataserviceport();
	diagDataServiceNetmask = QString::fromStdString(proto.diagdataservicenetmask());
	rupDiagDataUID = proto.rupdiagdatauid();
	diagDataSizeBytes = proto.diagdatasizebytes();
	diagDataFramesQuantity = proto.diagdataframesquantity();
	overrideDiagDataWordCount = proto.overridediagdatawordcount();
}

bool LanControllerInfo::operator == (int controllerNo) const
{
	return this->controllerNo == controllerNo;
}

bool LanControllerInfo::operator == (const QString& equipmentID) const
{
	return this->equipmentID == equipmentID;
}

// -----------------------------------------------------------------------------
//
// LanControllersInfo struct implementation
//
// -----------------------------------------------------------------------------

LanControllerInfo LanControllersInfo::m_notValidInfo;

const LanControllerInfo& LanControllersInfo::getInfo(int controllerNo) const
{
	return find(controllerNo);
}

const LanControllerInfo& LanControllersInfo::getInfo(const QString& controllerEquipmentID) const
{
	return find(controllerEquipmentID);
}

void LanControllersInfo::writeToXml(XmlWriteHelper& xml)const
{
	xml.writeStartElement(XmlElement::LAN_CONTROLLERS);
	xml.writeIntAttribute(XmlAttribute::COUNT, static_cast<int>(m_lans.size()));

	xml.writeIntAttribute(XmlAttribute::RUP_VERSION, m_rupVersion);
	xml.writeIntAttribute(XmlAttribute::FOTIP_VERSION, m_fotipVersion);

	for(const LanControllerInfo& lci : m_lans)
	{
		lci.writeToXml(xml);
	}

	xml.writeEndElement();	//		/XmlElement::LAN_CONTROLLERS
}

bool LanControllersInfo::readFromXml(XmlReadHelper& xml)
{
	if (xml.name() != XmlElement::LAN_CONTROLLERS)
	{
		return false;
	}

	bool result = true;

	int count = 0;

	result &= xml.readIntAttribute(XmlAttribute::COUNT, &count);
	result &= xml.readIntAttribute(XmlAttribute::RUP_VERSION, &m_rupVersion);
	result &= xml.readIntAttribute(XmlAttribute::FOTIP_VERSION, &m_fotipVersion);

	RETURN_IF_FALSE(result);

	resize(count);

	for(int i = 0; i < count; i++)
	{
		result &= xml.findElement(XmlElement::LAN_CONTROLLER);
		result &= m_lans[i].readFromXml(xml);

		BREAK_IF_FALSE(result);
	}

	return result;
}

bool LanControllersInfo::readFromXml(const QDomNode& lanControllersNode, QString* errMsg)
{
	if (lanControllersNode.isElement() == false || lanControllersNode.nodeName() != XmlElement::LAN_CONTROLLERS)
	{
		*errMsg = DomXmlHelper::errElementNotFound(XmlElement::LAN_CONTROLLERS);
		return false;
	}

	QDomElement lanControllersElem = lanControllersNode.toElement();

	bool result = true;

	int count = 0;

	result &= DomXmlHelper::getIntAttribute(lanControllersElem, XmlAttribute::COUNT, &count, errMsg);
	result &= DomXmlHelper::getIntAttribute(lanControllersElem, XmlAttribute::RUP_VERSION, &m_rupVersion, errMsg);
	result &= DomXmlHelper::getIntAttribute(lanControllersElem, XmlAttribute::FOTIP_VERSION, &m_fotipVersion, errMsg);

	QDomNodeList lanControllerNodes = lanControllersElem.elementsByTagName(XmlElement::LAN_CONTROLLER);

	if (lanControllerNodes.count() != count)
	{
		*errMsg = QString("File corruption! Count of LanController nodes is not equal to LanControllers Count attribute value");
		return false;
	}

	resize(count);

	for(int i = 0; i < count; i++)
	{
		QDomNode lanControllerNode = lanControllerNodes.at(i);

		result &= m_lans[i].readFromXml(lanControllerNode, errMsg);

		BREAK_IF_FALSE(result);
	}

	return result;
}

void LanControllersInfo::resize(int newSize)
{
	m_lans.clear();
	m_lans.resize(newSize);
}

void LanControllersInfo::clear()
{
	m_lans.clear();
}

void LanControllersInfo::append(const LanControllerInfo& info)
{
	Q_ASSERT(contains(info.controllerNo) == false);
	Q_ASSERT(contains(info.equipmentID) == false);

	m_lans.push_back(info);
}

void LanControllersInfo::filterLansByAppDataServiceID(const QString& appDataServiceID)
{
	auto it = m_lans.begin();

	while(it != m_lans.end())
	{
		Q_ASSERT(it->isProvideAppData() == true);

		if (it->appDataServiceID != appDataServiceID)
		{
			m_lans.erase(it);
			it = m_lans.begin();
		}
		else
		{
			it++;
		}
	}
}

void LanControllersInfo::filterLansByDiagDataServiceID(const QString& diagDataServiceID)
{
	auto it = m_lans.begin();

	while(it != m_lans.end())
	{
		Q_ASSERT(it->isProvideDiagData() == true);

		if (it->diagDataServiceID != diagDataServiceID)
		{
			m_lans.erase(it);
			it = m_lans.begin();
		}
		else
		{
			it++;
		}
	}
}

void LanControllersInfo::filterLansByTuningServiceID(const QString& tuningServiceID)
{
	auto it = m_lans.begin();

	while(it != m_lans.end())
	{
		Q_ASSERT(it->isProvideTuning() == true);

		if (it->tuningServiceID != tuningServiceID)
		{
			m_lans.erase(it);
			it = m_lans.begin();
		}
		else
		{
			it++;
		}
	}
}

void LanControllersInfo::filterLansByTuningServiceLinkIDs(const std::set<QString>& tuningServiceLinkIDs)
{
	auto it = m_lans.begin();

	while(it != m_lans.end())
	{
		Q_ASSERT(it->isProvideTuning() == true);

		if (tuningServiceLinkIDs.contains(it->tuningServiceID) == false)
		{
			m_lans.erase(it);
			it = m_lans.begin();
		}
		else
		{
			it++;
		}
	}
}

const std::vector<LanControllerInfo>& LanControllersInfo::operator ()() const
{
	return m_lans;
}

const LanControllerInfo& LanControllersInfo::operator[](int index) const
{
	return findByIndex(index);
}

const LanControllerInfo& LanControllersInfo::getFirstCompatibleController(E::LanControllerType type) const
{
	for(const LanControllerInfo& lci : m_lans)
	{
		if ((TO_INT(lci.lanControllerType) & TO_INT(type)) != 0)
		{
			return lci;
		}
	}

	return m_notValidInfo;
}

std::vector<quint32> LanControllersInfo::tuningIP32addresses() const
{
	std::vector<quint32> addresses;

	for(const LanControllerInfo& lci : m_lans)
	{
		if (lci.isProvideTuning() == true)
		{
			addresses.push_back(lci.tuningIP32());
		}
	}

	return addresses;
}

std::vector<quint32> LanControllersInfo::appDataIP32addresses() const
{
	std::vector<quint32> addresses;

	for(const LanControllerInfo& lci : m_lans)
	{
		if (lci.isProvideAppData() == true)
		{
			addresses.push_back(lci.appDataIP32());
		}
	}

	return addresses;
}

std::vector<HostAddressPort> LanControllersInfo::appDataHostAddressPorts() const
{
	std::vector<HostAddressPort> addresses;

	for(const LanControllerInfo& lci : m_lans)
	{
		if (lci.isProvideAppData() == true)
		{
			addresses.emplace_back(lci.appDataIP, lci.appDataPort);
		}
	}

	return addresses;
}

int LanControllersInfo::rupVersion() const
{
	return m_rupVersion;
}

void LanControllersInfo::setRupVersion(int v)
{
	m_rupVersion = v;
}

int LanControllersInfo::fotipVersion() const
{
	return m_fotipVersion;
}

void LanControllersInfo::setFotipVersion(int v)
{
	m_fotipVersion = v;
}

LanControllerInfo& LanControllersInfo::operator[](int index)
{
	return const_cast<LanControllerInfo&>(findByIndex(index));
}

const LanControllerInfo& LanControllersInfo::find(int controllerNo) const
{
	auto it = std::find(m_lans.begin(), m_lans.end(), controllerNo);

	if (it != m_lans.end())
	{
		return *it;
	}

//	Q_ASSERT(false);

	return m_notValidInfo;
}

const LanControllerInfo& LanControllersInfo::find(const QString& equipmentID) const
{
	auto it = std::find(m_lans.begin(), m_lans.end(), equipmentID);

	if (it != m_lans.end())
	{
		return *it;
	}

//	Q_ASSERT(false);

	return m_notValidInfo;
}

const LanControllerInfo& LanControllersInfo::findByIndex(int index) const
{
	if (index >=0 && index < std::ssize(m_lans))
	{
		return m_lans[index];
	}

	Q_ASSERT(false);

	return m_notValidInfo;
}


bool LanControllersInfo::contains(int controllerNo) const
{
	return std::find(m_lans.begin(), m_lans.end(), controllerNo) != m_lans.end();
}

bool LanControllersInfo::contains(const QString& equipmentID) const
{
	return std::find(m_lans.begin(), m_lans.end(), equipmentID) != m_lans.end();
}




