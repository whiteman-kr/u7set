#pragma once
#include <TrendView/Trend.h>
#include <TrendView/TrendSignal.h>
#include <VFrame30/Indicator.h>

#include <mutex>


namespace VFrame30
{
	class IRealTimeTrendSource;

	/*! \class IndicatorTrendSignalParam
		\ingroup dynamicSchemaItems
		\brief Per-signal trend display parameters.

		Stores visual limits and line appearance for a single signal trend in the
		trend indicator.
	*/
	class IndicatorTrendSignalParam : public PropertyObject
	{
		Q_OBJECT

		/// \brief Gets or sets signal color.
		Q_PROPERTY(QColor color READ color WRITE setColor)
		Q_PROPERTY(QColor Color READ color WRITE setColor)

		/// \brief Gets or sets signal line weight.
		Q_PROPERTY(int lineWeight READ lineWeight WRITE setLineWeight)
		Q_PROPERTY(int LineWeight READ lineWeight WRITE setLineWeight)

		/// \brief Gets or sets signal analog format.
		Q_PROPERTY(E::AnalogFormat analogFormat READ analogFormat WRITE setAnalogFormat)
		Q_PROPERTY(E::AnalogFormat AnalogFormat READ analogFormat WRITE setAnalogFormat)

		/// \brief Gets or sets signal precision (number of decimal places).
		Q_PROPERTY(int precision READ precision WRITE setPrecision)
		Q_PROPERTY(int Precision READ precision WRITE setPrecision)

		/// \brief Gets or sets lower signal limit.
		Q_PROPERTY(double lowLimit READ lowLimit WRITE setLowLimit)
		Q_PROPERTY(double LowLimit READ lowLimit WRITE setLowLimit)

		/// \brief Gets or sets upper signal limit.
		Q_PROPERTY(double highLimit READ highLimit WRITE setHighLimit)
		Q_PROPERTY(double HighLimit READ highLimit WRITE setHighLimit)

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

	public:
		[[nodiscard]] QColor color() const;
		void setColor(const QColor& value);

		[[nodiscard]] int lineWeight() const;
		void setLineWeight(int value);

		[[nodiscard]] E::AnalogFormat analogFormat() const;
		void setAnalogFormat(E::AnalogFormat value);

		[[nodiscard]] int precision() const;
		void setPrecision(int value);

		[[nodiscard]] double lowLimit() const;
		void setLowLimit(double value);

		[[nodiscard]] double highLimit() const;
		void setHighLimit(double value);


	private:
		QColor m_color = Qt::darkBlue;
		int m_lineWeight = 1;

		E::AnalogFormat m_analogFormat = E::AnalogFormat::f_9;
		int m_precision = 0;

		double m_lowLimit = 0.0;
		double m_highLimit = 100.0;
	};

	/*! \class IndicatorTrend
		\ingroup dynamicSchemaItems
		\brief Trend indicator for drawing real-time and historical signal trends.

		Provides trend visualization settings, signal presentation options and
		serialization support for schema usage.
	*/
	class IndicatorTrend : public Indicator
	{
		Q_OBJECT
		Q_DISABLE_COPY_MOVE(IndicatorTrend)

		/// \brief Gets or sets trend view mode.
		Q_PROPERTY(E::TrendViewMode viewMode READ viewMode WRITE setViewMode)
		Q_PROPERTY(E::TrendViewMode ViewMode READ viewMode WRITE setViewMode)

		/// \brief Gets or sets trend scale type.
		Q_PROPERTY(E::TrendScaleType scaleType READ scaleType WRITE setScaleType)
		Q_PROPERTY(E::TrendScaleType ScaleType READ scaleType WRITE setScaleType)

		/// \brief Gets or sets trend lane count.
		Q_PROPERTY(int laneCount READ laneCount WRITE setLaneCount)
		Q_PROPERTY(int LaneCount READ laneCount WRITE setLaneCount)

		/// \brief Gets or sets first background color.
		Q_PROPERTY(QColor backColor1st READ backColor1st WRITE setBackColor1st)
		Q_PROPERTY(QColor BackColor1st READ backColor1st WRITE setBackColor1st)

		/// \brief Gets or sets second background color.
		Q_PROPERTY(QColor backColor2nd READ backColor2nd WRITE setBackColor2nd)
		Q_PROPERTY(QColor BackColor2nd READ backColor2nd WRITE setBackColor2nd)

		/// \brief Gets or sets visibility of signal IDs.
		Q_PROPERTY(bool showSignalIds READ showSignalIds WRITE setShowSignalIds)
		Q_PROPERTY(bool ShowSignalIds READ showSignalIds WRITE setShowSignalIds)
		Q_PROPERTY(bool showSignalIDs READ showSignalIds WRITE setShowSignalIds)
		Q_PROPERTY(bool ShowSignalIDs READ showSignalIds WRITE setShowSignalIds)

		/// \brief Gets or sets visibility of signal captions.
		Q_PROPERTY(bool showSignalCaptions READ showSignalCaptions WRITE setShowSignalCaptions)
		Q_PROPERTY(bool ShowSignalCaptions READ showSignalCaptions WRITE setShowSignalCaptions)

		/// \brief Gets or sets visibility of signal scales.
		Q_PROPERTY(bool showSignalScales READ showSignalScales WRITE setShowSignalScales)
		Q_PROPERTY(bool ShowSignalScales READ showSignalScales WRITE setShowSignalScales)

		/// \brief Gets or sets visibility of the time labels on the time axis.
		Q_PROPERTY(bool showTimeLabels READ showTimeLabels WRITE setShowTimeLabels)
		Q_PROPERTY(bool ShowTimeLabels READ showTimeLabels WRITE setShowTimeLabels)

		/// \brief Gets or sets visibility of the date labels on the time axis.
		Q_PROPERTY(bool showDateLabels READ showDateLabels WRITE setShowDateLabels)
		Q_PROPERTY(bool ShowDateLabels READ showDateLabels WRITE setShowDateLabels)

		/// \brief Gets or sets left indent (if -1, then default indent is used).
		Q_PROPERTY(double indentLeft READ indentLeft WRITE setIndentLeft)
		Q_PROPERTY(double IndentLeft READ indentLeft WRITE setIndentLeft)

		/// \brief Gets or sets right indent (if -1, then default indent is used).
		Q_PROPERTY(double indentRight READ indentRight WRITE setIndentRight)
		Q_PROPERTY(double IndentRight READ indentRight WRITE setIndentRight)

		/// \brief Gets or sets top indent (if -1, then default indent is used).
		Q_PROPERTY(double indentTop READ indentTop WRITE setIndentTop)
		Q_PROPERTY(double IndentTop READ indentTop WRITE setIndentTop)

		/// \brief Gets or sets bottom indent (if -1, then default indent is used).
		Q_PROPERTY(double indentBottom READ indentBottom WRITE setIndentBottom)
		Q_PROPERTY(double IndentBottom READ indentBottom WRITE setIndentBottom)

		/// \brief Gets or sets redraw interval in milliseconds.
		Q_PROPERTY(int redrawInterval READ redrawInterval WRITE setRedrawInterval)
		Q_PROPERTY(int RedrawInterval READ redrawInterval WRITE setRedrawInterval)

		/// \brief Gets or sets lane duration in seconds.
		Q_PROPERTY(int durationSeconds READ durationSeconds WRITE setDurationSeconds)
		Q_PROPERTY(int DurationSeconds READ durationSeconds WRITE setDurationSeconds)

		/// \brief Gets list of signal parameters (type IndicatorTrendSignalParam) for each signal in the trend.
		Q_PROPERTY(QList<IndicatorTrendSignalParam*> trendSignalParams READ jsTrendSignalParams)
		Q_PROPERTY(QList<IndicatorTrendSignalParam*> TrendSignalParams READ jsTrendSignalParams)

	public:
		IndicatorTrend() = delete;
		explicit IndicatorTrend(SchemaUnit itemUnit);
		virtual ~IndicatorTrend();

	public:
		virtual void createProperties(SchemaItemIndicator* propertyObject, int signalCount) override;

		virtual bool load(const Proto::SchemaItemIndicator& message, SchemaUnit unit) override;
		virtual bool save(Proto::SchemaItemIndicator* message) const override;

		virtual void draw(CDrawParam* drawParam, const SchemaItemIndicator* schemaItem) const override;

	private:
		// Saves rendered image in m_image and m_drawFuture.
		// It is const because it called in draw(), but modifies m_image and m_drawFuture.
		//
		void saveRenderedImage(QFuture<QImage>& future) const;

		// Getting setting data, client functions
		//
	public:
		TrendLib::Trend& trend();
		const TrendLib::Trend& trend() const;

		/// \brief Forcibly redraw, ignoring the cached image.
		Q_INVOKABLE void forceRedraw();

		// Properties
		//
	public:
		E::TrendViewMode viewMode() const;
		void setViewMode(E::TrendViewMode value);

		E::TrendScaleType scaleType() const;
		void setScaleType(E::TrendScaleType value);

		int laneCount() const;
		void setLaneCount(int value);

		// Appearance properties
		//
		QColor backColor1st() const;
		void setBackColor1st(const QColor& value);

		QColor backColor2nd() const;
		void setBackColor2nd(const QColor& value);

		bool showSignalIds() const;
		void setShowSignalIds(bool value);

		bool showSignalCaptions() const;
		void setShowSignalCaptions(bool value);

		bool showSignalScales() const;
		void setShowSignalScales(bool value);

		bool showTimeLabels() const;
		void setShowTimeLabels(bool value);

		bool showDateLabels() const;
		void setShowDateLabels(bool value);

		// Indent properties
		// Gets and sets indents in native units (mm/in/pixels).
		//
		[[nodiscard]] double indentLeft() const;
		void setIndentLeft(double value);

		[[nodiscard]] double indentRight() const;
		void setIndentRight(double value);

		[[nodiscard]] double indentTop() const;
		void setIndentTop(double value);

		[[nodiscard]] double indentBottom() const;
		void setIndentBottom(double value);

		// Time properties
		//
		E::RtTrendsSamplePeriod samplePeriod() const;
		void setSamplePeriod(E::RtTrendsSamplePeriod value);

		E::TimeType timeType() const;
		void setTimeType(E::TimeType value);

		int redrawInterval() const;
		void setRedrawInterval(int value);

		int durationSeconds() const;
		void setDurationSeconds(int value);

	private:
		// Use this getter only from JS, because it sets ownership of returned pointers to JS, so they will be automatically deleted when JS
		// object is deleted.
		//
		QList<VFrame30::IndicatorTrendSignalParam*> jsTrendSignalParams() const;

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

		mutable std::atomic<bool> m_forceRedraw{false}; // Forcibly redraw, ignoring the cached image.
		mutable QImage m_image;                         // Image currently shown

		mutable QFuture<QImage> m_drawFuture;
		mutable QFutureWatcher<QImage> m_futureWatcher;
		mutable std::stop_source m_drawStopSource;

		mutable QElapsedTimer m_drawTimer;


		mutable QElapsedTimer m_updateSignalsTimer; // We need to update signal params, as program can start without
													// AppDataServices, and when connections is established the signals
													// should be updates. There is no suitable way to do it now,
													// so we just use update time and it will update signals every 10 secs.
	};
} // namespace VFrame30

Q_DECLARE_METATYPE(VFrame30::IndicatorTrendSignalParam)
Q_DECLARE_METATYPE(PropertyVector<VFrame30::IndicatorTrendSignalParam>)
Q_DECLARE_METATYPE(QList<VFrame30::IndicatorTrendSignalParam*>)