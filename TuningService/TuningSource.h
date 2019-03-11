#pragma once

#include "../lib/DataSource.h"
#include "TuningDataStorage.h"

namespace Tuning
{

	class TuningSource : public DataSource
	{
	public:
		TuningSource();
		~TuningSource();

		void setTuningData(TuningData* tuningData);
		const TuningData* tuningData() const;

		virtual void writeAdditionalSectionsToXml(XmlWriteHelper& xml) const override;
		virtual bool readAdditionalSectionsFromXml(XmlReadHelper& xml) override;

		bool hasTuningSignals() const;

	private:
		TuningData* m_tuningData = nullptr;

		bool m_deleteTuningData = false;
	};


	class TuningSources : public QVector<TuningSource>
	{
	public:
		~TuningSources();

		void clear();

		void buildMaps();

		const TuningSource* getSourceByIP(quint32 ip) const;
		const TuningSource* getSourceByID(const QString& sourceID) const;

	private:
		QHash<quint32, int> m_ip2Source;
		QHash<QString, int> m_id2Source;
	};
}
