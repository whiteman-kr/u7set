#include "ConnectionsInfo.h"

// -----------------------------------------------------------------------------------
//
// ConnectionTxRxSignal implementation
//
// -----------------------------------------------------------------------------------

bool ConnectionTxRxSignal::load(const QDomElement& txRxSignalElem, QString* errMsg)
{
	bool result = true;

	result &= ConnectionsInfo::getStringAttribute(txRxSignalElem, ConnectionsInfo::ATTR_ID, &ID, errMsg);

	QString signalTypeStr;

	result &= ConnectionsInfo::getStringAttribute(txRxSignalElem, ConnectionsInfo::ATTR_TYPE, &signalTypeStr, errMsg);

	bool ok = false;

	type = E::stringToValue<E::SignalType>(signalTypeStr, &ok);

	if (ok == false)
	{
		*errMsg = ConnectionsInfo::errAttributeParsing(txRxSignalElem, ConnectionsInfo::ATTR_TYPE);
		return false;
	}

	result &= ConnectionsInfo::getAddress16Attribute(txRxSignalElem, ConnectionsInfo::ATTR_ADDR_IN_BUF, &addrInBuf, errMsg);
	result &= ConnectionsInfo::getAddress16Attribute(txRxSignalElem, ConnectionsInfo::ATTR_ABS_ADDR, &absAddr, errMsg);

	return result;
}

#ifdef IS_BUILDER

	void ConnectionTxRxSignal::save(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(ConnectionsInfo::ELEM_TX_RX_SIGNAL);

		xml.writeStringAttribute(ConnectionsInfo::ATTR_ID, ID);
		xml.writeStringAttribute(ConnectionsInfo::ATTR_TYPE, E::valueToString<E::SignalType>(type));
		xml.writeAddress16Attribute(ConnectionsInfo::ATTR_ADDR_IN_BUF, addrInBuf);
		xml.writeAddress16Attribute(ConnectionsInfo::ATTR_ABS_ADDR, absAddr);

		xml.writeEndElement();
	}

#endif

// -----------------------------------------------------------------------------------
//
// ConnectionPortInfo implementation
//
// -----------------------------------------------------------------------------------

bool ConnectionPortInfo::load(const QDomElement& connectionElement, int prtNo, QString* errMsg)
{
	assert(connectionElement.tagName() == ConnectionsInfo::ELEM_CONNECTION);
	assert(prtNo == 1 || prtNo == 2);

	portNo = prtNo;

	QDomElement portElem;

	bool result = true;

	result = ConnectionsInfo::getSingleChildElement(connectionElement, portTag(), &portElem, errMsg);

	if (result == false)
	{
		return false;
	}

	result &= ConnectionsInfo::getStringAttribute(portElem, ConnectionsInfo::ATTR_EQUIPMENT_ID, &equipmentID, errMsg);
	result &= ConnectionsInfo::getStringAttribute(portElem, ConnectionsInfo::ATTR_MODULE_ID, &moduleID, errMsg);

	if (result == false)
	{
		return false;
	}

	//

	QDomElement manSettingElem;

	result = ConnectionsInfo::getSingleChildElement(portElem, ConnectionsInfo::ELEM_MANUAL_SETTINGS, &manSettingElem, errMsg);

	if (result == false)
	{
		return false;
	}

	result &= ConnectionsInfo::getIntAttribute(manSettingElem, ConnectionsInfo::ATTR_RX_WORDS_QUANTITY, &manualRxWordsQuantity, errMsg);
	result &= ConnectionsInfo::getIntAttribute(manSettingElem, ConnectionsInfo::ATTR_TX_START_ADDRESS, &manualTxStartAddr, errMsg);
	result &= ConnectionsInfo::getIntAttribute(manSettingElem, ConnectionsInfo::ATTR_TX_WORDS_QUANTITY, &manualTxWordsQuantity, errMsg);

	if (result == false)
	{
		return false;
	}

	//

	QDomElement serialSettingElem;

	result = ConnectionsInfo::getSingleChildElement(portElem, ConnectionsInfo::ELEM_SERIAL_SETTINGS, &serialSettingElem, errMsg);

	if (result == false)
	{
		return false;
	}

	result &= ConnectionsInfo::getBoolAttribute(serialSettingElem, ConnectionsInfo::ATTR_ENABLE_SERIAL, &enableSerial, errMsg);
	result &= ConnectionsInfo::getBoolAttribute(serialSettingElem, ConnectionsInfo::ATTR_ENABLE_DUPLEX, &enableDuplex, errMsg);
	result &= ConnectionsInfo::getStringAttribute(serialSettingElem, ConnectionsInfo::ATTR_SERIAL_MODE, &serialMode, errMsg);

	if (result == false)
	{
		return false;
	}

	//

	{
		QDomElement txElem;

		result = ConnectionsInfo::getSingleChildElement(portElem, ConnectionsInfo::ELEM_TX, &txElem, errMsg);

		if (result == false)
		{
			return false;
		}

		result &= ConnectionsInfo::getIntAttribute(txElem, ConnectionsInfo::ATTR_BUFFER_ABS_ADDR, &txBufferAbsAddr, errMsg);
		result &= ConnectionsInfo::getIntAttribute(txElem, ConnectionsInfo::ATTR_DATA_SIZE_W, &txDataSizeW, errMsg);
		result &= ConnectionsInfo::getUInt32Attribute(txElem, ConnectionsInfo::ATTR_DATA_ID, &txDataID, errMsg);

		QDomNodeList txSignalsNodes = txElem.elementsByTagName(ConnectionsInfo::ELEM_TX_RX_SIGNAL);

		int txSignalsCount = txSignalsNodes.count();

		txSignals.resize(txSignalsCount);

		for(int i = 0; i < txSignalsCount; i++)
		{
			QDomElement txSignalElem = txSignalsNodes.item(i).toElement();

			result &= txSignals[i].load(txSignalElem, errMsg);
		}

		if (result == false)
		{
			return false;
		}
	}

	//

	{
		QDomElement rxElem;

		result = ConnectionsInfo::getSingleChildElement(portElem, ConnectionsInfo::ELEM_RX, &rxElem, errMsg);

		if (result == false)
		{
			return false;
		}

		result &= ConnectionsInfo::getIntAttribute(rxElem, ConnectionsInfo::ATTR_BUFFER_ABS_ADDR, &rxBufferAbsAddr, errMsg);
		result &= ConnectionsInfo::getIntAttribute(rxElem, ConnectionsInfo::ATTR_DATA_SIZE_W, &rxDataSizeW, errMsg);
		result &= ConnectionsInfo::getUInt32Attribute(rxElem,ConnectionsInfo::ATTR_DATA_ID, &rxDataID, errMsg);

		QDomElement rxValiditySignalElem;

		result = ConnectionsInfo::getSingleChildElement(portElem, ConnectionsInfo::ELEM_RX_VALIDITY_SIGNAL, &rxValiditySignalElem, errMsg);

		if (result == false)
		{
			return false;
		}

		result &= ConnectionsInfo::getStringAttribute(rxValiditySignalElem, ConnectionsInfo::ATTR_EQUIPMENT_ID, &rxValiditySignalEquipmentID, errMsg);
		result &= ConnectionsInfo::getAddress16Attribute(rxValiditySignalElem, ConnectionsInfo::ATTR_ABS_ADDR, &rxValiditySignalAbsAddr, errMsg);

		QDomNodeList rxSignalsNodes = rxElem.elementsByTagName(ConnectionsInfo::ELEM_TX_RX_SIGNAL);

		int rxSignalsCount = rxSignalsNodes.count();

		rxSignals.resize(rxSignalsCount);

		for(int i = 0; i < rxSignalsCount; i++)
		{
			QDomElement rxSignalElem = rxSignalsNodes.item(i).toElement();

			result &= rxSignals[i].load(rxSignalElem, errMsg);
		}
	}

	return result;
}

#ifdef IS_BUILDER

	bool ConnectionPortInfo::fill(Hardware::SharedConnection connection, int prtNo, const Hardware::OptoModuleStorage& optoModuleStorage)
	{
		if (connection == nullptr)
		{
			assert(false);
			return false;
		}

		portNo = prtNo;

		switch(portNo)
		{
		case 1:
			equipmentID = connection->port1EquipmentID();
			moduleID = optoModuleStorage.getOptoModuleID(connection->port1EquipmentID());

			manualRxWordsQuantity = connection->port1ManualRxWordsQuantity();
			manualTxStartAddr = connection->port1ManualTxStartAddress();
			manualTxWordsQuantity = connection->port1ManualTxWordsQuantity();

			enableSerial = connection->port1EnableSerial();
			enableDuplex = connection->port1EnableDuplex();
			serialMode = connection->port1SerialModeStr();

			break;

		case 2:
			equipmentID = connection->port2EquipmentID();
			moduleID = optoModuleStorage.getOptoModuleID(connection->port2EquipmentID());

			manualRxWordsQuantity = connection->port2ManualRxWordsQuantity();
			manualTxStartAddr = connection->port2ManualTxStartAddress();
			manualTxWordsQuantity = connection->port2ManualTxWordsQuantity();

			enableSerial = connection->port2EnableSerial();
			enableDuplex = connection->port2EnableDuplex();
			serialMode = connection->port2SerialModeStr();

			break;

		default:
			assert(false);
			return false;
		}

		Hardware::OptoPortShared optoPort = optoModuleStorage.getOptoPort(equipmentID);

		if (optoPort == nullptr)
		{
			assert(false);
			return false;
		}

		txBufferAbsAddr = optoPort->txBufAbsAddress();
		txDataSizeW = optoPort->txDataSizeW();
		txDataID = optoPort->txDataID();

		txSignals.reserve(optoPort->txSignals().count());

		for(Hardware::TxRxSignalShared txSignal : optoPort->txSignals())
		{
			if (txSignal == nullptr)
			{
				assert(false);
				continue;
			}

			ConnectionTxRxSignal txs;

			txs.ID = txSignal->appSignalID();
			txs.type = txSignal->signalType();
			txs.addrInBuf = txSignal->addrInBuf();
			txs.absAddr = txSignal->addrInBuf();
			txs.absAddr.addWord(txBufferAbsAddr);

			txSignals.push_back(txs);
		}

		//

		rxBufferAbsAddr = optoPort->rxBufAbsAddress();
		rxDataSizeW = optoPort->rxDataSizeW();
		rxDataID = optoPort->rxDataID();

		rxValiditySignalEquipmentID = optoPort->validitySignalEquipmentID();
		rxValiditySignalAbsAddr = optoPort->validitySignalAbsAddr();

		rxSignals.reserve(optoPort->rxSignals().count());

		for(Hardware::TxRxSignalShared rxSignal : optoPort->rxSignals())
		{
			if (rxSignal == nullptr)
			{
				assert(false);
				continue;
			}

			ConnectionTxRxSignal rxs;

			rxs.ID = rxSignal->appSignalID();
			rxs.type = rxSignal->signalType();
			rxs.addrInBuf = rxSignal->addrInBuf();
			rxs.absAddr = rxSignal->addrInBuf();
			rxs.absAddr.addWord(rxBufferAbsAddr);

			rxSignals.push_back(rxs);
		}

		return true;
	}

	void ConnectionPortInfo::save(XmlWriteHelper &xml) const
	{
		xml.writeStartElement(portTag());

		xml.writeStringAttribute(ConnectionsInfo::ATTR_EQUIPMENT_ID, equipmentID);
		xml.writeStringAttribute(ConnectionsInfo::ATTR_MODULE_ID, moduleID);

		{
			xml.writeStartElement(ConnectionsInfo::ELEM_MANUAL_SETTINGS);

			xml.writeIntAttribute(ConnectionsInfo::ATTR_RX_WORDS_QUANTITY, manualRxWordsQuantity);
			xml.writeIntAttribute(ConnectionsInfo::ATTR_TX_START_ADDRESS, manualTxStartAddr);
			xml.writeIntAttribute(ConnectionsInfo::ATTR_TX_WORDS_QUANTITY, manualTxWordsQuantity);

			xml.writeEndElement();
		}

		{
			xml.writeStartElement(ConnectionsInfo::ELEM_SERIAL_SETTINGS);

			xml.writeBoolAttribute(ConnectionsInfo::ATTR_ENABLE_SERIAL, enableSerial);
			xml.writeBoolAttribute(ConnectionsInfo::ATTR_ENABLE_DUPLEX, enableDuplex);
			xml.writeStringAttribute(ConnectionsInfo::ATTR_SERIAL_MODE, serialMode);

			xml.writeEndElement();
		}

		{
			xml.writeStartElement(ConnectionsInfo::ELEM_TX);

			xml.writeIntAttribute(ConnectionsInfo::ATTR_BUFFER_ABS_ADDR, txBufferAbsAddr);
			xml.writeIntAttribute(ConnectionsInfo::ATTR_DATA_SIZE_W, txDataSizeW);
			xml.writeUInt32Attribute(ConnectionsInfo::ATTR_DATA_ID, txDataID, false);
			xml.writeUInt32Attribute(ConnectionsInfo::ATTR_HEX_DATA_ID, txDataID, true);

			for(const ConnectionTxRxSignal& txSignal : txSignals)
			{
				txSignal.save(xml);
			}

			xml.writeEndElement();
		}

		{
			xml.writeStartElement(ConnectionsInfo::ELEM_RX);

			xml.writeIntAttribute(ConnectionsInfo::ATTR_BUFFER_ABS_ADDR, rxBufferAbsAddr);
			xml.writeIntAttribute(ConnectionsInfo::ATTR_DATA_SIZE_W, rxDataSizeW);
			xml.writeUInt32Attribute(ConnectionsInfo::ATTR_DATA_ID, rxDataID, false);
			xml.writeUInt32Attribute(ConnectionsInfo::ATTR_HEX_DATA_ID, rxDataID, true);

			{
				xml.writeStartElement(ConnectionsInfo::ELEM_RX_VALIDITY_SIGNAL);

				xml.writeStringAttribute(ConnectionsInfo::ATTR_EQUIPMENT_ID, rxValiditySignalEquipmentID);
				xml.writeAddress16Attribute(ConnectionsInfo::ATTR_ABS_ADDR, rxValiditySignalAbsAddr);

				xml.writeEndElement();
			}

			for(const ConnectionTxRxSignal& rxSignal : rxSignals)
			{
				rxSignal.save(xml);
			}

			xml.writeEndElement();
		}

		xml.writeEndElement();
	}

#endif

QString ConnectionPortInfo::portTag() const
{
	return QString("Port%1").arg(portNo);
}

// -----------------------------------------------------------------------------------
//
// ConnectionInfo implementation
//
// -----------------------------------------------------------------------------------

bool ConnectionInfo::load(const QDomNode& node, QString* errMsg)
{
	if (node.isElement() == false || node.nodeName() != ConnectionsInfo::ELEM_CONNECTION)
	{
		*errMsg = ConnectionsInfo::errElementNotFound(ConnectionsInfo::ELEM_CONNECTION);
		return false;
	}

	QDomElement elem = node.toElement();

	bool result = true;

	result &= ConnectionsInfo::getStringAttribute(elem, ConnectionsInfo::ATTR_ID, &ID, errMsg);
	result &= ConnectionsInfo::getIntAttribute(elem, ConnectionsInfo::ATTR_LINK_ID, &linkID, errMsg);
	result &= ConnectionsInfo::getStringAttribute(elem, ConnectionsInfo::ATTR_TYPE, &type, errMsg);
	result &= ConnectionsInfo::getBoolAttribute(elem, ConnectionsInfo::ATTR_ENABLE_MANUAL_SETTINGS, &enableManualSettings, errMsg);
	result &= ConnectionsInfo::getBoolAttribute(elem, ConnectionsInfo::ATTR_DISABLE_DATA_ID_CONTROL, &disableDataIDControl, errMsg);

	int portsCount = 0;

	result &= ConnectionsInfo::getIntAttribute(elem, ConnectionsInfo::ATTR_PORTS_COUNT, &portsCount, errMsg);

	if (result == false)
	{
		return false;
	}

	if (portsCount != 1 && portsCount != 2)
	{
		assert(false);
		*errMsg = QString("Wrong ports count in connection %1").arg(ID);
		return false;
	}

	ports.resize(portsCount);

	for(int i = 0; i < portsCount; i++)
	{
		result &= ports[i].load(elem, i + 1 /* portNo */, errMsg);
	}

	return result;
}

#ifdef IS_BUILDER

	bool ConnectionInfo::fill(Hardware::SharedConnection connection, const Hardware::OptoModuleStorage& optoModuleStorage)
	{
		if (connection == nullptr)
		{
			assert(false);
			return false;
		}

		ID = connection->connectionID();
		linkID = connection->linkID();
		type = connection->typeStr();
		enableManualSettings = connection->manualSettings();
		disableDataIDControl = connection->disableDataId();

		int portsCount = 0;

		if (type == ConnectionsInfo::CONN_TYPE_SINGLE_PORT)
		{
			portsCount = 1;
		}
		else
		{
			if (type == ConnectionsInfo::CONN_TYPE_PORT_TO_PORT)
			{
				portsCount = 2;
			}
			else
			{
				assert(false);
				return false;
			}
		}

		ports.resize(portsCount);

		for(int i = 0; i < portsCount; i++)
		{
			ports[i].fill(connection, i + 1, optoModuleStorage);
		}

		return true;
	}

	void ConnectionInfo::save(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(ConnectionsInfo::ELEM_CONNECTION);

		xml.writeStringAttribute(ConnectionsInfo::ATTR_ID, ID);
		xml.writeIntAttribute(ConnectionsInfo::ATTR_LINK_ID, linkID);
		xml.writeStringAttribute(ConnectionsInfo::ATTR_TYPE, type);
		xml.writeBoolAttribute(ConnectionsInfo::ATTR_ENABLE_MANUAL_SETTINGS, enableManualSettings);
		xml.writeBoolAttribute(ConnectionsInfo::ATTR_DISABLE_DATA_ID_CONTROL, disableDataIDControl);
		xml.writeIntAttribute(ConnectionsInfo::ATTR_PORTS_COUNT, static_cast<int>(ports.size()));

		for(const ConnectionPortInfo& port : ports)
		{
			port.save(xml);
		}

		xml.writeEndElement();
	}

#endif

// -----------------------------------------------------------------------------------
//
// ConnectionsInfo implementation
//
// -----------------------------------------------------------------------------------

const QString ConnectionsInfo::ELEM_CONNECTIONS("Connections");
const QString ConnectionsInfo::ELEM_CONNECTION("Connection");
const QString ConnectionsInfo::ELEM_MANUAL_SETTINGS("ManualSettings");
const QString ConnectionsInfo::ELEM_SERIAL_SETTINGS("SerialSettings");
const QString ConnectionsInfo::ELEM_TX("Tx");
const QString ConnectionsInfo::ELEM_TX_RX_SIGNAL("TxRxSignal");
const QString ConnectionsInfo::ELEM_RX("Rx");
const QString ConnectionsInfo::ELEM_RX_VALIDITY_SIGNAL("RxValiditySignal");

const QString ConnectionsInfo::ATTR_COUNT("Count");
const QString ConnectionsInfo::ATTR_ID("ID");
const QString ConnectionsInfo::ATTR_LINK_ID("LinkID");
const QString ConnectionsInfo::ATTR_TYPE("Type");
const QString ConnectionsInfo::ATTR_ENABLE_MANUAL_SETTINGS("EnableManualSettings");
const QString ConnectionsInfo::ATTR_DISABLE_DATA_ID_CONTROL("DisableDataIDControl");
const QString ConnectionsInfo::ATTR_PORTS_COUNT("PortsCount");
const QString ConnectionsInfo::ATTR_EQUIPMENT_ID("EquipmentID");
const QString ConnectionsInfo::ATTR_MODULE_ID("ModuleID");
const QString ConnectionsInfo::ATTR_RX_WORDS_QUANTITY("RxWordsQuantity");
const QString ConnectionsInfo::ATTR_TX_START_ADDRESS("TxStartAddress");
const QString ConnectionsInfo::ATTR_TX_WORDS_QUANTITY("TxWordsQuantity");
const QString ConnectionsInfo::ATTR_ENABLE_SERIAL("EnableSerial");
const QString ConnectionsInfo::ATTR_ENABLE_DUPLEX("EnableDuplex");
const QString ConnectionsInfo::ATTR_SERIAL_MODE("SerialMode");
const QString ConnectionsInfo::ATTR_BUFFER_ABS_ADDR("BufferAbsAddr");
const QString ConnectionsInfo::ATTR_DATA_SIZE_W("DataSizeW");
const QString ConnectionsInfo::ATTR_DATA_ID("DataID");
const QString ConnectionsInfo::ATTR_HEX_DATA_ID("HexDataID");
const QString ConnectionsInfo::ATTR_ABS_ADDR("AbsAddr");
const QString ConnectionsInfo::ATTR_ADDR_IN_BUF("AddrInBuf");

const QString ConnectionsInfo::CONN_TYPE_PORT_TO_PORT("PortToPort");
const QString ConnectionsInfo::CONN_TYPE_SINGLE_PORT("SinglePort");

bool ConnectionsInfo::load(const QString& fileName, QString* errMsg)
{
	if (errMsg == nullptr)
	{
		assert(false);
		return false;
	}

	QFile file(fileName);

	if(file.open(QIODevice::ReadOnly) == false)
	{
		*errMsg = QString("File open error");
		return false;
	}

	QByteArray xmlData = file.readAll();

	if (xmlData.size() != QFileInfo(file).size())
	{
		*errMsg = QString("File read error");
		return false;
	}

	file.close();

	return load(xmlData, errMsg);
}

bool ConnectionsInfo::load(const QByteArray& xmlData, QString* errMsg)
{
	if (errMsg == nullptr)
	{
		assert(false);
		return false;
	}

	QDomDocument xmlDoc;

	QString parsingError;
	int errLine = 0;
	int errColumn = 0;

	bool result = xmlDoc.setContent(xmlData, false, &parsingError, &errLine, &errColumn);

	if (result == false)
	{
		*errMsg = QString("%1, line %2, column %3").arg(parsingError).arg(errLine).arg(errColumn);
		return false;
	}

	connections.clear();

	QDomElement connectionsElem = xmlDoc.documentElement();

	if (connectionsElem.isNull() == true || connectionsElem.tagName() != ConnectionsInfo::ELEM_CONNECTIONS)
	{
		*errMsg = errElementNotFound(ConnectionsInfo::ELEM_CONNECTIONS);
		return false;
	}

	int connectionsCount = 0;

	result = getIntAttribute(connectionsElem, ATTR_COUNT, &connectionsCount, errMsg);

	if (result == false)
	{
		return false;
	}

	QDomNodeList connectionNodes = connectionsElem.elementsByTagName(ConnectionsInfo::ELEM_CONNECTION);

	if (connectionNodes.count() != connectionsCount)
	{
		*errMsg = QString("File corruption! Count of Conection nodes is not equal to Connections Count attribute value");
		return false;
	}

	connections.resize(connectionsCount);

	for(int i = 0; i < connectionNodes.count(); i++)
	{
		bool res = connections[i].load(connectionNodes.item(i), errMsg);

		if (res == false)
		{
			return false;
		}
	}

	return true;
}

#ifdef IS_BUILDER

	bool ConnectionsInfo::fill(const Hardware::ConnectionStorage& connectionsStorage, const Hardware::OptoModuleStorage& optoModuleStorage)
	{
		bool result = true;

		std::vector<Hardware::SharedConnection> conns = connectionsStorage.getConnections();

		std::map<QString, Hardware::SharedConnection> connectionsSet;

		for(Hardware::SharedConnection connection : conns)
		{
			if (connection == nullptr)
			{
				assert(false);
				continue;
			}

			connectionsSet.insert(std::pair<QString, Hardware::SharedConnection>(connection->connectionID(), connection));
		}

		connections.clear();

		for(const std::pair<QString, Hardware::SharedConnection> p : connectionsSet)
		{
			Hardware::SharedConnection connection = p.second;

			ConnectionInfo ci;

			ci.fill(connection, optoModuleStorage);

			connections.push_back(ci);
		}

		return result;
	}

	void ConnectionsInfo::save(QByteArray* xmlFileData) const
	{
		if (xmlFileData == nullptr)
		{
			assert(false);
			return;
		}

		XmlWriteHelper xml(xmlFileData);

		xml.setAutoFormatting(true);
		xml.writeStartDocument();

		{
			xml.writeStartElement(ConnectionsInfo::ELEM_CONNECTIONS);

			xml.writeIntAttribute(ConnectionsInfo::ATTR_COUNT, static_cast<int>(connections.size()));

			for(const ConnectionInfo& connection : connections)
			{
				connection.save(xml);
			}

			xml.writeEndElement();
		}

		xml.writeEndDocument();
	}

#endif


QString ConnectionsInfo::errElementNotFound(const QString& elemName)
{
	return QString("Element is not found: %1").arg(elemName);
}

QString ConnectionsInfo::errAttributeNotFound(const QDomElement& elem, const QString& attrName)
{
	return QString("Attribute is not found: %1 (element %2)").arg(attrName).arg(elem.tagName());
}

QString ConnectionsInfo::errAttributeParsing(const QDomElement& elem, const QString& attrName)
{
	return QString("Attribute parsing error: %1 (element %2)").arg(attrName).arg(elem.tagName());
}

bool ConnectionsInfo::getSingleChildElement(const QDomElement& parentElement, const QString& childElementTagName,
											QDomElement* childElem, QString* errMsg)
{
	if (errMsg == nullptr)
	{
		assert(false);
		return false;
	}

	if (childElem == nullptr)
	{
		assert(false);
		*errMsg = "Nullptr!";
		return false;
	}

	QDomNodeList nodes = parentElement.elementsByTagName(childElementTagName);

	if (nodes.count() == 0)
	{
		*errMsg = QString("Child element %1 not found in parent element %2").
						arg(childElementTagName).
						arg(parentElement.tagName());
		return false;
	}

	if (nodes.count() > 1)
	{
		*errMsg = QString("More than one child element %1 in parent element %2").
					arg(childElementTagName).
					arg(parentElement.tagName());
		return false;
	}

	*childElem = nodes.item(0).toElement();

	return true;
}

bool ConnectionsInfo::getIntAttribute(const QDomElement& elem, const QString& attrName, int* value, QString* errMsg)
{
	if (value == nullptr)
	{
		assert(false);
		*errMsg = "Nullptr!";
		return false;
	}

	if (elem.hasAttribute(attrName) == false)
	{
		*errMsg = errAttributeNotFound(elem, attrName);
		return false;
	}

	QString attrValue = elem.attribute(attrName);

	bool ok = false;

	*value = attrValue.toInt(&ok);

	if (ok == false)
	{
		*errMsg = errAttributeParsing(elem, attrName);
		return false;
	}

	return true;
}

bool ConnectionsInfo::getStringAttribute(const QDomElement& elem, const QString& attrName, QString* value, QString* errMsg)
{
	if (value == nullptr)
	{
		assert(false);
		*errMsg = "Nullptr!";
		return false;
	}

	if (elem.hasAttribute(attrName) == false)
	{
		*errMsg = errAttributeNotFound(elem, attrName);
		return false;
	}

	*value = elem.attribute(attrName);

	return true;
}

bool ConnectionsInfo::getBoolAttribute(const QDomElement& elem, const QString& attrName, bool* value, QString* errMsg)
{
	if (value == nullptr)
	{
		assert(false);
		*errMsg = "Nullptr!";
		return false;
	}

	if (elem.hasAttribute(attrName) == false)
	{
		*errMsg = errAttributeNotFound(elem, attrName);
		return false;
	}

	QString attrValue = elem.attribute(attrName);

	if (attrValue == "true")
	{
		*value = true;
		return true;
	}

	if (attrValue == "false")
	{
		*value = false;
		return true;
	}

	*errMsg = errAttributeParsing(elem, attrName);
	return false;
}

bool ConnectionsInfo::getAddress16Attribute(const QDomElement& elem, const QString& attrName, Address16* value, QString* errMsg)
{
	if (value == nullptr)
	{
		assert(false);
		*errMsg = "Nullptr!";
		return false;
	}

	if (elem.hasAttribute(attrName) == false)
	{
		*errMsg = errAttributeNotFound(elem, attrName);
		return false;
	}

	QString attrValue = elem.attribute(attrName);

	bool ok = false;

	value->fromString(attrValue, &ok);

	if (ok == false)
	{
		*errMsg = errAttributeParsing(elem, attrName);
		return false;
	}

	return true;
}

bool ConnectionsInfo::getUInt32Attribute(const QDomElement& elem, const QString& attrName, quint32* value, QString* errMsg)
{
	if (value == nullptr)
	{
		assert(false);
		*errMsg = "Nullptr!";
		return false;
	}

	if (elem.hasAttribute(attrName) == false)
	{
		*errMsg = errAttributeNotFound(elem, attrName);
		return false;
	}

	QString attrValue = elem.attribute(attrName);

	bool ok = false;

	*value = attrValue.toULong(&ok, 0);

	if (ok == false)
	{
		*errMsg = errAttributeParsing(elem, attrName);
		return false;
	}

	return true;
}

/*
void testConnInfoLoad()
{
	ConnectionsInfo ci;

	QString err;

	bool res = ci.load(QString("d:/temp/connections-debug/build/common/connections.xml"), &err);

	if (res == false)
	{
		qDebug() << err;
	}
	else
	{
		qDebug() << "OK";
	}
}*/
