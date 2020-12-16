#include "SchemaItemLine.h"
#include "PropertyNames.h"
#include "DrawParam.h"

namespace VFrame30
{
	SchemaItemLine::SchemaItemLine(void) :
		SchemaItemLine(SchemaUnit::Inch)
	{
		// Serialization can call this constructor. All members must be initialized after that
		// Actually it the task of serialization
		//
	}

	SchemaItemLine::SchemaItemLine(SchemaUnit unit) :
		m_weight(0),
		m_lineColor(qRgb(0x00, 0x00, 0x00))
	{
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::lineWeight, PropertyNames::appearanceCategory, true, SchemaItemLine::weight, SchemaItemLine::setWeight);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::lineColor, PropertyNames::appearanceCategory, true, SchemaItemLine::lineColor, SchemaItemLine::setLineColor);

		ADD_PROPERTY_GET_SET_CAT(E::LineStyle, PropertyNames::lineStyle, PropertyNames::appearanceCategory, true, SchemaItemLine::lineStyle, SchemaItemLine::setLineStyle);
		ADD_PROPERTY_GET_SET_CAT(E::LineStyleCap, PropertyNames::lineStyleCap, PropertyNames::appearanceCategory, true, SchemaItemLine::lineStyleCap, SchemaItemLine::setLineStyleCap);

		ADD_PROPERTY_GET_SET_CAT(E::LineCap, PropertyNames::lineCapStart, PropertyNames::appearanceCategory, true, SchemaItemLine::lineCapStart, SchemaItemLine::setLineCapStart);
		ADD_PROPERTY_GET_SET_CAT(E::LineCap, PropertyNames::lineCapEnd, PropertyNames::appearanceCategory, true, SchemaItemLine::lineCapEnd, SchemaItemLine::setLineCapEnd);
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::lineCapFactor, PropertyNames::appearanceCategory, true, SchemaItemLine::lineCapFactor, SchemaItemLine::setLineCapFactor);

		// --
		//
		m_static = true;
		setItemUnit(unit);
	}


	SchemaItemLine::~SchemaItemLine(void)
	{
	}

	// Serialization
	//
	bool SchemaItemLine::SaveData(Proto::Envelope* message) const
	{
		bool result = PosLineImpl::SaveData(message);
		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}

		// --
		//
		Proto::SchemaItemLine* lineMessage = message->mutable_schemaitem()->mutable_line();

		lineMessage->set_weight(m_weight);
		lineMessage->set_linecolor(m_lineColor.rgba());

		lineMessage->set_linestyle(static_cast<int>(m_lineStyle));
		lineMessage->set_linestylecap(static_cast<int>(m_lineStyleCap));

		lineMessage->set_linecapstart(static_cast<int>(m_lineCapStart));
		lineMessage->set_linecapend(static_cast<int>(m_lineCapEnd));
		lineMessage->set_linecapfactor(m_lineCapFactor);

		return true;
	}

	bool SchemaItemLine::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return false;
		}

		// --
		//
		bool result = PosLineImpl::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		if (message.schemaitem().has_line() == false)
		{
			assert(message.schemaitem().has_line());
			return false;
		}

		const Proto::SchemaItemLine& lineMessage = message.schemaitem().line();

		m_weight = lineMessage.weight();
		m_lineColor = QColor::fromRgba(lineMessage.linecolor());

		m_lineStyle = static_cast<E::LineStyle>(lineMessage.linestyle());
		m_lineStyleCap = static_cast<E::LineStyleCap>(lineMessage.linestylecap());

		m_lineCapStart = static_cast<E::LineCap>(lineMessage.linecapstart());
		m_lineCapEnd = static_cast<E::LineCap>(lineMessage.linecapend());
		m_lineCapFactor = lineMessage.linecapfactor();

		return true;
	}

	// Drawing Functions
	//

	// Item is drawn in 100% scale
	// Graphcis must have scrren coordinate system (0, 0 - left upper corner, down and right - positive pos)
	//
	void SchemaItemLine::draw(CDrawParam* drawParam, const Schema*, const SchemaLayer*) const
	{
		if (drawParam == nullptr)
		{
			assert(drawParam);
			return;
		}

		QPainter* p = drawParam->painter();

		QPointF p1 = drawParam->gridToDpi(startXDocPt(), startYDocPt());
		QPointF p2 = drawParam->gridToDpi(endXDocPt(), endYDocPt());

		if (std::abs(p1.x() - p2.x()) < 0.000001 && std::abs(p1.y() - p2.y()) < 0.000001)
		{
			// Empty line is drawn very big
			//
			return;
		}

		const double lineWeight = m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight;

		QPen pen(m_lineColor);
		pen.setWidthF(lineWeight);
		pen.setStyle(static_cast<Qt::PenStyle>(m_lineStyle));
		pen.setCapStyle(static_cast<Qt::PenCapStyle>(m_lineStyleCap));

		p->setPen(pen);

		bool al = p->testRenderHint(QPainter::Antialiasing);	// Save antialising
		p->setRenderHint(QPainter::Antialiasing);

		p->drawLine(p1, p2);

		if (m_lineCapStart != E::LineCap::NoCap)
		{
			const double angleRadStart = std::atan2(p2.y() - p1.y(), p2.x() - p1.x());

			p->setPen(Qt::NoPen);
			p->setBrush(m_lineColor);

			SchemaItemLine::drawLineCap(p, itemUnit(), p1, angleRadStart, lineWeight, m_lineCapStart, m_lineCapFactor);
		}

		if (m_lineCapEnd != E::LineCap::NoCap)
		{
			const double angleRadEnd = std::atan2(p1.y() - p2.y(), p1.x() - p2.x());

			p->setPen(Qt::NoPen);
			p->setBrush(m_lineColor);

			SchemaItemLine::drawLineCap(p, itemUnit(), p2, angleRadEnd, lineWeight, m_lineCapEnd, m_lineCapFactor);
		}

		p->setRenderHint(QPainter::Antialiasing, al);			// Restore antialising

		return;
	}

	// Draw line cap, Pen and Brush MUST be already selected in the Painter
	//
	void SchemaItemLine::drawLineCap(QPainter* painter, SchemaUnit units, const QPointF& pos, double angleRad, double lineWeight, E::LineCap capStyle, double factor)
	{
		if (painter == nullptr)
		{
			Q_ASSERT(painter);
			return;
		}

		const double x = pos.x();
		const double y = pos.y();

		lineWeight = (units == SchemaUnit::Display) ?
						 std::max(1.0, lineWeight) :
						 std::max((1.0 / painter->device()->logicalDpiY()), lineWeight);

		double capHeight = lineWeight * 2.0 * factor;
		capHeight = (units == SchemaUnit::Display) ?
						std::max(3.0, capHeight) :
						std::max(mm2in(0.3), capHeight);

		const double capHeightHalf = capHeight / 2.0;
		const double capHeightThird = capHeight / 3.0;
		const double capHeightTwoThird = capHeight * 2.0 / 3.0;

		switch (capStyle)
		{
		case E::LineCap::NoCap:
			break;
		case E::LineCap::BarCap:
			{
				QPolygonF pol{QRectF(x - capHeightHalf,
									 y - capHeightHalf,
									 capHeight,
									 capHeight)};

				QTransform t;
				t.translate(x, y);
				t.rotateRadians(angleRad);
				t.translate(-x, -y);

				painter->drawConvexPolygon(t.map(pol));
			}
			break;
		case E::LineCap::CircleCap:
			{
				painter->drawEllipse(pos, capHeightHalf, capHeightHalf);
			}
			break;
		case E::LineCap::Arrow1Cap:
			{
				QVector<QPointF> pol = {{pos},
										{x - capHeightHalf, y + capHeightThird},
										{x, y - capHeightTwoThird},
										{x + capHeightHalf, y + capHeightThird}};

				QTransform t;
				t.translate(x, y);
				t.rotateRadians(angleRad - M_PI_2);	// -90 degrees
				t.translate(-x, -y);

				painter->drawConvexPolygon(t.map(pol));
			}
			break;
		case E::LineCap::Arrow2Cap:
			{
				QVector<QPointF> pol = {{x - capHeightHalf, y + capHeightThird},
										{x, y - capHeightTwoThird},
										{x + capHeightHalf, y + capHeightThird}};

				QTransform t;
				t.translate(x, y);
				t.rotateRadians(angleRad - M_PI_2);	// -90 degrees
				t.translate(-x, -y);

				painter->drawConvexPolygon(t.map(pol));
			}
			break;
		}

		return;
	}

	// Properties and Data
	//

	// Weight propertie
	//
	double SchemaItemLine::weight() const
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			return CUtils::RoundDisplayPoint(m_weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(m_weight, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			return CUtils::RoundPoint(pt, Settings::regionalUnit());
		}
	}

	void SchemaItemLine::setWeight(double weight)
	{
		if (weight < 0)
		{
			weight = 0;
		}

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

	// LineColor propertie
	//
	QColor SchemaItemLine::lineColor() const
	{
		return m_lineColor;
	}

	void SchemaItemLine::setLineColor(QColor color)
	{
		m_lineColor = color;
	}

	E::LineStyle SchemaItemLine::lineStyle() const
	{
		return m_lineStyle;
	}

	void SchemaItemLine::setLineStyle(E::LineStyle value)
	{
		m_lineStyle = value;
	}

	E::LineStyleCap SchemaItemLine::lineStyleCap() const
	{
		return m_lineStyleCap;
	}

	void SchemaItemLine::setLineStyleCap(E::LineStyleCap value)
	{
		m_lineStyleCap = value;
	}

	E::LineCap SchemaItemLine::lineCapStart() const
	{
		return m_lineCapStart;
	}

	void SchemaItemLine::setLineCapStart(E::LineCap value)
	{
		m_lineCapStart = value;
	}

	E::LineCap SchemaItemLine::lineCapEnd() const
	{
		return m_lineCapEnd;
	}

	void SchemaItemLine::setLineCapEnd(E::LineCap value)
	{
		m_lineCapEnd = value;
	}

	double SchemaItemLine::lineCapFactor() const
	{
		return m_lineCapFactor;
	}

	void SchemaItemLine::setLineCapFactor(double value)
	{
		m_lineCapFactor = value;
	}

}

