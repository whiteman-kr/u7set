#include <VFrame30/SchemaItemActuator.h>

#include <VFrame30/DrawParam.h>
#include <VFrame30/MacrosExpander.h>
#include <VFrame30/PropertyNames.h>
#include <VFrame30/SchemaLayer.h>

#include <QBuffer>


namespace VFrame30
{
	SchemaItemActuator::SchemaItemActuator(void) :
		SchemaItemActuator(SchemaUnit::Inch)
	{
		// This constructor can be called during serialization of objects of this type.
		// After this call, you need to initialize everything that is done by serialization itself.
		//
	}

	SchemaItemActuator::SchemaItemActuator(SchemaUnit unit) :
		FblItemRect(unit)
	{
		ADD_PROPERTY_GETTER_SETTER(QString, PropertyNames::caption, true, SchemaItemActuator::caption, SchemaItemActuator::setCaption)
			->setCategory(PropertyNames::functionalCategory)
			.setEssential(true);

		ADD_PROPERTY_GETTER_SETTER(QString,
								   PropertyNames::acmEquipmentId,
								   true,
								   SchemaItemActuator::acmEquipmentId,
								   SchemaItemActuator::setAcmEquipmentId)
			->setCategory(PropertyNames::functionalCategory)
			.setEssential(true);

		ADD_PROPERTY_GETTER(QString, PropertyNames::ActuatorTypeId, true, SchemaItemActuator::actuatorTypeId)
			->setCategory(PropertyNames::functionalCategory)
			.setEssential(true);

		ADD_PROPERTY_GETTER(QString, PropertyNames::ActuatorCaption, true, SchemaItemActuator::actuatorCaption)
			->setCategory(PropertyNames::functionalCategory);

		ADD_PROPERTY_GETTER(QString, PropertyNames::description, true, SchemaItemActuator::actuatorDescription)
			->setCategory(PropertyNames::functionalCategory);

		ADD_PROPERTY_GETTER(int, PropertyNames::actuatorVersion, true, SchemaItemActuator::actuatorHeaderVersion)
			->setCategory(PropertyNames::functionalCategory);

		return;
	}

	SchemaItemActuator::SchemaItemActuator(SchemaUnit unit, const ActuatorHeader& actuatorHeader) :
		SchemaItemActuator(unit)
	{
		// Add UfbSchemaID as tag, it's useful for scripting
		//
		addTag("actuator");
		addTag(actuatorHeader.actuatorTypeId());

		// Create input output signals in VFrame30::FblItem
		//
		updateElement(actuatorHeader);

		return;
	}

	SchemaItemActuator::~SchemaItemActuator(void) {}

	void SchemaItemActuator::draw(CDrawParam* drawParam) const
	{
		QPainter* p = drawParam->painter();
		p->save();

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
		QString text;
		if (m_caption.trimmed().isEmpty() == false)
		{
			QString normalized = m_caption;
			normalized.replace(QStringLiteral("\\r\\n"), QStringLiteral("\n"));
			normalized.replace(QStringLiteral("\\n"), QStringLiteral("\n"));

			text = MacrosExpander::parse(normalized, context().get(), &drawParam->session(), this);
		}
		else
		{
			text = m_actuatorHeader.caption();
		}

		p->setPen(textColor());
		DrawHelper::drawText(p, m_font, itemUnit(), text, r, Qt::AlignHCenter | Qt::AlignTop);

		p->restore();
		return;
	}

	void SchemaItemActuator::drawHighlight(CDrawParam* drawParam) const
	{
		bool highlight = drawParam->highlightIds().contains(label());

		if (highlight == true)
		{
			QRectF highlightRect = boundingRectInDocPt(drawParam);
			drawHighlightRect(drawParam, highlightRect);
		}

		return;
	}

	void SchemaItemActuator::drawActuatorItemHelp(QPainter* painter, const QRect& drawRect) const
	{
		if (painter == nullptr || drawRect.isEmpty() == true)
		{
			assert(painter);
			return;
		}

		auto drawTextFunc = [](QPainter* p, const QRectF rect, QString text, int flags)
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

		auto pinTypeText = [](E::SignalType type) -> QString
		{
			QString result = "UNK";

			switch (type)
			{
			case E::SignalType::Analog:
				result = "ANALOG";
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

		p->drawLine(QPointF(itemRect.left() + typeWidth, itemRect.top()), QPointF(itemRect.left() + typeWidth, itemRect.bottom()));

		p->drawLine(QPointF(itemRect.right() - typeWidth, itemRect.top()), QPointF(itemRect.right() - typeWidth, itemRect.bottom()));

		// Draw caption
		//
		QRectF captionRect(itemRect.left(), 0, itemRect.width(), intend);

		QString caption = m_caption.trimmed().isEmpty() ? m_actuatorHeader.caption() : m_caption;
		drawTextFunc(p, captionRect, caption, Qt::AlignCenter | Qt::TextDontClip);

		// Draw input pins
		//
		double pinY = intend + pinHeight / 2.0;

		for (const AfbPin& input : inputs())
		{
			// Drawing pin
			//
			p->drawLine(QPointF(intend, pinY), QPointF(itemRect.left(), pinY));

			// Draw pin text
			//
			QRectF pinTextRect(intend, pinY - pinHeight, pinWidth, pinHeight);

			drawTextFunc(p, pinTextRect, input.caption() + " ", Qt::AlignRight | Qt::AlignBaseline | Qt::TextDontClip);

			// Draw pin type
			//
			QRectF pinTypeRect(itemRect.left(), pinY - pinHeight / 2.0, typeWidth, pinHeight);

			QString pinTypeStr = pinTypeText(input.signalType());
			drawTextFunc(p, pinTypeRect, pinTypeStr, Qt::AlignCenter | Qt::TextDontClip);

			pinY += pinHeight;
			if (pinY > itemRect.bottom())
			{
				break;
			}
		}

		// Draw output pins
		//
		pinY = intend + pinHeight / 2.0;

		for (const AfbPin& out : outputs())
		{
			// Drawing pin
			//
			p->drawLine(QPointF(itemRect.right(), pinY), QPointF(itemRect.right() + pinWidth, pinY));

			// Draw pin text
			//
			QRectF pinTextRect(itemRect.right(), pinY - pinHeight, pinWidth, pinHeight);

			drawTextFunc(p, pinTextRect, " " + out.caption(), Qt::AlignLeft | Qt::AlignBaseline | Qt::TextDontClip);

			QRectF pinTypeRect(itemRect.right() - typeWidth, pinY - pinHeight / 2.0, typeWidth, pinHeight);

			QString pinTypeStr = pinTypeText(out.signalType());
			drawTextFunc(p, pinTypeRect, pinTypeStr, Qt::AlignCenter | Qt::TextDontClip);

			pinY += pinHeight;
			if (pinY > itemRect.bottom())
			{
				break;
			}
		}

		// --
		//
		p->resetTransform();
		return;
	}

	// Serialization
	//
	bool SchemaItemActuator::SaveData(Proto::Envelope* message) const
	{
		bool result = FblItemRect::SaveData(message);
		if (result == false || message->HasExtension(Proto::schemaitem) == false)
		{
			assert(result);
			assert(message->HasExtension(Proto::schemaitem));
			return false;
		}

		Proto::SchemaItemActuator* am = message->MutableExtension(Proto::schemaitem)->mutable_actuator();

		am->set_acmequipmentid(m_acmEquipmentId.toStdString());
		am->set_caption(m_caption.toStdString());

		m_actuatorHeader.SaveData(am->mutable_actuatorheader());

		return true;
	}

	bool SchemaItemActuator::LoadData(const Proto::Envelope& message)
	{
		if (message.HasExtension(Proto::schemaitem) == false)
		{
			assert(message.HasExtension(Proto::schemaitem));
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
		if (message.GetExtension(Proto::schemaitem).has_actuator() == false)
		{
			assert(message.GetExtension(Proto::schemaitem).has_actuator());
			return false;
		}

		const Proto::SchemaItemActuator& am = message.GetExtension(Proto::schemaitem).actuator();

		m_acmEquipmentId = QString::fromStdString(am.acmequipmentid());
		m_caption = QString::fromStdString(am.caption());

		m_actuatorHeader.LoadData(am.actuatorheader());

		return true;
	}

	QString SchemaItemActuator::toolTipText(double dpiX, double dpiY, double devicePixelRatio) const
	{
		QImage image(QSize(static_cast<int>(3 * dpiX * devicePixelRatio), static_cast<int>(3 * dpiY * devicePixelRatio)),
					 QImage::Format_RGB32); // size 3x3 inches

		image.fill(Qt::white);

		image.setDotsPerMeterX(static_cast<int>(1000.0 / 25.4 * dpiX * devicePixelRatio));
		image.setDotsPerMeterY(static_cast<int>(1000.0 / 25.4 * dpiY * devicePixelRatio));

		QPainter painter;
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.setRenderHint(QPainter::TextAntialiasing, true);

		painter.begin(&image);
		drawActuatorItemHelp(&painter, QRect(0, 0, image.width(), image.height()));
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

	QString SchemaItemActuator::buildName() const
	{
		return QString("%1 %2").arg(m_actuatorHeader.caption()).arg(label());
	}

	bool SchemaItemActuator::updateElement(const ActuatorHeader& actuatorHeader)
	{
		// Update actuator header
		//
		setActuatorHeader(actuatorHeader);

		// Add pins
		//
		removeAllInputs();
		removeAllOutputs();

		for (const auto& input : actuatorHeader.inputs())
		{
			addInput(-1, input->signalType(), input->signalId());
		}

		for (const auto& output : actuatorHeader.outputs())
		{
			addOutput(-1, output->signalType(), output->signalId());
		}

		adjustHeight();

		return true;
	}

	// IMatsSchemaItemAssociations implementation.
	//
	QStringList SchemaItemActuator::associatedAppSignalIds() const
	{
		return {};
	}

	QStringList SchemaItemActuator::associatedImpactAppSignalIds() const
	{
		return {};
	}

	QStringList SchemaItemActuator::associatedConnectionIds() const
	{
		return {};
	}

	QStringList SchemaItemActuator::associatedLoopbackIds() const
	{
		return {};
	}

	QStringList SchemaItemActuator::associatedSchemaItemLabels() const
	{
		return {};
	}

	QString SchemaItemActuator::acmEquipmentId() const
	{
		return m_acmEquipmentId;
	}

	void SchemaItemActuator::setAcmEquipmentId(const QString& value)
	{
		m_acmEquipmentId = value;
	}

	QString SchemaItemActuator::caption() const
	{
		return m_caption;
	}

	void SchemaItemActuator::setCaption(const QString& value)
	{
		m_caption = value;
	}

	const VFrame30::ActuatorHeader& SchemaItemActuator::actuatorHeader() const
	{
		return m_actuatorHeader;
	}

	void SchemaItemActuator::setActuatorHeader(const VFrame30::ActuatorHeader& value)
	{
		Proto::ActuatorHeader message;
		value.SaveData(&message);
		m_actuatorHeader.LoadData(message);
	}

	QString SchemaItemActuator::actuatorTypeId() const
	{
		return m_actuatorHeader.actuatorTypeId();
	}

	QString SchemaItemActuator::actuatorCaption() const
	{
		return m_actuatorHeader.caption();
	}

	QString SchemaItemActuator::actuatorDescription() const
	{
		return m_actuatorHeader.description();
	}

	int SchemaItemActuator::actuatorHeaderVersion() const
	{
		return m_actuatorHeader.version();
	}

} // namespace VFrame30
