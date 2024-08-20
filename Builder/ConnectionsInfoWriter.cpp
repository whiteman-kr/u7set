#include <CommonLib/ConstStrings.h>
#include "ConnectionsInfoWriter.h"


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
	ci->typeStr = connection->typeStr();
	ci->type = connection->type();
	ci->enableManualSettings = connection->manualSettings();
	ci->disableDataIDControl = connection->disableDataId();

	int portsCount = 0;

	if (ci->type == Hardware::Connection::Type::SinglePort)
	{
		portsCount = 1;
	}
	else
	{
		if (ci->type == Hardware::Connection::Type::PortToPort)
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
	xml.writeStringAttribute(ConnectionsInfo::ATTR_TYPE, ci.typeStr);
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

	cpi->txBufferAbsAddr = optoPort->txBufAddress();
	cpi->txDataSizeW = optoPort->txDataSizeW();
	cpi->txDataID = optoPort->txDataID();

	cpi->txSignals.reserve(optoPort->txSignals().size());

	for(Hardware::TxRxSignalShared txSignal : optoPort->txSignals())
	{
		TEST_PTR_CONTINUE(txSignal);

		ConnectionTxRxSignal txs;

		txs.IDs = txSignal->appSignalIDs();
		txs.type = txSignal->signalType();
		txs.analogFormat = txSignal->analogFormat();
		txs.busTypeID = txSignal->busTypeID();
		txs.addrInBuf = txSignal->addrInBuf();
		txs.absAddr = txSignal->addrInBuf();
		txs.absAddr.addWord(cpi->txBufferAbsAddr);
		txs.dataSizeBits = txSignal->dataSize();

		cpi->txSignals.push_back(txs);
	}

	//

	cpi->rxBufferAbsAddr = optoPort->rxBufAddress();
	cpi->rxDataSizeW = optoPort->rxDataSizeW();
	cpi->rxDataID = optoPort->rxDataID();

	cpi->rxValiditySignalEquipmentID = optoPort->validitySignalEquipmentID();
	cpi->rxValiditySignalAbsAddr = optoPort->validitySignalAbsAddr();

	cpi->rxSignals.reserve(optoPort->rxSignals().count());

	for(Hardware::TxRxSignalShared rxSignal : optoPort->rxSignals())
	{
		TEST_PTR_CONTINUE(rxSignal);

		ConnectionTxRxSignal rxs;

		rxs.IDs = rxSignal->appSignalIDs();
		rxs.type = rxSignal->signalType();
		rxs.analogFormat = rxSignal->analogFormat();
		rxs.busTypeID = rxSignal->busTypeID();
		rxs.addrInBuf = rxSignal->addrInBuf();
		rxs.absAddr = rxSignal->addrInBuf();
		rxs.absAddr.addWord(cpi->rxBufferAbsAddr);
		rxs.dataSizeBits = rxSignal->dataSize();

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

	QString ids = cs.IDs.join(Separator::SEMICOLON);

	xml.writeStringAttribute(XmlAttribute::IDs, ids);
	xml.writeStringAttribute(ConnectionsInfo::ATTR_TYPE, E::valueToString<E::SignalType>(cs.type));
	xml.writeStringAttribute(ConnectionsInfo::ATTR_ANALOG_FORMAT,
							 E::valueToString<E::AnalogAppSignalFormat>(cs.analogFormat));
	xml.writeStringAttribute(ConnectionsInfo::ATTR_BUS_TYPE_ID, cs.busTypeID);
	xml.writeAddress16Attribute(ConnectionsInfo::ATTR_ADDR_IN_BUF, cs.addrInBuf);
	xml.writeAddress16Attribute(ConnectionsInfo::ATTR_ABS_ADDR, cs.absAddr);
	xml.writeIntAttribute(ConnectionsInfo::ATTR_DATA_SIZE_BITS, cs.dataSizeBits);

	xml.writeEndElement();
}
