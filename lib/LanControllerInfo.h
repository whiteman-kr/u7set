#pragma once

#include "../CommonLib/Types.h"
#include "../UtilsLib/XmlHelper.h"
#include "../UtilsLib/DomXmlHelper.h"
#include "../UtilsLib/WUtils.h"
#include "../Proto/network.pb.h"
#include "../CommonLib/HostAddressPort.h"

struct LanControllerInfo
{
	QString equipmentID;
	int controllerNo = -1;			// == place
	E::LanControllerType lanControllerType = E::LanControllerType::Unknown;

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

	//

	bool isValid() const;

	//

	quint32 appDataIP32() const;
	quint32 tuningIP32() const;
	quint32 diagDataIP32() const;

	//

	static bool isProvideTuning(E::LanControllerType lanControllerType);
	static bool isProvideAppData(E::LanControllerType lanControllerType);
	static bool isProvideDiagData(E::LanControllerType lanControllerType);

	//

	bool isProvideTuning() const;
	bool isProvideAppData() const;
	bool isProvideDiagData() const;

	void writeToXml(XmlWriteHelper& xml)const;
	bool readFromXml(XmlReadHelper& xml);
	bool readFromXml(const QDomNode& lanControllerNode, QString* errMsg);

	void saveToProto(Network::LanControllerInfo* proto) const;
	void loadFromProto(const Network::LanControllerInfo& proto);

	bool operator == (int conrollerNo) const;
	bool operator == (const QString& equipmentID) const;
};

class LanControllersInfo
{
public:
	const LanControllerInfo& getInfo(int controllerNo) const;
	const LanControllerInfo& getInfo(const QString& controllerEquipmentID) const;

	void writeToXml(XmlWriteHelper& xml)const;
	bool readFromXml(XmlReadHelper& xml);
	bool readFromXml(const QDomNode& lanControllersNode, QString* errMsg);

	void resize(int newSize);
	void clear();
	void append(const LanControllerInfo& info);

	const std::vector<LanControllerInfo>& operator()() const;

	const LanControllerInfo& operator[](int index) const;
	LanControllerInfo& operator[](int index);

	const LanControllerInfo& getFirstCompatibleController(E::LanControllerType type) const;

	std::vector<quint32> tuningIP32addresses() const;
	std::vector<quint32> appDataIP32addresses() const;
	std::vector<HostAddressPort> appDataHostAddressPorts() const;

private:
	const LanControllerInfo& find(int controllerNo) const;
	const LanControllerInfo& find(const QString& equipmentID) const;
	const LanControllerInfo& findByIndex(int index) const;

	bool contains(int controllerNo) const;
	bool contains(const QString& equipmentID) const;

private:
	std::vector<LanControllerInfo> m_lans;

private:
	static LanControllerInfo m_notValidInfo;
};
