#pragma once

#include "../Builder/OptoModule.h"
#include "../UtilsLib/XmlHelper.h"
#include "../UtilsLib/WUtils.h"

#include "../HardwareLib/Connection.h"
#include "../HardwareLib/ConnectionsInfo.h"


class ConnectionsInfoWriter : public ConnectionsInfo
{
public:
	bool fill(const Builder::ConnectionStorage& connectionsStorage, const Hardware::OptoModuleStorage& optoModuleStorage);
	void save(QByteArray* xmlFileData) const;

private:

	bool fill(ConnectionInfo* ci, Hardware::SharedConnection connection, const Hardware::OptoModuleStorage& optoModuleStorage);
	void save(const ConnectionInfo& ci, XmlWriteHelper& xml) const;

	bool fill(ConnectionPortInfo* cpi, Hardware::SharedConnection connection, int prtNo, const Hardware::OptoModuleStorage& optoModuleStorage);
	void save(const ConnectionPortInfo& cpi, XmlWriteHelper& xml) const;

	void save(const ConnectionTxRxSignal& cs, XmlWriteHelper& xml) const;
};


