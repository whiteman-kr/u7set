#include "OptoModule.h"

#include "../Builder/Builder.h"
#include "../Builder/ApplicationLogicCompiler.h"

namespace Hardware
{

	// ------------------------------------------------------------------
	//
	// OptoPort class implementation
	//
	// ------------------------------------------------------------------

	OptoPort::OptoPort(const QString& optoModuleStrID, DeviceController* optoPortController, int port) :
		m_deviceController(optoPortController),
		m_optoModuleStrID(optoModuleStrID)
	{
		if (optoPortController == nullptr)
		{
			assert(false);
			return;
		}

		m_strID = optoPortController->strId();
		m_port = port;
	}


	void OptoPort::addTxSignalStrID(const QString& signalStrID)
	{
		if (signalStrID.isEmpty())
		{
			assert(false);
			return;
		}

		m_txSignalsStrIDList.append(signalStrID);
	}


	void OptoPort::addTxSignalsStrID(const QStringList& signalStrIDList)
	{
		for(const QString& signalStrID : signalStrIDList)
		{
			if (signalStrID.isEmpty())
			{
				assert(false);
				continue;
			}

			m_txSignalsStrIDList.append(signalStrID);
		}
	}


	void OptoPort::addTxSignal(Signal* txSignal)
	{
		if (txSignal == nullptr)
		{
			assert(false);
			return;
		}

		TxSignal txs;

		txs.strID = txSignal->strID();
		txs.sizeBit = txSignal->dataSize();

		if (txSignal->isAnalog())
		{
			m_txAnalogSignalList.append(txs);
		}
		else
		{
			if (txSignal->isDiscrete())
			{
				m_txDiscreteSignalList.append(txs);
			}
			else
			{
				assert(false);  // unknown type of signal
			}
		}
	}


	bool OptoPort::calculateTxSignalsAddresses(OutputLog* log)
	{
		if (log == nullptr)
		{
			assert(false);
			return false;
		}

		bool result = true;

		m_txDataID = CRC32_INITIAL_VALUE;

		// mix port StrID in dataID
		//
		m_txDataID = CRC32(m_txDataID, C_STR(m_strID), m_strID.length(), false);

		// m_txDataID first placed in buffer
		//

		int txDataIDSizeW = sizeof(m_txDataID) / sizeof(quint16);

		Address16 address(txDataIDSizeW, 0);

		for(TxSignal& txAnalogSignal : m_txAnalogSignalList)
		{
			txAnalogSignal.address = address;

			address.addBit(txAnalogSignal.sizeBit);
			address.wordAlign();

			m_txDataID = CRC32(m_txDataID, C_STR(txAnalogSignal.strID), txAnalogSignal.strID.length(), false);
		}

		m_txAnalogSignalsSizeW = address.offset() - txDataIDSizeW ;

		for(TxSignal& txDiscreteSignal : m_txDiscreteSignalList)
		{
			txDiscreteSignal.address = address;

			assert(txDiscreteSignal.sizeBit == 1);

			address.add1Bit();

			m_txDataID = CRC32(m_txDataID, C_STR(txDiscreteSignal.strID), txDiscreteSignal.strID.length(), false);
		}

		m_txDataID = CRC32(m_txDataID, nullptr, 0, true);       // finalize CRC32 calculation

		address.wordAlign();

		m_txDiscreteSignalsSizeW = address.offset() - m_txAnalogSignalsSizeW - txDataIDSizeW;

		int fullTxDataSizeW = txDataIDSizeW + m_txAnalogSignalsSizeW + m_txDiscreteSignalsSizeW;

		if (manualSettings())
		{
			m_txDataSizeW = m_manualTxSizeW;

			if (fullTxDataSizeW > m_txDataSizeW)
			{
				LOG_ERROR_OBSOLETE(log, Builder::IssueType::NotDefined,
								   QString(tr("Manual txDataSizeW - %1 less then needed size %2 (connection %3)")).
								   arg(m_manualTxSizeW).arg(fullTxDataSizeW).
								   arg(m_connectionCaption));

				result = false;
			}
		}
		else
		{
			m_txDataSizeW = fullTxDataSizeW;
		}

		return result;
	}


	// ------------------------------------------------------------------
	//
	// OptoModule class implementation
	//
	// ------------------------------------------------------------------

	OptoModule::OptoModule(DeviceModule* module, OutputLog* log) :
		m_deviceModule(module),
		m_log(log)
	{
		if (module == nullptr || m_log == nullptr)
		{
			assert(false);
			return;
		}

		m_strID = module->strId();
		m_place = module->place();

		bool result = true;

		result &= Builder::DeviceHelper::getIntProperty(module, "OptoInterfaceDataOffset", &m_optoInterfaceDataOffset, log);
		result &= Builder::DeviceHelper::getIntProperty(module, "OptoPortDataSize", &m_optoPortDataSize, log);
		result &= Builder::DeviceHelper::getIntProperty(module, "OptoPortAppDataOffset", &m_optoPortAppDataOffset, log);
		result &= Builder::DeviceHelper::getIntProperty(module, "OptoPortAppDataSize", &m_optoPortAppDataSize, log);
		result &= Builder::DeviceHelper::getIntProperty(module, "OptoPortCount", &m_optoPortCount, log);

		if (result == false)
		{
			return;
		}

		int findPortCount = 0;

		for(int i = 0; i < m_optoPortCount; i++)
		{
			QString portStrID = QString("%1_OPTOPORT0%2").arg(module->strId()).arg(i + 1);

			int childrenCount = module->childrenCount();

			for(int c = 0; c < childrenCount; c++)
			{
				DeviceObject* child = module->child(c);

				if (child == nullptr)
				{
					assert(false);
					continue;
				}

				if (child->isController() && child->strId() == portStrID)
				{
					DeviceController* optoPortController = child->toController();

					if (optoPortController == nullptr)
					{
						assert(false);
						LOG_INTERNAL_ERROR(m_log);
						return;
					}

					OptoPort* optoPort = new OptoPort(module->strId(), optoPortController, i + 1);

					m_ports.insert(optoPortController->strId(), optoPort);

					findPortCount++;

					break;
				}
			}
		}

		if (findPortCount != m_optoPortCount)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
							   QString(tr("Not all opto-port controllers found in module '%1'")).arg(module->strId()));
			return;
		}

		if (isLM() == true)
		{
			// OCM module
			//
			m_lmStrID = module->strId();
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
								   QString(tr("Not found chassis to module '%1'")).arg(module->strId()));
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

					m_lmStrID = childModule->strId();
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


	QList<OptoPort*> OptoModule::getRS232Ports()
	{
		QList<OptoPort*> rs232PortList;

		for(OptoPort* port : m_ports)
		{
			if (port == nullptr)
			{
				assert(false);
				continue;
			}

			if (port->mode() == OptoPort::Mode::Serial)
			{
				rs232PortList.append(port);
			}
		}

		return rs232PortList;
	}


	QList<OptoPort*> OptoModule::ports()
	{
		QList<OptoPort*> ports;

		for(OptoPort* port : m_ports)
		{
			ports.append(port);
		}

		return ports;
	}


	// ------------------------------------------------------------------
	//
	// OptoModuleStorage class implementation
	//
	// ------------------------------------------------------------------

	OptoModuleStorage::OptoModuleStorage(EquipmentSet* equipmentSet, OutputLog *log) :
		m_equipmentSet(equipmentSet),
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
				m_ports.insert(optoPort->strID(), optoPort);
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

		OptoModule* optoModule = new OptoModule(module, m_log);

		if (optoModule->isValid() == false)
		{
			delete optoModule;
			return false;
		}

		m_modules.insert(module->strId(), optoModule);

		m_lmAssociatedModules.insertMulti(optoModule->lmStrID(), optoModule);

		return true;
	}


	OptoModule* OptoModuleStorage::getOptoModule(const QString& optoModuleStrID)
	{
		if (m_modules.contains(optoModuleStrID))
		{
			return m_modules[optoModuleStrID];
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

		if (m_modules.contains(optoPort->optoModuleStrID()))
		{
			return m_modules[optoPort->optoModuleStrID()];
		}

		return nullptr;
	}


	OptoPort* OptoModuleStorage::getOptoPort(const QString& optoPortStrID)
	{
		if (m_ports.contains(optoPortStrID))
		{
			return m_ports[optoPortStrID];
		}

		return nullptr;
	}

	Hardware::OptoPort* OptoModuleStorage::jsGetOptoPort(const QString& optoPortStrID)
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
				if (port->manualSettings())
				{
					port->setRxDataSizeW(port->manualRxSizeW());
				}

				continue;
			}

			if (port->connectionCaption().isEmpty())
			{
				// optical port is not linked (used in connection)
				//
				assert(port->txDataSizeW() == 0);
				continue;
			}

			if (port->txDataSizeW() > 0)
			{
				Hardware::OptoPort* linkedPort = getOptoPort(port->linkedPortStrID());

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
										   arg(linkedPort->strID()).arg(port->strID()).
										   arg(port->connectionCaption()));
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


	bool OptoModuleStorage::calculatePortsTxRxStartAddresses()
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

			int txStartAddress = 0;

			QList<OptoPort*> portsList = module->ports();

			for(OptoPort* port : portsList)
			{
				if (port == nullptr)
				{
					LOG_INTERNAL_ERROR(m_log);
					assert(false);
					return false;
				}

				if (module->isLM())
				{
					port->setTxStartAddress(txStartAddress);

					if (port->txDataSizeW() > module->m_optoPortAppDataSize)
					{
						LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
										   QString(tr("Data size %1 to transmit on port '%2' exceeded limit OptoPortAppDataSize = %3 (connection %4)")).
										   arg(txStartAddress).arg(port->strID()).
										   arg(module->m_optoPortAppDataSize).arg(port->connectionCaption()));

						result = false;
						break;
					}

					continue;
				}

				if (module->isOCM())
				{
					// all data to transmit via all ports of OCM disposed in one buffer with max size - OptoPortAppDataSize
					//
					port->setTxStartAddress(txStartAddress);

					txStartAddress += port->txDataSizeW();

					if (txStartAddress > module->m_optoPortAppDataSize)
					{
						LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined,
										   QString(tr("Data size %1 to transmit on port '%2' exceeded limit OptoPortAppDataSize = %3 (connection %4)")).
										   arg(txStartAddress).arg(port->strID()).
										   arg(module->m_optoPortAppDataSize).arg(port->connectionCaption()));

						result = false;
						break;
					}
					continue;
				}

				LOG_INTERNAL_ERROR(m_log)
				assert(false);      // unknown module type
				result = false;
				break;
			}

		}

		return result;
	}

}
