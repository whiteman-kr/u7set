#pragma once

#include "../OnlineLib/SoftwareSettings.h"
#include "TuningSourceThread.h"

namespace Tuning
{
	// ----------------------------------------------------------------------------------------------
	//
	// TuningClientContext class declaration
	//
	// ----------------------------------------------------------------------------------------------

	class TuningClientContext
	{
	public:
		TuningClientContext(const QString& clientID,
							bool tuningLogin,
							const QString& matsUsersList,
							const std::vector<OnlineLib::MatsUser>& matsUsers,
							const QStringList& drivenSourcesIDs,
							const TuningSources& sources);
		~TuningClientContext();

		void readSignalStates(const Network::TuningSignalsRead& request, Network::TuningSignalsReadReply* reply) const;

		void writeSignalStates(const QString& clientEquipmentID,
							   const QString& matsUser,
							   const Network::TuningSignalsWrite& request,
							   Network::TuningSignalsWriteReply* reply) const;

		void applySignalStates(const QString& clientEquipmentID,
							   const QString& matsUser) const;

		void setSourceThread(TuningSourceThreadShared srcThread);
		void removeSourceThread(const QString& tuningSourceID);

		void registerStateChangesQueue(qint64 tcpConnectionID);
		void unregisterStateChangesQueue(qint64 tcpConnectionID);

		void pushSignalStateChange(const TuningSignal::State& state);

		TuningSignalsChangesQueue* getSignalChangesQueue(qint64 tcpConnectionID);

		const std::map<Hash, QString>& signalToSourceIdMap() const;

	private:
		TuningSourceThreadShared getSourceThread(const QString& sourceID) const;
		std::pair<bool, TuningSourceThreadShared> getSourceThreadBySignalHash(Hash signalHash) const;

		void readSignalState(Network::TuningSignalState* tss) const;

		void clear();

		int getStateChangesQueueSize() const;

	private:
		QString m_clientID;										// Tuning сlient EquipmentID
		bool m_tuningLogin = false;
		std::map<QString, std::set<QString>> m_matsUsers;		// MATS user login => MATS user set of AppSignalTags
		std::map<QString, std::set<Hash>> m_userAllowedSignals;	// MATS user login => set of allowed to control signal Hashes
		std::set<QString> m_disabledUsers;

		const TuningSources& m_tuningSources;

		std::map<QString, TuningSourceThreadShared> m_sourceThreadMap;	// source EquipmentID => TuningSourceThreadShared
		std::map<Hash, QString> m_signalToSourceIdMap;					// signal Hash => source EquipmentID

		//

		SpinLock m_queueMapMutex;
		std::map<qint64, TuningSignalsChangesQueue*> m_stateChangesQueueMap;		// client tcpConnectionID => state changes queue
	};

	// ----------------------------------------------------------------------------------------------
	//
	// TuningClientContextMap class declaration
	//
	// ----------------------------------------------------------------------------------------------

	class TuningClientContextMap
	{
	public:
		TuningClientContextMap();
		~TuningClientContextMap();

		void init(const TuningServiceSettings& tss, const TuningSources& sources);

		TuningClientContext* getClientContext(const QString& clientEquipmentID) const;
		void getAllClientContexts(QVector<const TuningClientContext*>& clientContexts) const;

		void setSourceThreadInTuningClientContexts(TuningSourceThreadShared thread);
		void removeSourceThreadFromTuningClientContexts(const QString& tuningSourceID);

		void clear();

		void pushSignalStateChange(const TuningSignal::State& state);

	private:
		std::map<QString, TuningClientContext*> m_clientsContextMap;		// clientEquipmentID => TuningClientContext*
		std::map<Hash, std::set<TuningClientContext*>> m_signalToClientContextMap;	// calcHash(AppSignalID) => set of TuningClientContext*
	};
}
