#pragma once

#include "../lib/DataSource.h"
#include "TuningDataStorage.h"

namespace Tuning
{

	class TuningSource : public DataSource
	{
	private:
		TuningData* m_tuningData = nullptr;

		bool m_deleteTuningData = false;

	public:
		TuningSource();
		~TuningSource();

		void setTuningData(TuningData* tuningData);

		virtual void writeAdditionalSectionsToXml(XmlWriteHelper& xml) override;
		virtual bool readAdditionalSectionsFromXml(XmlReadHelper& xml) override;

		quint64 uniqueID() const;
	};


	class TuningSources : public QHash<QString, TuningSource*>
	{
		QHash<quint32, TuningSource*> m_ip2Source;

	public:
		~TuningSources();

		void clear();

//		void getTuningDataSourcesInfo(QVector<TuningSourceInfo>& info);

		void buildIP2DataSourceMap();

		TuningSource* getDataSourceByIP(quint32 ip);
	};

}
