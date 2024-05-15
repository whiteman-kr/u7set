#include <VFrame30/IndicatorTrend.h>
#include <VFrame30/AppSignalController.h>
#include <VFrame30/ClientSchemaView.h>
#include <VFrame30/Context.h>
#include <VFrame30/DrawParam.h>
#include <VFrame30/PropertyNames.h>
#include <VFrame30/SchemaItemIndicator.h>

#include <TrendView/TrendSignalSet.h>

namespace VFrame30
{
	//
	// IndicatorTrendSignalParam
	//
	void IndicatorTrendSignalParam::propertyDemand(const QString&)
	{
		ADD_PROPERTY_GETTER_SETTER(QColor, PropertyNames::color, true, IndicatorTrendSignalParam::color, IndicatorTrendSignalParam::setColor)
				->setCategory(PropertyNames::indicatorSettings);

		ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::lineWeight, true, IndicatorTrendSignalParam::lineWeight, IndicatorTrendSignalParam::setLineWeight)
				->setCategory(PropertyNames::indicatorSettings);

		ADD_PROPERTY_GETTER_SETTER(double, PropertyNames::lowLimit, true, IndicatorTrendSignalParam::lowLimit, IndicatorTrendSignalParam::setLowLimit)
				->setCategory(PropertyNames::indicatorSettings);

		ADD_PROPERTY_GETTER_SETTER(double, PropertyNames::highLimit, true, IndicatorTrendSignalParam::highLimit, IndicatorTrendSignalParam::setHighLimit)
				->setCategory(PropertyNames::indicatorSettings);

		return;
	}

	bool IndicatorTrendSignalParam::save(Proto::IndicatorTrendSignalParam* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		message->set_color(m_color.rgba());
		message->set_linewieght(m_lineWeight);

		message->set_lowlimit(m_lowLimit);
		message->set_highlimit(m_highLimit);

		return true;
	}

	bool IndicatorTrendSignalParam::load(const Proto::IndicatorTrendSignalParam& message)
	{
		m_color = message.color();
		m_lineWeight = message.linewieght();

		m_lowLimit = message.lowlimit();
		m_highLimit = message.highlimit();

		return true;
	}

	void IndicatorTrendSignalParam::initTrensSignalParam(TrendLib::TrendSignalParam* trendSignalParam) const
	{
		if (trendSignalParam == nullptr)
		{
			Q_ASSERT(trendSignalParam);
			return;
		}

		trendSignalParam->setColor(color().rgb());
		trendSignalParam->setLineWeight(lineWeight());

		trendSignalParam->setLowLimit(lowLimit());
		trendSignalParam->setHighLimit(highLimit());

		return;
	}

	QColor IndicatorTrendSignalParam::color() const
	{
		return m_color;
	}

	void IndicatorTrendSignalParam::setColor(const QColor& value)
	{
		m_color = value;
	}

	int IndicatorTrendSignalParam::lineWeight() const
	{
		return m_lineWeight;
	}

	void IndicatorTrendSignalParam::setLineWeight(int value)
	{
		m_lineWeight = value;
	}

	double IndicatorTrendSignalParam::lowLimit() const
	{
		return m_lowLimit;
	}
	void IndicatorTrendSignalParam::setLowLimit(double value)
	{
		m_lowLimit = std::min(m_highLimit, value);
	}

	double IndicatorTrendSignalParam::highLimit() const
	{
		return m_highLimit;
	}
	void IndicatorTrendSignalParam::setHighLimit(double value)
	{
		m_highLimit = std::max(m_lowLimit, value);
	}

	//
	// IndicatorTrend
	//
	IndicatorTrend::IndicatorTrend(SchemaUnit itemUnit) :
		Indicator(itemUnit)
	{
		m_drawTimer.start();
		m_updateSignalsTimer.start();
	}

	void IndicatorTrend::createProperties(SchemaItemIndicator* propertyObject, int /*signalCount*/)
	{
		Property* p = nullptr;

		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(E::TrendViewMode, PropertyNames::indicatorTrendViewMode, true, IndicatorTrend::viewMode, IndicatorTrend::setViewMode);
		p->setCategory(PropertyNames::indicatorSettings);

		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(E::TrendScaleType, PropertyNames::indicatorTrendScaleType, true, IndicatorTrend::scaleType, IndicatorTrend::setScaleType);
		p->setCategory(PropertyNames::indicatorSettings);

		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::indicatorTrendLaneCount, true, IndicatorTrend::laneCount, IndicatorTrend::setLaneCount);
		p->setCategory(PropertyNames::indicatorSettings);

		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(QColor, PropertyNames::indicatorTrendBackColor1st, true, IndicatorTrend::backColor1st, IndicatorTrend::setBackColor1st);
		p->setCategory(PropertyNames::indicatorSettings);

		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(QColor, PropertyNames::indicatorTrendBackColor2nd, true, IndicatorTrend::backColor2nd, IndicatorTrend::setBackColor2nd);
		p->setCategory(PropertyNames::indicatorSettings);

		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(E::RtTrendsSamplePeriod, PropertyNames::indicatorTrendSamplePeriod, true, IndicatorTrend::samplePeriod, IndicatorTrend::setSamplePeriod);
		p->setCategory(PropertyNames::indicatorSettings);

		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(E::TimeType, PropertyNames::timeType, true, IndicatorTrend::timeType, IndicatorTrend::setTimeType);
		p->setCategory(PropertyNames::indicatorSettings);
		p->setDescription(PropertyNames::timeTypeToolTip);

		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::indicatorTrendRedrawInterval, true, IndicatorTrend::redrawInterval, IndicatorTrend::setRedrawInterval);
		p->setCategory(PropertyNames::indicatorSettings);
		p->setDescription(PropertyNames::indicatorTrendRedrawIntervalToolTip);

		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::indicatorTrendLaneDuration, true, IndicatorTrend::durationSeconds, IndicatorTrend::setDurationSeconds);
		p->setCategory(PropertyNames::indicatorSettings);
		p->setDescription(PropertyNames::indicatorTrendLaneDurationToolTip);

		propertyObject->ADD_PROPERTY_CAT_VAR(PropertyVector<IndicatorTrendSignalParam>,
											 PropertyNames::trendSignalParams,
											 PropertyNames::indicatorSettings,
											 true,
											 m_trendSignalParams);

		return;
	}

	// Load data
	//
	bool IndicatorTrend::load(const Proto::SchemaItemIndicator& message, SchemaUnit unit)
	{
		m_itemUnit = unit;

		if (message.has_indicatortrend() == false)						// Line to change 1
		{
			// It can be just added new item, default values are taken
			//
			return true;
		}

		const ::Proto::IndicatorTrend& m = message.indicatortrend();	// Line to change 2

		m_samplePeriod = static_cast<E::RtTrendsSamplePeriod>(m.sampleperiod());
		m_timeType = static_cast<E::TimeType>(m.timetype());
		m_redrawInterval = m.redrawinterval();
		m_trendParam.setLaneDuration(m.duration());						// Save/restore in ms

		m_trendSignalParams.clear();
		m_trendSignalParams.reserve(static_cast<size_t>(m.trendsignalparams_size()));
		for (int i = 0; i < m.trendsignalparams_size(); i++)
		{
			auto tsp = m_trendSignalParams.createItem();
			tsp->load(m.trendsignalparams(i));
			m_trendSignalParams.push_back(tsp);
		}

		if (m.has_viewmode() == true)
		{
			m_trendParam.setViewMode(static_cast<E::TrendViewMode>(m.viewmode()));
		}

		if (m.has_scaletype() == true)
		{
			m_trendParam.setScaleType(static_cast<E::TrendScaleType>(m.scaletype()));
		}

		if (m.has_lanecount() == true)
		{
			m_trendParam.setLaneCount(m.lanecount());
		}

		if (m.has_backcolor1st() == true)
		{
			m_trendParam.setBackColor1st(m.backcolor1st());
		}

		if (m.has_backcolor2nd() == true)
		{
			m_trendParam.setBackColor2nd(m.backcolor2nd());
		}

		return true;
	}

	// Save data
	//
	bool IndicatorTrend::save(Proto::SchemaItemIndicator* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		auto m = message->mutable_indicatortrend();						// Line to change 1

		m->set_viewmode(static_cast<int>(viewMode()));
		m->set_scaletype(static_cast<int>(scaleType()));
		m->set_lanecount(laneCount());
		m->set_backcolor1st(backColor1st().rgba());
		m->set_backcolor2nd(backColor2nd().rgba());
				
		m->set_sampleperiod(static_cast<int>(m_samplePeriod));
		m->set_timetype(static_cast<int>(m_timeType));
		m->set_redrawinterval(m_redrawInterval);
		m->set_duration(m_trendParam.duration());						// Save/restore in ms

		for (const auto& tsp : m_trendSignalParams)
		{
			::Proto::IndicatorTrendSignalParam* mtrsp = m->add_trendsignalparams();
			tsp->save(mtrsp);
		}

		return true;
	}

	// Draw trend
	//
	void IndicatorTrend::draw(CDrawParam* drawParam, const SchemaItemIndicator* schemaItem) const
	{
		if (drawParam == nullptr ||
			schemaItem == nullptr)
		{
			Q_ASSERT(drawParam);
			Q_ASSERT(schemaItem);
			return;
		}

		auto context = schemaItem->context();
		if (context == nullptr)
		{
			Q_ASSERT(context);
			return;
		}

		Q_ASSERT(drawParam->schemaUnit() == m_itemUnit);

		QPainter* painter = drawParam->painter();
		Q_ASSERT(painter);

		// --
		//
		double zoom = drawParam->schemaView()->zoom() / 100.0;
		if (zoom > 1.0)
		{
			zoom = 1.0;
		}

		// --
		//
		QRectF boundingRect = schemaItem->boundingRectInDocPt(drawParam);
		QRectF trendRect{};

		if (m_itemUnit == SchemaUnit::Inch)
		{
			trendRect = {0, 0,
						 boundingRect.width() * drawParam->realDpiX() * zoom,		// Zoom image so it will be well drawn on high zoom values
						 boundingRect.height() * drawParam->realDpiY() * zoom};
		}
		else
		{
			trendRect = {0, 0, boundingRect.width() * zoom, boundingRect.height() * zoom};
		}

		// --
		//
		m_trendParam.setRect(trendRect);
		m_trendParam.setTimeType(m_timeType);

		if (drawParam->drawMode() != DrawMode::Editor)
		{
			Q_ASSERT(drawParam->clientSchemaView());
			m_trendParam.setTrendDataProvider(drawParam->clientSchemaView()->schemaManager());

			// Shift realtime trend
			//
			TimeStamp maxTimeStamp = m_trendParam.trendDataProvider()->maxTimeStamp(schemaItem->guid(), m_timeType);

			if (maxTimeStamp.timeStamp != 0)
			{
				TimeStamp startTimeStamp = {maxTimeStamp.timeStamp - m_trendParam.duration() * m_trendParam.laneCount()};
				m_trendParam.setStartTimeStamp(startTimeStamp);
			}
		}

		// Detect if image update is required
		//
		bool needRedraw = (m_redrawInterval < 250_ms) ||
							  m_drawTimer.hasExpired(m_redrawInterval) ||
							  drawParam->drawMode() == DrawMode::Editor;

		if (m_image.width() != static_cast<int>(trendRect.width()) ||
			m_image.height() != static_cast<int>(trendRect.height()))
		{
			needRedraw = true;

			m_image = QImage{static_cast<int>(trendRect.width()), static_cast<int>(trendRect.height()), QImage::Format_RGB32};

			m_image.setDevicePixelRatio(drawParam->devicePixelRatio());
			m_image.setDotsPerMeterX(static_cast<int>(m_image.physicalDpiX() / 25.4 * 1000.0));
			m_image.setDotsPerMeterY(static_cast<int>(m_image.physicalDpiY() / 25.4 * 1000.0));
		}

		// Draw trend to QImage and then copy it to painter
		//
		if (needRedraw == true)
		{
			// Check if there are any new signals
			//
			QStringList itemSignalIds = schemaItem->signalIds();
			QStringList trendSignalIds = m_trend.signalSet().trendSignalIds();

			bool signalsAreTheSame = (itemSignalIds != trendSignalIds) ||
									 m_updateSignalsTimer.hasExpired(10'000);

			if (signalsAreTheSame)
			{
				std::list<TrendLib::TrendSignalParam> signalParams;

				const AppSignalController* appSignalController = context->appSignalController();
				Q_ASSERT(appSignalController);

				for (size_t index = 0;
					 const QString& appSignalId : qAsConst(itemSignalIds))
				{
					bool signalFound = false;

					AppSignalParam appSignalParam = appSignalController->signalParam(appSignalId, &signalFound);
					if (signalFound == false)
					{
						appSignalParam.setAppSignalId(appSignalId);
						appSignalParam.setCustomSignalId(appSignalId);
					}

					TrendLib::TrendSignalParam& trensSignalParam = signalParams.emplace_back(appSignalParam, TrendLib::ArchiveServer{});

					// Set trend line draw params
					//
					if (index < m_trendSignalParams.size())
					{
						std::shared_ptr<IndicatorTrendSignalParam> itsp = m_trendSignalParams[index];
						Q_ASSERT(itsp);

						itsp->initTrensSignalParam(&trensSignalParam);
					}

					index++;
				}

				m_trend.signalSet().addSignals(std::move(signalParams));

				m_updateSignalsTimer.restart();
			}

			//	--
			//
			m_drawTimer.restart();

			QElapsedTimer drawTimer;
			drawTimer.start();

			m_trend.setUuid(schemaItem->guid());

			m_trendParam.signalDescriptionRect().clear();
			m_trendParam.setDpi(m_image.dotsPerMeterX() / (1000.0 / 25.4), m_image.dotsPerMeterY() / (1000.0 / 25.4), m_image.devicePixelRatioF());

			m_trend.draw(&m_image, m_trendParam);

			//qDebug() << "m_trend.draw " << drawTimer.elapsed() << " ms";
		}

		painter->drawImage(boundingRect, m_image);
		return;
	}

	TrendLib::Trend& IndicatorTrend::trend()
	{
		return m_trend;
	}

	const TrendLib::Trend& IndicatorTrend::trend() const
	{
		return m_trend;
	}

	E::TrendViewMode IndicatorTrend::viewMode() const
	{
		return m_trendParam.viewMode();
	}

	void IndicatorTrend::setViewMode(E::TrendViewMode value)
	{
		m_trendParam.setViewMode(value);
	}

	E::TrendScaleType IndicatorTrend::scaleType() const
	{
		return m_trendParam.scaleType();
	}

	void IndicatorTrend::setScaleType(E::TrendScaleType value)
	{
		m_trendParam.setScaleType(value);
	}

	int IndicatorTrend::laneCount() const
	{
		return m_trendParam.laneCount();
	}

	void IndicatorTrend::setLaneCount(int value)
	{
		m_trendParam.setLaneCount(value);
	}

	QColor IndicatorTrend::backColor1st() const
	{
		return m_trendParam.backColor1st();
	}

	void IndicatorTrend::setBackColor1st(const QColor& value)
	{
		m_trendParam.setBackColor1st(value);
	}

	QColor IndicatorTrend::backColor2nd() const
	{
		return m_trendParam.backColor2nd();
	}

	void IndicatorTrend::setBackColor2nd(const QColor& value)
	{
		m_trendParam.setBackColor2nd(value);
	}

	E::RtTrendsSamplePeriod IndicatorTrend::samplePeriod() const
	{
		return m_samplePeriod;
	}

	void IndicatorTrend::setSamplePeriod(E::RtTrendsSamplePeriod value)
	{
		m_samplePeriod = value;
	}

	E::TimeType IndicatorTrend::timeType() const
	{
		return m_timeType;
	}

	void IndicatorTrend::setTimeType(E::TimeType value)
	{
		if (value == E::TimeType::ArchiveId)
		{
			value = E::TimeType::Local;
		}

		m_timeType = value;
	}

	int IndicatorTrend::redrawInterval() const
	{
		return static_cast<int>(m_redrawInterval);
	}

	void IndicatorTrend::setRedrawInterval(int value)
	{
		m_redrawInterval = std::clamp<qint64>(value, 0_ms, 1_hour);
	}

	int IndicatorTrend::durationSeconds() const
	{
		return static_cast<int>(m_trendParam.duration() / 1000);
	}

	void IndicatorTrend::setDurationSeconds(int value)
	{
		m_trendParam.setLaneDuration(value * 1000);
	}
}
