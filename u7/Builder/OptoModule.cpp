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

	bool TxRxSignal::init(const QString& appSignalID, E::SignalType signalType, int offset, int bitNo, int sizeB, TxRxSignal::Type txRxType)
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

		m_type = txRxType;

		if (isRaw() == true)
		{
			assert(offset >= 0 && offset < 65536);
			assert(bitNo >= 0 && bitNo < 16);

			m_addrInBuf.setOffset(offset);
			m_addrInBuf.setBit(bitNo);

			assert(sizeB >= SIZE_1BIT && sizeB <= SIZE_32BIT);

			m_sizeB = sizeB;
		}
		else
		{
			m_addrInBuf.reset();

			m_sizeB = sizeB;
		}

#ifdef QT_DEBUG
		if (isDiscrete() == true)
		{
			assert(m_sizeB == SIZE_1BIT);
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

	OptoPort::OptoPort(const DeviceController* controller, int portNo, Builder::IssueLogger* log) :
		m_controller(controller),
		m_portNo(portNo),
		m_log(log)
	{
		if (m_controller == nullptr || m_log == nullptr)
		{
			assert(false);
			return;
		}

		m_equipmentID = m_controller->equipmentIdTemplate();

		const DeviceModule* module = m_controller->getParentModule();

		if (module == nullptr)
		{
			assert(false);
			return;
		}

		m_optoModuleID = module->equipmentIdTemplate();
	}

	bool OptoPort::appendRegularTxSignal(const Signal* s)
	{
		if (s == nullptr)
		{
			assert(false);
			return false;
		}

		return appendTxSignal(s->appSignalID(), s->signalType(), -1, -1, s->dataSize(), TxRxSignal::Type::Regular);
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

	// initial txSignals addresses calculcation
	// zero-offset from port txStartAddress
	//
	bool OptoPort::calculateTxSignalsAddresses()
	{
		if (isUsedInConnection() == false)
		{
			m_txDataSizeW = 0;
			return true;
		}

		bool result = sortTxRxSignals(m_txSignals);

		if (result == false)
		{
			return false;
		}

		m_txDataID = CRC32_INITIAL_VALUE;

		// mix port StrID in dataID
		//
		m_txDataID = CRC32(m_txDataID, C_STR(m_equipmentID), m_equipmentID.length(), false);

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
		for(TxRxSignalShared txAnalogSignal : m_txSignals)
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

			address.addBit(txAnalogSignal->sizeB());
			address.wordAlign();

			m_txDataID = CRC32(m_txDataID, txAnalogSignal->appSignalID(), false);
		}

		m_txAnalogSignalsSizeW = address.offset() - startAddr ;

		startAddr = address.offset();

		// then place discrete signals
		//
		for(TxRxSignalShared txDiscreteSignal : m_txSignals)
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

			assert(txDiscreteSignal->sizeB() == SIZE_1BIT);

			address.add1Bit();

			m_txDataID = CRC32(m_txDataID, txDiscreteSignal->appSignalID(), false);
		}

		m_txDataID = CRC32(m_txDataID, nullptr, 0, true);       // finalize CRC32 calculation

		address.wordAlign();

		m_txDiscreteSignalsSizeW = address.offset() - startAddr;

		int fullTxDataSizeW = TX_DATA_ID_SIZE_W + m_txRawDataSizeW + m_txAnalogSignalsSizeW + m_txDiscreteSignalsSizeW;

		if (manualSettings())
		{
			m_txDataSizeW = m_manualTxSizeW;

			if (fullTxDataSizeW > m_txDataSizeW)
			{
				LOG_MESSAGE(m_log, QString(tr("Port %1: txIdSizeW = 2, txRawDataSizeW = %2, txAnalogSignalsSizeW = %3, txDiscreteSignalsSizeW = %4")).
									arg(m_equipmentID).arg(m_txRawDataSizeW).arg(m_txAnalogSignalsSizeW).arg(m_txDiscreteSignalsSizeW));
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
								   QString(tr("Manual txDataSizeW - %1 less then needed size %2 (port %3, connection %4)")).
								   arg(m_manualTxSizeW).arg(fullTxDataSizeW).
								   arg(m_equipmentID).arg(m_connectionID));

				result = false;
			}
		}
		else
		{
			m_txDataSizeW = fullTxDataSizeW;
		}

		return result;
	}

	bool OptoPort::isTxSignalExists(const QString& appSignalID)
	{
		TxRxSignalShared s = m_txSignals.value(appSignalID, nullptr);

		return s != nullptr;
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

	Address16 OptoPort::getTxSignalAbsAddr(const QString& appSignalID) const
	{
		Address16 addrInBuf = getTxSignalAddrInBuf(appSignalID);

		if (addrInBuf.isValid() == false)
		{
			return addrInBuf;
		}

		addrInBuf.addWord(m_absTxStartAddress);

		return addrInBuf;
	}

	bool OptoPort::parseRawDescription()
	{
		bool result = m_rawDataDescription.parse(*this, m_log);

		if (result == false)
		{
			return false;
		}

		result &= appendRawTxRxSignals();

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

	bool OptoPort::getSignalRxAddressSerial(std::shared_ptr<Connection> connection,
												const QString& appSignalID,
												QUuid receiverUuid,
												SignalAddress16& addr)
	{
		if (connection->mode() != OptoPort::Mode::Serial)
		{
			assert(false);
			return false;
		}

		for(const RawDataDescriptionItem& rdi : m_rawDataDescription)
		{
			if (rdi.type != RawDataDescriptionItem::Type::RxSignal)
			{
				continue;
			}

			// rdi.type == RawDataDescriptionItemType::InSignal

			if (rdi.appSignalID != appSignalID)
			{
				continue;
			}

			addr.setSignalType(rdi.signalType);
			addr.setDataFormat(rdi.dataFormat);
			addr.setDataSize(rdi.dataSize);
			addr.setByteOrder(rdi.byteOrder);

			addr.setOffset(rxStartAddress() + rdi.offsetW);

			if (rdi.signalType == E::SignalType::Discrete)
			{
				addr.setBit(rdi.bitNo);
			}
			else
			{
				addr.setBit(0);
			}

			return true;
		}

		// Signal '%1' is not exists in serial connection '%2'. Use PortRawDataDescription to define receiving signals.
		//
		m_log->errALC5084(appSignalID, connection->connectionID(), receiverUuid);

		return false;
	}

	void OptoPort::setTxRawDataSizeW(int rawDataSizeW)
	{
		assert(m_txRawDataSizeWIsCalculated == false);		// setTxRawDataSizeW() must be called once!

		m_txRawDataSizeW = rawDataSizeW;

		m_txRawDataSizeWIsCalculated = true;
	}

	bool OptoPort::appendTxSignal(const QString& appSignalID, E::SignalType signalType, int offset, int bitNo, int sizeB, TxRxSignal::Type type)
	{
		if (m_txSignals.contains(appSignalID))
		{
			// Signal ID '%1' is duplicate in opto port '%2'.
			//
			m_log->errALC5188(appSignalID, m_equipmentID);

			return false;
		}

		TxRxSignalShared txSignal = std::make_shared<TxRxSignal>();

		bool res = txSignal->init(appSignalID, signalType, offset, bitNo, sizeB, type);

		if (res == false)
		{
			return false;
		}

		m_txSignals.insert(txSignal->appSignalID(), txSignal);

		if (txSignal->isRaw() == false)
		{
			if (txSignal->isAnalog() == true)
			{
				m_txAnalogSignalsCount++;
			}
			else
			{
				if (txSignal->isDiscrete() == true)
				{
					m_txDiscreteSignalsCount++;
				}
				else
				{
					assert(false);  // unknown type of signal
					return false;
				}
			}
		}

		return true;
	}


	bool OptoPort::appendRxSignal(const QString& appSignalID, E::SignalType signalType, int offset, int bitNo, int sizeB, TxRxSignal::Type type)
	{
		if (m_rxSignals.contains(appSignalID))
		{
			// Signal ID '%1' is duplicate in opto port '%2'.
			//
			m_log->errALC5188(appSignalID, m_equipmentID);

			return false;
		}

		TxRxSignalShared rxSignal = std::make_shared<TxRxSignal>();

		bool res = rxSignal->init(appSignalID, signalType, offset, bitNo, sizeB, type);

		if (res == false)
		{
			return false;
		}

		m_rxSignals.insert(rxSignal->appSignalID(), rxSignal);

		return true;
	}



	bool OptoPort::appendRawTxRxSignals()
	{
		bool result = true;

		// append Raw Tx and Rx  signals
		//
		for(const RawDataDescriptionItem& item : m_rawDataDescription)
		{
			if (item.type == RawDataDescriptionItem::Type::TxSignal)
			{
				result &= appendTxSignal(item.appSignalID, item.signalType, item.offsetW, item.bitNo, item.dataSize, TxRxSignal::Type::Raw);
				continue;
			}

			if (item.type == RawDataDescriptionItem::Type::RxSignal)
			{
				result &= appendRxSignal(item.appSignalID, item.signalType, item.offsetW, item.bitNo, item.dataSize, TxRxSignal::Type::Raw);
			}
		}

		return result;
	}

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
			int s1EndBitAddress = s1->addrInBuf().bitAddress() + s1->sizeB() - 1;

			int s2StartBitAddress = s2->addrInBuf().bitAddress();
			int s2EndBitAddress = s2->addrInBuf().bitAddress() + s1->sizeB() - 1;

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


	bool OptoPort::sortTxRxSignals(HashedVector<QString, TxRxSignalShared>& signalList)
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

			bool result = true;

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

					OptoPortShared optoPort = std::make_shared<OptoPort>(optoPortController, i + 1, log);

					m_ports.insert(optoPortController->equipmentIdTemplate(), optoPort);

					findPortCount++;

					break;
				}
			}
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
				port->setTxStartAddress(port->manualTxStartAddressW());
			}
			else
			{
				if (isLM())
				{
					port->setTxStartAddress(0);					// in LM opto ports txStartAddress always 0
				}
				else
				{
					port->setTxStartAddress(txStartAddress);
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




	// ------------------------------------------------------------------
	//
	// OptoModuleStorage class implementation
	//
	// ------------------------------------------------------------------

	OptoModuleStorage::OptoModuleStorage(EquipmentSet* equipmentSet, Builder::LmDescriptionSet* lmDescriptionSet, Builder::IssueLogger *log) :
		m_equipmentSet(equipmentSet),
		m_lmDescriptionSet(lmDescriptionSet),
		m_log(log)
	{
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

		for(OptoModuleShared& optoModule : m_modules)
		{
			for(OptoPortShared& optoPort : optoModule->m_ports)
			{
				m_ports.insert(optoPort->equipmentID(), optoPort);
			}
		}

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


	QList<OptoModuleShared> OptoModuleStorage::getLmAssociatedOptoModules(const QString& lmStrID)
	{
		return m_lmAssociatedModules.values(lmStrID);
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
		}

		return result;
	}


	// checking ports addresses overlapping
	// usefull for manual settings of ports
	//
	bool OptoModuleStorage::checkPortsAddressesOverlapping(OptoModuleShared module)
	{
		if (module == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			assert(false);
			return false;
		}

		QList<OptoPortShared> portsList;

		module->getPorts(portsList);

		int portsCount = portsList.count();

		for(int i = 0; i < portsCount - 1; i++)
		{
			OptoPortShared port1 = portsList.at(i);

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
				OptoPortShared port2 = portsList.at(k);

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

				if (	(port1->absTxStartAddress() >= port2->absTxStartAddress() &&
						port1->absTxStartAddress() < port2->absTxStartAddress() + port2->txDataSizeW()) ||

						(port2->absTxStartAddress() >= port1->absTxStartAddress() &&
						port2->absTxStartAddress() < port1->absTxStartAddress() + port1->txDataSizeW())	)
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
		if (m_connections.contains(connectionID) == false)
		{
			return nullptr;
		}

		return m_connections[connectionID];
	}


	bool OptoModuleStorage::addTxSignal(const QString& schemaID,
										const QString& connectionID,
										QUuid transmitterUuid,
										const QString& lmID,
										Signal* appSignal,
										bool* signalAlreadyInList)
	{
		if (appSignal == nullptr ||
			signalAlreadyInList == nullptr)
		{
			assert(false);
			return false;
		}

		*signalAlreadyInList = false;

		std::shared_ptr<Connection> cn = getConnection(connectionID);

		if (cn == nullptr)
		{
			assert(false);
			return false;
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


	bool OptoModuleStorage::getSignalRxAddress(const QString& connectionID,
											   const QString& appSignalID,
											   const QString& receiverLM,
											   QUuid receiverUuid,
											   SignalAddress16 &addr)
	{
		addr.reset();

		std::shared_ptr<Connection> connection = getConnection(connectionID);

		if (connection == nullptr)
		{
			m_log->errALC5040(connectionID);
			return false;
		}

		switch(connection->mode())
		{
		case OptoPort::Mode::Optical:
			return getSignalRxAddressOpto(connection, appSignalID, receiverLM, receiverUuid, addr);

		case OptoPort::Mode::Serial:
			return getSignalRxAddressSerial(connection, appSignalID, receiverLM, receiverUuid, addr);

		default:
			assert(false);
		}

		return false;
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

}
