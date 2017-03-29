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

	const char* OptoPort::RAW_DATA_SIZE = "RAW_DATA_SIZE";
	const char* OptoPort::ALL_NATIVE_RAW_DATA = "ALL_NATIVE_RAW_DATA";
	const char* OptoPort::MODULE_RAW_DATA = "MODULE_RAW_DATA";
	const char* OptoPort::PORT_RAW_DATA = "PORT_RAW_DATA";
	const char* OptoPort::CONST16 = "CONST16";
	const char* OptoPort::IN_SIGNAL = "IN_SIGNAL";


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

			TxSignal txs;

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


	QVector<OptoPort::TxSignal> OptoPort::getTxSignals()
	{
		QVector<TxSignal> txSignals;

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

		/*if (m_txRawDataSizeW == 0 &&
			m_txAnalogSignals.count() == 0 &&
			m_txDiscreteSignals.count() == 0)
		{
			m_txAnalogSignalsSizeW = 0;
			m_txDiscreteSignalsSizeW = 0;
			m_txDataSizeW = 0;
			return true;
		}*/

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

		for(TxSignal& txAnalogSignal : m_txAnalogSignals)
		{
			txAnalogSignal.address = address;

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

		for(TxSignal& txDiscreteSignal : m_txDiscreteSignals)
		{
			txDiscreteSignal.address = address;

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


	void OptoPort::sortTxSignals()
	{
		// sort txSignals in ascending alphabetical order
		//
		sortTxSignals(m_txAnalogSignals);
		sortTxSignals(m_txDiscreteSignals);
	}


	void OptoPort::sortTxSignals(QVector<TxSignal>& array)
	{
		int count = array.count();

		for(int i = 0; i < count - 1; i++)
		{
			for(int k = i + 1; k < count; k++)
			{
				if (array[i].appSignalID > array[k].appSignalID)
				{
					TxSignal temp = array[i];
					array[i] = array[k];
					array[k] = temp;
				}
			}
		}
	}


	bool OptoPort::parseRawDescriptionStr(Builder::IssueLogger* log)
	{
		if (log == nullptr)
		{
			assert(false);
			return false;
		}

		m_rawDataDescription.clear();

		m_rawDataDescriptionStr = m_rawDataDescriptionStr.trimmed().toUpper();

		if (m_rawDataDescriptionStr.isEmpty() == true)
		{
			return true;
		}

		bool result = true;

		// split string

		QStringList list = m_rawDataDescriptionStr.split("\n", QString::SkipEmptyParts);

		bool rawDataSizeFound = false;
		bool needRawDataSize = false;

		QString msg;

		for(QString str : list)
		{
			RawDataDescriptionItem item;
			bool res = true;

			str = str.trimmed();

			QString itemTypeStr = str.section("=", 0, 0).trimmed();

			if (itemTypeStr == RAW_DATA_SIZE)
			{
				if (rawDataSizeFound == true)
				{
					msg = QString("Duplicate RAW_DATA_SIZE section in opto-port '%1' raw data description.").arg(equipmentID());
					LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
					result = false;
					continue;
				}

				rawDataSizeFound = true;

				QString sizeStr = str.section("=", 1, 1).trimmed();

				if (sizeStr != "AUTO" )
				{
					int size = sizeStr.toInt(&res);

					if (res == false)
					{
						msg = QString("Invalid RAW_DATA_SIZE value in opto-port '%1' raw data description.").arg(equipmentID());
						LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
						result = false;
						continue;
					}

					item.rawDataSize = size;
					item.rawDataSizeIsAuto = false;
				}
				else
				{
					item.rawDataSizeIsAuto = true;
				}

				item.type = RawDataDescriptionItemType::RawDataSize;
				m_rawDataDescription.insert(RAW_DATA_SIZE_INDEX, item);		// RAW_DATA_SIZE always first item
				continue;
			}

			if (itemTypeStr == ALL_NATIVE_RAW_DATA)
			{
				needRawDataSize = true;

				item.type = RawDataDescriptionItemType::AllNativeRawData;
				m_rawDataDescription.append(item);

				continue;
			}

			if (itemTypeStr == MODULE_RAW_DATA)
			{
				needRawDataSize = true;

				QString placeStr = str.section("=", 1, 1).trimmed();

				int place = placeStr.toInt(&res);

				if (res == false)
				{
					msg = QString("Invalid MODULE_RAW_DATA value in opto-port '%1' raw data description.").arg(equipmentID());
					LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
					result = false;
					continue;
				}
				else
				{
					item.type = RawDataDescriptionItemType::ModuleRawData;
					item.modulePlace = place;
					m_rawDataDescription.append(item);
				}

				continue;
			}

			if (itemTypeStr == PORT_RAW_DATA)
			{
				needRawDataSize = true;

				QString portEquipmentID = str.section("=", 1, 1).trimmed();

				item.type = RawDataDescriptionItemType::PortRawData;
				item.portEquipmentID = portEquipmentID;
				m_rawDataDescription.append(item);
				continue;
			}

			if (itemTypeStr == CONST16)
			{
				needRawDataSize = true;

				QString constStr = str.section("=", 1, 1).trimmed();

				int const16 = constStr.toInt(&res);

				if (res == false)
				{
					msg = QString("Invalid CONST16 value in opto-port '%1' raw data description.").arg(equipmentID());
					LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
					result = false;
					continue;
				}
				else
				{
					item.type = RawDataDescriptionItemType::Const16;
					item.const16Value = const16;
					m_rawDataDescription.append(item);
				}

				continue;
			}

			if (itemTypeStr == IN_SIGNAL)
			{
				//	needRawDataSize = true;		- not need!

				bool parseResult = parseInSignalRawDescriptionStr(str, item, log);

				if (parseResult == true)
				{
					m_rawDataDescription.append(item);
				}

				result &= parseResult;

				continue;
			}

			msg = QString("Unknown item %1 in opto-port '%2' raw data description.").arg(itemTypeStr).arg(equipmentID());
			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			result = false;

			break;
		}

		if (needRawDataSize == true && rawDataSizeFound == false)
		{
			msg = QString("RAW_DATA_SIZE value is not found in opto-port '%1' raw data description.").arg(equipmentID());
			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			result = false;
		}

		return result;
	}


	bool OptoPort::parseInSignalRawDescriptionStr(const QString& str, RawDataDescriptionItem &item, Builder::IssueLogger* log)
	{
		QString msg;

		bool res = true;

		// InSignal description examples:
		//
		// IN_SIGNAL=#EXT_DISCRETE1,D,UINT,1,BE,3,4
		// IN_SIGNAL=#EXT_ANALOG1,A,FLOAT,32,LE,6,0

		QString inSignalDescription = str.section("=", 1).trimmed();

		QStringList descItemsList = inSignalDescription.split(",", QString::SkipEmptyParts);

		if (descItemsList.size() != 7)			// must be 7 parameters!
		{
			msg = QString("Invalid IN_SIGNAL description parameters count, must be 7. (Port '%1')").arg(m_equipmentID);
			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			return false;
		}

		item.type = RawDataDescriptionItemType::InSignal;

		// 1) AppSignalID

		item.appSignalID = descItemsList.at(0).trimmed();

		// 2) Signal type

		QString signalTypeStr = descItemsList.at(1).trimmed();

		if (signalTypeStr != "A" && signalTypeStr != "D")
		{
			msg = QString("Invalid IN_SIGNAL value of parameter SignalType in opto-port '%1' raw data description.").arg(m_equipmentID);
			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			return false;
		}

		if (signalTypeStr == "A")
		{
			item.signalType = E::SignalType::Analog;
		}
		else
		{
			item.signalType = E::SignalType::Discrete;
		}

		// 3) DataFormat

		QString dataFormatStr = descItemsList.at(2).trimmed();

		if (dataFormatStr != "FLOAT" &&
			dataFormatStr != "SINT" &&
			dataFormatStr != "UINT")
		{
			msg = QString("Invalid IN_SIGNAL value of parameter DataFormat in opto-port '%1' raw data description.").arg(m_equipmentID);
			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			return false;
		}

		if (dataFormatStr == "FLOAT")
		{
			item.dataFormat = E::DataFormat::Float;
		}
		else
		{
			if (dataFormatStr == "SINT")
			{
				item.dataFormat = E::DataFormat::SignedInt;
			}
			else
			{
				item.dataFormat = E::DataFormat::UnsignedInt;
			}
		}

		// 4) DataSize

		item.dataSize = descItemsList.at(3).trimmed().toInt(&res);

		if (res == false)
		{
			msg = QString("Invalid IN_SIGNAL value of parameter DataSize in opto-port '%1' raw data description.").arg(m_equipmentID);
			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			return false;
		}

		// 5) ByteOrder

		QString byteOrderStr = descItemsList.at(4).trimmed();

		if (byteOrderStr != "BE" && byteOrderStr != "LE")
		{
			msg = QString("Invalid IN_SIGNAL value of parameter ByteOrder in opto-port '%1' raw data description.").arg(m_equipmentID);
			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			return false;
		}

		if (byteOrderStr == "BE")
		{
			item.byteOrder = E::ByteOrder::BigEndian;
		}
		else
		{
			item.byteOrder = E::ByteOrder::LittleEndian;
		}

		// 6) OffsetW

		item.offsetW = descItemsList.at(5).trimmed().toInt(&res);

		if (res == false)
		{
			msg = QString("Invalid IN_SIGNAL value of parameter OffsetW in opto-port '%1' raw data description.").arg(m_equipmentID);
			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			return false;
		}

		// 7) BitNo

		item.bitNo = descItemsList.at(6).trimmed().toInt(&res);

		if (res == false)
		{
			msg = QString("Invalid IN_SIGNAL value of parameter BitNo in opto-port '%1' raw data description.").arg(m_equipmentID);
			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			return false;
		}

		return true;
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

		const RawDataDescriptionItem& rawDataSizeItem = m_rawDataDescription[RAW_DATA_SIZE_INDEX];

		if (rawDataSizeItem.type != Hardware::OptoPort::RawDataDescriptionItemType::RawDataSize)
		{
			msg = QString("Can't calculate txRawDataSizeW for opto-port '%1'. RAW_DATA_SIZE item not found.").arg(equipmentID());
			LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
			return false;
		}

		m_txRawDataSizeWCalculationStarted = true;		// to prevent cyclic calculatePortRawDataSize()

		if (rawDataSizeItem.rawDataSizeIsAuto == false)
		{
			// txRawDataSizeW set manually
			//
			setTxRawDataSizeW(rawDataSizeItem.rawDataSize);

			// LOG_MESSAGE(log, QString(tr("Port %1 manual rawDataSizeW = %2")).arg(m_equipmentID).arg(m_txRawDataSizeW));
			return true;
		}

		//LOG_MESSAGE(log, QString(tr("Port %1 rawDataSizeW calculation...")).arg(m_equipmentID));

		// automatic txRawDataSizeW calculation
		//
		bool result = true;

		int size = 0;
		int partSizeW = 0;

		for(const RawDataDescriptionItem& item : m_rawDataDescription)
		{
			switch(item.type)
			{
			case RawDataDescriptionItemType::RawDataSize:
				break;

			case RawDataDescriptionItemType::AllNativeRawData:

				partSizeW = DeviceHelper::getAllNativeRawDataSize(lm, log);;

				size += partSizeW;

				// LOG_MESSAGE(log, QString(tr("ALL_NATIVE_RAW_DATA sizeW = %1")).arg(partSizeW));
				break;

			case RawDataDescriptionItemType::ModuleRawData:
				{
					bool moduleIsFound = false;

					partSizeW = DeviceHelper::getModuleRawDataSize(lm, item.modulePlace, &moduleIsFound, log);

					size += partSizeW;

					// LOG_MESSAGE(log, QString(tr("MODULE_RAW_DATA=%1 sizeW = %2")).arg(item.modulePlace).arg(partSizeW));

					if (moduleIsFound == false)
					{
						msg = QString("Module on place %1 is not found (opto port '%2' raw data description).").arg(item.modulePlace).arg(equipmentID());
						LOG_ERROR_OBSOLETE(log, Builder::IssueType::AlCompiler,  msg);
						result = false;
					}
				}

				break;

			case RawDataDescriptionItemType::PortRawData:
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

			case RawDataDescriptionItemType::Const16:

				size++;

				// LOG_MESSAGE(log, QString(tr("CONST16=%1 sizeW = 1")).arg(item.const16Value));

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
			if (rdi.type != RawDataDescriptionItemType::InSignal)
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

		m_optoInterfaceDataOffset = m_lmDescription->optoInterface().optoInterfaceDataOffset;
		m_optoPortDataSize = m_lmDescription->optoInterface().optoPortDataSize;
		m_optoPortAppDataOffset = m_lmDescription->optoInterface().optoPortAppDataOffset;
		m_optoPortAppDataSize = m_lmDescription->optoInterface().optoPortAppDataSize;
		m_optoPortCount = m_lmDescription->optoInterface().optoPortCount;

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

			if (port->mode() == OptoPort::Mode::Optical)
			{
				optoPorts.append(port);
			}
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


	// return only opto-mode ports sorted by equipmentID ascending alphabetical order
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

			if (port->mode() == OptoPort::Mode::Optical)
			{
				ports.append(port);
			}
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


	int OptoModule::allOptoPortsTxDataSizeW()
	{
		QList<OptoPort*> ports = getOptoPorts();

		int txDataSizeW = 0;

		for(OptoPort* port : ports)
		{
			if (port != nullptr)
			{
				txDataSizeW += port->txDataSizeW();
			}
			else
			{
				assert(false);
			}
		}

		return txDataSizeW;
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
			if (port->mode() == Hardware::OptoPort::Mode::Serial)
			{
				// RS232/485 port has no linked ports
				//
				if (port->manualSettings() == true)
				{
					port->setRxDataSizeW(port->manualRxSizeW());
				}
				else
				{
					m_log->errALC5085(port->equipmentID(), port->connectionID());
				}

				continue;
			}

			if (port->isLinked() == false)
			{
				// optical port is not linked (used in connection)
				//
				assert(port->txDataSizeW() == 0);
				continue;
			}

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

				continue;
			}

			LOG_INTERNAL_ERROR(m_log)
			assert(false);      // unknown module type
			result = false;
			break;
		}

		return result;
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
										bool* signalAllreadyInList)
	{
		if (appSignal == nullptr ||
			signalAllreadyInList == nullptr)
		{
			assert(false);
			return false;
		}

		*signalAllreadyInList = false;

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
				*signalAllreadyInList = true;
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
				*signalAllreadyInList = true;
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
