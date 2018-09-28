#include "TuningClientContext.h"
#include "../lib/WUtils.h"

namespace Tuning
{

	// ----------------------------------------------------------------------------------------------
	//
	// TuningSourceContext class implementation
	//
	// ----------------------------------------------------------------------------------------------

	TuningSourceContext::TuningSourceContext(const QString& sourceID, const TuningSource *source) :
		m_sourceID(sourceID)
	{
		if (source == nullptr)
		{
			assert(false);
			return;
		}

		source->getInfo(&m_sourceInfo);
	}

	void TuningSourceContext::getSourceInfo(Network::DataSourceInfo* si) const
	{
		TEST_PTR_RETURN(si);

		*si = m_sourceInfo;
	}

	void TuningSourceContext::getSourceState(Network::TuningSourceState* tss) const
	{
		TEST_PTR_RETURN(tss);

		if (m_sourceThread == nullptr)
		{
			tss->set_sourceid(m_sourceInfo.id());
			tss->set_isreply(false);
			tss->set_controlisactive(false);
			tss->set_setsor(false);
		}
		else
		{
			m_sourceThread->getState(tss);
		}
	}

	void TuningSourceContext::setSourceThread(TuningSourceThread* thread)
	{
		TEST_PTR_RETURN(thread);

		if (thread->sourceEquipmentID() != m_sourceID)
		{
			assert(false);
			return;
		}

		assert(m_sourceThread == nullptr);

		m_sourceThread = thread;
	}

	void TuningSourceContext::removeSourceThread(TuningSourceThread* thread)
	{
		TEST_PTR_RETURN(thread);

		if (thread->sourceEquipmentID() != m_sourceID)
		{
			assert(false);
			return;
		}

		if (m_sourceThread != thread)
		{
			assert(false);
			return;
		}

		m_sourceThread = nullptr;
	}

	void TuningSourceContext::readSignalState(Network::TuningSignalState* tss)
	{
		TEST_PTR_RETURN(tss);

		if (m_sourceThread == nullptr)
		{
			tss->set_valid(false);
			tss->set_error(TO_INT(NetworkError::LmControlIsNotActive));
			return;
		}

		m_sourceThread->readSignalState(tss);
	}

	NetworkError TuningSourceContext::writeSignalState(	const QString& clientEquipmentID,
														const QString& user,
														Hash signalHash,
														const TuningValue& newValue)
	{
		if (m_sourceThread == nullptr)
		{
			return NetworkError::LmControlIsNotActive;
		}

		return m_sourceThread->writeSignalState(clientEquipmentID, user, signalHash, newValue);
	}

	NetworkError TuningSourceContext::applySignalStates(const QString& clientEquipmentID,
														const QString& user)
	{
		if (m_sourceThread == nullptr)
		{
			return NetworkError::LmControlIsNotActive;
		}

		return m_sourceThread->applySignalStates(clientEquipmentID, user);
	}

	// ----------------------------------------------------------------------------------------------
	//
	// TuningClientContext class implementation
	//
	// ----------------------------------------------------------------------------------------------

	TuningClientContext::TuningClientContext(const QString &clientID, const QStringList &sourcesIDs, const TuningSources& sources) :
		m_clientID(clientID)
	{
		for(const QString& sourceID : sourcesIDs)
		{
			if (m_sourceContextMap.contains(sourceID))
			{
				assert(false);
				continue;
			}

			const TuningSource* source = sources.getSourceByID(sourceID);

			if (source == nullptr)
			{
				qDebug() << C_STR(QString("TuningClientContext: not found tuning source with ID '%1'").arg(sourceID));
				continue;
			}

			TuningSourceContext* sourceContext = new TuningSourceContext(sourceID, source);

			m_sourceContextMap.insert(sourceID, sourceContext);

			// fill m_signalToSourceMap
			//
			const TuningData* tuningData = source->tuningData();

			if (tuningData == nullptr)
			{
				continue;
			}

			QVector<Signal*> sourceSignals;

			tuningData->getSignals(sourceSignals);

			for(const Signal* signal : sourceSignals)
			{
				if (signal == nullptr)
				{
					assert(false);
					continue;
				}

				Hash signalHash = ::calcHash(signal->appSignalID());

				m_signalToSourceMap.insert(signalHash, sourceContext);
			}
		}
	}

	TuningClientContext::~TuningClientContext()
	{
		clear();
	}

	void TuningClientContext::getSourcesInfo(QVector<Network::DataSourceInfo>& dataSourcesInfo) const
	{
		dataSourcesInfo.clear();

		dataSourcesInfo.resize(m_sourceContextMap.count());

		int count = 0;

		for(const TuningSourceContext* sourceContext : m_sourceContextMap)
		{
			if (sourceContext == nullptr)
			{
				assert(false);
				continue;
			}

			sourceContext->getSourceInfo(&dataSourcesInfo[count]);

			count++;
		}

		if (count < dataSourcesInfo.size())
		{
			dataSourcesInfo.resize(count);
		}
	}

	void TuningClientContext::getSourcesStates(QVector<Network::TuningSourceState>& tuningSourcesStates) const
	{
		tuningSourcesStates.clear();

		tuningSourcesStates.resize(m_sourceContextMap.count());

		int count = 0;

		for(const TuningSourceContext* sourceContext : m_sourceContextMap)
		{
			if (sourceContext == nullptr)
			{
				assert(false);
				continue;
			}

			sourceContext->getSourceState(&tuningSourcesStates[count]);

			count++;
		}

		if (count < tuningSourcesStates.size())
		{
			tuningSourcesStates.resize(count);
		}
	}

	void TuningClientContext::readSignalStates(const Network::TuningSignalsRead& request, Network::TuningSignalsReadReply* reply) const
	{
		TEST_PTR_RETURN(reply);

		int signalCount = request.signalhash_size();

		//reply.mutable_tuningsignalstate()->Reserve(signalCount);

		reply->clear_tuningsignalstate();

		for(int i = 0; i < signalCount; i++)
		{
			Network::TuningSignalState* tss = reply->add_tuningsignalstate();

			if (tss == nullptr)
			{
				continue;
			}

			Hash signalHash = request.signalhash(i);

			tss->set_signalhash(signalHash);

			readSignalState(tss);
		}

		reply->set_error(TO_INT(NetworkError::Success));
	}

	void TuningClientContext::writeSignalStates(const QString& clientEquipmentID,
												const QString& user,
												const Network::TuningSignalsWrite& request,
												Network::TuningSignalsWriteReply* reply) const
	{
		TEST_PTR_RETURN(reply);

		int writeRequestCount = request.commands_size();

		bool autoApply = request.autoapply();

		reply->clear_writeresult();

		QHash<TuningSourceContext*, TuningSourceContext*> m_usedSrcContexts;

		bool hasErrors = false;

		for(int i = 0; i < writeRequestCount; i++)
		{
			Network::TuningSignalWriteResult* writeResult = reply->add_writeresult();

			TEST_PTR_CONTINUE(writeResult);

			const Network::TuningWriteCommand& writeCmd = request.commands(i);

			Hash signalHash = writeCmd.signalhash();

			writeResult->set_signalhash(signalHash);

			TuningSourceContext* sourceContext = getSourceContextBySignalHash(signalHash);

			if (sourceContext == nullptr)
			{
				writeResult->set_error(TO_INT(NetworkError::UnknownSignalHash));
				continue;
			}

			NetworkError err = sourceContext->writeSignalState(clientEquipmentID, user, signalHash, TuningValue(writeCmd.value()));

			if (err != NetworkError::Success)
			{
				hasErrors = true;
			}
			else
			{
				if (autoApply == true)
				{
					m_usedSrcContexts.insert(sourceContext, sourceContext);
				}
			}
		}

		if (autoApply == true && hasErrors == false)
		{
			for(TuningSourceContext* usedSrcContext : m_usedSrcContexts)
			{
				TEST_PTR_CONTINUE(usedSrcContext);

				NetworkError err = usedSrcContext->applySignalStates(clientEquipmentID, user);

				if (err != NetworkError::Success)
				{
					hasErrors = true;
				}
			}

			m_usedSrcContexts.clear();
		}

		NetworkError result = hasErrors == true ? NetworkError::InternalError : NetworkError::Success;

		reply->set_error(TO_INT(result));
	}

	void TuningClientContext::applySignalStates(const QString& clientEquipmentID, const QString& user) const
	{
		for(TuningSourceContext* srcContext : m_sourceContextMap)
		{
			if (srcContext == nullptr)
			{
				assert(false);
				continue;
			}

			srcContext->applySignalStates(clientEquipmentID, user);
		}
	}

	void TuningClientContext::setSourceThread(TuningSourceThread* thread)
	{
		TEST_PTR_RETURN(thread);

		TuningSourceContext* sourceContext = getSourceContext(thread->sourceEquipmentID());

		if (sourceContext == nullptr)
		{
			return;			// its OK
		}

		sourceContext->setSourceThread(thread);
	}

	void TuningClientContext::removeSourceThread(TuningSourceThread* thread)
	{
		TEST_PTR_RETURN(thread);

		TuningSourceContext* sourceContext = getSourceContext(thread->sourceEquipmentID());

		if (sourceContext == nullptr)
		{
			return;			// its OK
		}

		sourceContext->removeSourceThread(thread);
	}

	TuningSourceContext* TuningClientContext::getSourceContext(const QString& sourceID) const
	{
		TuningSourceContext* srcContext = m_sourceContextMap.value(sourceID, nullptr);

		return srcContext;
	}

	TuningSourceContext* TuningClientContext::getSourceContextBySignalHash(Hash signalHash) const
	{
		TuningSourceContext* srcContext = m_signalToSourceMap.value(signalHash, nullptr);

		return srcContext;
	}

	void TuningClientContext::readSignalState(Network::TuningSignalState* tss) const
	{
		TEST_PTR_RETURN(tss);

		// tss->signalHash is already filled!
		//
		Hash signalHash = tss->signalhash();

		TuningSourceContext* sourceContext = getSourceContextBySignalHash(signalHash);

		if (sourceContext == nullptr)
		{
			tss->set_valid(false);
			tss->set_error(TO_INT(NetworkError::UnknownSignalHash));
			return;
		}

		sourceContext->readSignalState(tss);
	}

	void TuningClientContext::clear()
	{
		for(TuningSourceContext* sourceContext : m_sourceContextMap)
		{
			if (sourceContext == nullptr)
			{
				assert(false);
				continue;
			}

			delete sourceContext;
		}

		m_sourceContextMap.clear();
	}

	// ----------------------------------------------------------------------------------------------
	//
	// TuningClientContextMap class implementation
	//
	// ----------------------------------------------------------------------------------------------

	TuningClientContextMap::TuningClientContextMap()
	{
	}


	TuningClientContextMap::~TuningClientContextMap()
	{
		clear();
	}

	void TuningClientContextMap::init(const TuningServiceSettings& tss, const TuningSources& sources)
	{
		for(const TuningServiceSettings::TuningClient& client : tss.clients)
		{
			if (contains(client.equipmentID) == true)
			{
				assert(false);
				continue;
			}

			TuningClientContext* clientContext = new TuningClientContext(client.equipmentID, client.sourcesIDs, sources);

			insert(client.equipmentID, clientContext);
		}
	}

	TuningClientContext* TuningClientContextMap::getClientContext(QString clientID) const
	{
		TuningClientContext* clientContext = value(clientID, nullptr);

		return clientContext;
	}

	void TuningClientContextMap::clear()
	{
		for(TuningClientContext* clientContext : *this)
		{
			if (clientContext == nullptr)
			{
				assert(false);
				continue;
			}

			delete clientContext;
		}

		QHash<QString, TuningClientContext*>::clear();
	}
}
