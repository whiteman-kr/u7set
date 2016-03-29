#include "Stable.h"
#include "SchemeItemPath.h"

namespace VFrame30
{
	SchemeItemPath::SchemeItemPath(void) :
		SchemeItemPath(SchemaUnit::Inch)
	{
		// Вызов этого конструктора возможен при сериализации объектов такого типа.
		// После этого вызова надо проинциализировать все, что и делается самой сериализацией.
		//
	}

	SchemeItemPath::SchemeItemPath(SchemaUnit unit) : 
		m_weight(0),
		m_lineColor(qRgb(0x00, 0x00, 0x00))
	{
		ADD_PROPERTY_GETTER_SETTER(double, LineWeight, true, SchemeItemPath::weight, SchemeItemPath::setWeight);

		// --
		//
		m_static = true;
		setItemUnit(unit);
	}


	SchemeItemPath::~SchemeItemPath(void)
	{
	}

	// Serialization
	//
	bool SchemeItemPath::SaveData(Proto::Envelope* message) const
	{
		bool result = PosConnectionImpl::SaveData(message);
		if (result == false || message->has_schemeitem() == false)
		{
			assert(result);
			assert(message->has_schemeitem());
			return false;
		}
	
		// --
		//
		Proto::SchemeItemPath* path = message->mutable_schemeitem()->mutable_path();

		path->set_weight(m_weight);
		path->set_linecolor(m_lineColor);

		return true;
	}

	bool SchemeItemPath::LoadData(const Proto::Envelope& message)
	{
		if (message.has_schemeitem() == false)
		{
			assert(message.has_schemeitem());
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
		if (message.schemeitem().has_path() == false)
		{
			assert(message.schemeitem().has_path());
			return false;
		}

		const Proto::SchemeItemPath& path = message.schemeitem().path();

		m_weight = path.weight();
		m_lineColor = path.linecolor();

		return true;
	}

	// Drawing Functions
	//

	// Рисование элемента, выполняется в 100% масштабе.
	// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
	//
	void SchemeItemPath::Draw(CDrawParam* drawParam, const Schema*, const SchemeLayer*) const
	{
		if (drawParam == nullptr)
		{
			assert(drawParam);
			return;
		}

		QPainter* p = drawParam->painter();

		// Draw the main part
		//
		const std::list<SchemePoint>& poinlist = GetPointList();

		QPolygonF polyline(static_cast<int>(poinlist.size()));
		int index = 0;

		for (auto pt = poinlist.cbegin(); pt != poinlist.cend(); ++pt)
		{
			polyline[index++] = QPointF(pt->X, pt->Y);
		}

		QPen pen(lineColor());
		pen.setWidthF(m_weight);
		p->setPen(pen);

		p->drawPolyline(polyline);

		return;
	}

	// Properties and Data
	//

	// Weight property
	//
	double SchemeItemPath::weight() const
	{
		if (itemUnit() == SchemaUnit::Display)
		{
			return CUtils::RoundDisplayPoint(m_weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(m_weight, SchemaUnit::Inch, Settings::regionalUnit(), ConvertDirection::Horz);
			return CUtils::RoundPoint(pt, Settings::regionalUnit());
		}
	}

	void SchemeItemPath::setWeight(double weight)
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
	QRgb SchemeItemPath::lineColor() const
	{
		return m_lineColor;
	}

	void SchemeItemPath::setLineColor(QRgb color)
	{
		m_lineColor = color;
	}

}

