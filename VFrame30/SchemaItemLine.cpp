#include "Stable.h"
#include "SchemaItemLine.h"

namespace VFrame30
{
	SchemaItemLine::SchemaItemLine(void) :
		SchemaItemLine(SchemaUnit::Inch)
	{
		// ����� ����� ������������ �������� ��� ������������ �������� ������ ����.
		// ����� ����� ������ ���� ������������������ ���, ��� � �������� ����� �������������.
		//
	}

	SchemaItemLine::SchemaItemLine(SchemaUnit unit) :
		m_weight(0),
		m_lineColor(qRgb(0x00, 0x00, 0x00))
	{
		ADD_PROPERTY_GETTER_SETTER(double, LineWeight, true, SchemaItemLine::weight, SchemaItemLine::setWeight);

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
		lineMessage->set_linecolor(m_lineColor);

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
		m_lineColor = lineMessage.linecolor();

		return true;
	}

	// Drawing Functions
	//

	// ��������� ��������, ����������� � 100% ��������.
	// Graphcis ������ ����� �������� ������������ ������� (0, 0 - ����� ������� ����, ���� � ������ - ������������� ����������)
	//
	void SchemaItemLine::Draw(CDrawParam* drawParam, const Schema*, const SchemaLayer*) const
	{
		if (drawParam == nullptr)
		{
			assert(drawParam);
			return;
		}

		QPainter* p = drawParam->painter();

		QPointF p1(startXDocPt(), startYDocPt());
		QPointF p2(endXDocPt(), endYDocPt());

		if (std::abs(p1.x() - p2.x()) < 0.000001 && std::abs(p1.y() - p2.y()) < 0.000001)
		{
			// ������ �����, �������� ����� �������
			//
			return;
		}

		QPen pen(lineColor());
		pen.setWidthF(m_weight);
		p->setPen(pen);

		p->drawLine(p1, p2);

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
			double pt = CUtils::ConvertPoint(m_weight, SchemaUnit::Inch, Settings::regionalUnit(), ConvertDirection::Horz);
			return CUtils::RoundPoint(pt, Settings::regionalUnit());
		}
	}

	void SchemaItemLine::setWeight(double weight)
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

	// LineColor propertie
	//
	QRgb SchemaItemLine::lineColor() const
	{
		return m_lineColor;
	}

	void SchemaItemLine::setLineColor(QRgb color)
	{
		m_lineColor = color;
	}

}
