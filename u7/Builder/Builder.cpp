#include "Builder.h"
#include "ApplicationLogicBuilder.h"
#include "ConfigurationBuilder.h"

#include "../../include/DbController.h"
#include "../../include/OutputLog.h"
#include "../../include/DeviceObject.h"
#include "Subsystem.h"

#include "../../VFrame30/LogicScheme.h"
#include "../../VFrame30/SchemeItemLink.h"
#include "../../VFrame30/HorzVertLinks.h"

#include "../Builder/ApplicationLogicCompiler.h"
#include <QBuffer>
#include <functional>

namespace Builder
{
	// ------------------------------------------------------------------------
	//
	//		BuildWorkerThread
	//
	// ------------------------------------------------------------------------

	void BuildWorkerThread::run()
	{
		QThread::currentThread()->setTerminationEnabled(true);

		bool ok = false;
		QString str;

		// Start logging to output string, this string will be written as file to build output
		//
		m_log->startStrLogging();

		// Create database controller and open project
		//
		DbController db;

		db.disableProgress();

		db.setHost(serverIpAddress());
		db.setPort(serverPort());
		db.setServerUsername(serverUsername());
		db.setServerPassword(serverPassword());

		ok = db.openProject(projectName(), projectUserName(), projectUserPassword(), nullptr);

		if (ok == false)
		{
			LOG_ERROR(m_log, db.lastError());
			LOG_ERROR(m_log, tr("Opening project %1: error").arg(projectName()));
			return;
		}
		else
		{
			LOG_MESSAGE(m_log, tr("Opening project %1: ok").arg(projectName()));
		}

		BuildResultWriter buildWriter;

#pragma message("################################ Load correct ChangesetID")
		buildWriter.start(&db, m_log, release(), 0 /* Load correct ChangesetID */);

		do
		{
			int lastChangesetId = 0;
			ok = db.lastChangesetId(&lastChangesetId);

			if (ok == false)
			{
				LOG_ERROR(m_log, tr("lastChangesetId Error."));
				break;
			}

			bool isAnyCheckedOut = false;
			ok = db.isAnyCheckedOut(&isAnyCheckedOut);

			if (ok == false)
			{
				LOG_ERROR(m_log, tr("isAnyCheckedOut Error."));
				QThread::currentThread()->requestInterruption();
				break;
			}

			if (release() == true && isAnyCheckedOut == true)
			{
				LOG_ERROR(m_log, tr("There are some checked out objects. Please check in all objects before building release version."));
				QThread::currentThread()->requestInterruption();
				break;
			}

			//
			// Get Equipment from the database
			//
			LOG_EMPTY_LINE(m_log);
			LOG_MESSAGE(m_log, tr("Getting equipment"));

			std::shared_ptr<Hardware::DeviceObject> deviceRoot = std::make_shared<Hardware::DeviceRoot>();

			int rootFileId = db.hcFileId();
			deviceRoot->fileInfo().setFileId(rootFileId);

			bool ok = getEquipment(&db, deviceRoot.get());

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			if (ok == false)
			{
				LOG_ERROR(m_log, tr("Error"));
				QThread::currentThread()->requestInterruption();
				break;
			}
			else
			{
				LOG_SUCCESS(m_log, tr("Ok"));
			}

			//
			// Expand Devices StrId
			//
			LOG_EMPTY_LINE(m_log);
			LOG_MESSAGE(m_log, tr("Expanding devices StrIds"));

			expandDeviceStrId(deviceRoot.get());

			LOG_SUCCESS(m_log, tr("Ok"));

			Hardware::EquipmentSet equipmentSet(deviceRoot);

			//
			// Check same Uuids and same StrIds
			//
			LOG_EMPTY_LINE(m_log);
			LOG_MESSAGE(m_log, tr("Checking for same Uuids and StrIds"));

			ok = checkSameUuidAndStrId(deviceRoot.get());

			if (ok == false)
			{
				break;
			}

			LOG_SUCCESS(m_log, tr("Ok"));

			//
			// SignalSet
			//

			//auto aaa = equipmentSet.deviceObject(QString("SYSTEMID1_RACKID2_SIGNAL1"));
			//auto aaa1 = equipmentSet.deviceObjectSharedPointer("SYSTEMID1_RACKID2_SIGNAL1");

			SignalSet signalSet;

			if (loadSignals(&db, &signalSet) == false)
			{
				break;
			}

			//
			// Loading AFB elements
			//
			LOG_EMPTY_LINE(m_log);
			LOG_MESSAGE(m_log, tr("Loading AFB elements"));

			Afb::AfbElementCollection afbCollection;

			ok = loadAfbl(&db, &afbCollection);

			if (ok == false)
			{
				LOG_ERROR(m_log, tr("Error"));
				QThread::currentThread()->requestInterruption();
				break;
			}
			else
			{
				LOG_SUCCESS(m_log, tr("Ok"));
			}

			LOG_MESSAGE(m_log, tr("%1 elements loaded.").arg(afbCollection.elements().size()));

			Hardware::SubsystemStorage subsystems;

			QString errorCode;
			ok = subsystems.load(&db, errorCode);

			if (ok == false)
			{
				LOG_ERROR(m_log, tr("Can't load subsystems file"));
				if (errorCode.isEmpty() == false)
				{
					LOG_ERROR(m_log, errorCode);
				}
			}

			//
			// Compile Module configuration
			//
			LOG_EMPTY_LINE(m_log);
			LOG_MESSAGE(m_log, tr("Module configurations compilation"));

			ok = modulesConfiguration(&db, dynamic_cast<Hardware::DeviceRoot*>(deviceRoot.get()), &signalSet, &subsystems, lastChangesetId, &buildWriter);

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			if (ok == false)
			{
				LOG_ERROR(m_log, tr("Error"));
				QThread::currentThread()->requestInterruption();
				break;
			}
			else
			{
				LOG_SUCCESS(m_log, tr("Ok"));
			}

			//
			// Build application logic
			//
			ApplicationLogicData appLogicData;

			buildApplicationLogic(&db, &appLogicData, &afbCollection, lastChangesetId);

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			//
			// Compile application logic
			//
			compileApplicationLogic(&subsystems, dynamic_cast<Hardware::DeviceRoot*>(deviceRoot.get()), &signalSet, &afbCollection, &appLogicData, &buildWriter);

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			//
			// Compile Data Aquisition Service configuration
			//
			UnitList unitInfo;
			db.getUnits(&unitInfo, nullptr);

			compileDataAquisitionServiceConfiguration(dynamic_cast<Hardware::DeviceRoot*>(deviceRoot.get()), &signalSet, unitInfo, &buildWriter);

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}
		}
		while (false);

		buildWriter.finish();

		db.closeProject(nullptr);

		emit resultReady(QString("Cool, we've done!"));

		return;
	}

	bool BuildWorkerThread::getEquipment(DbController* db, Hardware::DeviceObject* parent)
	{
		assert(db != nullptr);
		assert(db->isProjectOpened() == true);
		assert(parent != nullptr);

		if (QThread::currentThread()->isInterruptionRequested() == true)
		{
			return false;
		}

		if (parent->deviceType() == Hardware::DeviceType::System)
		{
			LOG_MESSAGE(m_log, tr("Getting system %1...").arg(parent->caption()));
		}

		// --
		//
		std::vector<DbFileInfo> files;

		bool ok = false;

		// Get file list with checked out files,
		// if this is release build, specific copies will be fetched later
		//
		ok = db->getFileList(&files, parent->fileInfo().fileId(), true, nullptr);

		if (ok == false)
		{
			LOG_ERROR(m_log, tr("Cannot get equipment file list"));
			return false;
		}

		if (release() == true)
		{
			// filter some files, which are not checkedin?
			assert(false);
		}
		else
		{
		}

		parent->deleteAllChildren();

		for (auto& fi : files)
		{
			std::shared_ptr<DbFile> file;

			if (release() == true)
			{
				assert(false);
			}
			else
			{
				ok = db->getLatestVersion(fi, &file, nullptr);
			}

			if (file == nullptr || ok == false)
			{
				LOG_ERROR(m_log, tr("Cannot get %1 instance.").arg(fi.fileName()));
				return false;
			}

			Hardware::DeviceObject* object = Hardware::DeviceObject::Create(file->data());

			if (object == nullptr)
			{
				return false;
			}
			else
			{
				assert(object);
			}

			object->setFileInfo(fi);

			std::shared_ptr<Hardware::DeviceObject> sp(object);

			parent->addChild(sp);
		}

		files.clear();

		for (int i = 0 ; i < parent->childrenCount(); i++)
		{
			std::shared_ptr<Hardware::DeviceObject> child = parent->childSharedPtr(i);

			ok = getEquipment(db, child.get());

			if (ok == false)
			{
				return false;
			}
		}

		return true;
	}

	bool BuildWorkerThread::expandDeviceStrId(Hardware::DeviceObject* device)
	{
		if (device == nullptr)
		{
			assert(device != nullptr);
			return false;
		}

		device->expandStrId();

		return true;
	}

	bool BuildWorkerThread::checkSameUuidAndStrId(Hardware::DeviceObject* root)
	{
		if (root == nullptr)
		{
			assert(root);
			return false;
		}

		std::map<QUuid, Hardware::DeviceObject*> uuidMap;
		std::map<QString, Hardware::DeviceObject*> strIdMap;

		// Recursive function
		//

		bool ok = checkSameUuidAndStrIdWorker(root, uuidMap, strIdMap);

		return ok;
	}

	bool BuildWorkerThread::checkSameUuidAndStrIdWorker(Hardware::DeviceObject* device,
									 std::map<QUuid, Hardware::DeviceObject*>& uuidMap,
									 std::map<QString, Hardware::DeviceObject*>& strIdMap)
	{
		if (device == nullptr)
		{
			assert(device);
			return false;
		}

		auto foundSameUuid = uuidMap.find(device->uuid());
		auto foundSameStrId = strIdMap.find(device->strId());

		bool ok = true;

		if (foundSameUuid != uuidMap.end())
		{
			LOG_ERROR(m_log, tr("There are DeviceObjects with the same Uuid %1, StrID1: %2, StrID2: %3")
					  .arg(device->uuid().toString())
					  .arg(device->strId())
					  .arg(foundSameUuid->second->strId()));

			ok = false;
		}
		else
		{
			uuidMap[device->uuid()] = device;
		}

		if (foundSameStrId != strIdMap.end())
		{
			LOG_ERROR(m_log, tr("There are DeviceObjects with the same StrID %1, Parent1: %2, Parent2: %3")
					  .arg(device->strId())
					  .arg(device->parent()->strId())
					  .arg(foundSameStrId->second->parent()->strId()));

			ok = false;
		}
		else
		{
			strIdMap[device->strId()] = device;
		}

		int childCount = device->childrenCount();

		for (int i = 0; i < childCount; i++)
		{
			ok &= checkSameUuidAndStrIdWorker(device->child(i), uuidMap, strIdMap);
		}

		return ok;
	}


	bool BuildWorkerThread::loadSignals(DbController* db, SignalSet* signalSet)
	{
		if (db == nullptr ||
			signalSet == nullptr)
		{
			assert(false);
			return false;
		}

		LOG_EMPTY_LINE(m_log);

		LOG_MESSAGE(m_log, tr("Loading application logic signals"));

		bool result = db->getSignals(signalSet, nullptr);

		if (result == false)
		{
			LOG_ERROR(m_log, tr("Error"));
			return false;
		}

		LOG_SUCCESS(m_log, tr("Ok"));

		return true;
	}

	bool BuildWorkerThread::loadAfbl(DbController* db, Afb::AfbElementCollection* afbCollection)
	{
		if (db == nullptr ||
			afbCollection == nullptr)
		{
			assert(db);
			assert(afbCollection);
			return false;
		}

		bool result = true;

		// Get file list from the DB
		//
		std::vector<DbFileInfo> files;

		if (db->getFileList(&files, db->afblFileId(), "afb", true, nullptr) == false)
		{
			LOG_ERROR(m_log, QObject::tr("Cannot get application functional block file list."));
			return false;
		}

		// Get files from the DB
		//
		std::vector<std::shared_ptr<Afb::AfbElement>> afbs;
		afbs.reserve(files.size());

		for (DbFileInfo& fi : files)
		{
			std::shared_ptr<DbFile> f;

			if (db->getLatestVersion(fi, &f, nullptr) == false)
			{
				LOG_ERROR(m_log, QObject::tr("Getting the latest version of the file %1 failed.").arg(fi.fileName()));
				result = false;
				continue;
			}

			std::shared_ptr<Afb::AfbElement> e = std::make_shared<Afb::AfbElement>();

			QXmlStreamReader reader(f->data());

			if (e->loadFromXml(&reader) == false)
			{
				LOG_ERROR(m_log, QObject::tr("Reading contents of the file %1 failed.").arg(fi.fileName()));

				if (reader.errorString().isEmpty() == false)
				{
					LOG_ERROR(m_log, "XML error: " + reader.errorString());
				}

				result = false;
				continue;
			}

			afbs.push_back(e);
		}

		afbCollection->setElements(afbs);

		return result;
	}

	bool BuildWorkerThread::modulesConfiguration(DbController* db, Hardware::DeviceRoot* deviceRoot, SignalSet* signalSet, Hardware::SubsystemStorage *subsystems, int changesetId, BuildResultWriter* buildWriter)
	{
		if (db == nullptr ||
			deviceRoot == nullptr ||
			signalSet == nullptr ||
			subsystems == nullptr ||
			buildWriter == nullptr)
		{
			assert(false);
			return false;
		}

		ConfigurationBuilder cfgBuilder = {db, deviceRoot, signalSet, subsystems, m_log, changesetId, debug(), projectName(), projectUserName(), buildWriter};

		bool result = cfgBuilder.build();

		return result;

	}

	bool BuildWorkerThread::buildApplicationLogic(DbController* db,
												  ApplicationLogicData* appLogicData,
												  Afb::AfbElementCollection* afbCollection,
												  int changesetId)
	{
		if (db == nullptr ||
			appLogicData == nullptr ||
			afbCollection == nullptr)
		{
			assert(db);
			assert(appLogicData);
			assert(afbCollection);
			return false;
		}

		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, tr("Application Logic building"));

		ApplicationLogicBuilder alBuilder = {db, m_log, appLogicData, afbCollection, changesetId, debug()};

		bool result = alBuilder.build();

		if (result == false)
		{
			//LOG_ERROR(m_log, tr("Error"));	// Error must be logged and described where it was found
			QThread::currentThread()->requestInterruption();
		}
		else
		{
			LOG_SUCCESS(m_log, tr("Ok"));
		}

		return result;
	}


	bool BuildWorkerThread::compileApplicationLogic(Hardware::SubsystemStorage* subsystems,
													Hardware::DeviceObject* equipment,
													SignalSet* signalSet,
													Afb::AfbElementCollection* afbCollection,
													ApplicationLogicData* appLogicData,
													BuildResultWriter* buildResultWriter)
	{
		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, tr("Application Logic compilation"));

		ApplicationLogicCompiler appLogicCompiler(subsystems, equipment, signalSet, afbCollection, appLogicData, buildResultWriter, m_log);

		bool result = appLogicCompiler.run();

		LOG_EMPTY_LINE(m_log);

		if (result == false)
		{
			LOG_MESSAGE(m_log, tr("Application Logic compilation was finished with errors"));
			QThread::currentThread()->requestInterruption();
		}
		else
		{
			LOG_SUCCESS(m_log, tr("Application Logic compilation was succesfully finished"));
		}

		return result;
	}

	bool BuildWorkerThread::compileDataAquisitionServiceConfiguration(Hardware::DeviceRoot* deviceRoot, SignalSet* signalSet, UnitList &unitInfo, BuildResultWriter* buildResultWriter)
	{
		DataFormatList dataFormatInfo;
		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, tr("Data Aquisition Service configuration compilation"));

		if (deviceRoot != nullptr)
		{
			QXmlStreamWriter equipmentWriter;
			QBuffer buffer;
			buffer.open(QIODevice::WriteOnly);
			equipmentWriter.setDevice(&buffer);
			equipmentWriter.setAutoFormatting(true);
			equipmentWriter.writeStartDocument();

			equipmentWalker(deviceRoot, [&equipmentWriter](Hardware::DeviceObject* currentDevice)
			{
				if (currentDevice == nullptr)
				{
					return;
				}
				const QMetaObject* metaObject = currentDevice->metaObject();
				QString name = metaObject->className();
				int position = name.lastIndexOf(QChar(':'));
				if (position == -1)
				{
					equipmentWriter.writeStartElement(name);
				}
				else
				{
					equipmentWriter.writeStartElement(name.mid(position + 1));
				}

				const std::string& className = metaObject->className();
				equipmentWriter.writeAttribute("classNameHash", QString::number(CUtils::GetClassHashCode(className), 16));

				equipmentWriter.writeAttribute("StrID", currentDevice->strId());
				equipmentWriter.writeAttribute("Caption", currentDevice->caption());
				equipmentWriter.writeAttribute("ChildRestriction", currentDevice->childRestriction());
				equipmentWriter.writeAttribute("Place", QString::number(currentDevice->place()));
				equipmentWriter.writeAttribute("ChildrenCount", QString::number(currentDevice->childrenCount()));
				equipmentWriter.writeAttribute("DynamicProperties", currentDevice->dynamicProperties());

				for(int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i)
				{
					const QMetaProperty& property = metaObject->property(i);
					if (property.isValid())
					{
						const char* name = property.name();
						QVariant tmp = currentDevice->property(name);
						assert(tmp.convert(QMetaType::QString));
						equipmentWriter.writeAttribute(name, tmp.toString());
					}
				}
			}, [&equipmentWriter](Hardware::DeviceObject*)
			{
				equipmentWriter.writeEndElement();
			});

			equipmentWriter.writeEndDocument();
			buffer.close();
			buildResultWriter->addFile("DataAquisitionService", "equipment.xml", buffer.buffer());
		}

		if (signalSet->count() > 0)
		{
			QXmlStreamWriter applicationSignalsWriter;
			QBuffer buffer;
			buffer.open(QIODevice::WriteOnly);
			applicationSignalsWriter.setDevice(&buffer);
			applicationSignalsWriter.setAutoFormatting(true);
			applicationSignalsWriter.writeStartDocument();
			applicationSignalsWriter.writeStartElement("configuration");

			// Writing units
			applicationSignalsWriter.writeStartElement("units");
			applicationSignalsWriter.writeAttribute("count", QString::number(unitInfo.count()));

			for (int i = 0; i < unitInfo.count(); i++)
			{
				applicationSignalsWriter.writeStartElement("unit");

				applicationSignalsWriter.writeAttribute("ID", QString::number(unitInfo.key(i)));
				applicationSignalsWriter.writeAttribute("name", unitInfo[i]);

				applicationSignalsWriter.writeEndElement();
			}

			applicationSignalsWriter.writeEndElement();

			// Writing signals
			applicationSignalsWriter.writeStartElement("applicationSignals");

			for (int i = 0; i < signalSet->count(); i++)
			{
				Signal& signal = (*signalSet)[i];
				bool hasWrongField = false;
				if (!dataFormatInfo.contains(signal.dataFormatInt()))
				{
					LOG_WARNING(m_log, QString("Signal %1 has wrong dataFormat field").arg(signal.strID()));
					hasWrongField = true;
				}
				if (!unitInfo.contains(signal.unitID()))
				{
					LOG_WARNING(m_log, QString("Signal %1 has wrong unitID field").arg(signal.strID()));
					hasWrongField = true;
				}
				if (!unitInfo.contains(signal.inputUnitID()))
				{
					LOG_WARNING(m_log, QString("Signal %1 has wrong inputUnitID field").arg(signal.strID()));
					hasWrongField = true;
				}
				if (!unitInfo.contains(signal.outputUnitID()))
				{
					LOG_WARNING(m_log, QString("Signal %1 has wrong outputUnitID field").arg(signal.strID()));
					hasWrongField = true;
				}
				if (signal.inputSensorID() < 0 || signal.inputSensorID() >= SENSOR_TYPE_COUNT)
				{
					LOG_WARNING(m_log, QString("Signal %1 has wrong inputSensorID field").arg(signal.strID()));
					hasWrongField = true;
				}
				if (signal.outputSensorID() < 0 || signal.outputSensorID() >= SENSOR_TYPE_COUNT)
				{
					LOG_WARNING(m_log, QString("Signal %1 has wrong outputSensorID field").arg(signal.strID()));
					hasWrongField = true;
				}
				if (signal.outputRangeMode() < 0 || signal.outputRangeMode() >= OUTPUT_RANGE_MODE_COUNT)
				{
					LOG_WARNING(m_log, QString("Signal %1 has wrong outputRangeMode field").arg(signal.strID()));
					hasWrongField = true;
				}
				if (signal.inOutType() < 0 || signal.inOutType() >= IN_OUT_TYPE_COUNT)
				{
					LOG_WARNING(m_log, QString("Signal %1 has wrong inOutType field").arg(signal.strID()));
					hasWrongField = true;
				}

				switch (static_cast<E::ByteOrder>(signal.byteOrderInt()))
				{
					case E::ByteOrder::LittleEndian:
					case E::ByteOrder::BigEndian:
						break;
					default:
						LOG_WARNING(m_log, QString("Signal %1 has wrong byteOrder field").arg(signal.strID()));
						hasWrongField = true;
				}


				if (hasWrongField)
				{
					continue;
				}

				applicationSignalsWriter.writeStartElement("signal");

				applicationSignalsWriter.writeAttribute("ID", QString::number(signal.ID()));
				applicationSignalsWriter.writeAttribute("signalGroupID", QString::number(signal.signalGroupID()));
				applicationSignalsWriter.writeAttribute("signalInstanceID", QString::number(signal.signalInstanceID()));
				applicationSignalsWriter.writeAttribute("channel", QString::number(signal.channel()));
				applicationSignalsWriter.writeAttribute("type", signal.type() == E::SignalType::Analog ? "Analog" : "Discrete");
				applicationSignalsWriter.writeAttribute("strID", signal.strID());
				applicationSignalsWriter.writeAttribute("extStrID", signal.extStrID());
				applicationSignalsWriter.writeAttribute("name", signal.name());
				applicationSignalsWriter.writeAttribute("dataFormat", dataFormatInfo.value(signal.dataFormatInt()));
				applicationSignalsWriter.writeAttribute("dataSize", QString::number(signal.dataSize()));
				applicationSignalsWriter.writeAttribute("lowADC", QString::number(signal.lowADC()));
				applicationSignalsWriter.writeAttribute("highADC", QString::number(signal.highADC()));
				applicationSignalsWriter.writeAttribute("lowLimit", QString::number(signal.lowLimit()));
				applicationSignalsWriter.writeAttribute("highLimit", QString::number(signal.highLimit()));
				applicationSignalsWriter.writeAttribute("unitID", unitInfo.value(signal.unitID()));
				applicationSignalsWriter.writeAttribute("adjustment", QString::number(signal.adjustment()));
				applicationSignalsWriter.writeAttribute("dropLimit", QString::number(signal.dropLimit()));
				applicationSignalsWriter.writeAttribute("excessLimit", QString::number(signal.excessLimit()));
				applicationSignalsWriter.writeAttribute("unbalanceLimit", QString::number(signal.unbalanceLimit()));
				applicationSignalsWriter.writeAttribute("inputLowLimit", QString::number(signal.inputLowLimit()));
				applicationSignalsWriter.writeAttribute("inputHighLimit", QString::number(signal.inputHighLimit()));
				applicationSignalsWriter.writeAttribute("inputUnitID", unitInfo.value(signal.inputUnitID()));
				applicationSignalsWriter.writeAttribute("inputSensorID", SensorTypeStr[signal.inputSensorID()]);
				applicationSignalsWriter.writeAttribute("outputLowLimit", QString::number(signal.outputLowLimit()));
				applicationSignalsWriter.writeAttribute("outputHighLimit", QString::number(signal.outputHighLimit()));
				applicationSignalsWriter.writeAttribute("outputUnitID", unitInfo.value(signal.outputUnitID()));
				applicationSignalsWriter.writeAttribute("outputRangeMode", OutputRangeModeStr[signal.outputRangeMode()]);
				applicationSignalsWriter.writeAttribute("outputSensorID", SensorTypeStr[signal.outputSensorID()]);
				applicationSignalsWriter.writeAttribute("acquire", signal.acquire() ? "true" : "false");
				applicationSignalsWriter.writeAttribute("calculated", signal.calculated() ? "true" : "false");
				applicationSignalsWriter.writeAttribute("normalState", QString::number(signal.normalState()));
				applicationSignalsWriter.writeAttribute("decimalPlaces", QString::number(signal.decimalPlaces()));
				applicationSignalsWriter.writeAttribute("aperture", QString::number(signal.aperture()));
				applicationSignalsWriter.writeAttribute("inOutType", InOutTypeStr[signal.inOutType()]);
				applicationSignalsWriter.writeAttribute("deviceStrID", signal.deviceStrID());
				applicationSignalsWriter.writeAttribute("filteringTime", QString::number(signal.filteringTime()));
				applicationSignalsWriter.writeAttribute("maxDifference", QString::number(signal.maxDifference()));
				applicationSignalsWriter.writeAttribute("byteOrder", ByteOrderStr[signal.byteOrderInt()]);
				applicationSignalsWriter.writeAttribute("ramAddr", signal.ramAddr().toString());
				applicationSignalsWriter.writeAttribute("regAddr", signal.regAddr().toString());

				applicationSignalsWriter.writeEndElement();	// signal
			}

			applicationSignalsWriter.writeEndElement();	// applicationSignals
			applicationSignalsWriter.writeEndElement();	// configuration
			applicationSignalsWriter.writeEndDocument();
			buffer.close();
			buildResultWriter->addFile("DataAquisitionService", "applicationSignals.xml", buffer.buffer());
		}
		else
		{
			LOG_MESSAGE(m_log, tr("Signals not found!"));
		}

		LOG_SUCCESS(m_log, tr("Data Aquisition Service configuration compilation was succesfully finished"));

		return true;
	}


	QString BuildWorkerThread::projectName() const
	{
		QMutexLocker m(&m_mutex);
		return m_projectName;
	}

	void BuildWorkerThread::setProjectName(const QString& value)
	{
		QMutexLocker m(&m_mutex);
		m_projectName = value;
	}

	QString BuildWorkerThread::serverIpAddress() const
	{
		QMutexLocker m(&m_mutex);
		return m_serverIpAddress;
	}

	void BuildWorkerThread::setServerIpAddress(const QString& value)
	{
		QMutexLocker m(&m_mutex);
		m_serverIpAddress = value;
	}

	int BuildWorkerThread::serverPort() const
	{
		QMutexLocker m(&m_mutex);
		return m_serverPort;
	}

	void BuildWorkerThread::setServerPort(int value)
	{
		QMutexLocker m(&m_mutex);
		m_serverPort = value;
	}

	QString BuildWorkerThread::serverUsername() const
	{
		QMutexLocker m(&m_mutex);
		return m_serverUsername;
	}

	void BuildWorkerThread::setServerUsername(const QString& value)
	{
		QMutexLocker m(&m_mutex);
		m_serverUsername = value;
	}

	QString BuildWorkerThread::serverPassword() const
	{
		QMutexLocker m(&m_mutex);
		return m_serverPassword;
	}

	void BuildWorkerThread::setServerPassword(const QString& value)
	{
		QMutexLocker m(&m_mutex);
		m_serverPassword = value;
	}

	void BuildWorkerThread::setOutputLog(OutputLog* value)
	{
		QMutexLocker m(&m_mutex);
		m_log = value;
	}

	QString BuildWorkerThread::projectUserName() const
	{
		QMutexLocker m(&m_mutex);
		return m_projectUserName;
	}

	void BuildWorkerThread::setProjectUserName(const QString& value)
	{
		QMutexLocker m(&m_mutex);
		m_projectUserName = value;
	}

	QString BuildWorkerThread::projectUserPassword() const
	{
		QMutexLocker m(&m_mutex);
		return m_projectUserPassword;
	}

	void BuildWorkerThread::setProjectUserPassword(const QString& value)
	{
		QMutexLocker m(&m_mutex);
		m_projectUserPassword = value;
	}

	bool BuildWorkerThread::debug() const
	{
		return m_debug;
	}

	void BuildWorkerThread::setDebug(bool value)
	{
		m_debug = value;
	}

	bool BuildWorkerThread::release() const
	{
		return !m_debug;
	}

	// ------------------------------------------------------------------------
	//
	//		Builder
	//
	// ------------------------------------------------------------------------


	Builder::Builder(OutputLog* log) :
		m_log(log)
	{
		assert(m_log != nullptr);

		m_thread = new BuildWorkerThread();
		m_thread->setObjectName(tr("BuildWorkerThread"));
		m_thread->setOutputLog(m_log);

		connect(m_thread, &BuildWorkerThread::resultReady, this, &Builder::handleResults);

		connect(m_thread, &BuildWorkerThread::started, this, &Builder::buildStarted);
		connect(m_thread, &BuildWorkerThread::finished, this, &Builder::buildFinished);

		return;
	}

	Builder::~Builder()
	{
		m_thread->requestInterruption();

		bool result = m_thread->wait(10000);		// Wait for 10 sec.

		if (result == false)
		{
			qDebug() << "Building thread was not finished.";
			m_thread->terminate();
		}

		delete m_thread;
		return;
	}

	bool Builder::start(QString projectName,
							   QString ipAddress,
							   int port,
							   QString serverUserName,
							   QString serverPassword,
							   QString projectUserName,
							   QString projectUserPassword,
							   bool debug)
	{
		assert(m_thread != nullptr);

		if (isRunning() == true)
		{
			assert(isRunning() == false);
			m_thread->wait(10000);
		}

		// Set params
		//

		m_thread->setProjectName(projectName);
		m_thread->setServerIpAddress(ipAddress);
		m_thread->setServerPort(port);
		m_thread->setServerUsername(serverUserName);
		m_thread->setServerPassword(serverPassword);
		m_thread->setProjectUserName(projectUserName);
		m_thread->setProjectUserPassword(projectUserPassword);
		m_thread->setDebug(debug);

		// Ready? Go!
		//
		m_thread->start();

		return true;
	}

	void Builder::stop()
	{
		m_thread->requestInterruption();
	}

	bool Builder::isRunning() const
	{
		bool result = m_thread->isRunning();
		return result;
	}

	void Builder::handleResults(QString /*result*/)
	{
	}

}
