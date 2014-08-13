#include "Stable.h"
#include "PosRectImpl.h"
#include <QRect>

namespace VFrame30
{
	CPosRectImpl::CPosRectImpl(void)
	{
		Init();
	}

	CPosRectImpl::~CPosRectImpl(void)
	{
	}

	void CPosRectImpl::Init()
	{
		m_leftDocPt = 0;
		m_topDocPt = 0;
		m_widthDocPt = 0;
		m_heightDocPt = 0;
	}

	bool CPosRectImpl::SaveData(VFrame30::Proto::Envelope* message) const
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
		Proto::PosRectImpl* posRectImplMessage = message->mutable_videoitem()->mutable_posrectimpl();

		posRectImplMessage->set_leftdocpt(m_leftDocPt);
		posRectImplMessage->set_topdocpt(m_topDocPt);
		posRectImplMessage->set_widthdocpt(m_widthDocPt);
		posRectImplMessage->set_heightdocpt(m_heightDocPt);

		return true;
	}

	bool CPosRectImpl::LoadData(const VFrame30::Proto::Envelope& message)
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
	void CPosRectImpl::MoveItem(double horzOffsetDocPt, double vertOffsetDocPt)
	{ 
		setLeftDocPt(leftDocPt() + horzOffsetDocPt);
		setTopDocPt(topDocPt() + vertOffsetDocPt);
	}

	double CPosRectImpl::GetWidthInDocPt() const
	{
		return m_widthDocPt;
	}

	double CPosRectImpl::GetHeightInDocPt() const
	{
		return m_heightDocPt;
	}

	void CPosRectImpl::SetWidthInDocPt(double val)
	{
		m_widthDocPt = val;
	}

	void CPosRectImpl::SetHeightInDocPt(double val)
	{
		m_heightDocPt = val;
	}

	// ��������� �������� ��� ��� �������� ���������
	//
	void CPosRectImpl::DrawOutline(CDrawParam* drawParam) const
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
	
	// ���������� ��������� �������, � ����������� �� ������������� ���������� ������������.
	//
	void CPosRectImpl::DrawSelection(CDrawParam* drawParam, bool drawSizeBar) const
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

		// ��������������, �� ������� ����� ��������� � �������� �������
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

	// �����������, ���������� �� ������� ��������� ������������� (������������ ��� ���������),
	// ���������� � ������ �������������� ������ � ������ ��� ��������
	//
	bool CPosRectImpl::IsIntersectRect(double x, double y, double width, double height) const
	{
		QRectF itemRect(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());
		QRectF detRect(x, y, width, height);
		return itemRect.intersects(detRect);
	}

	QRectF CPosRectImpl::boundingRectInDocPt() const
	{
		QRectF result(m_leftDocPt, m_topDocPt, m_widthDocPt, m_heightDocPt);
		return result;
	}

	double CPosRectImpl::leftDocPt() const 
	{
		return m_leftDocPt;
	}
	void CPosRectImpl::setLeftDocPt(double value) 
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			m_leftDocPt = CVFrameUtils::Round(value);
		}
		else
		{
			m_leftDocPt = value;
		}
	}

	double CPosRectImpl::topDocPt() const 
	{
		return m_topDocPt;
	}
	void CPosRectImpl::setTopDocPt(double value) 
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			m_topDocPt = CVFrameUtils::Round(value);
		}
		else
		{
			m_topDocPt = value;
		}
	}

	double CPosRectImpl::widthDocPt() const 
	{
		return m_widthDocPt;
	}
	void CPosRectImpl::setWidthDocPt(double value) 
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			m_widthDocPt = CVFrameUtils::Round(value);
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

	double CPosRectImpl::heightDocPt() const 
	{
		return m_heightDocPt;
	}
	void CPosRectImpl::setHeightDocPt(double value) 
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			m_heightDocPt = CVFrameUtils::Round(value);
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

	// ���������� ����������� IVideoItemPropertiesPos
	//
	double CPosRectImpl::left() const
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			return CVFrameUtils::RoundDisplayPoint(leftDocPt());
		}
		else
		{
			double pt = leftDocPt();
			pt = CVFrameUtils::ConvertPoint(pt, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Horz);
			return CVFrameUtils::RoundPoint(pt, CSettings::regionalUnit());
		}
	}
	void CPosRectImpl::setLeft(double value) 
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			setLeftDocPt(CVFrameUtils::RoundDisplayPoint(value));
		}
		else
		{
			double pt = CVFrameUtils::ConvertPoint(value, CSettings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Horz);
			setLeftDocPt(pt);
		}
	}

	double CPosRectImpl::top() const 
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			return CVFrameUtils::RoundDisplayPoint(topDocPt());
		}
		else
		{
			double pt = topDocPt();
			pt = CVFrameUtils::ConvertPoint(pt, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Vert);
			return CVFrameUtils::RoundPoint(pt, CSettings::regionalUnit());
		}			
	}
	void CPosRectImpl::setTop(double value) 
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			double pt = CVFrameUtils::RoundDisplayPoint(value);
			setTopDocPt(pt);
		}
		else
		{
			double pt = CVFrameUtils::ConvertPoint(value, CSettings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Vert);
			setTopDocPt(pt);
		}
	}

	double CPosRectImpl::width() const 
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			return CVFrameUtils::RoundDisplayPoint(widthDocPt());
		}
		else
		{
			double pt = widthDocPt();
			pt = CVFrameUtils::ConvertPoint(pt, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Horz);
			return CVFrameUtils::RoundPoint(pt, CSettings::regionalUnit());
		}
	}
	void CPosRectImpl::setWidth(double value) 
	{
		if (value < 0)
		{
			value = 0;
		}

		if (itemUnit() == SchemeUnit::Display)
		{
			setWidthDocPt(CVFrameUtils::RoundDisplayPoint(value));
		}
		else
		{
			double pt = CVFrameUtils::ConvertPoint(value, CSettings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Horz);
			setWidthDocPt(pt);
		}
	}

	double CPosRectImpl::height() const 
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			return CVFrameUtils::RoundDisplayPoint(heightDocPt());
		}
		else
		{
			double pt = heightDocPt();
			pt = CVFrameUtils::ConvertPoint(pt, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Vert);
			return CVFrameUtils::RoundPoint(pt, CSettings::regionalUnit());
		}			
	}
	void CPosRectImpl::setHeight(double value) 
	{
		if (value < 0)	
		{
			value = 0;
		}

		if (itemUnit() == SchemeUnit::Display)
		{
			setHeightDocPt(CVFrameUtils::RoundDisplayPoint(value));
		}
		else
		{
			double pt = CVFrameUtils::ConvertPoint(value, CSettings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Vert);
			setHeightDocPt(pt);
		}
	}

	std::vector<VideoItemPoint> CPosRectImpl::getPointList() const
	{
		std::vector<VideoItemPoint> v(2);

		v[0] = VideoItemPoint(m_leftDocPt, m_topDocPt);
		v[1] = VideoItemPoint(m_leftDocPt + m_widthDocPt, m_topDocPt + m_heightDocPt);

		return v;
	}

	void CPosRectImpl::setPointList(const std::vector<VideoItemPoint>& points)
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

