#include "SchemaItemConst.h"
#include "PropertyNames.h"
#include "DrawParam.h"

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
		ADD_PROPERTY_GET_SET_CAT(ConstType, PropertyNames::type, PropertyNames::constCategory, true, SchemaItemConst::type, SchemaItemConst::setType);
		ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::valueInteger, PropertyNames::constCategory, true, SchemaItemConst::intValue, SchemaItemConst::setIntValue);
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::valueFloat, PropertyNames::constCategory, true, SchemaItemConst::floatValue, SchemaItemConst::setFloatValue);
		ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::precision, PropertyNames::constCategory, true, SchemaItemConst::precision, SchemaItemConst::setPrecision);

		ADD_PROPERTY_GET_SET_CAT(E::HorzAlign, PropertyNames::alignHorz, PropertyNames::textCategory, true, SchemaItemConst::horzAlign, SchemaItemConst::setHorzAlign);
		ADD_PROPERTY_GET_SET_CAT(E::VertAlign, PropertyNames::alignVert, PropertyNames::textCategory, true, SchemaItemConst::vertAlign, SchemaItemConst::setVertAlign);

		setPrecision(precision());		// This function will set Presion for valueFloat property

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

		constitem->set_horzalign(static_cast<int32_t>(m_horzAlign));
		constitem->set_vertalign(static_cast<int32_t>(m_vertAlign));

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

		setType(static_cast<ConstType>(constitem.type()));		// Value properties created here

		m_intValue = constitem.intvalue();
		m_floatValue = constitem.floatvalue();
		m_precision = constitem.precision();

		setPrecision(m_precision);		// This function will set Presion for valueFloat property

		m_horzAlign = static_cast<E::HorzAlign>(constitem.horzalign());
		m_vertAlign = static_cast<E::VertAlign>(constitem.vertalign());

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

		int dpiX = drawParam->dpiX();
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

		DrawHelper::drawText(p, m_font, itemUnit(), text, r, horzAlign() | vertAlign());

		return;
	}

	double SchemaItemConst::minimumPossibleWidthDocPt(double gridSize, int pinGridStep) const
	{
		// Cache values
		//
		m_cachedGridSize = gridSize;
		m_cachedPinGridStep = pinGridStep;

		// --
		//
		return m_cachedGridSize * 7;
	}

	QString SchemaItemConst::valueToString() const
	{
		QString text;

		switch (type())
		{
			case ConstType::IntegerType:
				text = QString("int: %1").arg(QString::number(intValue()));
				break;
			case ConstType::FloatType:
				text = QString("float: %1").arg(QString::number(floatValue(), 'g', precision()));
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

	QString SchemaItemConst::toolTipText(int dpiX, int dpiY) const
	{
		Q_UNUSED(dpiX);
		Q_UNUSED(dpiY);

		QString typeStr;
		switch (type())
		{
			case ConstType::IntegerType:
				typeStr = "Integer";
				break;
			case ConstType::FloatType:
				typeStr = "Float";
				break;
			default:
				break;
		}

		QString str = QString("Constant:\n\tType: %1\n\tValue: %2")
							.arg(typeStr)
							.arg(valueToString());

		str.append(tr("\n\nHint: Press F2 to edit value"));

		return str;
	}

	SchemaItemConst::ConstType SchemaItemConst::type() const
	{
		return m_type;
	}

	void SchemaItemConst::setType(SchemaItemConst::ConstType value)
	{
		m_type = value;

		switch (m_type)
		{
			case ConstType::IntegerType:
				propertyByCaption(PropertyNames::valueInteger)->setVisible(true);
				propertyByCaption(PropertyNames::valueFloat)->setVisible(false);
				propertyByCaption(PropertyNames::precision)->setVisible(false);
				emit propertyListChanged();		// Explicit emmiting signal, as setVisible does not do it
				break;
			case ConstType::FloatType:
				propertyByCaption(PropertyNames::valueInteger)->setVisible(false);
				propertyByCaption(PropertyNames::valueFloat)->setVisible(true);
				propertyByCaption(PropertyNames::precision)->setVisible(true);
				emit propertyListChanged();		// Explicit emmiting signal, as setVisible does not do it
				break;
			default:
				assert(false);
				break;
		}

		return;
	}

	bool SchemaItemConst::isIntegral() const
	{
		return m_type == SchemaItemConst::ConstType::IntegerType;
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

		// Set precision to m_floatValue property
		//
		std::shared_ptr<Property> prop = propertyByCaption(PropertyNames::valueFloat);

		if (prop == nullptr)
		{
			assert(prop);
			return;
		}

		prop->setPrecision(m_precision);
	}

	// Align propertis
	//
	E::HorzAlign SchemaItemConst::horzAlign() const
	{
		return m_horzAlign;
	}
	void SchemaItemConst::setHorzAlign(E::HorzAlign align)
	{
		m_horzAlign = align;
	}

	E::VertAlign SchemaItemConst::vertAlign() const
	{
		return m_vertAlign;
	}

	void SchemaItemConst::setVertAlign(E::VertAlign align)
	{
		m_vertAlign = align;
	}

}
