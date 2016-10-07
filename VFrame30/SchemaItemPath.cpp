#include "Stable.h"
#include "SchemaItemPath.h"

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

		return true;
	}

	// Drawing Functions
	//

	// Рисование элемента, выполняется в 100% масштабе.
	// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
	//
	void SchemaItemPath::Draw(CDrawParam* drawParam, const Schema*, const SchemaLayer*) const
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
			polyline[index++] = QPointF(pt->X, pt->Y);
		}

		QPen pen(lineColor());
		pen.setWidthF(m_weight == 0.0 ? drawParam->cosmeticPenWidth() : m_weight);
		p->setPen(pen);

		p->drawPolyline(polyline);

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
			return CUtils::RoundDisplayPoint(m_weight);
		}
		else
		{
			double pt = CUtils::ConvertPoint(m_weight, SchemaUnit::Inch, Settings::regionalUnit(), ConvertDirection::Horz);
			return CUtils::RoundPoint(pt, Settings::regionalUnit());
		}
	}

	void SchemaItemPath::setWeight(double weight)
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
	QColor SchemaItemPath::lineColor() const
	{
		return m_lineColor;
	}

	void SchemaItemPath::setLineColor(QColor color)
	{
		m_lineColor = color;
	}

}

