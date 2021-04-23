#include "ConnectionsInfo.h"

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

const QString ConnectionsInfo::ATTR_LINK_ID("LinkID");
const QString ConnectionsInfo::ATTR_TYPE("Type");
const QString ConnectionsInfo::ATTR_ENABLE_MANUAL_SETTINGS("EnableManualSettings");
const QString ConnectionsInfo::ATTR_DISABLE_DATA_ID_CONTROL("DisableDataIDControl");
const QString ConnectionsInfo::ATTR_PORTS_COUNT("PortsCount");
const QString ConnectionsInfo::ATTR_EQUIPMENT_ID("EquipmentID");
const QString ConnectionsInfo::ATTR_MODULE_ID("ModuleID");
const QString ConnectionsInfo::ATTR_LM_ID("LmID");
const QString ConnectionsInfo::ATTR_RX_WORDS_QUANTITY("RxWordsQuantity");
const QString ConnectionsInfo::ATTR_TX_START_ADDRESS("TxStartAddress");
const QString ConnectionsInfo::ATTR_TX_WORDS_QUANTITY("TxWordsQuantity");
const QString ConnectionsInfo::ATTR_ENABLE_SERIAL("EnableSerial");
const QString ConnectionsInfo::ATTR_ENABLE_DUPLEX("EnableDuplex");
const QString ConnectionsInfo::ATTR_SERIAL_MODE("SerialMode");
const QString ConnectionsInfo::ATTR_BUFFER_ABS_ADDR("BufferAbsAddr");
const QString ConnectionsInfo::ATTR_DATA_SIZE_W("DataSizeW");
const QString ConnectionsInfo::ATTR_ABS_ADDR("AbsAddr");
const QString ConnectionsInfo::ATTR_ADDR_IN_BUF("AddrInBuf");

const QString ConnectionsInfo::CONN_TYPE_PORT_TO_PORT("PortToPort");
const QString ConnectionsInfo::CONN_TYPE_SINGLE_PORT("SinglePort");

bool ConnectionsInfo::load(const QString& fileName, QString* errMsg)
{
	if (errMsg == nullptr)
	{
		Q_ASSERT(false);
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
		Q_ASSERT(false);
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
		*errMsg = DomXmlHelper::errElementNotFound(ConnectionsInfo::ELEM_CONNECTIONS);
		return false;
	}

	int connectionsCount = 0;

	result = DomXmlHelper::getIntAttribute(connectionsElem, XmlAttribute::COUNT, &connectionsCount, errMsg);

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
		bool res = load(&connections[i], connectionNodes.item(i), errMsg);

		if (res == false)
		{
			return false;
		}
	}

	return true;
}

bool ConnectionsInfo::load(ConnectionInfo* ci, const QDomNode& node, QString* errMsg)
{
	if (ci == nullptr)
	{
		Q_ASSERT(false);
		return false;
	}

	if (node.isElement() == false || node.nodeName() != ConnectionsInfo::ELEM_CONNECTION)
	{
		*errMsg = DomXmlHelper::errElementNotFound(ConnectionsInfo::ELEM_CONNECTION);
		return false;
	}

	QDomElement elem = node.toElement();

	bool result = true;

	result &= DomXmlHelper::getStringAttribute(elem, XmlAttribute::ID, &ci->ID, errMsg);
	result &= DomXmlHelper::getIntAttribute(elem, ConnectionsInfo::ATTR_LINK_ID, &ci->linkID, errMsg);
	result &= DomXmlHelper::getStringAttribute(elem, ConnectionsInfo::ATTR_TYPE, &ci->type, errMsg);
	result &= DomXmlHelper::getBoolAttribute(elem, ConnectionsInfo::ATTR_ENABLE_MANUAL_SETTINGS, &ci->enableManualSettings, errMsg);
	result &= DomXmlHelper::getBoolAttribute(elem, ConnectionsInfo::ATTR_DISABLE_DATA_ID_CONTROL, &ci->disableDataIDControl, errMsg);

	int portsCount = 0;

	result &= DomXmlHelper::getIntAttribute(elem, ConnectionsInfo::ATTR_PORTS_COUNT, &portsCount, errMsg);

	if (result == false)
	{
		return false;
	}

	if (portsCount != 1 && portsCount != 2)
	{
		assert(false);
		*errMsg = QString("Wrong ports count in connection %1").arg(ci->ID);
		return false;
	}

	ci->ports.resize(portsCount);

	for(int i = 0; i < portsCount; i++)
	{
		result &= load(&ci->ports[i], elem, i + 1 /* portNo */, errMsg);
	}

	return result;
}

bool ConnectionsInfo::load(ConnectionPortInfo* cpi, const QDomElement& connectionElement, int prtNo, QString* errMsg)
{
	if (cpi == nullptr)
	{
		Q_ASSERT(false);
		return false;
	}

	Q_ASSERT(connectionElement.tagName() == ConnectionsInfo::ELEM_CONNECTION);
	Q_ASSERT(prtNo == 1 || prtNo == 2);

	cpi->portNo = prtNo;

	QDomElement portElem;

	bool result = true;

	result = DomXmlHelper::getSingleChildElement(connectionElement, portTag(prtNo), &portElem, errMsg);

	if (result == false)
	{
		return false;
	}

	result &= DomXmlHelper::getStringAttribute(portElem, ConnectionsInfo::ATTR_EQUIPMENT_ID, &cpi->equipmentID, errMsg);
	result &= DomXmlHelper::getStringAttribute(portElem, ConnectionsInfo::ATTR_MODULE_ID, &cpi->moduleID, errMsg);
	result &= DomXmlHelper::getStringAttribute(portElem, ConnectionsInfo::ATTR_LM_ID, &cpi->lmID, errMsg);

	if (result == false)
	{
		return false;
	}

	//

	QDomElement manSettingElem;

	result = DomXmlHelper::getSingleChildElement(portElem, ConnectionsInfo::ELEM_MANUAL_SETTINGS, &manSettingElem, errMsg);

	if (result == false)
	{
		return false;
	}

	result &= DomXmlHelper::getIntAttribute(manSettingElem, ConnectionsInfo::ATTR_RX_WORDS_QUANTITY, &cpi->manualRxWordsQuantity, errMsg);
	result &= DomXmlHelper::getIntAttribute(manSettingElem, ConnectionsInfo::ATTR_TX_START_ADDRESS, &cpi->manualTxStartAddr, errMsg);
	result &= DomXmlHelper::getIntAttribute(manSettingElem, ConnectionsInfo::ATTR_TX_WORDS_QUANTITY, &cpi->manualTxWordsQuantity, errMsg);

	if (result == false)
	{
		return false;
	}

	//

	QDomElement serialSettingElem;

	result = DomXmlHelper::getSingleChildElement(portElem, ConnectionsInfo::ELEM_SERIAL_SETTINGS, &serialSettingElem, errMsg);

	if (result == false)
	{
		return false;
	}

	result &= DomXmlHelper::getBoolAttribute(serialSettingElem, ConnectionsInfo::ATTR_ENABLE_SERIAL, &cpi->enableSerial, errMsg);
	result &= DomXmlHelper::getBoolAttribute(serialSettingElem, ConnectionsInfo::ATTR_ENABLE_DUPLEX, &cpi->enableDuplex, errMsg);
	result &= DomXmlHelper::getStringAttribute(serialSettingElem, ConnectionsInfo::ATTR_SERIAL_MODE, &cpi->serialMode, errMsg);

	if (result == false)
	{
		return false;
	}

	//

	{
		QDomElement txElem;

		result = DomXmlHelper::getSingleChildElement(portElem, ConnectionsInfo::ELEM_TX, &txElem, errMsg);

		if (result == false)
		{
			return false;
		}

		result &= DomXmlHelper::getIntAttribute(txElem, ConnectionsInfo::ATTR_BUFFER_ABS_ADDR, &cpi->txBufferAbsAddr, errMsg);
		result &= DomXmlHelper::getIntAttribute(txElem, ConnectionsInfo::ATTR_DATA_SIZE_W, &cpi->txDataSizeW, errMsg);
		result &= DomXmlHelper::getUInt32Attribute(txElem, XmlAttribute::DATA_ID, &cpi->txDataID, errMsg);

		QDomNodeList txSignalsNodes = txElem.elementsByTagName(ConnectionsInfo::ELEM_TX_RX_SIGNAL);

		int txSignalsCount = txSignalsNodes.count();

		cpi->txSignals.resize(txSignalsCount);

		for(int i = 0; i < txSignalsCount; i++)
		{
			QDomElement txSignalElem = txSignalsNodes.item(i).toElement();

			result &= load(&cpi->txSignals[i], txSignalElem, errMsg);
		}

		if (result == false)
		{
			return false;
		}
	}

	//

	{
		QDomElement rxElem;

		result = DomXmlHelper::getSingleChildElement(portElem, ConnectionsInfo::ELEM_RX, &rxElem, errMsg);

		if (result == false)
		{
			return false;
		}

		result &= DomXmlHelper::getIntAttribute(rxElem, ConnectionsInfo::ATTR_BUFFER_ABS_ADDR, &cpi->rxBufferAbsAddr, errMsg);
		result &= DomXmlHelper::getIntAttribute(rxElem, ConnectionsInfo::ATTR_DATA_SIZE_W, &cpi->rxDataSizeW, errMsg);
		result &= DomXmlHelper::getUInt32Attribute(rxElem, XmlAttribute::DATA_ID, &cpi->rxDataID, errMsg);

		QDomElement rxValiditySignalElem;

		result = DomXmlHelper::getSingleChildElement(portElem, ConnectionsInfo::ELEM_RX_VALIDITY_SIGNAL, &rxValiditySignalElem, errMsg);

		if (result == false)
		{
			return false;
		}

		result &= DomXmlHelper::getStringAttribute(rxValiditySignalElem, ConnectionsInfo::ATTR_EQUIPMENT_ID, &cpi->rxValiditySignalEquipmentID, errMsg);
		result &= DomXmlHelper::getAddress16Attribute(rxValiditySignalElem, ConnectionsInfo::ATTR_ABS_ADDR, &cpi->rxValiditySignalAbsAddr, errMsg);

		QDomNodeList rxSignalsNodes = rxElem.elementsByTagName(ConnectionsInfo::ELEM_TX_RX_SIGNAL);

		int rxSignalsCount = rxSignalsNodes.count();

		cpi->rxSignals.resize(rxSignalsCount);

		for(int i = 0; i < rxSignalsCount; i++)
		{
			QDomElement rxSignalElem = rxSignalsNodes.item(i).toElement();

			result &= load(&cpi->rxSignals[i], rxSignalElem, errMsg);
		}
	}

	return result;
}

bool ConnectionsInfo::load(ConnectionTxRxSignal* cs, const QDomElement& txRxSignalElem, QString* errMsg)
{
	if (cs == nullptr)
	{
		Q_ASSERT(false);
		return false;
	}

	bool result = true;

	result &= DomXmlHelper::getStringAttribute(txRxSignalElem, XmlAttribute::ID, &cs->ID, errMsg);

	QString signalTypeStr;

	result &= DomXmlHelper::getStringAttribute(txRxSignalElem, ConnectionsInfo::ATTR_TYPE, &signalTypeStr, errMsg);

	bool ok = false;

	cs->type = E::stringToValue<E::SignalType>(signalTypeStr, &ok);

	if (ok == false)
	{
		*errMsg = DomXmlHelper::errAttributeParsing(txRxSignalElem, ConnectionsInfo::ATTR_TYPE);
		return false;
	}

	result &= DomXmlHelper::getAddress16Attribute(txRxSignalElem, ConnectionsInfo::ATTR_ADDR_IN_BUF, &cs->addrInBuf, errMsg);
	result &= DomXmlHelper::getAddress16Attribute(txRxSignalElem, ConnectionsInfo::ATTR_ABS_ADDR, &cs->absAddr, errMsg);

	return result;
}

QString ConnectionsInfo::portTag(int portNo)
{
	Q_ASSERT(portNo == 1 || portNo == 2);

	return QString("Port%1").arg(portNo);
}

#ifdef IS_BUILDER

	// -----------------------------------------------------------------------------------
	//
	// ConnectionsInfoWriter implementation
	//
	// -----------------------------------------------------------------------------------

	bool ConnectionsInfoWriter::fill(const Builder::ConnectionStorage& connectionsStorage, const Hardware::OptoModuleStorage& optoModuleStorage)
	{
		bool result = true;

		std::vector<Hardware::SharedConnection> conns = connectionsStorage.getConnections();

		std::map<QString, Hardware::SharedConnection> connectionsSet;

		for(Hardware::SharedConnection connection : conns)
		{
			TEST_PTR_CONTINUE(connection);

			connectionsSet.insert(std::pair<QString, Hardware::SharedConnection>(connection->connectionID(), connection));
		}

		connections.clear();

		for(const std::pair<QString, Hardware::SharedConnection> p : connectionsSet)
		{
			Hardware::SharedConnection connection = p.second;

			ConnectionInfo ci;

			result &= fill(&ci, connection, optoModuleStorage);

			connections.push_back(ci);
		}

		return result;
	}

	void ConnectionsInfoWriter::save(QByteArray* xmlFileData) const
	{
		TEST_PTR_RETURN(xmlFileData);

		XmlWriteHelper xml(xmlFileData);

		xml.setAutoFormatting(true);
		xml.writeStartDocument();

		{
			xml.writeStartElement(ConnectionsInfo::ELEM_CONNECTIONS);

			xml.writeIntAttribute(XmlAttribute::COUNT, static_cast<int>(connections.size()));

			for(const ConnectionInfo& connectionInfo : connections)
			{
				save(connectionInfo, xml);
			}

			xml.writeEndElement();
		}

		xml.writeEndDocument();
	}

	bool ConnectionsInfoWriter::fill(ConnectionInfo* ci, Hardware::SharedConnection connection,
									 const Hardware::OptoModuleStorage& optoModuleStorage)
	{
		TEST_PTR_RETURN_FALSE(ci);
		TEST_PTR_RETURN_FALSE(connection);

		bool result = true;

		ci->ID = connection->connectionID();
		ci->linkID = connection->linkID();
		ci->type = connection->typeStr();
		ci->enableManualSettings = connection->manualSettings();
		ci->disableDataIDControl = connection->disableDataId();

		int portsCount = 0;

		if (ci->type == ConnectionsInfo::CONN_TYPE_SINGLE_PORT)
		{
			portsCount = 1;
		}
		else
		{
			if (ci->type == ConnectionsInfo::CONN_TYPE_PORT_TO_PORT)
			{
				portsCount = 2;
			}
			else
			{
				assert(false);
				return false;
			}
		}

		ci->ports.clear();

		for(int i = 0; i < portsCount; i++)
		{
			ConnectionPortInfo cpi;

			result &= fill(&cpi, connection, i + 1, optoModuleStorage);

			RETURN_IF_FALSE(result);

			ci->ports.push_back(cpi);
		}

		return result;
	}

	void ConnectionsInfoWriter::save(const ConnectionInfo& ci, XmlWriteHelper& xml) const
	{
		xml.writeStartElement(ConnectionsInfo::ELEM_CONNECTION);

		xml.writeStringAttribute(XmlAttribute::ID, ci.ID);
		xml.writeIntAttribute(ConnectionsInfo::ATTR_LINK_ID, ci.linkID);
		xml.writeStringAttribute(ConnectionsInfo::ATTR_TYPE, ci.type);
		xml.writeBoolAttribute(ConnectionsInfo::ATTR_ENABLE_MANUAL_SETTINGS, ci.enableManualSettings);
		xml.writeBoolAttribute(ConnectionsInfo::ATTR_DISABLE_DATA_ID_CONTROL, ci.disableDataIDControl);
		xml.writeIntAttribute(ConnectionsInfo::ATTR_PORTS_COUNT, static_cast<int>(ci.ports.size()));

		for(const ConnectionPortInfo& port : ci.ports)
		{
			save(port, xml);
		}

		xml.writeEndElement();
	}

	bool ConnectionsInfoWriter::fill(ConnectionPortInfo* cpi, Hardware::SharedConnection connection, int prtNo,
								  const Hardware::OptoModuleStorage& optoModuleStorage)
	{
		TEST_PTR_RETURN_FALSE(cpi);
		TEST_PTR_RETURN_FALSE(connection);

		cpi->portNo = prtNo;

		switch(prtNo)
		{
		case 1:
			cpi->equipmentID = connection->port1EquipmentID();

			cpi->manualRxWordsQuantity = connection->port1ManualRxWordsQuantity();
			cpi->manualTxStartAddr = connection->port1ManualTxStartAddress();
			cpi->manualTxWordsQuantity = connection->port1ManualTxWordsQuantity();

			cpi->enableSerial = connection->port1EnableSerial();
			cpi->enableDuplex = connection->port1EnableDuplex();
			cpi->serialMode = connection->port1SerialModeStr();

			break;

		case 2:
			cpi->equipmentID = connection->port2EquipmentID();

			cpi->manualRxWordsQuantity = connection->port2ManualRxWordsQuantity();
			cpi->manualTxStartAddr = connection->port2ManualTxStartAddress();
			cpi->manualTxWordsQuantity = connection->port2ManualTxWordsQuantity();

			cpi->enableSerial = connection->port2EnableSerial();
			cpi->enableDuplex = connection->port2EnableDuplex();
			cpi->serialMode = connection->port2SerialModeStr();

			break;

		default:
			assert(false);
			return false;
		}

		cpi->moduleID = optoModuleStorage.getOptoModuleID(cpi->equipmentID);
		cpi->lmID = optoModuleStorage.getOptoPortAssociatedLmID(cpi->equipmentID);

		Hardware::OptoPortShared optoPort = optoModuleStorage.getOptoPort(cpi->equipmentID);

		TEST_PTR_RETURN_FALSE(optoPort);

		cpi->txBufferAbsAddr = optoPort->txBufAbsAddress();
		cpi->txDataSizeW = optoPort->txDataSizeW();
		cpi->txDataID = optoPort->txDataID();

		cpi->txSignals.reserve(optoPort->txSignals().count());

		for(Hardware::TxRxSignalShared txSignal : optoPort->txSignals())
		{
			TEST_PTR_CONTINUE(txSignal);

			ConnectionTxRxSignal txs;

			txs.ID = txSignal->appSignalID();
			txs.type = txSignal->signalType();
			txs.addrInBuf = txSignal->addrInBuf();
			txs.absAddr = txSignal->addrInBuf();
			txs.absAddr.addWord(cpi->txBufferAbsAddr);

			cpi->txSignals.push_back(txs);
		}

		//

		cpi->rxBufferAbsAddr = optoPort->rxBufAbsAddress();
		cpi->rxDataSizeW = optoPort->rxDataSizeW();
		cpi->rxDataID = optoPort->rxDataID();

		cpi->rxValiditySignalEquipmentID = optoPort->validitySignalEquipmentID();
		cpi->rxValiditySignalAbsAddr = optoPort->validitySignalAbsAddr();

		cpi->rxSignals.reserve(optoPort->rxSignals().count());

		for(Hardware::TxRxSignalShared rxSignal : optoPort->rxSignals())
		{
			TEST_PTR_CONTINUE(rxSignal);

			ConnectionTxRxSignal rxs;

			rxs.ID = rxSignal->appSignalID();
			rxs.type = rxSignal->signalType();
			rxs.addrInBuf = rxSignal->addrInBuf();
			rxs.absAddr = rxSignal->addrInBuf();
			rxs.absAddr.addWord(cpi->rxBufferAbsAddr);

			cpi->rxSignals.push_back(rxs);
		}

		return true;
	}

	void ConnectionsInfoWriter::save(const ConnectionPortInfo& cpi, XmlWriteHelper &xml) const
	{
		xml.writeStartElement(portTag(cpi.portNo));

		xml.writeStringAttribute(ConnectionsInfo::ATTR_EQUIPMENT_ID, cpi.equipmentID);
		xml.writeStringAttribute(ConnectionsInfo::ATTR_MODULE_ID, cpi.moduleID);
		xml.writeStringAttribute(ConnectionsInfo::ATTR_LM_ID, cpi.lmID);

		{
			xml.writeStartElement(ConnectionsInfo::ELEM_MANUAL_SETTINGS);

			xml.writeIntAttribute(ConnectionsInfo::ATTR_RX_WORDS_QUANTITY, cpi.manualRxWordsQuantity);
			xml.writeIntAttribute(ConnectionsInfo::ATTR_TX_START_ADDRESS, cpi.manualTxStartAddr);
			xml.writeIntAttribute(ConnectionsInfo::ATTR_TX_WORDS_QUANTITY, cpi.manualTxWordsQuantity);

			xml.writeEndElement();
		}

		{
			xml.writeStartElement(ConnectionsInfo::ELEM_SERIAL_SETTINGS);

			xml.writeBoolAttribute(ConnectionsInfo::ATTR_ENABLE_SERIAL, cpi.enableSerial);
			xml.writeBoolAttribute(ConnectionsInfo::ATTR_ENABLE_DUPLEX, cpi.enableDuplex);
			xml.writeStringAttribute(ConnectionsInfo::ATTR_SERIAL_MODE, cpi.serialMode);

			xml.writeEndElement();
		}

		{
			xml.writeStartElement(ConnectionsInfo::ELEM_TX);

			xml.writeIntAttribute(ConnectionsInfo::ATTR_BUFFER_ABS_ADDR, cpi.txBufferAbsAddr);
			xml.writeIntAttribute(ConnectionsInfo::ATTR_DATA_SIZE_W, cpi.txDataSizeW);
			xml.writeUInt32Attribute(XmlAttribute::DATA_ID, cpi.txDataID, false);
			xml.writeUInt32Attribute(XmlAttribute::HEX_DATA_ID, cpi.txDataID, true);

			for(const ConnectionTxRxSignal& txSignal : cpi.txSignals)
			{
				save(txSignal, xml);
			}

			xml.writeEndElement();
		}

		{
			xml.writeStartElement(ConnectionsInfo::ELEM_RX);

			xml.writeIntAttribute(ConnectionsInfo::ATTR_BUFFER_ABS_ADDR, cpi.rxBufferAbsAddr);
			xml.writeIntAttribute(ConnectionsInfo::ATTR_DATA_SIZE_W, cpi.rxDataSizeW);
			xml.writeUInt32Attribute(XmlAttribute::DATA_ID, cpi.rxDataID, false);
			xml.writeUInt32Attribute(XmlAttribute::HEX_DATA_ID, cpi.rxDataID, true);

			{
				xml.writeStartElement(ConnectionsInfo::ELEM_RX_VALIDITY_SIGNAL);

				xml.writeStringAttribute(ConnectionsInfo::ATTR_EQUIPMENT_ID, cpi.rxValiditySignalEquipmentID);
				xml.writeAddress16Attribute(ConnectionsInfo::ATTR_ABS_ADDR, cpi.rxValiditySignalAbsAddr);

				xml.writeEndElement();
			}

			for(const ConnectionTxRxSignal& rxSignal : cpi.rxSignals)
			{
				save(rxSignal, xml);
			}

			xml.writeEndElement();
		}

		xml.writeEndElement();
	}

	void ConnectionsInfoWriter::save(const ConnectionTxRxSignal& cs, XmlWriteHelper& xml) const
	{
		xml.writeStartElement(ConnectionsInfo::ELEM_TX_RX_SIGNAL);

		xml.writeStringAttribute(XmlAttribute::ID, cs.ID);
		xml.writeStringAttribute(ConnectionsInfo::ATTR_TYPE, E::valueToString<E::SignalType>(cs.type));
		xml.writeAddress16Attribute(ConnectionsInfo::ATTR_ADDR_IN_BUF, cs.addrInBuf);
		xml.writeAddress16Attribute(ConnectionsInfo::ATTR_ABS_ADDR, cs.absAddr);

		xml.writeEndElement();
	}

#endif


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
