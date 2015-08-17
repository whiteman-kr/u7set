#include "Stable.h"
#include "SchemeItemRect.h"

namespace VFrame30
{
	SchemeItemRect::SchemeItemRect(void)
	{
		// ����� ����� ������������ �������� ��� ������������ �������� ������ ����.
		// ����� ����� ������ ���� ������������������ ���, ��� � �������� ����� �������������.
		//
	}

	SchemeItemRect::SchemeItemRect(SchemeUnit unit) : 
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

	SchemeItemRect::~SchemeItemRect(void)
	{
	}

	// Serialization
	//
	bool SchemeItemRect::SaveData(Proto::Envelope* message) const
	{
		bool result = PosRectImpl::SaveData(message);
		if (result == false || message->has_schemeitem() == false)
		{
			assert(result);
			assert(message->has_schemeitem());
			return false;
		}
		
		// --
		//
		Proto::SchemeItemRect* rectMessage = message->mutable_schemeitem()->mutable_rect();

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

	bool SchemeItemRect::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemeitem() == false)
		{
			assert(message.has_schemeitem());
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
		if (message.schemeitem().has_rect() == false)
		{
			assert(message.schemeitem().has_rect());
			return false;
		}

		const Proto::SchemeItemRect& rectMessage = message.schemeitem().rect();

		m_weight = rectMessage.weight();
		m_lineColor = rectMessage.linecolor();
		m_fillColor = rectMessage.fillcolor();
		m_fill = rectMessage.fill();
		Proto::Read(rectMessage.text(), &m_text);
		m_textColor = rectMessage.textcolor();
		m_drawRect = rectMessage.drawrect();
		m_font.LoadData(rectMessage.font());

		return true;
	}

	// Drawing Functions
	//

	// ��������� ��������, ����������� � 100% ��������.
	// Graphcis ������ ����� �������� ������������ ������� (0, 0 - ����� ������� ����, ���� � ������ - ������������� ����������)
	//
	void SchemeItemRect::Draw(CDrawParam* drawParam, const Scheme*, const SchemeLayer*) const
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
			p->fillRect(r, *m_fillBrush);		// 22% ���� ������������ Qcolor � ������� ������ ���� ������������ ������� Brush

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
	IMPLEMENT_FONT_PROPERTIES(SchemeItemRect, Font, m_font);

	// Weight property
	//
	double SchemeItemRect::weight() const
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			return CUtils::RoundDisplayPoint(m_weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(m_weight, SchemeUnit::Inch, Settings::regionalUnit(), ConvertDirection::Horz);
			return CUtils::RoundPoint(pt, Settings::regionalUnit());
		}
	}

	void SchemeItemRect::setWeight(double weight)
	{
		if (itemUnit() == SchemeUnit::Display)
		{
			m_weight = CUtils::RoundDisplayPoint(weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(weight, Settings::regionalUnit(), SchemeUnit::Inch, ConvertDirection::Horz);
			m_weight = pt;
		}
	}

	// LineColor property
	//
	QRgb SchemeItemRect::lineColor() const
	{
		return m_lineColor;
	}
	void SchemeItemRect::setLineColor(QRgb color)
	{
		m_lineColor = color;
	}

	// FillColor property
	//
	QRgb SchemeItemRect::fillColor() const
	{
		return m_fillColor;
	}
	void SchemeItemRect::setFillColor(QRgb color)
	{
		m_fillColor = color;
	}

	// Fill property
	//
	bool SchemeItemRect::fill() const
	{
		return m_fill;
	}
	void SchemeItemRect::setFill(bool fill)
	{
		m_fill = fill;
	}

	// Text property
	//
	const QString& SchemeItemRect::text() const
	{
		return m_text;
	}
	void SchemeItemRect::setText(QString& value)
	{
		m_text = value;
	}

	// TextColor property
	//
	QRgb SchemeItemRect::textColor() const
	{
		return m_textColor;
	}
	void SchemeItemRect::setTextColor(QRgb color)
	{
		m_textColor = color;
	}

	bool SchemeItemRect::drawRect() const
	{
		return m_drawRect;
	}

	void SchemeItemRect::setDrawRect(bool value)
	{
		m_drawRect = value;
	}

}
