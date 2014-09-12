#include "Stable.h"
#include "PosLineImpl.h"

namespace VFrame30
{
	CPosLineImpl::CPosLineImpl(void)
	{
		Init();
	}

	CPosLineImpl::~CPosLineImpl(void)
	{
	}

	void CPosLineImpl::Init()
	{
		m_startXDocPt = 0;
		m_startYDocPt = 0;
		m_endXDocPt = 0;
		m_endYDocPt = 0;	
	}

	// Serialization
	//
	bool CPosLineImpl::SaveData(::Proto::Envelope* message) const
	{
		bool result = CVideoItem::SaveData(message);
		if (result == false || message->has_videoitem() == false)
		{
			assert(result);
			assert(message->has_videoitem());
			return false;
		}

		// --
		//
		::Proto::PosLineImpl* posLineImplMessage = message->mutable_videoitem()->mutable_poslineimpl();

		posLineImplMessage->set_startxdocpt(m_startXDocPt);
		posLineImplMessage->set_startydocpt(m_startYDocPt);
		posLineImplMessage->set_endxdocpt(m_endXDocPt);
		posLineImplMessage->set_endydocpt(m_endYDocPt);

		return true;
	}

	bool CPosLineImpl::LoadData(const ::Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return false;
		}

		// --
		//
		bool result = CVideoItem::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.videoitem().has_poslineimpl() == false)
		{
			assert(message.videoitem().has_poslineimpl());
			return false;
		}

		const ::Proto::PosLineImpl& posLineImplMessage = message.videoitem().poslineimpl();

		m_startXDocPt = posLineImplMessage.startxdocpt();
		m_startYDocPt = posLineImplMessage.startydocpt();
		m_endXDocPt = posLineImplMessage.endxdocpt();
		m_endYDocPt = posLineImplMessage.endydocpt();

		return true;
	}

	// Action Functions
	//
	void CPosLineImpl::MoveItem(double horzOffsetDocPt, double vertOffsetDocPt)
	{ 
		setStartXDocPt(startXDocPt() + horzOffsetDocPt);
		setStartYDocPt(startYDocPt() + vertOffsetDocPt);

		setEndXDocPt(endXDocPt() + horzOffsetDocPt);
		setEndYDocPt(endYDocPt() + vertOffsetDocPt);
	}

	double CPosLineImpl::GetWidthInDocPt() const
	{
		double val = std::abs(m_startXDocPt - m_endXDocPt);
		return val;
	}

	double CPosLineImpl::GetHeightInDocPt() const
	{
		double val = std::abs(m_startYDocPt - m_endYDocPt);
		return val;
	}

	void CPosLineImpl::SetWidthInDocPt(double val)
	{
		if (val < 0)
		{
			val = 0;
		}

		if (m_endXDocPt >= m_startXDocPt)
		{
			m_endXDocPt = m_startXDocPt + val;
		}
		else
		{
			m_startXDocPt = m_endXDocPt + val;
		}		
	}

	void CPosLineImpl::SetHeightInDocPt(double val)
	{
		if (val < 0)
		{
			val = 0;
		}

		if (m_endYDocPt >= m_startYDocPt)
		{
			m_endYDocPt = m_startYDocPt + val;
		}
		else
		{
			m_startYDocPt = m_endYDocPt + val;
		}		
	}

	// Рисование элемента при его создании изменении
	//
	void CPosLineImpl::DrawOutline(CDrawParam* drawParam) const
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

		QPen pen(Qt::black);
		pen.setWidth(0);
		p->setPen(pen);

		p->drawLine(p1, p2);

		return;
	}

	// Нарисовать выделение объекта, в зависимости от используемого интрефейса расположения.
	//
	void CPosLineImpl::DrawSelection(CDrawParam* drawParam, bool drawSizeBar) const
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

		double cbs = drawParam->controlBarSize();

		double lineWeight = cbs / 2.0f;

		QPen pen(QColor(0x33, 0x99, 0xFF, 0x80));

		pen.setWidthF(lineWeight);
		p->setPen(pen);

		p->drawLine(p1, p2);
		
		// Draw control bars
		//
		if (drawSizeBar == true)
		{
			QRectF r1(p1.x() - cbs / 2.0f, p1.y() - cbs / 2.0f, cbs, cbs);
			QRectF r2(p2.x() - cbs / 2.0f, p2.y() - cbs / 2.0f, cbs, cbs);

			QBrush solidBrush(pen.color());

			p->fillRect(r1, solidBrush);
			p->fillRect(r2, solidBrush);
		}

		return;
	}	

	bool CPosLineImpl::IsIntersectRect(double x, double y, double width, double height) const
	{
		double ax1 = startXDocPt();
		double ay1 = startYDocPt();
		double ax2 = endXDocPt();
		double ay2 = endYDocPt();

		QRectF detRect(x, y, width, height);

		if (detRect.contains(ax1, ay1) == true ||
			detRect.contains(ax2, ay2) == true)
		{
			return true;
		}

		if (CVFrameUtils::IsLineIntersected(ax1, ay1, ax2, ay2,
			detRect.x(), detRect.y(),
			detRect.x() + detRect.width(), detRect.y()) == true)
		{
			return true;
		}

		if (CVFrameUtils::IsLineIntersected(ax1, ay1, ax2, ay2,
			detRect.x() + detRect.width(), detRect.y(),
			detRect.x() + detRect.width(), detRect.y() + detRect.height()) == true)
		{
			return true;
		}

		if (CVFrameUtils::IsLineIntersected(ax1, ay1, ax2, ay2,
			detRect.x() + detRect.width(), detRect.y() + detRect.height(),
			detRect.x(), detRect.y() + detRect.height()) == true)
		{
			return true;
		}

		if (CVFrameUtils::IsLineIntersected(ax1, ay1, ax2, ay2,
			detRect.x(), detRect.y() + detRect.height(),
			detRect.x(), detRect.y()) == true)
		{
			return true;
		}

		return false;
	}

	QRectF CPosLineImpl::boundingRectInDocPt() const
	{
		QRectF result(
			std::min(m_startXDocPt, m_endXDocPt), 
			std::min(m_startYDocPt, m_endYDocPt), 
			std::max(m_startXDocPt, m_endXDocPt) - std::min(m_startXDocPt, m_endXDocPt), 
			std::max(m_startYDocPt, m_endYDocPt) - std::min(m_startYDocPt, m_endYDocPt));

		return result;
	}

	// IVideoItemPosLine implementation
	//
	double CPosLineImpl::startXDocPt() const
	{
		return m_startXDocPt;
	}
	void CPosLineImpl::setStartXDocPt(double value)
	{
		m_startXDocPt = value;
	}

	double CPosLineImpl::startYDocPt() const
	{
		return m_startYDocPt;
	}
	void CPosLineImpl::setStartYDocPt(double value)
	{
		m_startYDocPt = value;
	}

	double CPosLineImpl::endXDocPt() const
	{
		return m_endXDocPt;
	}
	void CPosLineImpl::setEndXDocPt(double value)
	{
		m_endXDocPt = value;
	}

	double CPosLineImpl::endYDocPt() const
	{
		return m_endYDocPt;
	}
	void CPosLineImpl::setEndYDocPt(double value)
	{
		m_endYDocPt = value;
	}

	// Реализация интерефейса IVideoItemPropertiesPos
	//
	double CPosLineImpl::left() const 
	{
		double pt = std::min(m_startXDocPt, m_endXDocPt);		// Value in UnitDocPt

		if (itemUnit() == SchemeUnit::Display)
		{
			pt = CVFrameUtils::RoundDisplayPoint(pt);
		}
		else
		{
			pt = CVFrameUtils::ConvertPoint(pt, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Horz);
			pt = CVFrameUtils::RoundPoint(pt, CSettings::regionalUnit());
		}

		return pt;
	}
	void CPosLineImpl::setLeft(double value)
	{
		double left = std::min(m_startXDocPt, m_endXDocPt);

		if (itemUnit() != SchemeUnit::Display)
		{
			left = CVFrameUtils::ConvertPoint(left, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Horz);
		}

		double delta = value - left;

		if (itemUnit() != SchemeUnit::Display)
		{
			delta = CVFrameUtils::ConvertPoint(delta, CSettings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Horz);
		}

		m_startXDocPt = m_startXDocPt + delta;
		m_endXDocPt = m_endXDocPt + delta;
	}

	double CPosLineImpl::top() const
	{
		double pt = std::min(m_startYDocPt, m_endYDocPt);		// Value in UnitDocPt

		if (itemUnit() == SchemeUnit::Display)
		{
			pt = CVFrameUtils::RoundDisplayPoint(pt);
		}
		else
		{
			pt = CVFrameUtils::ConvertPoint(pt, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Vert);
			pt = CVFrameUtils::RoundPoint(pt, CSettings::regionalUnit());
		}
				
		return pt;
	}
	void CPosLineImpl::setTop(double value)
	{
		double top = std::min(m_startYDocPt, m_endYDocPt);

		if (itemUnit() != SchemeUnit::Display)
		{
			top = CVFrameUtils::ConvertPoint(top, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Vert);
		}

		double delta = value - top;

		if (itemUnit() != SchemeUnit::Display)
		{
			delta = CVFrameUtils::ConvertPoint(delta, CSettings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Vert);
		}

		m_startYDocPt = m_startYDocPt + delta;
		m_endYDocPt = m_endYDocPt + delta;
	}

	double CPosLineImpl::width() const
	{
		double pt = std::abs(m_startXDocPt - m_endXDocPt);

		if (itemUnit() == SchemeUnit::Display)
		{
			pt = CVFrameUtils::RoundDisplayPoint(pt);
		}
		else
		{
			pt = CVFrameUtils::ConvertPoint(pt, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Horz);
			pt = CVFrameUtils::RoundPoint(pt, CSettings::regionalUnit());
		}

		return pt;
	}
	void CPosLineImpl::setWidth(double value)
	{
		double pt = value;
		if (pt < 0)
		{
			pt = 0;
		}

		if (itemUnit() == SchemeUnit::Display)
		{
		}
		else
		{
			pt = CVFrameUtils::ConvertPoint(pt, CSettings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Horz);
		}

		if (m_endXDocPt >= m_startXDocPt)
		{
			m_endXDocPt = m_startXDocPt + pt;
		}
		else
		{
			m_startXDocPt = m_endXDocPt + pt;
		}
	}

	double CPosLineImpl::height() const
	{
		double pt = std::abs(m_startYDocPt - m_endYDocPt);

		if (itemUnit() == SchemeUnit::Display)
		{
			pt = CVFrameUtils::RoundDisplayPoint(pt);
		}
		else
		{
			pt = CVFrameUtils::ConvertPoint(pt, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Vert);
			pt = CVFrameUtils::RoundPoint(pt, CSettings::regionalUnit());
		}

		return pt;	
	}
	void CPosLineImpl::setHeight(double value)
	{
		double pt = value;
		if (pt < 0)
		{
			pt = 0;
		}

		if (itemUnit() == SchemeUnit::Display)
		{
		}
		else
		{
			pt = CVFrameUtils::ConvertPoint(pt, CSettings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Vert);
		}

		if (m_endYDocPt >= m_startYDocPt)
		{
			m_endYDocPt = m_startYDocPt + pt;
		}
		else
		{
			m_startYDocPt = m_endYDocPt + pt;
		}
	}

	std::vector<VideoItemPoint> CPosLineImpl::getPointList() const
	{
		std::vector<VideoItemPoint> v(2);

		v[0] = VideoItemPoint(m_startXDocPt, m_startYDocPt);
		v[1] = VideoItemPoint(m_endXDocPt, m_endYDocPt);

		return v;
	}

	void CPosLineImpl::setPointList(const std::vector<VideoItemPoint>& points)
	{
		if (points.size() != 2)
		{
			assert(points.size() == 2);
			return;
		}

		m_startXDocPt = points.front().X;
		m_startYDocPt = points.front().Y;

		m_endXDocPt = points.back().X;
		m_endYDocPt = points.back().Y;

		return;
	}
}

