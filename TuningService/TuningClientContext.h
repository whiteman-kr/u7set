#pragma once

#include "../OnlineLib/SoftwareSettings.h"
#include "TuningSourceThread.h"

namespace Tuning
{

	// ----------------------------------------------------------------------------------------------
	//
	// TuningSourceContext class declaration
	//
	// ----------------------------------------------------------------------------------------------

/*	class TuningSourceContext
	{
	public:
		TuningSourceContext(const QString& sourceID, const TuningSource* source);

		void getSourceInfo(Network::DataSourceInfo* si) const;
		void getSourceState(Network::TuningSourceState* tss) const;

		void setSourceThread(TuningSourceThread* thread);
		void removeSourceThread(TuningSourceThread* thread);

		void readSignalState(Network::TuningSignalState* tss);

		NetworkError writeSignalState(	const QString& clientEquipmentID,
										const QString& user,
										Hash signalHash,
										const TuningValue& newValue);

		NetworkError applySignalStates(	const QString& clientEquipmentID,
										const QString& user);

	private:
		QString m_sourceID;			// Tuning source (LM) equipmentID
		TuningSourceThread* m_sourceThread = nullptr;

		Network::DataSourceInfo m_sourceInfo;
		Network::TuningSourceState m_sourceState;
	};
*/

	// ----------------------------------------------------------------------------------------------
	//
	// TuningClientContext class declaration
	//
	// ----------------------------------------------------------------------------------------------

	class TuningClientContext
	{
	public:
		TuningClientContext(const QString& clientID,
							const QStringList& drivenSourcesIDs,
							const TuningSources& sources);
		~TuningClientContext();

		void readSignalStates(const Network::TuningSignalsRead& request, Network::TuningSignalsReadReply* reply) const;

		void writeSignalStates(const QString& clientEquipmentID,
							   const QString &user,
							   const Network::TuningSignalsWrite& request,
							   Network::TuningSignalsWriteReply* reply) const;

		void applySignalStates(const QString& clientEquipmentID,
							   const QString &user) const;

		void setSourceThread(TuningSourceThreadShared srcThread);
		void removeSourceThread(const QString& tuningSourceID);

	private:
		TuningSourceThreadShared getSourceThread(const QString& sourceID) const;
		std::pair<bool, TuningSourceThreadShared> getSourceThreadBySignalHash(Hash signalHash) const;

		void readSignalState(Network::TuningSignalState* tss) const;

		void clear();

	private:
		QString m_clientID;			// TuningClient equipmentID
		const TuningSources& m_tuningSources;

		std::map<QString, TuningSourceThreadShared> m_sourceThreadMap;	// source EquipmentID => TuningSourceThreadShared
		std::map<Hash, QString> m_signalToSourceIdMap;					// signal Hash => source EquipmentID
	};


	// ----------------------------------------------------------------------------------------------
	//
	// TuningClientContextMap class declaration
	//
	// ----------------------------------------------------------------------------------------------

	class TuningClientContextMap : public QHash<QString, TuningClientContext*>
	{
	public:
		TuningClientContextMap();
		~TuningClientContextMap();

		void init(const TuningServiceSettings& tss, const TuningSources& sources);

		TuningClientContext *getClientContext(QString clientID) const;

		void clear();
	};
}
