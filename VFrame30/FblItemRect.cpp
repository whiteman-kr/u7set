#include "Stable.h"
#include "FblItemRect.h"
#include "SchemeLayer.h"

#include "VideoItemSignal.h"
#include "SchemeItemConst.h"
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
		m_font.setName("Sans");

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

	//
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


	// Get pin position
	//
	void FblItemRect::SetConnectionsPos(double gridSize, int pinGridStep)
	{
		QRectF ir(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());

		// Inputs
		//
		{
			auto inputs = mutableInputs();
			int inputCount = static_cast<int>(inputs->size());
			int inputIndex = 0;

			for (auto input = inputs->begin(); input != inputs->end(); ++input)
			{
				assert(input->IsInput());

				VideoItemPoint calculatedPoint = CalcPointPos(ir, *input, inputCount, inputIndex, gridSize, pinGridStep);
				input->setPoint(calculatedPoint);

				inputIndex ++;
			}
		}

		// Outputs
		//
		{
			auto outputs = mutableOutputs();
			int outputCount = static_cast<int>(outputs->size());
			int outputIndex = 0;

			for (auto output = outputs->begin(); output != outputs->end(); ++output)
			{
				assert(output->IsOutput());

				VideoItemPoint calculatedPoint = CalcPointPos(ir, *output, outputCount, outputIndex, gridSize, pinGridStep);
				output->setPoint(calculatedPoint);

				outputIndex ++;
			}
		}
				
		return;
	}

	bool FblItemRect::GetConnectionPointPos(const QUuid& connectionPointGuid, VideoItemPoint* pResult, double gridSize, int pinGridStep) const
	{
		if (pResult == nullptr)
		{
			assert(pResult);
			return false;
		}

		QRectF ir(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());

		// Look for point in inputs
		//
		const std::list<CFblConnectionPoint>& inputPoints = inputs();
		int inputCount = inputsCount();
		int index = 0;

		for (auto pt = inputPoints.cbegin(); pt != inputPoints.cend(); ++pt)
		{
			assert(pt->dirrection() == ConnectionDirrection::Input);

			if (pt->guid() == connectionPointGuid)
			{
				*pResult = CalcPointPos(ir, *pt, inputCount, index, gridSize, pinGridStep);
				return true;
			}

			index ++;
		}

		// Look for point in outputs
		//
		const std::list<CFblConnectionPoint>& outputPoints = outputs();
		int outputCount = outputsCount();
		index = 0;

		for (auto pt = outputPoints.cbegin(); pt != outputPoints.cend(); ++pt)
		{
			assert(pt->dirrection() == ConnectionDirrection::Output);

			if (pt->guid() == connectionPointGuid)
			{
				*pResult = CalcPointPos(ir, *pt, outputCount, index, gridSize, pinGridStep);
				return true;
			}

			index ++;
		}
		
		// The point is not found
		//
		assert(false);
		return false;
	}

	VideoItemPoint FblItemRect::CalcPointPos(
			const QRectF& fblItemRect,
			const CFblConnectionPoint& connection,
			int pinCount,
			int index,
			double gridSize,
			int pinGridStep) const
	{
		if (pinCount == 0)
		{
			assert(pinCount != 0);
			return VideoItemPoint(0, 0);
		}

		// Cache values
		//
		m_cachedGridSize = gridSize;
		m_cachedPinGridStep = pinGridStep;

		// Calc
		//
		double x = connection.dirrection() == ConnectionDirrection::Input ? fblItemRect.left() : fblItemRect.right();

		double pinVertGap =	CUtils::snapToGrid(gridSize * static_cast<double>(pinGridStep), gridSize);
		double halfpinVertGap =	CUtils::snapToGrid(gridSize * static_cast<double>(pinGridStep) / 2.0, gridSize);

		double top = CUtils::snapToGrid(fblItemRect.top(), gridSize);

		double y = top + halfpinVertGap + pinVertGap * static_cast<double>(index);
		y = CUtils::snapToGrid(y, gridSize);

		return VideoItemPoint(x, y);
	}


	// Drawing Functions
	//

	// ��������� ��������, ����������� � 100% ��������.
	// Graphcis ������ ����� �������� ������������ ������� (0, 0 - ����� ������� ����, ���� � ������ - ������������� ����������)
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
		
		// Correct rect width
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

		// Draw main rect
		//
		p->fillRect(r, fillColor());

		//
		QPen pen(lineColor());
		pen.setWidthF(m_weight);		// Don't use getter! 
		p->setPen(pen);

		p->drawRect(r);
		
		// Draw in/outs
		//
		if (inputsCount() == 0 && outputsCount() == 0)
		{
			return;
		}

		// Draw input pins
		//
		const std::list<CFblConnectionPoint>& inputPins = inputs();

		QPen redPen(QColor(0xE0B00000));
		redPen.setWidthF(m_weight);		// Don't use getter!
		
		for (const CFblConnectionPoint& input : inputPins)
		{
			// Get pin position
			//
			VideoItemPoint vip;
			GetConnectionPointPos(input.guid(), &vip, drawParam->gridSize(), drawParam->pinGridStep());

			int connectionCount = layer->GetPinPosConnectinCount(vip, itemUnit());

			// Drawing pin
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
				// Draw red cross error mark
				//
				p->setPen(redPen);
				DrawPinCross(p, pt1.x(), pt1.y(), pinWidth);
			}

			// Draw pin text
			//
			QRectF pinTextRect;
			pinTextRect.setLeft(vip.X - pinWidth * 3);
			pinTextRect.setTop(vip.Y - m_font.drawSize() * 1.2);
			pinTextRect.setWidth(pinWidth * 3.8);
			pinTextRect.setHeight(m_font.drawSize() * 1.2);

			FontParam font = m_font;
			font.setDrawSize(m_font.drawSize() * 0.75);

			DrawHelper::DrawText(p,
								 font,
								 itemUnit(),
								 input.caption(),
								 pinTextRect,
								 Qt::TextDontClip | Qt::AlignVCenter | Qt::AlignRight);
		}

		// Drawing output pins
		//
		const std::list<CFblConnectionPoint>& outputPins = outputs();

		for (const CFblConnectionPoint& output : outputPins)
		{
			// Get pin position
			//
			VideoItemPoint vip;
			GetConnectionPointPos(output.guid(), &vip, drawParam->gridSize(), drawParam->pinGridStep());

			int connectionCount = layer->GetPinPosConnectinCount(vip, itemUnit());

			// Draw pin
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
				// Draw red cross error mark
				//
				p->setPen(redPen);
				DrawPinCross(p, pt1.x(), pt1.y(), pinWidth);
			}

			// Draw pin text
			//
			QRectF pinTextRect;
			pinTextRect.setLeft(pt2.x() + pinWidth * 0.2);
			pinTextRect.setTop(vip.Y - m_font.drawSize() * 1.2);
			pinTextRect.setWidth(pinWidth * 4.0);
			pinTextRect.setHeight(m_font.drawSize() * 1.2);

			FontParam font = m_font;
			font.setDrawSize(m_font.drawSize() * 0.75);

			DrawHelper::DrawText(p,
								 font,
								 itemUnit(),
								 output.caption(),
								 pinTextRect,
								 Qt::TextDontClip | Qt::AlignVCenter | Qt::AlignLeft);
		}
		
		return;
	}


	Q_INVOKABLE void FblItemRect::adjustHeight()
	{
		// Here m_gridSize and m_pingGridStep are cached copies from Scheme, they set in CalcPointPos
		//
		if (m_cachedGridSize < 0 || m_cachedPinGridStep == 0)
		{
			// Can't do anything, required variables are not set
			//
			qDebug() << Q_FUNC_INFO << " Variables m_gridSize and m_pingGridStep were not initiazized, cannot perform operation";
			return;
		}

		double minHeight = minimumPossibleHeightDocPt(m_cachedGridSize, m_cachedPinGridStep);

		if (heightDocPt() < minHeight)
		{
			setHeightDocPt(minHeight);
		}

		return;
	}

	double FblItemRect::minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const
	{
		// Cache values
		//
		m_cachedGridSize = gridSize;
		m_cachedPinGridStep = pinGridStep;

		// --
		//
		int pinCount = std::max(inputsCount(), outputsCount());
		if (pinCount == 0)
		{
			pinCount = 1;
		}

		double pinVertGap =	CUtils::snapToGrid(gridSize * static_cast<double>(pinGridStep), gridSize);
		double minHeight = CUtils::snapToGrid(pinVertGap * static_cast<double>(pinCount), gridSize);

		return minHeight;
	}

	double FblItemRect::minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const
	{
		// Cache values
		//
		m_cachedGridSize = gridSize;
		m_cachedPinGridStep = pinGridStep;

		// --
		//
		return m_cachedGridSize * 16;
	}

	// Properties and Data
	//
	IMPLEMENT_FONT_PROPERTIES(FblItemRect, Font, m_font);

	void FblItemRect::setNewGuid()
	{
		// Set new guid for the item
		//
		VideoItem::setNewGuid();

		// Set new guids for all inputs/outputs
		//
		std::list<CFblConnectionPoint>* inputs = mutableInputs();

		for (CFblConnectionPoint& in : *inputs)
		{
			in.setGuid(QUuid::createUuid());
		}

		std::list<CFblConnectionPoint>* outputs = mutableOutputs();

		for (CFblConnectionPoint& out : *outputs)
		{
			out.setGuid(QUuid::createUuid());
		}

		return;
	}

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

	bool FblItemRect::isConstElement() const
	{
		const VFrame30::SchemeItemConst* ptr = dynamic_cast<const VFrame30::SchemeItemConst*>(this);
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

	VFrame30::SchemeItemConst* FblItemRect::toSchemeItemConst()
	{
		return dynamic_cast<VFrame30::SchemeItemConst*>(this);
	}

	const VFrame30::SchemeItemConst* FblItemRect::toSchemeItemConst() const
	{
		return dynamic_cast<const VFrame30::SchemeItemConst*>(this);
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
			double pt = CUtils::ConvertPoint(m_weight, SchemeUnit::Inch, Settings::regionalUnit(), ConvertDirection::Horz);
			return CUtils::RoundPoint(pt, Settings::regionalUnit());
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
			double pt = CUtils::ConvertPoint(weight, Settings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Horz);
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

