#include "SchemaItemRect.h"
#include "MacrosExpander.h"
#include "PropertyNames.h"
#include "DrawParam.h"
#include "SchemaView.h"
#include <QAbstractTextDocumentLayout>

namespace VFrame30
{
	SchemaItemRect::SchemaItemRect(void) :
		SchemaItemRect(SchemaUnit::Inch)
	{
	}

	SchemaItemRect::SchemaItemRect(SchemaUnit unit) :
		m_lineColor(qRgb(0x00, 0x00, 0x00)),
		m_fillColor(qRgb(0xC0, 0xC0, 0xC0)),
		m_textColor(qRgb(0x00, 0x00, 0x00))
	{
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::lineWeight, PropertyNames::appearanceCategory, true, SchemaItemRect::weight, SchemaItemRect::setWeight);

		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::lineColor, PropertyNames::appearanceCategory, true, SchemaItemRect::lineColor, SchemaItemRect::setLineColor);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::fillColor, PropertyNames::appearanceCategory, true, SchemaItemRect::fillColor, SchemaItemRect::setFillColor);

		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::fill, PropertyNames::appearanceCategory, true, SchemaItemRect::fill, SchemaItemRect::setFill);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::drawRect, PropertyNames::appearanceCategory, true, SchemaItemRect::drawRect, SchemaItemRect::setDrawRect);

		ADD_PROPERTY_GET_SET_CAT(E::LineStyle, PropertyNames::lineStyle, PropertyNames::appearanceCategory, true, SchemaItemRect::lineStyle, SchemaItemRect::setLineStyle);

		// Text Category Properties
		//
		ADD_PROPERTY_GET_SET_CAT(E::TextFormat, PropertyNames::textFormat, PropertyNames::textCategory, true, SchemaItemRect::textFormat, SchemaItemRect::setTextFormat)
			->setDescription(PropertyNames::textFormatDescription);

		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::textColor, PropertyNames::textCategory, true, SchemaItemRect::textColor, SchemaItemRect::setTextColor);

		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::text, PropertyNames::textCategory, true, SchemaItemRect::text, SchemaItemRect::setText);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::wordWrap, PropertyNames::textCategory, true, SchemaItemRect::wordWrap, SchemaItemRect::setWordWrap);

		ADD_PROPERTY_GET_SET_CAT(E::HorzAlign, PropertyNames::alignHorz, PropertyNames::textCategory, true, SchemaItemRect::horzAlign, SchemaItemRect::setHorzAlign);
		ADD_PROPERTY_GET_SET_CAT(E::VertAlign, PropertyNames::alignVert, PropertyNames::textCategory, true, SchemaItemRect::vertAlign, SchemaItemRect::setVertAlign);

		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::fontName, PropertyNames::textCategory, true, SchemaItemRect::getFontName, SchemaItemRect::setFontName);
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::fontSize, PropertyNames::textCategory, true, SchemaItemRect::getFontSize, SchemaItemRect::setFontSize);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::fontBold, PropertyNames::textCategory, true, SchemaItemRect::getFontBold, SchemaItemRect::setFontBold);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::fontItalic, PropertyNames::textCategory, true,  SchemaItemRect::getFontItalic, SchemaItemRect::setFontItalic);

		// --
		//
		m_font.setName(QStringLiteral("Arial"));

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

		m_static = true;
		setItemUnit(unit);
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

		rectMessage->set_linestyle(static_cast<int>(m_lineStyle));

		rectMessage->set_textformat(static_cast<int32_t>(m_textFormat));
		rectMessage->set_horzalign(static_cast<int32_t>(m_horzAlign));
		rectMessage->set_vertalign(static_cast<int32_t>(m_vertAlign));

		Proto::Write(rectMessage->mutable_text(), m_text);
		rectMessage->set_textcolor(m_textColor.rgba());
		m_font.SaveData(rectMessage->mutable_font());

		rectMessage->set_wordwrap(m_wordWrap);

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

		m_lineStyle = static_cast<E::LineStyle>(rectMessage.linestyle());

		setTextFormat(static_cast<E::TextFormat>(rectMessage.textformat()));
		m_horzAlign = static_cast<E::HorzAlign>(rectMessage.horzalign());
		m_vertAlign = static_cast<E::VertAlign>(rectMessage.vertalign());

		m_font.LoadData(rectMessage.font());

		m_wordWrap = rectMessage.wordwrap();

		return true;
	}

	// Drawing Functions
	//
	void SchemaItemRect::draw(CDrawParam* drawParam) const
	{
		QPainter* painter = drawParam->painter();

		// Initialization drawing resources
		//
		if (m_rectPen.get() == nullptr)
		{
			m_rectPen = std::make_shared<QPen>();
		}

		if (m_rectPen->color() != m_lineColor)
		{
			m_rectPen->setColor(m_lineColor);
		}

		if (m_fillBrush == nullptr)
		{
			m_fillBrush = std::make_shared<QBrush>(m_fillColor, Qt::SolidPattern);
		}

		if (m_fillBrush->color() != m_fillColor)
		{
			m_fillBrush->setColor(m_fillColor);
		}
						
		// Calculate rectangle
		//
		QRectF boundingRect = boundingRectInDocPt(drawParam);

		// Filling rect 
		//
		if (fill() == true)
		{
			painter->setBrush(*m_fillBrush);

			if (drawRect() == false)
			{
				painter->fillRect(boundingRect, *m_fillBrush);
			}
		}
		else
		{
			painter->setBrush(Qt::NoBrush);
		}

		// Drawing rect 
		//
		if (drawRect() == true)
		{
			m_rectPen->setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);
			m_rectPen->setStyle(static_cast<Qt::PenStyle>(m_lineStyle));

			painter->setPen(*m_rectPen);
			painter->drawRect(boundingRect);
		}

		// Drawing Text
		//
		auto context = this->context();
		Q_ASSERT(context);

		QString text = MacrosExpander::parse(m_text, context.get(), &drawParam->session(), this);

		QFont font(m_font.name());
		font.setBold(m_font.bold());
		font.setItalic(m_font.italic());

		int flags = static_cast<int>(horzAlign()) |
					static_cast<int>(vertAlign()) |
					static_cast<int>((wordWrap() ? Qt::TextWordWrap : 0));

		if (itemUnit() == SchemaUnit::Display)
		{
			// Pixels
			//
			const double zoom = m_drawParam->schemaView()->zoom() / 100.0;
			const double imageWidth = widthDocPt() * zoom;
			const double imageHeight = heightDocPt()  * zoom;

			QRectF clipRect{0, 0, imageWidth, imageHeight};
			QRect clipRectInt{0, 0, static_cast<int>(imageWidth), static_cast<int>(imageHeight)};

			if (m_cacheTextImage.isNull() == true || m_cacheTextImage.size() != clipRectInt.size())
			{
				m_cacheTextImage = QImage{clipRectInt.size(), QImage::Format_ARGB32_Premultiplied};
				m_cacheTextImage.fill(qRgba(0, 0, 0, 0));	// Transparent

				QPainter p{&m_cacheTextImage};

				const int pixelSize = static_cast<int>(m_font.drawSize() * zoom);
				font.setPixelSize(pixelSize > 0 ? pixelSize : 1);

				if (textFormat() == E::TextFormat::PlainText)
				{
					p.setPen(m_textColor);
					p.setFont(font);

					p.drawText(clipRectInt, flags, text);
				}
				else
				{
					m_cacheTextDocument.documentLayout()->setPaintDevice(p.device());
					m_cacheTextDocument.setDefaultFont(font);

					m_cacheTextDocument.drawContents(&p, clipRect);
				}
			}

			QRectF sourceRect = m_cacheTextImage.rect();
			painter->drawImage(boundingRect, m_cacheTextImage, sourceRect);
		}
		else
		{
			// Inches
			//
			const double dpiX = CDrawParam::realDpiX(painter);
			const double dpiY = CDrawParam::realDpiY(painter);

			const double zoom = m_drawParam->schemaView()->zoom() / 100.0;
			const double imageWidth = widthDocPt() * dpiX * zoom;
			const double imageHeight = heightDocPt() * dpiY * zoom;

			QRectF clipRect{0, 0, imageWidth, imageHeight};
			QRect clipRectInt{0, 0, static_cast<int>(imageWidth), static_cast<int>(imageHeight)};

			if (m_cacheTextImage.isNull() == true || m_cacheTextImage.size() != clipRectInt.size())
			{
				m_cacheTextImage = QImage{clipRectInt.size(), QImage::Format_ARGB32_Premultiplied};
				m_cacheTextImage.setDevicePixelRatio(painter->device()->devicePixelRatioF());
				m_cacheTextImage.fill(qRgba(0, 0, 0, 0));	// Transparent

				QPainter p{&m_cacheTextImage};

				const int pixelSize = static_cast<int>(m_font.drawSize() * dpiY * zoom / m_cacheTextImage.devicePixelRatioF());
				font.setPixelSize(pixelSize > 0 ? pixelSize : 1);

				if (textFormat() == E::TextFormat::PlainText)
				{
					p.setPen(m_textColor);
					p.setFont(font);

					p.drawText(clipRectInt, flags, text);
				}
				else
				{
					m_cacheTextDocument.documentLayout()->setPaintDevice(p.device());
					m_cacheTextDocument.setDefaultFont(font);

					m_cacheTextDocument.drawContents(&p, clipRect);
				}
			}

			QRectF sourceRect = m_cacheTextImage.rect();
			painter->drawImage(boundingRect, m_cacheTextImage, sourceRect);
		}

		return;
	}

	double SchemaItemRect::minimumPossibleHeightDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
	}

	double SchemaItemRect::minimumPossibleWidthDocPt(double gridSize, int /*pinGridStep*/) const
	{
		return gridSize;
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
			return VFrame30::RoundDisplayPoint(m_weight);
		}
		else
		{
			double pt = VFrame30::ConvertPoint(m_weight, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			pt = VFrame30::RoundPoint(pt, Settings::regionalUnit());
			return pt;
		}
	}

	void SchemaItemRect::setWeight(double weight)
	{
		if (weight < 0)
		{
			weight = 0;
		}

		if (itemUnit() == SchemaUnit::Display)
		{
			m_weight = VFrame30::RoundDisplayPoint(weight);
		}
		else
		{
			double pt = VFrame30::ConvertPoint(weight, Settings::regionalUnit(), SchemaUnit::Inch, 0);
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

	// TextColor property
	//
	QColor SchemaItemRect::textColor() const
	{
		return m_textColor;
	}
	void SchemaItemRect::setTextColor(QColor color)
	{
		if (m_textColor != color)
		{
			m_textColor = color;
			m_cacheTextImage = {};
		}
	}

	// LineStyle property
	//
	E::LineStyle SchemaItemRect::lineStyle() const
	{
		return m_lineStyle;
	}

	void SchemaItemRect::setLineStyle(E::LineStyle value)
	{
		m_lineStyle = value;
	}

	// Text property
	//
	E::TextFormat SchemaItemRect::textFormat() const
	{
		return m_textFormat;
	}

	void SchemaItemRect::setTextFormat(E::TextFormat value)
	{
		if (m_textFormat != value)
		{
			m_textFormat = value;

			switch (m_textFormat)
			{
			case E::TextFormat::PlainText:
				m_cacheTextDocument.setPlainText({});
				break;
			case E::TextFormat::Markdown:
				m_cacheTextDocument.setMarkdown(m_text);
				break;
			case E::TextFormat::HtmlSubset:
				m_cacheTextDocument.setHtml(m_text);
				break;
			}

			m_cacheTextImage = {};
		}
	}

	const QString& SchemaItemRect::text() const
	{
		return m_text;
	}
	void SchemaItemRect::setText(QString value)
	{
		if (m_text != value)
		{
			m_text = std::move(value);

			switch (m_textFormat)
			{
			case E::TextFormat::PlainText:
				m_cacheTextDocument.setPlainText({});
				break;
			case E::TextFormat::Markdown:
				m_cacheTextDocument.setMarkdown(m_text);
				break;
			case E::TextFormat::HtmlSubset:
				m_cacheTextDocument.setHtml(m_text);
				break;
			}

			m_cacheTextImage = {};
		}

		return;
	}

	bool SchemaItemRect::wordWrap() const
	{
		return m_wordWrap;
	}

	void SchemaItemRect::setWordWrap(bool value)
	{
		m_wordWrap = value;
		m_cacheTextImage = {};
	}

	// Align propertis
	//
	E::HorzAlign SchemaItemRect::horzAlign() const
	{
		return m_horzAlign;
	}
	void SchemaItemRect::setHorzAlign(E::HorzAlign align)
	{
		m_horzAlign = align;
		m_cacheTextImage = {};
	}

	E::VertAlign SchemaItemRect::vertAlign() const
	{
		return m_vertAlign;
	}

	void SchemaItemRect::setVertAlign(E::VertAlign align)
	{
		m_vertAlign = align;
		m_cacheTextImage = {};
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


	bool SchemaItemRect::drawRect() const
	{
		return m_drawRect;
	}

	void SchemaItemRect::setDrawRect(bool value)
	{
		m_drawRect = value;
	}

}

