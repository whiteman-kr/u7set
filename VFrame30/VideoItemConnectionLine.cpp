#include "Stable.h"
#include "VideoItemConnectionLine.h"

namespace VFrame30
{
	VideoItemConnectionLine::VideoItemConnectionLine(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	VideoItemConnectionLine::VideoItemConnectionLine(SchemeUnit unit) : 
		m_weight(0),
		m_lineColor(qRgb(0x00, 0x00, 0x00))
	{
		m_static = true;
		setItemUnit(unit);
	}


	VideoItemConnectionLine::~VideoItemConnectionLine(void)
	{
	}

	// Serialization
	//
	bool VideoItemConnectionLine::SaveData(Proto::Envelope* message) const
	{
		bool result = PosConnectionImpl::SaveData(message);
		if (result == false || message->has_schemeitem() == false)
		{
			assert(result);
			assert(message->has_schemeitem());
			return false;
		}
	
		// --
		//
		Proto::VideoItemConnectionLine* connectionMessage = message->mutable_schemeitem()->mutable_connectionline();

		connectionMessage->set_weight(m_weight);
		connectionMessage->set_linecolor(m_lineColor);

		return true;
	}

	bool VideoItemConnectionLine::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemeitem() == false)
		{
			assert(message.has_schemeitem());
			return false;
		}

		// --
		//
		bool result = PosConnectionImpl::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemeitem().has_connectionline() == false)
		{
			assert(message.schemeitem().has_connectionline());
			return false;
		}

		const Proto::VideoItemConnectionLine& connectionMessage = message.schemeitem().connectionline();

		m_weight = connectionMessage.weight();
		m_lineColor = connectionMessage.linecolor();

		return true;
	}

	// Drawing Functions
	//

	// Рисование элемента, выполняется в 100% масштабе.
	// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
	//
	void VideoItemConnectionLine::Draw(CDrawParam* drawParam, const Scheme*, const SchemeLayer*) const
	{
		if (drawParam == nullptr)
		{
			assert(drawParam);
			return;
		}

		QPainter* p = drawParam->painter();

		// Draw the main part
		//
		const std::list<SchemePoint>& poinlist = GetPointList();

		QPolygonF polyline(static_cast<int>(poinlist.size()));
		int index = 0;

		for (auto pt = poinlist.cbegin(); pt != poinlist.cend(); ++pt)
		{
			polyline[index++] = QPointF(pt->X, pt->Y);
		}

		QPen pen(lineColor());
		pen.setWidthF(m_weight);
		p->setPen(pen);

		p->drawPolyline(polyline);

		return;
	}

	// Properties and Data
	//

	// Weight property
	//
	double VideoItemConnectionLine::weight() const
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

	void VideoItemConnectionLine::setWeight(double weight)
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

	// LineColor property
	//
	QRgb VideoItemConnectionLine::lineColor() const
	{
		return m_lineColor;
	}

	void VideoItemConnectionLine::setLineColor(QRgb color)
	{
		m_lineColor = color;
	}

}

