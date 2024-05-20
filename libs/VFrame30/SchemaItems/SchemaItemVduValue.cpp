#include <VFrame30/SchemaItemVduValue.h>
#include <VFrame30/DrawParam.h>
#include <VFrame30/MacrosExpander.h>
#include <VFrame30/PropertyNames.h>
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
								 PropertyNames::appSignalId,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduValue::appSignalId,
								 SchemaItemVduValue::setAppSignalId);

		ADD_PROPERTY_GET_SET_CAT(int,
								 PropertyNames::precision,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduValue::precision,
								 SchemaItemVduValue::setPrecision);

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
								 SchemaItemVduValue::setFontSize)->
			setPrecision(0);

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

		valueMessage->set_appsignalid(m_appSignalId.toUtf8());
		m_font.SaveData(valueMessage->mutable_font());

		valueMessage->set_precision(m_precision);

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

		m_appSignalId = QString::fromUtf8(valueMessage.appsignalid().c_str());
		m_font.LoadData(valueMessage.font());

		m_precision = valueMessage.precision();

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

		QString text{"?"};

		painter->drawText(boundingRect, static_cast<int>(Qt::AlignHCenter) | static_cast<int>(Qt::AlignVCenter), text, nullptr);

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

	const QString& SchemaItemVduValue::appSignalId() const
	{
		return m_appSignalId;
	}

	void SchemaItemVduValue::setAppSignalId(const QString& value)
	{
		m_appSignalId = value;
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

} // namespace VFrame30
