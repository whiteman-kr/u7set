#include "Stable.h"
#include "VideoItemRect.h"

namespace VFrame30
{
	VideoItemRect::VideoItemRect(void)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	VideoItemRect::VideoItemRect(SchemeUnit unit) : 
		m_weight(0),
		m_lineColor(qRgb(0x00, 0x00, 0x00)),
		m_fillColor(qRgb(0xC0, 0xC0, 0xC0)),
		m_textColor(qRgb(0x00, 0x00, 0x00)),
		m_fill(true)
	{
		m_font.setName("Arial");

		switch (unit)
		{
		case SchemeUnit::Display:
			m_font.setSize(12.0, unit);
			break;
		case SchemeUnit::Inch:
			m_font.setSize(mm2in(2.5), unit);
			break;
		case SchemeUnit::Millimeter:
			m_font.setSize(mm2in(2.5), unit);
			break;
		default:
			assert(false);
		}

		m_static = true;
		setItemUnit(unit);
	}

	VideoItemRect::~VideoItemRect(void)
	{
	}

	// Serialization
	//
	bool VideoItemRect::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectImpl::SaveData(message);
		if (result == false || message->has_videoitem() == false)
		{
			assert(result);
			assert(message->has_videoitem());
			return false;
		}
		
		// --
		//
		Proto::VideoItemRect* rectMessage = message->mutable_videoitem()->mutable_rect();

		rectMessage->set_weight(m_weight);
		rectMessage->set_linecolor(m_lineColor);
		rectMessage->set_fillcolor(m_fillColor);
		rectMessage->set_fill(m_fill);
		rectMessage->set_drawrect(m_drawRect);

		Proto::Write(rectMessage->mutable_text(), m_text);
		rectMessage->set_textcolor(m_textColor);
		m_font.SaveData(rectMessage->mutable_font());

		return true;
	}

	bool VideoItemRect::LoadData(const Proto::Envelope& message)
	{
		if (message.has_videoitem() == false)
		{
			assert(message.has_videoitem());
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
		if (message.videoitem().has_rect() == false)
		{
			assert(message.videoitem().has_rect());
			return false;
		}

		const Proto::VideoItemRect& rectMessage = message.videoitem().rect();

		m_weight = rectMessage.weight();
		m_lineColor = rectMessage.linecolor();
		m_fillColor = rectMessage.fillcolor();
		m_fill = rectMessage.fill();
		m_text = Proto::Read(rectMessage.text());
		m_textColor = rectMessage.textcolor();
		m_drawRect = rectMessage.drawrect();
		m_font.LoadData(rectMessage.font());

		return true;
	}

	// Drawing Functions
	//

	// Рисование элемента, выполняется в 100% масштабе.
	// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
	//
	void VideoItemRect::Draw(CDrawParam* drawParam, const Scheme*, const SchemeLayer*) const
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
			DrawHelper::DrawText(p, m_font, itemUnit(), m_text, r);
		}

		return;
	}

	// Properties and Data
	//
	IMPLEMENT_FONT_PROPERTIES(VideoItemRect, Font, m_font);

	// Weight property
	//
	double VideoItemRect::weight() const
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			return CUtils::RoundDisplayPoint(m_weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(m_weight, SchemeUnit::Inch, CSettings::regionalUnit(), ConvertDirection::Horz);
			return CUtils::RoundPoint(pt, CSettings::regionalUnit());
		}
	}

	void VideoItemRect::setWeight(double weight)
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			m_weight = CUtils::RoundDisplayPoint(weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(weight, CSettings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Horz);
			m_weight = pt;
		}
	}

	// LineColor property
	//
	QRgb VideoItemRect::lineColor() const
	{
		return m_lineColor;
	}
	void VideoItemRect::setLineColor(QRgb color)
	{
		m_lineColor = color;
	}

	// FillColor property
	//
	QRgb VideoItemRect::fillColor() const
	{
		return m_fillColor;
	}
	void VideoItemRect::setFillColor(QRgb color)
	{
		m_fillColor = color;
	}

	// Fill property
	//
	bool VideoItemRect::fill() const
	{
		return m_fill;
	}
	void VideoItemRect::setFill(bool fill)
	{
		m_fill = fill;
	}

	// Text property
	//
	const QString& VideoItemRect::text() const
	{
		return m_text;
	}
	void VideoItemRect::setText(QString& value)
	{
		m_text = value;
	}

	// TextColor property
	//
	QRgb VideoItemRect::textColor() const
	{
		return m_textColor;
	}
	void VideoItemRect::setTextColor(QRgb color)
	{
		m_textColor = color;
	}

	bool VideoItemRect::drawRect() const
	{
		return m_drawRect;
	}

	void VideoItemRect::setDrawRect(bool value)
	{
		m_drawRect = value;
	}

}

