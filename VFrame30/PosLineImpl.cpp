#include "PosLineImpl.h"
#include "DrawParam.h"
#include "PropertyNames.h"

namespace VFrame30
{
	PosLineImpl::PosLineImpl(void)
	{
		Init();
	}

	void PosLineImpl::Init()
	{
		m_startXDocPt = 0;
		m_startYDocPt = 0;
		m_endXDocPt = 0;
		m_endYDocPt = 0;	
	}

	void PosLineImpl::propertyDemand(const QString& prop)
	{
		SchemaItem::propertyDemand(prop);

		int precision = 0;
		if (itemUnit() == SchemaUnit::Display)
		{
			precision = 0;
		}
		else
		{
			precision = Settings::regionalUnit() == SchemaUnit::Millimeter ? 1 : 3;		// 1 for mm, 3 for inches
		}

		Property* p = nullptr;

		p = addProperty<double, PosLineImpl, &PosLineImpl::left, &PosLineImpl::setLeft>(PropertyNames::left, PropertyNames::positionAndSizeCategory, true);
		p->setPrecision(precision);
		p->setViewOrder(0);

		p = addProperty<double, PosLineImpl, &PosLineImpl::top, &PosLineImpl::setTop>(PropertyNames::top, PropertyNames::positionAndSizeCategory, true);
		p->setPrecision(precision);
		p->setViewOrder(1);

		p = addProperty<double, PosLineImpl, &PosLineImpl::width, &PosLineImpl::setWidth>(PropertyNames::width, PropertyNames::positionAndSizeCategory, true);
		p->setPrecision(precision);
		p->setViewOrder(2);

		p = addProperty<double, PosLineImpl, &PosLineImpl::height, &PosLineImpl::setHeight>(PropertyNames::height, PropertyNames::positionAndSizeCategory, true);
		p->setPrecision(precision);
		p->setViewOrder(3);

		return;
	}

	// Serialization
	//
	bool PosLineImpl::SaveData(Proto::Envelope* message) const
	{
		bool result = SchemaItem::SaveData(message);
		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}

		// --
		//
		Proto::PosLineImpl* posLineImplMessage = message->mutable_schemaitem()->mutable_poslineimpl();

		posLineImplMessage->set_startxdocpt(m_startXDocPt);
		posLineImplMessage->set_startydocpt(m_startYDocPt);
		posLineImplMessage->set_endxdocpt(m_endXDocPt);
		posLineImplMessage->set_endydocpt(m_endYDocPt);

		return true;
	}

	bool PosLineImpl::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return false;
		}

		// --
		//
		bool result = SchemaItem::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_poslineimpl() == false)
		{
			assert(message.schemaitem().has_poslineimpl());
			return false;
		}

		const Proto::PosLineImpl& posLineImplMessage = message.schemaitem().poslineimpl();

		m_startXDocPt = posLineImplMessage.startxdocpt();
		m_startYDocPt = posLineImplMessage.startydocpt();
		m_endXDocPt = posLineImplMessage.endxdocpt();
		m_endYDocPt = posLineImplMessage.endydocpt();

		return true;
	}

	// Action Functions
	//
	void PosLineImpl::moveItem(double horzOffsetDocPt, double vertOffsetDocPt)
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

	// ��������� �������� ��� ��� �������� ���������
	//
	void PosLineImpl::drawOutline(CDrawParam* drawParam) const
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
			// ������ �����, �������� ����� �������
			//
			return;
		}

		QPen pen(Qt::black);
		pen.setWidth(0);
		p->setPen(pen);

		p->drawLine(p1, p2);

		return;
	}

	// ���������� ��������� �������, � ����������� �� ������������� ���������� ������������.
	//
	void PosLineImpl::drawSelection(CDrawParam* drawParam, bool drawSizeBar) const
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
			// ������ �����, �������� ����� �������
			//
			return;
		}

		double cbs = drawParam->controlBarSize();

		double lineWeight = cbs / 2.0f;

		QPen pen(isLocked() == true ?  SchemaItem::lockedSelectionColor : SchemaItem::selectionColor);

		pen.setWidthF(lineWeight);
		p->setPen(pen);

		p->drawLine(p1, p2);
		
		// Draw control bars
		//
		if (drawSizeBar == true && isLocked() == false)
		{
			QRectF r1(p1.x() - cbs / 2.0f, p1.y() - cbs / 2.0f, cbs, cbs);
			QRectF r2(p2.x() - cbs / 2.0f, p2.y() - cbs / 2.0f, cbs, cbs);

			QBrush solidBrush(pen.color());

			p->fillRect(r1, solidBrush);
			p->fillRect(r2, solidBrush);
		}

		return;
	}

	void PosLineImpl::drawCompareAction(CDrawParam* drawParam, QColor color) const
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
			// ������ �����, �������� ����� �������
			//
			return;
		}

		double cbs = drawParam->controlBarSize();
		double lineWeight = cbs / 2.0f;

		QPen pen(color);

		pen.setWidthF(lineWeight);
		p->setPen(pen);

		p->drawLine(p1, p2);

		return;
	}

	bool PosLineImpl::isIntersectRect(double x, double y, double width, double height) const
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

	QRectF PosLineImpl::boundingRectInDocPt(CDrawParam* /*drawParam*/) const
	{
		QRectF result(
			std::min(m_startXDocPt, m_endXDocPt), 
			std::min(m_startYDocPt, m_endYDocPt), 
			std::max(m_startXDocPt, m_endXDocPt) - std::min(m_startXDocPt, m_endXDocPt), 
			std::max(m_startYDocPt, m_endYDocPt) - std::min(m_startYDocPt, m_endYDocPt));

		return result;
	}

	// IPosLine
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

	// ���������� ����������� ISchemaItemPropertiesPos
	//
	double PosLineImpl::left() const 
	{
		double pt = std::min(m_startXDocPt, m_endXDocPt);		// Value in UnitDocPt

		if (itemUnit() == SchemaUnit::Display)
		{
			pt = CUtils::RoundDisplayPoint(pt);
		}
		else
		{
			pt = CUtils::ConvertPoint(pt, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			pt = CUtils::RoundPoint(pt, Settings::regionalUnit());
		}

		return pt;
	}
	void PosLineImpl::setLeft(const double& value)
	{
		double left = std::min(m_startXDocPt, m_endXDocPt);

		if (itemUnit() != SchemaUnit::Display)
		{
			left = CUtils::ConvertPoint(left, SchemaUnit::Inch, Settings::regionalUnit(), 0);
		}

		double delta = value - left;

		if (itemUnit() != SchemaUnit::Display)
		{
			delta = CUtils::ConvertPoint(delta, Settings::regionalUnit(), SchemaUnit::Inch, 0);
		}

		m_startXDocPt = m_startXDocPt + delta;
		m_endXDocPt = m_endXDocPt + delta;
	}

	double PosLineImpl::top() const
	{
		double pt = std::min(m_startYDocPt, m_endYDocPt);		// Value in UnitDocPt

		if (itemUnit() == SchemaUnit::Display)
		{
			pt = CUtils::RoundDisplayPoint(pt);
		}
		else
		{
			pt = CUtils::ConvertPoint(pt, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			pt = CUtils::RoundPoint(pt, Settings::regionalUnit());
		}
				
		return pt;
	}
	void PosLineImpl::setTop(const double& value)
	{
		double top = std::min(m_startYDocPt, m_endYDocPt);

		if (itemUnit() != SchemaUnit::Display)
		{
			top = CUtils::ConvertPoint(top, SchemaUnit::Inch, Settings::regionalUnit(), 0);
		}

		double delta = value - top;

		if (itemUnit() != SchemaUnit::Display)
		{
			delta = CUtils::ConvertPoint(delta, Settings::regionalUnit(), SchemaUnit::Inch, 0);
		}

		m_startYDocPt = m_startYDocPt + delta;
		m_endYDocPt = m_endYDocPt + delta;
	}

	double PosLineImpl::width() const
	{
		double pt = std::abs(m_startXDocPt - m_endXDocPt);

		if (itemUnit() == SchemaUnit::Display)
		{
			pt = CUtils::RoundDisplayPoint(pt);
		}
		else
		{
			pt = CUtils::ConvertPoint(pt, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			pt = CUtils::RoundPoint(pt, Settings::regionalUnit());
		}

		return pt;
	}
	void PosLineImpl::setWidth(const double& value)
	{
		double pt = value < 0 ? 0 : value;

		if (itemUnit() == SchemaUnit::Display)
		{
		}
		else
		{
			pt = CUtils::ConvertPoint(pt, Settings::regionalUnit(), SchemaUnit::Inch, 0);
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

		if (itemUnit() == SchemaUnit::Display)
		{
			pt = CUtils::RoundDisplayPoint(pt);
		}
		else
		{
			pt = CUtils::ConvertPoint(pt, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			pt = CUtils::RoundPoint(pt, Settings::regionalUnit());
		}

		return pt;	
	}
	void PosLineImpl::setHeight(const double& value)
	{
		double pt = value < 0 ? 0 : value;

		if (itemUnit() == SchemaUnit::Display)
		{
		}
		else
		{
			pt = CUtils::ConvertPoint(pt, Settings::regionalUnit(), SchemaUnit::Inch, 0);
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

	std::vector<SchemaPoint> PosLineImpl::getPointList() const
	{
		std::vector<SchemaPoint> v(2);

		v[0] = SchemaPoint(m_startXDocPt, m_startYDocPt);
		v[1] = SchemaPoint(m_endXDocPt, m_endYDocPt);

		return v;
	}

	void PosLineImpl::setPointList(const std::vector<SchemaPoint>& points)
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

