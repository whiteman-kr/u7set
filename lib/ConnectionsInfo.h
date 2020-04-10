#pragma once

#ifndef IS_SIMULATOR

#include "Connection.h"
#include "../Builder/OptoModule.h"
#include "XmlHelper.h"

#endif

#include <QDomDocument>
#include "Address16.h"
#include "Types.h"


class ConnectionTxRxSignal
{
public:
	QString ID;
	E::SignalType type = E::SignalType::Discrete;
	Address16 addrInBuf;
	Address16 absAddr;

private:
	bool load(const QDomElement& txRxSignalElem, QString* errMsg);

#ifndef IS_SIMULATOR

	void save(XmlWriteHelper& xml) const;

#endif

	friend class ConnectionPortInfo;
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

private:
	bool load(const QDomElement& connectionElement, int prtNo, QString* errMsg);

#ifndef IS_SIMULATOR

	bool fill(Hardware::SharedConnection connection, int prtNo, const Hardware::OptoModuleStorage& optoModuleStorage);
	void save(XmlWriteHelper& xml) const;

#endif

	QString portTag() const;

	friend class ConnectionInfo;
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

private:

	bool load(const QDomNode& node, QString* errMsg);

#ifndef IS_SIMULATOR

	bool fill(Hardware::SharedConnection connection, const Hardware::OptoModuleStorage& optoModuleStorage);
	void save(XmlWriteHelper& xml) const;

#endif

	friend class ConnectionsInfo;

};

class ConnectionsInfo
{
public:
	std::vector<ConnectionInfo> connections;

	bool load(const QString& fileName, QString* errMsg);
	bool load(const QByteArray& xmlData, QString* errMsg);

#ifndef IS_SIMULATOR

	bool fill(const Hardware::ConnectionStorage& connectionsStorage, const Hardware::OptoModuleStorage& optoModuleStorage);
	void save(QByteArray* xmlFileData) const;

#endif

private:
	static QString errElementNotFound(const QString& elemName);
	static QString errAttributeNotFound(const QDomElement& elem, const QString& attrName);
	static QString errAttributeParsing(const QDomElement& elem, const QString& attrName);

	static bool getSingleChildElement(const QDomElement& parentElement, const QString& childElementTagName,
									  QDomElement* childElem, QString* errMsg);

	static bool getIntAttribute(const QDomElement& elem, const QString& attrName, int* value, QString* errMsg);
	static bool getStringAttribute(const QDomElement& elem, const QString& attrName, QString* value, QString* errMsg);
	static bool getBoolAttribute(const QDomElement& elem, const QString& attrName, bool* value, QString* errMsg);
	static bool getAddress16Attribute(const QDomElement& elem, const QString& attrName, Address16* value, QString* errMsg);
	static bool getUInt32Attribute(const QDomElement& elem, const QString& attrName, quint32* value, QString* errMsg);

	static const QString ELEM_CONNECTIONS;
	static const QString ELEM_CONNECTION;
	static const QString ELEM_MANUAL_SETTINGS;
	static const QString ELEM_SERIAL_SETTINGS;
	static const QString ELEM_TX;
	static const QString ELEM_TX_RX_SIGNAL;
	static const QString ELEM_RX;
	static const QString ELEM_RX_VALIDITY_SIGNAL;

	static const QString ATTR_COUNT;
	static const QString ATTR_ID;
	static const QString ATTR_LINK_ID;
	static const QString ATTR_TYPE;
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
	static const QString ATTR_DATA_ID;
	static const QString ATTR_HEX_DATA_ID;
	static const QString ATTR_ABS_ADDR;
	static const QString ATTR_ADDR_IN_BUF;

	static const QString CONN_TYPE_PORT_TO_PORT;
	static const QString CONN_TYPE_SINGLE_PORT;

	friend class ConnectionTxRxSignal;
	friend class ConnectionPortInfo;
	friend class ConnectionInfo;
};

//extern void testConnInfoLoad();

