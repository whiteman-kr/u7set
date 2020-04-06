#include "ConnectionsInfo.h"

QString ConnectionPortInfo::portTag() const
{
	return QString("Port%1").arg(portNo);
}

#ifndef IS_SIMULATOR

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
			manualTxStartAddress = connection->port1ManualTxStartAddress();
			manualTxWordsQuantiy = connection->port1ManualTxWordsQuantity();

			enableSerial = connection->port1EnableSerial();
			enableDuplex = connection->port1EnableDuplex();
			serialMode = connection->port1SerialModeStr();

			break;

		case 2:
			equipmentID = connection->port2EquipmentID();
			moduleID = optoModuleStorage.getOptoModuleID(connection->port2EquipmentID());

			manualRxWordsQuantity = connection->port2ManualRxWordsQuantity();
			manualTxStartAddress = connection->port2ManualTxStartAddress();
			manualTxWordsQuantiy = connection->port2ManualTxWordsQuantity();

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

		txBufferAbsAddressW = optoPort->txBufAbsAddress();
		txDataSizeW = optoPort->txDataSizeW();
		txDataID = optoPort->txDataID();

		//

		rxBufferAbsAddressW = optoPort->rxBufAbsAddress();
		rxDataSizeW = optoPort->rxDataSizeW();
		rxDataID = optoPort->rxDataID();

		rxValiditySignalID = optoPort->validitySignalID();
		rxValiditySignalAddressW = optoPort->validitySignalAbsAddr().offset();
		rxValiditySignalBit = optoPort->validitySignalAbsAddr().bit();

		return true;
	}

	bool ConnectionPortInfo::save(QDomDocument& doc, QDomElement& connectionElement) const
	{
		QDomElement portElement = doc.createElement(portTag());

		portElement.setAttribute("EquipmentID", equipmentID);
		portElement.setAttribute("ModuleID", moduleID);

		QDomElement manualSettingsElement = doc.createElement("ManualSettings");

		manualSettingsElement.setAttribute("RxWordsQuantity", manualRxWordsQuantity);
		manualSettingsElement.setAttribute("TxStartAddress", manualTxStartAddress);
		manualSettingsElement.setAttribute("TxWordsQuantity", manualTxWordsQuantiy);

		portElement.appendChild(manualSettingsElement);

		QDomElement serialSettingsElement = doc.createElement("SerialSettings");

		serialSettingsElement.setAttribute("EnableSerial", enableSerial);
		serialSettingsElement.setAttribute("EnableDuplex", enableDuplex);
		serialSettingsElement.setAttribute("SerialMode", serialMode);

		portElement.appendChild(serialSettingsElement);

		QDomElement txElement = doc.createElement("Tx");

		txElement.setAttribute("BufferAbsAddrW", txBufferAbsAddressW);
		txElement.setAttribute("DataSizeW", txDataSizeW);
		txElement.setAttribute("DataID", txDataID);
		txElement.setAttribute("HexDataID", QString().setNum(txDataID, 16).toUpper().rightJustified(8, '0'));

		portElement.appendChild(txElement);

		QDomElement rxElement = doc.createElement("Rx");

		rxElement.setAttribute("BufferAbsAddrW", rxBufferAbsAddressW);
		rxElement.setAttribute("DataSizeW", rxDataSizeW);
		rxElement.setAttribute("DataID", rxDataID);
		rxElement.setAttribute("HexDataID", QString().setNum(rxDataID, 16).toUpper().rightJustified(8, '0'));

		QDomElement validitySignalElement = doc.createElement("RxValiditySignal");

		validitySignalElement.setAttribute("ID", rxValiditySignalID);
		validitySignalElement.setAttribute("AddressW", rxValiditySignalAddressW);
		validitySignalElement.setAttribute("BitNo", rxValiditySignalBit);

		rxElement.appendChild(validitySignalElement);

		portElement.appendChild(rxElement);


		connectionElement.appendChild(portElement);

		return true;
	}

#endif


#ifndef IS_SIMULATOR

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
		disableDataIdControl = connection->disableDataId();

		port1.fill(connection, 1, optoModuleStorage);

		if (type == "PortToPort")
		{
			port2.fill(connection, 2, optoModuleStorage);
		}

		return true;
	}


	bool ConnectionInfo::save(QDomDocument& doc, QDomElement& connectionsElement) const
	{
		bool result = true;

		QDomElement connectionElement = doc.createElement("Connection");

		connectionElement.setAttribute("ID", ID);
		connectionElement.setAttribute("LinkID", linkID);
		connectionElement.setAttribute("Type", type);
		connectionElement.setAttribute("EnableManualSettings", enableManualSettings);
		connectionElement.setAttribute("DisableDataIdControl", disableDataIdControl);

		port1.save(doc, connectionElement);

		if (type == "PortToPort")
		{
			port2.save(doc, connectionElement);
		}

		connectionsElement.appendChild(connectionElement);

		return result;
	}

#endif

/*
bool ConnectionInfo::load(const XmlReadHelper& xml, QString* errMsg)
{
	if (errMsg == nullptr)
	{
		assert(false);
		return false;
	}

	bool result = true;
	return result;
}*/


#ifndef IS_SIMULATOR

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

		QDomDocument doc;

		QDomProcessingInstruction pocessInstruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");

		doc.appendChild(pocessInstruction);

		QDomElement connectionsElem = doc.createElement("Connections");

		connectionsElem.setAttribute("Count", connections.size());

		for(const ConnectionInfo& connection : connections)
		{
			connection.save(doc, connectionsElem);
		}

		doc.appendChild(connectionsElem);

		*xmlFileData = doc.toByteArray(4);
	}

#endif

bool ConnectionsInfo::load(const QString fileName, QString* errMsg)
{
	bool result = true;
	return result;
}


