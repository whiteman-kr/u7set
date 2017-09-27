#include "../lib/DeviceHelper.h"

#include "ApplicationLogicCompiler.h"
#include "SoftwareCfgGenerator.h"
#include "BdfFile.h"

#include "../lib/ServiceSettings.h"


namespace Builder
{

	// ---------------------------------------------------------------------------------
	//
	//	ApplicationLogicCompiler class implementation
	//
	// ---------------------------------------------------------------------------------

	IssueLogger* ApplicationLogicCompiler::m_log = nullptr;


	ApplicationLogicCompiler::ApplicationLogicCompiler(Hardware::SubsystemStorage *subsystems,
													   Hardware::EquipmentSet *equipmentSet,
													   Hardware::OptoModuleStorage *optoModuleStorage,
													   Hardware::ConnectionStorage *connections,
													   SignalSet* signalSet,
													   LmDescriptionSet* lmDescriptions,
													   AppLogicData* appLogicData,
													   Tuning::TuningDataStorage* tuningDataStorage,
													   ComparatorStorage* comparatorStorage,
													   VFrame30::BusSet* busSet,
													   BuildResultWriter* buildResultWriter,
													   IssueLogger *log) :
		m_subsystems(subsystems),
		m_equipmentSet(equipmentSet),
		m_optoModuleStorage(optoModuleStorage),
		m_signals(signalSet),
		m_lmDescriptions(lmDescriptions),
		m_appLogicData(appLogicData),
		m_tuningDataStorage(tuningDataStorage),
		m_cmpStorage(comparatorStorage),
		m_resultWriter(buildResultWriter),
		m_connections(connections),
		m_busSet(busSet)
	{
		if (m_log == nullptr)
		{
			m_log = log;
		}
		else
		{
			// only one instance of ApplicationLogicCompiler can be created !
			//
			assert(false);
		}

		if (m_equipmentSet != nullptr)
		{
			m_deviceRoot = m_equipmentSet->root();
		}
	}

	ApplicationLogicCompiler::~ApplicationLogicCompiler()
	{
		clear();
	}

	bool ApplicationLogicCompiler::run()
	{
		if (m_log == nullptr)
		{
			assert(m_log != nullptr);
			return false;
		}

		if (m_subsystems == nullptr ||
			m_equipmentSet == nullptr ||
			m_signals == nullptr ||
			m_lmDescriptions == nullptr ||
			m_appLogicData == nullptr ||
			m_tuningDataStorage == nullptr ||
			m_cmpStorage == nullptr ||
			m_resultWriter == nullptr ||
			m_connections == nullptr ||
			m_busSet == nullptr)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, tr("%1: Invalid params. Compilation aborted.").arg(__FUNCTION__));
			return false;
		}

		DeviceHelper::init();

		m_signals->resetAddresses();

		ApplicationLogicCompilerProc appLogicCompilerProcs[] =
		{
			&ApplicationLogicCompiler::findLMs,
			&ApplicationLogicCompiler::checkAppSignals,
			&ApplicationLogicCompiler::expandBusSignals,
			&ApplicationLogicCompiler::prepareOptoConnectionsProcessing,
			&ApplicationLogicCompiler::checkLmIpAddresses,
			&ApplicationLogicCompiler::compileModulesLogicsPass1,
			&ApplicationLogicCompiler::processBvbModules,
			&ApplicationLogicCompiler::compileModulesLogicsPass2,
			&ApplicationLogicCompiler::writeSerialDataXml,
			&ApplicationLogicCompiler::writeOptoConnectionsReport,
//			&ApplicationLogicCompiler::writeOptoModulesReport,
			&ApplicationLogicCompiler::writeOptoVhdFiles,
			&ApplicationLogicCompiler::writeAppSignalSetFile,
		};

		bool result = true;

		int procsCount = sizeof(appLogicCompilerProcs) / sizeof(ApplicationLogicCompilerProc);

		for(int i = 0; i < procsCount; i++)
		{
			if (isBuildCancelled() == true)
			{
				result = false;
				break;
			}

			result &= (this->*appLogicCompilerProcs[i])();		// call next ApplicationLogicCompiler procedure

			if (result == false)
			{
				break;
			}
		}

		clear();

		DeviceHelper::shutdown();

		return result;
	}

	bool ApplicationLogicCompiler::isBuildCancelled()
	{
		if (QThread::currentThread()->isInterruptionRequested() == true)
		{
			return true;
		}

		return false;
	}

	bool ApplicationLogicCompiler::findLMs()
	{
		// find all logic modules (LMs) in project
		// fills m_lm vector
		//
		m_lm.clear();

		findLM(m_deviceRoot);

		if (m_lm.count() == 0)
		{
			LOG_MESSAGE(m_log, tr("Logic modules (LMs) not found!"));
		}
		else
		{
			LOG_MESSAGE(m_log, QString(tr("Found logic modules (LMs): %1")).arg(m_lm.count()));
		}

		return true;
	}

	void ApplicationLogicCompiler::findLM(Hardware::DeviceObject* startFromDevice)
	{
		// find logic modules (LMs), recursive
		//
		if (startFromDevice == nullptr)
		{
			LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("%1: DeviceObject null pointer!")).arg(__FUNCTION__));
			assert(false);
			return;
		}

		if (startFromDevice->deviceType() == Hardware::DeviceType::Signal)
		{
			return;
		}

		if (startFromDevice->deviceType() == Hardware::DeviceType::Module)
		{
			Hardware::DeviceModule* module = reinterpret_cast<Hardware::DeviceModule*>(startFromDevice);

			if (module->moduleFamily() == Hardware::DeviceModule::FamilyType::LM)
			{
				Hardware::DeviceObject* parent = startFromDevice->parent();

				if (parent != nullptr)
				{
					if (parent->deviceType() == Hardware::DeviceType::Chassis)
					{
						// LM must be installed in the chassis
						//
						m_lm.append(reinterpret_cast<Hardware::DeviceModule*>(startFromDevice));
					}
					else
					{
						LOG_WARNING_OBSOLETE(m_log, Builder::IssueType::NotDefined, QString(tr("LM %1 is not installed in the chassis")).arg(module->equipmentIdTemplate()));
					}
				}
			}

			return;
		}

		int childrenCount = startFromDevice->childrenCount();

		for(int i = 0; i < childrenCount; i++)
		{
			Hardware::DeviceObject* device = startFromDevice->child(i);

			findLM(device);
		}
	}

	bool ApplicationLogicCompiler::checkAppSignals()
	{
		bool result = true;

		int count = m_signals->count();

		if (count == 0)
		{
			return true;
		}

		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, QString(tr("Checking application signals")));

		QHash<QString, int> appSignalIDs;
		QHash<QString, int> customAppSignalIDs;

		appSignalIDs.reserve(count * 1.3);

		m_busSignals.clear();

		for(int i = 0; i < count; i++)
		{
			Signal& s = (*m_signals)[i];

			if (s.ID() > m_maxSignalID)
			{
				m_maxSignalID = s.ID();
			}

			// check AppSignalID
			//
			if (appSignalIDs.contains(s.appSignalID()) == true)
			{
				// Application signal identifier '%1' is not unique.
				//
				m_log->errALC5016(s.appSignalID());
				result = false;
				continue;
			}

			appSignalIDs.insert(s.appSignalID(), i);

			// check CustomAppSignalID
			//
			if (customAppSignalIDs.contains(s.customAppSignalID()) == true)
			{
				// Custom application signal identifier '%1' is not unique.
				//
				m_log->errALC5017(s.customAppSignalID());
				result = false;
				continue;
			}

			customAppSignalIDs.insert(s.customAppSignalID(), i);

			// check other signal properties
			//
			if (s.byteOrder() != E::ByteOrder::BigEndian)
			{
				// Signal '%1' has Little Endian byte order.
				//
				m_log->wrnALC5070(s.appSignalID());
			}

			switch(s.signalType())
			{
			case E::SignalType::Discrete:
				if (s.dataSize() != 1)
				{
					// Discrete signal '%1' must have DataSize equal to 1.
					//
					m_log->errALC5014(s.appSignalID());
					result = false;
				}
				break;

			case E::SignalType::Analog:
				if (s.dataSize() != 32)
				{
					// Analog signal '%1' must have DataSize equal to 32.
					//
					m_log->errALC5015(s.appSignalID());
					result = false;
				}

				if (s.coarseAperture() <= 0 || s.fineAperture() <= 0)
				{
					// Analog signal '%1' aperture should be greate then 0.
					//
					m_log->errALC5090(s.appSignalID());
					result = false;
				}

				if (s.coarseAperture() < s.fineAperture())
				{
					// Coarse aperture of signal '%1' less then fine aperture.
					//
					m_log->wrnALC5093(s.appSignalID());
					result = false;
				}

				break;

			case E::SignalType::Bus:
				if (m_busSet->hasBus(s.busTypeID()) == false)
				{
					//  Bus type ID '%1' of signal '%2' is undefined.
					//
					m_log->errALC5092(s.busTypeID(), s.appSignalID());
					result = false;
				}
				else
				{
					m_busSignals.insert(s.appSignalID(), i);
				}
				break;

			default:
				assert(false);
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			// check tuningable signals properties
			//
			if (s.enableTuning() == false)
			{
				continue;
			}

			if (s.isAnalog() == true)
			{
				if (s.lowEngeneeringUnits() >= s.highEngeneeringUnits())
				{
					// LowEngeneeringUnits property of tuningable signal '%1' must be greate than HighEngeneeringUnits.
					//
					m_log->errALC5068(s.appSignalID());
					result = false;
				}
				else
				{
					// limits OK
					//
					if (s.tuningDefaultValue() < s.lowEngeneeringUnits() ||
						s.tuningDefaultValue() > s.highEngeneeringUnits())
					{
						// TuningDefaultValue property of tuningable signal '%1' must be in range from LowEngeneeringUnits toHighEngeneeringUnits.
						//
						m_log->errALC5069(s.appSignalID());
						result = false;
					}
				}
			}
		}

		if (result == true)
		{
			LOG_SUCCESS(m_log, QString(tr("Ok")))
		}

		return result;
	}

	bool ApplicationLogicCompiler::expandBusSignals()
	{
		if (m_busSignals.count() == 0)
		{
			return true;
		}

		bool result = true;

		for(QString busSignalID : m_busSignals)
		{
			Signal* s = m_signals->getSignal(busSignalID);

			if (s == nullptr)
			{
				result = false;
				LOG_INTERNAL_ERROR(m_log);
				continue;
			}

			const VFrame30::Bus& bus = m_busSet->bus(s->busTypeID());

			const std::vector<VFrame30::BusSignal>& busSignals = bus.busSignals();

			for(const VFrame30::BusSignal& busSignal : busSignals)
			{
				result &= appendBusSignal(*s, bus, busSignal);
			}
		}

		return result;
	}

	bool ApplicationLogicCompiler::appendBusSignal(const Signal& s, const VFrame30::Bus& bus, const VFrame30::BusSignal& busSignal)
	{
		bool result = true;

		Signal* newSignal = new Signal();

		newSignal->setAppSignalID(QString(s.appSignalID() + BUS_SIGNAL_ID_SEPARATOR + busSignal.signalId()).toUpper());
		newSignal->setCustomAppSignalID(QString(s.customAppSignalID() + BUS_SIGNAL_ID_SEPARATOR + busSignal.signalId()).toUpper());

/*
				// Signal identificators
				//
				QString m_appSignalID;
				QString m_customAppSignalID;
				QString m_caption;
				QString m_equipmentID;
				QString m_busTypeID;											// only for: m_signalType == E::SignalType::Bus
				E::Channel m_channel = E::Channel::A;

				// Signal type
				//
				E::SignalType m_signalType = E::SignalType::Analog;
				E::SignalInOutType m_inOutType = E::SignalInOutType::Internal;

				// Signal format
				//
				int m_dataSize = 32;											// signal data size in bits
				E::ByteOrder m_byteOrder = E::ByteOrder::BigEndian;

				// Analog signal properties
				//
				E::AnalogAppSignalFormat m_analogSignalFormat =					// only for m_signalType == E::SignalType::Analog
										E::AnalogAppSignalFormat::Float32;		// discrete signals is always treat as UnsignedInt and dataSize == 1

				QString m_unit;

				int m_lowADC = 0;
				int m_highADC = 0xFFFF;

				double m_lowEngeneeringUnits = 0;								// low physical value for input range
				double m_highEngeneeringUnits = 100;							// high physical value for input range

				double m_lowValidRange = 0;
				double m_highValidRange = 100;

				double m_filteringTime = 0.005;
				double m_spreadTolerance = 2;

				// Analog input/output signals properties
				//
				double m_electricLowLimit = 0;										// low electric value for input range
				double m_electricHighLimit = 0;										// high electric value for input range
				E::ElectricUnit m_electricUnit = E::ElectricUnit::NoUnit;		// electric unit for input range (mA, mV, Ohm, V ....)
				E::SensorType m_sensorType = E::SensorType::NoSensorType;			// electric sensor type for input range (was created for m_inputUnitID)
				E::OutputMode m_outputMode = E::OutputMode::Plus0_Plus5_V;			// output electric range (or mode ref. OutputModeStr[])

				// Tuning signal properties
				//
				bool m_enableTuning = false;
				float m_tuningDefaultValue = 0;
				float m_tuningLowBound = 0;
				float m_tuningHighBound = 100;

				// Signal properties for MATS
				//
				bool m_acquire = true;
				int m_decimalPlaces = 2;
				double m_coarseAperture = 1;
				double m_fineAperture = 0.5;
				bool m_adaptiveAperture = false;

				// Signal fields from database
				//
				int m_ID = 0;
				int m_signalGroupID = 0;
				int m_signalInstanceID = 0;
				int m_changesetID = 0;
				bool m_checkedOut = false;
				int m_userID = 0;
				QDateTime m_created;
				bool m_deleted = false;
				QDateTime m_instanceCreated;
				VcsItemAction m_instanceAction = VcsItemAction::Added;*/


		if (result == true)
		{
			m_maxSignalID++;

			m_signals->append(m_maxSignalID, newSignal);
		}
		else
		{
			delete newSignal;
		}

		return result;
	}

	bool ApplicationLogicCompiler::prepareOptoConnectionsProcessing()
	{
		if (m_optoModuleStorage == nullptr ||
			m_connections == nullptr)
		{
			LOG_INTERNAL_ERROR(m_log);
			ASSERT_RETURN_FALSE;
		}

		if (m_connections->count() == 0)
		{
			LOG_MESSAGE(m_log, QString(tr("No opto connections found")));
			LOG_SUCCESS(m_log, QString(tr("Ok")));
			return true;
		}

		if (m_optoModuleStorage->appendOptoModules() == false)
		{
			return false;
		}

		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, QString(tr("Checking opto connections")));

		if (m_optoModuleStorage->appendAndCheckConnections(*m_connections) == false)
		{
			return false;
		}

		return true;
	}

	bool ApplicationLogicCompiler::checkLmIpAddresses()
	{
		LOG_EMPTY_LINE(m_log);

		LOG_MESSAGE(m_log, QString(tr("Check LM's ethernet adapters IP addresses...")));

		bool result = true;

		QHash<QString, Hardware::DeviceModule*> ip2Modules;

		for(Hardware::DeviceModule* lm : m_lm)
		{
			if (lm == nullptr)
			{
				LOG_INTERNAL_ERROR(m_log);
				assert(false);
				return false;
			}

			for(int i = 0; i < SoftwareCfgGenerator::LM_ETHERNET_ADAPTERS_COUNT; i++)
			{
				SoftwareCfgGenerator::LmEthernetAdapterNetworkProperties lmNetProperties;

				int ethernetAdapterNo = SoftwareCfgGenerator::LM_ETHERNET_ADAPTER1 + i;

				lmNetProperties.getLmEthernetAdapterNetworkProperties(lm, ethernetAdapterNo, m_log);


				switch(ethernetAdapterNo)
				{
				case SoftwareCfgGenerator::LM_ETHERNET_ADAPTER1:
					// tuning data adapter
					//
					if (lmNetProperties.tuningEnable == true)
					{
						lmNetProperties.tuningIP = lmNetProperties.tuningIP.trimmed();

						if (ip2Modules.contains(lmNetProperties.tuningIP) == true)
						{
							Hardware::DeviceModule* lm1 = ip2Modules[lmNetProperties.tuningIP];

							m_log->errEQP6003(lm1->equipmentId(), lm->equipmentId(), lmNetProperties.tuningIP, lm1->uuid(), lm->uuid());

							result = false;
						}
						else
						{
							ip2Modules.insert(lmNetProperties.tuningIP, lm);
						}
					}
					break;

				case SoftwareCfgGenerator::LM_ETHERNET_ADAPTER2:
				case SoftwareCfgGenerator::LM_ETHERNET_ADAPTER3:
					// appllication and diagnostics data adapters
					//
					if (lmNetProperties.appDataEnable == true)
					{
						lmNetProperties.appDataIP = lmNetProperties.appDataIP.trimmed();

						if (ip2Modules.contains(lmNetProperties.appDataIP) == true)
						{
							Hardware::DeviceModule* lm1 = ip2Modules[lmNetProperties.appDataIP];

							m_log->errEQP6003(lm1->equipmentId(), lm->equipmentId(), lmNetProperties.appDataIP, lm1->uuid(), lm->uuid());

							result = false;
						}
						else
						{
							ip2Modules.insert(lmNetProperties.appDataIP, lm);
						}
					}

					if (lmNetProperties.diagDataEnable == true)
					{
						lmNetProperties.diagDataIP = lmNetProperties.diagDataIP.trimmed();

						if (ip2Modules.contains(lmNetProperties.diagDataIP) == true)
						{
							Hardware::DeviceModule* lm1 = ip2Modules[lmNetProperties.diagDataIP];

							m_log->errEQP6003(lm1->equipmentId(), lm->equipmentId(), lmNetProperties.diagDataIP, lm1->uuid(), lm->uuid());

							result = false;
						}
						else
						{
							ip2Modules.insert(lmNetProperties.diagDataIP, lm);
						}
					}

					break;

				default:
					assert(false);
				}
			}
		}

		if (result == true)
		{
			LOG_OK(m_log);
		}

		return result;
	}

	bool ApplicationLogicCompiler::compileModulesLogicsPass1()
	{
		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, QString(tr("Application logic compiler pass #1...")));

		bool result = true;

		// first compiler pass
		//
		for(int i = 0; i < m_lm.count(); i++)
		{
			ModuleLogicCompiler* moduleLogicCompiler = new ModuleLogicCompiler(*this, m_lm[i]);

			m_moduleCompilers.append(moduleLogicCompiler);

			result &= moduleLogicCompiler->pass1();

			if (isBuildCancelled() == true)
			{
				result = false;
				break;
			}
		}

		return result;
	}

	bool ApplicationLogicCompiler::processBvbModules()
	{
		return m_optoModuleStorage->processBvbModules();
	}

	bool ApplicationLogicCompiler::compileModulesLogicsPass2()
	{
		bool result = true;

		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, QString(tr("Application logic compiler pass #2...")));

		// second compiler pass
		//
		for(int i = 0; i < m_moduleCompilers.count(); i++)
		{
			ModuleLogicCompiler* moduleCompiler = m_moduleCompilers[i];

			if (moduleCompiler == nullptr)
			{
				assert(false);
				result = false;
				break;
			}

			result &= moduleCompiler->pass2();

			if (isBuildCancelled() == true)
			{
				result = false;
				break;
			}
		}

		return result;
	}

	bool ApplicationLogicCompiler::writeBinCodeForLm(QString subsystemID, int subsystemKey, QString lmEquipmentID, QString lmCaption, int lmNumber, int frameSize, int frameCount, quint64 uniqueID, ApplicationLogicCode& appLogicCode)
	{
		if (m_resultWriter == nullptr)
		{
			ASSERT_RETURN_FALSE
		}

		bool result = true;

		int metadataFieldsVersion = 0;

		QStringList metadataFields;

		appLogicCode.getAsmMetadataFields(metadataFields, &metadataFieldsVersion);

		MultichannelFile* multichannelFile = m_resultWriter->createMutichannelFile(subsystemID, subsystemKey, lmEquipmentID, lmCaption, frameSize, frameCount,metadataFieldsVersion, metadataFields);

		if (multichannelFile != nullptr)
		{
			QByteArray binCode;

			appLogicCode.getBinCode(binCode);

			std::vector<QVariantList> metadata;

			appLogicCode.getAsmMetadata(metadata);

			result &= multichannelFile->setChannelData(lmNumber, frameSize, frameCount, uniqueID, binCode, metadata);
		}
		else
		{
			result = false;
		}

		return result;
	}

	bool ApplicationLogicCompiler::writeSerialDataXml()
	{
		return m_optoModuleStorage->writeSerialDataXml(m_resultWriter);
	}

	bool ApplicationLogicCompiler::writeOptoConnectionsReport()
	{
		int count = m_connections->count();

		if (count == 0)
		{
			return true;
		}

		QStringList list;

		QString delim = "==================================================================================";
		QString delim2 = "----------------------------------------------------------------------------------";

		QString str;

		for(int i = 0; i < count; i++)
		{
			std::shared_ptr<Hardware::Connection> cn = m_connections->get(i);

			if (cn == nullptr)
			{
				assert(false);
				continue;
			}

			list.append(delim);

			list.append(QString(tr("Connection ID:\t\t\t%1")).arg(cn->connectionID()));
			list.append(QString(tr("Link ID:\t\t\t%1\n")).arg(cn->linkID()));
			list.append(QString(tr("Mode:\t\t\t\t%1")).arg(cn->typeStr()));
			list.append(QString(tr("Settings:\t\t\t%1")).arg(cn->manualSettings() == true ? "Manual" : "Auto"));
			list.append(QString(tr("Data ID control:\t\t%1\n")).arg(cn->disableDataId() == true ? "Disabled" : "Enabled"));

			list.append(QString(tr("Port1 equipmentID:\t\t%1")).arg(cn->port1EquipmentID()));

			if (cn->isPortToPort() == true)
			{
				list.append(QString(tr("Port2 equipmentID:\t\t%1")).arg(cn->port2EquipmentID()));
			}

			list.append(delim);
			list.append("");

			switch(cn->type())
			{
			case Hardware::Connection::Type::SinglePort:
				{
					Hardware::OptoPortShared p1 = m_optoModuleStorage->getOptoPort(cn->port1EquipmentID());

					if (p1 != nullptr)
					{
						p1->writeInfo(list);
					}
					else
					{
						assert(false);
					}
				}
				break;

			case Hardware::Connection::Type::PortToPort:
				{
					Hardware::OptoPortShared p1 = m_optoModuleStorage->getOptoPort(cn->port1EquipmentID());

					if (p1 != nullptr)
					{
						p1->writeInfo(list);
					}
					else
					{
						assert(false);
					}

					list.append(delim2);
					list.append("");

					Hardware::OptoPortShared p2 = m_optoModuleStorage->getOptoPort(cn->port2EquipmentID());

					if (p2 != nullptr)
					{
						p2->writeInfo(list);
					}
					else
					{
						assert(false);
					}
				}
				break;

			default:
				assert(false);
				return false;
			}
		}

		m_resultWriter->addFile("Reports", "connections.txt", "", "", list);

		return true;
	}

	bool ApplicationLogicCompiler::writeOptoVhdFiles()
	{
		int count = m_connections->count();

		if (count == 0)
		{
			return true;
		}

		QStringList list;

		QString delim = "--------------------------------------------------------------------";

		QString str;

		for(int i = 0; i < count; i++)
		{
			std::shared_ptr<Hardware::Connection> cn = m_connections->get(i);

			if (cn == nullptr)
			{
				assert(false);
				continue;
			}

			if (cn->generateVHDFile() == false)
			{
				continue;
			}

			if (cn->isPortToPort() == true)
			{
				Hardware::OptoPortShared p1 = m_optoModuleStorage->getOptoPort(cn->port1EquipmentID());
				Hardware::OptoPortShared p2 = m_optoModuleStorage->getOptoPort(cn->port2EquipmentID());

				writeOptoVhdFile(cn->connectionID(), p1, p2);
				writeOptoVhdFile(cn->connectionID(), p2, p1);
			}
			else
			{
//				Hardware::OptoPort* p1 = m_optoModuleStorage->getOptoPort(cn->port1EquipmentID());
//				writeOptoVhdFile(cn->connectionID(), p1);
			}
		}

		return true;
	}

	bool ApplicationLogicCompiler::writeOptoVhdFile(const QString& connectionID, Hardware::OptoPortShared outPort, Hardware::OptoPortShared inPort)
	{
		if (outPort == nullptr || inPort == nullptr)
		{
			assert(false);
			return false;
		}

		if (outPort->txSignalsCount() == 0)
		{
			return true;
		}

		QString outPortID = outPort->equipmentID().toLower();
		QString inPortID = inPort->equipmentID().toLower();

		QString vhdFileName = QString("%1.vhd").arg(inPortID);
		QString bdfFileName = QString("%1.bdf").arg(inPortID);

		BdfFile bdfFile;

		QStringList list;
		QString str;

		int inBusWidth = outPort->txDataSizeW() * SIZE_16BIT;

		quint32 dataID = outPort->txDataID();

		QVector<Hardware::TxRxSignalShared> txAnalogs;

		outPort->getTxAnalogSignals(txAnalogs, false);

		QVector<Hardware::TxRxSignalShared> txDiscretes;

		outPort->getTxDiscreteSignals(txDiscretes, false);

		list.append("--");
		list.append("-- This file has been generated automatically by RPCT software");
		list.append("--");

		BuildInfo bi = m_resultWriter->buildInfo();

		str = QString("-- Project:\t%1").arg(bi.project);
		list.append(str);

		str = QString("-- Build No:\t%1").arg(bi.id);
		list.append(str);

		str = QString("-- Build type:\t%1").arg(bi.release == true ? "Release" : "Debug");
		list.append(str);

		str = QString("-- Build date:\t%1").arg(bi.dateStr());
		list.append(str);

		str = QString("-- User:\t%1").arg(bi.user);
		list.append(str);

		str = QString("-- Host:\t%1").arg(bi.workstation);
		list.append(str);

		list.append("--");

		str = QString("-- Connection ID:\t%1").arg(connectionID);
		list.append(str);

		str = QString("-- Opto port ID:\t%1").arg(inPort->equipmentID());
		list.append(str);

		str = QString("-- Rx data size:\t%1 words (%2 bytes)").arg(outPort->txDataSizeW()).arg(outPort->txDataSizeW() * sizeof(quint16));
		list.append(str);

		str.sprintf("-- Rx data ID:\t\t%u (0x%08X)", dataID, dataID);
		list.append(str);

		list.append("--\n");

		// declaration section

		list.append("library ieee;");
		list.append("use ieee.std_logic_1164.all;");
		list.append("use ieee.numeric_std.all;\n");

		str = QString("entity %1 is\n\tport (\n").arg(inPortID);
		list.append(str);

		str = QString("\t\tconst_rx_data_id : out std_logic_vector(32-1 downto 0);");
		list.append(str);

		bdfFile.addConnector32("const_rx_data_id");

		str = QString("\t\trx_data_id : out std_logic_vector(32-1 downto 0);\n");
		list.append(str);

		bdfFile.addConnector32("rx_data_id");

		if (txAnalogs.count() > 0)
		{
			for(Hardware::TxRxSignalShared txAnalog :  txAnalogs)
			{
				str = QString("\t\t%1 : out std_logic_vector(%2-1 downto 0);").
						arg(txAnalog->appSignalID().remove("#")).
						arg(txAnalog->dataSize());

				list.append(str);

				bdfFile.addConnector(txAnalog->appSignalID(), txAnalog->dataSize());
			}

			list.append("");
		}

		if (txDiscretes.count() > 0)
		{
			for(Hardware::TxRxSignalShared txDiscrete :  txDiscretes)
			{
				str = QString("\t\t%1 : out std_logic;").arg(txDiscrete->appSignalID().remove("#"));
				list.append(str);

				bdfFile.addConnector1(txDiscrete->appSignalID());
			}

			list.append("");
		}

		str = QString("\t\t%1 : in std_logic_vector(%2-1 downto 0)").arg(outPortID).arg(inBusWidth);
		list.append(str);

		str = QString("\t);\nend %1;\n").arg(inPortID);
		list.append(str);

		// architecture section

		str = QString("architecture arch of %1 is").arg(inPortID);
		list.append(str);

		str = QString("\tsignal in_data : std_logic_vector(%1-1 downto 0);").arg(inBusWidth);
		list.append(str);

		str = QString("begin");
		list.append(str);

		str = QString("\tin_data <= %1;\n").arg(outPortID);
		list.append(str);

		str = QString("\tconst_rx_data_id <= std_logic_vector(to_unsigned(%1,32));").arg(dataID);
		list.append(str);
		str = QString("\trx_data_id <= in_data(32-1 downto 0);\n");
		list.append(str);

		if (txAnalogs.count() > 0)
		{
			for(Hardware::TxRxSignalShared txAnalog :  txAnalogs)
			{
				str = QString("\t%1 <= in_data(%2-1 downto %3);").
						arg(txAnalog->appSignalID().remove("#")).
						arg(txAnalog->addrInBuf().offset() * 16 + txAnalog->dataSize()).
						arg(txAnalog->addrInBuf().offset() * 16);

				list.append(str);
			}

			list.append("");
		}

		if (txDiscretes.count() > 0)
		{
			for(Hardware::TxRxSignalShared txDiscrete :  txDiscretes)
			{
				str = QString("\t%1 <= in_data(%2);").
						arg(txDiscrete->appSignalID().remove("#")).
						arg(txDiscrete->addrInBuf().offset() * 16 + txDiscrete->addrInBuf().bit());
				list.append(str);
			}

			list.append("");
		}

		list.append("end arch;");

		m_resultWriter->addFile("Opto-vhd", vhdFileName, list);

		m_resultWriter->addFile("Opto-vhd", bdfFileName, bdfFile.stringList());

		return true;
	}

	bool ApplicationLogicCompiler::writeOptoModulesReport()
	{
		QVector<Hardware::OptoModuleShared> modules;

		m_optoModuleStorage->getOptoModulesSorted(modules);

		int count = modules.count();

		if (count == 0)
		{
			return true;
		}

		QStringList list;

		QString delim = "--------------------------------------------------------------------";

		QString str;

		for(int i = 0; i < count; i++)
		{
			Hardware::OptoModuleShared module = modules[i];

			if (module == nullptr)
			{
				assert(false);
				continue;
			}

			list.append(delim);

			if (module->isLmOrBvb())
			{
				str = QString(tr("Opto module LM (or BVB) %1")).arg(module->equipmentID());
			}
			else
			{
				if (module->isOcm())
				{
					str = QString(tr("Opto module OCM %1")).arg(module->equipmentID());
				}
				else
				{
					assert(false);
				}
			}

			list.append(str);
			list.append(delim);
			list.append("");

			// write module's opto ports information
			//
			const HashedVector<QString, Hardware::OptoPortShared>& ports = module->ports();

			for(const Hardware::OptoPortShared& port : ports)
			{
				port->writeInfo(list);
			}
		}

		m_resultWriter->addFile("Reports", "opto-modules.txt", "", "", list);

		return true;
	}

	bool ApplicationLogicCompiler::writeAppSignalSetFile()
	{
		if (m_signals == nullptr)
		{
			assert(false);
			return false;
		}

		::Proto::AppSignalSet protoAppSignalSet;

		// fill signals
		//
		int signalCount = m_signals->count();

		for(int i = 0; i < signalCount; i++)
		{
			const Signal& s = (*m_signals)[i];

			::Proto::AppSignal* protoAppSignal = protoAppSignalSet.add_appsignal();

			s.serializeTo(protoAppSignal);
		}

		int dataSize = protoAppSignalSet.ByteSize();

		QByteArray data;

		data.resize(dataSize);

		protoAppSignalSet.SerializeWithCachedSizesToArray(reinterpret_cast<::google::protobuf::uint8*>(data.data()));

		BuildFile* appSignalSetFile = m_resultWriter->addFile("Common", QString("AppSignals.asgs"), CFG_FILE_ID_APP_SIGNAL_SET, "", data, true);

		return appSignalSetFile != nullptr;
	}

	const LmDescriptionSet& ApplicationLogicCompiler::lmDescriptionSet() const
	{
		assert(m_lmDescriptions);
		return *m_lmDescriptions;
	}

	void ApplicationLogicCompiler::clear()
	{
		for(ModuleLogicCompiler* mc : m_moduleCompilers)
		{
			delete mc;
		}

		m_moduleCompilers.clear();

		m_log = nullptr;
	}

}



