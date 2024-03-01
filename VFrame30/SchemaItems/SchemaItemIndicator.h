#pragma once

#include "../FontParam.h"
#include "../IMatsSchemaItemAssociations.h"
#include "Indicator.h"
#include "PosRectImpl.h"

#include "../../TrendView/Trend.h"


class AppSignalState;
class AppSignalParam;
class TuningSignalState;


namespace VFrame30
{
	class Indicator;

	//
	// SchemaItemIndicator
	//
	class SchemaItemIndicator : public PosRectImpl, public IMatsSchemaItemAssociations
	{
		Q_OBJECT

	public:
		SchemaItemIndicator(void);
		explicit SchemaItemIndicator(SchemaUnit unit);
		virtual ~SchemaItemIndicator(void) = default;

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:
		virtual void draw(CDrawParam* drawParam) const override;

		// Trend functions, only if IndicatorType == E::IndicatorType::Trend
		//
	public:
		bool isTrend() const;

		TrendLib::Trend& trend();
		const TrendLib::Trend& trend() const;

		E::RtTrendsSamplePeriod trendSamplePeriod() const;
		E::TimeType trendTimeType() const;
		int trendDurationSeconds() const;

		// IMatsSchemaItemAssociations implementation.
		//
	public:
		virtual QStringList associatedDiagObjectIds() const override { return {}; };
		virtual QStringList associatedAppSignalIds() const override;
		virtual QStringList associatedImpactAppSignalIds() const override;
		virtual QStringList associatedConnectionIds() const override;
		virtual QStringList associatedLoopbackIds() const override;
		virtual QStringList associatedSchemaItemLabels() const override;

		// Properties and Data
		//
	public:
		QString signalIdsString() const;
		QString signalIdsString(const Context* context) const;
		void setSignalIdsString(const QString& value);

		QStringList signalIds() const;
		QStringList signalIds(const Context* context) const;
		void setSignalIds(const QStringList& value);

		E::IndicatorType indicatorType() const;
		void setIndicatorType(E::IndicatorType value);

		// --
		//
		using IndicatorObjectPtr = std::unique_ptr<Indicator>;
		static const int MaxSignalsCound = 12;

	private slots:
		void updateIndicatorProperties();

	private:
		template<typename IndicatorType = VFrame30::Indicator>
		IndicatorType* indicatorObject();

		template<typename IndicatorType = VFrame30::Indicator>
		const IndicatorType* indicatorObject() const;

	private:
		QStringList m_signalIds = {"#APPSIGNALID"};

		E::IndicatorType m_indicatorType = E::IndicatorType::HistogramVert;

		// !!!
		// Do not remove any items (even obsolete) from the next array, do not change its order,
		// only add new items after adding them to E::IndicatorType
		// !!!
		std::array<IndicatorObjectPtr, E::IndicatorTypeCount> m_indicatorObjects;
	};


	template<typename IndicatorType /* = VFrame30::Indicator*/>
	IndicatorType* SchemaItemIndicator::indicatorObject()
	{
		size_t index = static_cast<size_t>(m_indicatorType);

		if (index >= m_indicatorObjects.size())
		{
			Q_ASSERT(index < m_indicatorObjects.size());
			index = 0;
		}

		return dynamic_cast<IndicatorType*>(m_indicatorObjects[index].get());
	}

	template<typename IndicatorType /* = VFrame30::Indicator*/>
	const IndicatorType* SchemaItemIndicator::indicatorObject() const
	{
		size_t index = static_cast<size_t>(m_indicatorType);

		if (index >= m_indicatorObjects.size())
		{
			Q_ASSERT(index < m_indicatorObjects.size());
			index = 0;
		}

		return dynamic_cast<const IndicatorType*>(m_indicatorObjects[index].get());
	}

} // namespace VFrame30
