#ifndef CLIENT_LIB_DOMAIN
	#error Do not include this file in the project! Link ClientLib instead.
#endif

#include "TcpAppSourcesState.h"


namespace ClientLib
{
	//
	// TcpAppSourcesState
	//
	TcpAppSourcesState::TcpAppSourcesState(const SoftwareInfo& softwareInfo,
										   const SoftwareEndpoint::AppDataService& ads,
										   ILogFile* logFile) :
		Tcp::Client(softwareInfo, ads.address, ads.address, "TcpAppSourcesState", ads.equipmentId),
		TcpClientStatistics(this),
		m_logFile(logFile, "TcpAppSourcesState")
	{
		Q_ASSERT(logFile);

		setObjectName("TcpSourcesStateClient");
		qDebug() << "TcpSourcesStateClient::TcpSourcesStateClient(...)";

		connect(this,
				&Tcp::Client::signal_wrongServerID,
				[this](const QString& errorMessage)
				{
					m_logFile.writeError(errorMessage);
				});

		return;
	}

	TcpAppSourcesState::~TcpAppSourcesState()
	{
		qDebug() << "TcpSourcesStateClient::~TcpSourcesStateClient()";
	}

	std::vector<ClientLib::AppDataSourceState> TcpAppSourcesState::appDataSourceStates() const
	{
		QReadLocker l(&m_appDataSourceStatesLock);

		std::vector<AppDataSourceState> result;
		result.reserve(m_appDataSourceStates.size());

		for (const auto& it : m_appDataSourceStates)
		{
			result.push_back(it.second);
		}

		return result;
	}

	int TcpAppSourcesState::sourceErrorCount()
	{
		QReadLocker l(&m_appDataSourceStatesLock);

		int result = 0;

		for (const auto& it : m_appDataSourceStates)
		{
			const AppDataSourceState& ads = it.second;

			if (ads.state.receivesdata() == false)
			{
				result++;
				continue;
			}

			result += ads.getErrorsCount();
		}

		return result;
	}

	void TcpAppSourcesState::onClientThreadStarted()
	{
		qDebug() << "TcpSourcesStateClient::onClientThreadStarted()";
		m_logFile.writeMessage("onClientThreadStarted()");

		return;
	}

	void TcpAppSourcesState::onClientThreadFinished()
	{
		qDebug() << "TcpSourcesStateClient::onClientThreadFinished()";
		m_logFile.writeMessage("onClientThreadFinished()");
	}

	void TcpAppSourcesState::onConnection()
	{
		qDebug() << "TcpSourcesStateClient::onConnection()";
		m_logFile.writeMessage("onConnection()");

		assert(isClearToSendRequest() == true);

		{
			QWriteLocker l(&m_appDataSourceStatesLock);
			m_appDataSourceStates.clear();
		}

		resetToGetAppDataSourcesInfo();

		return;
	}

	void TcpAppSourcesState::onDisconnection()
	{
		qDebug() << "TcpSourcesStateClient::onDisconnection";
		m_logFile.writeMessage("onDisconnection()");

		{
			QWriteLocker l(&m_appDataSourceStatesLock);
			for (auto& it : m_appDataSourceStates)
			{
				AppDataSourceState& ads = it.second;
				ads.invalidate();
			}
		}

		return;
	}

	void TcpAppSourcesState::onReplyTimeout()
	{
		qDebug() << "TcpSourcesStateClient::onReplyTimeout()";
		m_logFile.writeWarning("onReplyTimeout()");
	}

	void TcpAppSourcesState::processReply(quint32 requestID, const char* replyData, quint32 replyDataSize)
	{
		if (replyData == nullptr)
		{
			assert(replyData);
			return;
		}

		QByteArray data = QByteArray::fromRawData(replyData, replyDataSize);

		switch (requestID)
		{
		case ADS_GET_APP_DATA_SOURCES_INFO:
			processAppDataSourcesInfo(data);
			break;

		case ADS_GET_APP_DATA_SOURCES_STATES:
			processAppDataSourcesState(data);
			break;

		default:
			assert(false);

			qDebug() << "Wrong requestID in TcpAppDataSourcesStateClient::processReply()";
			m_logFile.writeError(QString("Wrong requestId in processReply(), requestId %1").arg(requestID));

			resetToGetAppDataSourcesInfo();
		}

		return;
	}

	void TcpAppSourcesState::resetToGetAppDataSourcesInfo()
	{
		QThread::msleep(m_requestPeriod);

		requestAppDataSourcesInfo();

		return;
	}

	void TcpAppSourcesState::resetToGetAppDataSourcesState()
	{
		QThread::msleep(m_requestPeriod);

		requestAppDataSourcesState();

		return;
	}


	void TcpAppSourcesState::requestAppDataSourcesInfo()
	{
		if (isClearToSendRequest() == false)
		{
			qDebug() << tr("TcpAppDataSourcesStateClient::requestTuningSourcesInfo, isClearToSendRequest() == false, reconnecting.");
			m_logFile.writeError(QString("requestTuningSourcesInfo, isClearToSendRequest() == false, reconnecting."));
			closeConnection();
			return;
		}

		{
			QWriteLocker l(&m_appDataSourceStatesLock);
			m_appDataSourceStates.clear();
		}

		sendRequest(ADS_GET_APP_DATA_SOURCES_INFO);
	}

	void TcpAppSourcesState::processAppDataSourcesInfo(const QByteArray& data)
	{
		bool ok = m_getDataSourcesInfoReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));

		if (ok == false)
		{
			assert(ok);
			resetToGetAppDataSourcesInfo();
			return;
		}

		if (m_getDataSourcesInfoReply.error() != static_cast<int>(E::NetworkError::Success))
		{
			qDebug() << tr("TcpAppDataSourcesStateClient::m_getDataSourcesInfoReply, error received: %1")
							.arg(E::valueToString(static_cast<E::NetworkError>(m_getDataSourcesInfoReply.error())));

			m_logFile.writeError(QString("m_getDataSourcesInfoReply, error received: %1")
									 .arg(E::valueToString(static_cast<E::NetworkError>(m_getDataSourcesInfoReply.error()))));

			resetToGetAppDataSourcesInfo();
			return;
		}

		{
			QWriteLocker l(&m_appDataSourceStatesLock);
			m_appDataSourceStates.clear();

			for (int i = 0; i < m_getDataSourcesInfoReply.datasourceinfo_size(); i++)
			{
				const ::Network::DataSourceInfo& dsi = m_getDataSourcesInfoReply.datasourceinfo(i);

				AppDataSourceState ads;
				ads.info = dsi;

				quint64 id = dsi.id();

				assert(m_appDataSourceStates.count(id) == 0);
				m_appDataSourceStates[id] = ads;
			}
		}

		resetToGetAppDataSourcesState();
	}


	void TcpAppSourcesState::requestAppDataSourcesState()
	{
		assert(isClearToSendRequest());
		sendRequest(ADS_GET_APP_DATA_SOURCES_STATES);
	}

	void TcpAppSourcesState::processAppDataSourcesState(const QByteArray& data)
	{
		bool ok = m_getAppDataSourcesStateReply.ParseFromArray(data.constData(), static_cast<int>(data.size()));

		if (ok == false)
		{
			assert(ok);
			resetToGetAppDataSourcesState();
			return;
		}

		if (m_getAppDataSourcesStateReply.error() != static_cast<int>(E::NetworkError::Success))
		{
			qDebug() << "TcpSourcesStateClient::processAppDataSourcesState, error received: " << m_getAppDataSourcesStateReply.error();
			m_logFile.writeError(QString("processAppDataSourcesState, error received: %1").arg(m_getAppDataSourcesStateReply.error()));

			assert(m_getAppDataSourcesStateReply.error() != static_cast<int>(E::NetworkError::Success));

			resetToGetAppDataSourcesState();
			return;
		}

		//
		{
			QWriteLocker l(&m_appDataSourceStatesLock);

			for (int i = 0; i < m_getAppDataSourcesStateReply.appdatasourcesstates_size(); i++)
			{
				const ::Network::AppDataSourceState& state = m_getAppDataSourcesStateReply.appdatasourcesstates(i);

				quint64 id = state.id();

				bool found = false;

				for (auto& it : m_appDataSourceStates)
				{
					AppDataSourceState& ads = it.second;

					if (ads.id() == id)
					{
						ads.setNewState(state);

						found = true;

						break;
					}
				}

				if (found == false)
				{
					assert(false);
				}
			}
		}

		//

		resetToGetAppDataSourcesState();

		return;
	}

} // namespace ClientLib
