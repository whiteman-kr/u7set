#pragma once
#include "Indicator.h"
#include "../CommonLib/Times.h"
#include "../TrendView/Trend.h"

namespace VFrame30
{
	class IRealTimeTrendSource;

	//
	// IndicatorTrendSignalParam
	//
	class IndicatorTrendSignalParam : public PropertyObject
	{
		Q_OBJECT

	public:
		IndicatorTrendSignalParam() = default;
		IndicatorTrendSignalParam(const IndicatorTrendSignalParam&) = default;
		virtual ~IndicatorTrendSignalParam() = default;

		IndicatorTrendSignalParam& operator=(const IndicatorTrendSignalParam&) noexcept = default;

	public:
		virtual void propertyDemand(const QString&) override;

		bool save(Proto::IndicatorTrendSignalParam* message) const;
		bool load(const Proto::IndicatorTrendSignalParam& message);

		void initTrensSignalParam(TrendLib::TrendSignalParam* trendSignalParam) const;

	public slots:
		QColor color() const;
		void setColor(const QColor& value);

		int lineWeight() const;
		void setLineWeight(int value);

		double lowLimit() const;
		void setLowLimit(double value);

		double highLimit() const;
		void setHighLimit(double value);

	private:
		QColor m_color = Qt::darkBlue;
		int m_lineWeight = 1;

		double m_lowLimit = 0.0;
		double m_highLimit = 100.0;
	};

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

		virtual void draw(CDrawParam* drawParam, const SchemaItemIndicator* schemaItem) const override;

		// Getting setting data, client functions
		//
	public:
		TrendLib::Trend& trend();
		const TrendLib::Trend& trend() const;

		// Properties
		//
	public:
		E::TrendViewMode viewMode() const;
		void setViewMode(E::TrendViewMode value);

		E::TrendScaleType scaleType() const;
		void setScaleType(E::TrendScaleType value);

		int laneCount() const;
		void setLaneCount(int value);

		QColor backColor1st() const;
		void setBackColor1st(const QColor& value);

		QColor backColor2nd() const;
		void setBackColor2nd(const QColor& value);

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

		PropertyVector<IndicatorTrendSignalParam> m_trendSignalParams;

		mutable TrendLib::TrendParam m_trendParam;

		//  --
		mutable TrendLib::Trend m_trend;

		mutable QImage m_image;
		mutable QElapsedTimer m_drawTimer;

		mutable QElapsedTimer m_updateSignalsTimer;		// We need to update signal params, as program can start without
														// AppDataServices, and when connections is established the signals
														// should be updates. There is no suitable way to do it now,
														// so we just use update time and it will update signals every 10 secs.
	};
}

Q_DECLARE_METATYPE(VFrame30::IndicatorTrendSignalParam)
Q_DECLARE_METATYPE(PropertyVector<VFrame30::IndicatorTrendSignalParam>)
