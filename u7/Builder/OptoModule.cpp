#include "OptoModule.h"

#include "../LogicModule.h"
#include "../Builder/Builder.h"
#include "../Builder/ApplicationLogicCompiler.h"
#include "../lib/DeviceHelper.h"


namespace Hardware
{
	// ------------------------------------------------------------------
	//
	// OptoPort class implementation
	//
	// ------------------------------------------------------------------

	OptoPort::OptoPort(const QString& optoModuleStrID, DeviceController* optoPortController, int port) :
		m_deviceController(optoPortController),
		m_optoModuleID(optoModuleStrID)
	{
		if (optoPortController == nullptr)
		{
			assert(false);
			return;
		}

		m_equipmentID = optoPortController->equipmentIdTemplate();
		m_port = port;
	}


	void OptoPort::addTxSignal(Signal* txSignal)
	{
		if (txSignal == nullptr)
		{
			assert(false);
			return;
		}

		if (m_txSignalsIDs.contains(txSignal->appSignalID()))
		{
			assert(false);
			return;
		}
		else
		{
			m_txSignalsIDs.insert(txSignal->appSignalID(), Address16());

			TxRxSignal txs;

			txs.appSignalID = txSignal->appSignalID();
			txs.sizeBit = txSignal->dataSize();

			if (txSignal->isAnalog())
			{
				m_txAnalogSignals.append(txs);
			}
			else
			{
				if (txSignal->isDiscrete())
				{
					m_txDiscreteSignals.append(txs);
				}
				else
				{
					assert(false);  // unknown type of signal
				}
			}
		}
	}


	QVector<OptoPort::TxRxSignal> OptoPort::getTxSignals()
	{
		QVector<TxRxSignal> txSignals;

		txSignals.append(m_txAnalogSignals);
		txSignals.append(m_txDiscreteSignals);

		return txSignals;
	}

	// initial txSignals addresses calculcation
	// zero-offset from port txStartAddress
	//
	bool OptoPort::calculateTxSignalsAddresses(Builder::IssueLogger *log)
	{
		if (log == nullptr)
		{
			assert(false);
			return false;
		}

		if (isUsedInConnection() == false)
		{
			m_txDataSizeW = 0;
			return true;
		}

		sortTxSignals();

		bool result = true;

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

		// then place analog signals

		int startAddr = address.offset();

		for(TxRxSignal& txAnalogSignal : m_txAnalogSignals)
		{
			txAnalogSignal.offset = address;

			if (m_txSignalsIDs.contains(txAnalogSignal.appSignalID))
			{
				m_txSignalsIDs[txAnalogSignal.appSignalID] = address;
			}
			else
			{
				assert(false);
			}

			address.addBit(txAnalogSignal.sizeBit);
			address.wordAlign();

			m_txDataID = CRC32(m_txDataID, C_STR(txAnalogSignal.appSignalID), txAnalogSignal.appSignalID.length(), false);
		}

		m_txAnalogSignalsSizeW = address.offset() - startAddr ;

		// then place discrete signals

		startAddr = address.offset();

		for(TxRxSignal& txDiscreteSignal : m_txDiscreteSignals)
		{
			txDiscreteSignal.offset = address;

			if (m_txSignalsIDs.contains(txDiscreteSignal.appSignalID))
			{
				m_txSignalsIDs[txDiscreteSignal.appSignalID] = address;
			}
			else
			{
				assert(false);
			}

			assert(txDiscreteSignal.sizeBit == 1);

			address.add1Bit();

			m_txDataID = CRC32(m_txDataID, C_STR(txDiscreteSignal.appSignalID), txDiscreteSignal.appSignalID.length(), false);
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
				LOG_MESSAGE(log, QString(tr("Port %1: txIdSizeW = 2, txRawDataSizeW = %2, txAnalogSignalsSizeW = %3, txDiscreteSignalsSizeW = %4")).
									arg(m_equipmentID).arg(m_txRawDataSizeW).arg(m_txAnalogSignalsSizeW).arg(m_txDiscreteSignalsSizeW));
				LOG_ERROR_OBSOLETE(log, Builder::IssueType::NotDefined,
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

	void OptoPort::setTxRawDataSizeW(int rawDataSizeW)
	{
		assert(m_txRawDataSizeWIsCalculated == false);		// setTxRawDataSizeW() must be called once!

		m_txRawDataSizeW = rawDataSizeW;

		m_txRawDataSizeWIsCalculated = true;
	}

/*	void OptoPort::setRawDataSizeIsCalculated()
	{
		assert(m_rawDataSizeIsCalculated == false);		// setRawDataSizeIsCalculated() must be called once!

		m_rawDataSizeIsCalculated = true;
	}*/


	bool OptoPort::isTxSignalIDExists(const QString& appSignalID)
	{
		return m_txSignalsIDs.contains(appSignalID);
	}


	bool OptoPort::isUsedInConnection() const
	{
		return m_connectionID.isEmpty() != true;
	}


	Address16 OptoPort::getTxSignalAddress(const QString& appSignalID) const
	{
		if (m_txSignalsIDs.contains(appSignalID))
		{
			return m_txSignalsIDs[appSignalID];
		}

		assert(false);

		return Address16();
	}

	bool OptoPort::parseRawDescription(Builder::IssueLogger* log)
	{
		bool result = m_rawDataDescription.parse(*this, log);

		if (result == false)
		{
			return false;
		}

		result &= buildTxRawSignalList(log);
		result &= buildRxRawSignalList(log);

		return result;
	}

	void OptoPort::sortTxSignals()
	{
		// sort txSignals in ascending alphabetical order
		//
		sortTxSignals(m_txAnalogSignals);
		sortTxSignals(m_txDiscreteSignals);
	}


	void OptoPort::sortTxSignals(QVector<TxRxSignal>& array)
	{
		int count = array.count();

		for(int i = 0; i < count - 1; i++)
		{
			for(int k = i + 1; k < count; k++)
			{
				if (array[i].appSignalID > array[k].appSignalID)
				{
					TxRxSignal temp = array[i];
					array[i] = array[k];
					array[k] = temp;
				}
			}
		}
	}



	bool OptoPort::calculatePortRawDataSize(OptoModuleStorage* optoStorage, Builder::IssueLogger* log)
	{
		const DeviceModule* lm = DeviceHelper::getAssociatedLM(m_deviceController);

		if (optoStorage == nullptr || log == nullptr || lm == nullptr)
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
			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
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
			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
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

				partSizeW = DeviceHelper::getAllNativeRawDataSize(lm, log);;

				size += partSizeW;

				break;

			case RawDataDescriptionItem::Type::TxModuleRawData:
				{
					bool moduleIsFound = false;

					partSizeW = DeviceHelper::getModuleRawDataSize(lm, item.modulePlace, &moduleIsFound, log);

					size += partSizeW;

					if (moduleIsFound == false)
					{
						msg = QString("Module on place %1 is not found (opto port '%2' raw data description).").arg(item.modulePlace).arg(equipmentID());
						LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
						result = false;
					}
				}

				break;

			case RawDataDescriptionItem::Type::TxPortRawData:
				{
					OptoPort* portRxRawData = optoStorage->getOptoPort(item.portEquipmentID);

					if (portRxRawData == nullptr)
					{
						msg = QString("Port '%1' is not found (opto port '%2' raw data description).").arg(item.portEquipmentID).arg(equipmentID());
						LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
						result = false;
						break;
					}

					OptoPort* portTxRawData = optoStorage->getOptoPort(portRxRawData->linkedPortID());

					if (portTxRawData == nullptr)
					{
						msg = QString("Port '%1' linked to '%2' is not found (opto port '%3' raw data description).").
								arg(portRxRawData->linkedPortID()).
								arg(portRxRawData->equipmentID()).
								arg(equipmentID());
						LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
						result = false;
						break;
					}

					bool res = portTxRawData->calculatePortRawDataSize(optoStorage, log);

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
												SignalAddress16& addr, Builder::IssueLogger *log)
	{
		if (connection->mode() != OptoPort::Mode::Serial)
		{
			assert(false);
			return false;
		}

		if (log == nullptr)
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
		log->errALC5084(appSignalID, connection->connectionID(), receiverUuid);

		return false;
	}


	bool OptoPort::buildTxRawSignalList(Builder::IssueLogger* log)
	{
		m_txRawSignals.clear();

		for(const RawDataDescriptionItem& item : m_rawDataDescription)
		{
			if (item.type == RawDataDescriptionItem::Type::TxSignal)
			{
				TxRxSignal txSignal;

				txSignal.appSignalID = item.appSignalID;
				txSignal.offset.set(item.offsetW, item.bitNo);
				txSignal.absAddress.reset();
				txSignal.sizeBit = item.dataSize;

				m_txRawSignals.append(txSignal);
			}
		}

		sortByOffsetBitNoAscending(m_txRawSignals);

		bool result = checkSignalsOffsets(m_txRawSignals, true, log);

		return result;
	}


	bool OptoPort::buildRxRawSignalList(Builder::IssueLogger* log)
	{
		m_rxRawSignals.clear();

		for(const RawDataDescriptionItem& item : m_rawDataDescription)
		{
			if (item.type == RawDataDescriptionItem::Type::RxSignal)
			{
				TxRxSignal rxSignal;

				rxSignal.appSignalID = item.appSignalID;
				rxSignal.offset.set(item.offsetW, item.bitNo);
				rxSignal.absAddress.reset();
				rxSignal.sizeBit = item.dataSize;

				m_rxRawSignals.append(rxSignal);
			}
		}

		sortByOffsetBitNoAscending(m_rxRawSignals);

		bool result = checkSignalsOffsets(m_rxRawSignals, false, log);

		return result;
	}


	void OptoPort::sortByOffsetBitNoAscending(QVector<TxRxSignal>& signalList)
	{
		int count = signalList.count();

		for(int i = 0; i < count - 1; i++)
		{
			for(int k = i + 1; k < count; k++)
			{
				if (signalList[i].offset.bitAddress() > signalList[k].offset.bitAddress())
				{
					TxRxSignal temp = signalList[i];
					signalList[i] = signalList[k];
					signalList[k] = temp;
				}
			}
		}
	}


	bool OptoPort::checkSignalsOffsets(const QVector<TxRxSignal>& signalList, bool overlapIsError, Builder::IssueLogger* log)
	{
		bool result = true;

		int count = signalList.count();

		for(int i = 0; i < count - 1; i++)
		{
			const TxRxSignal& s1 = signalList[i];
			const TxRxSignal& s2 = signalList[i + 1];

			int s1StartBitAddress = s1.offset.bitAddress();
			int s1EndBitAddress = s1.offset.bitAddress() + s1.sizeBit - 1;

			int s2StartBitAddress = s2.offset.bitAddress();
			int s2EndBitAddress = s2.offset.bitAddress() + s1.sizeBit - 1;

			if ((s1StartBitAddress >= s2StartBitAddress && s1StartBitAddress <= s2EndBitAddress) ||
				(s2StartBitAddress >= s1StartBitAddress && s2StartBitAddress <= s1EndBitAddress))
			{
				if (overlapIsError == true)
				{
					LOG_ERROR_OBSOLETE(log, Builder::IssueType::NotDefined,
									   QString("Raw signals '%1' and '%2' is overlapped (port '%3').").
									   arg(s1.appSignalID).arg(s2.appSignalID).arg(equipmentID()));
					result = false;
				}
				else
				{
					LOG_WARNING_OBSOLETE(log, Builder::IssueType::NotDefined,
										 QString("Raw signals '%1' and '%2' is overlapped (port '%3').").
										 arg(s1.appSignalID).arg(s2.appSignalID).arg(equipmentID()));
				}
			}
		}

		return result;
	}


	// ------------------------------------------------------------------
	//
	// OptoModule class implementation
	//
	// ------------------------------------------------------------------

	OptoModule::OptoModule(DeviceModule* module, LogicModule* lmDescription, Builder::IssueLogger* log) :
		m_deviceModule(module),
		m_lmDescription(lmDescription),
		m_log(log)
	{
		if (module == nullptr || m_lmDescription == nullptr || m_log == nullptr)
		{
			assert(false);
			return;
		}

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
			bool result = true;

			result &= DeviceHelper::getIntProperty(module, "OptoInterfaceDataOffset", &m_optoInterfaceDataOffset, log);
			result &= DeviceHelper::getIntProperty(module, "OptoPortDataSize", &m_optoPortDataSize, log);
			result &= DeviceHelper::getIntProperty(module, "OptoPortAppDataOffset", &m_optoPortAppDataOffset, log);
			result &= DeviceHelper::getIntProperty(module, "OptoPortAppDataSize", &m_optoPortAppDataSize, log);
			result &= DeviceHelper::getIntProperty(module, "OptoPortCount", &m_optoPortCount, log);

			if (result == false)
			{
				return;
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
						return;
					}

					OptoPort* optoPort = new OptoPort(module->equipmentIdTemplate(), optoPortController, i + 1);

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
			return;
		}

		if (isLM() == true)
		{
			// OCM module
			//
			m_lmID = module->equipmentIdTemplate();
			m_lmDeviceModule = module;
		}
		else
		{
			if (isOCM() != true)
			{
				assert(false);      // unknown module type
				return;
			}

			// OCM module
			//
			const DeviceChassis* chassis = module->getParentChassis();

			if (chassis == nullptr)
			{
				LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
								   QString(tr("Not found chassis to module '%1'")).arg(module->equipmentIdTemplate()));
				return;
			}

			int childrenCount = chassis->childrenCount();

			bool lmAlreadyFound = false;

			for(int i = 0; i < childrenCount; i++)
			{
				DeviceObject* child = chassis->child(i);

				if (child == nullptr)
				{
					assert(false);
					continue;
				}

				if (child->isModule() == false)
				{
					continue;
				}

				DeviceModule* childModule = child->toModule();

				if (childModule == nullptr)
				{
					assert(false);
					continue;
				}

				if (childModule->moduleFamily() == DeviceModule::FamilyType::LM)
				{
					if (lmAlreadyFound == true)
					{
						assert(false);          // second LM in chassis ?
					}

					m_lmID = childModule->equipmentIdTemplate();
					m_lmDeviceModule = childModule;

					lmAlreadyFound = true;
				}
			}
		}

		m_valid = true;
	}


	OptoModule::~OptoModule()
	{
		for(OptoPort* optoPort : m_ports)
		{
			delete optoPort;
		}

		m_ports.clear();
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


	QList<OptoPort*> OptoModule::getSerialPorts()
	{
		QList<OptoPort*> serialPorts;

		for(OptoPort* port : m_ports)
		{
			if (port == nullptr)
			{
				assert(false);
				continue;
			}

			if (port->mode() == OptoPort::Mode::Serial)
			{
				serialPorts.append(port);
			}
		}

		return serialPorts;
	}


	QList<OptoPort*> OptoModule::getOptoPorts()
	{
		QList<OptoPort*> optoPorts;

		for(OptoPort* port : m_ports)
		{
			if (port == nullptr)
			{
				assert(false);
				continue;
			}

			optoPorts.append(port);
		}

		return optoPorts;
	}


	QList<OptoPort*> OptoModule::ports()
	{
		QList<OptoPort*> ports;

		for(OptoPort* port : m_ports)
		{
			if (port == nullptr)
			{
				assert(false);
				continue;
			}

			ports.append(port);
		}

		return ports;
	}

	// return all ports sorted by equipmentID ascending alphabetical order
	//
	QVector<OptoPort*> OptoModule::getPortsSorted()
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

		for(OptoPort* port : m_ports)
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
		for(OptoModule* optoModule : m_modules)
		{
			delete optoModule;
		}

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


		for(OptoModule* optoModule : m_modules)
		{
			for(OptoPort* optoPort : optoModule->m_ports)
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
		OptoModule* optoModule = new OptoModule(module, lmDescription.get(), m_log);

		if (optoModule->isValid() == false)
		{
			delete optoModule;
			return false;
		}

		m_modules.insert(module->equipmentIdTemplate(), optoModule);

		m_lmAssociatedModules.insertMulti(optoModule->lmID(), optoModule);

		return true;
	}


	OptoModule* OptoModuleStorage::getOptoModule(const QString& optoModuleID)
	{
		if (m_modules.contains(optoModuleID))
		{
			return m_modules[optoModuleID];
		}

		return nullptr;
	}


	OptoModule* OptoModuleStorage::getOptoModule(const OptoPort* optoPort)
	{
		if (optoPort == nullptr)
		{
			assert(false);
			return nullptr;
		}

		if (m_modules.contains(optoPort->optoModuleID()))
		{
			return m_modules[optoPort->optoModuleID()];
		}

		return nullptr;
	}


	OptoPort* OptoModuleStorage::getOptoPort(const QString& optoPortID)
	{
		if (m_ports.contains(optoPortID))
		{
			return m_ports[optoPortID];
		}

		return nullptr;
	}


	QString OptoModuleStorage::getOptoPortAssociatedLmID(const OptoPort* optoPort)
	{
		if (optoPort == nullptr)
		{
			assert(false);
			return "";
		}

		OptoModule* optoModule = getOptoModule(optoPort);

		if (optoModule == nullptr)
		{
			assert(false);
			return "";
		}

		return optoModule->lmID();
	}


	OptoPort* OptoModuleStorage::jsGetOptoPort(const QString& optoPortStrID)
	{
		Hardware::OptoPort* port = getOptoPort(optoPortStrID);

		if (port != nullptr)
		{
			QQmlEngine::setObjectOwnership(port, QQmlEngine::ObjectOwnership::CppOwnership);
			return port;
		}

		return nullptr;
	}


	bool OptoModuleStorage::isCompatiblePorts(const OptoPort* optoPort1, const OptoPort* optoPort2)
	{
		if (optoPort1 == nullptr ||
			optoPort2 == nullptr)
		{
			assert(false);
			return false;
		}

		OptoModule* optoModule1 = getOptoModule(optoPort1);
		OptoModule* optoModule2 = getOptoModule(optoPort2);

		return  (optoModule1->isLM() == optoModule2->isLM()) ||
				(optoModule1->isOCM() == optoModule2->isOCM());
	}


	QList<OptoModule*> OptoModuleStorage::getLmAssociatedOptoModules(const QString& lmStrID)
	{
		return m_lmAssociatedModules.values(lmStrID);
	}


	QList<OptoModule*> OptoModuleStorage::modules()
	{
		QList<OptoModule*> modules;

		for(OptoModule* module : m_modules)
		{
			modules.append(module);
		}

		return modules;
	}


	QList<OptoPort*> OptoModuleStorage::ports()
	{
		QList<OptoPort*> ports;

		for(OptoPort* port : m_ports)
		{
			ports.append(port);
		}

		return ports;
	}

	bool OptoModuleStorage::setPortsRxDataSizes()
	{
		bool result = true;

		QList<Hardware::OptoPort*> portsList = ports();

		for(Hardware::OptoPort* port : portsList)
		{
			if (port->isUsedInConnection == false)
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
					asserf(false);
					//m_log->errALC5085(port->equipmentID(), port->connectionID());
				}

				continue;
			}

			assert(port->mode() == Hardware::OptoPort::Mode::Optical);

			if (port->txDataSizeW() > 0)
			{
				Hardware::OptoPort* linkedPort = getOptoPort(port->linkedPortID());

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

		QList<OptoModule*> modulesList = modules();

		for(OptoModule* module : modulesList)
		{
			if (module == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				assert(false);
				return false;
			}

			QList<OptoPort*> portsList = module->ports();

			if (module->isLM() == true)
			{
				// calculate tx addresses for ports of LM module
				//
				int portNo = 0;

				for(OptoPort* port : portsList)
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

				for(OptoPort* port : portsList)
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
	bool OptoModuleStorage::checkPortsAddressesOverlapping(OptoModule* module)
	{
		if (module == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			assert(false);
			return false;
		}

		QList<OptoPort*> portsList = module->ports();

		int portsCount = portsList.count();

		for(int i = 0; i < portsCount - 1; i++)
		{
			OptoPort* port1 = portsList.at(i);

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
				OptoPort* port2 = portsList.at(k);

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

		QList<OptoModule*> modulesList = modules();

		for(OptoModule* module : modulesList)
		{
			if (module == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				assert(false);
				return false;
			}

			QList<OptoPort*> portsList = module->ports();

			if (module->isLM() == true)
			{
				// calculate rx addresses for ports of LM module
				//
				int i = 0;

				for(OptoPort* port : portsList)
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

					OptoPort* linkedPort = getOptoPort(port->linkedPortID());

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

				for(OptoPort* port : portsList)
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

					OptoPort* linkedPort = getOptoPort(port->linkedPortID());

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

		OptoPort* p1 = getOptoPort(cn->port1EquipmentID());
		OptoPort* p2 = getOptoPort(cn->port2EquipmentID());

		if (p1 == nullptr || p2 == nullptr)
		{
			assert(false);
			return false;
		}

		OptoModule* m1 = getOptoModule(p1);
		OptoModule* m2 = getOptoModule(p2);

		if (m1 == nullptr || m2 == nullptr)
		{
			assert(false);
			return false;
		}

		assert(m1->lmID() != m2->lmID());

		if (m1->lmID() == lmID)
		{
			if (p1->isTxSignalIDExists(appSignal->appSignalID()))
			{
				*signalAlreadyInList = true;
			}
			else
			{
				p1->addTxSignal(appSignal);
			}
			return true;
		}

		if (m2->lmID() == lmID)
		{
			if (p2->isTxSignalIDExists(appSignal->appSignalID()))
			{
				*signalAlreadyInList = true;
			}
			else
			{
				p2->addTxSignal(appSignal);
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
	QVector<OptoModule*> OptoModuleStorage::getOptoModulesSorted()
	{
		QVector<OptoModule*> modules;

		for(OptoModule* optoModule : m_modules)
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
					OptoModule* temp = modules[i];
					modules[i] = modules[k];
					modules[k] = temp;
				}
			}
		}

		return modules;
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

		OptoPort* port1 = getOptoPort(connection->port1EquipmentID());

		if (port1 == nullptr)
		{
			assert(false);
			return false;
		}

		OptoPort* port2 = getOptoPort(connection->port2EquipmentID());

		if (port2 == nullptr)
		{
			assert(false);
			return false;
		}

		if (port1->isTxSignalIDExists(appSignalID))
		{
			OptoModule* module1 = getOptoModule(port1->optoModuleID());

			if (module1->lmID() == receiverLM)
			{
				// Signal '%1' exists in LM '%2'. No receivers needed.
				//
				m_log->errALC5041(appSignalID, receiverLM, receiverUuid);
				return false;
			}

			OptoModule* module2 = getOptoModule(port2->optoModuleID());

			if (module2->lmID() != receiverLM)
			{
				// Signal '%1' is not exists in connection '%2'. Use transmitter to send signal via connection.
				//
				m_log->errALC5042(appSignalID, connection->connectionID(), receiverUuid);
				return false;
			}

			addr = port1->getTxSignalAddress(appSignalID);

			addr.addWord(port2->rxStartAddress());

			return true;
		}

		if (port2->isTxSignalIDExists(appSignalID))
		{
			OptoModule* module2 = getOptoModule(port2->optoModuleID());

			if (module2->lmID() == receiverLM)
			{
				// Signal '%1' exists in LM '%2'. No receivers needed.
				//
				m_log->errALC5041(appSignalID, receiverLM, receiverUuid);
				return false;
			}

			OptoModule* module1 = getOptoModule(port1->optoModuleID());

			if (module1->lmID() != receiverLM)
			{
				// Signal '%1' is not exists in connection '%2'. Use transmitter to send signal via connection.
				//
				m_log->errALC5042(appSignalID, connection->connectionID(), receiverUuid);
				return false;
			}

			addr = port2->getTxSignalAddress(appSignalID);

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
		OptoPort* port1 = getOptoPort(connection->port1EquipmentID());

		if (port1 == nullptr)
		{
			assert(false);
			return false;
		}

		OptoModule* module1 = getOptoModule(port1->optoModuleID());

		if (module1->lmID() != receiverLM)
		{
			// Receiver of connection '%1' (port '%2') is not associated with LM '%3'
			//
			m_log->errALC5083(port1->equipmentID(), connection->connectionID(), receiverLM, receiverUuid);
			return false;
		}

		return port1->getSignalRxAddressSerial(connection, appSignalID, receiverUuid, addr, m_log);
	}

}
