#include <VFrame30/DrawParam.h>
#include <VFrame30/PropertyNames.h>
#include <VFrame30/SchemaItemVduLine.h>
#include <VFrame30/Settings.h>

namespace VFrame30
{
	SchemaItemVduLine::SchemaItemVduLine(void) :
		SchemaItemVduLine(SchemaUnit::Display)
	{
		// Serialization can call this constructor. All members must be initialized after that
		// Actually it the task of serialization
		//
	}

	SchemaItemVduLine::SchemaItemVduLine(SchemaUnit unit) :
		m_weight(1),
		m_lineColor(qRgb(0x00, 0x00, 0x00))
	{
		assert(unit == SchemaUnit::Display);

		ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::lineWeight, PropertyNames::appearanceCategory, true, SchemaItemVduLine::weight, SchemaItemVduLine::setWeight);
		ADD_PROPERTY_GET_SET_CAT(QColor, PropertyNames::lineColor, PropertyNames::appearanceCategory, true, SchemaItemVduLine::lineColor, SchemaItemVduLine::setLineColor);

		// --
		//
		m_static = true;
		setItemUnit(unit);

		return;
	}

	// Serialization
	//
	bool SchemaItemVduLine::SaveData(Proto::Envelope* message) const
	{
		bool result = PosLineImpl::SaveData(message);
		if (result == false || message->HasExtension(Proto::schemaitem) == false)
		{
			assert(result);
			assert(message->HasExtension(Proto::schemaitem));
			return false;
		}

		// --
		//
		auto lineMessage = message->MutableExtension(Proto::schemaitem)->mutable_vduline();

		lineMessage->set_weight(m_weight);
		lineMessage->set_linecolor(m_lineColor.rgba());

		return true;
	}

	bool SchemaItemVduLine::LoadData(const Proto::Envelope& message)
	{
		if (message.HasExtension(Proto::schemaitem) == false)
		{
			assert(message.HasExtension(Proto::schemaitem));
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
		//
		const auto& schemaItemMessage = message.GetExtension(Proto::schemaitem);
		if (schemaItemMessage.has_vduline() == false)
		{
			assert(schemaItemMessage.has_vduline());
			return false;
		}

		const auto& lineMessage = schemaItemMessage.vduline();

		m_weight = lineMessage.weight();
		m_lineColor = QColor::fromRgba(lineMessage.linecolor());

		return true;
	}

	void SchemaItemVduLine::draw(CDrawParam* drawParam) const
	{
		if (drawParam == nullptr)
		{
			assert(drawParam);
			return;
		}

		QPainter* painter = drawParam->painter();

		QPointF p1 = drawParam->gridToDpi(startXDocPt(), startYDocPt());
		QPointF p2 = drawParam->gridToDpi(endXDocPt(), endYDocPt());

		if (std::abs(p1.x() - p2.x()) < 0.000001 && std::abs(p1.y() - p2.y()) < 0.000001)
		{
			// Empty line is drawn very big
			//
			return;
		}

		const int lineWeight = m_weight <= 0 ? drawParam->cosmeticPenWidth() : m_weight;

		QPen pen(m_lineColor);
		pen.setWidth(lineWeight);
		painter->setPen(pen);

		bool al = painter->testRenderHint(QPainter::Antialiasing);	// Save antialiasing
		painter->setRenderHint(QPainter::Antialiasing);

		painter->drawLine(p1, p2);

		painter->setRenderHint(QPainter::Antialiasing, al);			// Restore antialiasing

		return;
	}

	bool SchemaItemVduLine::accept(VduItemVisitor& visitor) const
	{
		return visitor.visit(*this);
	}

	int SchemaItemVduLine::weight() const
	{
		return m_weight;
	}

	void SchemaItemVduLine::setWeight(int weight)
	{
		m_weight = std::clamp(weight, 1, 16);
	}

	QColor SchemaItemVduLine::lineColor() const
	{
		return m_lineColor;
	}

	void SchemaItemVduLine::setLineColor(QColor color)
	{
		m_lineColor = color;
	}
}

