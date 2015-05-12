#include "Builder.h"
#include "ApplicationLogicBuilder.h"
#include "ConfigurationBuilder.h"

#include "../../include/DbController.h"
#include "../../include/OutputLog.h"
#include "../../include/DeviceObject.h"

#include "../../VFrame30/LogicScheme.h"
#include "../../VFrame30/VideoItemLink.h"
#include "../../VFrame30/HorzVertLinks.h"

#include "../Builder/ApplicationLogicCompiler.h"
#include <QBuffer>

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
			m_log->writeError(db.lastError(), false, true);
			m_log->writeError(tr("Opening project %1: error").arg(projectName()), true, false);
			return;
		}
		else
		{
			m_log->writeMessage(tr("Opening project %1: ok").arg(projectName()), true);
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
				m_log->writeError(tr("lastChangesetId Error."), true, true);
				break;
			}

			bool isAnyCheckedOut = false;
			ok = db.isAnyCheckedOut(&isAnyCheckedOut);

			if (ok == false)
			{
				m_log->writeError(tr("isAnyCheckedOut Error."), true, true);
				QThread::currentThread()->requestInterruption();
				break;
			}

			if (release() == true && isAnyCheckedOut == true)
			{
				m_log->writeError(tr("There are some checked out objects. Please check in all objects before building release version."), true, true);
				QThread::currentThread()->requestInterruption();
				break;
			}

			//
			// Get Equipment from the database
			//
			m_log->writeMessage("", false);
			m_log->writeMessage(tr("Getting equipment"), true);

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
				m_log->writeError(tr("Error"), true, false);
				QThread::currentThread()->requestInterruption();
				break;
			}
			else
			{
				m_log->writeSuccess(tr("Ok"), true);
			}

			//
			// Expand Devices StrId
			//
			m_log->writeMessage("", false);
			m_log->writeMessage(tr("Expanding devices StrIds"), true);

			expandDeviceStrId(deviceRoot.get());

			m_log->writeSuccess(tr("Ok"), true);

			Hardware::EquipmentSet equipmentSet(deviceRoot);

			//auto aaa = equipmentSet.deviceObject(QString("SYSTEMID1_RACKID2_SIGNAL1"));
			//auto aaa1 = equipmentSet.deviceObjectSharedPointer("SYSTEMID1_RACKID2_SIGNAL1");

			SignalSet signalSet;

			if (loadSignals(&db, &signalSet) == false)
			{
				break;
			}

			m_log->writeMessage("", false);
			m_log->writeMessage(tr("Loading AFB elements"), true);

			AfblSet afblSet;

			if (afblSet.loadFromDatabase(&db, m_log, nullptr) == false)
			{
				break;
			}
			m_log->writeMessage(QString::number(afblSet.items.size()) + tr(" elements loaded."), false);

			//
			// Compile Module configuration
			//
			m_log->writeMessage("", false);
			m_log->writeMessage(tr("Module configurations compilation"), true);

			ok = modulesConfiguration(&db, dynamic_cast<Hardware::DeviceRoot*>(deviceRoot.get()), &signalSet, lastChangesetId, &buildWriter);

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			if (ok == false)
			{
				m_log->writeError(tr("Error"), true, false);
				QThread::currentThread()->requestInterruption();
				break;
			}
			else
			{
				m_log->writeSuccess(tr("Ok"), true);
			}

			//
			// Build application logic
			//
			ApplicationLogicData appLogicData;

			buildApplicationLogic(&db, &appLogicData, lastChangesetId);

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			//
			// Compile application logic
			//
			compileApplicationLogic(dynamic_cast<Hardware::DeviceRoot*>(deviceRoot.get()), &signalSet, &afblSet, &appLogicData, &buildWriter);

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}

			//
			// Compile Data Aquisition Service configuration
			//
			DataFormatList dataFormatInfo;
			UnitList unitInfo;
			db.getDataFormats(&dataFormatInfo, nullptr);
			db.getUnits(&unitInfo, nullptr);

			compileDataAquisitionServiceConfiguration(dynamic_cast<Hardware::DeviceRoot*>(deviceRoot.get()), &signalSet, dataFormatInfo, unitInfo, &buildWriter);

			if (QThread::currentThread()->isInterruptionRequested() == true)
			{
				break;
			}
		}
		while (false);

		buildWriter.finish();

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
			m_log->writeMessage(tr("Getting system %1...").arg(parent->caption()), false);
		}

		std::vector<DbFileInfo> files;

		bool ok = false;

		// Get file list with checked out files,
		// if this is release build, specific copies will be fetched later
		//
		ok = db->getFileList(&files, parent->fileInfo().fileId(), nullptr);

		if (ok == false)
		{
			m_log->writeError(tr("Cannot get equipment file list"), false, true);
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
				m_log->writeError(tr("Cannot get %1 instance.").arg(fi.fileName()), false, true);
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

	void BuildWorkerThread::equipmentWalker(Hardware::DeviceObject* currentDevice, std::function<void(Hardware::DeviceObject* device)> processBeforeChildren, std::function<void(Hardware::DeviceObject* device)> processAfterChildren)
	{
		if (currentDevice == nullptr)
		{
			assert(currentDevice != nullptr);

			QString msg = QString(QObject::tr("%1: DeviceObject null pointer!")).arg(__FUNCTION__);

			m_log->writeError(msg, false, true);

			qDebug() << msg;
			return;
		}

		if (processBeforeChildren != nullptr)
		{
			processBeforeChildren(currentDevice);
		}

		int childrenCount = currentDevice->childrenCount();

		for(int i = 0; i < childrenCount; i++)
		{
			Hardware::DeviceObject* device = currentDevice->child(i);

			equipmentWalker(device, processBeforeChildren, processAfterChildren);
		}

		if (processAfterChildren != nullptr)
		{
			processAfterChildren(currentDevice);
		}
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


	bool BuildWorkerThread::loadSignals(DbController* db, SignalSet* signalSet)
	{
		if (db == nullptr ||
			signalSet == nullptr)
		{
			assert(false);
			return false;
		}

		m_log->writeEmptyLine();

		m_log->writeMessage(tr("Loading application logic signals"), true);

		bool result = db->getSignals(signalSet, nullptr);

		if (result == false)
		{
			m_log->writeError(tr("Error"), true, true);
			return false;
		}

		m_log->writeSuccess(tr("Ok"), true);

		return true;
	}

	bool BuildWorkerThread::modulesConfiguration(DbController* db, Hardware::DeviceRoot* deviceRoot, SignalSet* signalSet, int changesetId, BuildResultWriter* buildWriter)
	{
		if (db == nullptr ||
			deviceRoot == nullptr ||
			signalSet == nullptr ||
			buildWriter == nullptr)
		{
			assert(false);
			return false;
		}

		ConfigurationBuilder cfgBuilder = {db, deviceRoot, signalSet, m_log, changesetId, debug(), projectName(), projectUserName(), buildWriter};

		bool result = cfgBuilder.build();

		return result;

	}

	bool BuildWorkerThread::buildApplicationLogic(DbController* db, ApplicationLogicData* appLogicData, int changesetId)
	{
		if (db == nullptr || appLogicData == nullptr)
		{
			assert(false);
			return false;
		}

		m_log->writeMessage("", false);
		m_log->writeMessage(tr("Application Logic building"), true);

		ApplicationLogicBuilder alBuilder = {db, m_log, appLogicData, changesetId, debug()};

		bool result = alBuilder.build();

		if (result == false)
		{
			m_log->writeError(tr("Error"), true, false);
			QThread::currentThread()->requestInterruption();
		}
		else
		{
			m_log->writeSuccess(tr("Ok"), true);
		}

		return result;
	}


	bool BuildWorkerThread::compileApplicationLogic(Hardware::DeviceObject* equipment, SignalSet* signalSet, AfblSet* afblSet, ApplicationLogicData* appLogicData, BuildResultWriter* buildResultWriter)
	{
		m_log->writeMessage("", false);
		m_log->writeMessage(tr("Application Logic compilation"), true);

		ApplicationLogicCompiler appLogicCompiler(equipment, signalSet, afblSet, appLogicData, buildResultWriter, m_log);

		bool result = appLogicCompiler.run();

		if (result == false)
		{
			m_log->writeError(tr("Application Logic compilation was finished with errors"), true, false);
			QThread::currentThread()->requestInterruption();
		}
		else
		{
			m_log->writeSuccess(tr("Application Logic compilation was succesfully finished"), true);
		}

		return result;
	}

	bool BuildWorkerThread::compileDataAquisitionServiceConfiguration(Hardware::DeviceRoot* deviceRoot, SignalSet* signalSet, DataFormatList &dataFormatInfo, UnitList &unitInfo, BuildResultWriter* buildResultWriter)
	{
		m_log->writeMessage("", false);
		m_log->writeMessage(tr("Data Aquisition Service configuration compilation"), true);

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
				equipmentWriter.writeStartElement(metaObject->className());

				for(int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i)
				{
					const QMetaProperty& property = metaObject->property(i);
					if (property.isValid())
					{
						const char* name = property.name();
						equipmentWriter.writeAttribute(name, currentDevice->property(name).toString());
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

			applicationSignalsWriter.writeStartElement("applicationSignals");
			applicationSignalsWriter.writeAttribute("count", QString::number(signalSet->count()));

			for (int i = 0; i < signalSet->count(); i++)
			{
				Signal& signal = (*signalSet)[i];
				bool hasWrongField = false;
				if (!dataFormatInfo.contains(signal.dataFormat()))
				{
					m_log->writeWarning(QString("Signal %1 has wrong dataFormat field").arg(signal.strID()), true, true);
					hasWrongField = true;
				}
				if (!unitInfo.contains(signal.unitID()))
				{
					m_log->writeWarning(QString("Signal %1 has wrong unitID field").arg(signal.strID()), true, true);
					hasWrongField = true;
				}
				if (!unitInfo.contains(signal.inputUnitID()))
				{
					m_log->writeWarning(QString("Signal %1 has wrong inputUnitID field").arg(signal.strID()), true, true);
					hasWrongField = true;
				}
				if (!unitInfo.contains(signal.outputUnitID()))
				{
					m_log->writeWarning(QString("Signal %1 has wrong outputUnitID field").arg(signal.strID()), true, true);
					hasWrongField = true;
				}
				if (signal.inputSensorID() < 0 || signal.inputSensorID() >= SENSOR_TYPE_COUNT)
				{
					m_log->writeWarning(QString("Signal %1 has wrong inputSensorID field").arg(signal.strID()), true, true);
					hasWrongField = true;
				}
				if (signal.outputSensorID() < 0 || signal.outputSensorID() >= SENSOR_TYPE_COUNT)
				{
					m_log->writeWarning(QString("Signal %1 has wrong outputSensorID field").arg(signal.strID()), true, true);
					hasWrongField = true;
				}
				if (signal.outputRangeMode() < 0 || signal.outputRangeMode() >= OUTPUT_RANGE_MODE_COUNT)
				{
					m_log->writeWarning(QString("Signal %1 has wrong outputRangeMode field").arg(signal.strID()), true, true);
					hasWrongField = true;
				}
				if (signal.inOutType() < 0 || signal.inOutType() >= IN_OUT_TYPE_COUNT)
				{
					m_log->writeWarning(QString("Signal %1 has wrong inOutType field").arg(signal.strID()), true, true);
					hasWrongField = true;
				}
				if (signal.byteOrder() < 0 || signal.byteOrder() >= BYTE_ORDER_COUNT)
				{
					m_log->writeWarning(QString("Signal %1 has wrong byteOrder field").arg(signal.strID()), true, true);
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
				applicationSignalsWriter.writeAttribute("type", signal.type() == SignalType::Analog ? tr("Analog") : tr("Discrete"));
				applicationSignalsWriter.writeAttribute("strID", signal.strID());
				applicationSignalsWriter.writeAttribute("extStrID", signal.extStrID());
				applicationSignalsWriter.writeAttribute("name", signal.name());
				applicationSignalsWriter.writeAttribute("dataFormat", dataFormatInfo.value(signal.dataFormat()));
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
				applicationSignalsWriter.writeAttribute("acquire", signal.acquire() ? tr("true") : tr("false"));
				applicationSignalsWriter.writeAttribute("calculated", signal.calculated() ? tr("true") : tr("false"));
				applicationSignalsWriter.writeAttribute("normalState", QString::number(signal.normalState()));
				applicationSignalsWriter.writeAttribute("decimalPlaces", QString::number(signal.decimalPlaces()));
				applicationSignalsWriter.writeAttribute("aperture", QString::number(signal.aperture()));
				applicationSignalsWriter.writeAttribute("inOutType", InOutTypeStr[signal.inOutType()]);
				applicationSignalsWriter.writeAttribute("deviceStrID", signal.deviceStrID());
				applicationSignalsWriter.writeAttribute("filteringTime", QString::number(signal.filteringTime()));
				applicationSignalsWriter.writeAttribute("maxDifference", QString::number(signal.maxDifference()));
				applicationSignalsWriter.writeAttribute("byteOrder", ByteOrderStr[signal.byteOrder()]);

				applicationSignalsWriter.writeEndElement();	// signal
			}

			applicationSignalsWriter.writeEndElement();	// applicationSignals
			applicationSignalsWriter.writeEndDocument();
			buffer.close();
			buildResultWriter->addFile("DataAquisitionService", "applicationSignals.xml", buffer.buffer());
		}
		else
		{
			m_log->writeMessage(tr("Signals not found!"), true);
		}

		m_log->writeSuccess(tr("Data Aquisition Service configuration compilation was succesfully finished"), true);

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
