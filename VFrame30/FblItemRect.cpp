#include "Stable.h"
#include "FblItemRect.h"
#include "VideoLayer.h"

namespace VFrame30
{
	CFblItemRect::CFblItemRect(void)
	{
	}

	CFblItemRect::CFblItemRect(SchemeUnit unit) :
		m_weight(0),
		m_lineColor(qRgb(0x00, 0x00, 0x00)),
		m_fillColor(qRgb(0xC0, 0xC0, 0xC0)),
		m_textColor(qRgb(0x00, 0x00, 0x00))
	{
		setItemUnit(unit);
		m_font.name = "Arial";

		switch (itemUnit())
		{
		case SchemeUnit::Display:
			m_font.size = 12.0;
			break;
		case SchemeUnit::Inch:
			m_font.size = mm2in(3.0);
			break;
		case SchemeUnit::Millimeter:
			m_font.size = mm2in(3.0);
			break;
		default:
			assert(false);
		}

		m_static = false;
	}

	CFblItemRect::~CFblItemRect(void)
	{
	}

	// ��������� ���������� �����
	//
	void CFblItemRect::SetConnectionsPos()
	{
		QRectF ir(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());

		// ������ �������� ������
		//
		{
			auto inputs = mutableInputs();
			int inputCount = static_cast<int>(inputs->size());
			int inputIndex = 0;

			for (auto input = inputs->begin(); input != inputs->end(); ++input)
			{
				assert(input->IsInput());

				VideoItemPoint calculatedPoint = CalcPointPos(ir, *input, inputCount, inputIndex);
				input->setPoint(calculatedPoint);

				inputIndex ++;
			}
		}

		// ������ �������� �������
		//
		{
			auto outputs = mutableOutputs();
			int outputCount = static_cast<int>(outputs->size());
			int outputIndex = 0;

			for (auto output = outputs->begin(); output != outputs->end(); ++output)
			{
				assert(output->IsOutput());

				VideoItemPoint calculatedPoint = CalcPointPos(ir, *output, outputCount, outputIndex);
				output->setPoint(calculatedPoint);

				outputIndex ++;
			}
		}
				
		return;
	}

	bool CFblItemRect::GetConnectionPointPos(const QUuid& connectionPointGuid, VideoItemPoint* pResult) const
	{
		if (pResult == nullptr)
		{
			assert(pResult);
			return false;
		}

		QRectF ir(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());

		// ������ ����� �� ������
		//
		const std::list<CFblConnectionPoint>& inputPoints = inputs();
		int inputCount = inputsCount();
		int index = 0;

		for (auto pt = inputPoints.cbegin(); pt != inputPoints.cend(); ++pt)
		{
			assert(pt->dirrection() == ConnectionDirrection::Input);

			if (pt->guid() == connectionPointGuid)
			{
				*pResult = CalcPointPos(ir, *pt, inputCount, index);
				return true;
			}

			index ++;
		}

		// ������ ����� � �������
		//
		const std::list<CFblConnectionPoint>& outputPoints = outputs();
		int outputCount = outputsCount();
		index = 0;

		for (auto pt = outputPoints.cbegin(); pt != outputPoints.cend(); ++pt)
		{
			assert(pt->dirrection() == ConnectionDirrection::Output);

			if (pt->guid() == connectionPointGuid)
			{
				*pResult = CalcPointPos(ir, *pt, outputCount, index);
				return true;
			}

			index ++;
		}
		
		// ����� �� �������
		//
		assert(false);
		return false;
	}

	VideoItemPoint CFblItemRect::CalcPointPos(const QRectF& fblItemRect, const CFblConnectionPoint& connection, int pinCount, int index) const
	{
		if (pinCount == 0)
		{
			assert(pinCount != 0);
			return VideoItemPoint(0, 0);
		}

		double x = connection.dirrection() == ConnectionDirrection::Input ? fblItemRect.left() : fblItemRect.right();
		double minFblGridSize = CSettings::minFblGridSize(itemUnit());

		// ������������ ���������� ����� ������
		//
		double height = fblItemRect.height();
		height = CUtils::Round(height, 6);

		double pinVertGap = height / pinCount;

		double y = pinVertGap / 2 + pinVertGap * index;
		y = floor(y / minFblGridSize) * minFblGridSize;			// ��������� �� �����
		y += fblItemRect.top();

		return VideoItemPoint(x, y);
	}
	
	// Serialization
	//
	bool CFblItemRect::SaveData(Proto::Envelope* message) const
	{
		bool result = CPosRectImpl::SaveData(message);
		if (result == false || message->has_videoitem() == false)
		{
			assert(result);
			assert(message->has_videoitem());
			return false;
		}

		// --
		//
		result = CFblItem::SaveData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		Proto::FblItemRect* itemMessage = message->mutable_videoitem()->mutable_fblitemrect();

		itemMessage->set_weight(m_weight);
		itemMessage->set_linecolor(m_lineColor);
		itemMessage->set_fillcolor(m_fillColor);
		itemMessage->set_textcolor(m_textColor);

		m_font.SaveData(itemMessage->mutable_font());

		return true;
	}

	bool CFblItemRect::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return false;
		}

		// --
		//
		bool result = CPosRectImpl::LoadData(message);
		if (result == false)
		{
			return false;
		}

		result = CFblItem::LoadData(message);
		if (result == false)
		{
			return false;
		}
		
		if (message.videoitem().has_fblitemrect() == false)
		{
			assert(message.videoitem().has_fblitemrect());
			return false;
		}

		const Proto::FblItemRect& itemMessage = message.videoitem().fblitemrect();

		m_weight = itemMessage.weight();
		m_lineColor = itemMessage.linecolor();
		m_fillColor = itemMessage.fillcolor();
		m_textColor = itemMessage.textcolor();

		m_font.LoadData(itemMessage.font());

		return true;
	}


	// Drawing Functions
	//

	// ��������� ��������, ����������� � 100% ��������.
	// Graphcis ������ ����� �������� ������������ ������� (0, 0 - ����� ������� ����, ���� � ������ - ������������� ����������)
	//
	void CFblItemRect::Draw(CDrawParam* drawParam, const CVideoFrame*, const CVideoLayer* pLayer) const
	{
		QPainter* p = drawParam->painter();
		p->setBrush(Qt::NoBrush);

		QRectF r(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());

		if (std::abs(r.left() - r.right()) < 0.000001)
		{
			r.setRight(r.left() + 0.000001);
		}

		if (std::abs(r.bottom() - r.top()) < 0.000001)
		{
			r.setBottom(r.top() + 0.000001);
		}

		// --
		//
		int dpiX = 96;
		QPaintDevice* pPaintDevice = p->device();
		if (pPaintDevice == nullptr)
		{
			assert(pPaintDevice);
			dpiX = 96;
		}
		else
		{
			dpiX = pPaintDevice->logicalDpiX();
		}
		
		// ���������� ������������� � ������ ������ �������
		//
		double pinWidth = GetPinWidth(itemUnit(), dpiX);

		if (inputsCount() > 0)
		{
			r.setLeft(r.left() + pinWidth);
		}

		if (outputsCount() > 0)
		{
			r.setRight(r.right() - pinWidth);
		}

		// ���������� ��������������
		//
		p->fillRect(r, fillColor());

		//
		QPen pen(lineColor());
		pen.setWidthF(m_weight);		// Don't use getter! 
		p->setPen(pen);

		p->drawRect(r);
		
		// ���� ������/�������
		//
		if (inputsCount() == 0 && outputsCount() == 0)
		{
			return;
		}

		// ��������� ������� �����
		//
		const std::list<CFblConnectionPoint>& inputPins = inputs();

		QPen redPen(QColor(0xE0B00000));
		redPen.setWidthF(m_weight);		// Don't use getter!
		
		for (auto input = inputPins.cbegin(); input != inputPins.cend(); ++input)
		{
			// ����������� ��������� ����
			//
			VideoItemPoint vip;
			GetConnectionPointPos(input->guid(), &vip);

			int connectionCount = pLayer->GetPinPosConnectinCount(vip);

			// ��������� ����
			//
			QPointF pt1(vip.X, vip.Y);
			QPointF pt2(vip.X + pinWidth, vip.Y);

			p->setPen(pen);
			p->drawLine(pt1, pt2);

			if (connectionCount > 1)
			{
				p->setBrush(pen.color());
				p->setPen(pen);
				DrawPinJoint(p, pt1.x(), pt1.y(), pinWidth);
				p->setBrush(Qt::NoBrush);
			}
			else
			{
				// ��������� �������� ������
				//
				p->setPen(redPen);
				DrawPinCross(p, pt1.x(), pt1.y(), pinWidth);
			}
		}

		// ��������� �������� �����
		//
		const std::list<CFblConnectionPoint>& outputPins = outputs();

		for (auto output = outputPins.cbegin(); output != outputPins.cend(); ++output)
		{
			// ����������� ��������� ����
			//
			VideoItemPoint vip;
			GetConnectionPointPos(output->guid(), &vip);

			int connectionCount = pLayer->GetPinPosConnectinCount(vip);

			// ��������� ����
			//
			QPointF pt1(vip.X, vip.Y);
			QPointF pt2(vip.X - pinWidth, vip.Y);

			p->setPen(pen);
			p->drawLine(pt1, pt2);

			if (connectionCount > 1)
			{
				p->setBrush(pen.color());
				p->setPen(pen);
				DrawPinJoint(p, pt1.x(), pt1.y(), pinWidth);
				p->setBrush(Qt::NoBrush);
			}
			else
			{
				// ��������� �������� ������
				//
				p->setPen(redPen);
				DrawPinCross(p, pt1.x(), pt1.y(), pinWidth);
			}
		}
		
		return;
	}


	// Properties and Data
	//
	IMPLEMENT_FONT_PROPERTIES(CFblItemRect, Font, m_font);

	bool CFblItemRect::IsFblItem() const
	{
		return true;
	}
	
	// Weight propertie
	//
	double CFblItemRect::weight() const
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			return CUtils::RoundDisplayPoint(m_weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(m_weight, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Horz);
			return CUtils::RoundPoint(pt, CSettings::regionalUnit());
		}
	}

	void CFblItemRect::setWeight(double weight)
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			m_weight = CUtils::RoundDisplayPoint(weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(weight, CSettings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Horz);
			m_weight = pt;
		}
	}

	// LineColor propertie
	//
	QRgb CFblItemRect::lineColor() const
	{
		return m_lineColor;
	}

	void CFblItemRect::setLineColor(QRgb color)
	{
		m_lineColor = color;
	}

	// FillColor propertie
	//
	QRgb CFblItemRect::fillColor() const
	{
		return m_fillColor;
	}

	void CFblItemRect::setFillColor(QRgb color)
	{
		m_fillColor = color;
	}

	QRgb CFblItemRect::textColor() const
	{
		return m_textColor;
	}
	void CFblItemRect::setTextColor(QRgb color)
	{
		m_textColor = color;
	}
}

