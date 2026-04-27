#include "TuningClientContext.h"
#include "../UtilsLib/WUtils.h"
#include <algorithm>

namespace Tuning
{
	// ----------------------------------------------------------------------------------------------
	//
	// TuningClientContext class implementation
	//
	// ----------------------------------------------------------------------------------------------

	TuningClientContext::TuningClientContext(const QString& clientID,
											 bool tuningLogin,
											 const QString& matsUsersList,
											 const std::vector<OnlineLib::MatsUser>& matsUsers,
											 const QStringList& drivenSourcesIDs,
											 const TuningSources& sources) :
		m_clientID(clientID),
		m_tuningLogin(tuningLogin),
		m_tuningSources(sources)
	{
		QStringList users = matsUsersList.split(Separator::SEMICOLON, Qt::SkipEmptyParts);

		for(const QString& userLogin : users)
		{
			for(const OnlineLib::MatsUser matsUser : matsUsers)
			{
				if (userLogin != matsUser.login())
				{
					continue;
				}

				if (matsUser.enabled() == false)
				{
					m_disabledUsers.emplace(userLogin);
					continue;
				}

				if (m_matsUsers.contains(userLogin) == true)
				{
					continue;
				}

				m_matsUsers.emplace(userLogin, matsUser.appSignalTags());
				break;
			}
		}

		//

		std::map<QString, std::set<Hash>> tagSignals;	// app signal tag => signal calcHash(appSignalID)

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

				//

				const std::set<QString>& signalTags = signal->tagsSet();

				for(const QString& tag : signalTags)
				{
					auto it = tagSignals.find(tag);

					if (it == tagSignals.end())
					{
						auto [newIt, b] = tagSignals.emplace(tag, std::set<Hash>{});
						it = newIt;
					}

					it->second.insert(signalHash);
				}
			}
		}

		//

		for(const auto& [user, userTags] : m_matsUsers)
		{
			auto userIt = m_userAllowedSignals.find(user);

			if (userIt == m_userAllowedSignals.end())
			{
				auto [newIt, b] = m_userAllowedSignals.emplace(user, std::set<Hash>{});
				userIt = newIt;
			}

			std::set<Hash>& userAllowedSignals = userIt->second;

			for(const QString& userTag : userTags)
			{
				auto tagIt = tagSignals.find(userTag);

				if (tagIt != tagSignals.end())
				{
					const std::set<Hash>& tagSignalsHashes = tagIt->second;
					userAllowedSignals.insert(tagSignalsHashes.begin(), tagSignalsHashes.end());
				}
			}
		}
	}

	TuningClientContext::~TuningClientContext()
	{
		clear();
	}

	void TuningClientContext::readSignalStates(const Network::TuningSignalsRead& request, Network::TuningSignalsReadReply* reply) const
	{
		TEST_PTR_RETURN(reply);

		reply->set_readrequestid(request.readrequestid());

		int signalCount = request.signalhash_size();

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

		reply->set_error(TO_INT(E::NetworkError::Success));
	}

	void TuningClientContext::writeSignalStates(const QString& clientEquipmentID,
												const QString& matsUser,
												const Network::TuningSignalsWrite& request,
												Network::TuningSignalsWriteReply* reply) const
	{
		TEST_PTR_RETURN(reply);

		//

		if (m_tuningLogin == true &&
			(matsUser.isEmpty() == true || m_userAllowedSignals.contains(matsUser) == false))
		{
			if (m_disabledUsers.contains(matsUser) == true)
			{
				reply->set_error(TO_INT(E::NetworkError::DisabledMatsUser));
				return;
			}

			reply->set_error(TO_INT(E::NetworkError::UnknownMatsUser));
			return;
		}

		const std::set<Hash>* userAllowedSignals = nullptr;

		if (m_tuningLogin == true)
		{
			auto it = m_userAllowedSignals.find(matsUser);

			if (it == m_userAllowedSignals.end() ||
				it->second.empty() == true)
			{
				reply->set_error(TO_INT(E::NetworkError::NoSignalsAllowedToControl));
				return;
			}

			userAllowedSignals = &it->second;
		}

		//

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
				writeResult->set_error(TO_INT(E::NetworkError::UnknownSignalHash));
				continue;
			}

			if (userAllowedSignals != nullptr &&
				userAllowedSignals->contains(signalHash) == false)
			{
				writeResult->set_error(TO_INT(E::NetworkError::SignalIsNotAllowedToControl));
				continue;
			}

			TuningSourceThreadShared sourceThread = result.second;

			if (sourceThread == nullptr)
			{
				writeResult->set_error(TO_INT(E::NetworkError::LmControlIsNotActive));
				continue;
			}

			E::NetworkError err = sourceThread->writeSignalState(clientEquipmentID, matsUser, signalHash, TuningValue(writeCmd.value()));

			if (err != E::NetworkError::Success)
			{
				writeResult->set_error(TO_INT(err));
				hasErrors = true;
			}
			else
			{
				writeResult->set_error(TO_INT(E::NetworkError::Success));
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

				E::NetworkError err = srcThread->applySignalStates(clientEquipmentID, matsUser);

				if (err != E::NetworkError::Success)
				{
					hasErrors = true;
				}
			}
		}

		E::NetworkError result = (hasErrors == true ? E::NetworkError::InternalError : E::NetworkError::Success);

		reply->set_error(TO_INT(result));
	}

	void TuningClientContext::applySignalStates(const QString& clientEquipmentID, const QString& matsUser) const
	{
		for(auto& p : m_sourceThreadMap)
		{
			TuningSourceThreadShared srcThread = p.second;

			if (srcThread == nullptr)
			{
				continue;		// it's Ok
			}

			srcThread->applySignalStates(clientEquipmentID, matsUser);
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

	void TuningClientContext::registerStateChangesQueue(qint64 tcpConnectionID)
	{
		AUTO_LOCK_BY_CURRENT_THREAD(m_queueMapMutex);

		auto it = m_stateChangesQueueMap.find(tcpConnectionID);

		if(it != m_stateChangesQueueMap.end())
		{
			Q_ASSERT(false);
			return;
		}

		m_stateChangesQueueMap.emplace(tcpConnectionID,
									   new TuningSignalsChangesQueue(getStateChangesQueueSize()));
	}

	void TuningClientContext::unregisterStateChangesQueue(qint64 tcpConnectionID)
	{
		AUTO_LOCK_BY_CURRENT_THREAD(m_queueMapMutex);

		auto it = m_stateChangesQueueMap.find(tcpConnectionID);

		if(it == m_stateChangesQueueMap.end())
		{
			Q_ASSERT(false);
			return;
		}

		delete it->second;

		m_stateChangesQueueMap.erase(it);
	}

	void TuningClientContext::pushSignalStateChange(const TuningSignal::State& state)
	{
		Q_ASSERT(m_signalToSourceIdMap.find(state.signalHash) != m_signalToSourceIdMap.end());

		AUTO_LOCK_BY_CURRENT_THREAD(m_queueMapMutex);

		for(auto& p : m_stateChangesQueueMap)
		{
			TuningSignalsChangesQueue* queue = p.second;

			TEST_PTR_CONTINUE(queue);

			queue->push(state);
		}
	}

	TuningSignalsChangesQueue* TuningClientContext::getSignalChangesQueue(qint64 tcpConnectionID)
	{
		AUTO_LOCK_BY_CURRENT_THREAD(m_queueMapMutex);

		auto it = m_stateChangesQueueMap.find(tcpConnectionID);

		if(it == m_stateChangesQueueMap.end())
		{
			Q_ASSERT(false);
			return nullptr;
		}

		return it->second;
	}

	const std::map<Hash, QString>& TuningClientContext::signalToSourceIdMap() const
	{
		return m_signalToSourceIdMap;
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
			tss->set_error(TO_INT(E::NetworkError::UnknownSignalHash));
			return;
		}

		TuningSourceThreadShared sourceThread = result.second;

		if (sourceThread == nullptr)
		{
			tss->set_valid(false);
			tss->set_error(TO_INT(E::NetworkError::LmControlIsNotActive));
			return;
		}

		sourceThread->readSignalState(tss);
	}

	void TuningClientContext::clear()
	{
		m_sourceThreadMap.clear();
		m_signalToSourceIdMap.clear();
	}

	int TuningClientContext::getStateChangesQueueSize() const
	{
		int signalCount = static_cast<int>(m_signalToSourceIdMap.size());

		return static_cast<int>(std::max(signalCount * 2, 100));
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
			TuningClientContext* clientContext = new TuningClientContext(client.equipmentID,
																		 client.tuningLogin,
																		 client.matsUsers,
																		 tss.matsUsers,
																		 client.uniqueSourcesIDs(),
																		 sources);

			m_clientsContextMap.emplace(client.equipmentID, clientContext);

			const std::map<Hash, QString>& signalsMap = clientContext->signalToSourceIdMap();

			for(const auto& p : signalsMap)
			{
				Hash signalHash = p.first;

				auto it = m_signalToClientContextMap.find(signalHash);

				if (it == m_signalToClientContextMap.end())
				{
					auto res = m_signalToClientContextMap.emplace(signalHash, std::set<TuningClientContext*>());

					it = res.first;
				}

				it->second.insert(clientContext);
			}
		}
	}

	TuningClientContext* TuningClientContextMap::getClientContext(const QString& clientEquipmentID) const
	{
		auto it = m_clientsContextMap.find(clientEquipmentID);

		if (it == m_clientsContextMap.end())
		{
			return nullptr;
		}

		return it->second;
	}

	void TuningClientContextMap::getAllClientContexts(QVector<const TuningClientContext*>& clientContexts) const
	{
		clientContexts.clear();

		for(const auto& p: m_clientsContextMap)
		{
			TEST_PTR_CONTINUE(p.second);

			clientContexts.append(p.second);
		}
	}

	void TuningClientContextMap::setSourceThreadInTuningClientContexts(TuningSourceThreadShared thread)
	{
		for(const auto& p: m_clientsContextMap)
		{
			TEST_PTR_CONTINUE(p.second);

			p.second->setSourceThread(thread);
		}
	}

	void TuningClientContextMap::removeSourceThreadFromTuningClientContexts(const QString& tuningSourceID)
	{
		for(const auto& p: m_clientsContextMap)
		{
			TEST_PTR_CONTINUE(p.second);

			p.second->removeSourceThread(tuningSourceID);
		}
	}

	void TuningClientContextMap::clear()
	{
		m_signalToClientContextMap.clear();

		for(const auto& p : m_clientsContextMap)
		{
			TuningClientContext* clientContext = p.second;

			if (clientContext == nullptr)
			{
				Q_ASSERT(false);
				continue;
			}

			delete clientContext;
		}

		m_clientsContextMap.clear();
	}

	void TuningClientContextMap::pushSignalStateChange(const TuningSignal::State& state)
	{
		auto it = m_signalToClientContextMap.find(state.signalHash);

		if (it == m_signalToClientContextMap.end())
		{
			return;
		}

		std::set<TuningClientContext*>& contexts = it->second;

		for(TuningClientContext* context : contexts)
		{
			context->pushSignalStateChange(state);
		}
	}

}
