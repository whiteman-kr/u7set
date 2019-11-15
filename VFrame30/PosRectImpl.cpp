#include "PosRectImpl.h"
#include "DrawParam.h"
#include "PropertyNames.h"

namespace VFrame30
{
	PosRectImpl::PosRectImpl(void)
	{
		Init();
	}

	void PosRectImpl::Init()
	{
		m_leftDocPt = 0;
		m_topDocPt = 0;
		m_widthDocPt = 0;
		m_heightDocPt = 0;
	}

	void PosRectImpl::propertyDemand(const QString& prop)
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

		p = addProperty<double, PosRectImpl, &PosRectImpl::left, &PosRectImpl::setLeft>(PropertyNames::left, PropertyNames::positionAndSizeCategory, true);
		p->setPrecision(precision);
		p->setViewOrder(0);

		p = addProperty<double, PosRectImpl, &PosRectImpl::top, &PosRectImpl::setTop>(PropertyNames::top, PropertyNames::positionAndSizeCategory, true);
		p->setPrecision(precision);
		p->setViewOrder(1);

		p = addProperty<double, PosRectImpl, &PosRectImpl::width, &PosRectImpl::setWidth>(PropertyNames::width, PropertyNames::positionAndSizeCategory, true);
		p->setPrecision(precision);
		p->setViewOrder(2);

		p = addProperty<double, PosRectImpl, &PosRectImpl::height, &PosRectImpl::setHeight>(PropertyNames::height, PropertyNames::positionAndSizeCategory, true);
		p->setPrecision(precision);
		p->setViewOrder(3);

		return;
	}

	void PosRectImpl::dump() const
	{
		SchemaItem::dump();
		qDebug() << "\t x, y, w, h: " << m_leftDocPt << ", " << m_topDocPt << ", " << m_widthDocPt << ", " << m_heightDocPt;
	}

	bool PosRectImpl::SaveData(Proto::Envelope* message) const
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
		Proto::PosRectImpl* posRectImplMessage = message->mutable_schemaitem()->mutable_posrectimpl();

		posRectImplMessage->set_leftdocpt(m_leftDocPt);
		posRectImplMessage->set_topdocpt(m_topDocPt);
		posRectImplMessage->set_widthdocpt(m_widthDocPt);
		posRectImplMessage->set_heightdocpt(m_heightDocPt);

		return true;
	}

	bool PosRectImpl::LoadData(const Proto::Envelope& message)
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
		if (message.schemaitem().has_posrectimpl() == false)
		{
			assert(message.schemaitem().has_posrectimpl());
			return false;
		}

		const Proto::PosRectImpl& posRectImplMessage = message.schemaitem().posrectimpl();

		m_leftDocPt = posRectImplMessage.leftdocpt();
		m_topDocPt = posRectImplMessage.topdocpt();
		m_widthDocPt = posRectImplMessage.widthdocpt();
		m_heightDocPt = posRectImplMessage.heightdocpt();

		return true;
	}

	// Action Functions
	//
	void PosRectImpl::moveItem(double horzOffsetDocPt, double vertOffsetDocPt)
	{ 
		setLeftDocPt(leftDocPt() + horzOffsetDocPt);
		setTopDocPt(topDocPt() + vertOffsetDocPt);
	}

	void PosRectImpl::snapToGrid(double gridSize)
	{
		QPointF lt(leftDocPt(), topDocPt());
		QPointF br(leftDocPt() + widthDocPt(), topDocPt() + heightDocPt());

		QPointF leftTop = CUtils::snapToGrid(lt, gridSize);
		QPointF bottomRight = CUtils::snapToGrid(br, gridSize);

		setLeftDocPt(leftTop.x());
		setTopDocPt(leftTop.y());
		setWidthDocPt(bottomRight.x() - leftDocPt());
		setHeightDocPt(bottomRight.y() - topDocPt());

		return;
	}

	double PosRectImpl::GetWidthInDocPt() const
	{
		return m_widthDocPt;
	}

	double PosRectImpl::GetHeightInDocPt() const
	{
		return m_heightDocPt;
	}

	void PosRectImpl::SetWidthInDocPt(double val)
	{
		m_widthDocPt = val;
	}

	void PosRectImpl::SetHeightInDocPt(double val)
	{
		m_heightDocPt = val;
	}

	double PosRectImpl::minimumPossibleHeightDocPt(double /*gridSize*/, int /*pinGridStep*/) const
	{
		return 0;
	}

	double PosRectImpl::minimumPossibleWidthDocPt(double /*gridSize*/, int /*pinGridStep*/) const
	{
		return 0;
	}

	void PosRectImpl::drawHighlightRect(CDrawParam* drawParam, const QRectF& rect) const
	{
		QPainter* p = drawParam->painter();

		QPen pen{drawParam->blinkPhase() ? SchemaItem::highlightColor1 : SchemaItem::highlightColor2};

		double lineWeight = drawParam->controlBarSize() / 3.0f;
		pen.setWidthF(lineWeight);

		p->setPen(pen);

		// --
		//
		QRectF r{rect};
		r.setTopRight(drawParam->gridToDpi(r.topRight()));
		r.setBottomLeft(drawParam->gridToDpi(r.bottomLeft()));

		p->drawRect(r);

		return;
	}

	void PosRectImpl::drawLabel(CDrawParam* drawParam) const
	{
		if (drawParam == nullptr)
		{
			Q_ASSERT(drawParam);
			return;
		}

		QPainter* p = drawParam->painter();

		// --
		//
		QRectF labelRect{leftDocPt(), topDocPt(), widthDocPt(), heightDocPt()};

		if (std::abs(labelRect.left() - labelRect.right()) < 0.000001)
		{
			labelRect.setRight(labelRect.left() + 0.000001);
		}

		if (std::abs(labelRect.bottom() - labelRect.top()) < 0.000001)
		{
			labelRect.setBottom(labelRect.top() + 0.000001);
		}

		// --
		//
		int alignFlags = Qt::AlignmentFlag::AlignCenter;

		switch (labelPos())
		{
		case E::TextPos::LeftTop:
			labelRect.moveBottomRight(labelRect.topLeft());
			alignFlags = Qt::AlignRight | Qt::AlignBottom;
			break;

		case E::TextPos::Top:
			labelRect.moveBottom(labelRect.top());
			alignFlags = Qt::AlignHCenter | Qt::AlignBottom;
			break;
		case E::TextPos::RightTop:
			labelRect.moveBottomLeft(labelRect.topRight());
			alignFlags = Qt::AlignLeft | Qt::AlignBottom;
			break;
		case E::TextPos::Right:
			labelRect.moveLeft(labelRect.right());
			alignFlags = Qt::AlignLeft | Qt::AlignVCenter;
			break;
		case E::TextPos::RightBottom:
			labelRect.moveTopLeft(labelRect.bottomRight());
			alignFlags = Qt::AlignLeft | Qt::AlignTop;
			break;
		case E::TextPos::Bottom:
			labelRect.moveTop(labelRect.bottom());
			alignFlags = Qt::AlignHCenter | Qt::AlignTop;
			break;
		case E::TextPos::LeftBottom:
			labelRect.moveTopRight(labelRect.bottomLeft());
			alignFlags = Qt::AlignRight | Qt::AlignTop;
			break;
		case E::TextPos::Left:
			labelRect.moveRight(labelRect.left());
			alignFlags = Qt::AlignRight | Qt::AlignVCenter;
			break;
		default:
			Q_ASSERT(false);
		}

		FontParam font("Sans", drawParam->gridSize() * 1.75, false, false);
		p->setPen(Qt::darkGray);

		DrawHelper::drawText(p, font, itemUnit(), label(), labelRect, Qt::TextDontClip | alignFlags);

		return;
	}

	// ��������� �������� ��� ��� �������� ���������
	//
	void PosRectImpl::drawOutline(CDrawParam* drawParam) const
	{
		QPainter* p = drawParam->painter();

		// Drwing resources initialization
		//
		if (outlinePen.get() == nullptr)
		{
			outlinePen = std::make_shared<QPen>(Qt::black);
			outlinePen->setWidth(0);
		}

		// --
		//
		QPainter::RenderHints oldrenderhints = p->renderHints();
		p->setRenderHint(QPainter::Antialiasing, false);

		// --
		//
		QRectF r(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt()); 

		if (std::abs(r.left() - r.right()) < 0.000001)
		{
			r.setRight(r.left() + 0.000001f);
		}

		if (std::abs(r.bottom() - r.top()) < 0.000001)
		{
			r.setBottom(r.top() + 0.000001f);
		}

		p->setPen(*outlinePen);
		p->setBrush(Qt::NoBrush);

		p->drawRect(r);

		// --
		//
		p->setRenderHints(oldrenderhints);
		return;
	}

	void PosRectImpl::drawIssue(CDrawParam* drawParam, OutputMessageLevel issue) const
	{
		if (drawParam == nullptr)
		{
			assert(drawParam);
			return;
		}

		QPainter* p = drawParam->painter();

		// Draw the main part
		//
		QColor color;

		switch (issue)
		{
		case OutputMessageLevel::Error:
			color = SchemaItem::errorColor;
			break;
		case OutputMessageLevel::Warning0:
		case OutputMessageLevel::Warning1:
		case OutputMessageLevel::Warning2:
			color = SchemaItem::warningColor;
			break;
		default:
			Q_ASSERT(false);
		}

		QPen pen(color);
		pen.setWidthF(0);
		p->setPen(pen);

		// --
		//
		double cbs = drawParam->controlBarSize();
		QRectF r(leftDocPt() + cbs / 3.0, topDocPt() + cbs / 3.0, widthDocPt(), heightDocPt());

		if (std::abs(r.left() - r.right()) < 0.000001)
		{
			r.setRight(r.left() + 0.000001f);
		}

		if (std::abs(r.bottom() - r.top()) < 0.000001)
		{
			r.setBottom(r.top() + 0.000001f);
		}

		p->drawLine(r.bottomLeft(), r.bottomRight());
		p->drawLine(r.topRight(), r.bottomRight());

		return;
	}
	
	// ���������� ��������� �������, � ����������� �� ������������� ���������� ������������.
	//
	void PosRectImpl::drawSelection(CDrawParam* drawParam, bool drawSizeBar) const
	{
		QPainter* p = drawParam->painter();

		// Drwing resources initialization
		//
		if (selectionPen.get() == nullptr)
		{
			selectionPen = std::make_shared<QPen>(QColor(0x33, 0x99, 0xFF, 0x80));
		}

		selectionPen->setColor(isLocked() == true ?  SchemaItem::lockedSelectionColor : SchemaItem::selectionColor);

		// --
		//
		QPainter::RenderHints oldrenderhints = p->renderHints();
		p->setRenderHint(QPainter::Antialiasing, false);

		// --
		//
		QRectF r(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt()); 

		if (std::abs(r.left() - r.right()) < 0.000001)
		{
			r.setRight(r.left() + 0.000001f);
		}

		if (std::abs(r.bottom() - r.top()) < 0.000001)
		{
			r.setBottom(r.top() + 0.000001f);
		}

		double cbs = drawParam->controlBarSize();

		double lineWeight = cbs / 2.0f;
		selectionPen->setWidthF(lineWeight);

		p->setPen(*selectionPen);
		p->drawRect(r);

		// ��������������, �� ������� ����� ��������� � �������� �������
		//
		if (drawSizeBar == true && isLocked() == false)
		{
			double fx = r.left();
			double fy = r.top();
			double width = r.width();
			double height = r.height();

			QRectF controlRectangles[] = 
			{
				QRectF(fx - cbs, fy - cbs, cbs, cbs),
				QRectF(fx + width / 2 - cbs / 2, fy - cbs, cbs, cbs),
				QRectF(fx + width, fy - cbs, cbs, cbs),
				QRectF(fx + width, fy + height / 2 - cbs / 2, cbs, cbs),
				QRectF(fx + width, fy + height, cbs, cbs),
				QRectF(fx + width / 2 - cbs / 2, fy + height, cbs, cbs),
				QRectF(fx - cbs, fy + height, cbs, cbs),
				QRectF(fx - cbs, fy + height / 2 - cbs / 2, cbs, cbs)
			};

			for (quint32 i = 0; i < sizeof(controlRectangles) / sizeof(controlRectangles[0]); i++)
			{
				p->fillRect(controlRectangles[i], selectionPen->color());
			}
		}
		
		// --
		//
		p->setRenderHints(oldrenderhints);
		return;
	}

	void PosRectImpl::drawCompareAction(CDrawParam* drawParam, QColor color) const
	{
		QPainter* p = drawParam->painter();

		QPen selectionPen(color);

		// --
		//
//		QPainter::RenderHints oldrenderhints = p->renderHints();
//		p->setRenderHint(QPainter::Antialiasing, false);

		// --
		//
		QRectF r(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());

		if (std::abs(r.left() - r.right()) < 0.000001)
		{
			r.setRight(r.left() + 0.000001f);
		}

		if (std::abs(r.bottom() - r.top()) < 0.000001)
		{
			r.setBottom(r.top() + 0.000001f);
		}

		double cbs = drawParam->controlBarSize();
		double lineWeight = cbs / 2.0f;
		selectionPen.setWidthF(lineWeight);

		p->setPen(selectionPen);
		p->setBrush(Qt::NoBrush);

		p->drawRect(r);

		// --
		//
		//p->setRenderHints(oldrenderhints);
		return;
	}

	void PosRectImpl::drawCommentDim(CDrawParam* drawParam) const
	{
		if (isCommented() == false)
		{
			return;
		}

		QPainter* p = drawParam->painter();

		// Drwing resources initialization
		//

		// --
		//
		QPainter::RenderHints oldrenderhints = p->renderHints();
		p->setRenderHint(QPainter::Antialiasing, false);

		// --
		//
		double margin = drawParam->controlBarSize() / 4.0;

		QRectF r(leftDocPt() - margin, topDocPt() - margin, widthDocPt() + margin * 2.0, heightDocPt() + margin * 2.0);

		if (std::abs(r.left() - r.right()) < 0.000001)
		{
			r.setRight(r.left() + 0.000001f);
		}

		if (std::abs(r.bottom() - r.top()) < 0.000001)
		{
			r.setBottom(r.top() + 0.000001f);
		}

		p->fillRect(r, QBrush(SchemaItem::commentedColor));

		// --
		//
		p->setRenderHints(oldrenderhints);
		return;
	}

	// �����������, ���������� �� ������� ��������� ������������� (������������ ��� ���������),
	// ���������� � ������ �������������� ������ � ������ ��� ��������
	//
	bool PosRectImpl::isIntersectRect(double x, double y, double width, double height) const
	{
		QRectF itemRect(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());
		QRectF detRect(x, y, width, height);

		if (detRect.isEmpty() == true)
		{
			bool result = false;

			result |= CUtils::IsLineIntersectRect(x, y, x, y + height, itemRect);
			result |= CUtils::IsLineIntersectRect(x, y, x + width, y, itemRect);

			return result;
		}

		if (itemRect.isEmpty() == true)
		{
			bool result = false;

			result |= CUtils::IsLineIntersectRect(itemRect.x(), itemRect.y(), itemRect.x() + itemRect.width(), itemRect.y(), detRect);
			result |= CUtils::IsLineIntersectRect(itemRect.x(), itemRect.y(), itemRect.x(), itemRect.y() + itemRect.height(), detRect);

			return result;
		}

		return itemRect.intersects(detRect) | detRect.contains(itemRect.topLeft());	// contains for the empty rect (width or height is 0)
	}

	QRectF PosRectImpl::boundingRectInDocPt(CDrawParam* drawParam) const
	{
		QRectF result(m_leftDocPt, m_topDocPt, m_widthDocPt, m_heightDocPt);

		result.setTopRight(drawParam->gridToDpi(result.topRight()));
		result.setBottomLeft(drawParam->gridToDpi(result.bottomLeft()));

		if (std::abs(result.left() - result.right()) < 0.000001)
		{
			result.setRight(result.left() + 0.000001f);
		}

		if (std::abs(result.bottom() - result.top()) < 0.000001)
		{
			result.setBottom(result.top() + 0.000001f);
		}

		return result;
	}

	double PosRectImpl::leftDocPt() const 
	{
		return m_leftDocPt;
	}
	void PosRectImpl::setLeftDocPt(double value)
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			m_leftDocPt = CUtils::Round(value);
		}
		else
		{
			m_leftDocPt = value;
		}
	}

	double PosRectImpl::topDocPt() const 
	{
		return m_topDocPt;
	}
	void PosRectImpl::setTopDocPt(double value) 
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			m_topDocPt = CUtils::Round(value);
		}
		else
		{
			m_topDocPt = value;
		}
	}

	double PosRectImpl::widthDocPt() const 
	{
		return m_widthDocPt;
	}
	void PosRectImpl::setWidthDocPt(double value) 
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			m_widthDocPt = CUtils::Round(value);
		}
		else
		{
			m_widthDocPt = value;
		}

		if (m_widthDocPt < 0)
		{
			m_widthDocPt = 0;
		}
	}

	double PosRectImpl::heightDocPt() const 
	{
		return m_heightDocPt;
	}
	void PosRectImpl::setHeightDocPt(double value) 
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			m_heightDocPt = CUtils::Round(value);
		}
		else
		{
			m_heightDocPt = value;
		}

		if (m_heightDocPt < 0)
		{
			m_heightDocPt = 0;
		}
	}

	// ���������� ����������� ISchemaItemPropertiesPos
	//
	double PosRectImpl::left() const
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			return CUtils::RoundDisplayPoint(leftDocPt());
		}
		else
		{
			double pt = leftDocPt();
			pt = CUtils::ConvertPoint(pt, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			return CUtils::RoundPoint(pt, Settings::regionalUnit());
		}
	}
	void PosRectImpl::setLeft(const double& value)
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			setLeftDocPt(CUtils::RoundDisplayPoint(value));
		}
		else
		{
			double pt = CUtils::ConvertPoint(value, Settings::regionalUnit(), SchemaUnit::Inch, 0);
			setLeftDocPt(pt);
		}
	}

	double PosRectImpl::top() const 
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			return CUtils::RoundDisplayPoint(topDocPt());
		}
		else
		{
			double pt = topDocPt();
			pt = CUtils::ConvertPoint(pt, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			return CUtils::RoundPoint(pt, Settings::regionalUnit());
		}			
	}
	void PosRectImpl::setTop(const double& value)
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			double pt = CUtils::RoundDisplayPoint(value);
			setTopDocPt(pt);
		}
		else
		{
			double pt = CUtils::ConvertPoint(value, Settings::regionalUnit(), SchemaUnit::Inch, 0);
			setTopDocPt(pt);
		}
	}

	double PosRectImpl::width() const 
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			return CUtils::RoundDisplayPoint(widthDocPt());
		}
		else
		{
			double pt = widthDocPt();
			pt = CUtils::ConvertPoint(pt, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			return CUtils::RoundPoint(pt, Settings::regionalUnit());
		}
	}
	void PosRectImpl::setWidth(const double& value)
	{
		const double normalizedValue = value < 0 ? 0 : value;

		if (itemUnit() == SchemaUnit::Display)
		{
			setWidthDocPt(CUtils::RoundDisplayPoint(normalizedValue));
		}
		else
		{
			double pt = CUtils::ConvertPoint(normalizedValue, Settings::regionalUnit(), SchemaUnit::Inch, 0);
			setWidthDocPt(pt);
		}
	}

	double PosRectImpl::height() const 
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			return CUtils::RoundDisplayPoint(heightDocPt());
		}
		else
		{
			double pt = heightDocPt();
			pt = CUtils::ConvertPoint(pt, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			return CUtils::RoundPoint(pt, Settings::regionalUnit());
		}			
	}
	void PosRectImpl::setHeight(const double& value)
	{
		double normalizedValue = value < 0 ? 0 : value;

		if (itemUnit() == SchemaUnit::Display)
		{
			setHeightDocPt(CUtils::RoundDisplayPoint(normalizedValue));
		}
		else
		{
			double pt = CUtils::ConvertPoint(normalizedValue, Settings::regionalUnit(), SchemaUnit::Inch, 0);
			setHeightDocPt(pt);
		}
	}

	std::vector<SchemaPoint> PosRectImpl::getPointList() const
	{
		std::vector<SchemaPoint> v = {{m_leftDocPt, m_topDocPt},
									  {m_leftDocPt + m_widthDocPt, m_topDocPt + m_heightDocPt}};
		return v;
	}

	void PosRectImpl::setPointList(const std::vector<SchemaPoint>& points)
	{
		if (points.size() != 2)
		{
			assert(points.size() == 2);
			return;
		}

		m_leftDocPt = points.front().X;
		m_topDocPt = points.front().Y;

		m_widthDocPt = points.back().X - m_leftDocPt;
		m_heightDocPt = points.back().Y - m_topDocPt;

		return;
	}
}

