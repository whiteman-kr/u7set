#include "Stable.h"
#include "PosConnectionImpl.h"

namespace VFrame30
{
	PosConnectionImpl::PosConnectionImpl(void)
	{
		Init();
	}

	PosConnectionImpl::~PosConnectionImpl(void)
	{
	}

	void PosConnectionImpl::Init(void)
	{
	}

	// Serialization
	//
	bool PosConnectionImpl::SaveData(Proto::Envelope* message) const
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
		Proto::PosConnectionImpl* posConnectionImplMessage = message->mutable_schemeitem()->mutable_posconnectionimpl();

		// Сохранить точки
		//
		for (auto pt = points.cbegin(); pt != points.cend(); ++pt)
		{
			Proto::SchemePoint* pPointMessage = posConnectionImplMessage->add_points();

			pPointMessage->set_x(pt->X);
			pPointMessage->set_y(pt->Y);
		}

		return true;
	}

	bool PosConnectionImpl::LoadData(const Proto::Envelope& message)
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
		
		//--
		//
		if (message.schemeitem().has_posconnectionimpl() == false)
		{
			assert(message.schemeitem().has_posconnectionimpl());
			return false;
		}

		const Proto::PosConnectionImpl& posConnectionImplMessage = message.schemeitem().posconnectionimpl();

		points.clear();
		for (int i = 0; i < posConnectionImplMessage.points().size(); i++)
		{
			points.push_back(SchemePoint(posConnectionImplMessage.points(i)));
		}

		return true;
	}

	// Action Functions
	//
	void PosConnectionImpl::MoveItem(double horzOffsetDocPt, double vertOffsetDocPt)
	{
		for(auto pt = points.begin(); pt != points.end(); ++pt)
		{
			pt->X += horzOffsetDocPt;
			pt->Y += vertOffsetDocPt;
		}
	}

	void PosConnectionImpl::snapToGrid(double gridSize)
	{
		for(auto pt = points.begin(); pt != points.end(); ++pt)
		{
			QPointF snapped = CUtils::snapToGrid(QPointF(pt->X, pt->Y), gridSize);

			pt->X = snapped.x();
			pt->Y = snapped.y();
		}

		return;
	}

	double PosConnectionImpl::GetWidthInDocPt() const
	{
		if (points.size() == 0)
		{
			return 0.0;
		}
		
		double minval = points.front().X;
		double maxval = points.front().X;

		for (auto pt = points.cbegin(); pt != points.cend(); ++pt)
		{
			minval = std::min(minval, pt->X);
			maxval = std::max(maxval, pt->X);
		}

		return maxval - minval;
	}

	double PosConnectionImpl::GetHeightInDocPt() const
	{
		if (points.size() == 0)
		{
			return 0.0;
		}

		double minval = points.front().Y;
		double maxval = points.front().Y;

		for (auto pt = points.cbegin(); pt != points.cend(); ++pt)
		{
			minval = std::min(minval, pt->Y);
			maxval = std::max(maxval, pt->Y);
		}

		return maxval - minval;
	}

	void PosConnectionImpl::SetWidthInDocPt(double /*val*/)
	{
	}

	void PosConnectionImpl::SetHeightInDocPt(double /*val*/)
	{
	}

	// Draw Functions
	//

	// Рисование элемента при его создании изменении
	//
	void PosConnectionImpl::DrawOutline(CDrawParam* drawParam) const
	{
		if (drawParam == nullptr)
		{
			assert(drawParam);
			return;
		}

		if (points.empty() == true || points.size() + extPoints.size() < 2)
		{
			return;
		}

		QPainter* p = drawParam->painter();
			
		// Draw the main part
		//
		QPolygonF polyline(static_cast<int>(points.size()));
		int index = 0;

		for (auto pt = points.cbegin(); pt != points.cend(); ++pt)
		{
			polyline[index++] = QPointF(pt->X, pt->Y);
		}

		QPen pen(Qt::darkRed);
		pen.setWidthF(0);
		p->setPen(pen);

		p->drawPolyline(polyline);
						
		// Draw extPoints
		//
		QPolygonF extPolyline;
		
		extPolyline.push_back(QPointF(points.back().X, points.back().Y));

		for (auto pt = extPoints.cbegin(); pt != extPoints.cend(); ++pt)
		{
			extPolyline.push_back(QPointF(pt->X, pt->Y));
		}

		//QPen extPen(QColor(0x061040C0));	// D2D1::ColorF(0x1040C0, 0.3f)
		QPen extPen(Qt::red);
		extPen.setWidth(0);
		p->setPen(extPen);

		p->drawPolyline(extPolyline);

		return;
	}

	void PosConnectionImpl::DrawIssue(CDrawParam* drawParam, OutputMessageLevel issue) const
	{
		if (drawParam == nullptr)
		{
			assert(drawParam);
			return;
		}

		QPainter* p = drawParam->painter();

		double cbs = drawParam->controlBarSize();

		// Draw the main part
		//
		QPolygonF polyline(static_cast<int>(points.size()));
		int index = 0;

		for (auto pt = points.cbegin(); pt != points.cend(); ++pt)
		{
			polyline[index++] = QPointF(pt->X + cbs / 3.0, pt->Y + cbs / 3.0);
		}

		QColor color;

		switch (issue)
		{
		case OutputMessageLevel::Error:
			color = SchemeItem::errorColor;
			break;
		case OutputMessageLevel::Warning:
			color = SchemeItem::warningColor;
			break;
		default:
			assert(false);
		}

		QPen pen(color);
		pen.setWidthF(0);
		p->setPen(pen);

		p->drawPolyline(polyline);

		return;
	}


	// Нарисовать выделение объекта, в зависимости от используемого интрефейса расположения.
	//
	void PosConnectionImpl::DrawSelection(CDrawParam* drawParam, bool drawSizeBar) const
	{
		if (drawParam == nullptr)
		{
			assert(drawParam);
			return;
		}

		QPainter* p = drawParam->painter();

		double cbs = drawParam->controlBarSize();

		double lineWeight = cbs / 2.0f;

		// Draw the main part
		//
		QPolygonF polyline(static_cast<int>(points.size()));
		int index = 0;

		for (auto pt = points.cbegin(); pt != points.cend(); ++pt)
		{
			polyline[index++] = QPointF(pt->X, pt->Y);
		}

		QPen pen(SchemeItem::selectionColor);
		pen.setWidthF(lineWeight);
		p->setPen(pen);

		p->drawPolyline(polyline);
			
		// Draw control bars
		//
		if (drawSizeBar == true)
		{
			QBrush solidBrush(pen.color());

			QRectF controlRectangles;

			for (auto pt = points.cbegin(); pt != points.cend(); ++pt)
			{
				controlRectangles.setLeft(pt->X - cbs / 2);
				controlRectangles.setTop(pt->Y - cbs / 2);
				controlRectangles.setWidth(cbs);
				controlRectangles.setHeight(cbs);

				p->fillRect(controlRectangles, solidBrush);
			}
		}

		return;
	}

	// Determine and Calculation Functions
	//

	// Определение, пересекает ли элемент указанный прямоугольник (использовать для выделения),
	// координаты и размер прямоугольника заданы в дюймах или пикселях
	// 
	bool PosConnectionImpl::IsIntersectRect(double x, double y, double width, double height) const
	{
		// Проверить, пересекает ли хоть одна прямая intersectRectangleIn
		//
		QRectF intersectRectangleIn(x, y, width, height);
		SchemePoint prevPoint;

		for(auto curPoint = points.cbegin(); curPoint != points.cend(); ++curPoint)
		{
			if (curPoint == points.cbegin())
			{
				prevPoint = curPoint.operator*();
				continue;
			}

			if (CUtils::IsLineIntersectRect(prevPoint.X, prevPoint.Y, curPoint->X, curPoint->Y, intersectRectangleIn) == true)
			{
				return true;
			}

			prevPoint = curPoint.operator*();
		}

		return false;
	}

	QRectF PosConnectionImpl::boundingRectInDocPt() const
	{
		if (points.size() == 0)
		{
			return QRectF();
		}

		double l = points.front().X;
		double r = l;
		double t = points.front().Y;
		double b = t;
					
		for (auto pt = points.cbegin(); pt != points.cend(); ++pt)
		{
			l = std::min(l, pt->X);
			r = std::max(r, pt->X);

			t = std::min(t, pt->Y);
			b = std::max(b, pt->Y);
		}

		QRectF result(l, t, std::abs(r - l), std::abs(b - t));
		return result;
	}
	
	// IPosLine Implementation
	//
	const std::list<SchemePoint>& PosConnectionImpl::GetPointList() const
	{
		return points;
	}

	void PosConnectionImpl::SetPointList(const std::list<SchemePoint>& newpoints)
	{
		points.assign(newpoints.begin(), newpoints.end());
	}

	void PosConnectionImpl::AddPoint(double x, double y)
	{
		points.push_back(SchemePoint(x, y));
	}

	void PosConnectionImpl::RemoveSamePoints()
	{
		if (points.size() <= 2)
		{
			return;
		}

		points.unique();
	}

	void PosConnectionImpl::DeleteAllPoints()
	{
		points.clear();
		extPoints.clear();
	}

	void PosConnectionImpl::DeleteLastPoint()
	{
		if (points.empty() == false)
		{
			points.pop_back();
		}
	}

	const std::list<SchemePoint>& PosConnectionImpl::GetExtensionPoints() const
	{
		return extPoints;
	}
	
	void PosConnectionImpl::SetExtensionPoints(const std::list<SchemePoint>& extPoints)
	{
		this->extPoints = extPoints;
	}

	void PosConnectionImpl::AddExtensionPoint(double x, double y)
	{
		extPoints.push_back(SchemePoint(x, y));
	}

	void PosConnectionImpl::DeleteAllExtensionPoints()
	{
		extPoints.clear();
	}

	void PosConnectionImpl::DeleteLastExtensionPoint()
	{
		if (extPoints.empty() == false)
		{
			extPoints.pop_back();
		}
	}
		
	// Реализация интерефейса ISchemeItemPropertiesPos
	//
	double PosConnectionImpl::left() const
	{
		if (points.size() == 0)
		{
			return 0.0;
		}
		
		double val = points.front().X;

		for (auto pt = points.cbegin(); pt != points.cend(); ++pt)
		{
			val = std::min(val, pt->X);
		}

		if (itemUnit() == SchemaUnit::Display)
		{
			val = CUtils::RoundDisplayPoint(val);
		}
		else
		{
			val = CUtils::ConvertPoint(val, SchemaUnit::Inch, Settings::regionalUnit(), ConvertDirection::Horz);
			val = CUtils::RoundPoint(val, Settings::regionalUnit());
		}

		return val;
	}
	
	void PosConnectionImpl::setLeft(double)
	{
		// Нет реализации - by design, хотя можно что то и придумать
		//
	}

	double PosConnectionImpl::top() const
	{
		if (points.size() == 0)
		{
			return 0.0;
		}
		
		double val = points.front().Y;

		for (auto pt = points.cbegin(); pt != points.cend(); ++pt)
		{
			val = std::min(val, pt->Y);
		}

		if (itemUnit() == SchemaUnit::Display)
		{
			val = CUtils::RoundDisplayPoint(val);
		}
		else
		{
			val = CUtils::ConvertPoint(val, SchemaUnit::Inch, Settings::regionalUnit(), ConvertDirection::Vert);
			val = CUtils::RoundPoint(val, Settings::regionalUnit());
		}

		return val;
	}
	
	void PosConnectionImpl::setTop(double)
	{
		// Нет реализации - by design, хотя можно что то и придумать
		//
	}

	double PosConnectionImpl::width() const
	{
		if (points.size() == 0)
		{
			return 0.0;
		}
		
		double minval = points.front().X;
		double maxval = points.front().X;

		for (auto pt = points.cbegin(); pt != points.cend(); ++pt)
		{
			minval = std::min(minval, pt->X);
			maxval = std::max(maxval, pt->X);
		}

		double val = maxval - minval;

		if (itemUnit() == SchemaUnit::Display)
		{
			val = CUtils::RoundDisplayPoint(val);
		}
		else
		{
			val = CUtils::ConvertPoint(val, SchemaUnit::Inch, Settings::regionalUnit(), ConvertDirection::Horz);
			val = CUtils::RoundPoint(val, Settings::regionalUnit());
		}

		return val;
	}
	
	void PosConnectionImpl::setWidth(double)
	{
		// Нет реализации - by design, хотя можно что то и придумать
		//
	}

	double PosConnectionImpl::height() const
	{
		if (points.size() == 0)
		{
			return 0.0;
		}

		double minval = points.front().Y;
		double maxval = points.front().Y;

		for (auto pt = points.cbegin(); pt != points.cend(); ++pt)
		{
			minval = std::min(minval, pt->Y);
			maxval = std::max(maxval, pt->Y);
		}

		double val = maxval - minval;

		if (itemUnit() == SchemaUnit::Display)
		{
			val = CUtils::RoundDisplayPoint(val);
		}
		else
		{
			val = CUtils::ConvertPoint(val, SchemaUnit::Inch, Settings::regionalUnit(), ConvertDirection::Vert);
			val = CUtils::RoundPoint(val, Settings::regionalUnit());
		}

		return val;
	}

	void PosConnectionImpl::setHeight(double)
	{
		// Нет реализации - by design, хотя можно что то и придумать
		//
	}

	std::vector<SchemePoint> PosConnectionImpl::getPointList() const
	{
		std::vector<SchemePoint> v(points.begin(), points.end());
		return v;
	}

	void PosConnectionImpl::setPointList(const std::vector<SchemePoint>& points)
	{
		this->points.assign(points.begin(), points.end());
		extPoints.clear();
		return;
	}
}

