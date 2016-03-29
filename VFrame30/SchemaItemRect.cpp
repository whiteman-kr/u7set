#include "Stable.h"
#include "SchemaItemRect.h"

namespace VFrame30
{
	SchemaItemRect::SchemaItemRect(void) :
		SchemaItemRect(SchemaUnit::Inch)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemaItemRect::SchemaItemRect(SchemaUnit unit) :
		m_weight(0),
		m_lineColor(qRgb(0x00, 0x00, 0x00)),
		m_fillColor(qRgb(0xC0, 0xC0, 0xC0)),
		m_textColor(qRgb(0x00, 0x00, 0x00)),
		m_fill(true)
	{
		ADD_PROPERTY_GETTER_SETTER(double, LineWeight, true, SchemaItemRect::weight, SchemaItemRect::setWeight);

		ADD_PROPERTY_GETTER_SETTER(QColor, LineColor, true, SchemaItemRect::lineColor, SchemaItemRect::setLineColor);
		ADD_PROPERTY_GETTER_SETTER(QColor, FillColor, true, SchemaItemRect::fillColor, SchemaItemRect::setFillColor)
		ADD_PROPERTY_GETTER_SETTER(QColor, TextColor, true, SchemaItemRect::textColor, SchemaItemRect::setTextColor);

		ADD_PROPERTY_GETTER_SETTER(bool, Fill, true, SchemaItemRect::fill, SchemaItemRect::setFill);
		ADD_PROPERTY_GETTER_SETTER(QString, Text, true, SchemaItemRect::text, SchemaItemRect::setText);

		ADD_PROPERTY_GETTER_SETTER(bool, DrawRect, true, SchemaItemRect::drawRect, SchemaItemRect::setDrawRect);

		ADD_PROPERTY_GETTER_SETTER(QString, FontName, true, SchemaItemRect::getFontName, SchemaItemRect::setFontName);
		ADD_PROPERTY_GETTER_SETTER(double, FontSize, true, SchemaItemRect::getFontSize, SchemaItemRect::setFontSize);
		ADD_PROPERTY_GETTER_SETTER(bool, FontBold, true, SchemaItemRect::getFontBold, SchemaItemRect::setFontBold);
		ADD_PROPERTY_GETTER_SETTER(bool, FontItalic, true,  SchemaItemRect::getFontItalic, SchemaItemRect::setFontItalic);

		// --
		//
		m_font.setName("Arial");

		switch (unit)
		{
		case SchemaUnit::Display:
			m_font.setSize(12.0, unit);
			break;
		case SchemaUnit::Inch:
			m_font.setSize(mm2in(2.5), unit);
			break;
		case SchemaUnit::Millimeter:
			m_font.setSize(mm2in(2.5), unit);
			break;
		default:
			assert(false);
		}

		m_static = true;
		setItemUnit(unit);
	}

	SchemaItemRect::~SchemaItemRect(void)
	{
	}

	// Serialization
	//
	bool SchemaItemRect::SaveData(Proto::Envelope* message) const
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
		Proto::SchemaItemRect* rectMessage = message->mutable_schemaitem()->mutable_rect();

		rectMessage->set_weight(m_weight);
		rectMessage->set_linecolor(m_lineColor.rgba());
		rectMessage->set_fillcolor(m_fillColor.rgba());
		rectMessage->set_fill(m_fill);
		rectMessage->set_drawrect(m_drawRect);

		Proto::Write(rectMessage->mutable_text(), m_text);
		rectMessage->set_textcolor(m_textColor.rgba());
		m_font.SaveData(rectMessage->mutable_font());

		return true;
	}

	bool SchemaItemRect::LoadData(const Proto::Envelope& message)
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
		if (message.schemaitem().has_rect() == false)
		{
			assert(message.schemaitem().has_rect());
			return false;
		}

		const Proto::SchemaItemRect& rectMessage = message.schemaitem().rect();

		m_weight = rectMessage.weight();
		m_lineColor = QColor::fromRgba(rectMessage.linecolor());
		m_fillColor = QColor::fromRgba(rectMessage.fillcolor());
		m_fill = rectMessage.fill();
		Proto::Read(rectMessage.text(), &m_text);
		m_textColor = QColor::fromRgba(rectMessage.textcolor());
		m_drawRect = rectMessage.drawrect();
		m_font.LoadData(rectMessage.font());

		return true;
	}

	// Drawing Functions
	//

	// Рисование элемента, выполняется в 100% масштабе.
	// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
	//
	void SchemaItemRect::Draw(CDrawParam* drawParam, const Schema*, const SchemaLayer*) const
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
			m_rectPen->setWidthF(static_cast<qreal>(m_weight));

			p->setPen(*m_rectPen);
			p->drawRect(r);
		}

		// Drawing Text
		//
		if (m_text.isEmpty() == false)
		{
			p->setPen(textColor());
			DrawHelper::DrawText(p, m_font, itemUnit(), m_text, r, Qt::AlignLeft | Qt::AlignTop);
		}

		return;
	}

	// Properties and Data
	//
	IMPLEMENT_FONT_PROPERTIES(SchemaItemRect, Font, m_font);

	// Weight property
	//
	double SchemaItemRect::weight() const
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			return CUtils::RoundDisplayPoint(m_weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(m_weight, SchemaUnit::Inch, Settings::regionalUnit(), ConvertDirection::Horz);
			pt = CUtils::RoundPoint(pt, Settings::regionalUnit());
			return pt;
		}
	}

	void SchemaItemRect::setWeight(double weight)
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			m_weight = CUtils::RoundDisplayPoint(weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(weight, Settings::regionalUnit(), SchemaUnit::Inch, ConvertDirection::Horz);
			m_weight = pt;
		}
	}

	// LineColor property
	//
	QColor SchemaItemRect::lineColor() const
	{
		return m_lineColor;
	}
	void SchemaItemRect::setLineColor(QColor color)
	{
		m_lineColor = color;
	}

	// FillColor property
	//
	QColor SchemaItemRect::fillColor() const
	{
		return m_fillColor;
	}
	void SchemaItemRect::setFillColor(QColor color)
	{
		m_fillColor = color;
	}

	// Fill property
	//
	bool SchemaItemRect::fill() const
	{
		return m_fill;
	}
	void SchemaItemRect::setFill(bool fill)
	{
		m_fill = fill;
	}

	// Text property
	//
	const QString& SchemaItemRect::text() const
	{
		return m_text;
	}
	void SchemaItemRect::setText(QString value)
	{
		m_text = value;
	}

	// TextColor property
	//
	QColor SchemaItemRect::textColor() const
	{
		return m_textColor;
	}
	void SchemaItemRect::setTextColor(QColor color)
	{
		m_textColor = color;
	}

	bool SchemaItemRect::drawRect() const
	{
		return m_drawRect;
	}

	void SchemaItemRect::setDrawRect(bool value)
	{
		m_drawRect = value;
	}

}

