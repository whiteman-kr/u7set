#pragma once

#ifdef IS_BUILDER

#include "../HardwareLib/Connection.h"
#include "../Builder/OptoModule.h"
#include "../UtilsLib/XmlHelper.h"
#include "../UtilsLib/WUtils.h"

#endif

#include "ConstStrings.h"
#include "../UtilsLib/Address16.h"
#include "../CommonLib/Types.h"
#include "../UtilsLib/DomXmlHelper.h"

class ConnectionTxRxSignal
{
public:
	QStringList IDs;
	E::SignalType type = E::SignalType::Discrete;
	E::AnalogAppSignalFormat analogFormat = E::AnalogAppSignalFormat::SignedInt32;	// matter only for Analog signals
	QString busTypeID;																// matter only for Bus signals
	Address16 addrInBuf;
	Address16 absAddr;
	int dataSizeBits = 0;
};

class ConnectionPortInfo
{
public:
	int portNo = -1;
	QString equipmentID;
	QString moduleID;
	QString lmID;

	//

	int manualRxWordsQuantity = 0;
	int manualTxStartAddr = 0;
	int manualTxWordsQuantity = 0;

	//

	bool enableSerial = false;
	bool enableDuplex = false;
	QString serialMode;

	//

	int txBufferAbsAddr = 0;
	int txDataSizeW = 0;
	quint32 txDataID = 0;

	std::vector<ConnectionTxRxSignal> txSignals;

	//

	int rxBufferAbsAddr = 0;
	int rxDataSizeW = 0;
	quint32 rxDataID = 0;

	QString rxValiditySignalEquipmentID;
	Address16 rxValiditySignalAbsAddr;

	std::vector<ConnectionTxRxSignal> rxSignals;
};

class ConnectionInfo
{
public:
	QString ID;
	int linkID = -1;
	QString type;
	bool enableManualSettings = false;
	bool disableDataIDControl = false;

	std::vector<ConnectionPortInfo> ports;
};

class ConnectionsInfo
{
public:
	std::vector<ConnectionInfo> connections;

	bool load(const QString& fileName, QString* errMsg);
	bool load(const QByteArray& xmlData, QString* errMsg);

private:
	bool load(ConnectionInfo* ci, const QDomNode& node, QString* errMsg);
	bool load(ConnectionPortInfo* cpi, const QDomElement& connectionElement, int prtNo, QString* errMsg);
	bool load(ConnectionTxRxSignal* cs, const QDomElement& txRxSignalElem, QString* errMsg);

protected:
	static QString portTag(int portNo);

	static const QString ELEM_CONNECTIONS;
	static const QString ELEM_CONNECTION;
	static const QString ELEM_MANUAL_SETTINGS;
	static const QString ELEM_SERIAL_SETTINGS;
	static const QString ELEM_TX;
	static const QString ELEM_TX_RX_SIGNAL;
	static const QString ELEM_RX;
	static const QString ELEM_RX_VALIDITY_SIGNAL;

	static const QString ATTR_LINK_ID;
	static const QString ATTR_TYPE;
	static const QString ATTR_ANALOG_FORMAT;
	static const QString ATTR_BUS_TYPE_ID;
	static const QString ATTR_ENABLE_MANUAL_SETTINGS;
	static const QString ATTR_DISABLE_DATA_ID_CONTROL;
	static const QString ATTR_PORTS_COUNT;
	static const QString ATTR_EQUIPMENT_ID;
	static const QString ATTR_MODULE_ID;
	static const QString ATTR_LM_ID;
	static const QString ATTR_RX_WORDS_QUANTITY;
	static const QString ATTR_TX_START_ADDRESS;
	static const QString ATTR_TX_WORDS_QUANTITY;
	static const QString ATTR_ENABLE_SERIAL;
	static const QString ATTR_ENABLE_DUPLEX;
	static const QString ATTR_SERIAL_MODE;
	static const QString ATTR_BUFFER_ABS_ADDR;
	static const QString ATTR_DATA_SIZE_W;
	static const QString ATTR_ABS_ADDR;
	static const QString ATTR_ADDR_IN_BUF;
	static const QString ATTR_DATA_SIZE_BITS;

	static const QString CONN_TYPE_PORT_TO_PORT;
	static const QString CONN_TYPE_SINGLE_PORT;
};

#ifdef IS_BUILDER

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

#endif


