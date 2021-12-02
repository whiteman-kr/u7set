#pragma once
#include "Indicator.h"
#include "../CommonLib/Times.h"
#include "../TrendView/Trend.h"

namespace VFrame30
{
	class IRealTimeTrendSource;

	//
	// Trend as indicator
	//
	class IndicatorTrend : public Indicator
	{
		Q_OBJECT

	public:
		IndicatorTrend() = delete;
		explicit IndicatorTrend(SchemaUnit itemUnit);
		virtual ~IndicatorTrend() = default;

	public:
		virtual void createProperties(SchemaItemIndicator* propertyObject, int signalCount) override;

		virtual bool load(const Proto::SchemaItemIndicator& message, SchemaUnit unit) override;
		virtual bool save(Proto::SchemaItemIndicator* message) const override;

		virtual void draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer, const SchemaItemIndicator* schemaItem) const override;

		// Getting setting data, client functions
		//
	public:
		TrendLib::Trend& trend();
		const TrendLib::Trend& trend() const;

		// Properties
		//
	public:
		E::RtTrendsSamplePeriod samplePeriod() const;
		void setSamplePeriod(E::RtTrendsSamplePeriod value);

		E::TimeType timeType() const;
		void setTimeType(E::TimeType value);

		int redrawInterval() const;
		void setRedrawInterval(int value);

		int durationSeconds() const;
		void setDurationSeconds(int value);

		// Data
		//
	private:
		E::RtTrendsSamplePeriod m_samplePeriod = E::RtTrendsSamplePeriod::sp_5s;
		E::TimeType m_timeType = E::TimeType::Local;
		qint64 m_redrawInterval = 1_sec;
		mutable TrendLib::TrendParam m_trendParam;

		//  --
		mutable TrendLib::Trend m_trend;

		mutable QImage m_image;
		mutable QElapsedTimer m_drawTimer;
	};

}
