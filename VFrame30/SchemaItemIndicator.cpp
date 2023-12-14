#include "SchemaItemIndicator.h"
#include "../AppSignalLib/TuningSignalState.h"
#include "PropertyNames.h"
#include "DrawParam.h"
#include "Schema.h"
#include "FblItemRect.h"
#include "MacrosExpander.h"
#include "AppSignalController.h"
#include "IndicatorHistogramVert.h"
#include "IndicatorArrowIndicator.h"
#include "IndicatorTrend.h"

namespace VFrame30
{
	//
	// SchemaItemIndicator
	//
	SchemaItemIndicator::SchemaItemIndicator(void) :
		SchemaItemIndicator(SchemaUnit::Inch)
	{
		// This constructor can be called during serialization, then all variables
		// are initialized in loading process
		//
	}

	SchemaItemIndicator::SchemaItemIndicator(SchemaUnit unit) :
		m_indicatorObjects
			{
				std::make_unique<IndicatorHistogramVert>(unit),		// E::IndicatorType::HistogramVert
				std::make_unique<IndicatorArrowIndicator>(unit),	// E::IndicatorType::ArrowIndicator
				std::make_unique<IndicatorTrend>(unit),				// E::IndicatorType::Trend
			}
	{
		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::appSignalIDs, PropertyNames::functionalCategory, true, SchemaItemIndicator::signalIdsString, SchemaItemIndicator::setSignalIdsString);

		ADD_PROPERTY_GETTER_SETTER(E::IndicatorType, PropertyNames::indicatorType, true, SchemaItemIndicator::indicatorType, SchemaItemIndicator::setIndicatorType);

		m_static = false;
		setItemUnit(unit);

		setIndicatorType(E::IndicatorType::HistogramVert);

		for (IndicatorObjectPtr& indicator : m_indicatorObjects)
		{
			connect(indicator.get(), &Indicator::updatePropertiesList, this, &SchemaItemIndicator::updateIndicatorProperties);
		}

		return;
	}

	// Serialization
	//
	bool SchemaItemIndicator::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectImpl::SaveData(message);
		if (result == false || message->has_schemaitem() == false)
		{
			Q_ASSERT(result);
			Q_ASSERT(message->has_schemaitem());
			return false;
		}

		// --
		//
		Proto::SchemaItemIndicator* indicatorMessage = message->mutable_schemaitem()->mutable_indicator();

		indicatorMessage->set_signalids(signalIdsString().toStdString());

		indicatorMessage->set_type(static_cast<int>(m_indicatorType));

		for (auto& io : m_indicatorObjects)
		{
			Q_ASSERT(io);
			io->save(indicatorMessage);
		}

		return true;
	}

	bool SchemaItemIndicator::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			Q_ASSERT(message.has_schemaitem());
			return false;
		}

		bool result = PosRectImpl::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_indicator() == false)
		{
			Q_ASSERT(message.schemaitem().has_indicator());
			return false;
		}

		const Proto::SchemaItemIndicator& indicatorMessage = message.schemaitem().indicator();

		setSignalIdsString(indicatorMessage.signalids().data());

		// Set iunits for the IndicatorObject, as it is not saved in proto container and it must be the same with schema item
		//
		for (auto& io : m_indicatorObjects)
		{
			io->setUnits(itemUnit());
			io->load(indicatorMessage, itemUnit());
		}

		// --
		//
		setIndicatorType(static_cast<E::IndicatorType>(indicatorMessage.type()));	// call setter to create properties

		return true;
	}

	// Drawing Functions
	//
	void SchemaItemIndicator::draw(CDrawParam* drawParam) const
	{
		QPainter* p = drawParam->painter();
		Q_ASSERT(p);

		auto context = this->context();
		Q_ASSERT(context);

		const Indicator* io = indicatorObject();

		if (io == nullptr)
		{
			Q_ASSERT(io);
			return;
		}

		io->draw(drawParam, this);

		return;
	}

	// Trend functions
	//
	bool SchemaItemIndicator::isTrend() const
	{
		return m_indicatorType == E::IndicatorType::Trend;
	}

	TrendLib::Trend& SchemaItemIndicator::trend()
	{
		Q_ASSERT(isTrend() == true);

		IndicatorTrend* i = indicatorObject<IndicatorTrend>();
		Q_ASSERT(i);

		return i->trend();
	}

	const TrendLib::Trend& SchemaItemIndicator::trend() const
	{
		Q_ASSERT(isTrend() == true);

		const IndicatorTrend* i = indicatorObject<IndicatorTrend>();
		Q_ASSERT(i);

		return i->trend();
	}

	E::RtTrendsSamplePeriod SchemaItemIndicator::trendSamplePeriod() const
	{
		Q_ASSERT(isTrend() == true);

		const IndicatorTrend* i = indicatorObject<IndicatorTrend>();
		Q_ASSERT(i);

		return i->samplePeriod();
	}

	E::TimeType SchemaItemIndicator::trendTimeType() const
	{
		Q_ASSERT(isTrend() == true);

		const IndicatorTrend* i = indicatorObject<IndicatorTrend>();
		Q_ASSERT(i);

		return i->timeType();
	}

	int SchemaItemIndicator::trendDurationSeconds() const
	{
		Q_ASSERT(isTrend() == true);

		const IndicatorTrend* i = indicatorObject<IndicatorTrend>();
		Q_ASSERT(i);

		return i->durationSeconds();
	}

	// IMatsSchemaItemAssociations implementation.
	//
	QStringList SchemaItemIndicator::associatedAppSignalIds() const
	{
		return signalIds();
	}

	QStringList SchemaItemIndicator::associatedImpactAppSignalIds() const
	{
		return {};
	}

	QStringList SchemaItemIndicator::associatedConnectionIds() const
	{
		return {};
	}

	QStringList SchemaItemIndicator::associatedLoopbackIds() const
	{
		return {};
	}

	QStringList SchemaItemIndicator::associatedSchemaItemLabels() const
	{
		return {};
	}

	// Properties and Data
	//
	// AppSignalIDs
	//
	QString SchemaItemIndicator::signalIdsString() const
	{
		auto context = this->context();
		return signalIdsString(context.get());
	}

	QString SchemaItemIndicator::signalIdsString(const Context* context) const
	{
		QString result = m_signalIds.join(QChar::LineFeed);

		// Expand variables in AppSignalIDs in MonitorMode
		//
		if (context != nullptr &&
			context->appSignalController() != nullptr &&
			context->viewVariables() != nullptr)
		{
			result = MacrosExpander::parse(result, context, nullptr, this);
		}

		return result;
	}

	void SchemaItemIndicator::setSignalIdsString(const QString& value)
	{
		setSignalIds(value.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts));
	}

	QStringList SchemaItemIndicator::signalIds() const
	{
		auto context = this->context();
		return signalIds(context.get());
	}

	QStringList SchemaItemIndicator::signalIds(const Context* context) const
	{
		QStringList resultList = m_signalIds;

		// Expand variables in AppSignalIDs in MonitorMode
		//
		if (context != nullptr &&
			context->appSignalController() != nullptr &&
			context->viewVariables() != nullptr)
		{
			resultList = MacrosExpander::parse(resultList, context, nullptr, this);
		}

		return resultList;
	}

	void SchemaItemIndicator::setSignalIds(const QStringList& value)
	{
		m_signalIds = value;

		while (m_signalIds.size() > MaxSignalsCound)
		{
			m_signalIds.removeLast();
		}

		return;
	}

	E::IndicatorType SchemaItemIndicator::indicatorType() const
	{
		return m_indicatorType;
	}

	void SchemaItemIndicator::setIndicatorType(E::IndicatorType value)
	{
		if (static_cast<size_t>(value) >= m_indicatorObjects.size())
		{
			Q_ASSERT(static_cast<size_t>(value) < m_indicatorObjects.size());
			value = E::IndicatorType::HistogramVert;
		}

		m_indicatorType = value;		// Getter will other object with different properties

		updateIndicatorProperties();

		return;
	}

	void SchemaItemIndicator::updateIndicatorProperties()
	{
		// Update specific properties for current indiocator type
		//
		removeCategoryProperties(PropertyNames::indicatorSettings);
		indicatorObject()->createProperties(this, static_cast<int>(m_signalIds.size()));

		// --
		//
		emit propertyListChanged();

		return;
	}

}

