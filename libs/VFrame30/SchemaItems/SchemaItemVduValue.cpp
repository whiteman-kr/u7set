#include <VFrame30/DrawParam.h>
#include <VFrame30/MacrosExpander.h>
#include <VFrame30/PropertyNames.h>
#include <VFrame30/SchemaItemVduValue.h>
#include <VFrame30/SchemaView.h>

namespace VFrame30
{
	SchemaItemVduValue::SchemaItemVduValue(void) :
		SchemaItemVduValue(SchemaUnit::Display)
	{
	}

	SchemaItemVduValue::SchemaItemVduValue(SchemaUnit units) :
		PosRectImpl{}
	{
		assert(units == SchemaUnit::Display);

		ADD_PROPERTY_GET_SET_CAT(int,
								 PropertyNames::lineWeight,
								 PropertyNames::appearanceCategory,
								 true,
								 SchemaItemVduValue::weight,
								 SchemaItemVduValue::setWeight);

		ADD_PROPERTY_GET_SET_CAT(bool,
								 PropertyNames::drawRect,
								 PropertyNames::appearanceCategory,
								 true,
								 SchemaItemVduValue::drawRect,
								 SchemaItemVduValue::setDrawRect);

		ADD_PROPERTY_GET_SET_CAT(QColor,
								 PropertyNames::lineColor,
								 PropertyNames::appearanceCategory,
								 true,
								 SchemaItemVduValue::lineColor,
								 SchemaItemVduValue::setLineColor);

		ADD_PROPERTY_GET_SET_CAT(QColor,
								 PropertyNames::fillColor,
								 PropertyNames::appearanceCategory,
								 true,
								 SchemaItemVduValue::fillColor,
								 SchemaItemVduValue::setFillColor);

		// Text Category Properties
		//
		ADD_PROPERTY_GET_SET_CAT(QColor,
								 PropertyNames::textColor,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduValue::textColor,
								 SchemaItemVduValue::setTextColor);

		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::text,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduValue::text,
								 SchemaItemVduValue::setText)
			->setDescription(PropertyNames::textVduItemValueDescription)
			.setEssential(true);

		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::appSignalIDs,
								 PropertyNames::functionalCategory,
								 true,
								 SchemaItemVduValue::appSignalIdsString,
								 SchemaItemVduValue::setAppSignalIdsString)
			->setEssential(true);

		ADD_PROPERTY_GET_SET_CAT(int,
								 PropertyNames::precision,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduValue::precision,
								 SchemaItemVduValue::setPrecision);

		ADD_PROPERTY_GET_SET_CAT(E::HorzAlign,
								 PropertyNames::alignHorz,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduValue::horzAlign,
								 SchemaItemVduValue::setHorzAlign);

		ADD_PROPERTY_GET_SET_CAT(E::VertAlign,
								 PropertyNames::alignVert,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduValue::vertAlign,
								 SchemaItemVduValue::setVertAlign);

		// Font
		//
		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::fontName,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduValue::getFontName,
								 SchemaItemVduValue::setFontName);

		ADD_PROPERTY_GET_SET_CAT(double,
								 PropertyNames::fontSize,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduValue::getFontSize,
								 SchemaItemVduValue::setFontSize)
			->setPrecision(0);

		ADD_PROPERTY_GET_SET_CAT(bool,
								 PropertyNames::fontBold,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduValue::getFontBold,
								 SchemaItemVduValue::setFontBold);

		ADD_PROPERTY_GET_SET_CAT(bool,
								 PropertyNames::fontItalic,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduValue::getFontItalic,
								 SchemaItemVduValue::setFontItalic);

		m_font.setName(QStringLiteral("Arial"));
		Q_ASSERT(units == SchemaUnit::Display);

		m_font.setSize(12.0, units);

		m_text = "%v";

		// --
		//
		m_static = false;
		setItemUnit(units);

		return;
	}

	// Serialization
	//
	bool SchemaItemVduValue::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectImpl::SaveData(message);
		if (result == false || message->HasExtension(Proto::schemaitem) == false)
		{
			assert(result);
			assert(message->HasExtension(Proto::schemaitem));
			return false;
		}

		// --
		//
		auto valueMessage = message->MutableExtension(Proto::schemaitem)->mutable_vduvalue();

		valueMessage->set_weight(m_weight);
		valueMessage->set_drawrect(m_drawRect);

		valueMessage->set_linecolor(m_lineColor.rgba());
		valueMessage->set_fillcolor(m_fillColor.rgba());
		valueMessage->set_textcolor(m_textColor.rgba());

		valueMessage->set_text(m_text.toUtf8());

		valueMessage->set_appsignalids(appSignalIdsString().toUtf8());
		m_font.SaveData(valueMessage->mutable_font());

		valueMessage->set_precision(m_precision);

		valueMessage->set_horzalign(static_cast<int32_t>(m_horzAlign));
		valueMessage->set_vertalign(static_cast<int32_t>(m_vertAlign));

		return true;
	}

	bool SchemaItemVduValue::LoadData(const Proto::Envelope& message)
	{
		if (message.HasExtension(Proto::schemaitem) == false)
		{
			assert(message.HasExtension(Proto::schemaitem));
			return false;
		}

		// --
		//
		bool result = PosRectImpl::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		const auto& schemaItemMessage = message.GetExtension(Proto::schemaitem);

		if (schemaItemMessage.has_vduvalue() == false)
		{
			assert(schemaItemMessage.has_vduvalue());
			return false;
		}

		const auto& valueMessage = schemaItemMessage.vduvalue();

		m_weight = valueMessage.weight();
		m_drawRect = valueMessage.drawrect();

		m_lineColor = QColor::fromRgba(valueMessage.linecolor());
		m_fillColor = QColor::fromRgba(valueMessage.fillcolor());
		m_textColor = QColor::fromRgba(valueMessage.textcolor());

		m_text = QString::fromUtf8(valueMessage.text().c_str());

		setAppSignalIdsString(QString::fromUtf8(valueMessage.appsignalids().c_str()));
		m_font.LoadData(valueMessage.font());

		m_precision = valueMessage.precision();

		m_horzAlign = valueMessage.has_horzalign() ? static_cast<E::HorzAlign>(valueMessage.horzalign()) : E::HorzAlign::AlignHCenter;
		m_vertAlign = valueMessage.has_vertalign() ? static_cast<E::VertAlign>(valueMessage.vertalign()) : E::VertAlign::AlignVCenter;

		return true;
	}

	// Drawing Functions
	//
	void SchemaItemVduValue::draw(CDrawParam* drawParam) const
	{
		QPainter* painter = drawParam->painter();
		QRectF boundingRect = boundingRectInDocPt(drawParam);

		// Draw rect
		//
		QBrush fillBrush{m_fillColor, Qt::SolidPattern};
		painter->setBrush(fillBrush);

		QPen rectPen{Qt::NoPen};

		if (drawRect() == true)
		{
			rectPen = QPen{m_lineColor};
			rectPen.setWidth(m_weight);
		}

		painter->setPen(rectPen);

		painter->drawRect(boundingRect);

		// Drawing Text
		//
		QFont font = m_font.qfont(itemUnit(), 0); // Dpi does not matter for SchemaUnit::Display.
		font.setStyleStrategy(QFont::PreferAntialias);

		painter->setFont(font);
		painter->setPen(m_textColor);

		QString text = parseText(m_text, context()->appSignalController());

		int alignFlags = static_cast<int>(m_horzAlign) | static_cast<int>(m_vertAlign);

		painter->drawText(boundingRect, alignFlags, text, nullptr);

		return;
	}

	double SchemaItemVduValue::minimumPossibleHeightDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	double SchemaItemVduValue::minimumPossibleWidthDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	QString SchemaItemVduValue::parseText(QStringView text, const VFrame30::AppSignalController* appSignalController) const
	{
		// %% - Percent
		// %i - CustomAppSignalID
		// %c - Signal caption
		// %v - Signal value
		// %V - Signal value + unit
		// %s - +/- signal value
		// %S - +/- signal value + unit
		// %u - unit
		// %e - Value in exponential form (1.0e-11)
		// %E - Value in exponential form (1.0E-11)
		// %x - Value in HEX (only for integer signal type). m_precision plays the role of the number of zeros to add (00009abc).
		// %X - Value in HEX (only for integer signal type). m_precision plays the role of the number of zeros to add (00009ABC).

		QString appSignalId = appSignalIds().isEmpty() ? QStringLiteral("#NOT_FOUND") : appSignalIds().first();

		AppSignalParam signalParam;

		if (appSignalController != nullptr)
		{
			signalParam = appSignalController->signalParam(appSignalId)
							  .or_else(
								  []()
								  {
									  std::optional asp = AppSignalParam{};
									  asp->setAppSignalId("#NOT_FOUND");
									  asp->setCustomSignalId("NOT_FOUND");
									  asp->setCaption("NOT_FOUND");
									  return asp;
								  })
							  .value();
		}

		bool treatAsFloat = E::SignalType::Analog && signalParam.analogSignalFormat() == E::AnalogAppSignalFormat::Float32;

		QString result = text.toString();

		qsizetype index = 0;
		for (;;)
		{
			index = result.indexOf('%', index); // index is increased in the loop body
			if (index == -1 || index + 1 >= result.size())
			{
				break;
			}

			QChar nextChar = result.at(index + 1);
			QString replaceText;

			switch (nextChar.unicode())
			{
			case '%':
				replaceText = QStringLiteral("%");
				break;

			case 'i':
				replaceText = signalParam.customSignalId();
				break;

			case 'c':
				replaceText = signalParam.caption();
				break;

			case 'v':
				replaceText = treatAsFloat ? QString::number(123.123456780123456789, 'f', precision()) : QString::number(123, 'f', 0);
				break;

			case 'V':
				{
					QString valueText =
						treatAsFloat ? QString::number(123.123456780123456789, 'f', precision()) : QString::number(123, 'f', 0);
					replaceText = QString("%1 %2").arg(valueText).arg(signalParam.units());
				}
				break;

			case 's':
				replaceText = treatAsFloat ? QString::number(123.123456780123456789, 'f', precision()) : QString::number(123, 'f', 0);
				break;

			case 'S':
				{
					QString valueText =
						treatAsFloat ? QString::number(123.123456780123456789, 'f', precision()) : QString::number(123, 'f', 0);

					replaceText = QString("+%1 %2").arg(valueText).arg(signalParam.units());
				}
				break;

			case 'u':
				replaceText = signalParam.units();
				break;

			case 'e':
				replaceText = QStringLiteral("1.0e-11");
				break;

			case 'E':
				replaceText = QStringLiteral("1.0E-11");
				break;

			case 'x':
				{
					QString valueText = treatAsFloat ? QString::number(0) : QString::number(123, 16);
					replaceText = valueText.rightJustified(precision(), '0').toLower();
				}
				break;

			case 'X':
				{
					QString valueText = treatAsFloat ? QString::number(0) : QString::number(123, 16);
					replaceText = valueText.rightJustified(precision(), '0').toUpper();
				}
				break;

			default:
				replaceText = QStringLiteral("???");
				break;
			}

			result.replace(index, 2, replaceText);
			index += replaceText.size();
		}

		return result;
	}

	// IMatsSchemaItemAssociations implementation.
	//
	QStringList SchemaItemVduValue::associatedDiagObjectIds() const
	{
		return {};
	};

	QStringList SchemaItemVduValue::associatedAppSignalIds() const
	{
		return appSignalIds();
	}

	QStringList SchemaItemVduValue::associatedImpactAppSignalIds() const
	{
		return {};
	}

	QStringList SchemaItemVduValue::associatedConnectionIds() const
	{
		return {};
	}

	QStringList SchemaItemVduValue::associatedLoopbackIds() const
	{
		return {};
	}

	QStringList SchemaItemVduValue::associatedSchemaItemLabels() const
	{
		return {};
	}

	int SchemaItemVduValue::weight() const
	{
		return m_weight;
	}

	void SchemaItemVduValue::setWeight(int weight)
	{
		m_weight = std::clamp(weight, 1, 16);
	}

	bool SchemaItemVduValue::drawRect() const
	{
		return m_drawRect;
	}

	void SchemaItemVduValue::setDrawRect(bool value)
	{
		m_drawRect = value;
	}

	QColor SchemaItemVduValue::lineColor() const
	{
		return m_lineColor;
	}

	void SchemaItemVduValue::setLineColor(QColor color)
	{
		m_lineColor = color;
	}

	QColor SchemaItemVduValue::fillColor() const
	{
		return m_fillColor;
	}

	void SchemaItemVduValue::setFillColor(QColor color)
	{
		m_fillColor = color;
	}

	QColor SchemaItemVduValue::textColor() const
	{
		return m_textColor;
	}

	void SchemaItemVduValue::setTextColor(QColor color)
	{
		m_textColor = color;
	}

	const QString& SchemaItemVduValue::text() const
	{
		return m_text;
	}

	void SchemaItemVduValue::setText(const QString& value)
	{
		m_text = value.left(127);
	}

	QString SchemaItemVduValue::appSignalIdsString() const
	{
		return m_appSignalIds.join(QChar::LineFeed);
	}

	void SchemaItemVduValue::setAppSignalIdsString(const QString& value)
	{
		thread_local const auto re = QRegularExpression("\\s+");
		m_appSignalIds = value.split(re, Qt::SkipEmptyParts);
	}

	QStringList SchemaItemVduValue::appSignalIds() const
	{
		return m_appSignalIds;
	}

	void SchemaItemVduValue::setAppSignalIds(const QStringList& value)
	{
		m_appSignalIds = value;
	}

	int SchemaItemVduValue::precision() const
	{
		return m_precision;
	}

	void SchemaItemVduValue::setPrecision(int value)
	{
		m_precision = std::clamp(value, 0, 32);
	}

	IMPLEMENT_FONT_PROPERTIES(SchemaItemVduValue, Font, m_font);

	E::HorzAlign SchemaItemVduValue::horzAlign() const
	{
		return m_horzAlign;
	}

	void SchemaItemVduValue::setHorzAlign(E::HorzAlign align)
	{
		m_horzAlign = align;
	}

	E::VertAlign SchemaItemVduValue::vertAlign() const
	{
		return m_vertAlign;
	}

	void SchemaItemVduValue::setVertAlign(E::VertAlign align)
	{
		m_vertAlign = align;
	}
} // namespace VFrame30
