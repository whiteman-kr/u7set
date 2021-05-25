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

		ADD_PROPERTY_GET_SET_CAT(Afb::AfbParamValue, PropertyNames::valueInteger, PropertyNames::constCategory, true, SchemaItemConst::signedInt32Value, SchemaItemConst::setSignedInt32Value);
		ADD_PROPERTY_GET_SET_CAT(Afb::AfbParamValue, PropertyNames::valueFloat, PropertyNames::constCategory, true, SchemaItemConst::floatValue, SchemaItemConst::setFloatValue);
		ADD_PROPERTY_GET_SET_CAT(Afb::AfbParamValue, PropertyNames::valueDiscrete, PropertyNames::constCategory, true, SchemaItemConst::discreteValue, SchemaItemConst::setDiscreteValue);

		ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::precision, PropertyNames::constCategory, true, SchemaItemConst::precision, SchemaItemConst::setPrecision);
		ADD_PROPERTY_GET_SET_CAT(E::AnalogFormat, PropertyNames::analogFormat, PropertyNames::constCategory, true, SchemaItemConst::analogFormat, SchemaItemConst::setAnalogFormat);

		ADD_PROPERTY_GET_SET_CAT(E::HorzAlign, PropertyNames::alignHorz, PropertyNames::textCategory, true, SchemaItemConst::horzAlign, SchemaItemConst::setHorzAlign);
		ADD_PROPERTY_GET_SET_CAT(E::VertAlign, PropertyNames::alignVert, PropertyNames::textCategory, true, SchemaItemConst::vertAlign, SchemaItemConst::setVertAlign);

		setPrecision(precision());		// This function will set Precision for valueFloat property for dispaying in property editor

		// --
		//
		addOutput();

		return;
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

		// --
		//
		constitem->set_intvalue(m_value.signedInt32.value().toInt());
		constitem->set_intref(m_value.signedInt32.reference().toStdString());

		constitem->set_floatvalue(m_value.float32.value().toFloat());
		constitem->set_floatref(m_value.float32.reference().toStdString());

		constitem->set_discretevalue(m_value.discrete.value().toUInt());
		constitem->set_discreteref(m_value.discrete.reference().toStdString());

		// --
		//
		constitem->set_precision(m_precision);
		constitem->set_analogformat(static_cast<int32_t>(m_analogFormat));

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

		// --
		//
		m_value.signedInt32.setValue(constitem.intvalue());
		m_value.signedInt32.setReference(QString::fromStdString(constitem.intref()));

		m_value.float32.setValue(constitem.has_floatvalue_obsolete() ?
									 static_cast<float>(constitem.floatvalue_obsolete()) :
									 constitem.floatvalue());
		m_value.float32.setReference(QString::fromStdString(constitem.floatref()));

		m_value.discrete.setValue(static_cast<quint16>(constitem.discretevalue()));
		m_value.discrete.setReference(QString::fromStdString(constitem.discreteref()));

		// --
		//
		setPrecision(m_precision);			// This function will set Precision for valueFloat property
		m_analogFormat = static_cast<E::AnalogFormat>(constitem.analogformat());

		m_horzAlign = static_cast<E::HorzAlign>(constitem.horzalign());
		m_vertAlign = static_cast<E::VertAlign>(constitem.vertalign());

		return true;
	}

	void SchemaItemConst::draw(CDrawParam* drawParam, const Schema* schema, const SchemaLayer* layer) const
	{
		FblItemRect::draw(drawParam, schema, layer);

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
//		SignedInteger32			SI: 100
//		UnsignedInteger32		UI: 100
//		SignedInteger64			SI64: 100
//		UnsignedInteger64		UI64: 100
//		Float 					FP: 100
//		Double					DBL: 0.123
//		Discrete				0 or 1
//
		QString text;

		switch (type())
		{
		case ConstType::IntegerType:
			{
				const Afb::AfbParamValue v = m_value.signedInt32;
				QString vstr = v.hasReference() ? v.reference() :
												  QString::number(v.value().toInt());

				text = QString("SI: %1").arg(vstr);
			}
			break;
		case ConstType::FloatType:
			{
				const Afb::AfbParamValue v = m_value.float32;
				QString vstr = v.hasReference() ? v.reference() :
												  QString::number(static_cast<double>(v.value().toFloat()), static_cast<char>(analogFormat()), precision());

				text = QString("FP: %1").arg(vstr);
			}
			break;
		case ConstType::Discrete:
			{
				const Afb::AfbParamValue v = m_value.discrete;
				QString vstr = v.hasReference() ? v.reference() :
												  QString::number(v.value().toInt());

				text = QString("%1").arg(vstr);
			}
			break;
		default:
			Q_ASSERT(false);
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
		case ConstType::Discrete:
			typeStr = "Discrete";
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
			propertyByCaption(PropertyNames::valueDiscrete)->setVisible(false);
			propertyByCaption(PropertyNames::precision)->setVisible(false);
			propertyByCaption(PropertyNames::analogFormat)->setVisible(false);
			emit propertyListChanged();		// Explicit emmiting signal, as setVisible does not do it
			break;
		case ConstType::FloatType:
			propertyByCaption(PropertyNames::valueInteger)->setVisible(false);
			propertyByCaption(PropertyNames::valueFloat)->setVisible(true);
			propertyByCaption(PropertyNames::valueDiscrete)->setVisible(false);
			propertyByCaption(PropertyNames::precision)->setVisible(true);
			propertyByCaption(PropertyNames::analogFormat)->setVisible(true);
			emit propertyListChanged();		// Explicit emmiting signal, as setVisible does not do it
			break;
		case ConstType::Discrete:
			propertyByCaption(PropertyNames::valueInteger)->setVisible(false);
			propertyByCaption(PropertyNames::valueFloat)->setVisible(false);
			propertyByCaption(PropertyNames::valueDiscrete)->setVisible(true);
			propertyByCaption(PropertyNames::precision)->setVisible(false);
			propertyByCaption(PropertyNames::analogFormat)->setVisible(false);
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

	bool SchemaItemConst::isDiscrete() const
	{
		return m_type == SchemaItemConst::ConstType::Discrete;
	}

	const Afb::AfbParamValue& SchemaItemConst::signedInt32Value() const
	{
		return m_value.signedInt32;
	}

	void SchemaItemConst::setSignedInt32Value(const Afb::AfbParamValue& intValue)
	{
		m_value.signedInt32.setValue(intValue.value().toInt());
		m_value.signedInt32.setReference(intValue.reference());
	}

	qint32 SchemaItemConst::signedInt32NativeValue() const
	{
		return m_value.signedInt32.value().value<qint32>();
	}

	void SchemaItemConst::setSignedInt32NativeValue(qint32 v)
	{
		m_value.signedInt32.setValue(v);
	}

	const Afb::AfbParamValue& SchemaItemConst::floatValue() const
	{
		return m_value.float32;
	}

	void SchemaItemConst::setFloatValue(const Afb::AfbParamValue& value)
	{
		m_value.float32.setValue(value.value().toFloat());
		m_value.float32.setReference(value.reference());
	}

	float SchemaItemConst::floatNativeValue() const
	{
		return m_value.float32.value().toFloat();
	}

	void SchemaItemConst::setFloatNativeValue(float v)
	{
		m_value.float32.setValue(v);
	}

	const Afb::AfbParamValue& SchemaItemConst::discreteValue() const
	{
		return m_value.discrete;
	}

	void SchemaItemConst::setDiscreteValue(const Afb::AfbParamValue& discreteValue)
	{
		quint16 v = qBound<quint16>(0, discreteValue.value().value<quint16>(), 1);
		m_value.discrete.setValue(v);
		m_value.discrete.setReference(discreteValue.reference());
	}

	quint16 SchemaItemConst::discreteNativeValue() const
	{
		return m_value.discrete.value().value<quint16>();
	}

	void SchemaItemConst::setDiscreteNativeValue(quint16 v)
	{
		m_value.discrete.setValue(qBound<quint16>(0, v, 1));
	}

	int SchemaItemConst::precision() const
	{
		return m_precision;
	}

	void SchemaItemConst::setPrecision(int value)
	{
		m_precision = qBound(-1, value, 24);

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

	E::AnalogFormat SchemaItemConst::analogFormat() const
	{
		return m_analogFormat;
	}

	void SchemaItemConst::setAnalogFormat(E::AnalogFormat value)
	{
		m_analogFormat = value;
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
