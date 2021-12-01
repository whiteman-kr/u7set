#include "TuningClientContext.h"
#include "../UtilsLib/WUtils.h"

namespace Tuning
{

	// ----------------------------------------------------------------------------------------------
	//
	// TuningSourceContext class implementation
	//
	// ----------------------------------------------------------------------------------------------

/*	TuningSourceContext::TuningSourceContext(const QString& sourceID, const TuningSource *source) :
		m_sourceID(sourceID)
	{
		if (source == nullptr)
		{
			assert(false);
			return;
		}

		source->saveToProto(&m_sourceInfo);
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
	}*/

	// ----------------------------------------------------------------------------------------------
	//
	// TuningClientContext class implementation
	//
	// ----------------------------------------------------------------------------------------------

	TuningClientContext::TuningClientContext(const QString &clientID,
											 const QStringList& drivenSourcesIDs,
											 const TuningSources& sources) :
		m_clientID(clientID),
		m_tuningSources(sources)
	{
		for(const QString& sourceID : drivenSourcesIDs)
		{
			if (m_sourceThreadMap.contains(sourceID) == true)
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

			m_sourceThreadMap.insert({sourceID, nullptr});

			// fill m_signalToSourceMap
			//
			TuningDataSharedConst tuningData = source->tuningData();

			if (tuningData == nullptr)
			{
				continue;
			}

			QVector<AppSignal*> sourceSignals;

			tuningData->getSignals(&sourceSignals);

			for(const AppSignal* signal : sourceSignals)
			{
				if (signal == nullptr)
				{
					assert(false);
					continue;
				}

				Hash signalHash = ::calcHash(signal->appSignalID());

				m_signalToSourceIdMap.insert({signalHash, sourceID});
			}
		}
	}

	TuningClientContext::~TuningClientContext()
	{
		clear();
	}

	void TuningClientContext::getSourcesInfo(std::vector<Network::DataSourceInfo>* dataSourcesInfo) const
	{
		TEST_PTR_RETURN(dataSourcesInfo);

		dataSourcesInfo->clear();

		dataSourcesInfo->resize(m_sourceThreadMap.size());

		int count = 0;

		for(auto& p : m_sourceThreadMap)
		{
			QString sourceID = p.first;

			const TuningSource* src = m_tuningSources.getSourceByID(sourceID);

			if (src == nullptr)
			{
				Q_ASSERT(false);
				continue;
			}

			src->saveToProto(&dataSourcesInfo->at(count));

			count++;
		}

		if (count < dataSourcesInfo->size())
		{
			dataSourcesInfo->resize(count);
		}
	}

	void TuningClientContext::getSourcesStates(std::vector<Network::TuningSourceState>* tuningSourcesStates) const
	{
		TEST_PTR_RETURN(tuningSourcesStates);

		tuningSourcesStates->clear();

		for(const auto& p : m_sourceThreadMap)
		{
			QString sourceID = p.first;
			const TuningSourceThreadShared sourceThread = p.second;

			if (sourceThread == nullptr)
			{
				const TuningSource* src = m_tuningSources.getSourceByID(sourceID);

				TEST_PTR_CONTINUE(src);

				const QStringList& tuningLans = src->getEnabledLansProvidedTuning();

				for(const QString& lanID : tuningLans)
				{
					Network::TuningSourceState s;

					s.set_sourceid(src->ID());
					s.set_lanequipmentid(lanID.toStdString());
					s.set_isreply(false);

					tuningSourcesStates->push_back(s);
				}
			}
			else
			{
				sourceThread->getSourceState(tuningSourcesStates);
			}
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

		std::set<TuningSourceThreadShared> usedSrcThreads;

		bool hasErrors = false;

		for(int i = 0; i < writeRequestCount; i++)
		{
			Network::TuningSignalWriteResult* writeResult = reply->add_writeresult();

			TEST_PTR_CONTINUE(writeResult);

			const Network::TuningWriteCommand& writeCmd = request.commands(i);

			Hash signalHash = writeCmd.signalhash();

			writeResult->set_signalhash(signalHash);

			std::pair<bool, TuningSourceThreadShared> result = getSourceThreadBySignalHash(signalHash);

			if (result.first == false)
			{
				writeResult->set_error(TO_INT(NetworkError::UnknownSignalHash));
				continue;
			}

			TuningSourceThreadShared sourceThread = result.second;

			if (sourceThread == nullptr)
			{
				writeResult->set_error(TO_INT(NetworkError::LmControlIsNotActive));
				continue;
			}

			NetworkError err = sourceThread->writeSignalState(clientEquipmentID, user, signalHash, TuningValue(writeCmd.value()));

			if (err != NetworkError::Success)
			{
				hasErrors = true;
			}
			else
			{
				if (autoApply == true)
				{
					usedSrcThreads.insert(sourceThread);
				}
			}
		}

		if (autoApply == true && hasErrors == false)
		{
			for(const TuningSourceThreadShared& srcThread : usedSrcThreads)
			{
				TEST_PTR_CONTINUE(srcThread);

				NetworkError err = srcThread->applySignalStates(clientEquipmentID, user);

				if (err != NetworkError::Success)
				{
					hasErrors = true;
				}
			}
		}

		NetworkError result = hasErrors == true ? NetworkError::InternalError : NetworkError::Success;

		reply->set_error(TO_INT(result));
	}

	void TuningClientContext::applySignalStates(const QString& clientEquipmentID, const QString& user) const
	{
		for(auto& p : m_sourceThreadMap)
		{
			TuningSourceThreadShared srcThread = p.second;

			if (srcThread == nullptr)
			{
				continue;		// it's Ok
			}

			srcThread->applySignalStates(clientEquipmentID, user);
		}
	}

	void TuningClientContext::setSourceThread(TuningSourceThreadShared srcThread)
	{
		TEST_PTR_RETURN(srcThread);

		if (m_sourceThreadMap.contains(srcThread->sourceEquipmentID()) == false)
		{
			return;			// it's OK
		}

		m_sourceThreadMap.insert_or_assign(srcThread->sourceEquipmentID(), srcThread);
	}

	void TuningClientContext::removeSourceThread(const QString& tuningSourceID)
	{
		if (m_sourceThreadMap.contains(tuningSourceID) == false)
		{
			return;			// it's OK
		}

		m_sourceThreadMap.insert_or_assign(tuningSourceID, nullptr);
	}

	TuningSourceThreadShared TuningClientContext::getSourceThread(const QString& sourceID) const
	{
		auto it = m_sourceThreadMap.find(sourceID);

		if (it == m_sourceThreadMap.end())
		{
			return nullptr;
		}
		return it->second;
	}

	std::pair<bool, TuningSourceThreadShared> TuningClientContext::getSourceThreadBySignalHash(Hash signalHash) const
	{
		std::pair<bool, TuningSourceThreadShared> result;

		// result.first - true == signal is exists
		// result.second - thread ptr, if == nullptr - thread is not started

		result.first = false;
		result.second = nullptr;

		auto it = m_signalToSourceIdMap.find(signalHash);

		if (it == m_signalToSourceIdMap.end())
		{
			return result;
		}

		result.first = true;	// signal is found

		const QString& tuningSourceID = it->second;

		auto it2 = m_sourceThreadMap.find(tuningSourceID);

		if (it2 == m_sourceThreadMap.end())
		{
			return result;
		}

		result.second = it2->second;	// may be == nullptr!

		return result;
	}

	void TuningClientContext::readSignalState(Network::TuningSignalState* tss) const
	{
		TEST_PTR_RETURN(tss);

		// tss->signalHash is already filled!
		//
		Hash signalHash = tss->signalhash();

		std::pair<bool, TuningSourceThreadShared> result = getSourceThreadBySignalHash(signalHash);

		if (result.first == false)
		{
			tss->set_valid(false);
			tss->set_error(TO_INT(NetworkError::UnknownSignalHash));
			return;
		}

		TuningSourceThreadShared sourceThread = result.second;

		if (sourceThread == nullptr)
		{
			tss->set_valid(false);
			tss->set_error(TO_INT(NetworkError::LmControlIsNotActive));
			return;
		}

		sourceThread->readSignalState(tss);
	}

	void TuningClientContext::clear()
	{
		m_sourceThreadMap.clear();
		m_signalToSourceIdMap.clear();
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
		for(const auto& client : tss.clients)
		{
			TuningClientContext* clientContext = new TuningClientContext(client.equipmentID, client.uniqueSourcesIDs(), sources);

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
