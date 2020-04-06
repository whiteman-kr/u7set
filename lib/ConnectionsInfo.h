#pragma once

#ifndef IS_SIMULATOR

#include "Connection.h"
#include "../Builder/OptoModule.h"
#include "../Builder/IssueLogger.h"
#include "WUtils.h"

#endif

#include <QDomDocument>

struct ConnectionPortInfo
{
	int portNo = -1;
	QString equipmentID;
	QString moduleID;

	//

	int manualRxWordsQuantity = 0;
	int manualTxStartAddress = 0;
	int manualTxWordsQuantiy = 0;

	//

	bool enableSerial = false;
	bool enableDuplex = false;
	QString serialMode;

	//

	int txBufferAbsAddressW = 0;
	int txDataSizeW = 0;
	quint32 txDataID = 0;

	//

	int rxBufferAbsAddressW = 0;
	int rxDataSizeW = 0;
	quint32 rxDataID = 0;

	QString rxValiditySignalID;
	int rxValiditySignalAddressW;
	int rxValiditySignalBit;

	//

	QString portTag() const;

#ifndef IS_SIMULATOR

	bool fill(Hardware::SharedConnection connection, int prtNo, const Hardware::OptoModuleStorage& optoModuleStorage);
	bool save(QDomDocument& doc, QDomElement& connectionElement) const;

#endif
};

struct ConnectionInfo
{
	// Common
	//
	QString ID;
	int linkID = -1;
	QString type;
	bool enableManualSettings = false;
	bool disableDataIdControl = false;

	ConnectionPortInfo port1;
	ConnectionPortInfo port2;

#ifndef IS_SIMULATOR

	bool fill(Hardware::SharedConnection connection, const Hardware::OptoModuleStorage& optoModuleStorage);
	bool save(QDomDocument& doc, QDomElement& connectionsElement) const;

#endif

	//bool load(const XmlReadHelper& xml, QString* errMsg);
};

struct ConnectionsInfo
{
	std::vector<ConnectionInfo> connections;

#ifndef IS_SIMULATOR

	bool fill(const Hardware::ConnectionStorage& connectionsStorage, const Hardware::OptoModuleStorage& optoModuleStorage);
	void save(QByteArray* xmlFileData) const;

#endif

	bool load(const QString fileName, QString* errMsg);
};



