#include "SchemaItemValue.h"
#include "SchemaView.h"
#include "MacrosExpander.h"
#include "PropertyNames.h"
#include "DrawParam.h"

namespace VFrame30
{
	SchemaItemValue::SchemaItemValue(void) :
		SchemaItemValue(SchemaUnit::Inch)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemaItemValue::SchemaItemValue(SchemaUnit unit)
	{
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::lineWeight, PropertyNames::appearanceCategory, true, SchemaItemValue::weight, SchemaItemValue::setWeight);

		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::lineColor, PropertyNames::appearanceCategory, true, SchemaItemValue::lineColor, SchemaItemValue::setLineColor);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::fillColor, PropertyNames::appearanceCategory, true, SchemaItemValue::fillColor, SchemaItemValue::setFillColor)

		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::fill, PropertyNames::appearanceCategory, true, SchemaItemValue::fill, SchemaItemValue::setFill);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::drawRect, PropertyNames::appearanceCategory, true, SchemaItemValue::drawRect, SchemaItemValue::setDrawRect);

		// Text Category Properties
		//
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::textColor, PropertyNames::textCategory, true, SchemaItemValue::textColor, SchemaItemValue::setTextColor);

		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::text, PropertyNames::textCategory, true, SchemaItemValue::text, SchemaItemValue::setText);

		ADD_PROPERTY_GET_SET_CAT(E::HorzAlign, PropertyNames::alignHorz, PropertyNames::textCategory, true, SchemaItemValue::horzAlign, SchemaItemValue::setHorzAlign);
		ADD_PROPERTY_GET_SET_CAT(E::VertAlign, PropertyNames::alignVert, PropertyNames::textCategory, true, SchemaItemValue::vertAlign, SchemaItemValue::setVertAlign);

		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::fontName, PropertyNames::textCategory, true, SchemaItemValue::getFontName, SchemaItemValue::setFontName);
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::fontSize, PropertyNames::textCategory, true, SchemaItemValue::getFontSize, SchemaItemValue::setFontSize);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::fontBold, PropertyNames::textCategory, true, SchemaItemValue::getFontBold, SchemaItemValue::setFontBold);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::fontItalic, PropertyNames::textCategory, true,  SchemaItemValue::getFontItalic, SchemaItemValue::setFontItalic);

		// --
		//
		m_font.setName("Arial");

		switch (unit)
		{
		case SchemaUnit::Display:
			m_font.setSize(12.0, unit);
			break;
		case SchemaUnit::Inch:
			m_font.setSize(1.0 / 8.0, unit);		// 1/8"
			break;
		case SchemaUnit::Millimeter:
			m_font.setSize(mm2in(3), unit);
			break;
		default:
			assert(false);
		}

		m_static = false;
		setItemUnit(unit);
	}

	SchemaItemValue::~SchemaItemValue(void)
	{
	}

	// Serialization
	//
	bool SchemaItemValue::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectImpl::SaveData(message);
		if (result == false ||
			message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}
		
		// --
		//
		Proto::SchemaItemValue* valueMessage = message->mutable_schemaitem()->mutable_value();

		valueMessage->set_weight(m_weight);
		valueMessage->set_linecolor(m_lineColor.rgba());
		valueMessage->set_fillcolor(m_fillColor.rgba());
		valueMessage->set_fill(m_fill);
		valueMessage->set_drawrect(m_drawRect);

		valueMessage->set_horzalign(static_cast<int32_t>(m_horzAlign));
		valueMessage->set_vertalign(static_cast<int32_t>(m_vertAlign));

		valueMessage->set_text(m_text.toStdString());
		valueMessage->set_textcolor(m_textColor.rgba());
		m_font.SaveData(valueMessage->mutable_font());

		return true;
	}

	bool SchemaItemValue::LoadData(const Proto::Envelope& message)
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

		// --
		//
		if (message.schemaitem().has_value() == false)
		{
			assert(message.schemaitem().has_value());
			return false;
		}

		const Proto::SchemaItemValue& valueMessage = message.schemaitem().value();

		m_weight = valueMessage.weight();
		m_lineColor = QColor::fromRgba(valueMessage.linecolor());
		m_fillColor = QColor::fromRgba(valueMessage.fillcolor());
		m_fill = valueMessage.fill();
		m_text = QString::fromStdString(valueMessage.text());
		m_textColor = QColor::fromRgba(valueMessage.textcolor());
		m_drawRect = valueMessage.drawrect();

		m_horzAlign = static_cast<E::HorzAlign>(valueMessage.horzalign());
		m_vertAlign = static_cast<E::VertAlign>(valueMessage.vertalign());

		m_font.LoadData(valueMessage.font());

		return true;
	}

	// Drawing Functions
	//

	// Рисование элемента, выполняется в 100% масштабе.
	// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
	//
	void SchemaItemValue::Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* /*layer*/) const
	{
		QPainter* p = drawParam->painter();

		// Initialization drawing resources
		//
		if (m_rectPen.get() == nullptr)
		{
			m_rectPen = std::make_shared<QPen>();
		}

		QColor qlinecolor(lineColor());
		if (m_rectPen->color() !=qlinecolor )
		{
			m_rectPen->setColor(qlinecolor);
		}

		if (m_fillBrush.get() == nullptr)
		{
			m_fillBrush = std::make_shared<QBrush>(Qt::SolidPattern);
		}
						
		// Calculate rectangle
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

		// Filling rect 
		//
		if (fill() == true)
		{
			QPainter::RenderHints oldrenderhints = p->renderHints();
			p->setRenderHint(QPainter::Antialiasing, false);

			QColor qfillcolor(fillColor());

			m_fillBrush->setColor(qfillcolor);
			p->fillRect(r, *m_fillBrush);		// 22% если использовать Qcolor и намного меньше если использовать готовый Brush

			p->setRenderHints(oldrenderhints);
		}

		// Drawing rect 
		//
		if (drawRect() == true)
		{
			m_rectPen->setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);

			p->setPen(*m_rectPen);
			p->drawRect(r);
		}

		// Drawing Text
		//
		MacrosExpander me;
		QString text = me.parse(m_text, drawParam->session(), schema, this);

		if (m_text.isEmpty() == false)
		{
			p->setPen(textColor());
			DrawHelper::drawText(p, m_font, itemUnit(), text, r, horzAlign() | vertAlign());
		}

		return;
	}

	bool SchemaItemValue::searchText(const QString& text) const
	{
		return SchemaItem::searchText(text) ||
				m_text.contains(text, Qt::CaseInsensitive);
	}

	double SchemaItemValue::minimumPossibleHeightDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	double SchemaItemValue::minimumPossibleWidthDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	// Properties and Data
	//
	IMPLEMENT_FONT_PROPERTIES(SchemaItemValue, Font, m_font);

	// Weight property
	//
	double SchemaItemValue::weight() const
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			return CUtils::RoundDisplayPoint(m_weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(m_weight, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			pt = CUtils::RoundPoint(pt, Settings::regionalUnit());
			return pt;
		}
	}

	void SchemaItemValue::setWeight(double weight)
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

	// LineColor property
	//
	QColor SchemaItemValue::lineColor() const
	{
		return m_lineColor;
	}
	void SchemaItemValue::setLineColor(QColor color)
	{
		m_lineColor = color;
	}

	// FillColor property
	//
	QColor SchemaItemValue::fillColor() const
	{
		return m_fillColor;
	}
	void SchemaItemValue::setFillColor(QColor color)
	{
		m_fillColor = color;
	}

	// TextColor property
	//
	QColor SchemaItemValue::textColor() const
	{
		return m_textColor;
	}
	void SchemaItemValue::setTextColor(QColor color)
	{
		m_textColor = color;
	}

	// Text property
	//
	const QString& SchemaItemValue::text() const
	{
		return m_text;
	}
	void SchemaItemValue::setText(QString value)
	{
		m_text = value;
	}

	// Align propertis
	//
	E::HorzAlign SchemaItemValue::horzAlign() const
	{
		return m_horzAlign;
	}
	void SchemaItemValue::setHorzAlign(E::HorzAlign align)
	{
		m_horzAlign = align;
	}

	E::VertAlign SchemaItemValue::vertAlign() const
	{
		return m_vertAlign;
	}

	void SchemaItemValue::setVertAlign(E::VertAlign align)
	{
		m_vertAlign = align;
	}

	// Fill property
	//
	bool SchemaItemValue::fill() const
	{
		return m_fill;
	}
	void SchemaItemValue::setFill(bool fill)
	{
		m_fill = fill;
	}


	bool SchemaItemValue::drawRect() const
	{
		return m_drawRect;
	}

	void SchemaItemValue::setDrawRect(bool value)
	{
		m_drawRect = value;
	}
}

