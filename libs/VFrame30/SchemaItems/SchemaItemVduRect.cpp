#include <VFrame30/SchemaItemVduRect.h>
#include <VFrame30/DrawParam.h>
#include <VFrame30/MacrosExpander.h>
#include <VFrame30/PropertyNames.h>
#include <VFrame30/SchemaView.h>

namespace VFrame30
{
	SchemaItemVduRect::SchemaItemVduRect(void) :
		SchemaItemVduRect(SchemaUnit::Display)
	{
	}

	SchemaItemVduRect::SchemaItemVduRect(SchemaUnit units) :
		PosRectImpl{}
	{
		assert(units == SchemaUnit::Display);

		ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::lineWeight, PropertyNames::appearanceCategory, true, SchemaItemVduRect::weight, SchemaItemVduRect::setWeight);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::fill, PropertyNames::appearanceCategory, true, SchemaItemVduRect::fill, SchemaItemVduRect::setFill);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::drawRect, PropertyNames::appearanceCategory, true, SchemaItemVduRect::drawRect, SchemaItemVduRect::setDrawRect);

		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::lineColor, PropertyNames::appearanceCategory, true, SchemaItemVduRect::lineColor, SchemaItemVduRect::setLineColor);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::fillColor, PropertyNames::appearanceCategory, true, SchemaItemVduRect::fillColor, SchemaItemVduRect::setFillColor);

		// Text Category Properties
		//
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::textColor, PropertyNames::textCategory, true, SchemaItemVduRect::textColor, SchemaItemVduRect::setTextColor);

		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::text, PropertyNames::textCategory, true, SchemaItemVduRect::text, SchemaItemVduRect::setText);

		ADD_PROPERTY_GET_SET_CAT(E::HorzAlign, PropertyNames::alignHorz, PropertyNames::textCategory, true, SchemaItemVduRect::horzAlign, SchemaItemVduRect::setHorzAlign);
		ADD_PROPERTY_GET_SET_CAT(E::VertAlign, PropertyNames::alignVert, PropertyNames::textCategory, true, SchemaItemVduRect::vertAlign, SchemaItemVduRect::setVertAlign);

		// Font
		//
		ADD_PROPERTY_GET_SET_CAT(QString,
								 PropertyNames::fontName,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduRect::getFontName,
								 SchemaItemVduRect::setFontName);

		ADD_PROPERTY_GET_SET_CAT(double,
								 PropertyNames::fontSize,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduRect::getFontSize,
								 SchemaItemVduRect::setFontSize)->
			setPrecision(0);

		ADD_PROPERTY_GET_SET_CAT(bool,
								 PropertyNames::fontBold,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduRect::getFontBold,
								 SchemaItemVduRect::setFontBold);

		ADD_PROPERTY_GET_SET_CAT(bool,
								 PropertyNames::fontItalic,
								 PropertyNames::textCategory,
								 true,
								 SchemaItemVduRect::getFontItalic,
								 SchemaItemVduRect::setFontItalic);

		m_font.setName(QStringLiteral("Arial"));
		Q_ASSERT(units == SchemaUnit::Display);

		m_font.setSize(12.0, units);

		// --
		//
		m_static = true;
		setItemUnit(units);

		return;
	}

	// Serialization
	//
	bool SchemaItemVduRect::SaveData(Proto::Envelope* message) const
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
		auto rectMessage = message->MutableExtension(Proto::schemaitem)->mutable_vdurect();

		rectMessage->set_weight(m_weight);
		rectMessage->set_fill(m_fill);
		rectMessage->set_drawrect(m_drawRect);

		rectMessage->set_linecolor(m_lineColor.rgba());
		rectMessage->set_fillcolor(m_fillColor.rgba());
		rectMessage->set_textcolor(m_textColor.rgba());

		rectMessage->set_text(m_text.toUtf8());
		m_font.SaveData(rectMessage->mutable_font());

		rectMessage->set_horzalign(static_cast<int32_t>(m_horzAlign));
		rectMessage->set_vertalign(static_cast<int32_t>(m_vertAlign));

		return true;
	}

	bool SchemaItemVduRect::LoadData(const Proto::Envelope& message)
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

		if (schemaItemMessage.has_vdurect() == false)
		{
			assert(schemaItemMessage.has_vdurect());
			return false;
		}

		const auto& rectMessage = schemaItemMessage.vdurect();

		m_weight = rectMessage.weight();
		m_fill = rectMessage.fill();
		m_drawRect = rectMessage.drawrect();

		m_lineColor = QColor::fromRgba(rectMessage.linecolor());
		m_fillColor = QColor::fromRgba(rectMessage.fillcolor());
		m_textColor = QColor::fromRgba(rectMessage.textcolor());

		m_text = QString::fromUtf8(rectMessage.text().c_str());
		m_font.LoadData(rectMessage.font());

		m_horzAlign = static_cast<E::HorzAlign>(rectMessage.horzalign());
		m_vertAlign = static_cast<E::VertAlign>(rectMessage.vertalign());

		return true;
	}

	// Drawing Functions
	//
	void SchemaItemVduRect::draw(CDrawParam* drawParam) const
	{
		QPainter* painter = drawParam->painter();
		QRectF boundingRect = boundingRectInDocPt(drawParam);

		// Draw rect
		//
		QBrush fillBrush = fill() ? QBrush{m_fillColor, Qt::SolidPattern} : QBrush{Qt::NoBrush};
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

		painter->drawText(boundingRect, static_cast<int>(m_horzAlign) | static_cast<int>(m_vertAlign), m_text, nullptr);

		return;
	}

	double SchemaItemVduRect::minimumPossibleHeightDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	double SchemaItemVduRect::minimumPossibleWidthDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	// void SchemaItemVduRect::drawPlainText(QPainter& painter, QRectF rect, QString text) const
	//{
	//	int flags = static_cast<int>(horzAlign()) |
	//				static_cast<int>(vertAlign()) |
	//				static_cast<int>((wordWrap() ? Qt::TextWordWrap : 0));

	//	painter.setPen(m_textColor);

	//	DrawHelper::drawText(&painter,
	//						 m_font,
	//						 itemUnit(),
	//						 text,
	//						 rect,
	//						 flags);

	//	return;
	//}

	void SchemaItemVduRect::accept(VduItemVisitor& visitor) const
	{
		return visitor.visit(*this);
	}

	int SchemaItemVduRect::weight() const
	{
		return m_weight;
	}

	void SchemaItemVduRect::setWeight(int weight)
	{
		m_weight = std::clamp(weight, 1, 16);
	}

	bool SchemaItemVduRect::fill() const
	{
		return m_fill;
	}

	void SchemaItemVduRect::setFill(bool fill)
	{
		m_fill = fill;
	}

	bool SchemaItemVduRect::drawRect() const
	{
		return m_drawRect;
	}

	void SchemaItemVduRect::setDrawRect(bool value)
	{
		m_drawRect = value;
	}

	QColor SchemaItemVduRect::lineColor() const
	{
		return m_lineColor;
	}

	void SchemaItemVduRect::setLineColor(QColor color)
	{
		m_lineColor = color;
	}

	QColor SchemaItemVduRect::fillColor() const
	{
		return m_fillColor;
	}

	void SchemaItemVduRect::setFillColor(QColor color)
	{
		m_fillColor = color;
	}

	QColor SchemaItemVduRect::textColor() const
	{
		return m_textColor;
	}

	void SchemaItemVduRect::setTextColor(QColor color)
	{
		m_textColor = color;
	}

	IMPLEMENT_FONT_PROPERTIES(SchemaItemVduRect, Font, m_font);

	const QString& SchemaItemVduRect::text() const
	{
		return m_text;
	}

	void SchemaItemVduRect::setText(const QString& value)
	{
		m_text = value;
	}

	E::HorzAlign SchemaItemVduRect::horzAlign() const
	{
		return m_horzAlign;
	}

	void SchemaItemVduRect::setHorzAlign(E::HorzAlign align)
	{
		m_horzAlign = align;
	}

	E::VertAlign SchemaItemVduRect::vertAlign() const
	{
		return m_vertAlign;
	}

	void SchemaItemVduRect::setVertAlign(E::VertAlign align)
	{
		m_vertAlign = align;
	}
} // namespace VFrame30
