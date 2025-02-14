#include "ConfigurationBuilder.h"

#include "../UtilsLib/Crc.h"
#include "../UtilsLib/WUtils.h"

#include <HardwareLib/DeviceObject.h>
#include <HardwareLib/ScriptDeviceObject.h>

#include "AppSignalProperties.h"
#include "IssueLogger.h"
#include "SignalSet.h"

namespace Builder
{
	// ------------------------------------------------------------------------
	//
	//		JsBusSignal
	//
	// ------------------------------------------------------------------------

	JsBusSignal::JsBusSignal(QObject* parent, const BusSignal* signal, int offsetW, QString busTypeId) :
		QObject(parent),
		m_signal(signal),
		m_offsetW(offsetW),
		m_busTypeId(std::move(busTypeId))
	{
	}

	QString JsBusSignal::propSignalId() const
	{
		return m_signal->signalID;
	}

	QString JsBusSignal::propCaption() const
	{
		return m_signal->caption;
	}

	QString JsBusSignal::propBusTypeId() const
	{
		return m_busTypeId;
	}

	E::SignalType JsBusSignal::propSignalType() const
	{
		return m_signal->signalType;
	}

	E::AnalogAppSignalFormat JsBusSignal::propAnalogFormat() const
	{
		return m_signal->inOutAnalogFormat;
	}

	E::DataFormat JsBusSignal::propDataFormat() const
	{
		return m_signal->inbusAnalogFormat;
	}

	E::ByteOrder JsBusSignal::propByteOrder() const
	{
		return m_signal->inbusAnalogByteOrder;
	}

	int JsBusSignal::offsetB() const
	{
		const int SHIFT_1_BIT = 1;
		return m_offsetW << SHIFT_1_BIT;
	}

	int JsBusSignal::offsetW() const
	{
		return m_offsetW;
	}

	int JsBusSignal::offsetBits() const
	{
		const int SHIFT_3_BITS = 3;
		return m_offsetW << SHIFT_3_BITS;
	}

	int JsBusSignal::sizeB() const
	{
		return static_cast<int>(ceil(m_signal->inbusSizeBits / 8.0));
	}

	int JsBusSignal::sizeW() const
	{
		return static_cast<int>(ceil(m_signal->inbusSizeBits / 16.0));
	}

	int JsBusSignal::sizeBits() const
	{
		return m_signal->inbusSizeBits;
	}

	// ------------------------------------------------------------------------
	//
	//		JsSignalSet
	//
	// ------------------------------------------------------------------------

	JsSignalSet::JsSignalSet(const SignalSet* signalSet) :
		m_signalSet(signalSet)
	{
		if (m_signalSet == nullptr)
		{
			assert(m_signalSet);
		}
	}

	QObject* JsSignalSet::getSignalByEquipmentID(const QString& equpmentID)
	{
		if (m_signalSet == nullptr)
		{
			assert(m_signalSet);
			return nullptr;
		}

		for (const AppSignal* s : *m_signalSet)
		{
			if (s->equipmentID() == equpmentID)
			{
				AppSignalProperties* sp = new AppSignalProperties(*s);
				return sp;

				// QObject* c = &(*m_signalSet)[i];
				// QJSEngine::setObjectOwnership(c, QJSEngine::ObjectOwnership::CppOwnership);
				// return c;
			}
		}
		return nullptr;
	}

	bool JsSignalSet::busExists(const QString& busTypeId)
	{
		BusShared busShared = m_signalSet->getBus(busTypeId);
		if (busShared == nullptr)
		{
			return false;
		}

		return true;
	}

	QVariantList JsSignalSet::getFlatBusSignalsList(const QString& busTypeId)
	{
		QVariantList busSignals;

		if (parseFlatBusSignals(busTypeId, busSignals, 0) == false)
		{
			return {};
		}

		// Sort signals by offset

		std::sort(busSignals.begin(),
				  busSignals.end(),
				  [](const QVariant& s1, const QVariant& s2)
				  {
					  const JsBusSignal* v1 = s1.value<Builder::JsBusSignal*>();
					  const JsBusSignal* v2 = s2.value<Builder::JsBusSignal*>();

					  if (v1 == nullptr || v2 == nullptr)
					  {
						  Q_ASSERT(v1);
						  Q_ASSERT(v2);
						  return false;
					  }

					  return v1->offsetW() < v2->offsetW();
				  });

		return busSignals;
	}

	bool JsSignalSet::parseFlatBusSignals(const QString& busTypeId, QVariantList& busSignals, int offset)
	{
		BusShared busShared = m_signalSet->getBus(busTypeId);
		if (busShared == nullptr)
		{
			Q_ASSERT(busShared);
			return false;
		}

		for (const BusSignal& bs : busShared->busSignals())
		{
			switch (bs.signalType)
			{
			case E::SignalType::Bus:
				{
					if (parseFlatBusSignals(bs.busTypeID, busSignals, offset + bs.inbusAddr.offset()) == false)
					{
						return false;
					}
				}
				break;
			case E::SignalType::Analog:
			case E::SignalType::Discrete:
				{
					// JsBusSignal has a parent QObject, so it will be deleted automatically
					//
					JsBusSignal* bsResult = new JsBusSignal(this, &bs, offset + bs.inbusAddr.offset(), busShared->busTypeID());
					busSignals.push_back(QVariant::fromValue<JsBusSignal*>(bsResult));
				}
				break;
			default:
				Q_ASSERT(false);
				return false;
			}
		}

		return true;
	}

	// ------------------------------------------------------------------------
	//
	//		ConfigurationBuilder
	//
	// ------------------------------------------------------------------------

	ConfigurationBuilder::ConfigurationBuilder(BuildWorkerThread* buildWorkerThread, Context* context) :
		m_buildResultWriter(context->m_buildResultWriter.get()),
		m_context(context),
		m_buildWorkerThread(buildWorkerThread),
		m_db(&context->m_db),
		m_deviceRoot(context->m_equipmentSet->root().get()),
		m_fscModules(context->m_fscModules),
		m_lmDescriptions(context->m_lmDescriptions.get()),
		m_signalSet(context->m_signalSet.get()),
		m_subsystems(context->m_subsystems.get()),
		m_opticModuleStorage(context->m_opticModuleStorage.get()),
		m_log(context->m_log),
		m_generateExtraDebugInfo(context->generateExtraDebugInfo())
	{
		assert(m_db);
		assert(m_deviceRoot);
		assert(m_signalSet);
		assert(m_subsystems);
		assert(m_opticModuleStorage);
		assert(m_log);
		assert(m_buildResultWriter);

		qRegisterMetaType<E::SignalType>();
		qRegisterMetaType<E::AnalogAppSignalFormat>();
		qRegisterMetaType<Builder::JsBusSignal*>();

		std::sort(m_fscModules.begin(),
				  m_fscModules.end(),
				  [](const Hardware::DeviceModule* a, const Hardware::DeviceModule* b) -> bool
				  {
					  return a->equipmentIdTemplate() < b->equipmentIdTemplate();
				  });
		return;
	}

	ConfigurationBuilder::~ConfigurationBuilder() {}

	bool ConfigurationBuilder::build()
	{
		bool ok = true;
		ok &= buildFSCConfiguration();
		if (ok == true)
		{
			ok &= createVDUConfigurationIDs();
			ok &= createJumpersConfigurationReport();
			ok &= createSubsystemsReport();
		}
		return ok;
	}

	bool ConfigurationBuilder::writeDataFiles()
	{
		TEST_PTR_RETURN_FALSE(m_buildResultWriter);

		QStringList subsystemsList = m_buildResultWriter->firmwareWriter()->subsystems();

		// Save confCollection items to binary files
		//
		for (const auto& ss : subsystemsList)
		{
			const QByteArray& log = m_buildResultWriter->firmwareWriter()->scriptLog(ss);

			if (log.isEmpty() == false)
			{
				if (m_buildResultWriter->addFile(m_buildResultWriter->subsystemDirectory(ss), ss.toLower() + ".mct", log) == nullptr)
				{
					return false;
				}
			}
		}

		if (m_generateExtraDebugInfo == true)
		{
			if (writeExtraDataFiles() == false)
			{
				return false;
			}
		}

		return true;
	}

	bool ConfigurationBuilder::jsIsInterruptRequested()
	{
		if (m_buildWorkerThread == nullptr)
		{
			assert(m_buildWorkerThread);
			return false;
		}

		return m_buildWorkerThread->isInterruptRequested();
	}

	bool ConfigurationBuilder::buildFSCConfiguration()
	{
		if (db() == nullptr || log() == nullptr)
		{
			assert(db());
			assert(log());
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("%1: Fatal error, input parameter is nullptr!").arg(__FUNCTION__));
			return false;
		}

		//
		// Generate Module Configuration Binary File
		//
		LOG_MESSAGE(m_log, "");
		LOG_MESSAGE(m_log, tr("Generating modules configurations"));

		int subsystemsCount = m_subsystems->count();

		// Check if logic modules have unknown subsystems
		//
		for (auto it = m_fscModules.begin(); it != m_fscModules.end(); it++)
		{
			Hardware::DeviceModule* lm = *it;
			if (lm == nullptr)
			{
				assert(lm);
				return false;
			}

			if (lm->propertyExists("SubsystemID") == false)
			{
				m_log->errCFG3000("SubsystemID", lm->equipmentId());
				return false;
			}

			QString subsystemID = lm->propertyValue("SubsystemID").toString();

			bool subsystemFound = false;

			for (int i = 0; i < subsystemsCount; i++)
			{
				std::shared_ptr<Hardware::Subsystem> subsystem = m_subsystems->get(i);
				if (subsystem == nullptr)
				{
					assert(subsystem);
					return false;
				}

				if (subsystem->subsystemId() == subsystemID)
				{
					subsystemFound = true;
					break;
				}
			}

			if (subsystemFound == false)
			{
				m_log->errCFG3001(subsystemID, lm->equipmentId());
				return false;
			}
		}

		// Find Logic modules for each subsystem and execute configuration script for each subsystem
		//
		int errorCount = m_log->errorCount(); // Save log errors count for later comparing

		for (int i = 0; i < subsystemsCount; i++)
		{
			std::shared_ptr<Hardware::Subsystem> subsystem = m_subsystems->get(i);
			if (subsystem == nullptr)
			{
				assert(subsystem);
				return false;
			}

			std::vector<Hardware::DeviceModule*> subsystemModules;

			std::vector<LmDescription*> subsystemModulesDescriptions;

			for (auto it = m_fscModules.begin(); it != m_fscModules.end(); it++)
			{
				Hardware::DeviceModule* lm = *it;
				if (lm == nullptr)
				{
					assert(lm);
					return false;
				}

				if (lm->propertyExists("SubsystemID") == false)
				{
					m_log->errCFG3000("SubsystemID", lm->equipmentId());
					return false;
				}

				if (lm->propertyExists("LmDescriptionFile") == false)
				{
					m_log->errCFG3000("LmDescriptionFile", lm->equipmentId());
					return false;
				}

				if (lm->propertyExists("LMNumber") == false)
				{
					m_log->errCFG3000("LMNumber", lm->equipmentId());
					return false;
				}

				QString subsystemID = lm->propertyValue("SubsystemID").toString();

				if (subsystemID == subsystem->subsystemId())
				{
					// Check for unique LmNumber
					//
					int lmNumber = lm->propertyValue("LMNumber").toInt();

					for (auto slm : subsystemModules)
					{
						int sLmNumber = slm->propertyValue("LMNumber").toInt();

						if (sLmNumber == lmNumber)
						{
							m_log->errCFG3003(lmNumber, lm->equipmentId());
							return false;
						}
					}

					// Add a module for this subsystem
					//
					subsystemModules.push_back(lm);

					LmDescription* description = m_lmDescriptions->get(lm).get();

					if (description == nullptr)
					{
						m_log->errEQP6004(lm->equipmentIdTemplate(), LogicModuleSet::lmDescriptionFile(lm), lm->uuid());
						return false;
					}

					if (std::find(subsystemModulesDescriptions.begin(), subsystemModulesDescriptions.end(), description) ==
						subsystemModulesDescriptions.end())
					{
						subsystemModulesDescriptions.push_back(description);
					}
				}
			}

			if (subsystemModules.empty() == true)
			{
				continue;
			}

			if (subsystemModulesDescriptions.empty() == true)
			{
				LOG_ERROR_OBSOLETE(m_log,
								   IssuePrefix::NotDefined,
								   tr("%1: Fatal error, Logic Modules descriptions for subsystem %2 is undefined!")
									   .arg(__FUNCTION__)
									   .arg(subsystem->caption()));
				return false;
			}

			for (LmDescription* logicModuleDescription : subsystemModulesDescriptions)
			{
				if (logicModuleDescription->flashMemory().m_configWriteBitstream == true)
				{
					QString subsystemStrId = subsystemModules[0]->propertyValue("SubsystemID").toString();
					if (runConfigurationScriptFile(subsystemStrId,
												   m_subsystems->ssKey(subsystemStrId),
												   subsystemModules,
												   logicModuleDescription,
												   m_buildResultWriter->firmwareWriter()) == false)
					{
						return false;
					}
				}
			}
		}

		if (m_log->errorCount() > errorCount)
		{
			// New error messages arrived during build - build failed
			return false;
		}

		return true;
	}

	bool ConfigurationBuilder::createVDUConfigurationIDs() 
	{
		// Generate VDU Configuration IDs to the context
		//
		for (auto it = m_fscModules.begin(); it != m_fscModules.end(); it++)
		{
			Hardware::DeviceModule* lm = *it;
			if (lm == nullptr)
			{
				assert(lm);
				return false;
			}
			if (lm->isVdu() == true)
			{
				if (lm->propertyExists("SubsystemID") == false)
				{
					m_log->errCFG3000("SubsystemID", lm->equipmentId());
					return false;
				}
				QString subsystemID = lm->propertyValue("SubsystemID").toString();

				if (lm->propertyExists("LMNumber") == false)
				{
					m_log->errCFG3000("LMNumber", lm->equipmentId());
					return false;
				}
				int lmNumber = lm->propertyValue("LMNumber").toInt();

				bool ok = false;
				Hardware::ModuleFirmware& fw = m_buildResultWriter->firmwareWriter()->firmware(subsystemID, &ok);
				if (ok == false)
				{
					Q_ASSERT(ok);
					continue;
				}

				LmDescription* description = m_lmDescriptions->get(lm).get();
				if (description == nullptr)
				{
					Q_ASSERT(description);
					continue;
				}

				const int vduConfigUartID = description->flashMemory().configUartId();
				if (fw.uartExists(vduConfigUartID) == false)
				{
					Q_ASSERT(ok);
					continue;
				}

				Hardware::ModuleFirmwareData& data = fw.firmwareData(vduConfigUartID, &ok);
				if (ok == false)
				{
					Q_ASSERT(ok);
					continue;
				}

				int frameIndex =
					description->flashMemory().m_singleConfigFirstFrame + (lmNumber - 1) * description->flashMemory().m_singleConfigFrameCount;

				const quint8* idOffset = data.frames[frameIndex].data() + 14;
				quint64 configID = qFromBigEndian(*reinterpret_cast<const quint64*>(idOffset));
				m_context->m_vduConfigIDs[lm->equipmentId()] = configID;
			}
		}

		return true;
	}

	bool ConfigurationBuilder::createJumpersConfigurationReport()
	{
		// Find all LM modules and save ssKey and channel information
		//

		QStringList lmReport;
		lmReport << "Jumpers configuration for LM modules";

		for (Hardware::DeviceModule* m : m_fscModules)
		{
			if (m->propertyExists("SubsystemID") == false)
			{
				lmReport << "No SubsystemID property found in " + m->equipmentIdTemplate();
				assert(false);
				continue;
			}

			if (m->propertyExists("LMNumber") == false)
			{
				lmReport << "No LMNumber property found in " + m->equipmentIdTemplate();
				assert(false);
				continue;
			}

			if (m->propertyExists("SubsystemChannel") == false)
			{
				lmReport << "No SubsystemChannel property found in " + m->equipmentIdTemplate();
				assert(false);
				continue;
			}

			const QString subsystemId = m->propertyValue("SubsystemID").toString();
			int ssKey = m_subsystems->ssKey(m->propertyValue("SubsystemID").toString());
			int lmNumber = m->propertyValue("LMNumber").toInt();
			int channel = m->propertyValue("SubsystemChannel").toInt();

			lmReport << "\r\n";
			lmReport << "Equipment ID: " + m->equipmentIdTemplate();
			lmReport << "Caption: " + m->caption();
			lmReport << "Place: " + QString::number(m->place());
			lmReport << "Subsystem ID: " + subsystemId;
			lmReport << "Subsystem Code: " + QString::number(ssKey);
			lmReport << "Subsystem Channel: " + E::valueToString<E::Channel>(channel);
			lmReport << "LM Number: " + QString::number(lmNumber);

			quint16 jumpers = jumpersCode(ssKey, lmNumber);

			QString jumpersHex = QString::number(jumpers, 2).rightJustified(16, '0');
			jumpersHex.insert(4, ' ');
			jumpersHex.insert(9, ' ');
			jumpersHex.insert(14, ' ');

			const int MODULE_LM11_VERSION = 0x90;

			if (m->moduleFamily() == static_cast<int>(Hardware::DeviceModule::FamilyType::LM) && m->moduleVersion() == MODULE_LM11_VERSION)
			{
				// LM-11 has rotary switches, so check if subsystem key and channel number have range 0..f and print their positions
				//
				if ((ssKey < 0) || (ssKey > 15))
				{
					m_log->errCFG3060(subsystemId, ssKey, 0, 15);
				}

				lmReport << "Internal Configuration Code (HEX): 0x" + QString::number(jumpers, 16);
				lmReport << "Internal Configuration Code (BIN): " + jumpersHex;

				lmReport << tr("Upper Rotary Switch Value: '%1'").arg(QString::number(lmNumber, 16));
				lmReport << tr("Lower Rotary Switch Value: '%1'").arg(QString::number(ssKey, 16));
			}
			else
			{
				// All other LMs use jumpers
				//
				if ((ssKey < 0) || (ssKey > std::numeric_limits<quint16>::max()))
				{
					m_log->errCFG3060(subsystemId, ssKey, 0, std::numeric_limits<quint16>::max());
				}

				lmReport << "Jumpers Configuration (HEX): 0x" + QString::number(jumpers, 16);
				lmReport << "Jumpers Configuration (BIN): " + jumpersHex;
			}
		}

		QByteArray lmReportData;
		for (const QString& s : lmReport)
		{
			lmReportData.append(s.toUtf8());
			lmReportData.append(QChar::LineFeed);
		}

		if (m_buildResultWriter->addFile("Reports", "LmJumpers.txt", lmReportData) == nullptr)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("Failed to save LmJumpers.txt file!"));
			return false;
		}

		return true;
	}

	bool ConfigurationBuilder::createSubsystemsReport()
	{
		if (m_subsystems == nullptr || log() == nullptr)
		{
			assert(m_subsystems);
			assert(log());
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("%1: Fatal error, input parameter is nullptr!").arg(__FUNCTION__));
			return false;
		}

		QJsonObject jObject;

		int subsystemsCount = m_subsystems->count();
		for (int i = 0; i < subsystemsCount; i++)
		{
			std::shared_ptr<Hardware::Subsystem> subsystem = m_subsystems->get(i);
			if (subsystem == nullptr)
			{
				assert(subsystem);
				return false;
			}

			QJsonObject jSubsystemInfo;

			jSubsystemInfo.insert(QLatin1String("ID"), subsystem->subsystemId());
			jSubsystemInfo.insert(QLatin1String("Key"), subsystem->key());

			QJsonArray jModuleInfoArray;
			// Check if logic modules have unknown subsystems
			//
			for (auto it = m_fscModules.begin(); it != m_fscModules.end(); it++)
			{
				Hardware::DeviceModule* lm = *it;
				if (lm == nullptr)
				{
					assert(lm);
					return false;
				}

				if (lm->propertyExists("SubsystemID") == false)
				{
					m_log->errCFG3000("SubsystemID", lm->equipmentId());
					return false;
				}
				QString subsystemID = lm->propertyValue("SubsystemID").toString();

				if (subsystemID == subsystem->subsystemId())
				{
					if (lm->propertyExists("LMNumber") == false)
					{
						m_log->errCFG3000("LMNumber", lm->equipmentId());
						return false;
					}
					int lmNumber = lm->propertyValue("LMNumber").toInt();

					QJsonObject jModuleInfo;

					jModuleInfo.insert(QLatin1String("EquipmentId"), lm->equipmentId());
					jModuleInfo.insert(QLatin1String("LmNumber"), lmNumber);
					
					quint16 jumpers = jumpersCode(subsystem->key(), lmNumber);

					QString jumpersBin = QString::number(jumpers, 2).rightJustified(16, '0');
					jumpersBin.insert(4, ' ');
					jumpersBin.insert(9, ' ');
					jumpersBin.insert(14, ' ');
				
					jModuleInfo.insert(QLatin1String("Jumpers"), QString::number(jumpers));
					jModuleInfo.insert(QLatin1String("JumpersHex"), "0x" + QString::number(jumpers, 16));
					jModuleInfo.insert(QLatin1String("JumpersBin"), jumpersBin);

					jModuleInfoArray.push_back(jModuleInfo);
				}
			}

			jSubsystemInfo.insert(QLatin1String("Modules"), jModuleInfoArray);

			jObject.insert(QLatin1String("Subsystem"), jSubsystemInfo);

			QByteArray dest = QJsonDocument(jObject).toJson();

			if (m_buildResultWriter->addFile(Directory::SUBSYSTEMS + Separator::DIR + subsystem->subsystemId(),
											 File::SUBSYSTEM_DESC_JSON,
											 dest) == nullptr)
			{
				LOG_ERROR_OBSOLETE(m_log,
								   IssuePrefix::NotDefined,
								   tr("Failed to save %1 file!")
									   .arg(Directory::SUBSYSTEMS + Separator::DIR + subsystem->subsystemId() + Separator::DIR +
											File::SUBSYSTEM_DESC_JSON));
				return false;
			}
		}

		return true;
	}

	quint16 ConfigurationBuilder::jumpersCode(int ssKey, int lmNumber) 
	{
		quint16 jumpers = static_cast<quint16>(ssKey) << 6;
		jumpers |= lmNumber;

		quint16 crc4 = Crc::crc4(jumpers);
		jumpers |= (crc4 << 12);

		return jumpers;
	}
	
	DbController* ConfigurationBuilder::db()
	{
		return m_db;
	}

	IssueLogger* ConfigurationBuilder::log() const
	{
		return m_log;
	}

	bool ConfigurationBuilder::runConfigurationScriptFile(const QString& subsystemID,
														  int subsystemKey,
														  const std::vector<Hardware::DeviceModule*>& subsystemModules,
														  LmDescription* lmDescription,
														  Hardware::ModuleFirmwareWriter* writer)
	{
		if (subsystemModules.empty() == true || lmDescription == nullptr)
		{
			assert(lmDescription);
			assert(subsystemModules.empty() == false);
			return false;
		}

		bool ok = false;

		// Get script file from the project databse
		//
		std::vector<DbFileInfo> fileList;

		ok = db()->getFileList(&fileList, DbDir::ModuleConfigurationDir, lmDescription->configurationStringFile(), true, nullptr);

		if (ok == false || fileList.size() != 1)
		{
			LOG_ERROR_OBSOLETE(m_log,
							   IssuePrefix::NotDefined,
							   tr("Can't get file list and find module configuration description file '%1'")
								   .arg(lmDescription->configurationStringFile()));
			return false;
		}

		std::shared_ptr<DbFile> scriptFile;
		ok = db()->getLatestVersion(fileList[0], &scriptFile, nullptr);

		if (ok == false || scriptFile == nullptr)
		{
			LOG_ERROR_OBSOLETE(m_log,
							   IssuePrefix::NotDefined,
							   tr("Can't get module configuration description file %1").arg(lmDescription->configurationStringFile()));
			return false;
		}

		QString contents = QString::fromLocal8Bit(scriptFile->data());

		// Attach objects
		//

		std::unique_ptr<QJSEngine> jsEngine = std::make_unique<QJSEngine>();

		jsEngine->installExtensions(QJSEngine::ConsoleExtension);

		JsSignalSet jsSignalSet(m_signalSet);

		QJSValue jsBuilder = jsEngine->newQObject(this);
		QJSEngine::setObjectOwnership(this, QJSEngine::CppOwnership);

		QJSValue jsRoot = jsEngine->newQObject(new Hardware::ScriptDeviceObject{m_deviceRoot->sharedPtr()});

		QJSValue jsLogicModules = jsEngine->newArray((int)subsystemModules.size());
		for (int i = 0; i < subsystemModules.size(); i++)
		{
			Hardware::ScriptDeviceModule* m = new Hardware::ScriptDeviceModule{subsystemModules[i]->toModule()};
			QJSValue module = jsEngine->newQObject(m);

			jsLogicModules.setProperty(i, module);
		}

		int frameSize = lmDescription->flashMemory().m_configFramePayload;
		int frameCount = lmDescription->flashMemory().m_configFrameCount;

		int configUartId = lmDescription->flashMemory().m_configUartId;

		writer->createFirmware(subsystemID,
							   subsystemKey,
							   configUartId,
							   "Configuration",
							   frameSize,
							   frameCount,
							   lmDescription->lmDescriptionFile(subsystemModules[0]),
							   lmDescription->descriptionNumber());

		writer->setScriptFirmware(subsystemID, configUartId);

		QJSValue jsFirmware = jsEngine->newQObject(writer);
		QJSEngine::setObjectOwnership(writer, QJSEngine::CppOwnership);

		QJSValue jsLog = jsEngine->newQObject(m_log);
		QJSEngine::setObjectOwnership(m_log, QJSEngine::CppOwnership);

		QJSValue jsSignalSetObject = jsEngine->newQObject(&jsSignalSet);
		QJSEngine::setObjectOwnership(&jsSignalSet, QJSEngine::CppOwnership);

		QJSValue jsSubsystems = jsEngine->newQObject(m_subsystems);
		QJSEngine::setObjectOwnership(m_subsystems, QJSEngine::CppOwnership);

		QJSValue jsOpticModuleStorage = jsEngine->newQObject(m_opticModuleStorage);
		QJSEngine::setObjectOwnership(m_opticModuleStorage, QJSEngine::CppOwnership);

		QJSValue jsLogicModuleDescription = jsEngine->newQObject(lmDescription);
		QJSEngine::setObjectOwnership(lmDescription, QJSEngine::CppOwnership);

		// Run script
		//
		QJSValue jsEval = jsEngine->evaluate(contents);
		if (jsEval.isError() == true)
		{
			LOG_ERROR_OBSOLETE(m_log,
							   IssuePrefix::NotDefined,
							   tr("Module configuration script '%1' evaluation failed at line %2: %3")
								   .arg(lmDescription->configurationStringFile())
								   .arg(jsEval.property("lineNumber").toInt())
								   .arg(jsEval.toString()));
			return false;
		}

		if (!jsEngine->globalObject().hasProperty("main"))
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("Script has no \"main\" function"));
			return false;
		}

		if (!jsEngine->globalObject().property("main").isCallable())
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("\"main\" property of script is not callable"));
			return false;
		}

		QJSValueList args;

		args << jsBuilder;
		args << jsRoot;
		args << jsLogicModules;
		args << jsFirmware;
		args << jsLog;
		args << jsSignalSetObject;
		args << jsSubsystems;
		args << jsOpticModuleStorage;
		args << jsLogicModuleDescription;

		QJSValue jsResult = jsEngine->globalObject().property("main").call(args);

		if (jsResult.isError() == true)
		{
			QString errorMessage = tr("Uncaught exception while generating module configuration '%1': %2, lineNumber: %3, Stack: %4, ")
									   .arg(lmDescription->configurationStringFile())
									   .arg(jsResult.toString())
									   .arg(jsResult.property("lineNumber").toInt())
									   .arg(jsResult.property("stack").toString());

			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, errorMessage);
			return false;
		}

		if (jsResult.toBool() == false)
		{
			return false;
		}

		return true;
	}

	bool ConfigurationBuilder::writeExtraDataFiles()
	{
		// Write equipment configuration to JSON

		QJsonObject jEquipment;

		writeDeviceObjectToJson(m_deviceRoot, jEquipment);

		QByteArray jEquipmentBytes = QJsonDocument(jEquipment).toJson();

		if (m_buildResultWriter->addFile("Reports", "Equipment.json", jEquipmentBytes) == nullptr)
		{
			LOG_ERROR_OBSOLETE(m_log, IssuePrefix::NotDefined, tr("Failed to save Equipment.json file!"));
			return false;
		}

		//

		return true;
	}

	bool ConfigurationBuilder::writeDeviceObjectToJson(const Hardware::DeviceObject* object, QJsonObject& jParent)
	{
		if (object == nullptr)
		{
			Q_ASSERT(object);
			return false;
		}

		QJsonObject jObject;

		// Type

		jObject.insert(QLatin1String("className"), object->metaObject()->className());

		// Properties

		QJsonObject jProperties;

		for (const std::shared_ptr<Property>& sp : object->properties())
		{
			Property* p = sp.get();
			if (p == nullptr)
			{
				Q_ASSERT(p);
				return false;
			}

			if (p->caption() == QLatin1String("ConfigurationScript") || p->caption() == QLatin1String("SpecificProperties") ||
				p->caption() == QLatin1String("SignalSpecificProperties") || p->caption() == QLatin1String("EquipmentIDTemplate"))
			{
				continue;
			}

			QString value = p->value().toString();
			if (value.length() > 512)
			{
				value = tr("<Text, %1 symbols>").arg(value.length());
			}

			jProperties.insert(p->caption(), value);
		}

		if (jProperties.count() != 0)
		{
			jObject.insert(QLatin1String("objectProperties"), jProperties);
		}

		// Children

		QJsonObject jObjects;

		int childCount = object->childrenCount();
		for (int i = 0; i < childCount; i++)
		{
			writeDeviceObjectToJson(object->child(i).get(), jObjects);
		}

		if (jObjects.count() != 0)
		{
			jObject.insert(QLatin1String("objects"), jObjects);
		}

		// Append to parent

		if (object->isRoot())
		{
			jParent.insert(QLatin1String("root"), jObject);
		}
		else
		{
			jParent.insert(object->equipmentId(), jObject);
		}

		return true;
	}


} // namespace Builder
