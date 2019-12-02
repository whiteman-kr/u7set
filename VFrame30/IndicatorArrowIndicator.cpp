#include "IndicatorArrowIndicator.h"
#include "SchemaItemIndicator.h"
#include "PropertyNames.h"
#include "DrawParam.h"

namespace VFrame30
{
	//
	// ArrowIndicator
	//
	IndicatorArrowIndicator::IndicatorArrowIndicator(SchemaUnit itemUnit) :
		Indicator(itemUnit)
	{
	}

	void IndicatorArrowIndicator::createProperties(SchemaItemIndicator* propertyObject, int /*signalCount*/)
	{
		propertyObject->ADD_PROPERTY_GETTER_SETTER(double, PropertyNames::indicatorStartValue, true, IndicatorArrowIndicator::startValue, IndicatorArrowIndicator::setStartValue)
				->setCategory(PropertyNames::indicatorSettings)
				.setViewOrder(0);

		propertyObject->ADD_PROPERTY_GETTER_SETTER(double, PropertyNames::indicatorEndValue, true, IndicatorArrowIndicator::endValue, IndicatorArrowIndicator::setEndValue)
				->setCategory(PropertyNames::indicatorSettings)
				.setViewOrder(1);

		propertyObject->ADD_PROPERTY_GETTER_SETTER(double, PropertyNames::indicatorStartAngle, true, IndicatorArrowIndicator::startAngle, IndicatorArrowIndicator::setStartAngle)
				->setCategory(PropertyNames::indicatorSettings)
				.setViewOrder(2);

		propertyObject->ADD_PROPERTY_GETTER_SETTER(double, PropertyNames::indicatorSpanAngle, true, IndicatorArrowIndicator::spanAngle, IndicatorArrowIndicator::setSpanAngle)
				->setCategory(PropertyNames::indicatorSettings)
				.setViewOrder(3);

		return;
	}

	bool IndicatorArrowIndicator::load(const Proto::SchemaItemIndicator& message, SchemaUnit unit)
	{
		m_itemUnit = unit;

		if (message.has_indicatorarrowindicator() == false)					// Line to change 1
		{
			// It can be just added new item, default values are taken
			//
			return true;
		}

		const ::Proto::IndicatorArrowIndicator& m = message.indicatorarrowindicator();	// Line to change 2

		m_startValue = m.startvalue();
		m_endValue = m.endvalue();

		m_startAngle = m.startangle();
		m_spanAngle = m.spanangle();

		return true;
	}

	bool IndicatorArrowIndicator::save(Proto::SchemaItemIndicator* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		auto m = message->mutable_indicatorarrowindicator();		// Line to change 1

		m->set_startvalue(m_startValue);
		m->set_endvalue(m_endValue);

		m->set_startangle(m_startAngle);
		m->set_spanangle(m_spanAngle);

		return true;
	}

	void IndicatorArrowIndicator::draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer, const SchemaItemIndicator* item) const
	{
		if (drawParam == nullptr ||
			schema == nullptr ||
			layer == nullptr ||
			item == nullptr)
		{
			Q_ASSERT(drawParam);
			Q_ASSERT(schema);
			Q_ASSERT(layer);
			Q_ASSERT(item);

			return;
		}

		QPainter* p = drawParam->painter();
		Q_ASSERT(p);

		QRectF rect{item->boundingRectInDocPt(drawParam)};

		p->fillRect(rect, item->backgroundColor());

		return;
	}

	double IndicatorArrowIndicator::startValue() const
	{
		return m_startValue;
	}

	void IndicatorArrowIndicator::setStartValue(double value)
	{
		m_startValue = value;
	}

	double IndicatorArrowIndicator::endValue() const
	{
		return m_endValue;
	}

	void IndicatorArrowIndicator::setEndValue(double value)
	{
		m_endValue = value;
	}

	double IndicatorArrowIndicator::startAngle() const
	{
		return m_startAngle;
	}

	void IndicatorArrowIndicator::setStartAngle(double value)
	{
		m_startAngle = qBound(0.0, value, 360.0);
	}

	double IndicatorArrowIndicator::spanAngle() const
	{
		return m_spanAngle;
	}

	void IndicatorArrowIndicator::setSpanAngle(double value)
	{
		m_spanAngle = qBound(1.0, value, 360.0);
	}
}
