#include "IvsImpulseGatewayHandler.h"

namespace Gateway
{
	// ---------------------------------------------------------------------------------
	//
	//	Gateway::IvsImpulseHandler class implementation
	//
	// ---------------------------------------------------------------------------------

	IvsImpulseHandler::IvsImpulseHandler(const SoftwareInfo& swInfo,
										 const GatewayServiceSettings& settings,
										 IvsImpulseGatewayShared gateway,
										 const AppSignals& appSignals,
										 CircularLoggerShared log,
										 bool logGatewayPackets) :
		Handler(swInfo, settings, log, logGatewayPackets),
		m_softwareInfo(swInfo),
		m_appDataService1(settings.appDataService1.address),
		m_appDataService2(settings.appDataService2.address),
		m_gateway(gateway),
		m_appSignals(appSignals)
	{
	}

	IvsImpulseHandler::~IvsImpulseHandler()
	{
		shutdown();
	}

	void IvsImpulseHandler::run()
	{
		init();

		m_appDataServiceClientThread =
				new AppDataServiceClientThread( m_softwareInfo,
												m_appDataService1,
												m_appDataService2,
												QString("GatewayService %1").arg(m_softwareInfo.equipmentID()),
												*this, m_log);
		m_appDataServiceClientThread->start();

		m_ivsImpulseCommThread = new IvsImpulseCommThread(*this);

		m_ivsImpulseCommThread->connect(m_appDataServiceClientThread->client());

		m_ivsImpulseCommThread->start();
	}

	void IvsImpulseHandler::shutdown()
	{
		if (m_ivsImpulseCommThread != nullptr)
		{
			m_ivsImpulseCommThread->quitAndWait();
			delete m_ivsImpulseCommThread;
			m_ivsImpulseCommThread = nullptr;
		}

		if (m_appDataServiceClientThread != nullptr)
		{
			m_appDataServiceClientThread->quitAndWait();
			delete m_appDataServiceClientThread;
			m_appDataServiceClientThread = nullptr;
		}
	}

	bool IvsImpulseHandler::init()
	{
		m_lists.clear();
		m_states.clear();

		int signalsCount = m_gateway->signalsCount();

		m_states.reserve(signalsCount);

		const SignalLists& lists = m_gateway->signalLists();

		int signalStateIndex = 0;

		for(SignalListShared sl : lists)
		{
			TEST_PTR_CONTINUE(sl);

			IvsImpulseSignalListShared ivsList = std::dynamic_pointer_cast<IvsImpulseSignalList>(sl);

			TEST_PTR_CONTINUE(ivsList);

			IvsImpulseListInfoShared li = std::make_shared<IvsImpulseListInfo>();

			li->info = ivsList;
			li->startIndex = signalStateIndex;

			const auto& ids = ivsList->signalIDs();

			// signal index in list, numbered from 1
			//
			int listIndex = 1;

			for(const QString& id : ids)
			{
				const AppSignal* s = m_appSignals.getSignalByID(id);

				if (s != nullptr)
				{
					Hash h = calcHash(id);

					AppSignalState& newState = m_states.emplace_back(h, ivsList->sendEvents());
					newState.setListIndex(listIndex);

					auto it = li->hashToListIndexes.find(h);

					if (it == li->hashToListIndexes.end())
					{
						li->hashToListIndexes.insert({h, {listIndex}});
					}
					else
					{
						it->second.push_back(listIndex);
					}
				}
				else
				{
					m_states.emplace_back(0, false);		// init as NOT workable
				}

				listIndex++;

				signalStateIndex++;
			}

			li->size = signalStateIndex - li->startIndex;

			m_lists.push_back(li);
		}

		//

		for(IvsImpulseListInfoShared& list : m_lists)
		{
			const auto& ids = list->info->signalIDs();

			for(const QString& id : ids)
			{
				Hash h = calcHash(id);

				auto it = m_hashToLists.find(h);

				if (it == m_hashToLists.end())
				{
					auto p = m_hashToLists.emplace(h, std::set<IvsImpulseListInfoShared>());

					it = p.first;
				}

				it->second.insert(list);
			}
		}

		return true;
	}
}
