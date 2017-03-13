#include "Stable.h"
#include "FblItemRect.h"
#include "SchemaLayer.h"

#include "SchemaItemSignal.h"
#include "SchemaItemConst.h"
#include "SchemaItemAfb.h"
#include "SchemaItemConnection.h"
#include "SchemaItemTerminator.h"


namespace VFrame30
{
	FblItemRect::FblItemRect(void) :
		FblItemRect(SchemaUnit::Inch)
	{
	}

	FblItemRect::FblItemRect(SchemaUnit unit) :
		m_weight(0),
		m_lineColor(qRgb(0x00, 0x00, 0xC0)),
		m_fillColor(qRgb(0xF0, 0xF0, 0xF0)),
		m_textColor(qRgb(0x00, 0x00, 0xC0))
	{
		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::fontName, PropertyNames::appearanceCategory, true, FblItemRect::getFontName, FblItemRect::setFontName);
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::fontSize, PropertyNames::appearanceCategory, true, FblItemRect::getFontSize, FblItemRect::setFontSize);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::fontBold, PropertyNames::appearanceCategory, true, FblItemRect::getFontBold, FblItemRect::setFontBold);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::fontItalic, PropertyNames::appearanceCategory, true, FblItemRect::getFontItalic, FblItemRect::setFontItalic);

		auto labelProp = ADD_PROPERTY_GETTER(QString, PropertyNames::label, true, SchemaItemAfb::label);
		labelProp->setCategory(PropertyNames::functionalCategory);

		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::userText, PropertyNames::textCategory, true, FblItemRect::userText, FblItemRect::setUserText);
		ADD_PROPERTY_GET_SET_CAT(E::UserTextPos, PropertyNames::userTextPos, PropertyNames::textCategory, true, FblItemRect::userTextPos, FblItemRect::setUserTextPos);

		// --
		//
		setItemUnit(unit);
		m_font.setName("Sans");

		switch (itemUnit())
		{
		case SchemaUnit::Display:
			m_font.setSize(12.0, SchemaUnit::Display);
			break;
		case SchemaUnit::Inch:
			m_font.setSize(1.0 / 8.0, SchemaUnit::Inch);
			break;
		case SchemaUnit::Millimeter:
			m_font.setSize(3.0, SchemaUnit::Millimeter);
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
		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
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
		Proto::FblItemRect* itemMessage = message->mutable_schemaitem()->mutable_fblitemrect();

		itemMessage->set_weight(m_weight);
		itemMessage->set_linecolor(m_lineColor);
		itemMessage->set_fillcolor(m_fillColor);
		itemMessage->set_textcolor(m_textColor);

		m_font.SaveData(itemMessage->mutable_font());

		itemMessage->set_label(m_label.toStdString());
		itemMessage->set_usertext(m_userText.toStdString());
		itemMessage->set_usertextpos(static_cast<::google::protobuf::int32>(m_userTextPos));

		return true;
	}

	bool FblItemRect::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
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

		if (message.schemaitem().has_fblitemrect() == false)
		{
			assert(message.schemaitem().has_fblitemrect());
			return false;
		}

		const Proto::FblItemRect& itemMessage = message.schemaitem().fblitemrect();

		m_weight = itemMessage.weight();
		m_lineColor = itemMessage.linecolor();
		m_fillColor = itemMessage.fillcolor();
		m_textColor = itemMessage.textcolor();

		m_font.LoadData(itemMessage.font());

		m_label = QString::fromStdString(itemMessage.label());

		m_userText = QString::fromStdString(itemMessage.usertext());
		m_userTextPos = static_cast<E::UserTextPos>(itemMessage.usertextpos());

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

				SchemaPoint calculatedPoint = CalcPointPos(ir, *input, inputCount, inputIndex, gridSize, pinGridStep);
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

				SchemaPoint calculatedPoint = CalcPointPos(ir, *output, outputCount, outputIndex, gridSize, pinGridStep);
				output->setPoint(calculatedPoint);

				outputIndex ++;
			}
		}

		return;
	}

	bool FblItemRect::GetConnectionPointPos(const QUuid& connectionPointGuid, SchemaPoint* pResult, double gridSize, int pinGridStep) const
	{
		if (pResult == nullptr)
		{
			assert(pResult);
			return false;
		}

		QRectF ir(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());

		// Look for point in inputs
		//
		const std::vector<AfbPin>& inputPoints = inputs();
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
		const std::vector<AfbPin>& outputPoints = outputs();
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

	SchemaPoint FblItemRect::CalcPointPos(
			const QRectF& fblItemRect,
			const AfbPin& connection,
			int pinCount,
			int index,
			double gridSize,
			int pinGridStep) const
	{
		if (pinCount == 0)
		{
			assert(pinCount != 0);
			return SchemaPoint(0, 0);
		}

		// Cache values
		//
		m_cachedGridSize = gridSize;
		m_cachedPinGridStep = pinGridStep;

		// Calc
		//
//		double x = connection.dirrection() == ConnectionDirrection::Input ? fblItemRect.left() : fblItemRect.right();

//		double pinVertGap =	CUtils::snapToGrid(gridSize * static_cast<double>(pinGridStep), gridSize);
//		double halfpinVertGap =	CUtils::snapToGrid(gridSize * static_cast<double>(pinGridStep) / 2.0, gridSize);

//		double top = CUtils::snapToGrid(fblItemRect.top(), gridSize);

//		double y = top + halfpinVertGap + pinVertGap * static_cast<double>(index);
//		y = CUtils::snapToGrid(y, gridSize);

//		return SchemaPoint(x, y);

		double x = connection.dirrection() == ConnectionDirrection::Input ? fblItemRect.left() : fblItemRect.right();

		double pinVertGap =	gridSize * static_cast<double>(pinGridStep);
		double halfpinVertGap =	gridSize * static_cast<double>(pinGridStep) / 2.0;

		double top = CUtils::snapToGrid(fblItemRect.top(), gridSize);

		double y = top + halfpinVertGap + pinVertGap * static_cast<double>(index);

		y = CUtils::snapToGrid(y, gridSize);
		x = CUtils::snapToGrid(x, gridSize);

		return SchemaPoint(x, y);
	}


	// Drawing Functions
	//

	// Рисование элемента, выполняется в 100% масштабе.
	// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
	//
	void FblItemRect::Draw(CDrawParam* drawParam, const Schema*, const SchemaLayer* layer) const
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
		int dpiX = drawParam->dpiX();

		// Correct rect width
		//
		double pinWidth = GetPinWidth(itemUnit(), dpiX);

		FontParam smallFont = m_font;
		smallFont.setDrawSize(m_font.drawSize() * 0.75);

		if (inputsCount() > 0)
		{
			r.setLeft(r.left() + pinWidth);
		}

		if (outputsCount() > 0)
		{
			r.setRight(r.right() - pinWidth);
		}

		QRectF labelRect(r);	// save rect for future use
		QRectF userTextRect(r);	// save rect for future use

		// Draw main rect
		//
		p->fillRect(r, fillColor());

		//
		QPen pen(lineColor());
		pen.setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);	// Don't use getter!
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
		const std::vector<AfbPin>& inputPins = inputs();

		QPen redPen(QColor(0xE0B00000));
		redPen.setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);	// Don't use getter!

		for (const AfbPin& input : inputPins)
		{
			// Get pin position
			//
			SchemaPoint vip;
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

			DrawHelper::drawText(p,
								 font,
								 itemUnit(),
								 input.caption(),
								 pinTextRect,
								 Qt::TextDontClip | Qt::AlignVCenter | Qt::AlignRight);
		}

		// Drawing output pins
		//
		const std::vector<AfbPin>& outputPins = outputs();

		for (const AfbPin& output : outputPins)
		{
			// Get pin position
			//
			SchemaPoint vip;
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

			DrawHelper::drawText(p,
								 font,
								 itemUnit(),
								 output.caption(),
								 pinTextRect,
								 Qt::TextDontClip | Qt::AlignVCenter | Qt::AlignLeft);
		}

		// Draw UserText
		//
		if (userTextRect.isEmpty() == false)
		{
			int alignFlags = Qt::AlignmentFlag::AlignCenter;
			switch (userTextPos())
			{
			case E::UserTextPos::LeftTop:
				userTextRect.moveBottomRight(userTextRect.topLeft());
				alignFlags = Qt::AlignRight | Qt::AlignBottom;
				break;
			case E::UserTextPos::Top:
				userTextRect.moveBottom(userTextRect.top());
				alignFlags = Qt::AlignHCenter | Qt::AlignBottom;
				break;
			case E::UserTextPos::RightTop:
				userTextRect.moveBottomLeft(userTextRect.topRight());
				alignFlags = Qt::AlignLeft | Qt::AlignBottom;
				break;
			case E::UserTextPos::Right:
				userTextRect.moveLeft(userTextRect.right());
				alignFlags = Qt::AlignLeft | Qt::AlignVCenter;
				break;
			case E::UserTextPos::RightBottom:
				userTextRect.moveTopLeft(userTextRect.bottomRight());
				alignFlags = Qt::AlignLeft | Qt::AlignTop;
				break;
			case E::UserTextPos::Bottom:
				userTextRect.moveTop(userTextRect.bottom());
				alignFlags = Qt::AlignHCenter | Qt::AlignTop;
				break;
			case E::UserTextPos::LeftBottom:
				userTextRect.moveTopRight(userTextRect.bottomLeft());
				alignFlags = Qt::AlignRight | Qt::AlignTop;
				break;
			case E::UserTextPos::Left:
				userTextRect.moveRight(userTextRect.left());
				alignFlags = Qt::AlignRight | Qt::AlignVCenter;
				break;

			default:
				assert(false);
			}

			p->setPen(Qt::black);
			DrawHelper::drawText(p, smallFont, itemUnit(), userText(), userTextRect, Qt::TextDontClip | alignFlags);
		}

		// Draw Label
		//
		if (drawParam->infoMode() == true)
		{
			QString labelText = label();

			labelRect.moveBottomLeft(labelRect.topRight());

			p->setPen(Qt::darkGray);
			DrawHelper::drawText(p, smallFont, itemUnit(), labelText, labelRect, Qt::TextDontClip | Qt::AlignLeft | Qt::AlignBottom);
		}

		return;
	}

	void FblItemRect::DrawDebugInfo(CDrawParam* drawParam, const QString& runOrderIndex) const
	{
		QPainter* p = drawParam->painter();

		QRectF r = itemRectPinIndent(drawParam->device());

		QRectF drawRect(r.right(), r.bottom(),
						widthDocPt(), m_font.drawSize());

		static FontParam font("Sans", drawParam->gridSize() * 1.75, false, false);
		p->setPen(Qt::red);

		QString str = QString("roi %1").arg(runOrderIndex);

		DrawHelper::drawText(p,
							 font,
							 itemUnit(),
							 str,
							 drawRect,
							 Qt::TextDontClip | Qt::AlignBottom | Qt::AlignLeft);
	}


	QRectF FblItemRect::itemRectWithPins() const
	{
		QRectF r(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());
		return r;
	}

	QRectF FblItemRect::itemRectPinIndent(QPaintDevice* paintDevice) const
	{
		if (paintDevice == nullptr)
		{
			assert(paintDevice);
			return QRectF();
		}

		QRectF r(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());

		double pinWidth = GetPinWidth(itemUnit(), paintDevice->physicalDpiX());

		if (inputsCount() > 0)
		{
			r.setLeft(r.left() + pinWidth);
		}
		if (outputsCount() > 0)
		{
			r.setRight(r.right() - pinWidth);
		}

		return r;
	}

	Q_INVOKABLE void FblItemRect::adjustHeight()
	{
		// Here m_gridSize and m_pingGridStep are cached copies from Schema, they set in CalcPointPos
		//
		if (m_cachedGridSize < 0 || m_cachedPinGridStep == 0)
		{
			// Can't do anything, required variables are not set
			//
			//qDebug() << Q_FUNC_INFO << " Variables m_gridSize and m_pingGridStep were not initiazized, cannot perform operation";
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

	void FblItemRect::dump() const
	{
		PosRectImpl::dump();

		qDebug() << "\tBuildName: " << buildName();

		if (inputsCount() != 0)
		{
			qDebug() << "\tInputs: ";
			const std::vector<VFrame30::AfbPin>& ins = inputs();
			for (const VFrame30::AfbPin& pin : ins)
			{
				qDebug() << "\t\tguid: " << pin.guid() << ", opIndex: " << pin.afbOperandIndex() << ", caption: " << pin.caption() << " Pos(x, y):" << pin.x() << pin.y();

				const std::vector<QUuid>& asios = pin.associatedIOs();
				if (asios.empty() == false)
				{
					for (const QUuid& u : asios)
					{
						qDebug() << "\t\t\tAssocIOs: " << u;
					}
				}
			}
		}

		if (outputsCount() != 0)
		{
			qDebug() << "\tOutputs: ";
			const std::vector<VFrame30::AfbPin>& outs = outputs();
			for (const VFrame30::AfbPin& pin : outs)
			{
				qDebug() << "\t\tguid: " << pin.guid() << ", opIndex: " << pin.afbOperandIndex() << ", caption: " << pin.caption() << " Pos(x, y):" << pin.x() << pin.y();

				const std::vector<QUuid>& asios = pin.associatedIOs();
				if (asios.empty() == false)
				{
					for (const QUuid& u : asios)
					{
						qDebug() << "\t\t\tAssocIOs: " << u;
					}
				}
			}
		}

		return;
	}

	bool FblItemRect::searchText(const QString& text) const
	{
		return	SchemaItem::searchText(text) ||
				m_label.contains(text, Qt::CaseInsensitive) ||
				m_userText.contains(text, Qt::CaseInsensitive);
	}

	// Properties and Data
	//
	IMPLEMENT_FONT_PROPERTIES(FblItemRect, Font, m_font);

	void FblItemRect::setNewGuid()
	{
		// Set new guid for the item
		//
		SchemaItem::setNewGuid();

		// Set new guids for all inputs/outputs
		//
		std::vector<AfbPin>* inputs = mutableInputs();

		for (AfbPin& in : *inputs)
		{
			in.setGuid(QUuid::createUuid());
		}

		std::vector<AfbPin>* outputs = mutableOutputs();

		for (AfbPin& out : *outputs)
		{
			out.setGuid(QUuid::createUuid());
		}

		return;
	}

	bool FblItemRect::isInputSignalElement() const
	{
		const VFrame30::SchemaItemInput* ptr = dynamic_cast<const VFrame30::SchemaItemInput*>(this);
		return ptr != nullptr;
	}

	bool FblItemRect::isOutputSignalElement() const
	{
		const VFrame30::SchemaItemOutput* ptr = dynamic_cast<const VFrame30::SchemaItemOutput*>(this);
		return ptr != nullptr;
	}

	bool FblItemRect::isSignalElement() const
	{
		return dynamic_cast<const VFrame30::SchemaItemSignal*>(this) != nullptr;
	}

	bool FblItemRect::isInOutSignalElement() const
	{
		return dynamic_cast<const VFrame30::SchemaItemInOut*>(this) != nullptr;
	}

	bool FblItemRect::isConstElement() const
	{
		const VFrame30::SchemaItemConst* ptr = dynamic_cast<const VFrame30::SchemaItemConst*>(this);
		return ptr != nullptr;
	}

	bool FblItemRect::isAfbElement() const
	{
		const VFrame30::SchemaItemAfb* ptr = dynamic_cast<const VFrame30::SchemaItemAfb*>(this);
		return ptr != nullptr;
	}

	bool FblItemRect::isConnectionElement() const
	{
		return dynamic_cast<const VFrame30::SchemaItemConnection*>(this) != nullptr;
	}

	bool FblItemRect::isReceiverElement() const
	{
		return dynamic_cast<const VFrame30::SchemaItemReceiver*>(this) != nullptr;
	}

	bool FblItemRect::isTransmitterElement() const
	{
		return dynamic_cast<const VFrame30::SchemaItemTransmitter*>(this) != nullptr;
	}

	bool FblItemRect::isTerminatorElement() const
	{
		return dynamic_cast<const VFrame30::SchemaItemTerminator*>(this) != nullptr;
	}

	VFrame30::SchemaItemSignal* FblItemRect::toSignalElement()
	{
		return dynamic_cast<VFrame30::SchemaItemSignal*>(this);
	}

	const VFrame30::SchemaItemSignal* FblItemRect::toSignalElement() const
	{
		return dynamic_cast<const VFrame30::SchemaItemSignal*>(this);
	}

	VFrame30::SchemaItemInput* FblItemRect::toInputSignalElement()
	{
		return dynamic_cast<VFrame30::SchemaItemInput*>(this);
	}

	const VFrame30::SchemaItemInput* FblItemRect::toInputSignalElement() const
	{
		return dynamic_cast<const VFrame30::SchemaItemInput*>(this);
	}

	VFrame30::SchemaItemOutput* FblItemRect::toOutputSignalElement()
	{
		return dynamic_cast<VFrame30::SchemaItemOutput*>(this);
	}

	const VFrame30::SchemaItemOutput* FblItemRect::toOutputSignalElement() const
	{
		return dynamic_cast<const VFrame30::SchemaItemOutput*>(this);
	}

	VFrame30::SchemaItemConst* FblItemRect::toSchemaItemConst()
	{
		return dynamic_cast<VFrame30::SchemaItemConst*>(this);
	}

	const VFrame30::SchemaItemConst* FblItemRect::toSchemaItemConst() const
	{
		return dynamic_cast<const VFrame30::SchemaItemConst*>(this);
	}

	VFrame30::SchemaItemAfb* FblItemRect::toAfbElement()
	{
		return dynamic_cast<VFrame30::SchemaItemAfb*>(this);
	}

	const VFrame30::SchemaItemAfb* FblItemRect::toAfbElement() const
	{
		return dynamic_cast<const VFrame30::SchemaItemAfb*>(this);
	}

	VFrame30::SchemaItemInOut* FblItemRect::toInOutSignalElement()
	{
		return dynamic_cast<VFrame30::SchemaItemInOut*>(this);
	}

	const VFrame30::SchemaItemInOut* FblItemRect::toInOutSignalElement() const
	{
		return dynamic_cast<const VFrame30::SchemaItemInOut*>(this);
	}

	VFrame30::SchemaItemReceiver* FblItemRect::toReceiverElement()
	{
		return dynamic_cast<VFrame30::SchemaItemReceiver*>(this);
	}

	const VFrame30::SchemaItemReceiver* FblItemRect::toReceiverElement() const
	{
		return dynamic_cast<const VFrame30::SchemaItemReceiver*>(this);
	}

	VFrame30::SchemaItemTransmitter* FblItemRect::toTransmitterElement()
	{
		return dynamic_cast<VFrame30::SchemaItemTransmitter*>(this);
	}

	const VFrame30::SchemaItemTransmitter* FblItemRect::toTransmitterElement() const
	{
		return dynamic_cast<const VFrame30::SchemaItemTransmitter*>(this);
	}

	VFrame30::SchemaItemTerminator* FblItemRect::toTerminatorElement()
	{
		return dynamic_cast<VFrame30::SchemaItemTerminator*>(this);
	}

	const VFrame30::SchemaItemTerminator* FblItemRect::toTerminatorElement() const
	{
		return dynamic_cast<const VFrame30::SchemaItemTerminator*>(this);
	}

	// Weight propertie
	//
	double FblItemRect::weight() const
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			return CUtils::RoundDisplayPoint(m_weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(m_weight, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			return CUtils::RoundPoint(pt, Settings::regionalUnit());
		}
	}

	void FblItemRect::setWeight(double weight)
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			m_weight = CUtils::RoundDisplayPoint(weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(weight, Settings::regionalUnit(), SchemaUnit::Inch, 0);
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

	QString FblItemRect::label() const
	{
		return m_label;
	}

	void FblItemRect::setLabel(const QString& value)
	{
		m_label = value;
	}

	QString FblItemRect::userText() const
	{
		return m_userText;
	}

	void FblItemRect::setUserText(const QString& value)
	{
		m_userText = value;
	}

	E::UserTextPos FblItemRect::userTextPos() const
	{
		return m_userTextPos;
	}

	void FblItemRect::setUserTextPos(E::UserTextPos value)
	{
		m_userTextPos = value;
	}

}

