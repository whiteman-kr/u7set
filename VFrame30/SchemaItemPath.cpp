#include "SchemaItemPath.h"
#include "PropertyNames.h"
#include "DrawParam.h"
#include "SchemaItemLine.h"

namespace VFrame30
{
	SchemaItemPath::SchemaItemPath(void) :
		SchemaItemPath(SchemaUnit::Inch)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemaItemPath::SchemaItemPath(SchemaUnit unit) :
		m_weight(0),
		m_lineColor(qRgb(0x00, 0x00, 0x00))
	{
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::lineWeight, PropertyNames::appearanceCategory, true, SchemaItemPath::weight, SchemaItemPath::setWeight);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::lineColor, PropertyNames::appearanceCategory, true, SchemaItemPath::lineColor, SchemaItemPath::setLineColor);

		ADD_PROPERTY_GET_SET_CAT(E::LineStyle, PropertyNames::lineStyle, PropertyNames::appearanceCategory, true, SchemaItemPath::lineStyle, SchemaItemPath::setLineStyle);
		ADD_PROPERTY_GET_SET_CAT(E::LineStyleCap, PropertyNames::lineStyleCap, PropertyNames::appearanceCategory, true, SchemaItemPath::lineStyleCap, SchemaItemPath::setLineStyleCap);

		ADD_PROPERTY_GET_SET_CAT(E::LineCap, PropertyNames::lineCapStart, PropertyNames::appearanceCategory, true, SchemaItemPath::lineCapStart, SchemaItemPath::setLineCapStart);
		ADD_PROPERTY_GET_SET_CAT(E::LineCap, PropertyNames::lineCapEnd, PropertyNames::appearanceCategory, true, SchemaItemPath::lineCapEnd, SchemaItemPath::setLineCapEnd);
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::lineCapFactor, PropertyNames::appearanceCategory, true, SchemaItemPath::lineCapFactor, SchemaItemPath::setLineCapFactor);

		// --
		//
		m_static = true;
		setItemUnit(unit);
	}


	SchemaItemPath::~SchemaItemPath(void)
	{
	}

	// Serialization
	//
	bool SchemaItemPath::SaveData(Proto::Envelope* message) const
	{
		bool result = PosConnectionImpl::SaveData(message);
		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}
	
		// --
		//
		Proto::SchemaItemPath* path = message->mutable_schemaitem()->mutable_path();

		path->set_weight(m_weight);
		path->set_linecolor(m_lineColor.rgba());

		path->set_linestyle(static_cast<int>(m_lineStyle));
		path->set_linestylecap(static_cast<int>(m_lineStyleCap));

		path->set_linecapstart(static_cast<int>(m_lineCapStart));
		path->set_linecapend(static_cast<int>(m_lineCapEnd));
		path->set_linecapfactor(m_lineCapFactor);

		return true;
	}

	bool SchemaItemPath::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemaitem() == false)
		{
			assert(message.has_schemaitem());
			return false;
		}

		// --
		//
		bool result = PosConnectionImpl::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_path() == false)
		{
			assert(message.schemaitem().has_path());
			return false;
		}

		const Proto::SchemaItemPath& path = message.schemaitem().path();

		m_weight = path.weight();
		m_lineColor = QColor::fromRgba(path.linecolor());

		m_lineStyle = static_cast<E::LineStyle>(path.linestyle());
		m_lineStyleCap = static_cast<E::LineStyleCap>(path.linestylecap());

		m_lineCapStart = static_cast<E::LineCap>(path.linecapstart());
		m_lineCapEnd = static_cast<E::LineCap>(path.linecapend());
		m_lineCapFactor = path.linecapfactor();

		return true;
	}

	// Drawing Functions
	//

	// Рисование элемента, выполняется в 100% масштабе.
	// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
	//
	void SchemaItemPath::draw(CDrawParam* drawParam, const Schema*, const SchemaLayer*) const
	{
		if (drawParam == nullptr)
		{
			assert(drawParam);
			return;
		}

		QPainter* p = drawParam->painter();

		// Draw the main part
		//
		const std::list<SchemaPoint>& poinlist = GetPointList();

		QPolygonF polyline(static_cast<int>(poinlist.size()));
		int index = 0;

		for (auto pt = poinlist.cbegin(); pt != poinlist.cend(); ++pt)
		{
			polyline[index++] = drawParam->gridToDpi(pt->X, pt->Y);
		}

		QPen pen(lineColor());
		pen.setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);
		pen.setStyle(static_cast<Qt::PenStyle>(m_lineStyle));
		pen.setCapStyle(static_cast<Qt::PenCapStyle>(m_lineStyleCap));

		p->setPen(pen);
		p->drawPolyline(polyline);

		// Set antializasing for drawing line caps
		//
		bool al = p->testRenderHint(QPainter::Antialiasing);	// Save antialising
		p->setRenderHint(QPainter::Antialiasing);

		if (m_lineCapStart != E::LineCap::NoCap && poinlist.size() > 1)
		{
			QPointF p1{*poinlist.begin()};
			QPointF p2{*(std::next(poinlist.begin()))};

			const double angleRadStart = std::atan2(p2.y() - p1.y(), p2.x() - p1.x());

			p->setPen(Qt::NoPen);
			p->setBrush(m_lineColor);

			SchemaItemLine::drawLineCap(p, itemUnit(), p1, angleRadStart, m_weight, m_lineCapStart, m_lineCapFactor);
		}

		if (m_lineCapEnd != E::LineCap::NoCap && poinlist.size() > 1)
		{
			QPointF p1{*(std::next(poinlist.rbegin()))};
			QPointF p2{*poinlist.rbegin()};

			const double angleRadEnd = std::atan2(p1.y() - p2.y(), p1.x() - p2.x());

			p->setPen(Qt::NoPen);
			p->setBrush(m_lineColor);

			SchemaItemLine::drawLineCap(p, itemUnit(), p2, angleRadEnd, m_weight, m_lineCapEnd, m_lineCapFactor);
		}

		p->setRenderHint(QPainter::Antialiasing, al);			// Restore antialising

		return;
	}

	// Properties and Data
	//

	// Weight property
	//
	double SchemaItemPath::weight() const
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			return VFrame30::RoundDisplayPoint(m_weight);
		}
		else
		{
			double pt = VFrame30::ConvertPoint(m_weight, SchemaUnit::Inch, Settings::regionalUnit(), 0);
			return VFrame30::RoundPoint(pt, Settings::regionalUnit());
		}
	}

	void SchemaItemPath::setWeight(double weight)
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
	QColor SchemaItemPath::lineColor() const
	{
		return m_lineColor;
	}

	void SchemaItemPath::setLineColor(QColor color)
	{
		m_lineColor = color;
	}

	E::LineStyle SchemaItemPath::lineStyle() const
	{
		return m_lineStyle;
	}

	void SchemaItemPath::setLineStyle(E::LineStyle value)
	{
		m_lineStyle = value;
	}

	E::LineStyleCap SchemaItemPath::lineStyleCap() const
	{
		return m_lineStyleCap;
	}

	void SchemaItemPath::setLineStyleCap(E::LineStyleCap value)
	{
		m_lineStyleCap = value;
	}

	E::LineCap SchemaItemPath::lineCapStart() const
	{
		return m_lineCapStart;
	}

	void SchemaItemPath::setLineCapStart(E::LineCap value)
	{
		m_lineCapStart = value;
	}

	E::LineCap SchemaItemPath::lineCapEnd() const
	{
		return m_lineCapEnd;
	}

	void SchemaItemPath::setLineCapEnd(E::LineCap value)
	{
		m_lineCapEnd = value;
	}

	double SchemaItemPath::lineCapFactor() const
	{
		return m_lineCapFactor;
	}

	void SchemaItemPath::setLineCapFactor(double value)
	{
		m_lineCapFactor = value;
	}


}

