#include "OptoModule.h"

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


	void OptoPort::addTxSignalID(const QString& signalID)
	{
		if (signalID.isEmpty())
		{
			assert(false);
			return;
		}

		m_txSignalsIDList.append(signalID);
	}


	void OptoPort::addTxSignalsID(const QStringList& signalStrIDList)
	{
		for(const QString& signalStrID : signalStrIDList)
		{
			if (signalStrID.isEmpty())
			{
				assert(false);
				continue;
			}

			m_txSignalsIDList.append(signalStrID);
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

		txs.strID = txSignal->appSignalID();
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
		m_txDataID = CRC32(m_txDataID, C_STR(m_equipmentID), m_equipmentID.length(), false);

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
								   arg(m_connectionID));

				result = false;
			}
		}
		else
		{
			m_txDataSizeW = fullTxDataSizeW;
		}

		return result;
	}


	bool OptoPort::isTxSignalIDExists(const QString& appSignalID)
	{
		for(const QString& id : m_txSignalsIDList)
		{
			if (id == appSignalID)
			{
				return true;
			}
		}

		return false;
	}


	// ------------------------------------------------------------------
	//
	// OptoModule class implementation
	//
	// ------------------------------------------------------------------

	OptoModule::OptoModule(DeviceModule* module, Builder::IssueLogger* log) :
		m_deviceModule(module),
		m_log(log)
	{
		if (module == nullptr || m_log == nullptr)
		{
			assert(false);
			return;
		}

		m_equipmentID = module->equipmentIdTemplate();
		m_place = module->place();

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
			ports.append(port);
		}

		return ports;
	}


	bool OptoModule::calculateTxStartAddresses()
	{
		quint16 txStartAddress = 0;

		//qDebug() << QString("TxStartAddresses of  module %1").arg(m_strID);

		for(OptoPort* port : m_ports)
		{
			if (port == nullptr)
			{
				assert(false);
				continue;
			}

			port->setTxStartAddress(txStartAddress);

			//qDebug() << QString("Port %1 = %2").arg(port->strID()).arg(port->txStartAddress());

			txStartAddress += port->txDataSizeW();
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

	OptoModuleStorage::OptoModuleStorage(EquipmentSet* equipmentSet, Builder::IssueLogger *log) :
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

		OptoModule* optoModule = new OptoModule(module, m_log);

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

			if (port->connectionID().isEmpty())
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
										   arg(txStartAddress).arg(port->equipmentID()).
										   arg(module->m_optoPortAppDataSize).arg(port->connectionID()));

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
										   arg(txStartAddress).arg(port->equipmentID()).
										   arg(module->m_optoPortAppDataSize).arg(port->connectionID()));

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


	bool OptoModuleStorage::addTxSignal(const QString& connectionID,
										const QString& lmID,
										const QString& appSignalID,
										bool* signalAllreadyInList)
	{
		if (signalAllreadyInList == nullptr)
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
			if (p1->isTxSignalIDExists(appSignalID))
			{
				*signalAllreadyInList = true;
			}
			else
			{
				p1->addTxSignalID(appSignalID);
			}
			return true;
		}

		if (m2->lmID() == lmID)
		{
			if (p2->isTxSignalIDExists(appSignalID))
			{
				*signalAllreadyInList = true;
			}
			else
			{
				p2->addTxSignalID(appSignalID);
			}
			return true;
		}

		assert(false);		// WTF?

		return false;
	}

}
