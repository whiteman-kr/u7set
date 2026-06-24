#include <TrendView/TrendParam.h>

namespace TrendLib
{
	TrendParam::TrendParam() {}

	TrendParam::TrendParam(ITrendDataProvider* dataProvider) :
		m_dataProvider(dataProvider)
	{
	}

	bool TrendParam::save(::Proto::TrendParam* message) const
	{
		if (message == nullptr)
		{
			return false;
		}

		message->set_view_mode(static_cast<int>(m_viewMode));
		message->set_scale_type(static_cast<int>(m_scaleType));
		message->set_time_type(static_cast<int>(m_timeType));
		message->set_lane_count(m_laneCount);

		message->set_start_time(m_startTimeStamp.timeStamp);
		message->set_duration(m_duration);

		message->set_back_color_1st(m_backColor1st.rgb());
		message->set_back_color_2nd(m_backColor2nd.rgb());

		message->set_show_signal_ids(m_showSignalIds);
		message->set_show_signal_captions(m_showSignalCaptions);
		message->set_show_signal_scales(m_showSignalScales);
		message->set_show_time_labels(m_showTimeLabels);
		message->set_show_date_labels(m_showDateLabels);

		message->set_indent_left(m_indentLeft);
		message->set_indent_right(m_indentRight);
		message->set_indent_top(m_indentTop);
		message->set_indent_bottom(m_indentBottom);

		message->set_projectname(m_project.toStdString());

		return true;
	}

	bool TrendParam::load(const ::Proto::TrendParam& message)
	{
		if (message.IsInitialized() == false)
		{
			Q_ASSERT(message.IsInitialized());
			return false;
		}

		m_viewMode = static_cast<E::TrendViewMode>(message.view_mode());
		m_scaleType = static_cast<E::TrendScaleType>(message.scale_type());
		m_timeType = static_cast<E::TimeType>(message.time_type());
		m_laneCount = message.lane_count();

		m_startTimeStamp.timeStamp = message.start_time();
		m_duration = message.duration();

		m_backColor1st = QColor::fromRgb(message.back_color_1st());
		m_backColor2nd = QColor::fromRgb(message.back_color_2nd());

		m_showSignalIds = message.show_signal_ids();
		m_showSignalCaptions = message.show_signal_captions();
		m_showSignalScales = message.show_signal_scales();
		m_showTimeLabels = message.show_time_labels();
		m_showDateLabels = message.show_date_labels();

		m_indentLeft = message.indent_left();
		m_indentRight = message.indent_right();
		m_indentTop = message.indent_top();
		m_indentBottom = message.indent_bottom();

		m_project = QString::fromStdString(message.projectname());

		return true;
	}

	const QRectF& TrendParam::rectPx() const
	{
		return m_rectPx;
	}

	const QRectF& TrendParam::rectIn() const
	{
		return m_rectIn;
	}

	void TrendParam::setRectPx(const QRectF& value, double dpiX, double dpiY, double devicePixelRatio)
	{
		setDpi(dpiX, dpiY, devicePixelRatio);

		m_rectPx = value;

		m_rectIn.setLeft(value.left() / realDpiX());
		m_rectIn.setTop(value.top() / realDpiY());
		m_rectIn.setWidth(value.width() / realDpiX());
		m_rectIn.setHeight(value.height() / realDpiY());

		return;
	}

	double TrendParam::dpiX() const
	{
		return m_dpiX;
	}

	double TrendParam::dpiY() const
	{
		return m_dpiY;
	}

	double TrendParam::realDpiX() const
	{
		return m_dpiX * m_devicePixelRatio;
	}

	double TrendParam::realDpiY() const
	{
		return m_dpiY * m_devicePixelRatio;
	}

	double TrendParam::devicePixelRatio() const
	{
		return m_devicePixelRatio;
	}

	void TrendParam::setDpi(double dpiX, double dpiY, double devicePixelRatio)
	{
		m_dpiX = dpiX;
		m_dpiY = dpiY;
		m_devicePixelRatio = devicePixelRatio;

		m_cosmeticPenWidth = (m_dpiX >= 600) ? (1.0 / 128.0) : 0.0;

		return;
	}

	E::TrendViewMode TrendParam::viewMode() const
	{
		return m_viewMode;
	}

	void TrendParam::setViewMode(E::TrendViewMode value)
	{
		m_viewMode = value;
	}

	E::TrendScaleType TrendParam::scaleType() const
	{
		return m_scaleType;
	}

	void TrendParam::setScaleType(E::TrendScaleType value)
	{
		m_scaleType = value;
	}

	E::TimeType TrendParam::timeType() const
	{
		return m_timeType;
	}

	void TrendParam::setTimeType(E::TimeType value)
	{
		m_timeType = value;
	}

	int TrendParam::laneCount() const
	{
		return m_laneCount;
	}

	void TrendParam::setLaneCount(int value)
	{
		m_laneCount = value;
	}

	E::TrendMode TrendParam::trendMode() const
	{
		return m_trendMode;
	}

	void TrendParam::setTrendMode(E::TrendMode value)
	{
		m_trendMode = value;
		return;
	}

	TrendLib::ITrendDataProvider* TrendParam::trendDataProvider()
	{
		return m_dataProvider;
	}

	const TrendLib::ITrendDataProvider* TrendParam::trendDataProvider() const
	{
		return m_dataProvider;
	}

	void TrendParam::setTrendDataProvider(TrendLib::ITrendDataProvider* dataProvider)
	{
		m_dataProvider = dataProvider;
	}

	QColor TrendParam::backColor1st() const
	{
		return m_backColor1st;
	}

	void TrendParam::setBackColor1st(const QColor& value)
	{
		m_backColor1st = value;
	}

	QColor TrendParam::backColor2nd() const
	{
		return m_backColor2nd;
	}

	void TrendParam::setBackColor2nd(const QColor& value)
	{
		m_backColor2nd = value;
	}

	bool TrendParam::showSignalIds() const
	{
		return m_showSignalIds;
	}

	void TrendParam::setShowSignalIds(bool value)
	{
		m_showSignalIds = value;
	}

	bool TrendParam::showSignalCaptions() const
	{
		return m_showSignalCaptions;
	}

	void TrendParam::setShowSignalCaptions(bool value)
	{
		m_showSignalCaptions = value;
	}

	bool TrendParam::showSignalScales() const
	{
		return m_showSignalScales;
	}

	void TrendParam::setShowSignalScales(bool value)
	{
		m_showSignalScales = value;
	}

	bool TrendParam::showTimeLabels() const
	{
		return m_showTimeLabels;
	}

	void TrendParam::setShowTimeLabels(bool value)
	{
		m_showTimeLabels = value;
	}

	bool TrendParam::showDateLabels() const
	{
		return m_showDateLabels;
	}

	void TrendParam::setShowDateLabels(bool value)
	{
		m_showDateLabels = value;
	}

	double TrendParam::indentLeft() const
	{
		return m_indentLeft;
	}

	void TrendParam::setIndentLeft(double value)
	{
		m_indentLeft = value;
	}

	double TrendParam::indentRight() const
	{
		return m_indentRight;
	}

	void TrendParam::setIndentRight(double value)
	{
		m_indentRight = value;
	}

	double TrendParam::indentTop() const
	{
		return m_indentTop;
	}
	void TrendParam::setIndentTop(double value)
	{
		m_indentTop = value;
	}

	double TrendParam::indentBottom() const
	{
		return m_indentBottom;
	}

	void TrendParam::setIndentBottom(double value)
	{
		m_indentBottom = value;
	}

	QDateTime TrendParam::startTime() const
	{
		return m_startTimeStamp.toDateTime();
	}

	void TrendParam::setStartTime(const QDateTime& value)
	{
		m_startTimeStamp.timeStamp = value.toMSecsSinceEpoch();
	}

	TimeStamp TrendParam::startTimeStamp() const
	{
		return m_startTimeStamp;
	}

	void TrendParam::setStartTimeStamp(const TimeStamp& value)
	{
		m_startTimeStamp = value;
	}

	qint64 TrendParam::duration() const
	{
		return m_duration;
	}

	void TrendParam::setLaneDuration(qint64 value)
	{
		m_duration = std::clamp(value, 500_ms, 24_hours * 365);
	}

	int TrendParam::hightlightRulerIndex() const
	{
		return m_highlightRulerIndex;
	}

	void TrendParam::setHightlightRulerIndex(int value)
	{
		m_highlightRulerIndex = value;
	}

	void TrendParam::resetHightlightRulerIndex()
	{
		m_highlightRulerIndex = -1;
	}

	double TrendParam::cosmeticPenWidth() const
	{
		return m_cosmeticPenWidth;
	}

	QString TrendParam::project() const
	{
		return m_project;
	}
	void TrendParam::setProject(const QString& value)
	{
		m_project = value;
	}

	std::vector<std::pair<QString, QRectF>>& TrendParam::signalDescriptionRect()
	{
		return m_signalDescriptionRect;
	}

	std::vector<std::pair<QString, QRectF>>& TrendParam::signalDescriptionRect() const
	{
		return m_signalDescriptionRect;
	}

} // namespace TrendLib
