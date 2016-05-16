#include "../include/WUtils.h"
#include "TuningService.h"


namespace Tuning
{


	// -------------------------------------------------------------------------------------
	//
	// TuningServiceWorker class implementation
	//
	// -------------------------------------------------------------------------------------

	TuningServiceWorker::TuningServiceWorker(const QString& serviceStrID,
											 const QString& cfgServiceIP1,
											 const QString& cfgServiceIP2,
											 const QString& cfgFileName) :
		ServiceWorker(ServiceType::TuningService, serviceStrID, cfgServiceIP1, cfgServiceIP2),
		m_cfgFileName(cfgFileName),
		m_timer(this)
	{
	}


	TuningServiceWorker::~TuningServiceWorker()
	{
		clear();
	}


	void TuningServiceWorker::clear()
	{
	}


	TuningServiceWorker* TuningServiceWorker::createInstance()
	{
		TuningServiceWorker* worker = new TuningServiceWorker(serviceStrID(), cfgServiceIP1(), cfgServiceIP2(), m_cfgFileName);

		worker->setTuningService(m_tuningService);

		m_tuningService->setTuningServiceWorker(worker);

		return worker;
	}


	bool TuningServiceWorker::loadConfigurationFromFile(const QString& fileName)
	{
		QByteArray cfgXmlData;

		QFile file(fileName);

		if (file.open(QIODevice::ReadOnly) == false)
		{
			return false;
		}

		cfgXmlData = file.readAll();

		XmlReadHelper xml(cfgXmlData);

		bool result = true;

		result &= m_tuningSettings.readFromXml(xml);
		result &= readTuningDataSources(xml);

		m_dataSources.buildIP2DataSourceMap();

		return result;
	}


	void TuningServiceWorker::getTuningDataSourcesInfo(QVector<TuningDataSourceInfo>& info)
	{
		m_dataSources.getTuningDataSourcesInfo(info);
	}



	bool TuningServiceWorker::readTuningDataSources(XmlReadHelper& xml)
	{
		bool result = true;

		m_dataSources.clear();

		result = xml.findElement("TuningLMs");

		if (result == false)
		{
			return false;
		}

		int sourceCount = 0;

		result &= xml.readIntAttribute("Count", &sourceCount);

		for(int i = 0; i < sourceCount; i++)
		{
			result = xml.findElement(DataSource::ELEMENT_DATA_SOURCE);

			if (result == false)
			{
				return false;
			}

			TuningDataSource* ds = new TuningDataSource();

			result &= ds->readFromXml(xml);

			if (result == false)
			{
				delete ds;
				break;
			}

			m_dataSources.insert(ds->lmEquipmentID(), ds);
		}

		return result;
	}


	void TuningServiceWorker::initialize()
	{
		loadConfigurationFromFile(m_cfgFileName);
		allocateSignalsAndStates();

		runTuningSocket();

		if (m_tuningService != nullptr)
		{
			connect(this, &TuningServiceWorker::tuningServiceReady, m_tuningService, &TuningService::tuningServiceReady);
			connect(this, &TuningServiceWorker::signalStateReady, m_tuningService, &TuningService::signalStateReady);
			connect(this, &TuningServiceWorker::tuningDataSourceStateUpdate, m_tuningService, &TuningService::tuningDataSourceStateUpdate);

			connect(m_tuningService, &TuningService::signal_getSignalState, this, &TuningServiceWorker::onGetSignalState);
			connect(m_tuningService, &TuningService::signal_setSignalState, this, &TuningServiceWorker::onSetSignalState);
		}

		connect(&m_timer, &QTimer::timeout, this, &TuningServiceWorker::onTimer);

		m_timer.setInterval(333);
		m_timer.start();

		emit tuningServiceReady();
	}


	void TuningServiceWorker::shutdown()
	{
		m_timer.stop();

		stopTuningSocket();
	}


	void TuningServiceWorker::runTuningSocket()
	{
		m_tuningSocket = new Tuning::TuningSocketWorker(m_tuningSettings.tuningDataIP, m_tuningService);

		connect(m_tuningSocket, &Tuning::TuningSocketWorker::replyReady, this, &TuningServiceWorker::onReplyReady);

		m_tuningSocketThread = new SimpleThread(m_tuningSocket);
		m_tuningSocketThread->start();
	}


	void TuningServiceWorker::stopTuningSocket()
	{
		m_tuningSocketThread->quitAndWait();		// m_tuningSocket delete inside
		delete m_tuningSocketThread;
	}


	void TuningServiceWorker::onSetSignalState(QString appSignalID, double value)
	{
		if (m_tuningSocket == nullptr)
		{
			assert(false);
			return;
		}

		if (m_signal2Source.contains(appSignalID) == false)
		{
			return;
		}

		QString sourceID = m_signal2Source[appSignalID];

		if (m_dataSources.contains(sourceID) == false)
		{
			return;
		}

		TuningDataSource* source = m_dataSources[sourceID];

		if (source == nullptr)
		{
			assert(false);
			return;
		}

		quint16 updateFrameStartAddressW = 0;

		Tuning::SocketRequest sr;

		bool result = source->setSignalState(appSignalID, value, &sr);

		if (result == false)
		{
			return;
		}

		// send request
		//
		//	allready filled inside source->setSignalState(appSignalID, value, &sr):
		//
		//	sr.dataType
		//	sr.startAddressW - offset of frame!
		//	sr.frameData
		//

		sr.lmIP = source->lmAddress32();
		sr.lmPort = source->lmPort();
		sr.lmNumber = source->lmNumber();
		sr.lmSubsystemID = source->lmSubsystemID();
		sr.uniqueID = source->uniqueID();
		sr.numerator = source->numerator();
		sr.operation = Tuning::OperationCode::Write;
		sr.frameSizeW = m_tuningSettings.tuningRomFrameSizeW;
		sr.romSizeW = m_tuningSettings.tuningRomSizeW;

		sr.startAddressW += m_tuningSettings.tuningDataOffsetW;		// !!!

		sr.userRequest = true;

		source->incNumerator();
		source->setWaitReply();
		source->incSentRequestCount();

		requestPreprocessing(sr);

		m_tuningSocket->sendRequest(sr);
	}


	void TuningServiceWorker::allocateSignalsAndStates()
	{
		QVector<TuningDataSourceInfo> info;

		getTuningDataSourcesInfo(info);

		// allocate Signals
		//
		m_appSignals.clear();
		m_signal2Source.clear();

		for(const TuningDataSourceInfo& source : info)
		{
			for(const Signal& signal : source.tuningSignals)
			{
				Signal* appSignal = new Signal(false);

				*appSignal = signal;

				m_appSignals.insert(appSignal->appSignalID(), appSignal);

				if (m_signal2Source.contains(appSignal->appSignalID()))
				{
					assert(false);
					qDebug() << "Duplicate AppSignalID" << appSignal->appSignalID();
				}
				else
				{
					m_signal2Source.insert(appSignal->appSignalID(), source.lmEquipmentID);
				}
			}
		}

		int signalCount = m_appSignals.count();

		// allocate Signal states
		//
		m_appSignalStates.clear();

		m_appSignalStates.setSize(signalCount);

		for(int i = 0; i < signalCount; i++)
		{
			Signal* appSignal = m_appSignals[i];
			AppSignalState* appSignalState = m_appSignalStates[i];

			appSignalState->setSignalParams(i, appSignal);
		}
	}


	void TuningServiceWorker::onTimer()
	{
		sendPeriodicReadRequests();

		testConnections();

		emitTuningDataSourcesStates();
	}


	void TuningServiceWorker::sendPeriodicReadRequests()
	{
		for(TuningDataSource* source : m_dataSources)
		{
			sendPeriodicFrameRequest(source);
		}
	}


	void TuningServiceWorker::testConnections()
	{
		qint64 nowTime = QDateTime::currentMSecsSinceEpoch();

		for(TuningDataSource* source : m_dataSources)
		{
			if (source == nullptr)
			{
				continue;
			}

			source->testConnection(nowTime);
		}
	}


	void TuningServiceWorker::sendPeriodicFrameRequest(TuningDataSource* source)
	{
		if (m_tuningSocket == nullptr)
		{
			assert(false);
			return;
		}

		Tuning::SocketRequest sr;

		sr.lmIP = source->lmAddress32();
		sr.lmPort = source->lmPort();
		sr.lmNumber = source->lmNumber();
		sr.lmSubsystemID = source->lmSubsystemID();
		sr.uniqueID = source->uniqueID();
		sr.numerator = source->numerator();
		sr.operation = Tuning::OperationCode::Read;
		sr.startAddressW = source->frameToRequest() * m_tuningSettings.tuningRomFrameSizeW + m_tuningSettings.tuningDataOffsetW;
		sr.frameSizeW = m_tuningSettings.tuningRomFrameSizeW;
		sr.dataType = Tuning::DataType::Discrete;						//
		sr.romSizeW = m_tuningSettings.tuningRomSizeW;
		sr.userRequest = false;

		source->incNumerator();
		source->setWaitReply();
		source->nextFrameToRequest();
		source->incSentRequestCount();

		requestPreprocessing(sr);

		m_tuningSocket->sendRequest(sr);
	}


	void TuningServiceWorker::onReplyReady()
	{
		if (m_tuningSocket == nullptr)
		{
			assert(false);
			return;
		}

		Tuning::SocketReply sr;

		int count = 0;

		while(count < 10)
		{
			bool result = m_tuningSocket->getReply(&sr);

			if (result == false)
			{
				break;
			}

			replyPreprocessing(sr);

			TuningDataSource* source = m_dataSources.getDataSourceByIP(sr.lmIP);

			if (source == nullptr)
			{
				assert(false);
			}
			else
			{
				sr.frameNo = (sr.fotipHeader.startAddress - m_tuningSettings.tuningDataOffsetW) / m_tuningSettings.tuningRomFrameSizeW;

				source->processReply(sr);

				if (source->frameToRequest() != 0)
				{
					sendPeriodicFrameRequest(source);
				}
			}

			count++;
		}
	}


	void TuningServiceWorker::onGetSignalState(QString appSignalID)
	{
		if (m_signal2Source.contains(appSignalID) == false)
		{
			emit signalStateReady(appSignalID, 0, 0, 0, false);
			return;
		}

		QString sourceID = m_signal2Source[appSignalID];

		if (m_dataSources.contains(sourceID) == false)
		{
			emit signalStateReady(appSignalID, 0, 0, 0, false);
			return;
		}

		TuningDataSource* source = m_dataSources[sourceID];

		if (source == nullptr)
		{
			assert(false);
			emit signalStateReady(appSignalID, 0, 0, 0, false);
			return;
		}

		TuningSignalState tss;

		bool result = source->getSignalState(appSignalID, &tss);

		if (result == false)
		{
			emit signalStateReady(appSignalID, 0, 0, 0, false);
			return;
		}

		emit signalStateReady(appSignalID, tss.currentValue, tss.lowLimit, tss.highLimit, tss.valid);
	}


	void TuningServiceWorker::emitTuningDataSourcesStates()
	{
		for(TuningDataSource* source : m_dataSources)
		{
			if (source == nullptr)
			{
				continue;
			}

			emit tuningDataSourceStateUpdate(source->getState());
		}
	}


	void TuningServiceWorker::requestPreprocessing(Tuning::SocketRequest& sr)
	{
	}


	void TuningServiceWorker::replyPreprocessing(Tuning::SocketReply& sr)
	{
	}


	// -------------------------------------------------------------------------------------
	//
	// TuningService class implementation
	//
	// -------------------------------------------------------------------------------------

	TuningService::TuningService(TuningServiceWorker* worker) :
		Service(worker)
	{
		worker->setTuningService(this);
	}


	void TuningService::getTuningDataSourcesInfo(QVector<TuningDataSourceInfo>& info)
	{
		if (m_tuningServiceWorker == nullptr)
		{
			assert(false);
			return;
		}

		m_tuningServiceWorker->getTuningDataSourcesInfo(info);
	}


	void TuningService::setSignalState(QString appSignalID, double value)
	{
		emit signal_setSignalState(appSignalID, value);
	}

	void TuningService::getSignalState(QString appSignalID)
	{
		emit signal_getSignalState(appSignalID);
	}

}
