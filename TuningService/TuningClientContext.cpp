#include "TuningClientContext.h"

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


	void TuningSourceContext::getSourceInfo(Network::DataSourceInfo& si) const
	{
		si = m_sourceInfo;
	}


	void TuningSourceContext::getSourceState(Network::TuningSourceState& tss) const
	{
		if (m_sourceWorker == nullptr)
		{
			assert(false);
		}
		else
		{
			m_sourceWorker->getState(tss);
		}
	}


	void TuningSourceContext::setSourceWorker(TuningSourceWorker* worker)
	{
		if (worker == nullptr)
		{
			assert(false);
			return;
		}

		if (worker->sourceEquipmentID() != m_sourceID)
		{
			assert(false);
			return;
		}

		m_sourceWorker = worker;
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

			sourceContext->getSourceInfo(dataSourcesInfo[count]);

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

			sourceContext->getSourceState(tuningSourcesStates[count]);

			count++;
		}

		if (count < tuningSourcesStates.size())
		{
			tuningSourcesStates.resize(count);
		}
	}


	void TuningClientContext::setSourceWorker(const QString& sourceID, TuningSourceWorker *worker)
	{
		TuningSourceContext* sourceContext = getSourceContext(sourceID);

		if (sourceContext == nullptr)
		{
			return;			// its OK
		}

		sourceContext->setSourceWorker(worker);
	}


	TuningSourceContext* TuningClientContext::getSourceContext(const QString& sourceID)
	{
		return m_sourceContextMap.value(sourceID, nullptr);
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
			if (contains(client.equipmentID))
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
