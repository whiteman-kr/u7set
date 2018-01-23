#pragma once

#include "../lib/ServiceSettings.h"
#include "TuningSourceWorker.h"

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

		void setSourceWorker(TuningSourceWorker* worker);

		void readSignalState(Network::TuningSignalState& tss);
		void writeSignalState(Hash signalHash, const TuningValue& newValue, Network::TuningSignalWriteResult* writeResult);
		void applySignalStates();

	private:
		QString m_sourceID;			// Tuning source (LM) equipmentID
		TuningSourceWorker* m_sourceWorker = nullptr;

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

		void readSignalStates(const Network::TuningSignalsRead& request, Network::TuningSignalsReadReply& reply) const;
		void writeSignalStates(const Network::TuningSignalsWrite& request, Network::TuningSignalsWriteReply& reply) const;
		void applySignalStates() const;

		void setSourceWorker(const QString& sourceID, TuningSourceWorker* worker);

	private:
		TuningSourceContext* getSourceContext(const QString& sourceID) const;
		TuningSourceContext* getSourceContextBySignalHash(Hash signalHash) const;

		void readSignalState(Network::TuningSignalState& tss) const;

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
