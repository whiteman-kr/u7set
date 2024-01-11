#include "DiagSignalType.h"

namespace Hardware
{
	//
	// DiagSignalType
	//
	const char* DiagSignalType::mimeType = "application/x-radiydiagsignaltype";

	DiagSignalType::DiagSignalType(QObject* parent) :
		PropertyObject(parent)
	{
		addProperty<QUuid, DiagSignalType, &DiagSignalType::uuid, &DiagSignalType::setUuid>(PropertyNames::uuid, {}, true)
			->setReadOnly(true)
			.setExpert(true)
			.setViewOrder(10);

		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::diagSignalTypeId, PropertyNames::categoryDiagSignal, true, DiagSignalType::signalTypeId, DiagSignalType::setSignalTypeId)
			->setEssential(true)
			.setViewOrder(11);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::systemSignalType, PropertyNames::categoryDiagSignal, true, DiagSignalType::systemSignalType, DiagSignalType::setSystemSignalType)
			->setExpert(true)
			.setDescription(QStringLiteral("System signal types are predefined and cannot be changed or deleted."))
			.setViewOrder(12);
		ADD_PROPERTY_GET_SET_CAT(E::DiagSignalType, PropertyNames::type, PropertyNames::categoryDiagSignal, true, DiagSignalType::type, DiagSignalType::setType)
			->setEssential(true)
			.setViewOrder(13);
		ADD_PROPERTY_GET_SET_CAT(E::DiagByteOrder, PropertyNames::byteOrder, PropertyNames::categoryDiagSignal, true, DiagSignalType::byteOrder, DiagSignalType::setByteOrder)
			->setViewOrder(14);

		// categoryDiscrete
		//
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::inverseValue, PropertyNames::categoryDiscrete, true, DiagSignalType::inverseValue, DiagSignalType::setInverseValue)
			->setViewOrder(200);
		ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::normalState, PropertyNames::categoryDiscrete, true, DiagSignalType::normalState, DiagSignalType::setNormalState)
			->setViewOrder(201);
		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::normalStateString0, PropertyNames::categoryDiscrete, true, DiagSignalType::normalStateString0, DiagSignalType::setNormalStateString0)
			->setViewOrder(202);
		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::normalStateString1, PropertyNames::categoryDiscrete, true, DiagSignalType::normalStateString1, DiagSignalType::setNormalStateString1)
			->setViewOrder(203);

		// categoryAnalog
		//
		ADD_PROPERTY_GET_SET_CAT(E::DiagAnalogFormat, PropertyNames::analogFormat, PropertyNames::categoryAnalog, true, DiagSignalType::analogFormat, DiagSignalType::setAnalogFormat)
			->setViewOrder(100);
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::adcHighLimit, PropertyNames::categoryAnalog, true, DiagSignalType::adcHighLimit, DiagSignalType::setAdcHighLimit)
			->setViewOrder(101);
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::adcLowLimit, PropertyNames::categoryAnalog, true, DiagSignalType::adcLowLimit, DiagSignalType::setAdcLowLimit)
			->setViewOrder(102);
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::valueHighLimit, PropertyNames::categoryAnalog, true, DiagSignalType::valueHighLimit, DiagSignalType::setValueHighLimit)
			->setViewOrder(103);
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::valueLowLimit, PropertyNames::categoryAnalog, true, DiagSignalType::valueLowLimit, DiagSignalType::setValueLowLimit)
			->setViewOrder(104);
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::valueMultiplier, PropertyNames::categoryAnalog, true, DiagSignalType::valueMultiplier, DiagSignalType::setValueMultiplier)
			->setViewOrder(105);
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::valueOffset, PropertyNames::categoryAnalog, true, DiagSignalType::valueOffset, DiagSignalType::setValueOffset)
			->setViewOrder(106);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::useLimits, PropertyNames::categoryAnalog, true, DiagSignalType::useLimits, DiagSignalType::setUseLimits)
			->setViewOrder(107);
		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::units, PropertyNames::categoryAnalog, true, DiagSignalType::units, DiagSignalType::setUnits)
			->setViewOrder(108);

		return;
	}

	void DiagSignalType::writeToXml(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(XmlElement::DIAG_SIGNAL_TYPE);

		xml.writeStringAttribute(XmlAttribute::SIGNAL_TYPE_ID, m_signalTypeId);
		xml.writeBoolAttribute(XmlAttribute::SYSTEM_SIGNAL_TYPE, m_systemSignalType);

		xml.writeEnumKeyAttribute(XmlAttribute::DIAG_SIGNAL_TYPE, m_type);
		xml.writeEnumKeyAttribute(XmlAttribute::DIAG_BYTE_ORDER, m_byteOrder);

		xml.writeStringAttribute(XmlAttribute::UNITS, m_units);

		xml.writeUuidAttribute(XmlAttribute::UUID, m_uuid);

		switch(m_type)
		{
		case E::DiagSignalType::Discrete:
			xml.writeBoolAttribute(XmlAttribute::INVERSE_VALUE, m_inverseValue);

			xml.writeIntAttribute(XmlAttribute::NORMAL_STATE, m_normalState);

			xml.writeStringAttribute(XmlAttribute::NORMAL_STATE_STR0, m_normalStateString0);
			xml.writeStringAttribute(XmlAttribute::NORMAL_STATE_STR1, m_normalStateString1);
			break;

		case E::DiagSignalType::Analog:
			xml.writeEnumKeyAttribute(XmlAttribute::DIAG_ANALOG_FORMAT, m_analogFormat);

			xml.writeBoolAttribute(XmlAttribute::USE_LIMITS, m_useLimits);

			xml.writeDoubleAttribute(XmlAttribute::ADC_HIGH_LIMIT, m_adcHighLimit);
			xml.writeDoubleAttribute(XmlAttribute::ADC_LOW_LIMIT, m_adcLowLimit);

			xml.writeDoubleAttribute(XmlAttribute::VALUE_HIGH_LIMIT, m_valueHighLimit);
			xml.writeDoubleAttribute(XmlAttribute::VALUE_LOW_LIMIT, m_valueLowLimit);

			xml.writeDoubleAttribute(XmlAttribute::VALUE_MULTIPLIER, m_valueMultiplier);
			xml.writeDoubleAttribute(XmlAttribute::VALUE_OFFSET, m_valueOffset);
			break;

		default:
			Q_ASSERT(false);
		}

		xml.writeEndElement();		// XmlElement::DIAG_SIGNAL_TYPE
	}

	bool DiagSignalType::readFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		result &= xml.findElement(XmlElement::DIAG_SIGNAL_TYPE);

		RETURN_IF_FALSE(result);

		result &= xml.readStringAttribute(XmlAttribute::SIGNAL_TYPE_ID, &m_signalTypeId);
		result &= xml.readBoolAttribute(XmlAttribute::SYSTEM_SIGNAL_TYPE, &m_systemSignalType);

		result &= xml.readEnumKeyAttribute(XmlAttribute::DIAG_SIGNAL_TYPE, &m_type);
		result &= xml.readEnumKeyAttribute(XmlAttribute::DIAG_BYTE_ORDER, &m_byteOrder);

		result &= xml.readStringAttribute(XmlAttribute::UNITS, &m_units);

		result &= xml.readUuidAttribute(XmlAttribute::UUID, &m_uuid);

		RETURN_IF_FALSE(result);

		switch(m_type)
		{
		case E::DiagSignalType::Discrete:
			result &= xml.readBoolAttribute(XmlAttribute::INVERSE_VALUE, &m_inverseValue);

			result &= xml.readIntAttribute(XmlAttribute::NORMAL_STATE, &m_normalState);

			result &= xml.readStringAttribute(XmlAttribute::NORMAL_STATE_STR0, &m_normalStateString0);
			result &= xml.readStringAttribute(XmlAttribute::NORMAL_STATE_STR1, &m_normalStateString1);
			break;

		case E::DiagSignalType::Analog:
			result &= xml.readEnumKeyAttribute(XmlAttribute::DIAG_ANALOG_FORMAT, &m_analogFormat);

			result &= xml.readBoolAttribute(XmlAttribute::USE_LIMITS, &m_useLimits);

			result &= xml.readDoubleAttribute(XmlAttribute::ADC_HIGH_LIMIT, &m_adcHighLimit);
			result &= xml.readDoubleAttribute(XmlAttribute::ADC_LOW_LIMIT, &m_adcLowLimit);

			result &= xml.readDoubleAttribute(XmlAttribute::VALUE_HIGH_LIMIT, &m_valueHighLimit);
			result &= xml.readDoubleAttribute(XmlAttribute::VALUE_LOW_LIMIT, &m_valueLowLimit);

			result &= xml.readDoubleAttribute(XmlAttribute::VALUE_MULTIPLIER, &m_valueMultiplier);
			result &= xml.readDoubleAttribute(XmlAttribute::VALUE_OFFSET, &m_valueOffset);
			break;

		default:
			Q_ASSERT(false);
		}

		return result;
	}

	std::shared_ptr<DiagSignalType> DiagSignalType::CreateObject(QObject* parent)
	{
		return std::shared_ptr<DiagSignalType>(new DiagSignalType{parent}); // cannot use make_shared as constructor is protected ((
	}

	std::shared_ptr<DiagSignalType> DiagSignalType::CreateObject(const Proto::Envelope& message)
	{
		std::shared_ptr<DiagSignalType> dst = DiagSignalType::CreateObject();

		bool ok = dst->LoadData(message);

		return ok ? dst : std::shared_ptr<DiagSignalType>();
	}

	bool DiagSignalType::SaveData(Proto::Envelope* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		const std::string& className = this->metaObject()->className();
		quint32 classnameHash = ::ClassNameHashCode(className);

		message->set_classnamehash(classnameHash);

		Proto::DiagSignalType* m = message->mutable_diagsignaltype();

		Proto::Write(m->mutable_uuid(), m_uuid);

		m->set_signaltypeid(m_signalTypeId.toStdString());
		m->set_systemsignaltype(m_systemSignalType);
		m->set_type(static_cast<int>(m_type));
		m->set_byteorder(static_cast<int>(m_byteOrder));

		m->set_inversevalue(m_inverseValue);
		m->set_normalstate(m_normalState);
		m->set_normalstatestring0(m_normalStateString0.toStdString());
		m->set_normalstatestring1(m_normalStateString1.toStdString());

		m->set_analogformat(static_cast<int>(m_analogFormat));
		m->set_adchighlimit(m_adcHighLimit);
		m->set_adclowlimit(m_adcLowLimit);
		m->set_valuehighlimit(m_valueHighLimit);
		m->set_valuelowlimit(m_valueLowLimit);
		m->set_valuemultiplier(m_valueMultiplier);
		m->set_valueoffset(m_valueOffset);
		m->set_uselimits(m_useLimits);
		m->set_units(m_units.toStdString());

		return true;
	}

	bool DiagSignalType::LoadData(const Proto::Envelope& message)
	{
		if (message.has_diagsignaltype() == false)
		{
			Q_ASSERT(message.has_diagsignaltype());
			return false;
		}

		const Proto::DiagSignalType& m = message.diagsignaltype();

		m_uuid = Proto::Read(m.uuid());
		Q_ASSERT(m_uuid.isNull() == false);

		m_signalTypeId = QString::fromStdString(m.signaltypeid());
		m_systemSignalType = m.systemsignaltype();
		m_type = static_cast<E::DiagSignalType>(m.type());
		m_byteOrder = static_cast<E::DiagByteOrder>(m.byteorder());

		m_inverseValue = m.inversevalue();
		m_normalState = m.normalstate();
		m_normalStateString0 = QString::fromStdString(m.normalstatestring0());
		m_normalStateString1 = QString::fromStdString(m.normalstatestring1());

		m_analogFormat = static_cast<E::DiagAnalogFormat>(m.analogformat());
		m_adcHighLimit = m.adchighlimit();
		m_adcLowLimit = m.adclowlimit();
		m_valueHighLimit = m.valuehighlimit();
		m_valueLowLimit = m.valuelowlimit();
		m_valueMultiplier = m.valuemultiplier();
		m_valueOffset = m.valueoffset();
		m_useLimits = m.uselimits();
		m_units = QString::fromStdString(m.units());

		return true;
	}

	QUuid DiagSignalType::uuid() const
	{
		return m_uuid;
	}

	void DiagSignalType::setUuid(QUuid uuid)
	{
		m_uuid = uuid;
	}

	bool DiagSignalType::isSystemSignalType() const
	{
		return systemSignalType();
	}

	bool DiagSignalType::systemSignalType() const
	{
		return m_systemSignalType;
	}

	void DiagSignalType::setSystemSignalType(bool value)
	{
		m_systemSignalType = value;
	}

	const QString& DiagSignalType::signalTypeId() const
	{
		return m_signalTypeId;
	}

	void DiagSignalType::setSignalTypeId(const QString& value)
	{
		m_signalTypeId = value;
	}

	E::DiagSignalType DiagSignalType::type() const
	{
		return m_type;
	}

	void DiagSignalType::setType(E::DiagSignalType value)
	{
		m_type = value;
	}

	E::DiagByteOrder DiagSignalType::byteOrder() const
	{
		return m_byteOrder;
	}

	void DiagSignalType::setByteOrder(E::DiagByteOrder value)
	{
		m_byteOrder = value;
	}

	bool DiagSignalType::inverseValue() const
	{
		return m_inverseValue;
	}

	void DiagSignalType::setInverseValue(bool value)
	{
		m_inverseValue = value;
	}

	int DiagSignalType::normalState() const
	{
		return m_normalState;
	}

	void DiagSignalType::setNormalState(int value)
	{
		m_normalState = value;
	}

	const QString& DiagSignalType::normalStateString0() const
	{
		return m_normalStateString0;
	}

	void DiagSignalType::setNormalStateString0(const QString& value)
	{
		m_normalStateString0 = value;
	}

	const QString& DiagSignalType::normalStateString1() const
	{
		return m_normalStateString1;
	}

	void DiagSignalType::setNormalStateString1(const QString& value)
	{
		m_normalStateString1 = value;
	}

	E::DiagAnalogFormat DiagSignalType::analogFormat() const
	{
		return m_analogFormat;
	}

	void DiagSignalType::setAnalogFormat(E::DiagAnalogFormat value)
	{
		m_analogFormat = value;
	}

	double DiagSignalType::adcHighLimit() const
	{
		return m_adcHighLimit;
	}

	void DiagSignalType::setAdcHighLimit(double value)
	{
		m_adcHighLimit = value;
	}

	double DiagSignalType::adcLowLimit() const
	{
		return m_adcLowLimit;
	}

	void DiagSignalType::setAdcLowLimit(double value)
	{
		m_adcLowLimit = value;
	}

	double DiagSignalType::valueHighLimit() const
	{
		return m_valueHighLimit;
	}

	void DiagSignalType::setValueHighLimit(double value)
	{
		m_valueHighLimit = value;
	}

	double DiagSignalType::valueLowLimit() const
	{
		return m_valueLowLimit;
	}

	void DiagSignalType::setValueLowLimit(double value)
	{
		m_valueLowLimit = value;
	}

	double DiagSignalType::valueMultiplier() const
	{
		return m_valueMultiplier;
	}

	void DiagSignalType::setValueMultiplier(double value)
	{
		m_valueMultiplier = value;
	}

	double DiagSignalType::valueOffset() const
	{
		return m_valueOffset;
	}

	void DiagSignalType::setValueOffset(double value)
	{
		m_valueOffset = value;
	}

	bool DiagSignalType::useLimits() const
	{
		return m_useLimits;
	}

	void DiagSignalType::setUseLimits(bool value)
	{
		m_useLimits = value;
	}

	const QString& DiagSignalType::units() const
	{
		return m_units;
	}

	void DiagSignalType::setUnits(const QString& value)
	{
		m_units = value;
	}

} // namespace Hardware
