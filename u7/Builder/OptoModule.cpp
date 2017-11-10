#include "OptoModule.h"

#include "../LogicModule.h"
#include "../Builder/Builder.h"
#include "../Builder/ApplicationLogicCompiler.h"
#include "../lib/DeviceHelper.h"


namespace Hardware
{

	// --------------------------------------------------------------------------------------
	//
	// TxRxSignal class impementation
	//
	// --------------------------------------------------------------------------------------

	TxRxSignal::TxRxSignal()
	{
	}

	bool TxRxSignal::init(const Builder::UalSignal* ualSignal)
	{
		if (ualSignal == nullptr)
		{
			assert(false);
			return false;
		}

		m_appSignalIDs = ualSignal->refSignalIDs();
		m_signalType = ualSignal->signalType();
		m_byteOrder = ualSignal->byteOrder();

		switch(m_signalType)
		{
		case E::SignalType::Analog:
		case E::SignalType::Discrete:

			m_dataFormat = ualSignal->dataFormat();
			m_dataSize = ualSignal->dataSize();
			break;

		case E::SignalType::Bus:

			if (ualSignal->bus() == nullptr)
			{
				assert(false);
				return false;
			}

			m_dataFormat = E::DataFormat::UnsignedInt;
			m_dataSize = ualSignal->bus()->sizeBit();
			m_busTypeID = ualSignal->bus()->busTypeID();
			break;

		default:
			assert(false);		// wtf?
			return false;
		}

		m_type = TxRxSignal::Type::Regular;

		return true;
	}

	bool TxRxSignal::initRawSignal(const RawDataDescriptionItem& item, int offsetFromBeginningOfBuffer)
	{
		if (item.type != RawDataDescriptionItem::Type::TxSignal &&
			item.type != RawDataDescriptionItem::Type::RxSignal)
		{
			ASSERT_RETURN_FALSE;
		}

		if (hasSignalID(item.appSignalID) == false)
		{
			ASSERT_RETURN_FALSE
		}

		m_type = TxRxSignal::Type::Raw;

		m_signalType = item.signalType;
		m_dataFormat = item.dataFormat;
		m_byteOrder = item.byteOrder;
		m_dataSize = item.dataSize;
		m_addrInBuf.set(item.offsetW + offsetFromBeginningOfBuffer, item.bitNo);
		m_busTypeID = item.busTypeID;

		return true;
	}

	bool TxRxSignal::hasSignalID(const QString& signalID)
	{
		for(const QString& appSignalID : m_appSignalIDs)
		{
			if (appSignalID == signalID)
			{
				return true;
			}
		}

		return false;
	}

	void TxRxSignal::setAddrInBuf(const Address16& addr)
	{
		m_addrInBuf = addr;
	}


	// --------------------------------------------------------------------------------------
	//
	// OptoPort class implementation
	//
	// --------------------------------------------------------------------------------------

	OptoPort::OptoPort()
	{
	}

	bool OptoPort::init(const DeviceController* controller, int portNo, LogicModule* lmDescription, Builder::IssueLogger* log)
	{
		if (controller == nullptr ||
			log == nullptr ||
			lmDescription == nullptr)
		{
			assert(false);
			return false;
		}

		m_controller = controller;
		m_portNo = portNo;
		m_log = log;

		m_equipmentID = m_controller->equipmentIdTemplate();

		const DeviceModule* module = m_controller->getParentModule();

		if (module == nullptr)
		{
			assert(false);
			return false;
		}

		m_optoModuleID = module->equipmentIdTemplate();

		const DeviceModule* lm = DeviceHelper::getAssociatedLmOrBvb(m_controller);

		if (lm == nullptr)
		{
			assert(false);
			return false;
		}

		m_lmID = lm->equipmentIdTemplate();

		// opto port validity signal finding
		//
		DeviceController* platformInterface = DeviceHelper::getPlatformInterfaceController(module, m_log);

		if (platformInterface == nullptr)
		{
			return false;
		}

		QString optoPortValiditySignalSuffix = QString("_OPTOPORT0%1VALID").arg(portNo);

		const DeviceObject* validityObject = DeviceHelper::getChildDeviceObjectBySuffix(platformInterface,
																			  optoPortValiditySignalSuffix,
																			  m_log);
		if (validityObject == nullptr)
		{
			return false;
		}

		const DeviceSignal* validitySignal = validityObject->toSignal();

		if (validitySignal == nullptr)
		{
			assert(false);
			return false;
		}

		m_validitySignalID = validitySignal->equipmentIdTemplate();

		if (validitySignal->memoryArea() != E::MemoryArea::DiagnosticsData)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		Address16 addr(validitySignal->valueOffset(), validitySignal->valueBit());

		if (module->isLogicModule() == true || module->isBvb() == true)
		{
			addr.addWord(lmDescription->memory().m_txDiagDataOffset);
		}
		else
		{
			if (module->isOptoModule() == true)
			{
				addr.addWord(lmDescription->memory().m_moduleDataOffset +
							 lmDescription->memory().m_moduleDataSize * (module->place() - 1));

				int txDiagDataOffset = BAD_ADDRESS;

				bool res = DeviceHelper::getIntProperty(module, "TxDiagDataOffset", &txDiagDataOffset, m_log);

				if (res == false)
				{
					return false;
				}

				if (txDiagDataOffset == BAD_ADDRESS)
				{
					assert(false);
					return false;
				}

				addr.addWord(txDiagDataOffset);
			}
			else
			{
				// unknown opto module
				//
				assert(false);
				return false;
			}
		}

		m_validitySignalAbsAddr = addr;

		return true;
	}

	bool OptoPort::initSettings(ConnectionShared cn)
	{
		if (cn == nullptr)
		{
			ASSERT_RETURN_FALSE;
		}

		bool isFirstPort = false;

		if (cn->port1EquipmentID() == m_equipmentID)
		{
			isFirstPort = true;
		}
		else
		{
			if (cn->isSinglePort() == true)
			{
				ASSERT_RETURN_FALSE;
			}

			if (cn->port2EquipmentID() == m_equipmentID)
			{
				isFirstPort = false;
			}
			else
			{
				ASSERT_RETURN_FALSE;
			}
		}

		if (isFirstPort == true)
		{
			// this port is the first port in connection
			//
			setLinkID(cn->linkID());
			setConnectionType(cn->type());
			setEnableSerial(cn->port1EnableSerial());
			setSerialMode(cn->port1SerialMode());
			setEnableDuplex(cn->port1EnableDuplex());

			setManualSettings(cn->manualSettings());
			setManualTxStartAddressW(cn->port1ManualTxStartAddress());
			setManualTxSizeW(cn->port1ManualTxWordsQuantity());
			setManualRxSizeW(cn->port1ManualRxWordsQuantity());
			setRawDataDescriptionStr(cn->port1RawDataDescription());

			if (cn->isPortToPort() == true)
			{
				setLinkedPortID(cn->port2EquipmentID());
			}
			else
			{
				setLinkedPortID("");
			}
		}
		else
		{
			// this port is the second port in connection
			//
			setLinkID(cn->linkID());
			setConnectionType(cn->type());
			setEnableSerial(cn->port2EnableSerial());
			setSerialMode(cn->port2SerialMode());
			setEnableDuplex(cn->port2EnableDuplex());

			setManualSettings(cn->manualSettings());
			setManualTxStartAddressW(cn->port2ManualTxStartAddress());
			setManualTxSizeW(cn->port2ManualTxWordsQuantity());
			setManualRxSizeW(cn->port2ManualRxWordsQuantity());
			setRawDataDescriptionStr(cn->port2RawDataDescription());

			assert(cn->isPortToPort() == true);

			setLinkedPortID(cn->port1EquipmentID());
		}

		bool res = parseRawDescription();

		return res;
	}

	bool OptoPort::initRawTxSignals()
	{
		bool result = true;

		for(const RawDataDescriptionItem& rawTxSignal : m_rawTxSignals)
		{
			TxRxSignalShared txSignal = m_txSignalIDs.value(rawTxSignal.appSignalID, nullptr);

			if (txSignal == nullptr)
			{
				// Tx signal '%1' specified in port '%2' raw data description isn't connected to transmitter (Connection '%3').
				//
				m_log->errALC5192(rawTxSignal.appSignalID, m_equipmentID, m_connectionID);
				result = false;
			}
			else
			{
				txSignal->initRawSignal(rawTxSignal, TX_DATA_ID_SIZE_W);
			}
		}

		return result;
	}

	bool OptoPort::sortTxSignals()
	{
		return sortTxRxSignalList(m_txSignals);
	}

	bool OptoPort::calculateTxSignalsAddresses()
	{
		// initial txSignals addresses calculcation
		// zero-offset from port txStartAddress
		//
		m_txDataSizeW = 0;
		m_txAnalogSignalsSizeW = 0;
		m_txBusSignalsSizeW = 0;
		m_txDiscreteSignalsSizeW = 0;

		if (isUsedInConnection() == false)
		{
			return true;
		}

		Address16 address(0, 0);

		// m_txDataID first placed in buffer
		//
		address.addWord(TX_DATA_ID_SIZE_W);

		// then place Raw Data
		//

		bool res = initRawTxSignals();

		if (res == false)
		{
			return false;
		}

		address.addWord(m_txRawDataSizeW);

		bool result = true;

		for(TxRxSignalShared& txRawSignal : m_txSignals)
		{
			if (txRawSignal->isRaw() == true)
			{
				if (txRawSignal->addrInBuf().bitAddress() + txRawSignal->dataSize() > address.bitAddress())
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
									   QString(tr("Raw tx signal '%1' out of raw data area size (port '%2', connection '%3'')")).
									   arg(txRawSignal->appSignalID()).
									   arg(m_equipmentID).arg(m_connectionID));
					result = false;
				}
			}
		}

		if (result == false)
		{
			return false;
		}

		int startAddr = address.offset();

		// then place analog signals
		//
		for(TxRxSignalShared& txAnalogSignal : m_txSignals)
		{
			if (txAnalogSignal->isRaw() == true || txAnalogSignal->isAnalog() == false)
			{
				continue;
			}

			txAnalogSignal->setAddrInBuf(address);

			address.addBit(txAnalogSignal->dataSize());
			address.wordAlign();
		}

		m_txAnalogSignalsSizeW = address.offset() - startAddr ;

		startAddr = address.offset();

		// then place bus signals
		//
		for(TxRxSignalShared& txBusSignal : m_txSignals)
		{
			if (txBusSignal->isRaw() == true || txBusSignal->isBus() == false)
			{
				continue;
			}

			txBusSignal->setAddrInBuf(address);

			address.addBit(txBusSignal->dataSize());

			assert(address.bit() == 0);

			address.wordAlign();
		}

		m_txBusSignalsSizeW = address.offset() - startAddr ;

		startAddr = address.offset();

		// then place discrete signals
		//
		for(TxRxSignalShared& txDiscreteSignal : m_txSignals)
		{
			if (txDiscreteSignal->isRaw() == true || txDiscreteSignal->isDiscrete() == false)
			{
				continue;
			}

			txDiscreteSignal->setAddrInBuf(address);

			assert(txDiscreteSignal->dataSize() == SIZE_1BIT);

			address.add1Bit();
		}

		address.wordAlign();

		m_txDiscreteSignalsSizeW = address.offset() - startAddr;

		m_txUsedDataSizeW = TX_DATA_ID_SIZE_W + m_txRawDataSizeW + m_txAnalogSignalsSizeW +
										m_txBusSignalsSizeW + m_txDiscreteSignalsSizeW;

		if (manualSettings() == true)
		{
			if (m_txUsedDataSizeW > m_manualTxSizeW)
			{
				LOG_MESSAGE(m_log, QString(tr("Port %1: txIdSizeW = %2, txRawDataSizeW = %3, txAnalogSignalsSizeW = %4, txBusSignalsSizeW = %5, txDiscreteSignalsSizeW = %6")).
									arg(m_equipmentID).arg(TX_DATA_ID_SIZE_W).arg(m_txRawDataSizeW).arg(m_txAnalogSignalsSizeW).arg(m_txBusSignalsSizeW).arg(m_txDiscreteSignalsSizeW));
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
								   QString(tr("Manual txDataSizeW - %1 less then needed size %2 (port %3, connection %4)")).
								   arg(m_manualTxSizeW).arg(m_txUsedDataSizeW).
								   arg(m_equipmentID).arg(m_connectionID));

				return false;
			}

			m_txDataSizeW = m_manualTxSizeW;
		}
		else
		{
			m_txDataSizeW = m_txUsedDataSizeW;
		}

		return true;
	}

	bool OptoPort::calculateTxDataID()
	{
		m_txDataID = 0;

		if (isUsedInConnection() == false)
		{
			return true;
		}

		m_txDataID = CRC32_INITIAL_VALUE;

		// mix port EquipmentID in dataID
		//
		m_txDataID = CRC32(m_txDataID, C_STR(m_equipmentID), m_equipmentID.length(), false);

		// mix txSignals ID and bitAddress in buffer
		//
		for(const TxRxSignalShared& txSignal : m_txSignals)
		{
			m_txDataID = CRC32(m_txDataID, txSignal->appSignalID(), false);
			m_txDataID = CRC32(m_txDataID, txSignal->addrInBuf().bitAddress(), false);
		}

		m_txDataID = CRC32(m_txDataID, nullptr, 0, true);       // finalize CRC32 calculation

		return true;
	}

	bool OptoPort::appendSerialRxSignal(const Builder::UalSignal* rxSignal)
	{
		TEST_PTR_RETURN_FALSE(rxSignal);

		if (isSinglePortConnection() == false)
		{
			assert(false);				// port is not Serial!
			return false;
		}

		if (m_rxSignalIDs.contains(rxSignal->appSignalID()) == true)
		{
			return true;				// signal already in list, nothing to do
		}

		bool result = appendRxSignal(rxSignal);

		return result;
	}

	bool OptoPort::initSerialRawRxSignals()
	{
		if (isSinglePortConnection() == false)
		{
			return true;
		}

		bool result = true;

		for(const RawDataDescriptionItem& rawRxSignal : m_rawRxSignals)
		{
			TxRxSignalShared rxSignal = m_rxSignalIDs.value(rawRxSignal.appSignalID, nullptr);

			if (rxSignal == nullptr)
			{
				// Rx signal '%1' specified in port '%2' raw data description isn't assigned to receiver (Connection '%3').
				//
				m_log->errALC5193(rawRxSignal.appSignalID, m_equipmentID, m_connectionID);
				result = false;
				continue;
			}

			rxSignal->initRawSignal(rawRxSignal, TX_DATA_ID_SIZE_W);
		}

		return result;
	}

	bool OptoPort::sortSerialRxSignals()
	{
		if (isSinglePortConnection() == false)
		{
			// on this step of execution m_rxSignals of Optical ports must be empty!
			//
			assert(m_rxSignals.count() == 0);
			return true;
		}

		return sortTxRxSignalList(m_rxSignals);
	}

	bool OptoPort::calculateSerialRxSignalsAddresses()
	{
		if (isUsedInConnection() == false)
		{
			return true;
		}

		if (isSinglePortConnection() == false)
		{
			return true;				// process Serial ports only
		}

		m_rxDataSizeW = 0;
		m_rxAnalogSignalsSizeW = 0;
		m_rxBusSignalsSizeW = 0;
		m_rxDiscreteSignalsSizeW = 0;

		if (m_rxRawDataSizeW == 0 && m_rxSignals.count() == 0)
		{
			if (manualSettings() == true)
			{
				m_rxDataSizeW = m_manualRxSizeW;
			}

			return true;
		}

		// check raw signals addresses
		//
		for(TxRxSignalShared& rxSignal : m_rxSignals)
		{
			if (rxSignal->isRaw() == false)
			{
				continue;
			}

			if (rxSignal->addrInBuf().offset() < 0 ||
				rxSignal->addrInBuf().offset() >= m_rxRawDataSizeW + OptoPort::TX_DATA_ID_SIZE_W)
			{
				assert(false);			// address out of range of raw data area
				return false;
			}
		}

		Address16 address(0, 0);

		// m_rxDataID first placed in buffer
		//
		address.addWord(TX_DATA_ID_SIZE_W);

		// then place Raw Data
		//
		address.addWord(m_rxRawDataSizeW);

		int startAddr = address.offset();

		// then place analog rx signals
		//
		for(TxRxSignalShared& rxAnalogSignal : m_rxSignals)
		{
			if (rxAnalogSignal->isRaw() == true || rxAnalogSignal->isAnalog() == false)
			{
				continue;					// exclude raw signals
			}

			rxAnalogSignal->setAddrInBuf(address);

			address.addBit(rxAnalogSignal->dataSize());
			address.wordAlign();
		}

		m_rxAnalogSignalsSizeW = address.offset() - startAddr ;

		startAddr = address.offset();

		// then place bus rx signals
		//
		for(TxRxSignalShared& rxBusSignal : m_rxSignals)
		{
			if (rxBusSignal->isRaw() == true || rxBusSignal->isBus() == false)
			{
				continue;					// exclude raw signals
			}

			rxBusSignal->setAddrInBuf(address);

			address.addBit(rxBusSignal->dataSize());
			address.wordAlign();
		}

		m_rxAnalogSignalsSizeW = address.offset() - startAddr ;

		startAddr = address.offset();

		// then place discrete signals
		//
		for(TxRxSignalShared& rxDiscreteSignal : m_rxSignals)
		{
			if (rxDiscreteSignal->isRaw() == true || rxDiscreteSignal->isDiscrete() == false)
			{
				continue;					// exclude raw signals
			}

			rxDiscreteSignal->setAddrInBuf(address);

			assert(rxDiscreteSignal->dataSize() == SIZE_1BIT);

			address.add1Bit();
		}

		address.wordAlign();

		m_rxDiscreteSignalsSizeW = address.offset() - startAddr;

		m_rxUsedDataSizeW = TX_DATA_ID_SIZE_W + m_rxRawDataSizeW + m_rxAnalogSignalsSizeW + m_rxDiscreteSignalsSizeW;

		if (manualSettings() == true)
		{
			if (m_rxUsedDataSizeW > m_manualRxSizeW)
			{
				LOG_MESSAGE(m_log, QString(tr("Port %1: rxIdSizeW = %2, rxRawDataSizeW = %3, rxAnalogSignalsSizeW = %4, rxDiscreteSignalsSizeW = %5")).
									arg(m_equipmentID).arg(TX_DATA_ID_SIZE_W).arg(m_txRawDataSizeW).arg(m_txAnalogSignalsSizeW).arg(m_txDiscreteSignalsSizeW));
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
								   QString(tr("Manual rxDataSizeW - %1 less then needed size %2 (port %3, connection %4)")).
								   arg(m_manualRxSizeW).arg(m_rxUsedDataSizeW).
								   arg(m_equipmentID).arg(m_connectionID));

				return false;
			}

			m_rxDataSizeW = m_manualRxSizeW;
		}
		else
		{
			m_rxDataSizeW = m_rxUsedDataSizeW;
		}

		return true;
	}

	bool OptoPort::calculateSerialRxDataID()
	{
		m_rxDataID = 0;

		if (isUsedInConnection() == false)
		{
			return true;
		}

		if (isSinglePortConnection() == false)
		{
			return true;
		}

		// rxDataID should be calculate for serial ports only!
		//
		m_rxDataID = CRC32_INITIAL_VALUE;

		// mix port EquipmentID in dataID
		//
		m_rxDataID = CRC32(m_rxDataID, C_STR(m_equipmentID), m_equipmentID.length(), false);

		// mix rxSignals ID and bitAddress in buffer
		//
		for(const TxRxSignalShared& rxSignal : m_txSignals)
		{
			m_rxDataID = CRC32(m_rxDataID, rxSignal->appSignalID(), false);
			m_rxDataID = CRC32(m_rxDataID, rxSignal->addrInBuf().bitAddress(), false);
		}

		m_rxDataID = CRC32(m_rxDataID, nullptr, 0, true);       // finalize CRC32 calculation

		return true;
	}

	bool OptoPort::copyOpticalPortsTxInRxSignals()
	{
		if (isUsedInConnection() == false)
		{
			return true;
		}

		if (isPortToPortConnection() == false)
		{
			return true;
		}

		assert(m_rxSignals.count() == 0);				// before this m_rxSignals must be clear!

		OptoPortShared linkedPort = OptoModuleStorage::getOptoPort(m_linkedPortID);

		qDebug() << "This port " << m_equipmentID;

		if (linkedPort == nullptr)
		{
			ASSERT_RETURN_FALSE;
		}

		// copying txSignals of linked port to rxSignals of current port
		//
		const QVector<TxRxSignalShared>& txSignals = linkedPort->txSignals();

		for(const TxRxSignalShared txSignal : txSignals)
		{
			if (txSignal == nullptr)
			{
				ASSERT_RETURN_FALSE;
			}

			m_rxSignals.append(txSignal);

			for(const QString& appSignalID : txSignal->appSignalIDs())
			{
				m_rxSignalIDs.insert(appSignalID, txSignal);
			}
		}

		// copying Tx to Rx parameters
		//
		m_rxDataID = linkedPort->txDataID();
		m_rxDataSizeW = linkedPort->txDataSizeW();
		m_rxUsedDataSizeW = linkedPort->txUsedDataSizeW();
		m_rxRawDataSizeW = linkedPort->txRawDataSizeW();
		m_rxAnalogSignalsSizeW = linkedPort->txAnalogSignalsSizeW();
		m_rxBusSignalsSizeW = linkedPort->txBusSignalsSizeW();
		m_rxDiscreteSignalsSizeW = linkedPort->txDiscreteSignalsSizeW();

		return true;
	}

	bool OptoPort::writeSerialDataXml(Builder::BuildResultWriter* resultWriter)
	{
		TEST_PTR_RETURN_FALSE(resultWriter);

		if (isSinglePortConnection() == false)
		{
			assert(false);
			return false;
		}

		return true;

/*		if (optoModule == nullptr ||
			rs232Port == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			assert(false);
			return false;
		}

		QByteArray xmlData;
		QXmlStreamWriter xmlWriter(&xmlData);

		xmlWriter.setAutoFormatting(true);
		xmlWriter.writeStartDocument();
		xmlWriter.writeStartElement("SerialData");

		m_resultWriter->buildInfo().writeToXml(xmlWriter);

		xmlWriter.writeStartElement("PortInfo");

		xmlWriter.writeAttribute("StrID", rs232Port->equipmentID());
		xmlWriter.writeAttribute("ID", QString::number(rs232Port->linkID()));
		xmlWriter.writeAttribute("DataID", QString::number(rs232Port->txDataID()));
		xmlWriter.writeAttribute("Speed", "115200");
		xmlWriter.writeAttribute("Bits", "8");
		xmlWriter.writeAttribute("StopBits", "2");
		xmlWriter.writeAttribute("ParityControl", "false");
		xmlWriter.writeAttribute("DataSize", QString::number(rs232Port->txDataSizeW()));

		xmlWriter.writeEndElement();	// </PortInfo>

		xmlWriter.writeStartElement("Signals");

		xmlWriter.writeAttribute("Count", QString::number(rs232Port->txSignalsCount()));

		bool result = true;

		m_code.newLine();

		Comment comment(QString(tr("Copy signals to RS232/485 port %1, connection - %2")).
						arg(rs232Port->equipmentID()).arg(rs232Port->connectionID()));

		m_code.append(comment);

		m_code.newLine();

		int portDataAddress = rs232Port->txBufAbsAddress();
		// write data UID
		//
		Command cmd;

		cmd.movConstInt32(portDataAddress, rs232Port->txDataID());

		QString hexStr;

		hexStr.sprintf("0x%X", rs232Port->txDataID());

		cmd.setComment(QString(tr("data UID - %1")).arg(hexStr));

		m_code.append(cmd);

		result &= copyPortRS232AnalogSignals(portDataAddress, rs232Port, xmlWriter);
		result &= copyPortRS232DiscreteSignals(portDataAddress, rs232Port, xmlWriter);

		xmlWriter.writeEndElement();	// </Signals>

		xmlWriter.writeEndElement();	// </SerialData>
		xmlWriter.writeEndDocument();

		m_resultWriter->addFile(m_lm->propertyValue("SubsystemID").toString(), QString("rs232-%1.xml").arg(rs232Port->equipmentID()), xmlData);

		return result;
		*/
	}

	void OptoPort::getTxSignals(QVector<TxRxSignalShared>& txSignals) const
	{
		txSignals.clear();

		for(const TxRxSignalShared& s : m_txSignals)
		{
			if (s == nullptr)
			{
				assert(false);
				continue;
			}

			txSignals.append(s);
		}
	}

	void OptoPort::getRxSignals(QVector<TxRxSignalShared>& rxSignals) const
	{
		rxSignals.clear();

		for(const TxRxSignalShared& s : m_rxSignals)
		{
			if (s == nullptr)
			{
				assert(false);
				continue;
			}

			rxSignals.append(s);
		}
	}

	void OptoPort::getTxAnalogSignals(QVector<TxRxSignalShared>& txSignals, bool excludeRawSignals) const
	{
		txSignals.clear();

		for(TxRxSignalShared txSignal : m_txSignals)
		{
			if (txSignal == nullptr)
			{
				assert(false);
				continue;
			}

			if (excludeRawSignals == true && txSignal->isRaw() == true)
			{
				continue;
			}

			if (txSignal->isAnalog() == true)
			{
				txSignals.append(txSignal);
			}
		}
	}

	void OptoPort::getTxDiscreteSignals(QVector<TxRxSignalShared>& txSignals, bool excludeRawSignals) const
	{
		txSignals.clear();

		for(TxRxSignalShared txSignal : m_txSignals)
		{
			if (txSignal == nullptr)
			{
				assert(false);
				continue;
			}

			if (excludeRawSignals == true && txSignal->isRaw() == true)
			{
				continue;
			}

			if (txSignal->isDiscrete() == true)
			{
				txSignals.append(txSignal);
			}
		}
	}

	bool OptoPort::isTxSignalExists(const QString& appSignalID)
	{
		return m_txSignalIDs.contains(appSignalID);
	}

	bool OptoPort::isTxSignalExists(const Builder::UalSignal* ualSignal)
	{
		if (ualSignal == nullptr)
		{
			assert(false);
			return false;
		}

		QStringList refSignalIDs;

		ualSignal->refSignalIDs(&refSignalIDs);

		for(const QString& appSignalID : refSignalIDs)
		{
			if (m_txSignalIDs.contains(appSignalID) == true)
			{
				return true;
			}
		}

		return false;
	}

	bool OptoPort::isRxSignalExists(const QString& appSignalID)
	{
		return m_rxSignalIDs.contains(appSignalID);
	}

	bool OptoPort::isRxSignalExists(const Builder::UalSignal* ualSignal)
	{
		if (ualSignal == nullptr)
		{
			assert(false);
			return false;
		}

		QStringList refSignalIDs;

		ualSignal->refSignalIDs(&refSignalIDs);

		for(const QString& appSignalID : refSignalIDs)
		{
			if (m_rxSignalIDs.contains(appSignalID) == true)
			{
				return true;
			}
		}

		return false;
	}

	bool OptoPort::isSerialRxSignalExists(const QString& appSignalID)
	{
		if (isSinglePortConnection() == false)
		{
			return false;
		}

		return m_rxSignalIDs.contains(appSignalID);
	}

	bool OptoPort::isUsedInConnection() const
	{
		return m_connectionID.isEmpty() != true;
	}

/*	Address16 OptoPort::getTxSignalAddrInBuf(const QString& appSignalID) const
	{
		TxRxSignalShared s = m_txSignals.value(appSignalID, nullptr);

		if (s != nullptr)
		{
			return s->addrInBuf();
		}

		assert(false);

		return Address16();
	}*/

	bool OptoPort::getTxSignalAbsAddress(const QString& appSignalID, SignalAddress16& addr) const
	{
		TxRxSignalShared txSignal = m_txSignalIDs.value(appSignalID, nullptr);

		if (txSignal == nullptr)
		{
			ASSERT_RETURN_FALSE;
		}

		int absTxBufAddr = txBufAbsAddress();

		if (absTxBufAddr == BAD_ADDRESS)
		{
			ASSERT_RETURN_FALSE;
		}

		Address16 absAddr = txSignal->addrInBuf();

		absAddr.addWord(absTxBufAddr);

		addr = absAddr;

		addr.setSignalType(txSignal->signalType());
		addr.setDataFormat(txSignal->dataFormat());
		addr.setDataSize(txSignal->dataSize());
		addr.setByteOrder(txSignal->byteOrder());

		return true;
	}

	bool OptoPort::getRxSignalAbsAddress(const QString& appSignalID, SignalAddress16 &addr) const
	{
		TxRxSignalShared rxSignal = m_rxSignalIDs.value(appSignalID);

		if (rxSignal == nullptr)
		{
			ASSERT_RETURN_FALSE;
		}

		int rxBufAbsAddr = rxBufAbsAddress();

		if (rxBufAbsAddr == BAD_ADDRESS)
		{
			ASSERT_RETURN_FALSE;
		}

		Address16 rxSignalAddr = rxSignal->addrInBuf();

		rxSignalAddr.addWord(rxBufAbsAddr);

		addr = rxSignalAddr;

		addr.setSignalType(rxSignal->signalType());
		addr.setDataFormat(rxSignal->dataFormat());
		addr.setDataSize(rxSignal->dataSize());
		addr.setByteOrder(rxSignal->byteOrder());

		return true;
	}

	bool OptoPort::parseRawDescription()
	{
		bool result = m_rawDataDescription.parse(*this, m_log);

		if (result == false)
		{
			return false;
		}

		m_rawTxSignals.clear();
		m_rawRxSignals.clear();

		m_txRawDataSizeW = 0;
		m_rxRawDataSizeW = 0;

		for(const RawDataDescriptionItem& item : m_rawDataDescription)
		{
			switch(item.type)
			{
			case RawDataDescriptionItem::Type::TxRawDataSize:
				//m_txRawDataSizeW = item.txRawDataSize;
				break;

			case RawDataDescriptionItem::Type::RxRawDataSize:
				m_rxRawDataSizeW = item.rxRawDataSize;
				break;

			case RawDataDescriptionItem::Type::TxSignal:
				m_rawTxSignals.insert(item.appSignalID, item);
				break;

			case RawDataDescriptionItem::Type::RxSignal:
				m_rawRxSignals.insert(item.appSignalID, item);
				break;

			case RawDataDescriptionItem::Type::TxAllModulesRawData:
			case RawDataDescriptionItem::Type::TxModuleRawData:
			case RawDataDescriptionItem::Type::TxPortRawData:
			case RawDataDescriptionItem::Type::TxConst16:
				break;

			default:
				assert(false);
			}
		}

		return true;
	}

	bool OptoPort::calculatePortRawDataSize(OptoModuleStorage* optoStorage)
	{
		const DeviceModule* lm = DeviceHelper::getAssociatedLmOrBvb(m_controller);

		if (optoStorage == nullptr || lm == nullptr)
		{
			assert(false);
			return false;
		}

		QString msg;

		if (m_txRawDataSizeWCalculationStarted == true)
		{
			// cyclic call of this->calculatePortRawDataSize()
			//
			msg = QString("Can't calculate txRawDataSizeW for opto-port '%1'. Cyclic dependence.").arg(equipmentID());
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::AlCompiler,  msg);
			return false;
		}

		if (txRawDataSizeWIsCalculated() == true)
		{
			return true;
		}

		if (m_rawDataDescription.isEmpty() == true)
		{
			setTxRawDataSizeW(0);
			return true;
		}

		if (m_rawDataDescription.txRawDataSizeIsValid() == false)
		{
			msg = QString("Can't calculate txRawDataSizeW for opto-port '%1'. TX_RAW_DATA_SIZE item not found.").arg(equipmentID());
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::AlCompiler,  msg);
			return false;
		}

		m_txRawDataSizeWCalculationStarted = true;		// to prevent cyclic calculatePortRawDataSize()

		if (m_rawDataDescription.txRawDataSizeIsAuto() == false)
		{
			// txRawDataSizeW set manually
			//
			setTxRawDataSizeW(m_rawDataDescription.txRawDataSize());

			return true;
		}

		// automatic txRawDataSizeW calculation
		//
		bool result = true;

		int size = 0;
		int partSizeW = 0;

		for(const RawDataDescriptionItem& item : m_rawDataDescription)
		{
			switch(item.type)
			{
			case RawDataDescriptionItem::Type::TxRawDataSize:
				break;

			case RawDataDescriptionItem::Type::TxAllModulesRawData:

				partSizeW = DeviceHelper::getAllNativeRawDataSize(lm, m_log);;

				size += partSizeW;

				break;

			case RawDataDescriptionItem::Type::TxModuleRawData:
				{
					bool moduleIsFound = false;

					partSizeW = DeviceHelper::getModuleRawDataSize(lm, item.modulePlace, &moduleIsFound, m_log);

					size += partSizeW;

					if (moduleIsFound == false)
					{
						msg = QString("Module on place %1 is not found (opto port '%2' raw data description).").arg(item.modulePlace).arg(equipmentID());
						LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::AlCompiler,  msg);
						result = false;
					}
				}

				break;

			case RawDataDescriptionItem::Type::TxPortRawData:
				{
					OptoPortShared portRxRawData = optoStorage->getOptoPort(item.portEquipmentID);

					if (portRxRawData == nullptr)
					{
						msg = QString("Port '%1' is not found (opto port '%2' raw data description).").arg(item.portEquipmentID).arg(equipmentID());
						LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::AlCompiler,  msg);
						result = false;
						break;
					}

					OptoPortShared portTxRawData = optoStorage->getOptoPort(portRxRawData->linkedPortID());

					if (portTxRawData == nullptr)
					{
						msg = QString("Port '%1' linked to '%2' is not found (opto port '%3' raw data description).").
								arg(portRxRawData->linkedPortID()).
								arg(portRxRawData->equipmentID()).
								arg(equipmentID());
						LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::AlCompiler,  msg);
						result = false;
						break;
					}

					bool res = portTxRawData->calculatePortRawDataSize(optoStorage);

					if (res == true)
					{
						partSizeW = portTxRawData->txRawDataSizeW();

						size += partSizeW;

						// LOG_MESSAGE(log, QString(tr("PORT_RAW_DATA=%1 sizeW = %2")).arg(item.portEquipmentID).arg(partSizeW));
					}
					else
					{
						result = false;
					}
				}
				break;

			case RawDataDescriptionItem::Type::TxConst16:

				size++;

				break;

			case RawDataDescriptionItem::Type::TxSignal:

				if (item.offsetW + item.dataSize / SIZE_16BIT > size)
				{
					size = item.offsetW + item.dataSize / SIZE_16BIT;
				}

				break;

			case RawDataDescriptionItem::Type::RxRawDataSize:
			case RawDataDescriptionItem::Type::RxSignal:
				break;

			default:
				assert(false);
				result = false;
			}

			if (result == false)
			{
				break;
			}
		}

		if (result == true)
		{
			setTxRawDataSizeW(size);
			// LOG_MESSAGE(log, QString(tr("Port %1 rawDataSizeW = %2")).arg(m_equipmentID).arg(m_txRawDataSizeW));
		}

		m_txRawDataSizeWCalculationStarted = false;

		return result;
	}

	QString OptoPort::serialModeStr() const
	{
		return Connection::serialModeStr(m_serialMode);
	}

	int OptoPort::txBufAbsAddress() const
	{
		if (m_txBufAddress == BAD_ADDRESS)
		{
			assert(false);
			return BAD_ADDRESS;
		}

		OptoModuleShared module = OptoModuleStorage::getOptoModule(m_optoModuleID);

		if (module == nullptr)
		{
			assert(false);
			return BAD_ADDRESS;
		}

		if (module->isLmOrBvb() == true)
		{
			return module->optoInterfaceDataOffset() + (m_portNo - 1) * module->optoPortAppDataSize() + m_txBufAddress;
		}

		assert(module->isOcm() == true);

		return module->optoInterfaceDataOffset() + m_txBufAddress;
	}

	int OptoPort::rxBufAbsAddress() const
	{
		if (m_rxBufAddress == BAD_ADDRESS)
		{
			assert(false);
			return BAD_ADDRESS;
		}

		OptoModuleShared module = OptoModuleStorage::getOptoModule(m_optoModuleID);

		if (module == nullptr)
		{
			assert(false);
			return BAD_ADDRESS;
		}

		if (module->isLmOrBvb() == true)
		{
			return module->optoInterfaceDataOffset() + (m_portNo - 1) * module->optoPortAppDataSize() + m_rxBufAddress;
		}

		assert(module->isOcm() == true);

		return module->optoInterfaceDataOffset() + m_rxBufAddress;
	}

	void OptoPort::setTxRawDataSizeW(int rawDataSizeW)
	{
		assert(m_txRawDataSizeWIsCalculated == false);		// setTxRawDataSizeW() must be called once!

		m_txRawDataSizeW = rawDataSizeW;

		m_txRawDataSizeWIsCalculated = true;
	}

	void OptoPort::writeInfo(QStringList& list) const
	{
		if (isUsedInConnection() == false)
		{
			list.append(QString(tr("Port %1 isn't used in connections\n")).arg(equipmentID()));
			return;
		}

		QString str;

		list.append(QString(tr("Port %1 information\n")).arg(equipmentID()));

		list.append(QString(tr("Manual settings:\t\t%1")).arg(manualSettings() == true ? "True" : "False"));

		if (manualSettings() == true)
		{
			list.append(QString(tr("Manual Tx start address:\t%1")).arg(manualTxStartAddressW()));
			list.append(QString(tr("Manual Tx size:\t\t\t%1")).arg(manualTxSizeW()));
			list.append(QString(tr("Manual Rx size:\t\t\t%1")).arg(manualRxSizeW()));
		}

		list.append("");

		list.append(QString(tr("Enable serial:\t\t\t%1")).arg(enableSerial() == true ? "True" : "False"));

		if (enableSerial() == true)
		{
			list.append(QString(tr("Serial mode:\t\t\t%1")).arg(serialModeStr()));
			list.append(QString(tr("Enable duplex:\t\t\t%1\n")).arg(enableDuplex() == true ? "True" : "False"));
		}
		else
		{
			list.append("");
		}

		list.append(QString(tr("Tx buffer abs address:\t\t%1")).arg(txBufAbsAddress()));
		list.append(QString(tr("Tx buffer offset:\t\t%1")).arg(txBufAddress()));
		list.append(QString(tr("Tx data full size:\t\t%1")).arg(txDataSizeW()));
		list.append(QString(tr("Tx data used size:\t\t%1")).arg(txUsedDataSizeW()));
		list.append(QString("\t\t\t\t-----"));

		str = QString(tr("Tx data ID size:\t\t%1")).arg(Hardware::OptoPort::TX_DATA_ID_SIZE_W);
		list.append(str);

		str = QString(tr("Tx raw data size:\t\t%1")).arg(txRawDataSizeW());
		list.append(str);

		str = QString(tr("Tx analog signals size:\t\t%1")).arg(txAnalogSignalsSizeW());
		list.append(str);

		str = QString(tr("Tx discrete signals size:\t%1\n")).arg(txDiscreteSignalsSizeW());
		list.append(str);

		list.append(QString(tr("Port Tx data:\n")));

		str.sprintf("%04d:%02d  [%04d:%02d]  TxDataID = 0x%08X (%u)\n", txBufAbsAddress(), 0, 0, 0, txDataID(), txDataID());
		list.append(str);

		list.append("Tx raw signals:\n");

		bool hasSignals = false;

		for(const Hardware::TxRxSignalShared& tx : m_txSignals)
		{
			if (tx->isRaw() == true)
			{
				str.sprintf("%04d:%02d  [%04d:%02d]  %s",
							txBufAbsAddress() + tx->addrInBuf().offset(), tx->addrInBuf().bit(),
							tx->addrInBuf().offset(), tx->addrInBuf().bit(),
							C_STR(tx->appSignalIDs().join(", ")));
				list.append(str);

				hasSignals = true;
			}
		}

		if (hasSignals == true)
		{
			list.append("");
		}

		list.append("Tx analog signals:\n");

		hasSignals = false;

		for(const Hardware::TxRxSignalShared& tx : m_txSignals)
		{
			if (tx->isRegular() == true && tx->isAnalog() == true)
			{
				str.sprintf("%04d:%02d  [%04d:%02d]  %s",
							txBufAbsAddress() + tx->addrInBuf().offset(), tx->addrInBuf().bit(),
							tx->addrInBuf().offset(), tx->addrInBuf().bit(),
							C_STR(tx->appSignalIDs().join(", ")));
				list.append(str);

				hasSignals = true;
			}
		}

		if (hasSignals == true)
		{
			list.append("");
		}

		list.append("Tx bus signals:\n");

		hasSignals = false;

		for(const Hardware::TxRxSignalShared& tx : m_txSignals)
		{
			if (tx->isRegular() == true && tx->isBus() == true)
			{
				str.sprintf("%04d:%02d  [%04d:%02d]  %s",
							txBufAbsAddress() + tx->addrInBuf().offset(), tx->addrInBuf().bit(),
							tx->addrInBuf().offset(), tx->addrInBuf().bit(),
							C_STR(tx->appSignalIDs().join(", ")));
				list.append(str);

				hasSignals = true;
			}
		}

		if (hasSignals == true)
		{
			list.append("");
		}

		list.append("Tx discrete signals:\n");

		hasSignals = false;

		for(const Hardware::TxRxSignalShared& tx : m_txSignals)
		{
			if (tx->isRegular() == true && tx->isDiscrete() == true)
			{
				str.sprintf("%04d:%02d  [%04d:%02d]  %s",
							txBufAbsAddress() + tx->addrInBuf().offset(), tx->addrInBuf().bit(),
							tx->addrInBuf().offset(), tx->addrInBuf().bit(),
							C_STR(tx->appSignalIDs().join(", ")));
				list.append(str);

				hasSignals = true;
			}
		}

		if (hasSignals == true)
		{
			list.append("");
		}

		list.append("-------------------------------------\n");

		list.append(QString(tr("Rx buffer abs address:\t\t%1")).arg(rxBufAbsAddress()));
		list.append(QString(tr("Rx buffer offset:\t\t%1")).arg(rxBufAddress()));
		list.append(QString(tr("Rx data full size:\t\t%1")).arg(rxDataSizeW()));
		list.append(QString(tr("Rx data used size:\t\t%1")).arg(rxUsedDataSizeW()));
		list.append(QString("\t\t\t\t-----"));

		str = QString(tr("Rx data ID size:\t\t%1")).arg(Hardware::OptoPort::TX_DATA_ID_SIZE_W);
		list.append(str);

		str = QString(tr("Rx raw data size:\t\t%1")).arg(rxRawDataSizeW());
		list.append(str);

		str = QString(tr("Rx analog signals size:\t\t%1")).arg(rxAnalogSignalsSizeW());
		list.append(str);

		str = QString(tr("Rx discrete signals size:\t%1\n")).arg(rxDiscreteSignalsSizeW());
		list.append(str);

		list.append(QString(tr("Rx validity signal:\t\t%1\t%2\n")).
					arg(validitySignalAbsAddr().toString()).
					arg(validitySignalID()));

		list.append(QString(tr("Port Rx data:\n")));

		str.sprintf("%04d:%02d  [%04d:%02d]  RxDataID = 0x%08X (%u)\n", rxBufAbsAddress(), 0, 0, 0, rxDataID(), rxDataID());
		list.append(str);

		list.append("Rx raw signals:\n");

		hasSignals = false;

		for(const Hardware::TxRxSignalShared& rx : m_rxSignals)
		{
			if (rx->isRaw() == true)
			{
				str.sprintf("%04d:%02d  [%04d:%02d]  %s",
							rxBufAbsAddress() + rx->addrInBuf().offset(), rx->addrInBuf().bit(),
							rx->addrInBuf().offset(), rx->addrInBuf().bit(),
							C_STR(rx->appSignalIDs().join(", ")));
				list.append(str);

				hasSignals = true;
			}
		}

		if (hasSignals == true)
		{
			list.append("");
		}

		list.append("Rx analog signals:\n");

		hasSignals = false;

		for(const Hardware::TxRxSignalShared& rx : m_rxSignals)
		{
			if (rx->isRegular() == true && rx->isAnalog() == true)
			{
				str.sprintf("%04d:%02d  [%04d:%02d]  %s",
							rxBufAbsAddress() + rx->addrInBuf().offset(), rx->addrInBuf().bit(),
							rx->addrInBuf().offset(), rx->addrInBuf().bit(),
							C_STR(rx->appSignalIDs().join(", ")));
				list.append(str);

				hasSignals = true;
			}
		}

		if (hasSignals == true)
		{
			list.append("");
		}

		list.append("Rx bus signals:\n");

		hasSignals = false;

		for(const Hardware::TxRxSignalShared& rx : m_rxSignals)
		{
			if (rx->isRegular() == true && rx->isBus() == true)
			{
				str.sprintf("%04d:%02d  [%04d:%02d]  %s",
							rxBufAbsAddress() + rx->addrInBuf().offset(), rx->addrInBuf().bit(),
							rx->addrInBuf().offset(), rx->addrInBuf().bit(),
							C_STR(rx->appSignalIDs().join(", ")));
				list.append(str);

				hasSignals = true;
			}
		}

		if (hasSignals == true)
		{
			list.append("");
		}

		list.append("Rx discrete signals:\n");

		hasSignals = false;

		for(const Hardware::TxRxSignalShared& rx : m_rxSignals)
		{
			if (rx->isRegular() == true && rx->isDiscrete() == true)
			{
				str.sprintf("%04d:%02d  [%04d:%02d]  %s",
							rxBufAbsAddress() + rx->addrInBuf().offset(), rx->addrInBuf().bit(),
							rx->addrInBuf().offset(), rx->addrInBuf().bit(),
							C_STR(rx->appSignalIDs().join(", ")));
				list.append(str);

				hasSignals = true;
			}
		}

		if (hasSignals == true)
		{
			list.append("");
		}
	}

	bool OptoPort::appendTxSignal(const Builder::UalSignal* ualSignal)
	{
		if (ualSignal == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		QString ualSignalID = ualSignal->appSignalID();

		if (m_txSignalIDs.contains(ualSignalID) == true)
		{
			return true;			// signal already in tx list
		}

		TxRxSignalShared txSignal = std::make_shared<TxRxSignal>();

		bool res = txSignal->init(ualSignal);

		if (res == false)
		{
			return false;
		}

		QStringList appSignalIDs;

		ualSignal->refSignalIDs(&appSignalIDs);

		m_txSignals.append(txSignal);

		for(const QString& appSignalID : appSignalIDs)
		{
			m_txSignalIDs.insert(appSignalID, txSignal);
		}

		return true;
	}


	bool OptoPort::appendRxSignal(const Builder::UalSignal* ualSignal)
	{
		if (ualSignal == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		QString ualSignalID = ualSignal->appSignalID();

		if (m_rxSignalIDs.contains(ualSignalID) == true)
		{
			assert(false);					// for debug
			return true;					// signal already in rs list
		}

		TxRxSignalShared rxSignal = std::make_shared<TxRxSignal>();

		bool res = rxSignal->init(ualSignal);

		if (res == false)
		{
			return false;
		}

		m_rxSignals.append(rxSignal);

		QStringList appSignalIDs;

		ualSignal->refSignalIDs(&appSignalIDs);

		for(const QString& appSignalID : appSignalIDs)
		{
			m_rxSignalIDs.insert(appSignalID, rxSignal);
		}

		return true;
	}

	void OptoPort::sortByOffsetBitNoAscending(QVector<TxRxSignalShared>& list)
	{
		int count = list.count();

		for(int i = 0; i < count - 1; i++)
		{
			for(int k = i + 1; k < count; k++)
			{
				if (list[i]->addrInBuf().bitAddress() > list[k]->addrInBuf().bitAddress())
				{
					TxRxSignalShared temp = list[i];
					list[i] = list[k];
					list[k] = temp;
				}
			}
		}
	}

	void OptoPort::sortByAppSignalIdAscending(QVector<TxRxSignalShared>& list)
	{
		int count = list.count();

		for(int i = 0; i < count - 1; i++)
		{
			for(int k = i + 1; k < count; k++)
			{
				if (list[i]->appSignalID() > list[k]->appSignalID())
				{
					TxRxSignalShared temp = list[i];
					list[i] =  list[k];
					list[k] = temp;
				}
			}
		}
	}

	bool OptoPort::checkSignalsOffsets(const QVector<TxRxSignalShared>& signalList, int startIndex, int count)
	{
		bool result = true;

		for(int i = startIndex; i < startIndex + count - 1; i++)
		{
			const TxRxSignalShared& s1 = signalList[i];
			const TxRxSignalShared& s2 = signalList[i + 1];

			int s1StartBitAddress = s1->addrInBuf().bitAddress();
			int s1EndBitAddress = s1->addrInBuf().bitAddress() + s1->dataSize() - 1;

			int s2StartBitAddress = s2->addrInBuf().bitAddress();
			int s2EndBitAddress = s2->addrInBuf().bitAddress() + s1->dataSize() - 1;

			if ((s1StartBitAddress >= s2StartBitAddress && s1StartBitAddress <= s2EndBitAddress) ||
				(s2StartBitAddress >= s1StartBitAddress && s2StartBitAddress <= s1EndBitAddress))
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
								   QString("Raw signals '%1' and '%2' is overlapped (port '%3').").
								   arg(s1->appSignalID()).arg(s2->appSignalID()).arg(equipmentID()));
			}
		}

		return result;
	}

	bool OptoPort::sortTxRxSignalList(QVector<TxRxSignalShared>& signalList)
	{
		// Tx/Rx signals sorting order:
		//
		// 1. All Raw signals (on addrInBuf ascending)
		// 2. All Regular Analog signals (on appSignalID ascending)
		// 3. All Regular BusSignals (on appSignalID ascending)
		// 4. All Regular Discrete signals (on appSignalID ascending)

		QVector<TxRxSignalShared> tempList;
		QVector<TxRxSignalShared> tempSignalList = signalList;

		int count = tempSignalList.count();

		signalList.clear();

		// 1. Fetch and sort all Raw Tx signals
		//
		for(int i = 0; i < count; i++)
		{
			TxRxSignalShared& s = tempSignalList[i];

			if (s->isRaw() == true)
			{
				tempList.append(s);
			}
		}

		sortByOffsetBitNoAscending(tempList);

		signalList.append(tempList);

		if (checkSignalsOffsets(signalList, 0, signalList.count()) == false)
		{
			return false;
		}

		// 2. Fetch and sort all regular analog Tx signals
		//
		tempList.clear();

		for(int i = 0; i < count; i++)
		{
			TxRxSignalShared& s = tempSignalList[i];

			if (s->isRegular() == true && s->isAnalog() == true)
			{
				tempList.append(s);
			}
		}

		sortByAppSignalIdAscending(tempList);

		signalList.append(tempList);

		// 3. Fetch and sort all regular bus Tx signals
		//
		tempList.clear();

		for(int i = 0; i < count; i++)
		{
			TxRxSignalShared& s = tempSignalList[i];

			if (s->isRegular() == true && s->isBus() == true)
			{
				tempList.append(s);
			}
		}

		sortByAppSignalIdAscending(tempList);

		signalList.append(tempList);

		// 4. Fetch and sort all regular discrete Tx signals
		//
		tempList.clear();

		for(int i = 0; i < count; i++)
		{
			TxRxSignalShared& s = tempSignalList[i];

			if (s->isRegular() == true && s->isDiscrete() == true)
			{
				tempList.append(s);
			}
		}

		sortByAppSignalIdAscending(tempList);

		signalList.append(tempList);

		return true;
	}

	// --------------------------------------------------------------------------------------
	//
	// OptoModule class implementation
	//
	// --------------------------------------------------------------------------------------

	OptoModule::OptoModule()
	{
	}

	OptoModule::~OptoModule()
	{
	}

	bool OptoModule::init(DeviceModule* module, LogicModule* lmDescription, Builder::IssueLogger* log)
	{
		if (module == nullptr || lmDescription == nullptr || log == nullptr)
		{
			assert(false);
			return false;
		}

		m_deviceModule = module;
		m_lmDescription = lmDescription;
		m_log = log;

		m_equipmentID = module->equipmentIdTemplate();
		m_place = module->place();

		bool result = true;

		if (module->isLogicModule() == true || module->isBvb() == true)
		{
			m_optoInterfaceDataOffset = m_lmDescription->optoInterface().m_optoInterfaceDataOffset;
			m_optoPortDataSize = m_lmDescription->optoInterface().m_optoPortDataSize;
			m_optoPortAppDataOffset = m_lmDescription->optoInterface().m_optoPortAppDataOffset;
			m_optoPortAppDataSize = m_lmDescription->optoInterface().m_optoPortAppDataSize;
			m_optoPortCount = m_lmDescription->optoInterface().m_optoPortCount;
		}
		else
		{
			assert(module->isOptoModule() == true);

			result &= DeviceHelper::getIntProperty(module, "OptoInterfaceDataOffset", &m_optoInterfaceDataOffset, log);
			result &= DeviceHelper::getIntProperty(module, "OptoPortDataSize", &m_optoPortDataSize, log);
			result &= DeviceHelper::getIntProperty(module, "OptoPortAppDataOffset", &m_optoPortAppDataOffset, log);
			result &= DeviceHelper::getIntProperty(module, "OptoPortAppDataSize", &m_optoPortAppDataSize, log);
			result &= DeviceHelper::getIntProperty(module, "OptoPortCount", &m_optoPortCount, log);

			if (result == false)
			{
				return false;
			}
		}

		// set actual OptoInterfaceDataOffset for OCM module according to place of module
		//
		if (isOcm() == true)
		{
			// OCM's OptoPortDataSize property (m_optoPortDataSize) is equal to LM's ModuleDataSize property
			//
			m_optoInterfaceDataOffset = (m_deviceModule->place() - 1) * m_optoPortDataSize;
		}

		int findPortCount = 0;

		for(int i = 0; i < m_optoPortCount; i++)
		{
			QString portStrID = QString("%1_OPTOPORT0%2").arg(module->equipmentIdTemplate()).arg(i + 1);

			int childrenCount = module->childrenCount();

			for(int c = 0; c < childrenCount; c++)
			{
				DeviceObject* child = module->child(c);

				if (child == nullptr)
				{
					assert(false);
					continue;
				}

				if (child->isController() && child->equipmentIdTemplate() == portStrID)
				{
					DeviceController* optoPortController = child->toController();

					if (optoPortController == nullptr)
					{
						assert(false);
						LOG_INTERNAL_ERROR(m_log);
						return false;
					}

					OptoPortShared optoPort = std::make_shared<OptoPort>();

					result &= optoPort->init(optoPortController, i + 1, m_lmDescription, log);

					m_ports.insert(optoPortController->equipmentIdTemplate(), optoPort);

					findPortCount++;

					break;
				}
			}
		}

		if (result == false)
		{
			return false;
		}

		if (findPortCount != m_optoPortCount)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
							   QString(tr("Not all opto-port controllers found in module '%1'")).arg(module->equipmentIdTemplate()));
			return false;
		}

		if (isLmOrBvb() == true)
		{
			m_lmID = module->equipmentIdTemplate();
			m_lm = module;
		}
		else
		{
			assert(isOcm() == true);

			const DeviceModule* lm = DeviceHelper::getAssociatedLmOrBvb(module);

			if (lm == nullptr)
			{
				assert(false);
				LOG_INTERNAL_ERROR(log);
				return false;
			}

			m_lm = lm;
			m_lmID = lm->equipmentIdTemplate();
		}

		m_valid = true;

		return true;
	}

	bool OptoModule::isLmOrBvb()
	{
		if (m_deviceModule == nullptr)
		{
			assert(false);
			return false;
		}

		return m_deviceModule->isLogicModule() || m_deviceModule->isBvb();
	}

	bool OptoModule::isOcm()
	{
		if (m_deviceModule == nullptr)
		{
			assert(false);
			return false;
		}

		return m_deviceModule->moduleFamily() == DeviceModule::FamilyType::OCM;
	}

	bool OptoModule::isBvb()
	{
		if (m_deviceModule == nullptr)
		{
			assert(false);
			return false;
		}

		return m_deviceModule->isBvb();
	}

	void OptoModule::getSerialPorts(QList<OptoPortShared>& serialPortsList)
	{
		for(OptoPortShared& port : m_ports)
		{
			if (port == nullptr)
			{
				assert(false);
				continue;
			}

			if (port->isSinglePortConnection() == true)
			{
				serialPortsList.append(port);
			}
		}
	}

	void OptoModule::getOptoPorts(QList<OptoPortShared>& optoPortsList)
	{
		for(OptoPortShared& port : m_ports)
		{
			if (port == nullptr)
			{
				assert(false);
				continue;
			}

			if (port->isPortToPortConnection() == true)
			{
				optoPortsList.append(port);
			}
		}
	}

	bool OptoModule::calculateTxBufAddresses()
	{
		bool result = true;

		if (isLmOrBvb() == true)
		{
			// calculate tx buffers absolute addresses for ports of LM module
			//
			for(OptoPortShared& port : m_ports)
			{
				if (port == nullptr)
				{
					LOG_INTERNAL_ERROR(m_log);
					ASSERT_RETURN_FALSE;
				}

				if (port->isUsedInConnection() == false)
				{
					continue;
				}

				if (port->manualSettings() == true)
				{
					int manualTxStartAddr = port->manualTxStartAddressW();

					if (manualTxStartAddr < 0 || manualTxStartAddr >= optoPortDataSize())
					{
						LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
										   QString(tr("Manual TxStartAddress of port '%1' out of range 0..%2 (connection %3)")).
										   arg(port->equipmentID()).arg(optoPortDataSize()).arg(port->connectionID()));
						return false;
					}

					// exotic 'manual' settings for LMs tx buffers
					// ok, user is always right
					//
					port->setTxBufAddress(manualTxStartAddr);

					if (manualTxStartAddr + port->manualTxSizeW() > optoPortAppDataSize())
					{
						// TxData size (%1 words) of opto port '%2' exceed value of OptoPortAppDataSize property of module '%3' (%4 words).
						//
						m_log->errALC5032(port->txDataSizeW(), port->equipmentID(), equipmentID(), optoPortAppDataSize());
						result = false;
						break;
					}
				}
				else
				{
					// each LMs opto-port has separate memory area for tx/rx buffer
					// so, tx buffers offset in this area always equal to 0
					//
					port->setTxBufAddress(0);

					if (port->txDataSizeW() > optoPortAppDataSize())
					{
						// TxData size (%1 words) of opto port '%2' exceed value of OptoPortAppDataSize property of module '%3' (%4 words).
						//
						m_log->errALC5032(port->txDataSizeW(), port->equipmentID(), equipmentID(), optoPortAppDataSize());
						result = false;
						break;
					}
				}
			}

			result &= checkPortsAddressesOverlapping();

			return result;
		}

		if (isOcm() == true)
		{
			// calculate tx addresses for ports of OCM module
			//
			// all OCM's ports tx buffers is disposed in one memory area with max size - OptoPortAppDataSize
			// tx buffers begin place from offset 0
			//
			int txBufAddress = 0;

			int txDataSizeW = 0;

			for(OptoPortShared& port : m_ports)
			{
				if (port == nullptr)
				{
					LOG_INTERNAL_ERROR(m_log);
					ASSERT_RETURN_FALSE;
				}

				if (port->isUsedInConnection() == false)
				{
					continue;
				}

				if (port->manualSettings() == true)
				{
					txBufAddress = port->manualTxStartAddressW();

					port->setTxBufAddress(txBufAddress);

					if (port->manualTxStartAddressW() + port->manualTxSizeW() > optoPortAppDataSize())
					{
						// TxData size (%1 words) of opto port '%2' exceed value of OptoPortAppDataSize property of module '%3' (%4 words).
						//
						m_log->errALC5032(port->txDataSizeW(), port->equipmentID(), equipmentID(), optoPortAppDataSize());
						return false;
					}

					// calculate TxStartAddr for next port with auto settings (if exists)
					//
					txBufAddress += port->manualTxSizeW();

					txDataSizeW = port->manualTxStartAddressW() + port->manualTxSizeW();
				}
				else
				{
					//
					port->setTxBufAddress(txBufAddress);

					txBufAddress += port->txDataSizeW();

					txDataSizeW += port->txDataSizeW();
				}

				if (txDataSizeW > optoPortAppDataSize())
				{
					// TxData size (%1 words) of opto port '%2' exceed value of OptoPortAppDataSize property of module '%3' (%4 words).
					//
					m_log->errALC5032(port->txDataSizeW(), port->equipmentID(), equipmentID(), optoPortAppDataSize());
					result = false;
					break;
				}
			}

			result &= checkPortsAddressesOverlapping();

			return result;
		}

		// unknown module family
		//
		ASSERT_RETURN_FALSE;
	}

	bool OptoModule::checkPortsAddressesOverlapping()
	{
		// checking ports addresses overlapping
		// usefull for manual settings of ports
		//
		int portsCount = m_ports.count();

		bool result = true;

		for(int i = 0; i < portsCount - 1; i++)
		{
			OptoPortShared port1 = m_ports[i];

			if (port1 == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				assert(false);
				result = false;
				continue;
			}

			if (port1->isUsedInConnection() == false)
			{
				continue;
			}

			for(int k = i + 1; k < portsCount; k++)
			{
				OptoPortShared port2 = m_ports[k];

				if (port2 == nullptr)
				{
					LOG_INTERNAL_ERROR(m_log);
					assert(false);
					result = false;
					continue;
				}

				if (port2->isUsedInConnection() == false)
				{
					continue;
				}

				if (	(port1->txBufAbsAddress() >= port2->txBufAbsAddress() &&
						port1->txBufAbsAddress() < port2->txBufAbsAddress() + port2->txDataSizeW()) ||

						(port2->txBufAbsAddress() >= port1->txBufAbsAddress() &&
						port2->txBufAbsAddress() < port1->txBufAbsAddress() + port1->txDataSizeW())	)
				{
					// ports memory areas are overlapped
					//
					if (port1->manualSettings() == true && port2->manualSettings() == true)
					{
						// if both ports with manual settings - generate warning
						//
						m_log->wrnALC5194(port1->equipmentID(), port2->equipmentID());
					}
					else
					{
						// if single or both ports with Auto settings - generate error
						//
						// Tx data memory areas of ports '%1' and '%2' are overlapped.
						//
						m_log->errALC5187(port1->equipmentID(), port2->equipmentID());

						result = false;
					}
				}
			}
		}

		return result;
	}

	bool OptoModule::calculateRxBufAddresses()
	{
		bool result = true;

		if (isLmOrBvb() == true)
		{
			// calculate rx addresses for ports of LM module
			//
			for(OptoPortShared& port : m_ports)
			{
				if (port == nullptr)
				{
					LOG_INTERNAL_ERROR(m_log);
					assert(false);
					return false;
				}

				port->setRxBufAddress(0);

				if (port->isUsedInConnection() == false)
				{
					continue;
				}

				std::shared_ptr<Connection> connection = OptoModuleStorage::getConnection(port->connectionID());

				if (connection == nullptr)
				{
					assert(false);
					continue;
				}

				if (connection->isSinglePort() == true)
				{
					continue;
				}

				assert(connection->isPortToPort() == true);

				OptoPortShared linkedPort = OptoModuleStorage::getOptoPort(port->linkedPortID());

				if (linkedPort != nullptr)
				{
					if (linkedPort->txDataSizeW() > optoPortAppDataSize())
					{
						// RxData size (%1 words) of opto port '%2' exceed value of OptoPortAppDataSize property of module '%3' (%4 words).
						//
						m_log->errALC5035(linkedPort->txDataSizeW(), port->equipmentID(), equipmentID(), optoPortAppDataSize());
						result = false;
						break;
					}
				}
				else
				{
					LOG_INTERNAL_ERROR(m_log);
					assert(false);
					return false;
				}
			}

			return true;
		}

		if (isOcm() == true)
		{
			// calculate rx addresses for ports of OCM module
			//
			int rxStartAddress = optoPortAppDataOffset();				// offset on OCMs diag data size

			int rxDataSizeW = 0;

			for(OptoPortShared& port : m_ports)
			{
				if (port == nullptr)
				{
					LOG_INTERNAL_ERROR(m_log);
					assert(false);
					return false;
				}

				port->setRxBufAddress(0);			// initilaize to 0

				// all OCM's ports data disposed in one buffer with max size - OptoPortAppDataSize
				//

				if (port->isUsedInConnection() == false)
				{
					continue;			// port is not connected
				}

				std::shared_ptr<Connection> connection = OptoModuleStorage::getConnection(port->connectionID());

				if (connection == nullptr)
				{
					assert(false);
					continue;
				}

				if (connection->isSinglePort() == true)
				{
					if (connection->port1EquipmentID() != port->equipmentID() )
					{
						continue;
					}

					port->setRxBufAddress(rxStartAddress);

					if (port->manualSettings() == true)
					{
						rxStartAddress += port->manualRxSizeW();
					}
					else
					{
						rxStartAddress += port->rxDataSizeW();
					}

					continue;
				}

				assert(connection->isPortToPort() == true);

				port->setRxBufAddress(rxStartAddress);

				OptoPortShared linkedPort = OptoModuleStorage::getOptoPort(port->linkedPortID());

				if (linkedPort != nullptr)
				{
					rxStartAddress += linkedPort->txDataSizeW();

					rxDataSizeW += linkedPort->txDataSizeW();

					if (rxDataSizeW > optoPortAppDataSize())
					{
						// RxData size (%1 words) of opto port '%2' exceed value of OptoPortAppDataSize property of module '%3' (%4 words).
						//
						m_log->errALC5035(rxDataSizeW, port->equipmentID(), equipmentID(), optoPortAppDataSize());
						result = false;
						break;
					}
				}
				else
				{
					LOG_INTERNAL_ERROR(m_log);
					assert(false);
					return false;
				}
			}

			return true;
		}

		LOG_INTERNAL_ERROR(m_log);

		ASSERT_RETURN_FALSE;
	}

	bool OptoModule::forEachPort(OptoPortFunc funcPtr)
	{
		bool result = true;

		for(OptoPortShared& port : m_ports)
		{
			if (port == nullptr)
			{
				ASSERT_RETURN_FALSE;
			}

			OptoPort* optoPort = port.get();

			result &= (optoPort->*funcPtr)();
		}

		return result;
	}

	bool OptoModule::isSerialRxSignalExists(const QString& appSignalID)
	{
		for(OptoPortShared& port : m_ports)
		{
			if (port == nullptr)
			{
				ASSERT_RETURN_FALSE;
			}

			if (port->isSerialRxSignalExists(appSignalID) == true)
			{
				return true;
			}
		}

		return false;
	}

	bool OptoModule::calculateTxSignalsAddresses()
	{
		assert(isBvb() == true);

		bool result = true;

		for(OptoPortShared& port : m_ports)
		{
			if (port == nullptr)
			{
				ASSERT_RETURN_FALSE;
			}

			result &= port->calculateTxSignalsAddresses();
		}

		return result;
	}

	bool OptoModule::copyOpticalPortsTxInRxSignals()
	{
		assert(isBvb() == true);

		bool result = true;

		for(OptoPortShared& port : m_ports)
		{
			if (port == nullptr)
			{
				ASSERT_RETURN_FALSE;
			}

			result &= port->copyOpticalPortsTxInRxSignals();
		}

		return result;
	}

	void OptoModule::sortPortsByEquipmentIDAscending(QVector<OptoPort*>& ports)
	{
		int count = ports.count();

		for(int i = 0; i < count - 1; i++)
		{
			for(int k = i + 1; k < count; k++)
			{
				if (ports[i]->equipmentID() > ports[k]->equipmentID())
				{
					OptoPort* temp = ports[i];
					ports[i] = ports[k];
					ports[k] = temp;
				}
			}
		}
	}

	// --------------------------------------------------------------------------------------
	//
	// OptoModuleStorage class implementation
	//
	// --------------------------------------------------------------------------------------

	EquipmentSet* OptoModuleStorage::m_equipmentSet = nullptr;
	Builder::LmDescriptionSet* OptoModuleStorage::m_lmDescriptionSet = nullptr;
	Builder::IssueLogger* OptoModuleStorage::m_log = nullptr;

	int OptoModuleStorage::m_instanceCount = 0;
	HashedVector<QString, OptoModuleShared> OptoModuleStorage::m_modules;
	HashedVector<QString, OptoPortShared> OptoModuleStorage::m_ports;
	QHash<QString, OptoModuleShared> OptoModuleStorage::m_lmAssociatedModules;
	QHash<QString, std::shared_ptr<Connection>> OptoModuleStorage::m_connections;

	OptoModuleStorage::OptoModuleStorage(EquipmentSet* equipmentSet, Builder::LmDescriptionSet* lmDescriptionSet, Builder::IssueLogger *log)
	{
		assert(m_instanceCount == 0);			// OptoModuleStorage is singleton!

		m_instanceCount++;

		m_equipmentSet = equipmentSet;
		m_lmDescriptionSet = lmDescriptionSet;
		m_log = log;
	}

	OptoModuleStorage::~OptoModuleStorage()
	{
		clear();

		m_instanceCount--;
	}

	bool OptoModuleStorage::appendOptoModules()
	{
		LOG_EMPTY_LINE(m_log);

		LOG_MESSAGE(m_log, QString(tr("Searching opto-modules")));

		clear();

		bool result = true;

		equipmentWalker(m_equipmentSet->root(), [this, &result](DeviceObject* currentDevice)
			{
				if (currentDevice == nullptr)
				{
					assert(false);
					result = false;
					return;
				}

				if (currentDevice->isModule() == false)
				{
					return;
				}

				Hardware::DeviceModule* module = currentDevice->toModule();

				result &= addModule(module);
			}
		);

		if (result == true)
		{
			LOG_MESSAGE(m_log, QString(tr("Opto-modules found: %1")).arg(m_modules.count()))
			LOG_SUCCESS(m_log, QString(tr("Ok")));
		}

		return result;
	}

	bool OptoModuleStorage::appendAndCheckConnections(const Hardware::ConnectionStorage& connectionStorage)
	{
		bool result = true;

		int count = connectionStorage.count();

		for(int i = 0; i < count; i++)
		{
			ConnectionShared connection = connectionStorage.get(i);

			if (connection == nullptr)
			{
				assert(false);
				continue;
			}

			if (m_connections.contains(connection->connectionID()) == false)
			{
				bool res = processConnection(connection);

				if (res == false)
				{
					result = false;
					continue;
				}

				m_connections.insert(connection->connectionID(), connection);
			}
			else
			{
				m_log->errALC5023(connection->connectionID());
				result = false;
				break;
			}
		}

		for(OptoPortShared& port : m_ports)
		{
			TEST_PTR_CONTINUE(port);

			port->calculatePortRawDataSize(this);		// set m_txRawDataSizeW !
		}

		return result;
	}

	bool OptoModuleStorage::processConnection(ConnectionShared connection)
	{
		if (connection == nullptr)
		{
			ASSERT_RETURN_FALSE;
		}

		if (connection->manualSettings() == true)
		{
			m_log->wrnALC5055(connection->connectionID());
		}

		quint16 linkID = connection->linkID();

		QString connectionID = connection->connectionID();

		Hardware::OptoPortShared optoPort1 = getOptoPort(connection->port1EquipmentID());

		// check port 1
		//
		if (optoPort1 == nullptr)
		{
			// Undefined opto port '%1' in the connection '%2'.
			//
			m_log->errALC5021(connection->port1EquipmentID(), connection->connectionID());
			return false;
		}

		if (optoPort1->connectionID().isEmpty() == true)
		{
			optoPort1->setConnectionID(connectionID);
		}
		else
		{
			// Opto port '%1' of connection '%2' is already used in connection '%3'.
			//
			m_log->errALC5019(optoPort1->equipmentID(), connectionID, optoPort1->connectionID());
			return false;
		}

		Hardware::OptoModuleShared optoModule = getOptoModule(optoPort1);

		if (optoModule == nullptr)
		{
			assert(false);
			return false;
		}

		if (connection->isSinglePort() == true)
		{
			bool res = optoPort1->initSettings(connection);

			if (res == false)
			{
				return false;
			}

			LOG_MESSAGE(m_log, QString(tr("Single port connection '%1' ID = %2... Ok")).
							arg(connectionID).arg(linkID));
		}
		else
		{
			assert(connection->isPortToPort() == true);

			// check port 2
			//
			Hardware::OptoPortShared optoPort2 = getOptoPort(connection->port2EquipmentID());

			if (optoPort2 == nullptr)
			{
				// Undefined opto port '%1' in the connection '%2'.
				//
				m_log->errALC5021(connection->port2EquipmentID(), connectionID);
				return false;
			}

			if (optoPort1->lmID() == optoPort2->lmID())
			{
				//  Opto ports of the same chassis is linked via connection '%1'.
				//
				m_log->errALC5022(connectionID);
				return false;
			}

			if (optoPort2->connectionID().isEmpty() == true)
			{
				optoPort2->setConnectionID(connectionID);
			}
			else
			{
				// Opto port '%1' of connection '%2' is already used in connection '%3'.
				//
				m_log->errALC5019(optoPort2->equipmentID(), connectionID, optoPort2->connectionID());
				return false;
			}

			bool res = true;

			res &= optoPort1->initSettings(connection);
			res &= optoPort2->initSettings(connection);

			if (res == false)
			{
				return false;
			}

			if (connection->manualSettings() == true)
			{
				if (optoPort1->manualRxSizeW() != optoPort2->manualTxSizeW())
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
									   QString(tr("Manual rxDataSizeW of port '%1' is not equal to manual txDataSizeW of linked port '%2' (connection %3)")).
									   arg(optoPort1->equipmentID()).
									   arg(optoPort2->equipmentID()).
									   arg(optoPort1->connectionID()));
					return false;
				}

				if (optoPort2->manualRxSizeW() != optoPort1->manualTxSizeW())
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
									   QString(tr("Manual rxDataSizeW of port '%1' is not equal to manual txDataSizeW of linked port '%2' (connection %3)")).
									   arg(optoPort2->equipmentID()).
									   arg(optoPort1->equipmentID()).
									   arg(optoPort1->connectionID()));
					return false;
				}

				if (optoPort1->manualTxSizeW() < OptoPort::TX_DATA_ID_SIZE_W)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
									   QString(tr("Manual txDataSizeW of port '%1' should be greate or equal %2 (connection %3)")).
									   arg(optoPort1->equipmentID()).
									   arg(OptoPort::TX_DATA_ID_SIZE_W).
									   arg(optoPort1->connectionID()));
					return false;
				}

				if (optoPort2->manualTxSizeW() < OptoPort::TX_DATA_ID_SIZE_W)
				{
					LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
									   QString(tr("Manual txDataSizeW of port '%1' should be greate or equal %2 (connection %3)")).
									   arg(optoPort2->equipmentID()).
									   arg(OptoPort::TX_DATA_ID_SIZE_W).
									   arg(optoPort2->connectionID()));
					return false;
				}
			}

			LOG_MESSAGE(m_log, QString(tr("Optical connection '%1' ID = %2... Ok")).
						arg(connectionID).arg(linkID));
		}

		return true;
	}

	bool OptoModuleStorage::sortTxSignals(const QString& lmID)
	{
		return forEachPortOfLmAssociatedOptoModules(lmID, &OptoPort::sortTxSignals);
	}

	bool OptoModuleStorage::sortSerialRxSignals(const QString& lmID)
	{
		return forEachPortOfLmAssociatedOptoModules(lmID, &OptoPort::sortSerialRxSignals);
	}

	bool OptoModuleStorage::initRawTxSignals(const QString& lmID)
	{
		return forEachPortOfLmAssociatedOptoModules(lmID, &OptoPort::initRawTxSignals);
	}

	bool OptoModuleStorage::calculateTxSignalsAddresses(const QString& lmID)
	{
		return forEachPortOfLmAssociatedOptoModules(lmID, &OptoPort::calculateTxSignalsAddresses);
	}

	bool OptoModuleStorage::calculateTxDataIDs(const QString& lmID)
	{
		return forEachPortOfLmAssociatedOptoModules(lmID, &OptoPort::calculateTxDataID);
	}

	bool OptoModuleStorage::calculateTxBufAddresses(const QString& lmID)
	{
		return forEachOfLmAssociatedOptoModules(lmID, &OptoModule::calculateTxBufAddresses);
	}

	bool OptoModuleStorage::initSerialRawRxSignals(const QString& lmID)
	{
		return forEachPortOfLmAssociatedOptoModules(lmID, &OptoPort::initSerialRawRxSignals);
	}

	bool OptoModuleStorage::calculateSerialRxSignalsAddresses(const QString& lmID)
	{
		return forEachPortOfLmAssociatedOptoModules(lmID, &OptoPort::calculateSerialRxSignalsAddresses);
	}

	bool OptoModuleStorage::calculateSerialRxDataIDs(const QString& lmID)
	{
		return forEachPortOfLmAssociatedOptoModules(lmID, &OptoPort::calculateSerialRxDataID);
	}

	bool OptoModuleStorage::calculateRxBufAddresses(const QString &lmID)
	{
		return forEachOfLmAssociatedOptoModules(lmID, &OptoModule::calculateRxBufAddresses);
	}

	bool OptoModuleStorage::copyOpticalPortsTxInRxSignals(const QString& lmID)
	{
		return forEachPortOfLmAssociatedOptoModules(lmID, &OptoPort::copyOpticalPortsTxInRxSignals);
	}

	bool OptoModuleStorage::writeSerialDataXml(Builder::BuildResultWriter* resultWriter)
	{
		TEST_PTR_RETURN_FALSE(resultWriter);

		bool result = true;

		for(const OptoPortShared& port : m_ports)
		{
			if (port == nullptr)
			{
				assert(false);
				return false;
			}

			if (port->isSinglePortConnection() == true)
			{
				result &= port->writeSerialDataXml(resultWriter);
			}
		}

		return result;
	}

	bool OptoModuleStorage::appendTxSignal(const QString& schemaID,
										const QString& connectionID,
										QUuid transmitterUuid,
										const QString& lmID,
										const Builder::UalSignal* ualSignal,
										bool* signalAlreadyInList)
	{
		if (ualSignal == nullptr ||
			signalAlreadyInList == nullptr)
		{
			LOG_NULLPTR_ERROR(m_log);
			return false;
		}

		*signalAlreadyInList = false;

		std::shared_ptr<Hardware::Connection> cn = getConnection(connectionID);

		if (cn == nullptr)
		{
			m_log->errALC5024(connectionID, transmitterUuid, schemaID);
			return false;
		}

		if (cn->isSinglePort() == true)
		{
			OptoPortShared p1 = getOptoPort(cn->port1EquipmentID());

			if (p1 == nullptr)
			{
				assert(false);
				return false;
			}

			OptoModuleShared m1 = getOptoModule(p1);

			if (m1 == nullptr)
			{
				assert(false);
				return false;
			}

			if (m1->lmID() == lmID)
			{
				bool result = true;

				if (p1->isTxSignalExists(ualSignal))
				{
					*signalAlreadyInList = true;
				}
				else
				{
					result = p1->appendTxSignal(ualSignal);
				}
				return result;
			}

			// Ports of connection '%1' are not accessible in LM '%2.
			//
			m_log->errALC5059(schemaID, connectionID, lmID, transmitterUuid);

			return false;
		}

		// Connection is Optical
		//
		OptoPortShared p1 = getOptoPort(cn->port1EquipmentID());
		OptoPortShared p2 = getOptoPort(cn->port2EquipmentID());

		if (p1 == nullptr || p2 == nullptr)
		{
			assert(false);
			return false;
		}

		OptoModuleShared m1 = getOptoModule(p1);
		OptoModuleShared m2 = getOptoModule(p2);

		if (m1 == nullptr || m2 == nullptr)
		{
			assert(false);
			return false;
		}

		assert(m1->lmID() != m2->lmID());

		bool result = true;

		if (m1->lmID() == lmID)
		{
			if (p1->isTxSignalExists(ualSignal))
			{
				*signalAlreadyInList = true;
			}
			else
			{
				result = p1->appendTxSignal(ualSignal);
			}

			return result;
		}

		if (m2->lmID() == lmID)
		{
			if (p2->isTxSignalExists(ualSignal))
			{
				*signalAlreadyInList = true;
			}
			else
			{
				result = p2->appendTxSignal(ualSignal);
			}

			return result;
		}

		// Ports of connection '%1' are not accessible in LM '%2.
		//
		m_log->errALC5059(schemaID, connectionID, lmID, transmitterUuid);

		return false;
	}

	bool OptoModuleStorage::appendSerialRxSignal(const QString& schemaID,
													 const QString& connectionID,
													 QUuid receiverUuid,
													 const QString& lmID,
													 const Builder::UalSignal* ualSignal)
	{
		if (ualSignal == nullptr)
		{
			ASSERT_RETURN_FALSE;
		}

		std::shared_ptr<Hardware::Connection> cn = getConnection(connectionID);

		if (cn == nullptr)
		{
			m_log->errALC5025(connectionID, receiverUuid, schemaID);
			ASSERT_RETURN_FALSE;
		}

		OptoPortShared p1 = getOptoPort(cn->port1EquipmentID());

		if (p1 == nullptr)
		{
			ASSERT_RETURN_FALSE;
		}

		OptoModuleShared m1 = getOptoModule(p1);

		if (m1 == nullptr)
		{
			ASSERT_RETURN_FALSE;
		}

		if (m1->lmID() != lmID)
		{
			// Ports of connection '%1' are not accessible in LM '%2.
			//
			m_log->errALC5059(schemaID, connectionID, lmID, receiverUuid);

			return false;
		}

		bool result = p1->appendSerialRxSignal(ualSignal);

		return result;
	}

	bool OptoModuleStorage::getRxSignalAbsAddress(const QString& schemaID,
												  const QString& connectionID,
												  const QString& appSignalID,
												  const QString& receiverLM,
												  QUuid receiverUuid,
												  SignalAddress16 &addr)
	{
		addr.reset();

		std::shared_ptr<Connection> connection = getConnection(connectionID);

		if (connection == nullptr)
		{
			m_log->errALC5040(connectionID, receiverUuid);
			return false;
		}

		OptoPortShared p1 = getOptoPort(connection->port1EquipmentID());

		if (p1 == nullptr)
		{
			ASSERT_RETURN_FALSE;
		}

		if (p1->lmID() == receiverLM)
		{
			if (p1->isRxSignalExists(appSignalID) == true)
			{
				return p1->getRxSignalAbsAddress(appSignalID, addr);
			}
			else
			{
				m_log->errALC5042(appSignalID, connectionID, receiverUuid, schemaID);
				return false;
			}
		}

		if (connection->isSinglePort() == true)
		{
			// this is Serial connection
			// port2 is not used
			// signal is not found
			//
			m_log->errALC5042(appSignalID, connectionID, receiverUuid, schemaID);
			return false;
		}

		OptoPortShared p2 = getOptoPort(connection->port2EquipmentID());

		if (p2 == nullptr)
		{
			ASSERT_RETURN_FALSE;
		}

		if (p2->lmID() == receiverLM)
		{
			if (p2->isRxSignalExists(appSignalID) == true)
			{
				return p2->getRxSignalAbsAddress(appSignalID, addr);
			}
			else
			{
				m_log->errALC5042(appSignalID, connectionID, receiverUuid, schemaID);
				return false;
			}
		}

		ASSERT_RETURN_FALSE;			// signal is not found in both ports
	}

	std::shared_ptr<Connection> OptoModuleStorage::getConnection(const QString& connectionID)
	{
		return m_connections.value(connectionID, nullptr);
	}

	OptoModuleShared OptoModuleStorage::getOptoModule(const QString& optoModuleID)
	{
		return m_modules.value(optoModuleID, nullptr);
	}

	OptoModuleShared OptoModuleStorage::getOptoModule(const OptoPortShared optoPort)
	{
		if (optoPort == nullptr)
		{
			assert(false);
			return nullptr;
		}

		return m_modules.value(optoPort->optoModuleID(), nullptr);
	}

	QList<OptoModuleShared> OptoModuleStorage::getLmAssociatedOptoModules(const QString& lmID)
	{
		return m_lmAssociatedModules.values(lmID);
	}

	void OptoModuleStorage::getOptoModulesSorted(QVector<OptoModuleShared>& modules)
	{
		// return all opto modules sorted by equipmentID ascending alphabetical order
		//
		modules.clear();

		for(OptoModuleShared optoModule : m_modules)
		{
			if (optoModule == nullptr)
			{
				assert(false);
				continue;
			}

			modules.append(optoModule);
		}

		int count = modules.count();

		for(int i = 0; i < count - 1; i++)
		{
			for(int k = i + 1; k < count; k++)
			{
				if (modules[i]->equipmentID() > modules[k]->equipmentID())
				{
					OptoModuleShared temp = modules[i];
					modules[i] = modules[k];
					modules[k] = temp;
				}
			}
		}
	}

	OptoPortShared OptoModuleStorage::getOptoPort(const QString& optoPortID)
	{
		return m_ports.value(optoPortID, nullptr);
	}

	bool OptoModuleStorage::getLmAssociatedOptoPorts(const QString& lmID, QList<OptoPortShared>& associatedPorts)
	{
		QList<OptoModuleShared> modules = m_lmAssociatedModules.values(lmID);

		for(OptoModuleShared& module : modules)
		{
			if (module == nullptr)
			{
				assert(false);
				continue;
			}

			const HashedVector<QString, Hardware::OptoPortShared>& ports = module->ports();

			for(const Hardware::OptoPortShared& port : ports)
			{
				associatedPorts.append(port);
			}
		}

		return true;
	}

	QString OptoModuleStorage::getOptoPortAssociatedLmID(OptoPortShared optoPort)
	{
		if (optoPort == nullptr)
		{
			assert(false);
			return "";
		}

		OptoModuleShared optoModule = getOptoModule(optoPort);

		if (optoModule == nullptr)
		{
			assert(false);
			return "";
		}

		return optoModule->lmID();
	}

	bool OptoModuleStorage::getOptoPortValidityAbsAddr(const QString& lmID,
													   const QString& connectionID,
													   const QString& schemaID,
													   QUuid receiverUuid,
													   Address16& validityAddr)
	{
		ConnectionShared connection = getConnection(connectionID);

		if (connection == nullptr)
		{
			m_log->errALC5040(connectionID, QUuid());
			return false;
		}

		OptoPortShared p1 = getOptoPort(connection->port1EquipmentID());

		if (p1 == nullptr)
		{
			m_log->errALC5021(connection->port1EquipmentID(), connection->connectionID());
			return false;
		}

		if (p1->lmID() == lmID)
		{
			validityAddr = p1->validitySignalAbsAddr();
			return true;
		}

		if (connection->isSinglePort() == true)
		{
			// in serial connections port2 isn't used
			return false;
		}

		OptoPortShared p2 = getOptoPort(connection->port2EquipmentID());

		if (p2 == nullptr)
		{
			m_log->errALC5021(connection->port2EquipmentID(), connection->connectionID());
			return false;
		}

		if (p2->lmID() == lmID)
		{
			validityAddr = p2->validitySignalAbsAddr();
			return true;
		}

		m_log->errALC5059(schemaID, connectionID, lmID, receiverUuid);
		return false;
	}

	OptoPort* OptoModuleStorage::jsGetOptoPort(const QString& optoPortStrID)
	{
		OptoPortShared port = getOptoPort(optoPortStrID);

		if (port != nullptr)
		{
			OptoPort* portPtr = port.get();

			QQmlEngine::setObjectOwnership(portPtr, QQmlEngine::ObjectOwnership::CppOwnership);
			return portPtr;
		}

		return nullptr;
	}

	bool OptoModuleStorage::isSerialRxSignalExists(const QString& lmID, const QString& appSignalID)
	{
		QList<OptoModuleShared> modules = getLmAssociatedOptoModules(lmID);

		for(OptoModuleShared& module : modules)
		{
			if (module == nullptr)
			{
				ASSERT_RETURN_FALSE;
			}

			if (module->isSerialRxSignalExists(appSignalID) == true)
			{
				return true;
			}
		}

		return false;
	}

	bool OptoModuleStorage::processBvbModules()
	{
		bool result = true;

		for(OptoModuleShared module : m_modules)
		{
			if (module->isBvb() == false)
			{
				continue;
			}

			result &= module->calculateTxSignalsAddresses();
			result &= module->calculateTxBufAddresses();
		}

		// Not unite this cycles!!!!
		// calculateTxSignalsAddresses() must be called for ALL bvb modules before calling copyOpticalPortsTxInRxSignals()

		for(OptoModuleShared module : m_modules)
		{
			if (module->isBvb() == false)
			{
				continue;
			}

			result &= module->copyOpticalPortsTxInRxSignals();
			result &= module->calculateRxBufAddresses();
		}

		return result;
	}

	bool OptoModuleStorage::addModule(DeviceModule* module)
	{
		if (module == nullptr)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		if (module->isLogicModule() != true &&
			module->isOptoModule() != true &&
			module->isBvb() != true)
		{
			// this is not opto-module
			//
			return true;
		}

		// Get LogicModule description
		//
		assert(m_lmDescriptionSet);

		const DeviceModule* chassisLm = DeviceHelper::getAssociatedLmOrBvb(module);

		if (chassisLm == nullptr)
		{
			assert(chassisLm);
			return false;
		}

		std::shared_ptr<LogicModule> lmDescription = m_lmDescriptionSet->get(chassisLm);

		if (lmDescription == nullptr)
		{
			QString lmDescriptionFile = LogicModule::lmDescriptionFile(module);

			m_log->errEQP6004(module->equipmentIdTemplate(), lmDescriptionFile, module->uuid());
			return false;
		}

		// Create and add OptoModule
		//
		OptoModuleShared optoModule = std::make_shared<OptoModule>();

		if (optoModule->init(module, lmDescription.get(), m_log) == false)
		{
			return false;
		}

		m_modules.insert(module->equipmentIdTemplate(), optoModule);

		m_lmAssociatedModules.insertMulti(optoModule->lmID(), optoModule);

		const HashedVector<QString, OptoPortShared>& ports = optoModule->ports();

		for(const OptoPortShared& port : ports)
		{
			m_ports.insert(port->equipmentID(), port);
		}

		return true;
	}

	bool OptoModuleStorage::forEachPortOfLmAssociatedOptoModules(const QString& lmID, OptoPortFunc funcPtr)
	{
		QList<OptoModuleShared> modules = getLmAssociatedOptoModules(lmID);

		bool result = true;

		for(OptoModuleShared& module : modules)
		{
			if (module == nullptr)
			{
				ASSERT_RETURN_FALSE;
			}

			result &= module->forEachPort(funcPtr);
		}

		return result;
	}

	bool OptoModuleStorage::forEachOfLmAssociatedOptoModules(const QString& lmID, OptoModuleFunc funcPtr)
	{
		QList<OptoModuleShared> modules = getLmAssociatedOptoModules(lmID);

		bool result = true;

		for(OptoModuleShared& module : modules)
		{
			if (module == nullptr)
			{
				ASSERT_RETURN_FALSE;
			}

			OptoModule* modulePtr = module.get();

			result &= (modulePtr->*funcPtr)();
		}

		return result;
	}

	void OptoModuleStorage::clear()
	{
		m_modules.clear();
		m_ports.clear();
		m_lmAssociatedModules.clear();
		m_connections.clear();
	}

}
