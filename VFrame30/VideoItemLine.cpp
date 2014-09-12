#include "Stable.h"
#include "VideoItemLine.h"

namespace VFrame30
{
	CVideoItemLine::CVideoItemLine(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	CVideoItemLine::CVideoItemLine(SchemeUnit unit) : 
		m_weight(0),
		m_lineColor(qRgb(0x00, 0x00, 0x00))
	{
		m_static = true;
		setItemUnit(unit);
	}


	CVideoItemLine::~CVideoItemLine(void)
	{
	}

	// Serialization
	//
	bool CVideoItemLine::SaveData(::Proto::Envelope* message) const
	{
		bool result = CPosLineImpl::SaveData(message);
		if (result == false || message->has_videoitem() == false)
		{
			assert(result);
			assert(message->has_videoitem());
			return false;
		}

		// --
		//
		::Proto::VideoItemLine* lineMessage = message->mutable_videoitem()->mutable_line();

		lineMessage->set_weight(m_weight);
		lineMessage->set_linecolor(m_lineColor);

		return true;
	}

	bool CVideoItemLine::LoadData(const ::Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return false;
		}

		// --
		//
		bool result = CPosLineImpl::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		if (message.videoitem().has_line() == false)
		{
			assert(message.videoitem().has_line());
			return false;
		}

		const ::Proto::VideoItemLine& lineMessage = message.videoitem().line();

		m_weight = lineMessage.weight();
		m_lineColor = lineMessage.linecolor();

		return true;
	}

	// Drawing Functions
	//

	// Рисование элемента, выполняется в 100% масштабе.
	// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
	//
	void CVideoItemLine::Draw(CDrawParam* drawParam, const CVideoFrame*, const CVideoLayer*) const
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
	double CVideoItemLine::weight() const
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			return CVFrameUtils::RoundDisplayPoint(m_weight);
		}
		else
		{
			double pt = CVFrameUtils::ConvertPoint(m_weight, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Horz);
			return CVFrameUtils::RoundPoint(pt, CSettings::regionalUnit());
		}
	}

	void CVideoItemLine::setWeight(double weight)
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			m_weight = CVFrameUtils::RoundDisplayPoint(weight);
		}
		else
		{
			double pt = CVFrameUtils::ConvertPoint(weight, CSettings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Horz);
			m_weight = pt;
		}
	}

	// LineColor propertie
	//
	QRgb CVideoItemLine::lineColor() const
	{
		return m_lineColor;
	}

	void CVideoItemLine::setLineColor(QRgb color)
	{
		m_lineColor = color;
	}

}

