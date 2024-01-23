#include "SchemaItemAfb.h"
#include "DrawParam.h"
#include "PropertyNames.h"
#include "Schema.h"
#include "SchemaView.h"

namespace VFrame30
{
	SchemaItemAfb::SchemaItemAfb(void) :
		SchemaItemAfb(SchemaUnit::Inch)
	{
		// This constructor can be called while serialization,
		// Object will be created and serialization will initialize all the members.
		//
	}

	SchemaItemAfb::SchemaItemAfb(SchemaUnit unit) :
		FblItemRect(unit)
	{
		auto precisionProp = ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::precision, true, SchemaItemAfb::precision, SchemaItemAfb::setPrecision);
		precisionProp->setCategory(PropertyNames::functionalCategory);
	}

	SchemaItemAfb::SchemaItemAfb(SchemaUnit unit, const Afb::AfbElement& fblElement, QString* errorMsg) :
		FblItemRect(unit),
		m_afbElement(fblElement)
	{
		assert(errorMsg);

		auto precisionProp = ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::precision, true, SchemaItemAfb::precision, SchemaItemAfb::setPrecision);
		precisionProp->setCategory(PropertyNames::functionalCategory);

		// Add tags for scripting
		//
		addTag("afb");
		addTag(fblElement.caption());

		// Create input output signals in VFrame30::FblItem
		//
		updateAfbElement(fblElement, errorMsg);

		addSpecificParamProperties();

		return;
	}

	void SchemaItemAfb::draw(CDrawParam* drawParam) const
	{
		QPainter* p = drawParam->painter();

		//double dpiX = drawParam->realDpiX();
		//double pinWidth = GetPinWidth(itemUnit(), dpiX);

		FontParam smallFont = m_font;
		smallFont.setDrawSize(m_font.drawSize() * 0.75);

		// Draw rect and pins
		//
		FblItemRect::draw(drawParam);

		// Draw other
		//
		QRectF r = itemRectPinIndent(drawParam);

		r.setLeft(r.left() + m_font.drawSize() / 4.0);
		r.setRight(r.right() - m_font.drawSize() / 4.0);

		// Draw caption
		//
		QString text = m_afbElement.caption();

		p->setPen(textColor());

#ifdef VFRAME30_CACHE_DRAW_TEXT
		if (drawParam->pdfMode() == true)
		{
			DrawHelper::drawText(p, m_font, itemUnit(), text, r, Qt::AlignHCenter | Qt::AlignTop);
		}
		else
		{
			DrawHelper::drawTextCahed(p, m_font, itemUnit(), text, r, Qt::AlignHCenter | Qt::AlignTop, drawParam->schemaView()->zoom());
		}
#else
		DrawHelper::drawText(p, m_font, itemUnit(), text, r, Qt::AlignHCenter | Qt::AlignTop);
#endif

		// Draw params
		//
		text.clear();
		r.setTop(topDocPt() + m_font.drawSize() * 1.2);
		r.setBottom(r.bottom() - m_font.drawSize() / 8.0);

		const std::vector<Afb::AfbParam>& params = m_afbElement.params();

		for (size_t i = 0; i < params.size(); i++)
		{
			const Afb::AfbParam& param = params[i];

			if (param.visible() == false)
			{
				continue;
			}

			QString paramValue = getAfbParamValueText(param);

			// Param string LOWERCASED
			//
			QString paramStr = QString("%1: %2 %3").arg(param.caption(), paramValue, param.units()).toLower();

			if (text.isEmpty() == true)
			{
				text = paramStr;
			}
			else
			{
				text.append(QString("\n%1").arg(paramStr));
			}
		}

		if (isPackedLogic() == true)
		{
			if (text.isEmpty() == false)
			{
				text.append("\n--");
			}

			text += "\n" + packedLogicId();
		}

		p->setPen(textColor());

#ifdef VFRAME30_CACHE_DRAW_TEXT
		if (drawParam->pdfMode() == true)
		{
			DrawHelper::drawText(p, smallFont, itemUnit(), text, r, Qt::AlignLeft | Qt::AlignBottom);
		}
		else
		{
			DrawHelper::drawTextCahed(p, smallFont, itemUnit(), text, r, Qt::AlignLeft | Qt::AlignBottom, drawParam->schemaView()->zoom());
		}
#else
		DrawHelper::drawText(p, smallFont, itemUnit(), text, r, Qt::AlignLeft | Qt::AlignBottom);
#endif
		return;
	}

	void SchemaItemAfb::drawAfbHelp(QPainter* painter, const QRect& drawRect) const
	{
		if (painter == nullptr ||
			drawRect.isEmpty() == true)
		{
			assert(painter);
			return;
		}

		auto drawTextFunc =
			[](QPainter* p, const QRectF rect, QString text, int flags)
		{
			p->save();
			p->resetTransform();

			QRectF textRect(rect.left() * p->device()->physicalDpiX(),
							rect.top() * p->device()->physicalDpiY(),
							rect.width() * p->device()->physicalDpiX(),
							rect.height() * p->device()->physicalDpiY());

			p->drawText(textRect, flags, text);
			p->restore();
		};

		auto pinTypeText =
			[](E::SignalType type, E::DataFormat dataFormat) -> QString
		{
			QString result = "UNK";

			switch (type)
			{
			case E::SignalType::Analog:
				switch (dataFormat)
				{
				case E::DataFormat::UnsignedInt:
					result = "UINT";
					break;
				case E::DataFormat::SignedInt:
					result = "SINT";
					break;
				case E::DataFormat::Float:
					result = "FLOAT";
					break;
				default:
					assert(false);
				}
				break;
			case E::SignalType::Discrete:
				result = "DISCR";
				break;
			case E::SignalType::Bus:
				result = "BUS";
				break;
			default:
				assert(false);
				break;
			}

			return result;
		};

		QPainter* p = painter;

		const Afb::AfbElement& afb = afbElement();

		// set DPI independent draw
		//
		p->scale(p->device()->physicalDpiX(), p->device()->physicalDpiY());

		const double intend = 1.0 / 4.0;
		const double pinWidth = 2.0 / 4.0;
		const double pinHeight = static_cast<double>(p->fontInfo().pixelSize()) / p->device()->physicalDpiY() * 1.25;
		const double typeWidth = 2.0 / 4.0;

		QRectF rect(static_cast<double>(drawRect.left()) / p->device()->physicalDpiX(),
					static_cast<double>(drawRect.top()) / p->device()->physicalDpiY(),
					static_cast<double>(drawRect.width()) / p->device()->physicalDpiX(),
					static_cast<double>(drawRect.height()) / p->device()->physicalDpiY());

		// --
		//
		QPen pen(lineColor());
		pen.setWidth(0);
		p->setPen(pen);

		QRectF itemRect(rect.left() + intend + pinWidth,
						rect.top() + intend,
						rect.width() - (intend + pinWidth) * 2.0,
						pinHeight * qMax(inputsCount(), outputsCount()));

		if (itemRect.width() < 1.5)
		{
			itemRect.setWidth(1.5);
		}

		p->drawRect(itemRect);

		p->drawLine(QPointF(itemRect.left() + typeWidth, itemRect.top()),
					QPointF(itemRect.left() + typeWidth, itemRect.bottom()));

		p->drawLine(QPointF(itemRect.right() - typeWidth, itemRect.top()),
					QPointF(itemRect.right() - typeWidth, itemRect.bottom()));

		// Draw caption
		//
		QRectF captionRect(itemRect.left(),
						   0,
						   itemRect.width(),
						   intend);

		drawTextFunc(p, captionRect, afb.caption(), Qt::AlignCenter | Qt::TextDontClip);

		// Draw input pins
		//
		const std::vector<AfbPin>& inputPins = inputs();
		double pinY = intend + pinHeight / 2.0;

		for (const AfbPin& input : inputPins)
		{
			// Drawing pin
			//
			p->drawLine(QPointF(intend, pinY),
						QPointF(itemRect.left(), pinY));

			// Draw pin text
			//
			QRectF pinTextRect(intend,
							   pinY - pinHeight,
							   pinWidth,
							   pinHeight);

			drawTextFunc(p, pinTextRect, input.caption() + " ", Qt::AlignRight | Qt::AlignBaseline | Qt::TextDontClip);

			// Draw pin type
			//
			{
				QRectF pinTypeRect(itemRect.left(),
								   pinY - pinHeight / 2.0,
								   typeWidth,
								   pinHeight);

				bool found = false;
				const std::vector<Afb::AfbSignal>& afbInputs = afb.inputSignals();
				for (const Afb::AfbSignal& afbPin : afbInputs)
				{
					if (afbPin.caption() == input.caption())
					{
						QString str = pinTypeText(afbPin.type(), afbPin.dataFormat());

						drawTextFunc(p, pinTypeRect, str, Qt::AlignCenter | Qt::TextDontClip);

						found = true;
						break;
					}
				}

				if (found == false)
				{
					// Draw type info from the pin.
					// This is the fix for packed logic, these items do not have AfbSignals, just pins.
					//
					QString str = E::valueToString<E::SignalType>(input.signalType());

					drawTextFunc(p, pinTypeRect, str, Qt::AlignCenter | Qt::TextDontClip);
				}
			}

			// --
			//
			pinY += pinHeight;

			if (pinY > itemRect.bottom())
			{
				break;
			}
		}

		// Draw output pins
		//
		const std::vector<AfbPin>& outputPins = outputs();
		pinY = intend + pinHeight / 2.0;

		for (const AfbPin& out : outputPins)
		{
			// Drawing pin
			//
			p->drawLine(QPointF(itemRect.right(), pinY),
						QPointF(itemRect.right() + pinWidth, pinY));

			// Draw pin text
			//
			QRectF pinTextRect(itemRect.right(),
							   pinY - pinHeight,
							   pinWidth,
							   pinHeight);

			drawTextFunc(p, pinTextRect, " " + out.caption(), Qt::AlignLeft | Qt::AlignBaseline | Qt::TextDontClip);

			// Draw pin type
			//
			const std::vector<Afb::AfbSignal>& afbOuts = afb.outputSignals();
			for (const Afb::AfbSignal& afbPin : afbOuts)
			{
				if (afbPin.caption() == out.caption())
				{
					QRectF pinTypeRect(itemRect.right() - typeWidth,
									   pinY - pinHeight / 2.0,
									   typeWidth,
									   pinHeight);

					QString str = pinTypeText(afbPin.type(), afbPin.dataFormat());

					drawTextFunc(p, pinTypeRect, str, Qt::AlignCenter | Qt::TextDontClip);
					break;
				}
			}

			// --
			//
			pinY += pinHeight;

			if (pinY > itemRect.bottom())
			{
				break;
			}
		}

		// Draw params
		//
		double paramY = itemRect.bottom();

		std::vector<Afb::AfbParam> sortedParams = params();
		sortedParams.erase(
			std::remove_if(sortedParams.begin(), sortedParams.end(), [](const Afb::AfbParam& p)
						   {
							   return p.user() == false;
						   }),
			sortedParams.end());

		if (sortedParams.empty() == false)
		{
			QRectF paramRect(intend, paramY, rect.width() - intend * 2, pinHeight);
			drawTextFunc(p, paramRect, "Params:", Qt::AlignLeft | Qt::AlignVCenter | Qt::TextDontClip);

			paramY += pinHeight;
		}

		std::sort(sortedParams.begin(), sortedParams.end(), [](const Afb::AfbParam& p1, const Afb::AfbParam& p2)
				  {
					  return p1.caption() < p2.caption();
				  });

		for (const Afb::AfbParam& param : sortedParams)
		{
			if (param.user() == false) // Actually it was already filtered in erase/remove_if
			{
				continue;
			}

			QRectF paramRect(intend, paramY, rect.width() - intend * 2, pinHeight);

			QString paramValue = getAfbParamValueText(param);

			QString str;
			if (param.isAnalog() == true)
			{
				QString typeStr = pinTypeText(param.type(), param.dataFormat());

				if (param.units().isEmpty() == false)
				{
					str = QString("%1, %2: %3 (%4 - %5), %6")
							  .arg(param.caption())
							  .arg(param.units())
							  .arg(paramValue)
							  .arg(param.lowLimit().toString())
							  .arg(param.highLimit().toString())
							  .arg(typeStr);
				}
				else
				{
					str = QString("%1: %2 (%3 - %4), %5")
							  .arg(param.caption())
							  .arg(paramValue)
							  .arg(param.lowLimit().toString())
							  .arg(param.highLimit().toString())
							  .arg(typeStr);
				}
			}
			else
			{
				assert(param.isDiscrete() == true);

				str = QString("%1: %2")
						  .arg(param.caption())
						  .arg(paramValue);
			}

			drawTextFunc(p, paramRect, str, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextDontClip);

			paramY += pinHeight;
		}

		// --
		//
		p->resetTransform();
		return;
	}

	// Serialization
	//
	bool SchemaItemAfb::SaveData(Proto::Envelope* message) const
	{
		bool result = FblItemRect::SaveData(message);
		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}

		// --
		//
		Proto::SchemaItemAfb* vifble = message->mutable_schemaitem()->mutable_afb();

		vifble->set_precision(m_precision);
		m_afbElement.saveToXml(vifble->mutable_afbelement());

		for (const QString& pls : m_packedLogicInputSignalIds)
		{
			vifble->add_packedlogicinputs(pls.toStdString());
		}

		return true;
	}

	bool SchemaItemAfb::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return false;
		}

		// --
		//
		bool result = FblItemRect::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_afb() == false)
		{
			assert(message.schemaitem().has_afb());
			return false;
		}

		const Proto::SchemaItemAfb& vifble = message.schemaitem().afb();

		m_precision = vifble.precision();

		if (vifble.has_deprecated_afbelement() == true)
		{
			QString errorMsg;
			bool ok = m_afbElement.deprecatedFormatLoad(vifble.deprecated_afbelement(), errorMsg);

			if (ok == false)
			{
				qDebug() << "SchemaItemAfb::LoadData: Parsing AFB element error: " << errorMsg;
				return false;
			}
		}
		else
		{
			assert(vifble.has_afbelement());

			QString errorMsg;
			bool ok = m_afbElement.loadFromXml(vifble.afbelement(), &errorMsg);

			if (ok == false)
			{
				qDebug() << "SchemaItemAfb::LoadData: Parsing AFB element error: " << errorMsg;
				return false;
			}
		}

		m_packedLogicInputSignalIds.clear();
		for (const std::string& pls : vifble.packedlogicinputs())
		{
			m_packedLogicInputSignalIds.push_back(QString::fromStdString(pls));
		}

		// Add afb properties to class meta object
		//
		addSpecificParamProperties();

		return true;
	}

	QString SchemaItemAfb::toolTipText(double dpiX, double dpiY, double devicePixelRatio) const
	{
		QImage image(QSize(static_cast<int>(3 * dpiX * devicePixelRatio),
						   static_cast<int>(3 * dpiY * devicePixelRatio)),
					 QImage::Format_RGB32); // size 3x3 inches

		image.fill(Qt::white);

		image.setDotsPerMeterX(static_cast<int>(1000.0 / 25.4 * dpiX * devicePixelRatio));
		image.setDotsPerMeterY(static_cast<int>(1000.0 / 25.4 * dpiX * devicePixelRatio));

		QPainter painter;
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setRenderHint(QPainter::TextAntialiasing, true);

		painter.begin(&image);
		drawAfbHelp(&painter, QRect(0, 0, image.width(), image.height()));
		painter.end();

		QByteArray data;
		QBuffer buffer(&data);
		image.save(&buffer, "PNG", 100);

		QString html = QString("<img src='data:image/png;base64, %0' height=\"%2\" width=\"%3\"/>")
						   .arg(QString(data.toBase64()))
						   .arg(image.size().height() / devicePixelRatio)
						   .arg(image.size().width() / devicePixelRatio);

		return html;
	}


	QString SchemaItemAfb::buildName() const
	{
		return QString("%1 %2")
			.arg(afbStrID())
			.arg(label());
	}

	bool SchemaItemAfb::setAfbParam(const QString& name, QVariant value, std::shared_ptr<Schema> schema, QString* errorMsg)
	{
		if (name.isEmpty() == true ||
			schema == nullptr ||
			errorMsg == nullptr)
		{
			assert(name.isEmpty() != true);
			assert(schema != nullptr);
			assert(errorMsg);
			return false;
		}

		if (value.canConvert<Afb::AfbParamValue>() == false)
		{
			Q_ASSERT(value.canConvert<Afb::AfbParamValue>());
			return false;
		}

		Afb::AfbParamValue newValue = value.value<Afb::AfbParamValue>();

		auto found = std::find_if(m_afbElement.params().begin(), m_afbElement.params().end(), [&name](const Afb::AfbParam& p)
								  {
									  return p.caption() == name;
								  });

		if (found == m_afbElement.params().end())
		{
			assert(found != m_afbElement.params().end());
			return false;
		}

		if (found->afbParamValue() != newValue)
		{
			qDebug() << tr("Param %1 was changed from %2 to %3").arg(name).arg(found->afbParamValue().toString()).arg(newValue.toString());

			found->setAfbParamValue(newValue);

			// Call script here
			//
			QString changedScript = found->changedScript();
			if (changedScript.isEmpty() == false)
			{
				bool result = executeScript(changedScript, m_afbElement, errorMsg);
				if (result == false)
				{
					return false;
				}
			}
		}

		return true;
	}

	bool SchemaItemAfb::setAfbParamByOpName(const QString& opName, const Afb::AfbParamValue& value)
	{
		if (opName.isEmpty() == true)
		{
			assert(opName.isEmpty() != true);
			return false;
		}

		//		if (value.canConvert<Afb::AfbParamValue>() == false)
		//		{
		//			Q_ASSERT(value.canConvert<Afb::AfbParamValue>());
		//			return false;
		//		}

		//		Afb::AfbParamValue newValue = value.value<Afb::AfbParamValue>();

		auto found = std::find_if(m_afbElement.params().begin(), m_afbElement.params().end(), [&opName](const Afb::AfbParam& p)
								  {
									  return p.opName() == opName;
								  });

		if (found == m_afbElement.params().end())
		{
			assert(found != m_afbElement.params().end());
			return false;
		}

		//		qDebug() << tr("Param %1 was changed from %2 to %3").
		//					arg(opName).
		//					arg(found->value().toString()).
		//					arg(value.toString());

		found->setAfbParamValue(value);

		return true;
	}

	QVariant SchemaItemAfb::getAfbParam(const QString& name)
	{
		for (Afb::AfbParam& p : m_afbElement.params())
		{
			if (p.caption() == name)
			{
				return p.afbParamValue().toVariant();
			}
		}
		return {};
	}

	Afb::AfbParam SchemaItemAfb::afbParam(const QString& name)
	{
		for (Afb::AfbParam& p : m_afbElement.params())
		{
			if (p.caption() == name)
			{
				return p;
			}
		}

		return {};
	}

	std::optional<bool> SchemaItemAfb::getAssignFlagsValue() const
	{
		for (const Afb::AfbParam& p : m_afbElement.params())
		{
			if (p.isDiscrete() == true && p.caption() == QStringLiteral("AssignFlags"))
			{
				return p.afbParamValue().value().toBool();
			}
		}

		return {};
	}

	bool SchemaItemAfb::setAfbElementParams(Afb::AfbElement* afbElement) const
	{
		if (afbElement == nullptr)
		{
			assert(afbElement);
			return false;
		}

		for (Afb::AfbParam& param : afbElement->params())
		{
			if (param.user() == false)
			{
				continue;
			}

			QVariant propValue = propertyValue(param.caption());

			if (propValue.isValid() == false)
			{
				// Was not found
				//
				assert(propValue.isValid() == true);
				return false;
			}

			param.afbParamValue().fromVariant(propValue);
		}

		return true;
	}

	bool SchemaItemAfb::updateAfbElement(const Afb::AfbElement& sourceAfb, QString* errorMessage)
	{
		if (errorMessage == nullptr)
		{
			assert(errorMessage);
			return false;
		}

		if (m_afbElement.strID() != sourceAfb.strID())
		{
			assert(m_afbElement.strID() == sourceAfb.strID());
			return false;
		}

		QString packedLogicId = m_afbElement.packedLogicId();

		// Update params, m_afbElement contains old parameters
		//
		std::vector<Afb::AfbParam> newParams = sourceAfb.params();
		const std::vector<Afb::AfbParam>& currentParams = m_afbElement.params();

		for (Afb::AfbParam& p : newParams)
		{
			if (p.user() == false)
			{
				continue;
			}

			auto foundExistingParam = std::find_if(currentParams.begin(), currentParams.end(), [&p](const Afb::AfbParam& mp)
												   {
													   return p.caption() == mp.caption(); // Don't use opIndex, it can be same (-1)
												   });

			if (foundExistingParam != currentParams.end())
			{
				// Try to set old value to the param
				//
				const Afb::AfbParam& currentParam = *foundExistingParam;

				if (p.afbParamValue().value().metaType() == currentParam.afbParamValue().value().metaType())
				{
					p.setAfbParamValue(currentParam.afbParamValue());
					// qDebug() << "Param: " << currentParam.caption() << ", value: " << p.afbParamValue().toString();
				}
			}
		}

		// Update description
		//
		m_afbElement = sourceAfb;

		std::swap(params(), newParams); // The prev assignment (m_afbElement = sourceAfb) just reset all params
										// Set them to the actual values

		// Restore packedLogicId, as it is stored in m_afbElement.
		//
		if (packedLogicId.isEmpty() == false)
		{
			setPackedLogicId(packedLogicId);
		}

		// Update in/out pins
		//
		removeAllInputs();
		removeAllOutputs();

		const std::vector<Afb::AfbSignal>& inputSignals = m_afbElement.inputSignals();
		for (const Afb::AfbSignal& s : inputSignals)
		{
			addInput(s);
		}

		const std::vector<Afb::AfbSignal>& outputSignals = m_afbElement.outputSignals();
		for (const Afb::AfbSignal& s : outputSignals)
		{
			addOutput(s);
		}

		// Run afterCreationScript, lets assume we create almost new items, as we deleted all inputs/outs and updated params
		//
		QString afterCreationScript = m_afbElement.afterCreationScript();

		if (afterCreationScript.isEmpty() == false)
		{
			executeScript(afterCreationScript, m_afbElement, errorMessage);
		}

		// Here is remove all props and add new from m_params
		//
		addSpecificParamProperties();

		return true;
	}

	double SchemaItemAfb::minimumPossibleHeightDocPt(double gridSize, int pinGridStep) const
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

		// 1 string for caption
		// N param count
		//
		const double smallFontSize = m_font.drawSize() * 0.85;

		double textLineCount = 1.2 * m_font.drawSize();       // First line for caption.
		for (const Afb::AfbParam& p : m_afbElement.params())
		{
			if (p.visible() == true)
			{
				textLineCount += smallFontSize;               // All params are small font.
			}
		}

		textLineCount += isPackedLogic() ? smallFontSize : 0; // +1 line for packed logic id.
		textLineCount += m_font.drawSize() / 8.0;             // Bottom margin

		double pinVertGap = VFrame30::snapToGrid(gridSize * static_cast<double>(pinGridStep), gridSize);

		double minPinHeight = VFrame30::snapToGrid(pinVertGap * static_cast<double>(pinCount), gridSize);
		double minTextHeight = VFrame30::snapToGrid(textLineCount, gridSize);

		if (minTextHeight < minPinHeight)
		{
			minTextHeight = minPinHeight;
		}

		if (minTextHeight < m_cachedGridSize * static_cast<double>(m_afbElement.minHeight()))
		{
			minTextHeight = m_cachedGridSize * static_cast<double>(m_afbElement.minHeight());
		}

		return minTextHeight;
	}

	double SchemaItemAfb::minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const
	{
		// Cache values
		//
		m_cachedGridSize = gridSize;
		m_cachedPinGridStep = pinGridStep;

		return m_cachedGridSize * m_afbElement.minWidth();
	}

	void SchemaItemAfb::addSpecificParamProperties()
	{
		// Clear all dynamic properties
		//
		removeSpecificProperties();

		// Set new Param Propereties
		//
		for (Afb::AfbParam& p : m_afbElement.params())
		{
			if (p.user() == false)
			{
				continue;
			}

			QVariant value = p.afbParamValue().toVariant();

			auto prop = this->addProperty(p.caption(), PropertyNames::parametersCategory, true, value);

			prop->setReadOnly(false);
			prop->setSpecific(true);
			if (p.isAnalog())
			{
				prop->setLimits(p.lowLimit(), p.highLimit());
			}
			prop->setPrecision(precision());
		}

		// Add or remove special property for packed logic.
		//
		if (m_afbElement.isPackedLogic() == true)
		{
			ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::packedLogicId, PropertyNames::parametersCategory, true, SchemaItemAfb::packedLogicId, SchemaItemAfb::setPackedLogicId)
				->setEssential(true);
		}
		else
		{
			removeProperty(PropertyNames::packedLogicId);
		}

		return;
	}

	bool SchemaItemAfb::executeScript(const QString& script, const Afb::AfbElement& afb, QString* errorMessage)
	{
		if (errorMessage == nullptr)
		{
			assert(errorMessage);
			return false;
		}

		if (script.isEmpty() == true)
		{
			return true;
		}

		QString exeScript = afb.libraryScript() + script;

		exeScript.replace(QString("&lt;"), QString("<"));
		exeScript.replace(QString("&gt;"), QString(">"));
		exeScript.replace(QString("&amp;"), QString("&"));

		QJSEngine jsEngine;

		Afb::AfbElement jsAfb = afb;

		QJSValue jsElement = jsEngine.newQObject(this);
		QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

		QJSValue jsAfbElement = jsEngine.newQObject(&jsAfb);
		QQmlEngine::setObjectOwnership(&jsAfb, QQmlEngine::CppOwnership);

		// Run script
		//
		QJSValue jsEval = jsEngine.evaluate(exeScript);
		if (jsEval.isError() == true)
		{
			*errorMessage += tr("Script evaluation failed in AFB element '%1': %2\n").arg(afb.caption()).arg(jsEval.toString());
			return false;
		}

		QJSValueList args;

		args << jsElement;
		args << jsAfbElement;

		QJSValue jsResult = jsEval.call(args);

		if (jsResult.isError() == true)
		{
			*errorMessage += tr("Script evaluation failed in AFB element '%1': %2\n").arg(afb.caption()).arg(jsResult.toString());
			return false;
		}

		return true;
	}

	double SchemaItemAfb::getParamDoubleValue(const QString& name)
	{
		QVariant result = getAfbParam(name);
		if (result.isNull() || result.isValid() == false)
		{
			return -1;
		}

		if (result.typeId() == qMetaTypeId<Afb::AfbParamValue>())
		{
			Afb::AfbParamValue v = result.value<Afb::AfbParamValue>();
			return v.value().toDouble();
		}

		return result.toDouble();
	}

	bool SchemaItemAfb::setParamDoubleValue(const QString& opName, double value)
	{
		if (opName.isEmpty() == true)
		{
			assert(opName.isEmpty() != true);
			return false;
		}

		auto found = std::find_if(m_afbElement.params().begin(), m_afbElement.params().end(), [&opName](const Afb::AfbParam& p)
								  {
									  return p.opName() == opName;
								  });

		if (found == m_afbElement.params().end())
		{
			assert(found != m_afbElement.params().end());
			return false;
		}

		Afb::AfbParamValue& v = found->afbParamValue();
		v.setValue(value);

		return true;
	}

	int SchemaItemAfb::getParamIntValue(const QString& name)
	{
		QVariant result = getAfbParam(name);
		if (result.isNull() || result.isValid() == false)
		{
			return -1;
		}

		if (result.metaType().id() == qMetaTypeId<Afb::AfbParamValue>())
		{
			Afb::AfbParamValue v = result.value<Afb::AfbParamValue>();
			return v.value().toInt();
		}

		return result.toInt();
	}

	bool SchemaItemAfb::setParamIntValue(const QString& opName, int value)
	{
		if (opName.isEmpty() == true)
		{
			assert(opName.isEmpty() != true);
			return false;
		}

		auto found = std::find_if(m_afbElement.params().begin(), m_afbElement.params().end(), [&opName](const Afb::AfbParam& p)
								  {
									  return p.opName() == opName;
								  });

		if (found == m_afbElement.params().end())
		{
			assert(found != m_afbElement.params().end());
			return false;
		}

		Afb::AfbParamValue& v = found->afbParamValue();
		v.setValue(value);

		return true;
	}

	bool SchemaItemAfb::getParamBoolValue(const QString& name)
	{
		QVariant result = getAfbParam(name);
		if (result.isNull() || result.isValid() == false)
		{
			return false;
		}

		if (result.metaType().id() == qMetaTypeId<Afb::AfbParamValue>())
		{
			Afb::AfbParamValue v = result.value<Afb::AfbParamValue>();
			return v.value().toBool();
		}

		return result.toBool();
	}

	bool SchemaItemAfb::setParamBoolValue(const QString& opName, bool value)
	{
		if (opName.isEmpty() == true)
		{
			assert(opName.isEmpty() != true);
			return false;
		}

		auto found = std::find_if(m_afbElement.params().begin(), m_afbElement.params().end(), [&opName](const Afb::AfbParam& p)
								  {
									  return p.opName() == opName;
								  });

		if (found == m_afbElement.params().end())
		{
			assert(found != m_afbElement.params().end());
			return false;
		}

		Afb::AfbParamValue& v = found->afbParamValue();
		v.setValue(value);

		return true;
	}

	bool SchemaItemAfb::setParamVisible(const QString& name, bool visible)
	{
		for (Afb::AfbParam& p : m_afbElement.params())
		{
			if (p.caption() == name)
			{
				p.setVisible(visible);
				return true;
			}
		}
		return false;
	}

	void SchemaItemAfb::addInputSignal(QString caption, int type, int opIndex, int /*size*/)
	{
		addInput(opIndex, static_cast<E::SignalType>(type), std::move(caption));
	}

	void SchemaItemAfb::addOutputSignal(QString caption, int type, int opIndex, int /*size*/)
	{
		addOutput(opIndex, static_cast<E::SignalType>(type), std::move(caption));
	}

	void SchemaItemAfb::removeInputSignals()
	{
		removeAllInputs();
	}

	void SchemaItemAfb::removeInputSignal(QString caption)
	{
		removeInput(caption);
	}

	void SchemaItemAfb::removeOutputSignals()
	{
		removeAllOutputs();
	}

	void SchemaItemAfb::removeOutputSignal(QString caption)
	{
		removeOutput(caption);
	}

	QString SchemaItemAfb::getAfbParamValueText(const Afb::AfbParam& param) const
	{
		QString paramValue;
		bool afbParamValueIsString = param.afbParamValue().reference().isEmpty() == false;

		QVariant a = afbParamValueIsString ?
						 param.afbParamValue().reference() :
						 param.afbParamValue().value();

		switch (param.type())
		{
		case E::SignalType::Analog:
			{
				char paramFormat = precision() > 5 ? 'g' : 'f';

				switch (param.dataFormat())
				{
				case E::DataFormat::UnsignedInt:
					paramValue = a.toString();
					break;

				case E::DataFormat::SignedInt:
					paramValue = a.toString();
					break;

				case E::DataFormat::Float:
					if (afbParamValueIsString == true)
					{
						paramValue = a.toString();
					}
					else
					{
						paramValue = param.afbParamValue().toString(paramFormat, precision());

						if (paramValue.contains(QChar('.')) == true &&
							paramValue.contains(QChar('e')) == false &&
							paramValue.size() > 2)
						{
							while (paramValue.endsWith('0'))
							{
								paramValue.chop(1);
							}

							if (paramValue.endsWith('.'))
							{
								paramValue.chop(1);
							}
						}
					}
					break;

				default:
					assert(false);
				}
			}
			break;

		case E::SignalType::Discrete:
			{
				paramValue = a.toString();
			}
			break;
		default:
			assert(false);
		}
		return paramValue;
	}

	// IMatsSchemaItemAssociations implementation.
	//
	QStringList SchemaItemAfb::associatedAppSignalIds() const
	{
		QStringList result; 

		if (isPackedLogic() == true)
		{
			result += packedLogicInputSignalIds();
		}

		return result;
	}

	QStringList SchemaItemAfb::associatedImpactAppSignalIds() const
	{
		return {};
	}

	QStringList SchemaItemAfb::associatedConnectionIds() const
	{
		return {};
	}

	QStringList SchemaItemAfb::associatedLoopbackIds() const
	{
		return {};
	}

	QStringList SchemaItemAfb::associatedSchemaItemLabels() const
	{
		return {};
	}

	// Properties
	//	
	const QString& SchemaItemAfb::afbStrID() const
	{
		return m_afbElement.strID();
	}

	Afb::AfbElement& SchemaItemAfb::afbElement()
	{
		return m_afbElement;
	}

	const Afb::AfbElement& SchemaItemAfb::afbElement() const
	{
		return m_afbElement;
	}

	std::vector<Afb::AfbParam>& SchemaItemAfb::params()
	{
		return m_afbElement.params();
	}

	const std::vector<Afb::AfbParam>& SchemaItemAfb::params() const
	{
		return m_afbElement.params();
	}

	int SchemaItemAfb::precision() const
	{
		return m_precision;
	}

	void SchemaItemAfb::setPrecision(int value)
	{
		if (value < 0)
		{
			value = 0;
		}

		if (value > 24)
		{
			value = 24;
		}

		m_precision = value;

		// Set new precsion to all dynamic properties
		//
		for (Afb::AfbParam& p : m_afbElement.params())
		{
			if (p.user() == false)
			{
				continue;
			}

			auto prop = this->propertyByCaption(p.caption());

			if (prop.get() != nullptr && prop->specific() == true)
			{
				prop->setPrecision(m_precision);
			}
		}

		return;
	}

	bool SchemaItemAfb::isPackedLogic() const
	{
		return m_afbElement.isPackedLogic();
	}

	const QString& SchemaItemAfb::packedLogicId() const
	{
		return m_afbElement.packedLogicId();
	}

	void SchemaItemAfb::setPackedLogicId(const QString& value)
	{
		m_afbElement.setPackedLogicId(value);
	}

	Afb::AfbElement::PackedLogicData SchemaItemAfb::packedLogic() const
	{
		return m_afbElement.packedLogic();
	}

	QStringList SchemaItemAfb::packedLogicInputSignalIds() const
	{
		return m_packedLogicInputSignalIds;
	}

	void SchemaItemAfb::setPackedLogicInputSignalIds(const QStringList& value)
	{
		m_packedLogicInputSignalIds = value;
	}

} // namespace VFrame30