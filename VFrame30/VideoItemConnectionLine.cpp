#include "Stable.h"
#include "VideoItemConnectionLine.h"

namespace VFrame30
{
	CVideoItemConnectionLine::CVideoItemConnectionLine(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	CVideoItemConnectionLine::CVideoItemConnectionLine(SchemeUnit unit) : 
		m_weight(0),
		m_lineColor(qRgb(0x00, 0x00, 0x00))
	{
		m_static = true;
		setItemUnit(unit);
	}


	CVideoItemConnectionLine::~CVideoItemConnectionLine(void)
	{
	}

	// Serialization
	//
	bool CVideoItemConnectionLine::SaveData(Proto::Envelope* message) const
	{
		bool result = CPosConnectionImpl::SaveData(message);
		if (result == false || message->has_videoitem() == false)
		{
			assert(result);
			assert(message->has_videoitem());
			return false;
		}
	
		// --
		//
		Proto::VideoItemConnectionLine* connectionMessage = message->mutable_videoitem()->mutable_connectionline();

		connectionMessage->set_weight(m_weight);
		connectionMessage->set_linecolor(m_lineColor);

		return true;
	}

	bool CVideoItemConnectionLine::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return false;
		}

		// --
		//
		bool result = CPosConnectionImpl::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.videoitem().has_connectionline() == false)
		{
			assert(message.videoitem().has_connectionline());
			return false;
		}

		const Proto::VideoItemConnectionLine& connectionMessage = message.videoitem().connectionline();

		m_weight = connectionMessage.weight();
		m_lineColor = connectionMessage.linecolor();

		return true;
	}

	// Drawing Functions
	//

	// Рисование элемента, выполняется в 100% масштабе.
	// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
	//
	void CVideoItemConnectionLine::Draw(CDrawParam* drawParam, const CVideoFrame*, const CVideoLayer*) const
	{
		if (drawParam == nullptr)
		{
			assert(drawParam);
			return;
		}

		QPainter* p = drawParam->painter();

		// Draw the main part
		//
		const std::list<VideoItemPoint>& poinlist = GetPointList();

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
	double CVideoItemConnectionLine::weight() const
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

	void CVideoItemConnectionLine::setWeight(double weight)
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

	// LineColor property
	//
	QRgb CVideoItemConnectionLine::lineColor() const
	{
		return m_lineColor;
	}

	void CVideoItemConnectionLine::setLineColor(QRgb color)
	{
		m_lineColor = color;
	}

}

