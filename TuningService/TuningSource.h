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

		virtual void writeAdditionalSectionsToXml(XmlWriteHelper& xml) override;
		virtual bool readAdditionalSectionsFromXml(XmlReadHelper& xml) override;

	private:
		TuningData* m_tuningData = nullptr;

		bool m_deleteTuningData = false;
	};


	class TuningSources : public QHash<QString, TuningSource*>
	{
	public:
		~TuningSources();

		void clear();

		void buildIP2DataSourceMap();

		const TuningSource* getSourceByIP(quint32 ip) const;
		const TuningSource *getSourceByID(const QString& sourceID) const;

	private:
		QHash<quint32, TuningSource*> m_ip2Source;
	};
}
