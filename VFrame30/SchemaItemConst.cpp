#include "SchemaItemConst.h"

namespace VFrame30
{

	SchemaItemConst::SchemaItemConst() :
		SchemaItemConst(SchemaUnit::Inch)
	{
		// This constructor can be called while serialization
		//
	}

	SchemaItemConst::SchemaItemConst(SchemaUnit unit) :
		FblItemRect(unit)
	{
		auto typeProp = ADD_PROPERTY_GETTER_SETTER(ConstType, PropertyNames::type, true, SchemaItemConst::type, SchemaItemConst::setType);
		auto valIntProp = ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::valueInteger, true, SchemaItemConst::intValue, SchemaItemConst::setIntValue);
		auto valFloatProp = ADD_PROPERTY_GETTER_SETTER(double, PropertyNames::valueFloat, true, SchemaItemConst::floatValue, SchemaItemConst::setFloatValue);
		auto precisionProp = ADD_PROPERTY_GETTER_SETTER(int, PropertyNames::precision, true, SchemaItemConst::precision, SchemaItemConst::setPrecision);

		typeProp->setCategory(PropertyNames::functionalCategory);
		valIntProp->setCategory(PropertyNames::functionalCategory);
		valFloatProp->setCategory(PropertyNames::functionalCategory);
		precisionProp->setCategory(PropertyNames::functionalCategory);

		// --
		//
		addOutput();
	}

	SchemaItemConst::~SchemaItemConst()
	{
	}

	bool SchemaItemConst::SaveData(Proto::Envelope* message) const
	{
		bool result = FblItemRect::SaveData(message);

		if (result == false || message->has_schemaitem() == false)
		{
			assert(result);
			assert(message->has_schemaitem());
			return false;
		}

		// --
		//
		Proto::SchemaItemConst* constitem = message->mutable_schemaitem()->mutable_constitem();

		constitem->set_type(m_type);
		constitem->set_intvalue(m_intValue);
		constitem->set_floatvalue(m_floatValue);
		constitem->set_precision(m_precision);

		return true;
	}

	bool SchemaItemConst::LoadData(const Proto::Envelope& message)
	{
		bool result = FblItemRect::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.schemaitem().has_constitem() == false)
		{
			assert(message.schemaitem().has_constitem() == true);
			return false;
		}

		const Proto::SchemaItemConst& constitem = message.schemaitem().constitem();

		m_type = static_cast<ConstType>(constitem.type());
		m_intValue = constitem.intvalue();
		m_floatValue = constitem.floatvalue();
		m_precision = constitem.precision();

		return true;
	}

	void SchemaItemConst::Draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const
	{
		FblItemRect::Draw(drawParam, schema, layer);

		//--
		//
		QPainter* p = drawParam->painter();

		QRectF r(leftDocPt(), topDocPt(), widthDocPt(), heightDocPt());

		if (std::abs(r.left() - r.right()) < 0.000001)
		{
			r.setRight(r.left() + 0.000001);
		}

		if (std::abs(r.bottom() - r.top()) < 0.000001)
		{
			r.setBottom(r.top() + 0.000001);
		}

		int dpiX = 96;
		QPaintDevice* pPaintDevice = p->device();
		if (pPaintDevice == nullptr)
		{
			assert(pPaintDevice);
			dpiX = 96;
		}
		else
		{
			dpiX = pPaintDevice->logicalDpiX();
		}

		double pinWidth = GetPinWidth(itemUnit(), dpiX);

		if (inputsCount() > 0)
		{
			r.setLeft(r.left() + pinWidth);
		}

		if (outputsCount() > 0)
		{
			r.setRight(r.right() - pinWidth);
		}

		r.setLeft(r.left() + m_font.drawSize() / 4.0);
		r.setRight(r.right() - m_font.drawSize() / 4.0);

		// Draw Signals StrIDs
		//
		QString text = valueToString();

		p->setPen(textColor());

		DrawHelper::DrawText(p, m_font, itemUnit(), text, r, Qt::AlignLeft | Qt::AlignTop);

		return;
	}

	QString SchemaItemConst::valueToString() const
	{
		QString text;

		switch (type())
		{
			case ConstType::IntegerlType:
				text = QString::number(intValue());
				break;
			case ConstType::FloatType:
				text = QString::number(floatValue(), 'f', precision());
				break;
			default:
				assert(false);
				break;
		}

		return text;
	}

	QString SchemaItemConst::buildName() const
	{
		return QString("Const (%1)").arg(valueToString());
	}

	SchemaItemConst::ConstType SchemaItemConst::type() const
	{
		return m_type;
	}

	void SchemaItemConst::setType(SchemaItemConst::ConstType value)
	{
		m_type = value;
	}

	bool SchemaItemConst::isIntegral() const
	{
		return m_type == SchemaItemConst::ConstType::IntegerlType;
	}

	bool SchemaItemConst::isFloat() const
	{
		return m_type == SchemaItemConst::ConstType::FloatType;
	}

	int SchemaItemConst::intValue() const
	{
		return m_intValue;
	}

	void SchemaItemConst::setIntValue(int intValue)
	{
		m_intValue = intValue;
	}
	double SchemaItemConst::floatValue() const
	{
		return m_floatValue;
	}

	void SchemaItemConst::setFloatValue(double doubleValue)
	{
		m_floatValue = doubleValue;
	}

	int SchemaItemConst::precision() const
	{
		return m_precision;
	}

	void SchemaItemConst::setPrecision(int value)
	{
		if (value < 0)
		{
			value = 0;
		}

		if (value > 24)
		{
			value = 24;
		}

		m_precision = value;
	}

}
