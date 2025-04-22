#include <VFrame30/DrawParam.h>
#include <VFrame30/MacrosExpander.h>
#include <VFrame30/PropertyNames.h>
#include <VFrame30/SchemaItemRect.h>
#include <VFrame30/SchemaView.h>


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
		m_font.setName(QStringLiteral("Arial"));

		switch (unit)
		{
		case SchemaUnit::Display:
			m_font.setSize(12.0, unit);
			break;
		case SchemaUnit::Inch:
			m_font.setSize(1.0 / 8.0, unit); // 1/8"
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

	void SchemaItemRect::propertyDemand(const QString& prop)
	{
		PosRectRotatable::propertyDemand(prop);

		// clang-format off

		if (prop.isEmpty() == true || prop == PropertyNames::lineWeight)
		{
			ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::lineWeight, PropertyNames::appearanceCategory, true, SchemaItemRect::weight, SchemaItemRect::setWeight);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::lineColor)
		{
			ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::lineColor, PropertyNames::appearanceCategory, true, SchemaItemRect::lineColor, SchemaItemRect::setLineColor);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::fillColor)
		{
			ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::fillColor, PropertyNames::appearanceCategory, true, SchemaItemRect::fillColor, SchemaItemRect::setFillColor);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::fill)
		{
			ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::fill, PropertyNames::appearanceCategory, true, SchemaItemRect::fill, SchemaItemRect::setFill);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::drawRect)
		{
			ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::drawRect, PropertyNames::appearanceCategory, true, SchemaItemRect::drawRect, SchemaItemRect::setDrawRect);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::lineStyle)
		{
			ADD_PROPERTY_GET_SET_CAT(E::LineStyle, PropertyNames::lineStyle, PropertyNames::appearanceCategory, true, SchemaItemRect::lineStyle, SchemaItemRect::setLineStyle);
		}

		// Text Category Properties
		//
		if (prop.isEmpty() == true || prop == PropertyNames::textFormat)
		{
			ADD_PROPERTY_GET_SET_CAT(E::TextFormat, PropertyNames::textFormat, PropertyNames::textCategory, true, SchemaItemRect::textFormat, SchemaItemRect::setTextFormat)
				->setDescription(PropertyNames::textFormatDescription);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::textColor)
		{
			ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::textColor, PropertyNames::textCategory, true, SchemaItemRect::textColor, SchemaItemRect::setTextColor);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::text)
		{
			ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::text, PropertyNames::textCategory, true, SchemaItemRect::text, SchemaItemRect::setText)
				->setEssential(true);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::wordWrap)
		{
			ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::wordWrap, PropertyNames::textCategory, true, SchemaItemRect::wordWrap, SchemaItemRect::setWordWrap);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::alignHorz)
		{
			ADD_PROPERTY_GET_SET_CAT(E::HorzAlign, PropertyNames::alignHorz, PropertyNames::textCategory, true, SchemaItemRect::horzAlign, SchemaItemRect::setHorzAlign);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::alignVert)
		{
			ADD_PROPERTY_GET_SET_CAT(E::VertAlign, PropertyNames::alignVert, PropertyNames::textCategory, true, SchemaItemRect::vertAlign, SchemaItemRect::setVertAlign);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::fontName)
		{
			ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::fontName, PropertyNames::textCategory, true, SchemaItemRect::getFontName, SchemaItemRect::setFontName);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::fontSize)
		{
			ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::fontSize, PropertyNames::textCategory, true, SchemaItemRect::getFontSize, SchemaItemRect::setFontSize);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::fontBold)
		{
			ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::fontBold, PropertyNames::textCategory, true, SchemaItemRect::getFontBold, SchemaItemRect::setFontBold);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::fontItalic)
		{
			ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::fontItalic, PropertyNames::textCategory, true, SchemaItemRect::getFontItalic, SchemaItemRect::setFontItalic);
		}

		// clang-format on
		return;
	}

	// Serialization
	//
	bool SchemaItemRect::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectRotatable::SaveData(message);
		if (result == false || message->HasExtension(Proto::schemaitem) == false)
		{
			assert(result);
			assert(message->HasExtension(Proto::schemaitem));
			return false;
		}

		// --
		//
		Proto::SchemaItemRect* rectMessage = message->MutableExtension(Proto::schemaitem)->mutable_rect();

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
		if (message.HasExtension(Proto::schemaitem) == false)
		{
			assert(message.HasExtension(Proto::schemaitem));
			return false;
		}

		// --
		//
		bool result = PosRectRotatable::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.GetExtension(Proto::schemaitem).has_rect() == false)
		{
			assert(message.GetExtension(Proto::schemaitem).has_rect());
			return false;
		}

		const Proto::SchemaItemRect& rectMessage = message.GetExtension(Proto::schemaitem).rect();

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
		return drawRotated(drawParam,
						   [drawParam, this]()
						   {
							   return drawPrivate(drawParam);
						   });
	}

	void SchemaItemRect::drawPrivate(CDrawParam* drawParam) const
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

		if (text.trimmed().isEmpty() == true)
		{
			return;
		}

#ifdef SCHEMA_ITEM_RECT_CACHE_TEXT_DRAWING
		bool textChanged = (text != m_cacheDrewText) || (m_cachetextFormat != m_textFormat) || (m_cachedFont != m_font);

		if (textChanged == true)
		{
			m_cacheDrewText = text;
			m_cachetextFormat = m_textFormat;
			m_cachedFont = m_font;
		}

		if (drawParam->pdfMode() == true)
#endif
		{
			// For pdf draw text without caching to image, it makes pdf much smaller and will allows
			// to select and copy text to the clipboard.
			//
			if (textFormat() == E::TextFormat::PlainText)
			{
				drawPlainText(*painter, boundingRect, text);
			}
			else
			{
				// Problems in drawing directly to Pdf painter device, it is possible to draw normaly only to pos 0, 0
				// in other cases text is cut. Translate device to avoid the problem.
				//
				painter->save();
				painter->translate(boundingRect.left(), boundingRect.top());

				QRectF drawRect = boundingRect;
				drawRect.moveTo(0, 0);

				drawMarkdown(*painter, drawRect, text, true);

				painter->restore();
			}

			return;
		}

#ifdef SCHEMA_ITEM_RECT_CACHE_TEXT_DRAWING
		// Check if image already cached, if not, then create one
		//
		if (itemUnit() == SchemaUnit::Display)
		{
			// Pixels
			//
			const double zoom = m_drawParam->schemaView()->zoom() / 100.0;
			const double imageWidth = widthDocPt() * zoom;
			const double imageHeight = heightDocPt() * zoom;

			QRect clipRectInt{0, 0, static_cast<int>(imageWidth), static_cast<int>(imageHeight)};

			if (textChanged == true || m_cacheTextImage.isNull() == true || m_cacheTextImage.size() != clipRectInt.size())
			{
				const double deviceDpr = drawParam->devicePixelRatio();

				const double dpiX = CDrawParam::realDpiX(painter);
				const double dpiY = CDrawParam::realDpiY(painter);
				const double physicalDpiX = dpiX / deviceDpr;
				const double physicalDpiY = dpiY / deviceDpr;

				m_cacheTextImage = QImage{clipRectInt.size(), QImage::Format_ARGB32_Premultiplied};
				m_cacheTextImage.setDevicePixelRatio(deviceDpr);

				m_cacheTextImage.fill(qRgba(0, 0, 0, 0)); // Transparent

				QPainter p{&m_cacheTextImage};

				SchemaView::Ajust(&p,
								  physicalDpiX,
								  physicalDpiY,
								  painter->device()->devicePixelRatioF(),
								  itemUnit(),
								  0,
								  0,
								  m_drawParam->schemaView()->zoom());

				QRectF textRect = boundingRect;
				textRect.moveTo(0, 0);

				if (textFormat() == E::TextFormat::PlainText)
				{
					drawPlainText(p, textRect, text);
				}
				else
				{
					drawMarkdown(p, textRect, text, textChanged);
				}
			}
		}
		else
		{
			// Inches
			//
			const double dpiX = CDrawParam::realScreenDpiX(painter);
			const double dpiY = CDrawParam::realScreenDpiY(painter);

			const double zoomFactor = m_drawParam->schemaView()->zoom() / 100.0;

			const double imageWidth = std::ceil(boundingRect.width() * dpiX * zoomFactor);
			const double imageHeight = std::ceil(boundingRect.height() * dpiY * zoomFactor);

			QSize imageSize{static_cast<int>(imageWidth), static_cast<int>(imageHeight)};
			QRect clipRectInt{0, 0, static_cast<int>(imageWidth), static_cast<int>(imageHeight)};

			if (textChanged == true || m_cacheTextImage.isNull() == true || m_cacheTextImage.size() != clipRectInt.size())
			{
				double deviceDpr = drawParam->devicePixelRatio();
				double physicalDpiX = dpiX / deviceDpr;
				double physicalDpiY = dpiY / deviceDpr;

				m_cacheTextImage = QImage{clipRectInt.size(), QImage::Format_ARGB32_Premultiplied};
				m_cacheTextImage.setDotsPerMeterX(static_cast<int>(physicalDpiX / 25.4 * 1000.0));
				m_cacheTextImage.setDotsPerMeterY(static_cast<int>(physicalDpiY / 25.4 * 1000.0));
				m_cacheTextImage.setDevicePixelRatio(deviceDpr);

				m_cacheTextImage.fill(qRgba(0, 0, 0, 0)); // Transparent

				QPainter p{&m_cacheTextImage};
				SchemaView::Ajust(&p, physicalDpiX, physicalDpiY, deviceDpr, itemUnit(), 0, 0, m_drawParam->schemaView()->zoom());

				QRectF textRect = boundingRect;
				textRect.moveTo(0, 0);

				if (textFormat() == E::TextFormat::PlainText)
				{
					drawPlainText(p, textRect, text);
				}
				else
				{
					drawMarkdown(p, textRect, text, textChanged || drawParam->pdfMode());
				}
			}
		}

		// Draw cached image
		//
		QRectF sourceRect = m_cacheTextImage.rect();
		painter->drawImage(boundingRect, m_cacheTextImage, sourceRect);
#endif

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

	void SchemaItemRect::drawPlainText(QPainter& painter, QRectF rect, QString text) const
	{
		int flags = static_cast<int>(horzAlign()) | static_cast<int>(vertAlign()) | static_cast<int>((wordWrap() ? Qt::TextWordWrap : 0));

		painter.setPen(m_textColor);

		DrawHelper::drawText(&painter, m_font, itemUnit(), text, rect, flags);

		return;
	}

	void SchemaItemRect::drawMarkdown(QPainter& painter, QRectF rect, QString text, bool textChanged) const
	{
		const double dpiX = CDrawParam::realDpiX(&painter);
		const double dpiY = CDrawParam::realDpiY(&painter);

		QFont font(m_font.name());
		font.setBold(m_font.bold());
		font.setItalic(m_font.italic());

		if (itemUnit() == SchemaUnit::Display)
		{
			const int pixelSize = static_cast<int>(m_font.drawSize());
			font.setPixelSize(pixelSize > 0 ? pixelSize : 1);
		}
		else
		{
			painter.save();
			painter.scale(1.0 / dpiX, 1.0 / dpiY);

			const int pixelSize = static_cast<int>(m_font.drawSize() * dpiY);
			font.setPixelSize(pixelSize > 0 ? pixelSize : 1);

			rect = QRectF{rect.left() * dpiX, rect.top() * dpiY, rect.width() * dpiX, rect.height() * dpiY};
		}

		painter.setFont(font);

		m_cacheTextDocument.documentLayout()->setPaintDevice(painter.device());
		m_cacheTextDocument.setDefaultFont(font);

		auto dto = m_cacheTextDocument.defaultTextOption();
		dto.setWrapMode(m_wordWrap ? QTextOption::WrapMode::WrapAtWordBoundaryOrAnywhere : QTextOption::WrapMode::NoWrap);
		m_cacheTextDocument.setTextWidth(rect.width());
		m_cacheTextDocument.setDefaultTextOption(dto);

		// Set new text to m_cacheTextDocument only after setting paint device and font
		// or it will calculate wrong line indents.
		//
		if (textChanged == true)
		{
			switch (m_textFormat)
			{
			case E::TextFormat::PlainText:
				break;
			case E::TextFormat::Markdown:
				m_cacheTextDocument.setMarkdown(text);
				break;
			case E::TextFormat::HtmlSubset:
				m_cacheTextDocument.setHtml(text);
				break;
			}
		}

		m_cacheTextDocument.drawContents(&painter, rect);

		if (itemUnit() == SchemaUnit::Display) {}
		else
		{
			painter.restore();
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
#ifdef SCHEMA_ITEM_RECT_CACHE_TEXT_DRAWING
			m_cacheTextImage = {};
#endif
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
#ifdef SCHEMA_ITEM_RECT_CACHE_TEXT_DRAWING
			m_cacheTextImage = {};
#endif
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
#ifdef SCHEMA_ITEM_RECT_CACHE_TEXT_DRAWING
			m_cacheTextImage = {};
#endif
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
#ifdef SCHEMA_ITEM_RECT_CACHE_TEXT_DRAWING
		m_cacheTextImage = {};
#endif
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
#ifdef SCHEMA_ITEM_RECT_CACHE_TEXT_DRAWING
		m_cacheTextImage = {};
#endif
	}

	E::VertAlign SchemaItemRect::vertAlign() const
	{
		return m_vertAlign;
	}

	void SchemaItemRect::setVertAlign(E::VertAlign align)
	{
		m_vertAlign = align;
#ifdef SCHEMA_ITEM_RECT_CACHE_TEXT_DRAWING
		m_cacheTextImage = {};
#endif
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

} // namespace VFrame30
