#include "Stable.h"
#include "PosRectImpl.h"
#include <QRect>

namespace VFrame30
{
	PosRectImpl::PosRectImpl(void)
	{
		Init();
	}

	PosRectImpl::~PosRectImpl(void)
	{
	}

	void PosRectImpl::Init()
	{
		m_leftDocPt = 0;
		m_topDocPt = 0;
		m_widthDocPt = 0;
		m_heightDocPt = 0;
	}

	bool PosRectImpl::SaveData(Proto::Envelope* message) const
	{
		bool result = VideoItem::SaveData(message);
		if (result == false || message->has_videoitem() == false)
		{
			assert(result);
			assert(message->has_videoitem());
			return false;
		}

		// --
		//
		Proto::PosRectImpl* posRectImplMessage = message->mutable_videoitem()->mutable_posrectimpl();

		posRectImplMessage->set_leftdocpt(m_leftDocPt);
		posRectImplMessage->set_topdocpt(m_topDocPt);
		posRectImplMessage->set_widthdocpt(m_widthDocPt);
		posRectImplMessage->set_heightdocpt(m_heightDocPt);

		return true;
	}

	bool PosRectImpl::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return false;
		}
		
		// --
		//
		bool result = VideoItem::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		if (message.videoitem().has_posrectimpl() == false)
		{
			assert(message.videoitem().has_posrectimpl());
			return false;
		}

		const Proto::PosRectImpl& posRectImplMessage = message.videoitem().posrectimpl();

		m_leftDocPt = posRectImplMessage.leftdocpt();
		m_topDocPt = posRectImplMessage.topdocpt();
		m_widthDocPt = posRectImplMessage.widthdocpt();
		m_heightDocPt = posRectImplMessage.heightdocpt();

		return true;
	}

	// Action Functions
	//
	void PosRectImpl::MoveItem(double horzOffsetDocPt, double vertOffsetDocPt)
	{ 
		setLeftDocPt(leftDocPt() + horzOffsetDocPt);
		setTopDocPt(topDocPt() + vertOffsetDocPt);
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

	// Рисование элемента при его создании изменении
	//
	void PosRectImpl::DrawOutline(CDrawParam* drawParam) const
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
	
	// Нарисовать выделение объекта, в зависимости от используемого интрефейса расположения.
	//
	void PosRectImpl::DrawSelection(CDrawParam* drawParam, bool drawSizeBar) const
	{
		QPainter* p = drawParam->painter();

		// Drwing resources initialization
		//
		if (selectionPen.get() == nullptr)
		{
			selectionPen = std::make_shared<QPen>(QColor(0x33, 0x99, 0xFF, 0x80));
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

		double cbs = drawParam->controlBarSize();

		double lineWeight = cbs / 2.0f;
		selectionPen->setWidthF(lineWeight);

		p->setPen(*selectionPen);
		p->drawRect(r);

		// Прямоугольники, за которые можно хвататься и изменять элемент
		//
		if (drawSizeBar == true)
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

	// Определение, пересекает ли элемент указанный прямоугольник (использовать для выделения),
	// координаты и размер прямоугольника заданы в дюймах или пикселях
	//
	bool PosRectImpl::IsIntersectRect(double x, double y, double width, double height) const
	{
		QRectF itemRect(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());
		QRectF detRect(x, y, width, height);
		return itemRect.intersects(detRect);
	}

	QRectF PosRectImpl::boundingRectInDocPt() const
	{
		QRectF result(m_leftDocPt, m_topDocPt, m_widthDocPt, m_heightDocPt);
		return result;
	}

	double PosRectImpl::leftDocPt() const 
	{
		return m_leftDocPt;
	}
	void PosRectImpl::setLeftDocPt(double value) 
	{
		if (itemUnit() == SchemeUnit::Display)
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
		if (itemUnit() == SchemeUnit::Display)
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
		if (itemUnit() == SchemeUnit::Display)
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
		if (itemUnit() == SchemeUnit::Display)
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

	// Реализация интерефейса IVideoItemPropertiesPos
	//
	double PosRectImpl::left() const
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			return CUtils::RoundDisplayPoint(leftDocPt());
		}
		else
		{
			double pt = leftDocPt();
			pt = CUtils::ConvertPoint(pt, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Horz);
			return CUtils::RoundPoint(pt, CSettings::regionalUnit());
		}
	}
	void PosRectImpl::setLeft(double value) 
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			setLeftDocPt(CUtils::RoundDisplayPoint(value));
		}
		else
		{
			double pt = CUtils::ConvertPoint(value, CSettings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Horz);
			setLeftDocPt(pt);
		}
	}

	double PosRectImpl::top() const 
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			return CUtils::RoundDisplayPoint(topDocPt());
		}
		else
		{
			double pt = topDocPt();
			pt = CUtils::ConvertPoint(pt, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Vert);
			return CUtils::RoundPoint(pt, CSettings::regionalUnit());
		}			
	}
	void PosRectImpl::setTop(double value) 
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			double pt = CUtils::RoundDisplayPoint(value);
			setTopDocPt(pt);
		}
		else
		{
			double pt = CUtils::ConvertPoint(value, CSettings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Vert);
			setTopDocPt(pt);
		}
	}

	double PosRectImpl::width() const 
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			return CUtils::RoundDisplayPoint(widthDocPt());
		}
		else
		{
			double pt = widthDocPt();
			pt = CUtils::ConvertPoint(pt, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Horz);
			return CUtils::RoundPoint(pt, CSettings::regionalUnit());
		}
	}
	void PosRectImpl::setWidth(double value) 
	{
		if (value < 0)
		{
			value = 0;
		}

		if (itemUnit() == SchemeUnit::Display)
		{
			setWidthDocPt(CUtils::RoundDisplayPoint(value));
		}
		else
		{
			double pt = CUtils::ConvertPoint(value, CSettings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Horz);
			setWidthDocPt(pt);
		}
	}

	double PosRectImpl::height() const 
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			return CUtils::RoundDisplayPoint(heightDocPt());
		}
		else
		{
			double pt = heightDocPt();
			pt = CUtils::ConvertPoint(pt, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Vert);
			return CUtils::RoundPoint(pt, CSettings::regionalUnit());
		}			
	}
	void PosRectImpl::setHeight(double value) 
	{
		if (value < 0)	
		{
			value = 0;
		}

		if (itemUnit() == SchemeUnit::Display)
		{
			setHeightDocPt(CUtils::RoundDisplayPoint(value));
		}
		else
		{
			double pt = CUtils::ConvertPoint(value, CSettings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Vert);
			setHeightDocPt(pt);
		}
	}

	std::vector<VideoItemPoint> PosRectImpl::getPointList() const
	{
		std::vector<VideoItemPoint> v(2);

		v[0] = VideoItemPoint(m_leftDocPt, m_topDocPt);
		v[1] = VideoItemPoint(m_leftDocPt + m_widthDocPt, m_topDocPt + m_heightDocPt);

		return v;
	}

	void PosRectImpl::setPointList(const std::vector<VideoItemPoint>& points)
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

