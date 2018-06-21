#pragma once

#include "../lib/ServiceSettings.h"
#include "TuningSourceThread.h"

namespace Tuning
{

	// ----------------------------------------------------------------------------------------------
	//
	// TuningSourceContext class declaration
	//
	// ----------------------------------------------------------------------------------------------

	class TuningSourceContext
	{
	public:
		TuningSourceContext(const QString& sourceID, const TuningSource* source);

		void getSourceInfo(Network::DataSourceInfo& si) const;
		void getSourceState(Network::TuningSourceState& tss) const;

		void setSourceHandler(TuningSourceHandler* handler);
		void removeSourceHandler(TuningSourceHandler* handler);

		void readSignalState(Network::TuningSignalState* tss);

		NetworkError writeSignalState(	const QString& clientEquipmentID,
										const QString& user,
										Hash signalHash,
										const TuningValue& newValue);

		NetworkError applySignalStates(	const QString& clientEquipmentID,
										const QString& user);

	private:
		QString m_sourceID;			// Tuning source (LM) equipmentID
		TuningSourceHandler* m_sourceHandler = nullptr;

		Network::DataSourceInfo m_sourceInfo;
		Network::TuningSourceState m_sourceState;
	};


	// ----------------------------------------------------------------------------------------------
	//
	// TuningClientContext class declaration
	//
	// ----------------------------------------------------------------------------------------------

	class TuningClientContext
	{
	public:
		TuningClientContext(const QString& clientID, const QStringList& sourcesIDs, const TuningSources& sources);
		~TuningClientContext();

		void getSourcesInfo(QVector<Network::DataSourceInfo>& dataSourcesInfo) const;
		void getSourcesStates(QVector<Network::TuningSourceState>& tuningSourcesStates) const;

		void readSignalStates(const Network::TuningSignalsRead& request, Network::TuningSignalsReadReply* reply) const;

		void writeSignalStates(const QString& clientEquipmentID,
							   const QString &user,
							   const Network::TuningSignalsWrite& request,
							   Network::TuningSignalsWriteReply* reply) const;

		void applySignalStates(const QString& clientEquipmentID,
							   const QString &user) const;

		void setSourceHandler(TuningSourceHandler* handler);
		void removeSourceHandler(TuningSourceHandler* handler);

	private:
		TuningSourceContext* getSourceContext(const QString& sourceID) const;
		TuningSourceContext* getSourceContextBySignalHash(Hash signalHash) const;

		void readSignalState(Network::TuningSignalState* tss) const;

		void clear();

	private:
		QString m_clientID;			// TuningClient equipmentID

		QHash<QString, TuningSourceContext*> m_sourceContextMap;

		QHash<Hash, TuningSourceContext*> m_signalToSourceMap;
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
