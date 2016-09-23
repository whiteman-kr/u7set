#include "TuningIPENService.h"
#include "../lib/WUtils.h"

namespace TuningIPEN
{

	// -------------------------------------------------------------------------------------
	//
	// TuningIPENServiceWorker class implementation
	//
	// -------------------------------------------------------------------------------------


	TuningIPENServiceWorker::TuningIPENServiceWorker(const QString& serviceStrID,
														const QString& buildPath) :
		ServiceWorker(ServiceType::TuningService, serviceStrID, "", "", buildPath),
		m_timer(this)
	{
	}


	TuningIPENServiceWorker::~TuningIPENServiceWorker()
	{
		clear();
	}


	void TuningIPENServiceWorker::clear()
	{
	}


	TuningIPENServiceWorker* TuningIPENServiceWorker::createInstance()
	{
		TuningIPENServiceWorker* worker = new TuningIPENServiceWorker(serviceEquipmentID(), buildPath());

		worker->setTuningService(m_tuningIPENService);

		m_tuningIPENService->setTuningServiceWorker(worker);

		return worker;
	}


	void TuningIPENServiceWorker::requestPreprocessing(Tuning::SocketRequest& sr)
	{
		if (sr.operation != Tuning::OperationCode::Write)
		{
			return;
		}

		sr.dataType = Tuning::DataType::Discrete;			// turn off limits control!

		quint16* ptr = reinterpret_cast<quint16*>(sr.fotipData);

		for(int i = 0; i < sizeof(sr.fotipData) / sizeof(quint16); i++)
		{
			*ptr = reverseBytes<quint16>(*ptr);

			ptr++;
		}
	}


	void TuningIPENServiceWorker::replyPreprocessing(Tuning::SocketReply& sr)
	{
		quint16* ptr = reinterpret_cast<quint16*>(sr.fotipData);

		for(int i = 0; i < sizeof(sr.fotipData) / sizeof(quint16); i++)
		{
			*ptr = reverseBytes<quint16>(*ptr);

			ptr++;
		}
	}


	bool TuningIPENServiceWorker::loadConfigurationFromFile(const QString& fileName)
	{
		QString str;

		QByteArray cfgXmlData;

		QFile file(fileName);

		if (file.open(QIODevice::ReadOnly) == false)
		{
			str = QString("Error open configuration file: %1").arg(fileName);

			qDebug() << C_STR(str);

			return false;
		}

		bool result = true;

		cfgXmlData = file.readAll();

		XmlReadHelper xml(cfgXmlData);

		result &= m_tuningSettings.readFromXml(xml);
		result &= readTuningDataSources(xml);

		m_dataSources.buildIP2DataSourceMap();

		if  (result == true)
		{
			str = QString("Configuration is loaded from file: %1").arg(fileName);
		}
		else
		{
			str = QString("Loading configuration error from file: %1").arg(fileName);
		}

		qDebug() << C_STR(str);

		return result;
	}


	void TuningIPENServiceWorker::getTuningDataSourcesInfo(QVector<TuningSourceInfo>& info)
	{
		m_dataSources.getTuningDataSourcesInfo(info);
	}


	bool TuningIPENServiceWorker::readTuningDataSources(XmlReadHelper& xml)
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

			TuningIPEN::TuningSource* ds = new TuningIPEN::TuningSource();

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


	void TuningIPENServiceWorker::initialize()
	{
		loadConfigurationFromFile(cfgFileName());

		allocateSignalsAndStates();

		runTuningSocket();

		if (m_tuningIPENService != nullptr)
		{
			connect(this, &TuningIPENServiceWorker::tuningServiceReady, m_tuningIPENService, &TuningIPENService::tuningServiceReady);
			connect(this, &TuningIPENServiceWorker::signalStateReady, m_tuningIPENService, &TuningIPENService::signalStateReady);
			connect(this, &TuningIPENServiceWorker::tuningDataSourceStateUpdate, m_tuningIPENService, &TuningIPENService::tuningDataSourceStateUpdate);

			connect(m_tuningIPENService, &TuningIPENService::signal_getSignalState, this, &TuningIPENServiceWorker::onGetSignalState);
			connect(m_tuningIPENService, &TuningIPENService::signal_setSignalState, this, &TuningIPENServiceWorker::onSetSignalState);
		}

		connect(&m_timer, &QTimer::timeout, this, &TuningIPENServiceWorker::onTimer);

		m_timer.setInterval(333);
		m_timer.start();

		emit tuningServiceReady();
	}


	void TuningIPENServiceWorker::shutdown()
	{
		m_timer.stop();

		stopTuningSocket();
	}


	void TuningIPENServiceWorker::runTuningSocket()
	{
		m_tuningSocket = new TuningIPENSocketWorker(m_tuningSettings.tuningDataIP, m_tuningIPENService);

		connect(m_tuningSocket, &TuningIPENSocketWorker::replyReady, this, &TuningIPENServiceWorker::onReplyReady);

		m_tuningSocketThread = new SimpleThread(m_tuningSocket);
		m_tuningSocketThread->start();
	}


	void TuningIPENServiceWorker::stopTuningSocket()
	{
		m_tuningSocketThread->quitAndWait();		// m_tuningSocket delete inside
		delete m_tuningSocketThread;
	}


	void TuningIPENServiceWorker::onSetSignalState(QString appSignalID, double value)
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

		TuningIPEN::TuningSource* source = m_dataSources[sourceID];

		if (source == nullptr)
		{
			assert(false);
			return;
		}

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


	void TuningIPENServiceWorker::allocateSignalsAndStates()
	{
		QVector<TuningIPEN::TuningSourceInfo> info;

		getTuningDataSourcesInfo(info);

		// allocate Signals
		//
		m_appSignals.clear();
		m_signal2Source.clear();

		for(const TuningIPEN::TuningSourceInfo& source : info)
		{
			for(const Signal& signal : source.tuningSignals)
			{
				Signal* appSignal = new Signal();

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
			AppSignalStateEx* appSignalState = m_appSignalStates[i];

			appSignalState->setSignalParams(i, appSignal);
		}
	}


	void TuningIPENServiceWorker::onTimer()
	{
		sendPeriodicReadRequests();

		testConnections();

		emitTuningDataSourcesStates();
	}


	void TuningIPENServiceWorker::sendPeriodicReadRequests()
	{
		for(TuningIPEN::TuningSource* source : m_dataSources)
		{
			sendPeriodicFrameRequest(source);
		}
	}


	void TuningIPENServiceWorker::testConnections()
	{
		qint64 nowTime = QDateTime::currentMSecsSinceEpoch();

		for(TuningIPEN::TuningSource* source : m_dataSources)
		{
			if (source == nullptr)
			{
				continue;
			}

			source->testConnection(nowTime);
		}
	}


	void TuningIPENServiceWorker::sendPeriodicFrameRequest(TuningIPEN::TuningSource* source)
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


	void TuningIPENServiceWorker::onReplyReady()
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

			TuningIPEN::TuningSource* source = m_dataSources.getDataSourceByIP(sr.lmIP);

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


	void TuningIPENServiceWorker::onGetSignalState(QString appSignalID)
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

		TuningIPEN::TuningSource* source = m_dataSources[sourceID];

		if (source == nullptr)
		{
			assert(false);
			emit signalStateReady(appSignalID, 0, 0, 0, false);
			return;
		}

		Tuning::TuningSignalState tss;

		bool result = source->getSignalState(appSignalID, &tss);

		if (result == false)
		{
			emit signalStateReady(appSignalID, 0, 0, 0, false);
			return;
		}

		emit signalStateReady(appSignalID, tss.currentValue, tss.lowLimit, tss.highLimit, tss.valid);
	}


	void TuningIPENServiceWorker::emitTuningDataSourcesStates()
	{
		for(TuningIPEN::TuningSource* source : m_dataSources)
		{
			if (source == nullptr)
			{
				continue;
			}

			emit tuningDataSourceStateUpdate(source->getState());
		}
	}


	// -------------------------------------------------------------------------------------
	//
	// TuningIPENService class implementation
	//
	// -------------------------------------------------------------------------------------

	TuningIPENService::TuningIPENService(TuningIPENServiceWorker* worker) :
		Service(worker)
	{
		worker->setTuningService(this);
	}


	void TuningIPENService::getTuningDataSourcesInfo(QVector<TuningIPEN::TuningSourceInfo>& info)
	{
		if (m_tuningServiceWorker == nullptr)
		{
			assert(false);
			return;
		}

		m_tuningServiceWorker->getTuningDataSourcesInfo(info);
	}


	void TuningIPENService::setSignalState(QString appSignalID, double value)
	{
		emit signal_setSignalState(appSignalID, value);
	}

	void TuningIPENService::getSignalState(QString appSignalID)
	{
		emit signal_getSignalState(appSignalID);
	}

}
