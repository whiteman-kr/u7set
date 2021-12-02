#include "IndicatorTrend.h"
#include "Schema.h"
#include "SchemaView.h"
#include "SchemaItemIndicator.h"
#include "PropertyNames.h"
#include "DrawParam.h"
#include "AppSignalController.h"

namespace VFrame30
{
	//
	// IndicatorTrend
	//
	IndicatorTrend::IndicatorTrend(SchemaUnit itemUnit) :
		Indicator(itemUnit)
	{
	}

	void IndicatorTrend::createProperties(SchemaItemIndicator* propertyObject, int /*signalCount*/)
	{
		Property* p = nullptr;

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

		m_drawTimer.start();

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

		m->set_sampleperiod(static_cast<int>(m_samplePeriod));
		m->set_timetype(static_cast<int>(m_timeType));
		m->set_redrawinterval(m_redrawInterval);
		m->set_duration(m_trendParam.duration());						// Save/restore in ms

		return true;
	}

	// Draw trend
	//
	void IndicatorTrend::draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer, const SchemaItemIndicator* schemaItem) const
	{
		if (drawParam == nullptr ||
			schema == nullptr ||
			layer == nullptr ||
			schemaItem == nullptr)
		{
			Q_ASSERT(drawParam);
			Q_ASSERT(schema);
			Q_ASSERT(layer);
			Q_ASSERT(schemaItem);
			return;
		}

		Q_ASSERT(schema->unit() == m_itemUnit);

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
						 boundingRect.width() * drawParam->dpiX() * zoom,		// Zoom image so it will be well drawn on high zoom values
						 boundingRect.height() * drawParam->dpiY() * zoom};
		}
		else
		{
			trendRect = {0, 0, boundingRect.width() * zoom, boundingRect.height() * zoom};
		}

		// --
		//
		m_trendParam.setRect(trendRect);
		m_trendParam.setDpi(drawParam->dpiX(), drawParam->dpiY());

		// Detect if image update is required
		//
		bool requiredRedraw = (m_redrawInterval < 250_ms) ||
							  m_drawTimer.hasExpired(m_redrawInterval) ||
							  drawParam->isEditMode();

		if (m_image.width() != static_cast<int>(trendRect.width()) ||
			m_image.height() != static_cast<int>(trendRect.height()))
		{
			requiredRedraw = true;

			m_image = QImage{static_cast<int>(trendRect.width()), static_cast<int>(trendRect.height()), QImage::Format_RGB32};

			qDebug() << "Trend image size " << m_image.rect().size();
			qDebug() << "Trend image dpis " << m_image.logicalDpiX() << " x " << m_image.logicalDpiY();
		}

		// Check if there are any new signals
		//
		QStringList itemSignalIds = schemaItem->signalIds();
		QStringList trendSignalIds = m_trend.signalSet().trendSignalIds();

		if (itemSignalIds != trendSignalIds)
		{
			std::list<TrendLib::TrendSignalParam> signalParams;

			AppSignalController* appSignalController = drawParam->appSignalController();
			Q_ASSERT(appSignalController);

			for (const QString& appSignalId : qAsConst(itemSignalIds))
			{
				bool signalFound = false;

				AppSignalParam appSignalParam = appSignalController->signalParam(appSignalId, &signalFound);
				if (signalFound == false)
				{
					appSignalParam.setAppSignalId(appSignalId);
				}

				signalParams.emplace_back(appSignalParam);
			}

			m_trend.signalSet().addSignals(std::move(signalParams));
		}

		// Draw trend to QImage and then copy it to painter
		//
		if (requiredRedraw == true)
		{
			m_drawTimer.restart();

			QElapsedTimer drawTimer;
			drawTimer.start();

			m_trendParam.signalDescriptionRect().clear();
			m_trend.draw(&m_image, m_trendParam);

			qDebug() << "m_trend.draw " << drawTimer.elapsed() << " ms";
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
