#include <VFrame30/AppSignalController.h>
#include <VFrame30/ClientSchemaView.h>
#include <VFrame30/Context.h>
#include <VFrame30/DrawParam.h>
#include <VFrame30/IndicatorTrend.h>
#include <VFrame30/PropertyNames.h>
#include <VFrame30/SchemaItemIndicator.h>

#include <TrendView/TrendSignalSet.h>


namespace
{
	class TrendImageCache
	{
		Q_DISABLE_COPY_MOVE(TrendImageCache)

	public:
		TrendImageCache() = default;
		~TrendImageCache() { cache.clear(); }

		QImage* getCachedImage(const QUuid& trendUuid)
		{
			Key key = trendUuid;
			return cache.object(key);
		}

		void insertCachedImage(const QUuid& trendUuid, QImage* image)
		{
			Key key = trendUuid;
			cache.insert(key, image, image ? image->sizeInBytes() : 1);
		}

		using Key = QUuid; // Items QUuid

	private:
		QCache<Key, QImage> cache{60'000'000};
	};

	TrendImageCache& getTrendImageCache()
	{
		thread_local TrendImageCache s_trendImageCache;
		return s_trendImageCache;
	}

	// Draw Sand Clock
	//
	void drawSandClock(QPainter& painter, const QRectF& rect)
	{
		painter.setRenderHint(QPainter::Antialiasing);

		// Dim drawn image to indicate that it is cached
		//
		painter.fillRect(rect, QColor{255, 255, 255, 100});

		QRectF waitRect = rect;
		waitRect.setWidth(qMin(rect.width(), rect.height()) / 6);
		waitRect.setHeight(qMin(rect.width(), rect.height()) / 5);
		waitRect.moveCenter(rect.center());
		painter.fillRect(waitRect, QColor{200, 200, 200, 200});

		// Draw sand clock by primitive shapes
		//
		painter.setPen(Qt::NoPen);
		painter.setBrush(QBrush{QColor{150, 150, 150, 255}});

		// Platforms (top/bottom)
		qreal platformHeight = waitRect.height() * 0.06;
		qreal platformInset = waitRect.width() * 0.20;

		QRectF topPlatform{waitRect.left() + platformInset,
						   waitRect.top() + waitRect.height() * 0.08,
						   waitRect.width() - platformInset * 2.0,
						   platformHeight};

		QRectF bottomPlatform{waitRect.left() + platformInset,
							  waitRect.bottom() - waitRect.height() * 0.08 - platformHeight,
							  waitRect.width() - platformInset * 2.0,
							  platformHeight};

		painter.drawRect(topPlatform);
		painter.drawRect(bottomPlatform);

		QPointF topLeft{waitRect.left() + waitRect.width() * 0.25, topPlatform.bottom() + waitRect.height() * 0.01};
		QPointF topRight{waitRect.right() - waitRect.width() * 0.25, topPlatform.bottom() + waitRect.height() * 0.01};
		QPointF bottomLeft{waitRect.left() + waitRect.width() * 0.25, bottomPlatform.top() - waitRect.height() * 0.01};
		QPointF bottomRight{waitRect.right() - waitRect.width() * 0.25, bottomPlatform.top() - waitRect.height() * 0.01};

		QPolygonF topTriangle;
		topTriangle << topLeft << topRight << QPointF{waitRect.center().x(), waitRect.center().y()};
		painter.drawPolygon(topTriangle);

		QPolygonF bottomTriangle;
		bottomTriangle << bottomLeft << bottomRight << QPointF{waitRect.center().x(), waitRect.center().y()};
		painter.drawPolygon(bottomTriangle);

		return;
	}
} // namespace


namespace VFrame30
{
	//
	// IndicatorTrendSignalParam
	//
	void IndicatorTrendSignalParam::propertyDemand(const QString& prop)
	{
		// clang-format off

		if (prop.isEmpty() == true || prop == PropertyNames::color)
		{
			ADD_PROPERTY_GETTER_SETTER(QColor, PropertyNames::color, true, IndicatorTrendSignalParam::color, IndicatorTrendSignalParam::setColor)
				->setCategory(PropertyNames::appearanceCategory);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::lineWeight)
		{
			ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::lineWeight, true, IndicatorTrendSignalParam::lineWeight, IndicatorTrendSignalParam::setLineWeight)
				->setCategory(PropertyNames::appearanceCategory);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::analogFormat)
		{
			ADD_PROPERTY_GETTER_SETTER(E::AnalogFormat, PropertyNames::analogFormat, true, IndicatorTrendSignalParam::analogFormat, IndicatorTrendSignalParam::setAnalogFormat)
				->setCategory(PropertyNames::appearanceCategory)
				.setDescription(PropertyNames::analogFormatDescription);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::precision)
		{
			ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::precision, true, IndicatorTrendSignalParam::precision, IndicatorTrendSignalParam::setPrecision)
				->setCategory(PropertyNames::appearanceCategory);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::lowLimit)
		{
			ADD_PROPERTY_GETTER_SETTER(double, PropertyNames::lowLimit, true, IndicatorTrendSignalParam::lowLimit, IndicatorTrendSignalParam::setLowLimit)
				->setCategory(PropertyNames::indicatorSettings);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::highLimit)
		{
			ADD_PROPERTY_GETTER_SETTER(double, PropertyNames::highLimit, true, IndicatorTrendSignalParam::highLimit, IndicatorTrendSignalParam::setHighLimit)
				->setCategory(PropertyNames::indicatorSettings);
		}

		// clang-format on
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

		message->set_analogformat(static_cast<int>(m_analogFormat));
		message->set_precision(m_precision);

		message->set_lowlimit(m_lowLimit);
		message->set_highlimit(m_highLimit);

		return true;
	}

	bool IndicatorTrendSignalParam::load(const Proto::IndicatorTrendSignalParam& message)
	{
		m_color = message.color();
		m_lineWeight = message.linewieght();

		m_analogFormat = static_cast<E::AnalogFormat>(message.analogformat());
		m_precision = message.precision();

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

		trendSignalParam->setAnalogFormat(analogFormat());
		trendSignalParam->setPrecision(precision());

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

	E::AnalogFormat IndicatorTrendSignalParam::analogFormat() const
	{
		return m_analogFormat;
	}

	void IndicatorTrendSignalParam::setAnalogFormat(E::AnalogFormat value)
	{
		m_analogFormat = value;
	}

	int IndicatorTrendSignalParam::precision() const
	{
		return m_precision;
	}

	void IndicatorTrendSignalParam::setPrecision(int value)
	{
		m_precision = value;
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

		connect(&m_futureWatcher,
				&QFutureWatcher<QImage>::finished,
				this,
				[this]()
				{
					saveRenderedImage(m_drawFuture);
				});
	}

	IndicatorTrend::~IndicatorTrend()
	{
		disconnect(&m_futureWatcher, &QFutureWatcher<QImage>::finished, this, nullptr);

		// Async drawing must be stopped before destruction, it uses member data
		//
		if (m_drawFuture.isValid() == true && m_drawFuture.isRunning() == true)
		{
			qDebug() << "IndicatorTrend::~IndicatorTrend Stopping async drawing";

			m_drawStopSource.request_stop();
			m_drawFuture.waitForFinished();
		}

		return;
	}

	void IndicatorTrend::createProperties(SchemaItemIndicator* propertyObject, int /*signalCount*/)
	{
		Property* p = nullptr;

		// viewMode
		//
		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(E::TrendViewMode,
													   PropertyNames::indicatorTrendViewMode,
													   true,
													   IndicatorTrend::viewMode,
													   IndicatorTrend::setViewMode);
		p->setCategory(PropertyNames::indicatorSettings);

		// scaleType
		//
		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(E::TrendScaleType,
													   PropertyNames::indicatorTrendScaleType,
													   true,
													   IndicatorTrend::scaleType,
													   IndicatorTrend::setScaleType);
		p->setCategory(PropertyNames::indicatorSettings);

		// laneCount
		//
		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(int,
													   PropertyNames::indicatorTrendLaneCount,
													   true,
													   IndicatorTrend::laneCount,
													   IndicatorTrend::setLaneCount);
		p->setCategory(PropertyNames::indicatorSettings);

		// laneSpacing
		//
		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(QColor,
													   PropertyNames::indicatorTrendBackColor1st,
													   true,
													   IndicatorTrend::backColor1st,
													   IndicatorTrend::setBackColor1st);
		p->setCategory(PropertyNames::indicatorSettings);

		// laneSpacing
		//
		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(QColor,
													   PropertyNames::indicatorTrendBackColor2nd,
													   true,
													   IndicatorTrend::backColor2nd,
													   IndicatorTrend::setBackColor2nd);
		p->setCategory(PropertyNames::indicatorSettings);

		// showSignalIds
		//
		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(bool,
													   PropertyNames::indicatorTrendShowSignalIds,
													   true,
													   IndicatorTrend::showSignalIds,
													   IndicatorTrend::setShowSignalIds);
		p->setCategory(PropertyNames::indicatorSettings);

		// showSignalCaptions
		//
		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(bool,
													   PropertyNames::indicatorTrendShowSignalCaptions,
													   true,
													   IndicatorTrend::showSignalCaptions,
													   IndicatorTrend::setShowSignalCaptions);
		p->setCategory(PropertyNames::indicatorSettings);

		// showSignalScales
		//
		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(bool,
													   PropertyNames::indicatorTrendShowSignalScales,
													   true,
													   IndicatorTrend::showSignalScales,
													   IndicatorTrend::setShowSignalScales);
		p->setCategory(PropertyNames::indicatorSettings);

		// showTimeLabels
		//
		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(bool,
													   PropertyNames::indicatorTrendShowTimeLabels,
													   true,
													   IndicatorTrend::showTimeLabels,
													   IndicatorTrend::setShowTimeLabels);
		p->setCategory(PropertyNames::indicatorSettings);

		// showDateLabels
		//
		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(bool,
													   PropertyNames::indicatorTrendShowDateLabels,
													   true,
													   IndicatorTrend::showDateLabels,
													   IndicatorTrend::setShowDateLabels);
		p->setCategory(PropertyNames::indicatorSettings);

		// samplePeriod
		//
		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(E::RtTrendsSamplePeriod,
													   PropertyNames::indicatorTrendSamplePeriod,
													   true,
													   IndicatorTrend::samplePeriod,
													   IndicatorTrend::setSamplePeriod);
		p->setCategory(PropertyNames::indicatorSettings);

		// timeType
		//
		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(E::TimeType,
													   PropertyNames::timeType,
													   true,
													   IndicatorTrend::timeType,
													   IndicatorTrend::setTimeType);
		p->setCategory(PropertyNames::indicatorSettings);
		p->setDescription(PropertyNames::timeTypeToolTip);

		// redrawInterval
		//
		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(int,
													   PropertyNames::indicatorTrendRedrawInterval,
													   true,
													   IndicatorTrend::redrawInterval,
													   IndicatorTrend::setRedrawInterval);
		p->setCategory(PropertyNames::indicatorSettings);
		p->setDescription(PropertyNames::indicatorTrendRedrawIntervalToolTip);

		// laneDuration
		//
		p = propertyObject->ADD_PROPERTY_GETTER_SETTER(int,
													   PropertyNames::indicatorTrendLaneDuration,
													   true,
													   IndicatorTrend::durationSeconds,
													   IndicatorTrend::setDurationSeconds);
		p->setCategory(PropertyNames::indicatorSettings);
		p->setDescription(PropertyNames::indicatorTrendLaneDurationToolTip);

		// trendSignalParams
		//
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

		if (message.has_indicatortrend() == false) // Line to change 1
		{
			// It can be just added new item, default values are taken
			//
			return true;
		}

		const ::Proto::IndicatorTrend& m = message.indicatortrend(); // Line to change 2

		m_samplePeriod = static_cast<E::RtTrendsSamplePeriod>(m.sampleperiod());
		m_timeType = static_cast<E::TimeType>(m.timetype());
		m_redrawInterval = m.redrawinterval();
		m_trendParam.setLaneDuration(m.duration());                  // Save/restore in ms

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

		if (m.has_showsignalids() == true)
		{
			m_trendParam.setShowSignalIds(m.showsignalids());
		}

		if (m.has_showsignalcaptions() == true)
		{
			m_trendParam.setShowSignalCaptions(m.showsignalcaptions());
		}

		if (m.has_showsignalscales() == true)
		{
			m_trendParam.setShowSignalScales(m.showsignalscales());
		}

		if (m.has_showtimelabels() == true)
		{
			m_trendParam.setShowTimeLabels(m.showtimelabels());
		}

		if (m.has_showdatelabels() == true)
		{
			m_trendParam.setShowDateLabels(m.showdatelabels());
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

		auto m = message->mutable_indicatortrend(); // Line to change 1

		m->set_viewmode(static_cast<int>(viewMode()));
		m->set_scaletype(static_cast<int>(scaleType()));
		m->set_lanecount(laneCount());
		m->set_backcolor1st(backColor1st().rgba());
		m->set_backcolor2nd(backColor2nd().rgba());
		m->set_showsignalids(showSignalIds());
		m->set_showsignalcaptions(showSignalCaptions());
		m->set_showsignalscales(showSignalScales());
		m->set_showtimelabels(showTimeLabels());
		m->set_showdatelabels(showDateLabels());

		m->set_sampleperiod(static_cast<int>(m_samplePeriod));
		m->set_timetype(static_cast<int>(m_timeType));
		m->set_redrawinterval(m_redrawInterval);
		m->set_duration(m_trendParam.duration()); // Save/restore in ms

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
		if (drawParam == nullptr || schemaItem == nullptr)
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
			trendRect = {0,
						 0,
						 boundingRect.width() * drawParam->realDpiX() * zoom, // Zoom image so it will be well drawn on high zoom values
						 boundingRect.height() * drawParam->realDpiY() * zoom};
		}
		else
		{
			trendRect = {0, 0, boundingRect.width() * zoom, boundingRect.height() * zoom};
		}

		QSize expectedImageSize{static_cast<int>(trendRect.width()), static_cast<int>(trendRect.height())};

		// --
		//
		m_trendParam.setDpi(painter->device()->physicalDpiX(), painter->device()->physicalDpiY(), painter->device()->devicePixelRatioF());

		m_trendParam.setRectPx(trendRect, m_trendParam.dpiX(), m_trendParam.dpiY(), m_trendParam.devicePixelRatio() * zoom);
		m_trendParam.setTimeType(m_timeType);

		// Shift real-time trend
		//
		if (drawParam->drawMode() != DrawMode::Editor)
		{
			Q_ASSERT(drawParam->clientSchemaView());
			m_trendParam.setTrendDataProvider(drawParam->clientSchemaView()->schemaManager());
			TimeStamp maxTimeStamp = m_trendParam.trendDataProvider()->maxTimeStamp(schemaItem->guid(), m_timeType);

			if (maxTimeStamp.timeStamp != 0)
			{
				TimeStamp startTimeStamp = {maxTimeStamp.timeStamp - m_trendParam.duration() * m_trendParam.laneCount()};
				m_trendParam.setStartTimeStamp(startTimeStamp);
			}
		}

		bool forceRedraw = false;
		if (m_forceRedraw.load(std::memory_order_acquire) == true)
		{
			forceRedraw = true;
			m_forceRedraw.store(false, std::memory_order_release);
			m_image = {};
		}

		if (m_image.isNull() == false)
		{
			painter->drawImage(boundingRect, m_image);
		}
		else
		{
			QImage* cachedImage = getTrendImageCache().getCachedImage(schemaItem->guid());
			if (cachedImage != nullptr)
			{
				// m_image = *cachedImage; -- Do not set m_image, we need needRedraw to set to true (different size).
				painter->drawImage(boundingRect, *cachedImage);
			}
			else
			{
				// Backup, draw gray rect.
				//
				painter->fillRect(boundingRect, backColor1st());
			}

			drawSandClock(*painter, boundingRect);
		}

		// Redraw image if needed
		//

		// Detect if image update is required
		//
		bool needRedraw = m_redrawInterval < 250_ms ||                 //
						  forceRedraw == true ||                       //
						  m_drawTimer.hasExpired(m_redrawInterval) ||  //
						  drawParam->drawMode() == DrawMode::Editor || //
						  (expectedImageSize != m_image.size());

		// Start async redraw
		// IsCanceled means that we already taken the result and there is no running task
		// If is pdf mode, do not draw trend, the current issues is very long pdf generation time for many points
		// after we optimize drawing to O(width) instead of O(Npoints), when we can allow rendering in pdf mode.
		//
		if (needRedraw == true && m_drawFuture.isCanceled() == true && drawParam->pdfMode() == false)
		{
			// Check if there are any new signals
			//
			QStringList itemSignalIds = schemaItem->signalIds();
			QStringList trendSignalIds = m_trend.signalSet().trendSignalIds();

			if (bool signalsNotSame = (itemSignalIds != trendSignalIds) || m_updateSignalsTimer.hasExpired(10'000) || forceRedraw; //
				signalsNotSame == true)
			{
				std::list<TrendLib::TrendSignalParam> signalParams;

				const AppSignalController* appSignalController = context->appSignalController();
				Q_ASSERT(appSignalController);

				for (size_t index = 0; const QString& appSignalId : std::as_const(itemSignalIds))
				{
					auto appSignalParam = appSignalController->signalParam(appSignalId)
											  .or_else(
												  [&appSignalId]
												  {
													  std::optional<AppSignalParam> asp = AppSignalParam{};
													  asp->setAppSignalId(appSignalId);
													  asp->setCustomSignalId(appSignalId);
													  return asp;
												  })
											  .value();

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

			// Async redraw
			//
			m_drawTimer.restart();

			m_trend.setUuid(schemaItem->guid());

			auto drawFuture = QtConcurrent::run(
				[](TrendLib::Trend& trend, TrendLib::TrendParam drawParam, QSize imageSize, std::stop_token stoken) -> QImage
				{
					try
					{
						if (imageSize.width() > 32767 || imageSize.height() > 32767 || imageSize.isEmpty() == true ||
							imageSize.width() * imageSize.height() * 4 > 256'000'000)
						{
							qWarning() << "IndicatorTrend, AsyncDraw: Invalid image size " << imageSize;
							return QImage{};
						}

						QImage image{imageSize, QImage::Format_RGB32};

						image.setDevicePixelRatio(drawParam.devicePixelRatio());
						image.setDotsPerMeterX(static_cast<int>(image.physicalDpiX() / 25.4 * 1000.0));
						image.setDotsPerMeterY(static_cast<int>(image.physicalDpiY() / 25.4 * 1000.0));

						// QElapsedTimer drawTimer{};
						// drawTimer.start();

						drawParam.signalDescriptionRect().clear();
						trend.draw(&image, drawParam, stoken);

						// qDebug() << "AsyncDraw: TrendIndicator.draw " << drawTimer.elapsed() << " ms";
						return image;
					}
					catch (const std::exception& ex)
					{
						qWarning() << "IndicatorTrend, AsyncDraw: Exception during draw: " << ex.what();
						return QImage{};
					}
					catch (...)
					{
						qWarning() << "IndicatorTrend, AsyncDraw: Unknown exception during draw";
						return QImage{};
					}
				},
				std::ref(m_trend),
				m_trendParam,
				expectedImageSize,
				m_drawStopSource.get_token());

			bool requiredInstantDraw = drawParam->drawMode() == DrawMode::Editor;

			if (requiredInstantDraw == true)
			{
				drawFuture.waitForFinished();
				saveRenderedImage(drawFuture); // The result is now in m_image, so we can draw it immediately.

				painter->drawImage(boundingRect, m_image);
			}
			else
			{
				m_drawFuture = std::move(drawFuture);
				// Reassign future to watcher.
				// Watcher has a connected slot to get the result -- see IndicatorTrend ctor.
				//
				m_futureWatcher.setFuture(m_drawFuture);

				// The image will be drawn on the next paint -- see saveRenderedImage().
			}
		}

		return;
	}

	void IndicatorTrend::saveRenderedImage(QFuture<QImage>& future) const
	{
		if (future.isValid() == true && future.isFinished() == true)
		{
			m_image = future.result();
			m_drawFuture = {};

			getTrendImageCache().insertCachedImage(m_trend.uuid(), new QImage{m_image});
		}
		else
		{
			Q_ASSERT(future.isValid());
			Q_ASSERT(future.isValid() == true && future.isFinished() == false);
		}

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

	void IndicatorTrend::forceRedraw()
	{
		m_forceRedraw.store(true, std::memory_order_release);
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

	bool IndicatorTrend::showSignalIds() const
	{
		return m_trendParam.showSignalIds();
	}

	void IndicatorTrend::setShowSignalIds(bool value)
	{
		m_trendParam.setShowSignalIds(value);
	}

	bool IndicatorTrend::showSignalCaptions() const
	{
		return m_trendParam.showSignalCaptions();
	}

	void IndicatorTrend::setShowSignalCaptions(bool value)
	{
		m_trendParam.setShowSignalCaptions(value);
	}

	bool IndicatorTrend::showSignalScales() const
	{
		return m_trendParam.showSignalScales();
	}

	void IndicatorTrend::setShowSignalScales(bool value)
	{
		m_trendParam.setShowSignalScales(value);
	}

	bool IndicatorTrend::showTimeLabels() const
	{
		return m_trendParam.showTimeLabels();
	}

	void IndicatorTrend::setShowTimeLabels(bool value)
	{
		m_trendParam.setShowTimeLabels(value);
	}

	bool IndicatorTrend::showDateLabels() const
	{
		return m_trendParam.showDateLabels();
	}

	void IndicatorTrend::setShowDateLabels(bool value)
	{
		m_trendParam.setShowDateLabels(value);
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

	QList<VFrame30::IndicatorTrendSignalParam*> IndicatorTrend::jsTrendSignalParams() const
	{
		QList<VFrame30::IndicatorTrendSignalParam*> list;
		list.reserve(m_trendSignalParams.size());

		for (const auto& tsp : m_trendSignalParams)
		{
			QJSEngine::setObjectOwnership(tsp.get(), QJSEngine::CppOwnership);
			list.append(tsp.get());
		}

		return list;
	}
} // namespace VFrame30
