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

		const Network::DataSourceInfo& sourceInfo() const;
		Network::TuningSourceState& sourceState();

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

	private:
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

	class TuningClientContextMap
	{
	public:
		TuningClientContextMap();
		~TuningClientContextMap();

		void init(const TuningServiceSettings& tss, const TuningSources& sources);

		const TuningClientContext* getClientContext(QString clientID) const;

		void clear();

	private:
		QHash<QString, TuningClientContext*> m_clientContextMap;
	};
}
