#include "Stable.h"
#include "VideoItemLine.h"

namespace VFrame30
{
	VideoItemLine::VideoItemLine(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	VideoItemLine::VideoItemLine(SchemeUnit unit) : 
		m_weight(0),
		m_lineColor(qRgb(0x00, 0x00, 0x00))
	{
		m_static = true;
		setItemUnit(unit);
	}


	VideoItemLine::~VideoItemLine(void)
	{
	}

	// Serialization
	//
	bool VideoItemLine::SaveData(Proto::Envelope* message) const
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
		Proto::VideoItemLine* lineMessage = message->mutable_schemeitem()->mutable_line();

		lineMessage->set_weight(m_weight);
		lineMessage->set_linecolor(m_lineColor);

		return true;
	}

	bool VideoItemLine::LoadData(const Proto::Envelope& message)
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

		const Proto::VideoItemLine& lineMessage = message.schemeitem().line();

		m_weight = lineMessage.weight();
		m_lineColor = lineMessage.linecolor();

		return true;
	}

	// Drawing Functions
	//

	// Рисование элемента, выполняется в 100% масштабе.
	// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
	//
	void VideoItemLine::Draw(CDrawParam* drawParam, const Scheme*, const SchemeLayer*) const
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
	double VideoItemLine::weight() const
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			return CUtils::RoundDisplayPoint(m_weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(m_weight, SchemeUnit::Inch, Settings::regionalUnit(), ConvertDirection::Horz);
			return CUtils::RoundPoint(pt, Settings::regionalUnit());
		}
	}

	void VideoItemLine::setWeight(double weight)
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			m_weight = CUtils::RoundDisplayPoint(weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(weight, Settings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Horz);
			m_weight = pt;
		}
	}

	// LineColor propertie
	//
	QRgb VideoItemLine::lineColor() const
	{
		return m_lineColor;
	}

	void VideoItemLine::setLineColor(QRgb color)
	{
		m_lineColor = color;
	}

}

