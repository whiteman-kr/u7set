#pragma once

#include "../CommonLib/Types.h"
#include "../UtilsLib/XmlHelper.h"
#include "../UtilsLib/DomXmlHelper.h"
#include "../UtilsLib/WUtils.h"
#include "../Proto/network.pb.h"

struct LanControllerInfo
{
	QString equipmentID;
	int controllerNo = -1;			// == place
	E::LanControllerType lanControllerType = E::LanControllerType::Unknown;
	int channel = 0;				// 0 is a First channel!

	// used if LAN controller provide Tuning
	//
	bool tuningEnable = false;
	QString tuningIP;
	int tuningPort = 0;
	QString tuningServiceID;
	QString tuningServiceIP;
	int tuningServicePort = 0;
	QString tuningServiceNetmask;

	// used if LAN controller provide AppData
	//
	bool appDataEnable = false;
	QString appDataIP;
	int appDataPort = 0;
	QString appDataServiceID;
	QString appDataServiceIP;
	int appDataServicePort = 0;
	QString appDataServiceNetmask;

	// used if LAN controller provide DiagData
	//
	bool diagDataEnable = false;
	QString diagDataIP;
	int diagDataPort = 0;
	QString diagDataServiceID;
	QString diagDataServiceIP;
	int diagDataServicePort = 0;
	QString diagDataServiceNetmask;

	//

	bool isValid() const;

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

private:
	const LanControllerInfo& find(int controllerNo) const;
	const LanControllerInfo& find(const QString& equipmentID) const;

	bool contains(int controllerNo) const;
	bool contains(const QString& equipmentID) const;

private:
	std::vector<LanControllerInfo> m_lans;

private:
	static LanControllerInfo m_notValidInfo;
};
