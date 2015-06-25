#include "Stable.h"
#include "PosLineImpl.h"

namespace VFrame30
{
	PosLineImpl::PosLineImpl(void)
	{
		Init();
	}

	PosLineImpl::~PosLineImpl(void)
	{
	}

	void PosLineImpl::Init()
	{
		m_startXDocPt = 0;
		m_startYDocPt = 0;
		m_endXDocPt = 0;
		m_endYDocPt = 0;	
	}

	// Serialization
	//
	bool PosLineImpl::SaveData(Proto::Envelope* message) const
	{
		bool result = SchemeItem::SaveData(message);
		if (result == false || message->has_schemeitem() == false)
		{
			assert(result);
			assert(message->has_schemeitem());
			return false;
		}

		// --
		//
		Proto::PosLineImpl* posLineImplMessage = message->mutable_schemeitem()->mutable_poslineimpl();

		posLineImplMessage->set_startxdocpt(m_startXDocPt);
		posLineImplMessage->set_startydocpt(m_startYDocPt);
		posLineImplMessage->set_endxdocpt(m_endXDocPt);
		posLineImplMessage->set_endydocpt(m_endYDocPt);

		return true;
	}

	bool PosLineImpl::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemeitem() == false)
		{
			assert(message.has_schemeitem());
			return false;
		}

		// --
		//
		bool result = SchemeItem::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemeitem().has_poslineimpl() == false)
		{
			assert(message.schemeitem().has_poslineimpl());
			return false;
		}

		const Proto::PosLineImpl& posLineImplMessage = message.schemeitem().poslineimpl();

		m_startXDocPt = posLineImplMessage.startxdocpt();
		m_startYDocPt = posLineImplMessage.startydocpt();
		m_endXDocPt = posLineImplMessage.endxdocpt();
		m_endYDocPt = posLineImplMessage.endydocpt();

		return true;
	}

	// Action Functions
	//
	void PosLineImpl::MoveItem(double horzOffsetDocPt, double vertOffsetDocPt)
	{ 
		setStartXDocPt(startXDocPt() + horzOffsetDocPt);
		setStartYDocPt(startYDocPt() + vertOffsetDocPt);

		setEndXDocPt(endXDocPt() + horzOffsetDocPt);
		setEndYDocPt(endYDocPt() + vertOffsetDocPt);
	}

	void PosLineImpl::snapToGrid(double gridSize)
	{
		QPointF sp = CUtils::snapToGrid(QPointF(startXDocPt(), startYDocPt()), gridSize);
		QPointF ep = CUtils::snapToGrid(QPointF(endXDocPt(), endYDocPt()), gridSize);

		setStartXDocPt(sp.x());
		setStartYDocPt(sp.y());
		setEndXDocPt(ep.x());
		setEndYDocPt(ep.y());

		return;
	}

	double PosLineImpl::GetWidthInDocPt() const
	{
		double val = std::abs(m_startXDocPt - m_endXDocPt);
		return val;
	}

	double PosLineImpl::GetHeightInDocPt() const
	{
		double val = std::abs(m_startYDocPt - m_endYDocPt);
		return val;
	}

	void PosLineImpl::SetWidthInDocPt(double val)
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

	void PosLineImpl::SetHeightInDocPt(double val)
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
	void PosLineImpl::DrawOutline(CDrawParam* drawParam) const
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
	void PosLineImpl::DrawSelection(CDrawParam* drawParam, bool drawSizeBar) const
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

	bool PosLineImpl::IsIntersectRect(double x, double y, double width, double height) const
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

		if (CUtils::IsLineIntersected(ax1, ay1, ax2, ay2,
			detRect.x(), detRect.y(),
			detRect.x() + detRect.width(), detRect.y()) == true)
		{
			return true;
		}

		if (CUtils::IsLineIntersected(ax1, ay1, ax2, ay2,
			detRect.x() + detRect.width(), detRect.y(),
			detRect.x() + detRect.width(), detRect.y() + detRect.height()) == true)
		{
			return true;
		}

		if (CUtils::IsLineIntersected(ax1, ay1, ax2, ay2,
			detRect.x() + detRect.width(), detRect.y() + detRect.height(),
			detRect.x(), detRect.y() + detRect.height()) == true)
		{
			return true;
		}

		if (CUtils::IsLineIntersected(ax1, ay1, ax2, ay2,
			detRect.x(), detRect.y() + detRect.height(),
			detRect.x(), detRect.y()) == true)
		{
			return true;
		}

		return false;
	}

	QRectF PosLineImpl::boundingRectInDocPt() const
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
	double PosLineImpl::startXDocPt() const
	{
		return m_startXDocPt;
	}
	void PosLineImpl::setStartXDocPt(double value)
	{
		m_startXDocPt = value;
	}

	double PosLineImpl::startYDocPt() const
	{
		return m_startYDocPt;
	}
	void PosLineImpl::setStartYDocPt(double value)
	{
		m_startYDocPt = value;
	}

	double PosLineImpl::endXDocPt() const
	{
		return m_endXDocPt;
	}
	void PosLineImpl::setEndXDocPt(double value)
	{
		m_endXDocPt = value;
	}

	double PosLineImpl::endYDocPt() const
	{
		return m_endYDocPt;
	}
	void PosLineImpl::setEndYDocPt(double value)
	{
		m_endYDocPt = value;
	}

	// Реализация интерефейса IVideoItemPropertiesPos
	//
	double PosLineImpl::left() const 
	{
		double pt = std::min(m_startXDocPt, m_endXDocPt);		// Value in UnitDocPt

		if (itemUnit() == SchemeUnit::Display)
		{
			pt = CUtils::RoundDisplayPoint(pt);
		}
		else
		{
			pt = CUtils::ConvertPoint(pt, SchemeUnit::Inch, Settings::regionalUnit(), ConvertDirection::Horz);
			pt = CUtils::RoundPoint(pt, Settings::regionalUnit());
		}

		return pt;
	}
	void PosLineImpl::setLeft(double value)
	{
		double left = std::min(m_startXDocPt, m_endXDocPt);

		if (itemUnit() != SchemeUnit::Display)
		{
			left = CUtils::ConvertPoint(left, SchemeUnit::Inch, Settings::regionalUnit(), ConvertDirection::Horz);
		}

		double delta = value - left;

		if (itemUnit() != SchemeUnit::Display)
		{
			delta = CUtils::ConvertPoint(delta, Settings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Horz);
		}

		m_startXDocPt = m_startXDocPt + delta;
		m_endXDocPt = m_endXDocPt + delta;
	}

	double PosLineImpl::top() const
	{
		double pt = std::min(m_startYDocPt, m_endYDocPt);		// Value in UnitDocPt

		if (itemUnit() == SchemeUnit::Display)
		{
			pt = CUtils::RoundDisplayPoint(pt);
		}
		else
		{
			pt = CUtils::ConvertPoint(pt, SchemeUnit::Inch, Settings::regionalUnit(), ConvertDirection::Vert);
			pt = CUtils::RoundPoint(pt, Settings::regionalUnit());
		}
				
		return pt;
	}
	void PosLineImpl::setTop(double value)
	{
		double top = std::min(m_startYDocPt, m_endYDocPt);

		if (itemUnit() != SchemeUnit::Display)
		{
			top = CUtils::ConvertPoint(top, SchemeUnit::Inch, Settings::regionalUnit(), ConvertDirection::Vert);
		}

		double delta = value - top;

		if (itemUnit() != SchemeUnit::Display)
		{
			delta = CUtils::ConvertPoint(delta, Settings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Vert);
		}

		m_startYDocPt = m_startYDocPt + delta;
		m_endYDocPt = m_endYDocPt + delta;
	}

	double PosLineImpl::width() const
	{
		double pt = std::abs(m_startXDocPt - m_endXDocPt);

		if (itemUnit() == SchemeUnit::Display)
		{
			pt = CUtils::RoundDisplayPoint(pt);
		}
		else
		{
			pt = CUtils::ConvertPoint(pt, SchemeUnit::Inch, Settings::regionalUnit(), ConvertDirection::Horz);
			pt = CUtils::RoundPoint(pt, Settings::regionalUnit());
		}

		return pt;
	}
	void PosLineImpl::setWidth(double value)
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
			pt = CUtils::ConvertPoint(pt, Settings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Horz);
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

	double PosLineImpl::height() const
	{
		double pt = std::abs(m_startYDocPt - m_endYDocPt);

		if (itemUnit() == SchemeUnit::Display)
		{
			pt = CUtils::RoundDisplayPoint(pt);
		}
		else
		{
			pt = CUtils::ConvertPoint(pt, SchemeUnit::Inch, Settings::regionalUnit(), ConvertDirection::Vert);
			pt = CUtils::RoundPoint(pt, Settings::regionalUnit());
		}

		return pt;	
	}
	void PosLineImpl::setHeight(double value)
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
			pt = CUtils::ConvertPoint(pt, Settings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Vert);
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

	std::vector<SchemePoint> PosLineImpl::getPointList() const
	{
		std::vector<SchemePoint> v(2);

		v[0] = SchemePoint(m_startXDocPt, m_startYDocPt);
		v[1] = SchemePoint(m_endXDocPt, m_endYDocPt);

		return v;
	}

	void PosLineImpl::setPointList(const std::vector<SchemePoint>& points)
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

