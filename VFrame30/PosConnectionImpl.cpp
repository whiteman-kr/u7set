#include "Stable.h"
#include "PosConnectionImpl.h"

namespace VFrame30
{
	CPosConnectionImpl::CPosConnectionImpl(void)
	{
		Init();
	}

	CPosConnectionImpl::~CPosConnectionImpl(void)
	{
	}

	void CPosConnectionImpl::Init(void)
	{
	}

	// Serialization
	//
	bool CPosConnectionImpl::SaveData(Proto::Envelope* message) const
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
		Proto::PosConnectionImpl* posConnectionImplMessage = message->mutable_videoitem()->mutable_posconnectionimpl();

		// Сохранить точки
		//
		for (auto pt = points.cbegin(); pt != points.cend(); ++pt)
		{
			Proto::VideoItemPoint* pPointMessage = posConnectionImplMessage->add_points();

			pPointMessage->set_x(pt->X);
			pPointMessage->set_y(pt->Y);
		}

		return true;
	}

	bool CPosConnectionImpl::LoadData(const Proto::Envelope& message)
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
		
		//--
		//
		if (message.videoitem().has_posconnectionimpl() == false)
		{
			assert(message.videoitem().has_posconnectionimpl());
			return false;
		}

		const Proto::PosConnectionImpl& posConnectionImplMessage = message.videoitem().posconnectionimpl();

		points.clear();
		for (int i = 0; i < posConnectionImplMessage.points().size(); i++)
		{
			points.push_back(VideoItemPoint(posConnectionImplMessage.points(i)));
		}

		return true;
	}

	// Action Functions
	//
	void CPosConnectionImpl::MoveItem(double horzOffsetDocPt, double vertOffsetDocPt)
	{
		for(auto pt = points.begin(); pt != points.end(); ++pt)
		{
			pt->X += horzOffsetDocPt;
			pt->Y += vertOffsetDocPt;
		}
	}

	double CPosConnectionImpl::GetWidthInDocPt() const
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

	double CPosConnectionImpl::GetHeightInDocPt() const
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

	void CPosConnectionImpl::SetWidthInDocPt(double /*val*/)
	{
	}

	void CPosConnectionImpl::SetHeightInDocPt(double /*val*/)
	{
	}

	// Draw Functions
	//

	// Рисование элемента при его создании изменении
	//
	void CPosConnectionImpl::DrawOutline(CDrawParam* drawParam) const
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

		QPen pen(Qt::black);
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

		QPen extPen(QColor(0x061040C0));	// D2D1::ColorF(0x1040C0, 0.3f)
		extPen.setWidth(0);
		p->setPen(extPen);

		p->drawPolyline(extPolyline);

		return;
	}

	// Нарисовать выделение объекта, в зависимости от используемого интрефейса расположения.
	//
	void CPosConnectionImpl::DrawSelection(CDrawParam* drawParam, bool drawSizeBar) const
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

		QPen pen(QColor(0x33, 0x99, 0xFF, 0x80));
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
	bool CPosConnectionImpl::IsIntersectRect(double x, double y, double width, double height) const
	{
		// Проверить, пересекает ли хоть одна прямая intersectRectangleIn
		//
		QRectF intersectRectangleIn(x, y, width, height);
		VideoItemPoint prevPoint;

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

	QRectF CPosConnectionImpl::boundingRectInDocPt() const
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

		QRectF result(l, t, abs(r - l), abs(b - t));
		return result;
	}
	
	// Реализация интерефейса IVideoItemPosLine
	//
	const std::list<VideoItemPoint>& CPosConnectionImpl::GetPointList() const
	{
		return points;
	}

	void CPosConnectionImpl::SetPointList(const std::list<VideoItemPoint>& newpoints)
	{
		points.assign(newpoints.begin(), newpoints.end());
	}

	void CPosConnectionImpl::AddPoint(double x, double y)
	{
		points.push_back(VideoItemPoint(x, y));
	}

	void CPosConnectionImpl::RemoveSamePoints()
	{
		if (points.size() <= 2)
		{
			return;
		}

		points.unique();
	}

	void CPosConnectionImpl::DeleteAllPoints()
	{
		points.clear();
		extPoints.clear();
	}

	void CPosConnectionImpl::DeleteLastPoint()
	{
		if (points.empty() == false)
		{
			points.pop_back();
		}
	}

	const std::list<VideoItemPoint>& CPosConnectionImpl::GetExtensionPoints() const
	{
		return extPoints;
	}
	
	void CPosConnectionImpl::SetExtensionPoints(const std::list<VideoItemPoint>& extPoints)
	{
		this->extPoints = extPoints;
	}

	void CPosConnectionImpl::AddExtensionPoint(double x, double y)
	{
		extPoints.push_back(VideoItemPoint(x, y));
	}

	void CPosConnectionImpl::DeleteAllExtensionPoints()
	{
		extPoints.clear();
	}

	void CPosConnectionImpl::DeleteLastExtensionPoint()
	{
		if (extPoints.empty() == false)
		{
			extPoints.pop_back();
		}
	}
		
	// Реализация интерефейса IVideoItemPropertiesPos
	//
	double CPosConnectionImpl::left() const
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

		if (itemUnit() == SchemeUnit::Display)
		{
			val = CUtils::RoundDisplayPoint(val);
		}
		else
		{
			val = CUtils::ConvertPoint(val, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Horz);
			val = CUtils::RoundPoint(val, CSettings::regionalUnit());
		}

		return val;
	}
	
	void CPosConnectionImpl::setLeft(double)
	{
		// Нет реализации - by design, хотя можно что то и придумать
		//
	}

	double CPosConnectionImpl::top() const
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

		if (itemUnit() == SchemeUnit::Display)
		{
			val = CUtils::RoundDisplayPoint(val);
		}
		else
		{
			val = CUtils::ConvertPoint(val, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Vert);
			val = CUtils::RoundPoint(val, CSettings::regionalUnit());
		}

		return val;
	}
	
	void CPosConnectionImpl::setTop(double)
	{
		// Нет реализации - by design, хотя можно что то и придумать
		//
	}

	double CPosConnectionImpl::width() const
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

		if (itemUnit() == SchemeUnit::Display)
		{
			val = CUtils::RoundDisplayPoint(val);
		}
		else
		{
			val = CUtils::ConvertPoint(val, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Horz);
			val = CUtils::RoundPoint(val, CSettings::regionalUnit());
		}

		return val;
	}
	
	void CPosConnectionImpl::setWidth(double)
	{
		// Нет реализации - by design, хотя можно что то и придумать
		//
	}

	double CPosConnectionImpl::height() const
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

		if (itemUnit() == SchemeUnit::Display)
		{
			val = CUtils::RoundDisplayPoint(val);
		}
		else
		{
			val = CUtils::ConvertPoint(val, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Vert);
			val = CUtils::RoundPoint(val, CSettings::regionalUnit());
		}

		return val;
	}

	void CPosConnectionImpl::setHeight(double)
	{
		// Нет реализации - by design, хотя можно что то и придумать
		//
	}

	std::vector<VideoItemPoint> CPosConnectionImpl::getPointList() const
	{
		std::vector<VideoItemPoint> v(points.begin(), points.end());
		return v;
	}

	void CPosConnectionImpl::setPointList(const std::vector<VideoItemPoint>& points)
	{
		this->points.assign(points.begin(), points.end());
		extPoints.clear();
		return;
	}
}

