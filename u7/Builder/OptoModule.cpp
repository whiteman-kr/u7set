#include "OptoModule.h"

#include "../LogicModule.h"
#include "../Builder/Builder.h"
#include "../Builder/ApplicationLogicCompiler.h"
#include "../lib/DeviceHelper.h"


namespace Hardware
{

	// ------------------------------------------------------------------
	//
	// TxRxSignal class implementation
	//
	// ------------------------------------------------------------------

	TxRxSignal::TxRxSignal()
	{
	}

	bool TxRxSignal::init(const QString& appSignalID,
						  E::SignalType signalType,
						  E::DataFormat dataFormat,
						  int dataSize,
						  E::ByteOrder byteOrder,
						  int offset,
						  int bitNo,
						  TxRxSignal::Type txRxType)
	{
		if (appSignalID == nullptr)
		{
			assert(false);
			return false;
		}

		m_appSignalID = appSignalID;
		m_signalType = signalType;

		if (m_signalType != E::SignalType::Analog && m_signalType != E::SignalType::Discrete)
		{
			assert(false);		// wtf?
			return false;
		}
		m_dataFormat = dataFormat;
		m_byteOrder = byteOrder;

		m_type = txRxType;

		if (isRaw() == true)
		{
			assert(offset >= 0 && offset < 65536);
			assert(bitNo >= 0 && bitNo < 16);

			m_addrInBuf.setOffset(offset);
			m_addrInBuf.setBit(bitNo);

			assert(dataSize >= SIZE_1BIT && dataSize <= SIZE_32BIT);

			m_dataSize = dataSize;
		}
		else
		{
			m_addrInBuf.reset();

			m_dataSize = dataSize;
		}

#ifdef QT_DEBUG
		if (isDiscrete() == true)
		{
			assert(m_dataSize == SIZE_1BIT);
		}
#endif
		return true;
	}

	void TxRxSignal::setAddrInBuf(Address16& addr)
	{
		m_addrInBuf = addr;
	}


	// ------------------------------------------------------------------
	//
	// OptoPort class implementation
	//
	// ------------------------------------------------------------------

	OptoPort::OptoPort()
	{
	}

	bool OptoPort::init(const DeviceController* controller, int portNo, Builder::IssueLogger* log)
	{
		if (m_controller == nullptr || m_log == nullptr)
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

		const DeviceModule* lm = DeviceHelper::getAssociatedLM(m_controller);

		if (lm == nullptr)
		{
			assert(false);
			return false;
		}

		m_lmID = lm->equipmentIdTemplate();

		return true;
	}

	bool OptoPort::appendRegularTxSignal(const Signal* txSignal)
	{
		if (txSignal == nullptr)
		{
			assert(false);
			return false;
		}

		return appendTxSignal(txSignal->appSignalID(),
							  txSignal->signalType(),
							  txSignal->dataFormat(),
							  txSignal->dataSize(),
							  txSignal->byteOrder(),
							  -1,
							  -1,
							  TxRxSignal::Type::Regular);
	}

	bool OptoPort::sortTxSignals()
	{
		return sortTxRxSignalList(m_txSignals);
	}

	bool OptoPort::sortSerialRxSignals()
	{
		if (isSerial() == false)
		{
			// on this step of execution m_rxSignals of Optical ports must be empty!
			//
			assert(m_rxSignals.count() == 0);
			return true;
		}

		return sortTxRxSignalList(m_rxSignals);
	}

	void OptoPort::getTxSignals(QVector<TxRxSignalShared>& txSignals) const
	{
		txSignals.clear();

		for(TxRxSignalShared s : m_txSignals)
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

		for(TxRxSignalShared s : m_txSignals)
		{
			if (s == nullptr)
			{
				assert(false);
				continue;
			}

			rxSignals.append(s);
		}
	}

	void OptoPort::getTxAnalogSignals(QVector<TxRxSignalShared>& txSignals) const
	{
		txSignals.clear();

		for(TxRxSignalShared txSignal : m_txSignals)
		{
			if (txSignal == nullptr)
			{
				assert(false);
				continue;
			}

			if (txSignal->isAnalog() == true)
			{
				txSignals.append(txSignal);
			}
		}
	}

	void OptoPort::getTxDiscreteSignals(QVector<TxRxSignalShared>& txSignals) const
	{
		txSignals.clear();

		for(TxRxSignalShared txSignal : m_txSignals)
		{
			if (txSignal == nullptr)
			{
				assert(false);
				continue;
			}

			if (txSignal->isDiscrete() == true)
			{
				txSignals.append(txSignal);
			}
		}
	}

	bool OptoPort::appendRawTxSignals(const HashedVector<QString, Signal*>& lmAssociatedSignals)
	{
		bool result = true;

		for(const RawDataDescriptionItem& item : m_rawDataDescription)
		{
			if (item.type != RawDataDescriptionItem::Type::TxSignal)
			{
				continue;
			}

			QString appSignalID = item.appSignalID;

			if (lmAssociatedSignals.contains(appSignalID) == false)
			{
				// Tx signal '%1' specified in opto port '%2' raw data description is not exists in LM '%3'.
				//
				m_log->errALC5189(appSignalID, m_equipmentID, m_lmID);
				result = false;
				continue;
			}

			if (m_txSignals.contains(appSignalID) == true)
			{
				// Signal ID '%1' is duplicate in opto port '%2' raw data description.
				//
				m_log->errALC5188(appSignalID, m_equipmentID);
				result = false;
			}

			result &= appendTxSignal(appSignalID,
									 item.signalType,
									 item.dataFormat,
									 item.dataSize,
									 item.byteOrder,
									 item.offsetW,
									 item.bitNo,
									 TxRxSignal::Type::Raw);
		}

		return result;
	}


	bool OptoPort::appendSerialRawRxSignals(const HashedVector<QString, Signal *>& lmAssociatedSignals)
	{
		if (m_mode != OptoPort::Mode::Serial)
		{
			return true;				// add raw signals of serial ports only
		}

		bool result = true;

		for(const RawDataDescriptionItem& item : m_rawDataDescription)
		{
			if (item.type != RawDataDescriptionItem::Type::RxSignal)
			{
				continue;
			}

			QString appSignalID = item.appSignalID;

			if (lmAssociatedSignals.contains(appSignalID) == false)
			{
				// Rx signal '%1' specified in opto port '%2' raw data description is not exists in LM '%3'.
				//
				m_log->errALC5190(appSignalID, m_equipmentID, m_lmID);
				result = false;
				continue;
			}

			if (m_rxSignals.contains(appSignalID) == true)
			{
				// Signal ID '%1' is duplicate in opto port '%2' raw data description.
				//
				m_log->errALC5188(appSignalID, m_equipmentID);
				result = false;
			}

			result &= appendRxSignal(appSignalID,
									 item.signalType,
									 item.dataFormat,
									 item.dataSize,
									 item.byteOrder,
									 item.offsetW,
									 item.bitNo,
									 TxRxSignal::Type::Raw);
		}

		return result;
	}

	bool OptoPort::appendSerialRegularRxSignal(const Signal* rxSignal)
	{
		TEST_PTR_RETURN_FALSE(rxSignal);

		if (isSerial() == false)
		{
			assert(false);				// port is not Serial!
			return false;
		}

		if (m_rxSignals.contains(rxSignal->appSignalID()) == true)
		{
			return true;				// signal already in list, nothing to do
		}

		bool result = appendRxSignal(rxSignal->appSignalID(),
									 rxSignal->signalType(),
									 rxSignal->dataFormat(),
									 rxSignal->dataSize(),
									 rxSignal->byteOrder(),
									 -1,
									 -1,
									 TxRxSignal::Type::Regular);

		return result;
	}

	// initial txSignals addresses calculcation
	// zero-offset from port txStartAddress
	//
	bool OptoPort::calculateTxSignalsAddresses()
	{
		m_txDataSizeW = 0;
		m_txAnalogSignalsSizeW = 0;
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

		address.addWord(m_txRawDataSizeW);

		int startAddr = address.offset();

		// then place analog signals
		//
		for(TxRxSignalShared& txAnalogSignal : m_txSignals)
		{
			if (txAnalogSignal->isRaw() == true)
			{
				continue;					// exclude raw signals
			}

			if (txAnalogSignal->isDiscrete() == true)
			{
				break;						// no more analog signals, exit
			}

			txAnalogSignal->setAddrInBuf(address);

			address.addBit(txAnalogSignal->dataSize());
			address.wordAlign();
		}

		m_txAnalogSignalsSizeW = address.offset() - startAddr ;

		startAddr = address.offset();

		// then place discrete signals
		//
		for(TxRxSignalShared& txDiscreteSignal : m_txSignals)
		{
			if (txDiscreteSignal->isRaw() == true)
			{
				continue;					// exclude raw signals
			}

			if (txDiscreteSignal->isAnalog() == true)
			{
				continue;					// skip analog signals
			}

			txDiscreteSignal->setAddrInBuf(address);

			assert(txDiscreteSignal->dataSize() == SIZE_1BIT);

			address.add1Bit();
		}

		address.wordAlign();

		m_txDiscreteSignalsSizeW = address.offset() - startAddr;

		int fullTxDataSizeW = TX_DATA_ID_SIZE_W + m_txRawDataSizeW + m_txAnalogSignalsSizeW + m_txDiscreteSignalsSizeW;

		if (manualSettings() == true)
		{
			m_txDataSizeW = m_manualTxSizeW;

			if (fullTxDataSizeW > m_txDataSizeW)
			{
				LOG_MESSAGE(m_log, QString(tr("Port %1: txIdSizeW = %2, txRawDataSizeW = %3, txAnalogSignalsSizeW = %4, txDiscreteSignalsSizeW = %5")).
									arg(m_equipmentID).arg(TX_DATA_ID_SIZE_W).arg(m_txRawDataSizeW).arg(m_txAnalogSignalsSizeW).arg(m_txDiscreteSignalsSizeW));
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
								   QString(tr("Manual txDataSizeW - %1 less then needed size %2 (port %3, connection %4)")).
								   arg(m_manualTxSizeW).arg(fullTxDataSizeW).
								   arg(m_equipmentID).arg(m_connectionID));

				return false;
			}
		}
		else
		{
			m_txDataSizeW = fullTxDataSizeW;
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

	bool OptoPort::calculateSerialRxSignalsAddresses()
	{
		if (isUsedInConnection() == false)
		{
			return true;
		}

		if (m_mode != OptoPort::Mode::Serial)
		{
			return true;				// process Serial ports only
		}

		m_rxDataSizeW = 0;
		m_rxAnalogSignalsSizeW = 0;
		m_rxDiscreteSignalsSizeW = 0;

		// check raw signals addresses
		//
		for(TxRxSignalShared& rxSignal : m_rxSignals)
		{
			if (rxSignal->isRaw() == false)
			{
				continue;
			}

			if (rxSignal->addrInBuf().offset() < 0 || rxSignal->addrInBuf().offset() >= m_rxRawDataSizeW)
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
			if (rxAnalogSignal->isRaw() == true)
			{
				continue;					// exclude raw signals
			}

			if (rxAnalogSignal->isDiscrete() == true)
			{
				break;						// no more analog signals, exit
			}

			rxAnalogSignal->setAddrInBuf(address);

			address.addBit(rxAnalogSignal->dataSize());
			address.wordAlign();
		}

		m_rxAnalogSignalsSizeW = address.offset() - startAddr ;

		startAddr = address.offset();

		// then place discrete signals
		//
		for(TxRxSignalShared& rxDiscreteSignal : m_rxSignals)
		{
			if (rxDiscreteSignal->isRaw() == true)
			{
				continue;					// exclude raw signals
			}

			if (rxDiscreteSignal->isAnalog() == true)
			{
				continue;					// skip analog signals
			}

			rxDiscreteSignal->setAddrInBuf(address);

			assert(rxDiscreteSignal->dataSize() == SIZE_1BIT);

			address.add1Bit();
		}

		address.wordAlign();

		m_rxDiscreteSignalsSizeW = address.offset() - startAddr;

		int fullRxDataSizeW = TX_DATA_ID_SIZE_W + m_rxRawDataSizeW + m_rxAnalogSignalsSizeW + m_rxDiscreteSignalsSizeW;

		if (manualSettings() == true)
		{
			m_rxDataSizeW = m_manualTxSizeW;

			if (fullRxDataSizeW > m_rxDataSizeW)
			{
				LOG_MESSAGE(m_log, QString(tr("Port %1: rxIdSizeW = %2, rxRawDataSizeW = %3, rxAnalogSignalsSizeW = %4, rxDiscreteSignalsSizeW = %5")).
									arg(m_equipmentID).arg(TX_DATA_ID_SIZE_W).arg(m_txRawDataSizeW).arg(m_txAnalogSignalsSizeW).arg(m_txDiscreteSignalsSizeW));
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
								   QString(tr("Manual rxDataSizeW - %1 less then needed size %2 (port %3, connection %4)")).
								   arg(m_manualRxSizeW).arg(fullRxDataSizeW).
								   arg(m_equipmentID).arg(m_connectionID));

				return false;
			}
		}
		else
		{
			m_rxDataSizeW = fullRxDataSizeW;
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

		if (m_mode != OptoPort::Mode::Serial)
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

		if (m_mode != OptoPort::Mode::Optical)
		{
			return true;
		}

		assert(m_rxSignals.count() == 0);				// before this m_rxSignals must be clear!

		OptoPortShared linkedPort = OptoModuleStorage::getOptoPort(m_linkedPortID);

		if (linkedPort == nullptr)
		{
			ASSERT_RETURN_FALSE;
		}

		// copying txSignals of linked port to rxSignals of current port
		//
		const HashedVector<QString, TxRxSignalShared>& txSignals = linkedPort->txSignals();

		for(const TxRxSignalShared& txSignal : txSignals)
		{
			if (txSignal == nullptr)
			{
				ASSERT_RETURN_FALSE;
			}

			m_rxSignals.insert(txSignal->appSignalID(), txSignal);
		}

		// copying tx to rx parameters
		//
		m_rxDataID = linkedPort->txDataID();
		m_rxDataSizeW = linkedPort->txDataSizeW();
		m_rxRawDataSizeW = linkedPort->txRawDataSizeW();
		m_rxAnalogSignalsSizeW = linkedPort->txAnalogSignalsSizeW();
		m_rxDiscreteSignalsSizeW = linkedPort->txDiscreteSignalsSizeW();

		return true;
	}


	bool OptoPort::isTxSignalExists(const QString& appSignalID)
	{
		return m_txSignals.contains(appSignalID);
	}

	bool OptoPort::isRxSignalExists(const QString& appSignalID)
	{
		return m_rxSignals.contains(appSignalID);
	}

	bool OptoPort::isUsedInConnection() const
	{
		return m_connectionID.isEmpty() != true;
	}

	Address16 OptoPort::getTxSignalAddrInBuf(const QString& appSignalID) const
	{
		TxRxSignalShared s = m_txSignals.value(appSignalID, nullptr);

		if (s != nullptr)
		{
			return s->addrInBuf();
		}

		assert(false);

		return Address16();
	}

	bool OptoPort::getTxSignalAbsAddr(const QString& appSignalID, SignalAddress16& addr) const
	{
		TxRxSignalShared txSignal = m_txSignals.value(appSignalID);

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
	}

	Address16 OptoPort::getRxSignalAbsAddr(const QString& appSignalID) const
	{
		TxRxSignalShared rxSignal = m_rxSignals.value(appSignalID);

		if (rxSignal == nullptr)
		{
			assert(false);
			return Address16();
		}

		int absRxBufAddr = rxBufAbsAddress();

		if (absRxBufAddr == BAD_ADDRESS)
		{
			assert(false);
			return Address16();
		}

		Address16 absAddr = rxSignal->addrInBuf();

		absAddr.addWord(absRxBufAddr);

		return absAddr;
	}


	bool OptoPort::parseRawDescription()
	{
		bool result = m_rawDataDescription.parse(*this, m_log);

		if (result == false)
		{
			return false;
		}

		return result;
	}

	bool OptoPort::calculatePortRawDataSize(OptoModuleStorage* optoStorage)
	{
		const DeviceModule* lm = DeviceHelper::getAssociatedLM(m_controller);

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

		return module->optoInterfaceDataOffset() + module->optoPortAppDataOffset() + m_rxBufAddress;
	}

	void OptoPort::setTxRawDataSizeW(int rawDataSizeW)
	{
		assert(m_txRawDataSizeWIsCalculated == false);		// setTxRawDataSizeW() must be called once!

		m_txRawDataSizeW = rawDataSizeW;

		m_txRawDataSizeWIsCalculated = true;
	}

	bool OptoPort::appendTxSignal(const QString& appSignalID,
								  E::SignalType signalType,
								  E::DataFormat dataFormat,
								  int dataSize,
								  E::ByteOrder byteOrder,
								  int offset,
								  int bitNo,
								  TxRxSignal::Type type)
	{
		if (m_txSignals.contains(appSignalID) == true)
		{
			return false;
		}

		TxRxSignalShared txSignal = std::make_shared<TxRxSignal>();

		bool res = txSignal->init(appSignalID, signalType, dataFormat, dataSize, byteOrder, offset, bitNo, type);

		if (res == false)
		{
			return false;
		}

		m_txSignals.insert(txSignal->appSignalID(), txSignal);

		return true;
	}

	bool OptoPort::appendRxSignal(const QString& appSignalID,
								  E::SignalType signalType,
								  E::DataFormat dataFormat,
								  int dataSize,
								  E::ByteOrder byteOrder,
								  int offset,
								  int bitNo,
								  TxRxSignal::Type type)
	{
		if (m_rxSignals.contains(appSignalID) == true)
		{
			// Signal ID '%1' is duplicate in opto port '%2'.
			//
			m_log->errALC5188(appSignalID, m_equipmentID);

			return false;
		}

		TxRxSignalShared rxSignal = std::make_shared<TxRxSignal>();

		bool res = rxSignal->init(appSignalID, signalType, dataFormat, dataSize, byteOrder, offset, bitNo, type);

		if (res == false)
		{
			return false;
		}

		m_rxSignals.insert(rxSignal->appSignalID(), rxSignal);

		return true;
	}
•
	void OptoPort::sortByOffsetBitNoAscending(HashedVector<QString, TxRxSignalShared> &signalList, int startIndex, int count)
	{
		for(int i = startIndex; i < startIndex + count - 1; i++)
		{
			for(int k = i + 1; k < startIndex + count; k++)
			{
				if (signalList[i]->addrInBuf().bitAddress() > signalList[k]->addrInBuf().bitAddress())
				{
					TxRxSignalShared temp = signalList[i];
					signalList[i] = signalList[k];
					signalList[k] = temp;
				}
			}
		}
	}

	void OptoPort::sortByAppSignalIdAscending(HashedVector<QString, TxRxSignalShared>& signalList, int startIndex, int count)
	{
		for(int i = startIndex; i < startIndex + count - 1; i++)
		{
			for(int k = i + 1; k < startIndex + count; k++)
			{
				if (signalList[i]->appSignalID() > signalList[k]->appSignalID())
				{
					TxRxSignalShared temp = signalList[i];
					signalList[i] = signalList[k];
					signalList[k] = temp;
				}
			}
		}
	}

	bool OptoPort::checkSignalsOffsets(const HashedVector<QString, TxRxSignalShared>& signalList, int startIndex, int count)
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

/*	void OptoPort::setRawDataSizeIsCalculated()
	{
		assert(m_rawDataSizeIsCalculated == false);		// setRawDataSizeIsCalculated() must be called once!

		m_rawDataSizeIsCalculated = true;
	}*/


	bool OptoPort::sortTxRxSignalList(HashedVector<QString, TxRxSignalShared>& signalList)
	{
		// Tx/Rx signals sorting order:
		//
		// 1. All Raw signals (on addrInBuf ascending)
		// 2. All Regular Analog signals (on appSignalID ascending)
		// 3. All Regular Discrete signals (on appSignalID ascending)

		HashedVector<QString, TxRxSignalShared> tempSignalList = signalList;

		int count = tempSignalList.count();

		signalList.clear();

		// 1. Fetch and sort all Raw Tx signals
		//
		int rawSignalCount = 0;

		for(int i = 0; i < count; i++)
		{
			TxRxSignalShared& s = tempSignalList[i];

			if (s->isRaw() == true)
			{
				signalList.insert(s->appSignalID(), s);
				rawSignalCount++;
			}
		}

		sortByOffsetBitNoAscending(signalList, 0, rawSignalCount);

		if (checkSignalsOffsets(signalList, 0, rawSignalCount) == false)
		{
			return false;
		}

		// 2. Fetch and sort all regular analog Tx signals
		//
		int analogSignalFirstIndex = signalList.count();
		int analogSignalCount = 0;

		for(int i = 0; i < count; i++)
		{
			TxRxSignalShared& s = tempSignalList[i];

			if (s->isRegular() == true && s->isAnalog() == true)
			{
				signalList.insert(s->appSignalID(), s);

				analogSignalCount++;
			}
		}

		sortByAppSignalIdAscending(signalList, analogSignalFirstIndex, analogSignalCount);

		// 3. Fetch and sort all regular discrete Tx signals
		//
		int discreteSignalFirstIndex = signalList.count();
		int discreteSignalCount = 0;

		for(int i = 0; i < count; i++)
		{
			TxRxSignalShared& s = tempSignalList[i];

			if (s->isRegular() == true && s->isDiscrete() == true)
			{
				signalList.insert(s->appSignalID(), s);

				discreteSignalCount++;
			}
		}

		sortByAppSignalIdAscending(signalList, discreteSignalFirstIndex, discreteSignalCount);

		return true;
	}


	// ------------------------------------------------------------------
	//
	// OptoModule class implementation
	//
	// ------------------------------------------------------------------

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

		if (module->isLogicModule() == true)
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
		if (isOCM() == true)
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

					result &= optoPort->init(optoPortController, i + 1, log);

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

		if (isLM() == true)
		{
			m_lmID = module->equipmentIdTemplate();
			m_lm = module;
		}
		else
		{
			assert(isOCM() == true);

			const DeviceModule* lm = DeviceHelper::getAssociatedLM(module);

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

	bool OptoModule::isLM()
	{
		if (m_deviceModule == nullptr)
		{
			assert(false);
			return false;
		}

		return m_deviceModule->moduleFamily() == DeviceModule::FamilyType::LM;
	}

	bool OptoModule::isOCM()
	{
		if (m_deviceModule == nullptr)
		{
			assert(false);
			return false;
		}

		return m_deviceModule->moduleFamily() == DeviceModule::FamilyType::OCM;
	}


	void OptoModule::getSerialPorts(QList<OptoPortShared>& serialPortsList)
	{
		serialPortsList.clear();

		for(OptoPortShared& port : m_ports)
		{
			if (port == nullptr)
			{
				assert(false);
				continue;
			}

			if (port->mode() == OptoPort::Mode::Serial)
			{
				serialPortsList.append(port);
			}
		}
	}

	void OptoModule::getOptoPorts(QList<OptoPortShared>& optoPortsList)
	{
		optoPortsList.clear();

		for(OptoPortShared& port : m_ports)
		{
			if (port == nullptr)
			{
				assert(false);
				continue;
			}

			if (port->mode() == OptoPort::Mode::Optical)
			{
				optoPortsList.append(port);
			}
		}
	}

	void OptoModule::getPorts(QList<OptoPortShared>& portsList)
	{
		portsList.clear();

		for(OptoPortShared& port : m_ports)
		{
			if (port == nullptr)
			{
				assert(false);
				continue;
			}

			portsList.append(port);
		}
	}

	// return all ports sorted by equipmentID ascending alphabetical order
	//
/*	QVector<OptoPort*> OptoModule::getPortsSorted()
	{
		QVector<OptoPort*> ports;

		for(OptoPort* port : m_ports)
		{
			if (port == nullptr)
			{
				assert(false);
				continue;
			}

			ports.append(port);
		}

		sortPortsByEquipmentIDAscending(ports);

		return ports;
	}


	// return ports sorted by equipmentID ascending alphabetical order
	//
	QVector<OptoPort*> OptoModule::getOptoPortsSorted()
	{
		QVector<OptoPort*> ports;

		for(OptoPort* port : m_ports)
		{
			if (port == nullptr)
			{
				assert(false);
				continue;
			}

			ports.append(port);
		}

		sortPortsByEquipmentIDAscending(ports);

		return ports;
	}
*/

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


	bool OptoModule::calculateTxStartAddresses()
	{
		quint16 txStartAddress = 0;

		for(OptoPortShared& port : m_ports)
		{
			if (port == nullptr)
			{
				assert(false);
				continue;
			}

			if (port->manualSettings() == true)
			{
				port->setTxBufAddress(port->manualTxStartAddressW());
			}
			else
			{
				if (isLM() == true)
				{
					port->setTxBufAddress((port->portNo() - 1) * optoPortAppDataSize());
				}
				else
				{
					port->setTxBufAddress(txStartAddress);
					txStartAddress += port->txDataSizeW();
				}
			}
		}

		if (txStartAddress > m_optoPortAppDataSize)
		{
			LOG_WARNING_OBSOLETE(m_log, Builder::IssueType::NotDefined,
						  QString(tr("TxDataSize exceeded OptoPortAppDataSize in module '%1'")).
						  arg(m_equipmentID));
			return false;
		}

		return true;
	}

	bool OptoModule::addRawTxSignals(const HashedVector<QString, Signal*>& lmAssociatedSignals)
	{
		bool result = true;

		for(OptoPortShared& optoPort : m_ports)
		{
			if (optoPort == nullptr)
			{
				assert(false);
				return false;
			}

			result &= optoPort->appendRawTxSignals(lmAssociatedSignals);
		}

		return result;
	}

	bool OptoModule::addSerialRawRxSignals(const HashedVector<QString, Signal*>& lmAssociatedSignals)
	{
		bool result = true;

		for(OptoPortShared& optoPort : m_ports)
		{
			if (optoPort == nullptr)
			{
				assert(false);
				return false;
			}

			if (optoPort->mode() == OptoPort::Mode::Serial)
			{
				result &= optoPort->appendSerialRawRxSignals(lmAssociatedSignals);
			}
		}

		return result;
	}

	bool OptoModule::sortTxSignals()
	{
		bool result = true;

		for(OptoPortShared& port : m_ports)
		{
			if (port == nullptr)
			{
				ASSERT_RETURN_FALSE;
			}

			result &= port->sortTxSignals();
		}

		return result;
	}

	bool OptoModule::calculateTxSignalsAddresses()
	{
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

	bool OptoModule::calculateTxDataIDs()
	{
		bool result = true;

		for(OptoPortShared& port : m_ports)
		{
			if (port == nullptr)
			{
				ASSERT_RETURN_FALSE;
			}

			result &= port->calculateTxDataID();
		}

		return result;
	}

	bool OptoModule::calculateTxBuffersAbsAddresses()
	{
		bool result = true;

		int absTxStartAddress = 0;

		if (isLM() == true)
		{
			for(OptoPortShared& port : m_ports)
			{
				// calculate tx buffers absolute addresses for ports of LM module
				//
				if (port == nullptr)
				{
					LOG_INTERNAL_ERROR(m_log);
					ASSERT_RETURN_FALSE;
				}

				if (port->isUsedInConnection() == false)
				{
					continue;
				}

				absTxStartAddress =	optoInterfaceDataOffset() +	(port->portNo() - 1) * optoPortDataSize();

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

					absTxStartAddress += manualTxStartAddr;

					port->setAbsTxStartAddress(absTxStartAddress);

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
					port->setAbsTxStartAddress(absTxStartAddress);

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

		if (isOCM() == true)
		{
			// calculate tx addresses for ports of OCM module
			//
			absTxStartAddress = optoInterfaceDataOffset();

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
					absTxStartAddress = optoInterfaceDataOffset() + port->manualTxStartAddressW();

					port->setAbsTxStartAddress(absTxStartAddress);

					if (port->manualTxStartAddressW() + port->manualTxSizeW() > optoPortAppDataSize())
					{
						// TxData size (%1 words) of opto port '%2' exceed value of OptoPortAppDataSize property of module '%3' (%4 words).
						//
						m_log->errALC5032(port->txDataSizeW(), port->equipmentID(), equipmentID(), optoPortAppDataSize());
						return false;
					}

					// calculate TxStartAddr for next port with auto settings (if exists)
					//
					absTxStartAddress += port->manualTxSizeW();

					txDataSizeW = port->manualTxStartAddressW() + port->manualTxSizeW();
				}
				else
				{
					// all OCM's ports data disposed in one buffer with max size - OptoPortAppDataSize
					//
					port->setAbsTxStartAddress(absTxStartAddress);

					absTxStartAddress += port->txDataSizeW();

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

				result &= checkPortsAddressesOverlapping();
			}
		}

		// unknown module family
		//
		ASSERT_RETURN_FALSE;
	}

	// checking ports addresses overlapping
	// usefull for manual settings of ports
	//
	bool OptoModule::checkPortsAddressesOverlapping()
	{
		int portsCount = m_ports.count();

		for(int i = 0; i < portsCount - 1; i++)
		{
			OptoPortShared port1 = m_ports[i];

			if (port1 == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				assert(false);
				return false;
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
					return false;
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
					// Tx data memory areas of ports '%1' and '%2' are overlapped.
					//
					m_log->errALC5187(port1->equipmentID(), port2->equipmentID());

					return false;
				}
			}
		}

		return true;
	}


	// ------------------------------------------------------------------
	//
	// OptoModuleStorage class implementation
	//
	// ------------------------------------------------------------------

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
	}


	void OptoModuleStorage::clear()
	{
		m_modules.clear();
		m_ports.clear();
	}


	bool OptoModuleStorage::build()
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


	bool OptoModuleStorage::addModule(DeviceModule* module)
	{
		if (module == nullptr)
		{
			assert(false);
			LOG_INTERNAL_ERROR(m_log);
			return false;
		}

		if (module->moduleFamily() != DeviceModule::FamilyType::LM &&
			module->moduleFamily() != DeviceModule::FamilyType::OCM)
		{
			// this is not opto-module
			//
			return true;
		}

		// Get LogicModule description
		//
		assert(m_lmDescriptionSet);

		const DeviceModule* chassisLm = DeviceHelper::getAssociatedLM(module);
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

		for(OptoPortShared& optoPort : optoModule->m_ports)
		{
			m_ports.insert(optoPort->equipmentID(), optoPort);
		}

		return true;
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


	OptoPortShared OptoModuleStorage::getOptoPort(const QString& optoPortID)
	{
		return m_ports.value(optoPortID, nullptr);
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


	bool OptoModuleStorage::isCompatiblePorts(OptoPortShared optoPort1, OptoPortShared optoPort2)
	{
		if (optoPort1 == nullptr ||
			optoPort2 == nullptr)
		{
			assert(false);
			return false;
		}

		OptoModuleShared optoModule1 = getOptoModule(optoPort1);
		OptoModuleShared optoModule2 = getOptoModule(optoPort2);

		return  (optoModule1->isLM() == optoModule2->isLM()) ||
				(optoModule1->isOCM() == optoModule2->isOCM());
	}


	QList<OptoModuleShared> OptoModuleStorage::getLmAssociatedOptoModules(const QString& lmID)
	{
		return m_lmAssociatedModules.values(lmID);
	}

	bool OptoModuleStorage::appendRawTxSignals(const QString& lmID, const HashedVector<QString, Signal*>& lmAssociatedSignals)
	{
		QList<OptoModuleShared> optoModules = getLmAssociatedOptoModules(lmID);

		bool result = true;

		for(OptoModuleShared& optoModule : optoModules)
		{
			if (optoModule == nullptr)
			{
				assert(false);
				return false;
			}

			result &= optoModule->addRawTxSignals(lmAssociatedSignals);
		}

		return result;
	}

	bool OptoModuleStorage::appendSerialRawRxSignals(const QString& lmID, const HashedVector<QString, Signal*>& lmAssociatedSignals)
	{
		QList<OptoModuleShared> optoModules = getLmAssociatedOptoModules(lmID);

		bool result = true;

		for(OptoModuleShared& optoModule : optoModules)
		{
			if (optoModule == nullptr)
			{
				assert(false);
				return false;
			}

			result &= optoModule->addSerialRawRxSignals(lmAssociatedSignals);
		}

		return result;
	}

	bool OptoModuleStorage::sortTxSignals(const QString& lmID)
	{
		return forEachPortOfLmAssociatedOptoModules(lmID, &OptoPort::sortTxSignals);
/*		QList<OptoModuleShared> optoModules = getLmAssociatedOptoModules(lmID);

		bool result = true;

		for(OptoModuleShared& optoModule : optoModules)
		{
			if (optoModule == nullptr)
			{
				assert(false);
				return false;
			}

			result &= optoModule->sortTxSignals();
		}

		return result;*/
	}

	bool OptoModuleStorage::sortSerialRxSignals(const QString& lmID)
	{
		return forEachPortOfLmAssociatedOptoModules(lmID, &OptoPort::sortSerialRxSignals);
/*		QList<OptoModuleShared> optoModules = getLmAssociatedOptoModules(lmID);

		bool result = true;

		for(OptoModuleShared& optoModule : optoModules)
		{
			if (optoModule == nullptr)
			{
				assert(false);
				return false;
			}

			result &= optoModule->sortTxSignals();
		}

		return result;*/
	}


	bool OptoModuleStorage::calculateTxSignalsAddresses(const QString& lmID)
	{
		return forEachPortOfLmAssociatedOptoModules(lmID, &OptoPort::calculateTxSignalsAddresses);

/*		QList<OptoModuleShared> optoModules = getLmAssociatedOptoModules(lmID);

		bool result = true;

		for(OptoModuleShared& optoModule : optoModules)
		{
			if (optoModule == nullptr)
			{
				assert(false);
				return false;
			}

			result &= optoModule->calculateTxSignalsAddresses();
		}

		return result;*/
	}

	bool OptoModuleStorage::calculateTxDataIDs(const QString& lmID)
	{
		return forEachPortOfLmAssociatedOptoModules(lmID, &OptoPort::calculateTxDataID);

/*		QList<OptoModuleShared> optoModules = getLmAssociatedOptoModules(lmID);

		bool result = true;

		for(OptoModuleShared& optoModule : optoModules)
		{
			if (optoModule == nullptr)
			{
				assert(false);
				return false;
			}

			result &= optoModule->calculateTxDataIDs();
		}

		return result;*/
	}

	bool OptoModuleStorage::calculateTxBuffersAbsAddresses(const QString& lmID)
	{
		return forEachOfLmAssociatedOptoModules(lmID, &OptoModule::calculateTxSignalsAddresses);
		/*QList<OptoModuleShared> optoModules = getLmAssociatedOptoModules(lmID);

		bool result = true;

		for(OptoModuleShared& optoModule : optoModules)
		{
			if (optoModule == nullptr)
			{
				assert(false);
				return false;
			}

			result &= optoModule->calculateTxBuffersAbsAddresses();
		}

		return result; */
	}

	bool OptoModuleStorage::calculateSerialRxSignalsAddresses(const QString& lmID)
	{
		return forEachPortOfLmAssociatedOptoModules(lmID, &OptoPort::calculateSerialRxSignalsAddresses);
		/*QList<OptoModuleShared> optoModules = getLmAssociatedOptoModules(lmID);

		bool result = true;

		for(OptoModuleShared& optoModule : optoModules)
		{
			if (optoModule == nullptr)
			{
				assert(false);
				return false;
			}

			result &= optoModule->calculateTxBuffersAbsAddresses();
		}

		return result; */
	}


	QList<OptoModuleShared> OptoModuleStorage::modules()
	{
		QList<OptoModuleShared> modules;

		for(OptoModuleShared module : m_modules)
		{
			modules.append(module);
		}

		return modules;
	}


	QList<OptoPortShared> OptoModuleStorage::ports()
	{
		QList<OptoPortShared> ports;

		for(OptoPortShared port : m_ports)
		{
			ports.append(port);
		}

		return ports;
	}

	bool OptoModuleStorage::setPortsRxDataSizes()
	{
		bool result = true;

		for(Hardware::OptoPortShared& port : m_ports)
		{
			if (port->isUsedInConnection() == false)
			{
				// optical port is not used in connection
				//
				port->setRxDataSizeW(0);
				continue;
			}

			if (port->mode() == Hardware::OptoPort::Mode::Serial)
			{
				// RS232/485 port has no linked ports
				//
				if (port->manualSettings() == true)
				{
					// set manual rx data size
					//
					port->setRxDataSizeW(port->manualRxSizeW());
				}
				else
				{
					//
					assert(false);
					//m_log->errALC5085(port->equipmentID(), port->connectionID());
				}

				continue;
			}

			assert(port->mode() == Hardware::OptoPort::Mode::Optical);

			if (port->txDataSizeW() > 0)
			{
				OptoPortShared linkedPort = getOptoPort(port->linkedPortID());

				if (linkedPort == nullptr)
				{
					LOG_INTERNAL_ERROR(m_log);
					assert(false);
					result = false;
					break;
				}

				if (linkedPort->manualSettings())
				{
					linkedPort->setRxDataSizeW(linkedPort->manualRxSizeW());

					if (port->txDataSizeW() > linkedPort->rxDataSizeW())
					{
						LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
										   QString(tr("Manual rxDataSizeW of port '%1' less then txDataSizeW of linked port '%2' (connection %3)")).
										   arg(linkedPort->equipmentID()).arg(port->equipmentID()).
										   arg(port->connectionID()));
						result = false;
					}
				}
				else
				{
					linkedPort->setRxDataSizeW(port->txDataSizeW());
				}
			}
		}

		return result;
	}


	bool OptoModuleStorage::calculatePortsAbsoulteTxStartAddresses()
	{
		bool result = true;

//		QList<OptoModule*> modulesList = modules();

/*		for(OptoModuleShared& module : m_modules)
		{
			if (module == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				assert(false);
				return false;
			}

			QList<OptoPortShared> portsList;

			module->getPorts(portsList);

			if (module->isLM() == true)
			{
				// calculate tx addresses for ports of LM module
				//
				int portNo = 0;

				for(OptoPortShared& port : portsList)
				{
					if (port == nullptr)
					{
						LOG_INTERNAL_ERROR(m_log);
						assert(false);
						return false;
					}

					int absTxStartAddress =	module->optoInterfaceDataOffset() +
											portNo * module->optoPortDataSize();

					if (port->manualSettings() == true)
					{
						absTxStartAddress += port->manualTxStartAddressW();

						port->setAbsTxStartAddress(absTxStartAddress);

						if (port->manualTxStartAddressW() + port->manualTxSizeW() > module->optoPortAppDataSize())
						{
							// TxData size (%1 words) of opto port '%2' exceed value of OptoPortAppDataSize property of module '%3' (%4 words).
							//
							m_log->errALC5032(port->txDataSizeW(), port->equipmentID(), module->equipmentID(), module->optoPortAppDataSize());
							result = false;
							break;
						}
					}
					else
					{
						port->setAbsTxStartAddress(absTxStartAddress);

						if (port->txDataSizeW() > module->optoPortAppDataSize())
						{
							// TxData size (%1 words) of opto port '%2' exceed value of OptoPortAppDataSize property of module '%3' (%4 words).
							//
							m_log->errALC5032(port->txDataSizeW(), port->equipmentID(), module->equipmentID(), module->optoPortAppDataSize());
							result = false;
							break;
						}
					}

					portNo++;
				}

				result &= checkPortsAddressesOverlapping(module);

				continue;
			}

			if (module->isOCM() == true)
			{
				// calculate tx addresses for ports of OCM module
				//
				int autoAbsTxStartAddress = module->optoInterfaceDataOffset();

				int txDataSizeW = 0;

				for(OptoPortShared& port : portsList)
				{
					if (port == nullptr)
					{
						LOG_INTERNAL_ERROR(m_log);
						assert(false);
						return false;
					}

					if (port->manualSettings() == true)
					{
						port->setAbsTxStartAddress(module->optoInterfaceDataOffset() + port->manualTxStartAddressW());

						// calculate TxStartAddr for next port with auto settings (if exists)
						//
						autoAbsTxStartAddress = module->optoInterfaceDataOffset() +
												port->manualTxStartAddressW() +
												port->manualTxSizeW();

						if (port->manualTxStartAddressW() + port->manualTxSizeW() > module->optoPortAppDataSize())
						{
							// TxData size (%1 words) of opto port '%2' exceed value of OptoPortAppDataSize property of module '%3' (%4 words).
							//
							m_log->errALC5032(port->txDataSizeW(), port->equipmentID(), module->equipmentID(), module->optoPortAppDataSize());
							result = false;
							break;
						}

					}
					else
					{
						// all OCM's ports data disposed in one buffer with max size - OptoPortAppDataSize
						//
						port->setAbsTxStartAddress(autoAbsTxStartAddress);

						autoAbsTxStartAddress += port->txDataSizeW();

						txDataSizeW += port->txDataSizeW();

						if (txDataSizeW > module->optoPortAppDataSize())
						{
							// TxData size (%1 words) of opto port '%2' exceed value of OptoPortAppDataSize property of module '%3' (%4 words).
							//
							m_log->errALC5032(port->txDataSizeW(), port->equipmentID(), module->equipmentID(), module->optoPortAppDataSize());
							result = false;
							break;
						}
					}
				}

				result &= checkPortsAddressesOverlapping(module);

				continue;
			}

			LOG_INTERNAL_ERROR(m_log)
			assert(false);      // unknown module type
			result = false;
			break;
		}*/

		return result;
	}


	bool OptoModuleStorage::calculatePortsRxStartAddresses()
	{
		bool result = true;

		for(OptoModuleShared& module : m_modules)
		{
			if (module == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				assert(false);
				return false;
			}

			QList<OptoPortShared> portsList;

			module->getPorts(portsList);

			if (module->isLM() == true)
			{
				// calculate rx addresses for ports of LM module
				//
				int i = 0;

				for(OptoPortShared& port : portsList)
				{
					if (port == nullptr)
					{
						LOG_INTERNAL_ERROR(m_log);
						assert(false);
						return false;
					}

					if (port->isUsedInConnection() == false)
					{
						port->setRxStartAddress(0);
						i++;
						continue;
					}

					int rxStartAddress =	module->optoInterfaceDataOffset() +
											i * module->optoPortDataSize();

					port->setRxStartAddress(rxStartAddress);

					i++;

					OptoPortShared linkedPort = getOptoPort(port->linkedPortID());

					if (linkedPort != nullptr)
					{
						if (linkedPort->txDataSizeW() > module->optoPortAppDataSize())
						{
							// RxData size (%1 words) of opto port '%2' exceed value of OptoPortAppDataSize property of module '%3' (%4 words).
							//
							m_log->errALC5035(linkedPort->txDataSizeW(), port->equipmentID(), module->equipmentID(), module->optoPortAppDataSize());
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

				continue;
			}

			if (module->isOCM() == true)
			{
				// calculate rx addresses for ports of OCM module
				//
				int rxStartAddress = module->optoInterfaceDataOffset() + module->optoPortAppDataOffset();

				int rxDataSizeW = 0;

				for(OptoPortShared& port : portsList)
				{
					if (port == nullptr)
					{
						LOG_INTERNAL_ERROR(m_log);
						assert(false);
						return false;
					}

					port->setRxStartAddress(0);

					// all OCM's ports data disposed in one buffer with max size - OptoPortAppDataSize
					//

					if (port->isUsedInConnection() == false)
					{
						continue;			// port is not connected
					}

					std::shared_ptr<Connection> connection = getConnection(port->connectionID());

					if (connection == nullptr)
					{
						assert(false);
						continue;
					}

					if (connection->mode() == OptoPort::Mode::Serial)
					{
						if (connection->port1EquipmentID() != port->equipmentID() )
						{
							continue;
						}

						port->setRxStartAddress(rxStartAddress);

						if (port->manualSettings() == true)
						{
							rxStartAddress += port->manualRxSizeW();
						}
						else
						{
							// Rx data size of RS232/485 port '%1' is undefined (connection '%2').
							//
							m_log->errALC5085(port->equipmentID(), port->connectionID());
						}

						continue;
					}

					// connection is in OptoPort::Mode::Opto

					port->setRxStartAddress(rxStartAddress);

					OptoPortShared linkedPort = getOptoPort(port->linkedPortID());

					if (linkedPort != nullptr)
					{
						rxStartAddress += linkedPort->txDataSizeW();

						rxDataSizeW += linkedPort->txDataSizeW();

						if (rxDataSizeW > module->optoPortAppDataSize())
						{
							// RxData size (%1 words) of opto port '%2' exceed value of OptoPortAppDataSize property of module '%3' (%4 words).
							//
							m_log->errALC5035(rxDataSizeW, port->equipmentID(), module->equipmentID(), module->optoPortAppDataSize());
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

				continue;
			}

			LOG_INTERNAL_ERROR(m_log)
			assert(false);      // unknown module type
			result = false;
			break;
		}

		return result;
	}

	bool OptoModuleStorage::calculateSerialRxDataIDs(const QString& lmID)
	{
		return forEachPortOfLmAssociatedOptoModules(lmID, &OptoPort::calculateSerialRxDataID);

/*		QList<OptoModuleShared> optoModules = getLmAssociatedOptoModules(lmID);

		bool result = true;

		for(OptoModuleShared& optoModule : optoModules)
		{
			if (optoModule == nullptr)
			{
				assert(false);
				return false;
			}

			result &= optoModule->calculateTxDataIDs();
		}

		return result;*/
	}


	bool OptoModuleStorage::copyOpticalPortsTxInRxSignals(const QString& lmID)
	{
		return forEachPortOfLmAssociatedOptoModules(lmID, &OptoPort::copyOpticalPortsTxInRxSignals);
	}

	bool OptoModuleStorage::addConnections(const Hardware::ConnectionStorage& connectionStorage)
	{
		bool result = true;

		int count = connectionStorage.count();

		for(int i = 0; i < count; i++)
		{
			std::shared_ptr<Connection> connection = connectionStorage.get(i);

			if (connection == nullptr)
			{
				assert(false);
				continue;
			}

			if (m_connections.contains(connection->connectionID()) == false)
			{
				m_connections.insert(connection->connectionID(), connection);
			}
			else
			{
				m_log->errALC5023(connection->connectionID());
				result = false;
				break;
			}
		}

		return result;
	}

	std::shared_ptr<Connection> OptoModuleStorage::getConnection(const QString& connectionID)
	{
		return m_connections.value(connectionID, nullptr);
	}

	bool OptoModuleStorage::addRegularTxSignal(const QString& schemaID,
										const QString& connectionID,
										QUuid transmitterUuid,
										const QString& lmID,
										const Signal *appSignal,
										bool* signalAlreadyInList)
	{
		if (appSignal == nullptr ||
			signalAlreadyInList == nullptr)
		{
			ASSERT_RETURN_FALSE;
		}

		*signalAlreadyInList = false;

		std::shared_ptr<Hardware::Connection> cn = getConnection(connectionID);

		if (cn == nullptr)
		{
			m_log->errALC5024(connectionID, transmitterUuid, schemaID);
			ASSERT_RETURN_FALSE;
		}

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

		if (m1->lmID() == lmID)
		{
			if (p1->isTxSignalExists(appSignal->appSignalID()))
			{
				*signalAlreadyInList = true;
			}
			else
			{
				p1->appendRegularTxSignal(appSignal);
			}
			return true;
		}

		if (m2->lmID() == lmID)
		{
			if (p2->isTxSignalExists(appSignal->appSignalID()))
			{
				*signalAlreadyInList = true;
			}
			else
			{
				p1->appendRegularTxSignal(appSignal);
			}
			return true;
		}

		// Ports of connection '%1' are not accessible in LM '%2.
		//
		m_log->errALC5059(schemaID, connectionID, lmID, transmitterUuid);

		return false;
	}

	bool OptoModuleStorage::addSerialRegularRxSignal(const QString& schemaID,
													 const QString& connectionID,
													 QUuid receiverUuid,
													 const QString& lmID,
													 const Signal *appSignal)
	{
		if (appSignal == nullptr)
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

		p1->appendSerialRegularRxSignal(appSignal);

		return false;
	}


	// return all opto modules sorted by equipmentID ascending alphabetical order
	//
	void OptoModuleStorage::getOptoModulesSorted(QVector<OptoModuleShared>& modules)
	{
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


	bool OptoModuleStorage::getRxSignalAbsAddress(const QString& connectionID,
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
			if (p1->rxSignalExists() == true)
			{
				return p1->getRxSignalAbsAddress(addr);
			}
			else
			{
				ASSERT_RETURN_FALSE;			// Signal isn't exists in port p1
			}
		}

		if (connection->mode() != OptoPort::Mode::Optical)
		{
			// this is Serial connection
			// port2 is not used
			// signal is not found
			//
			ASSERT_RETURN_FALSE;
		}

		OptoPortShared p2 = getOptoPort(connection->port2EquipmentID());

		if (p2 == nullptr)
		{
			ASSERT_RETURN_FALSE;
		}

		if (p2->lmID() == receiverLM)
		{
			if (p2->rxSignalExists() == true)
			{
				return p2->getRxSignalAbsAddress(addr);
			}
			else
			{
				ASSERT_RETURN_FALSE;			// Signal isn't exists in port p2
			}
		}

		ASSERT_RETURN_FALSE;			// signal is not found in both ports
	}


	bool OptoModuleStorage::getSignalRxAddressOpto(	std::shared_ptr<Connection> connection,
													const QString& appSignalID,
													const QString& receiverLM,
													QUuid receiverUuid,
													SignalAddress16& addr)
	{
		assert(connection->mode() == OptoPort::Mode::Optical);

		OptoPortShared port1 = getOptoPort(connection->port1EquipmentID());

		if (port1 == nullptr)
		{
			assert(false);
			return false;
		}

		OptoPortShared port2 = getOptoPort(connection->port2EquipmentID());

		if (port2 == nullptr)
		{
			assert(false);
			return false;
		}

		if (port1->isTxSignalExists(appSignalID))
		{
			OptoModuleShared module1 = getOptoModule(port1->optoModuleID());

			if (module1->lmID() == receiverLM)
			{
				// Signal '%1' exists in LM '%2'. No receivers needed.
				//
				m_log->errALC5041(appSignalID, receiverLM, receiverUuid);
				return false;
			}

			OptoModuleShared module2 = getOptoModule(port2->optoModuleID());

			if (module2->lmID() != receiverLM)
			{
				// Signal '%1' is not exists in connection '%2'. Use transmitter to send signal via connection.
				//
				m_log->errALC5042(appSignalID, connection->connectionID(), receiverUuid);
				return false;
			}

			addr = port1->getTxSignalAddrInBuf(appSignalID);

			addr.addWord(port2->rxStartAddress());

			return true;
		}

		if (port2->isTxSignalExists(appSignalID))
		{
			OptoModuleShared module2 = getOptoModule(port2->optoModuleID());

			if (module2->lmID() == receiverLM)
			{
				// Signal '%1' exists in LM '%2'. No receivers needed.
				//
				m_log->errALC5041(appSignalID, receiverLM, receiverUuid);
				return false;
			}

			OptoModuleShared module1 = getOptoModule(port1->optoModuleID());

			if (module1->lmID() != receiverLM)
			{
				// Signal '%1' is not exists in connection '%2'. Use transmitter to send signal via connection.
				//
				m_log->errALC5042(appSignalID, connection->connectionID(), receiverUuid);
				return false;
			}

			addr = port2->getTxSignalAddrInBuf(appSignalID);

			addr.addWord(port1->rxStartAddress());

			return true;
		}

		// Signal '%1' is not exists in connection '%2'. Use transmitter to send signal via connection.
		//
		m_log->errALC5042(appSignalID, connection->connectionID(), receiverUuid);
		return false;
	}


	bool OptoModuleStorage::getSignalRxAddressSerial(	std::shared_ptr<Connection> connection,
														const QString& appSignalID,
														const QString& receiverLM,
														QUuid receiverUuid,
														SignalAddress16& addr)
	{
		assert(connection->mode() == OptoPort::Mode::Serial);

		// only  port1 is used in serial connections
		//
		OptoPortShared port1 = getOptoPort(connection->port1EquipmentID());

		if (port1 == nullptr)
		{
			assert(false);
			return false;
		}

		OptoModuleShared module1 = getOptoModule(port1->optoModuleID());

		if (module1->lmID() != receiverLM)
		{
			// Receiver of connection '%1' (port '%2') is not associated with LM '%3'
			//
			m_log->errALC5083(port1->equipmentID(), connection->connectionID(), receiverLM, receiverUuid);
			return false;
		}

		return port1->getSignalRxAddressSerial(connection, appSignalID, receiverUuid, addr);
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



}
