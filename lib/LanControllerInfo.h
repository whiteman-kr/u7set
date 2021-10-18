#pragma once

#include "../CommonLib/Types.h"
#include "../UtilsLib/XmlHelper.h"
#include "../UtilsLib/DomXmlHelper.h"
#include "../UtilsLib/WUtils.h"
#include "../Proto/network.pb.h"

struct LanControllerInfo
{
	QString equipmentID;
	int controllerNo = -1;				// == place
	E::LanControllerType lanControllerType = E::LanControllerType::Unknown;

	// used if LAN controller provide Tuning
	//
	bool tuningEnable = false;
	QString tuningIP;
	int tuningPort = 0;
	QString tuningServiceID;
	QString tuningServiceIP;
	int tuningServicePort = 0;
	QString tuningServiceNetmask;
	quint64 tuningDataUID = 0;
	QStringList tuningAssociatedSignals;

	// used if LAN controller provide AppData
	//
	bool appDataEnable = false;
	QString appDataIP;
	int appDataPort = 0;
	QString appDataServiceID;
	QString appDataServiceIP;
	int appDataServicePort = 0;
	QString appDataServiceNetmask;
	quint32 appDataUID = 0;
	int appDataSizeBytes = 0;
	int appDataFramesQuantity = 0;
	int overrideAppDataWordCount = -1;
	QStringList appDataAssociatedSignals;

	// used if LAN controller provide DiagData
	//
	bool diagDataEnable = false;
	QString diagDataIP;
	int diagDataPort = 0;
	QString diagDataServiceID;
	QString diagDataServiceIP;
	int diagDataServicePort = 0;
	QString diagDataServiceNetmask;
	quint32 diagDataUID = 0;
	int diagDataSizeBytes = 0;
	int diagDataFramesQuantity = 0;
	int overrideDiagDataWordCount = -1;
	QStringList diagDataAssociatedSignals;

	//

	static bool isProvideTuning(E::LanControllerType lanControllerType);
	static bool isProvideAppData(E::LanControllerType lanControllerType);
	static bool isProvideDiagData(E::LanControllerType lanControllerType);

	//

	bool isProvideTuning() const;
	bool isProvideAppData() const;
	bool isProvideDiagData() const;

	bool writeToXml(XmlWriteHelper& xml)const;
	bool readFromXml(XmlReadHelper& xml);
	bool readFromXml(const QDomNode& lanControllerNode, QString* errMsg);

	void saveToProto(Network::LanControllerInfo* proto, bool includeSignals) const;
	void loadFromProto(const Network::LanControllerInfo& proto);
};
