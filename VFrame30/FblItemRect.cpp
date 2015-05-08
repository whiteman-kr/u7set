#include "Stable.h"
#include "FblItemRect.h"
#include "SchemeLayer.h"

#include "VideoItemSignal.h"
#include "VideoItemFblElement.h"


namespace VFrame30
{
	FblItemRect::FblItemRect(void)
	{
	}

	FblItemRect::FblItemRect(SchemeUnit unit) :
		m_weight(0),
		m_lineColor(qRgb(0x00, 0x00, 0xC0)),
		m_fillColor(qRgb(0xF0, 0xF0, 0xF0)),
		m_textColor(qRgb(0x00, 0x00, 0xC0))
	{
		setItemUnit(unit);
		m_font.setName("Arial");

		switch (itemUnit())
		{
		case SchemeUnit::Display:
			m_font.setSize(10.0, SchemeUnit::Display);
			break;
		case SchemeUnit::Inch:
			m_font.setSize(mm2in(2.2), SchemeUnit::Inch);
			break;
		case SchemeUnit::Millimeter:
			m_font.setSize(2.2, SchemeUnit::Millimeter);
			break;
		default:
			assert(false);
		}

		m_static = false;
	}

	FblItemRect::~FblItemRect(void)
	{
	}

	// Вычислить координаты точки
	//
	void FblItemRect::SetConnectionsPos()
	{
		QRectF ir(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());

		// посчет кооринат входов
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

		// посчет кооринат выходов
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

	bool FblItemRect::GetConnectionPointPos(const QUuid& connectionPointGuid, VideoItemPoint* pResult) const
	{
		if (pResult == nullptr)
		{
			assert(pResult);
			return false;
		}

		QRectF ir(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());

		// Искать точку во входах
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

		// Искать точку в выходах
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
		
		// Точка не найдена
		//
		assert(false);
		return false;
	}

	VideoItemPoint FblItemRect::CalcPointPos(const QRectF& fblItemRect, const CFblConnectionPoint& connection, int pinCount, int index) const
	{
		if (pinCount == 0)
		{
			assert(pinCount != 0);
			return VideoItemPoint(0, 0);
		}

		double x = connection.dirrection() == ConnectionDirrection::Input ? fblItemRect.left() : fblItemRect.right();
		double minFblGridSize = CSettings::minFblGridSize(CSettings::regionalUnit());

		// вертикальное расстояние между пинами
		//
		double height = fblItemRect.height();

		double pinVertGap = height / static_cast<double>(pinCount + 1);
		pinVertGap = CUtils::snapToGrid(pinVertGap, minFblGridSize);

		double y = fblItemRect.top() + pinVertGap * static_cast<double>(index + 1);
		y = CUtils::snapToGrid(y ,minFblGridSize);

		return VideoItemPoint(x, y);
	}
	
	// Serialization
	//
	bool FblItemRect::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectImpl::SaveData(message);
		if (result == false || message->has_videoitem() == false)
		{
			assert(result);
			assert(message->has_videoitem());
			return false;
		}

		// --
		//
		result = FblItem::SaveData(message);
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

	bool FblItemRect::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
			return false;
		}

		// --
		//
		bool result = PosRectImpl::LoadData(message);
		if (result == false)
		{
			return false;
		}

		result = FblItem::LoadData(message);
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

	// Рисование элемента, выполняется в 100% масштабе.
	// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
	//
	void FblItemRect::Draw(CDrawParam* drawParam, const Scheme*, const SchemeLayer* layer) const
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
		
		// Подправить прямоугольник с учетом входов выходов
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

		// Рисолвание прямоугольника
		//
		p->fillRect(r, fillColor());

		//
		QPen pen(lineColor());
		pen.setWidthF(m_weight);		// Don't use getter! 
		p->setPen(pen);

		p->drawRect(r);
		
		// Пины входов/выходов
		//
		if (inputsCount() == 0 && outputsCount() == 0)
		{
			return;
		}

		// Рисование входных пинов
		//
		const std::list<CFblConnectionPoint>& inputPins = inputs();

		QPen redPen(QColor(0xE0B00000));
		redPen.setWidthF(m_weight);		// Don't use getter!
		
		for (auto input = inputPins.cbegin(); input != inputPins.cend(); ++input)
		{
			// Определение координат пина
			//
			VideoItemPoint vip;
			GetConnectionPointPos(input->guid(), &vip);

			int connectionCount = layer->GetPinPosConnectinCount(vip, itemUnit());

			// Рисование пина
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
				// рисование красного креста
				//
				p->setPen(redPen);
				DrawPinCross(p, pt1.x(), pt1.y(), pinWidth);
			}
		}

		// рисование выходных пинов
		//
		const std::list<CFblConnectionPoint>& outputPins = outputs();

		for (auto output = outputPins.cbegin(); output != outputPins.cend(); ++output)
		{
			// Определение координат пина
			//
			VideoItemPoint vip;
			GetConnectionPointPos(output->guid(), &vip);

			int connectionCount = layer->GetPinPosConnectinCount(vip, itemUnit());

			// Рисование пина
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
				// рисование красного креста
				//
				p->setPen(redPen);
				DrawPinCross(p, pt1.x(), pt1.y(), pinWidth);
			}
		}
		
		return;
	}


	// Properties and Data
	//
	IMPLEMENT_FONT_PROPERTIES(FblItemRect, Font, m_font);

	bool FblItemRect::IsFblItem() const
	{
		return true;
	}

	bool FblItemRect::isInputSignalElement() const
	{
		const VFrame30::VideoItemInputSignal* ptr = dynamic_cast<const VFrame30::VideoItemInputSignal*>(this);
		return ptr != nullptr;
	}

	bool FblItemRect::isOutputSignalElement() const
	{
		const VFrame30::VideoItemOutputSignal* ptr = dynamic_cast<const VFrame30::VideoItemOutputSignal*>(this);
		return ptr != nullptr;
	}

	bool FblItemRect::isFblElement() const
	{
		const VFrame30::VideoItemFblElement* ptr = dynamic_cast<const VFrame30::VideoItemFblElement*>(this);
		return ptr != nullptr;
	}

	bool FblItemRect::isSignalElement() const
	{
		return isInputSignalElement() || isOutputSignalElement();
	}

	VFrame30::VideoItemSignal* FblItemRect::toSignalElement()
	{
		return dynamic_cast<VFrame30::VideoItemSignal*>(this);
	}

	const VFrame30::VideoItemSignal* FblItemRect::toSignalElement() const
	{
		return dynamic_cast<const VFrame30::VideoItemSignal*>(this);
	}

	VFrame30::VideoItemInputSignal* FblItemRect::toInputSignalElement()
	{
		return dynamic_cast<VFrame30::VideoItemInputSignal*>(this);
	}

	const VFrame30::VideoItemInputSignal* FblItemRect::toInputSignalElement() const
	{
		return dynamic_cast<const VFrame30::VideoItemInputSignal*>(this);
	}

	VFrame30::VideoItemOutputSignal* FblItemRect::toOutputSignalElement()
	{
		return dynamic_cast<VFrame30::VideoItemOutputSignal*>(this);
	}

	const VFrame30::VideoItemOutputSignal* FblItemRect::toOutputSignalElement() const
	{
		return dynamic_cast<const VFrame30::VideoItemOutputSignal*>(this);
	}

	VFrame30::VideoItemFblElement* FblItemRect::toFblElement()
	{
		return dynamic_cast<VFrame30::VideoItemFblElement*>(this);
	}

	const VFrame30::VideoItemFblElement* FblItemRect::toFblElement() const
	{
		return dynamic_cast<const VFrame30::VideoItemFblElement*>(this);
	}
	
	// Weight propertie
	//
	double FblItemRect::weight() const
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

	void FblItemRect::setWeight(double weight)
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
	QRgb FblItemRect::lineColor() const
	{
		return m_lineColor;
	}

	void FblItemRect::setLineColor(QRgb color)
	{
		m_lineColor = color;
	}

	// FillColor propertie
	//
	QRgb FblItemRect::fillColor() const
	{
		return m_fillColor;
	}

	void FblItemRect::setFillColor(QRgb color)
	{
		m_fillColor = color;
	}

	QRgb FblItemRect::textColor() const
	{
		return m_textColor;
	}
	void FblItemRect::setTextColor(QRgb color)
	{
		m_textColor = color;
	}
}

