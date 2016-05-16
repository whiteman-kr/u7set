#include "TuningBuilder.h"

namespace Builder
{
	// ------------------------------------------------------------------------
	//
	//		TuningBuilder
	//
	// ------------------------------------------------------------------------

	TuningBuilder::TuningBuilder(DbController* db, Hardware::DeviceRoot* deviceRoot, SignalSet* signalSet, Hardware::SubsystemStorage* subsystems, Tuning::TuningDataStorage *tuningDataStorage, IssueLogger *log, int changesetId, bool debug, QString projectName, QString userName, BuildResultWriter* buildWriter):
		m_db(db),
		m_deviceRoot(deviceRoot),
		m_signalSet(signalSet),
		m_subsystems(subsystems),
		m_tuningDataStorage(tuningDataStorage),
		m_log(log),
		m_buildWriter(buildWriter),
		m_changesetId(changesetId),
		m_debug(debug),
		m_projectName(projectName),
		m_userName(userName)
	{
		assert(m_db);
		assert(m_deviceRoot);
		assert(m_signalSet);
		assert(m_subsystems);
		assert(m_tuningDataStorage);
		assert(m_log);
		assert(m_buildWriter);

		return;
	}

	TuningBuilder::~TuningBuilder()
	{
	}

	bool TuningBuilder::build()
	{
		if (db() == nullptr || log() == nullptr)
		{
			assert(db());
			assert(log());
			LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, tr("%1: Fatal error, input parammeter is nullptr!").arg(__FUNCTION__));
			return false;
		}

		Hardware::ModuleFirmwareCollection firmwareCollection(m_projectName, m_userName, changesetId());

		std::vector<Hardware::DeviceModule*> lmModules;
		findLmModules(m_deviceRoot, lmModules);

		QString errorString;

		for (Hardware::DeviceModule* m : lmModules)
		{

			if (m->propertyExists("SubsystemID") == false)
			{
				assert(false);
				LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, QString(tr("No property SubsystemID found in LM %1")).arg(m->caption()));
				return false;
			}

			if (m->propertyExists("LMNumber") == false)
			{
				assert(false);
				LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, QString(tr("No property LMNumber found in LM %1")).arg(m->caption()));
				return false;
			}

			if (m->propertyExists("TuningFrameSize") == false)
			{
				assert(false);
				LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, QString(tr("No property TuningFrameSize found in LM %1")).arg(m->caption()));
				return false;
			}

			if (m->propertyExists("TuningFrameCount") == false)
			{
				assert(false);
				LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, QString(tr("No property TuningFrameCount found in LM %1")).arg(m->caption()));
				return false;
			}

			QString subsysStrID = m->propertyValue("SubsystemID").toString();

			int channel = m->propertyValue("LMNumber").toInt();

			int frameSize = m->propertyValue("TuningFrameSize").toInt();

			int frameCount = m->propertyValue("TuningFrameCount").toInt();

			int subsysID = m_subsystems->ssKey(subsysStrID);

			if (subsysID == -1)
			{
				LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, QString(tr("Undefined subsystem strID %1 assigned in LM %2")).arg(subsysStrID).arg(m->caption()));
				return false;
			}

			Hardware::ModuleFirmware* firmware = (Hardware::ModuleFirmware*)firmwareCollection.jsGet(tr("LM-1"), subsysStrID, subsysID, 0x104, frameSize, frameCount);
			if (firmware == nullptr)
			{
				assert(firmware);
				return false;
			}

			QByteArray data;
			quint64 uniqueID = 0;

			Tuning::TuningDataStorage::iterator it = m_tuningDataStorage->find(m->equipmentId());

			if (it == m_tuningDataStorage->end())
			{
				data.fill(0, 100);
			}
			else
			{
				Tuning::TuningData *tuningData = it.value();
				if (tuningData == nullptr)
				{
					assert(tuningData);
					return false;
				}

				tuningData->getTuningData(&data);
				uniqueID = tuningData->uniqueID();

			}


			if (firmware->setChannelData(channel, frameSize, frameCount, uniqueID, data, &errorString) == false)
			{
				LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, QString(tr("%1, LM %2")).arg(errorString).arg(m->caption()));
				return false;
			}
		}

		std::map<QString, Hardware::ModuleFirmware>& firmwares = firmwareCollection.firmwares();
		for (auto it = firmwares.begin(); it != firmwares.end(); it++)
		{
			Hardware::ModuleFirmware& f = it->second;

			QByteArray data;

			QString errorMsg;
			if (f.save(data, &errorMsg) == false)
			{
				LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, errorMsg);
				return false;
			}

			QString path = f.subsysId();
			QString fileName = f.caption();

			if (path.isEmpty())
			{
				LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, tr("Failed to save module configuration output file, subsystemId is empty."));
				return false;
			}
			if (fileName.isEmpty())
			{
				LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, tr("Failed to save module configuration output file, module type string is empty."));
				return false;
			}

			if (m_buildWriter->addFile(path, fileName + ".tub", data) == false)
			{
				LOG_ERROR_OBSOLETE(m_log, IssuePrexif::NotDefined, tr("Failed to save tuning parameters output file for") + f.subsysId() + ", " + f.caption() + "!");
				return false;
			}

			/*if (m_buildWriter->addFile(path, fileName + ".mct", f.log()) == false)
			{
				LOG_ERROR(m_log, tr("Failed to save module configuration output log file for") + f.subsysId() + ", " + f.caption() + "!");
				return false;
			}*/
		}


		return true;
	}

	void TuningBuilder::findLmModules(Hardware::DeviceObject* object, std::vector<Hardware::DeviceModule *> &modules)
	{
		if (object == nullptr)
		{
			assert(object);
			return;
		}

		for (int i = 0; i < object->childrenCount(); i++)
		{
			Hardware::DeviceObject* child = object->child(i);

			if (child->deviceType() == Hardware::DeviceType::Module)
			{
				Hardware::DeviceModule* module = dynamic_cast<Hardware::DeviceModule*>(child);

				if (module->moduleFamily() == Hardware::DeviceModule::LM)
				{
					modules.push_back(module);
				}
			}

			findLmModules(child, modules);
		}

		return;
	}

	DbController* TuningBuilder::db()
	{
		return m_db;
	}

	OutputLog* TuningBuilder::log() const
	{
		return m_log;
	}

	int TuningBuilder::changesetId() const
	{
		return m_changesetId;
	}

	bool TuningBuilder::debug() const
	{
		return m_debug;
	}

	bool TuningBuilder::release() const
	{
		return !debug();
	}
}
/*
	//
	// Generate Module Confuiguration Binary File
	//
	LOG_MESSAGE(m_log, "");
	LOG_MESSAGE(m_log, tr("Generating modules configurations"));

	bool ok = false;

	// Check if connections' identifiers (non-empty) exist in the database
	//
	for (int i = 0; i < m_connections->count(); i++)
	{
		std::shared_ptr<Hardware::Connection> connection = m_connections->get(i);

		std::vector<Hardware::DeviceObject*> list;

		if (connection->connectionType() == Hardware::Connection::ConnectionType::OpticalConnectionType)
		{
			if (connection->device1StrID().length() > 0)
			{
				list.clear();
				m_deviceRoot->findChildObjectsByMask(connection->device1StrID(), list);
				if (list.empty() == true)
				{
					LOG_ERROR(m_log, tr("No source port %1 was found for optical connection %2").arg(connection->device1StrID()).arg(connection->caption()));
					return false;
				}
			}

			if (connection->device2StrID().length() > 0)
			{
				list.clear();
				m_deviceRoot->findChildObjectsByMask(connection->device2StrID(), list);
				if (list.empty() == true)
				{
					LOG_ERROR(m_log, tr("No target port %1 was found for optical connection %2").arg(connection->device2StrID()).arg(connection->caption()));
					return false;
				}
			}
		}
		else
		{
			m_deviceRoot->findChildObjectsByMask(connection->ocmPortStrID(), list);
			if (list.empty() == true)
			{
				LOG_ERROR(m_log, tr("No port %1 was found for serial connection %2").arg(connection->ocmPortStrID()).arg(connection->caption()));
				return false;
			}
		}


	}


	// Get script file from the project databse
	//
	std::vector<DbFileInfo> fileList;

	if (release() == true)
	{
		assert(false);
	}
	else
	{
		ok = db()->getFileList(&fileList, db()->mcFileId(), "ModulesConfigurations.descr", true, nullptr);
	}

	FILTER filList here for ActionDeleted (have a look at getEquipment)

	if (ok == false || fileList.size() != 1)
	{
		LOG_ERROR(m_log, tr("Can't get file list and find Module Configuration description file"));
		return false;
	}

	std::shared_ptr<DbFile> scriptFile;

	if (release() == true)
	{
		assert(false);
	}
	else
	{
		ok = db()->getLatestVersion(fileList[0], &scriptFile, nullptr);
	}

	if (ok == false || scriptFile == nullptr)
	{
		LOG_ERROR(m_log, tr("Can't get Module Configuration description file"));
		return false;
	}

	QString contents = QString::fromLocal8Bit(scriptFile->data());

	// Attach objects
	//
	QJSEngine jsEngine;

	//qmlRegisterType<MySliderItem>("com.mycompany.qmlcomponents", 1, 0, "Slider");


	JsSignalSet jsSignalSet(m_signalSet);

	Hardware::ModuleFirmwareCollection confCollection(m_projectName, m_userName, m_changesetId);

	QJSValue jsRoot = jsEngine.newQObject(m_deviceRoot);
	QQmlEngine::setObjectOwnership(m_deviceRoot, QQmlEngine::CppOwnership);

	QJSValue jsConfCollection = jsEngine.newQObject(&confCollection);
	QQmlEngine::setObjectOwnership(&confCollection, QQmlEngine::CppOwnership);

	QJSValue jsLog = jsEngine.newQObject(m_log);
	QQmlEngine::setObjectOwnership(m_log, QQmlEngine::CppOwnership);

	QJSValue jsSignalSetObject = jsEngine.newQObject(&jsSignalSet);
	QQmlEngine::setObjectOwnership(&jsSignalSet, QQmlEngine::CppOwnership);

	QJSValue jsSubsystems = jsEngine.newQObject(m_subsystems);
	QQmlEngine::setObjectOwnership(m_subsystems, QQmlEngine::CppOwnership);

	QJSValue jsConnections = jsEngine.newQObject(m_connections);
	QQmlEngine::setObjectOwnership(m_connections, QQmlEngine::CppOwnership);

	// Run script
	//
	QJSValue jsEval = jsEngine.evaluate(contents, "ModulesConfigurations.descr");
	if (jsEval.isError() == true)
	{
		LOG_ERROR(m_log, tr("Module configuration script evaluation failed: %1").arg(jsEval.toString()));
		return false;
	}

	QJSValueList args;

	args << jsRoot;
	args << jsConfCollection;
	args << jsLog;
	args << jsSignalSetObject;
	args << jsSubsystems;
	args << jsConnections;

	QJSValue jsResult = jsEval.call(args);

	if (jsResult.isError() == true)
	{
		LOG_ERROR(m_log, tr("Uncaught exception while generating module configuration: %1").arg(jsResult.toString()));
		return false;
	}

	if (jsResult.toBool() == false)
	{
		LOG_ERROR(m_log, tr("Module configuration generating failed!"));
		return false;
	}
	qDebug() << jsResult.toInt();

	// Find all LM modules and save ssKey and channel information
	//
	std::vector<Hardware::DeviceModule*> lmModules;
	findLmModules(m_deviceRoot, lmModules);

	QStringList lmReport;
	lmReport << "Jumpers configuration for LM modules";

	for (Hardware::DeviceModule* m : lmModules)
	{
		if (m->propertyExists("SubsystemID") == false)
		{
			lmReport << "No SubsystemID property found in " + m->strId();
			assert(false);
			continue;
		}

		if (m->propertyExists("LMNumber") == false)
		{
			lmReport << "No LMNumber property found in " + m->strId();
			assert(false);
			continue;
		}

		int ssKey = m_subsystems->ssKey(m->propertyValue("SubsystemID").toString());
		int channel = m->propertyValue("LMNumber").toInt();

		lmReport << "\r\n";
		lmReport << "StrID: " + m->strId();
		lmReport << "Caption: " + m->caption();
		lmReport << "Place: " + QString::number(m->place());
		lmReport << "Subsystem ID: " + m->propertyValue("SubsystemID").toString();
		lmReport << "Subsystem code: " + QString::number(ssKey);
		lmReport << "Channel: " + QString::number(channel);

		quint16 jumpers = ssKey << 6;
		jumpers |= channel;

		quint16 crc4 = Crc::crc4(jumpers);
		jumpers |= (crc4 << 12);

		lmReport << "Jumpers configuration (HEX): 0x" + QString::number(jumpers, 16);

		QString jumpersHex = QString::number(jumpers, 2).rightJustified(16, '0');
		jumpersHex.insert(4, ' ');
		jumpersHex.insert(9, ' ');
		jumpersHex.insert(14, ' ');
		lmReport << "Jumpers configuration (BIN): " + jumpersHex;
	}

	QByteArray lmReportData;
	for (QString s : lmReport)
	{
		lmReportData.append(s + "\r\n");
	}

	if (m_buildWriter->addFile("Reports", "lmJumpers.txt", lmReportData) == false)
	{
		LOG_ERROR(m_log, tr("Failed to save lmJumpers.txt file!"));
		return false;
	}

	// Save confCollection items to binary files
	//
	if (release() == true)
	{
		assert(false);
	}
	else
	{
		for (auto i = confCollection.firmwares().begin(); i != confCollection.firmwares().end(); i++)
		{
			Hardware::ModuleFirmware& f = i->second;

			QByteArray data;

			QString errorMsg;
			if (f.save(data, &errorMsg) == false)
			{
				LOG_ERROR(m_log, errorMsg);
				return false;
			}

			QString path = f.subsysId();
			QString fileName = f.caption();

			if (path.isEmpty())
			{
				LOG_ERROR(m_log, tr("Failed to save module configuration output file, subsystemId is empty."));
				return false;
			}
			if (fileName.isEmpty())
			{
				LOG_ERROR(m_log, tr("Failed to save module configuration output file, module type string is empty."));
				return false;
			}

			if (m_buildWriter->addFile(path, fileName + ".mcb", data) == false)
			{
				LOG_ERROR(m_log, tr("Failed to save module configuration output file for") + f.subsysId() + ", " + f.caption() + "!");
				return false;
			}

			if (m_buildWriter->addFile(path, fileName + ".mct", f.log()) == false)
			{
				LOG_ERROR(m_log, tr("Failed to save module configuration output log file for") + f.subsysId() + ", " + f.caption() + "!");
				return false;
			}
		}
	}*/

