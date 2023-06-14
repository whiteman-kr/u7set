#pragma once

#include <optional>

#include "../lib/DataSource.h"
#include "../lib/TuningDataStorage.h"

namespace Tuning
{
	class TuningSource : public DataSource
	{
	public:
		TuningSource();
		~TuningSource();

		void setTuningData(TuningDataShared tuningData);
		TuningDataSharedConst tuningData() const;

		virtual void writeAdditionalSectionsToXml(XmlWriteHelper& xml) const override;
		virtual bool readAdditionalSectionsFromXml(XmlReadHelper& xml) override;

		bool hasTuningSignals() const;

		const QStringList& getEnabledLansProvidedTuning() const;

		int getSignalsCount() const;

	private:
		TuningDataShared m_tuningData;

		mutable std::optional<QStringList> m_enabledLansProvidedTuning;
	};

	class TuningSources : public QVector<TuningSource>
	{
	public:
		~TuningSources();

		void clear();
		void buildMaps();

		const TuningSource* getSourceByID(const QString& sourceID) const;
		QStringList getAllSourcesIDs() const;

		int getSignalsCount() const;

	private:
		QHash<QString, int> m_id2Source;
	};
}
