#pragma once

#include "../UtilsLib/XmlHelper.h"
#include "../CommonLib/HostAddressPort.h"
#include "DataProtocols.h"

class QDomNode;

namespace Network
{
	class LanControllerInfo;
}

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
	quint32 rupAppDataUID = 0;
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
	quint32 rupTuningDataUID = 0;
	quint64 fotipTuningDataUID = 0;

	// used if LAN controller provide DiagData
	//
	bool diagDataEnable = false;
	QString diagDataIP;
	int diagDataPort = 0;
	QString diagDataServiceID;
	QString diagDataServiceIP;
	int diagDataServicePort = 0;
	QString diagDataServiceNetmask;
	quint32 rupDiagDataUID = 0;
	int diagDataSizeBytes = 0;
	int diagDataFramesQuantity = 0;
	int overrideDiagDataWordCount = -1;

	HostAddressPort diagDataHostAddressPort() { return HostAddressPort(diagDataIP, diagDataPort); }

	//

	bool isValid() const;

	//

	HostAddressPort appDataHostAddressPort() const;
	HostAddressPort tuningHostAddressPort() const;
	HostAddressPort diagDataHostAddressPort() const;

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

	bool isTuningEnabled() const;
	bool isAppDataEnabled() const;
	bool isDiagDataEnabled() const;

	bool isEnabled(E::LanControllerType lanCtrlType) const;

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

	void filterLansByAppDataServiceID(const QString& appDataServiceID);
	void filterLansByDiagDataServiceID(const QString& diagDataServiceID);
	void filterLansByTuningServiceID(const QString& tuningServiceID);
	void filterLansByTuningServiceLinkIDs(const std::set<QString>& tuningServiceLinkIDs);

	const std::vector<LanControllerInfo>& operator()() const;

	const LanControllerInfo& operator[](int index) const;
	LanControllerInfo& operator[](int index);

	const LanControllerInfo& getFirstCompatibleController(E::LanControllerType type) const;

	std::vector<quint32> tuningIP32addresses() const;
	std::vector<quint32> appDataIP32addresses() const;
	std::vector<HostAddressPort> appDataHostAddressPorts() const;

	int rupVersion() const;
	void setRupVersion(int v);

	int fotipVersion() const;
	void setFotipVersion(int v);

private:
	const LanControllerInfo& find(int controllerNo) const;
	const LanControllerInfo& find(const QString& equipmentID) const;
	const LanControllerInfo& findByIndex(int index) const;

	bool contains(int controllerNo) const;
	bool contains(const QString& equipmentID) const;

private:
	int m_rupVersion = Rup::V5;
	int m_fotipVersion = Fotip::V2;
	std::vector<LanControllerInfo> m_lans;

private:
	static LanControllerInfo m_notValidInfo;
};
