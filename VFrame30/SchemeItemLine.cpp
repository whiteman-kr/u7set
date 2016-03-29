#include "Stable.h"
#include "SchemeItemLine.h"

namespace VFrame30
{
	SchemeItemLine::SchemeItemLine(void) :
		SchemeItemLine(SchemaUnit::Inch)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemeItemLine::SchemeItemLine(SchemaUnit unit) : 
		m_weight(0),
		m_lineColor(qRgb(0x00, 0x00, 0x00))
	{
		ADD_PROPERTY_GETTER_SETTER(double, LineWeight, true, SchemeItemLine::weight, SchemeItemLine::setWeight);

		// --
		//
		m_static = true;
		setItemUnit(unit);
	}


	SchemeItemLine::~SchemeItemLine(void)
	{
	}

	// Serialization
	//
	bool SchemeItemLine::SaveData(Proto::Envelope* message) const
	{
		bool result = PosLineImpl::SaveData(message);
		if (result == false || message->has_schemeitem() == false)
		{
			assert(result);
			assert(message->has_schemeitem());
			return false;
		}

		// --
		//
		Proto::SchemeItemLine* lineMessage = message->mutable_schemeitem()->mutable_line();

		lineMessage->set_weight(m_weight);
		lineMessage->set_linecolor(m_lineColor);

		return true;
	}

	bool SchemeItemLine::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemeitem() == false)
		{
			assert(message.has_schemeitem());
			return false;
		}

		// --
		//
		bool result = PosLineImpl::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		if (message.schemeitem().has_line() == false)
		{
			assert(message.schemeitem().has_line());
			return false;
		}

		const Proto::SchemeItemLine& lineMessage = message.schemeitem().line();

		m_weight = lineMessage.weight();
		m_lineColor = lineMessage.linecolor();

		return true;
	}

	// Drawing Functions
	//

	// Рисование элемента, выполняется в 100% масштабе.
	// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
	//
	void SchemeItemLine::Draw(CDrawParam* drawParam, const Schema*, const SchemaLayer*) const
	{
		if (drawParam == nullptr)
		{
			assert(drawParam);
			return;
		}

		QPainter* p = drawParam->painter();

		QPointF p1(startXDocPt(), startYDocPt());
		QPointF p2(endXDocPt(), endYDocPt());

		if (std::abs(p1.x() - p2.x()) < 0.000001 && std::abs(p1.y() - p2.y()) < 0.000001)
		{
			// Пустая линия, рисуется очень большой
			//
			return;
		}

		QPen pen(lineColor());
		pen.setWidthF(m_weight);
		p->setPen(pen);

		p->drawLine(p1, p2);

		return;
	}

	// Properties and Data
	//

	// Weight propertie
	//
	double SchemeItemLine::weight() const
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			return CUtils::RoundDisplayPoint(m_weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(m_weight, SchemaUnit::Inch, Settings::regionalUnit(), ConvertDirection::Horz);
			return CUtils::RoundPoint(pt, Settings::regionalUnit());
		}
	}

	void SchemeItemLine::setWeight(double weight)
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			m_weight = CUtils::RoundDisplayPoint(weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(weight, Settings::regionalUnit(), SchemaUnit::Inch, ConvertDirection::Horz);
			m_weight = pt;
		}
	}

	// LineColor propertie
	//
	QRgb SchemeItemLine::lineColor() const
	{
		return m_lineColor;
	}

	void SchemeItemLine::setLineColor(QRgb color)
	{
		m_lineColor = color;
	}

}

