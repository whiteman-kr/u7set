#include "./include/HardwareLib/DiagSignalType.h"
#include "./include/HardwareLib/PropertyNames.h"
#include "../Proto/ProtoCommonHelper.h"
#include "../UtilsLib/XmlHelper.h"

namespace Hardware
{
	void DiagSignalType::writeToXml(XmlWriteHelper& xml) const
	{
		xml.writeStartElement(XmlElement::DIAG_SIGNAL_TYPE);

		xml.writeStringAttribute(XmlAttribute::SIGNAL_TYPE_ID, signalTypeId);
		xml.writeBoolAttribute(XmlAttribute::SYSTEM_SIGNAL_TYPE, systemSignalType);

		xml.writeEnumKeyAttribute(XmlAttribute::DIAG_SIGNAL_TYPE, type);
		xml.writeEnumKeyAttribute(XmlAttribute::DIAG_BYTE_ORDER, byteOrder);

		xml.writeStringAttribute(XmlAttribute::UNITS, units);

		xml.writeUuidAttribute(XmlAttribute::UUID, uuid);

		switch (type)
		{
		case E::DiagSignalType::Discrete:
			xml.writeBoolAttribute(XmlAttribute::INVERSE_VALUE, inverseValue);

			xml.writeIntAttribute(XmlAttribute::NORMAL_STATE, normalState);

			xml.writeStringAttribute(XmlAttribute::NORMAL_STATE_STR0, normalStateString0);
			xml.writeStringAttribute(XmlAttribute::NORMAL_STATE_STR1, normalStateString1);
			break;

		case E::DiagSignalType::Analog:
			xml.writeEnumKeyAttribute(XmlAttribute::DIAG_ANALOG_FORMAT, analogFormat);

			xml.writeBoolAttribute(XmlAttribute::USE_LIMITS, useLimits);

			xml.writeDoubleAttribute(XmlAttribute::ADC_HIGH_LIMIT, adcHighLimit);
			xml.writeDoubleAttribute(XmlAttribute::ADC_LOW_LIMIT, adcLowLimit);

			xml.writeDoubleAttribute(XmlAttribute::VALUE_HIGH_LIMIT, valueHighLimit);
			xml.writeDoubleAttribute(XmlAttribute::VALUE_LOW_LIMIT, valueLowLimit);

			xml.writeDoubleAttribute(XmlAttribute::VALUE_MULTIPLIER, valueMultiplier);
			xml.writeDoubleAttribute(XmlAttribute::VALUE_OFFSET, valueOffset);
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

		result &= xml.readStringAttribute(XmlAttribute::SIGNAL_TYPE_ID, &signalTypeId);
		result &= xml.readBoolAttribute(XmlAttribute::SYSTEM_SIGNAL_TYPE, &systemSignalType);

		result &= xml.readEnumKeyAttribute(XmlAttribute::DIAG_SIGNAL_TYPE, &type);
		result &= xml.readEnumKeyAttribute(XmlAttribute::DIAG_BYTE_ORDER, &byteOrder);

		result &= xml.readStringAttribute(XmlAttribute::UNITS, &units);

		result &= xml.readUuidAttribute(XmlAttribute::UUID, &uuid);

		RETURN_IF_FALSE(result);

		switch (type)
		{
		case E::DiagSignalType::Discrete:
			result &= xml.readBoolAttribute(XmlAttribute::INVERSE_VALUE, &inverseValue);

			result &= xml.readIntAttribute(XmlAttribute::NORMAL_STATE, &normalState);

			result &= xml.readStringAttribute(XmlAttribute::NORMAL_STATE_STR0, &normalStateString0);
			result &= xml.readStringAttribute(XmlAttribute::NORMAL_STATE_STR1, &normalStateString1);
			break;

		case E::DiagSignalType::Analog:
			result &= xml.readEnumKeyAttribute(XmlAttribute::DIAG_ANALOG_FORMAT, &analogFormat);

			result &= xml.readBoolAttribute(XmlAttribute::USE_LIMITS, &useLimits);

			result &= xml.readDoubleAttribute(XmlAttribute::ADC_HIGH_LIMIT, &adcHighLimit);
			result &= xml.readDoubleAttribute(XmlAttribute::ADC_LOW_LIMIT, &adcLowLimit);

			result &= xml.readDoubleAttribute(XmlAttribute::VALUE_HIGH_LIMIT, &valueHighLimit);
			result &= xml.readDoubleAttribute(XmlAttribute::VALUE_LOW_LIMIT, &valueLowLimit);

			result &= xml.readDoubleAttribute(XmlAttribute::VALUE_MULTIPLIER, &valueMultiplier);
			result &= xml.readDoubleAttribute(XmlAttribute::VALUE_OFFSET, &valueOffset);
			break;

		default:
			Q_ASSERT(false);
		}

		return result;
	}

	void DiagSignalType::save(Proto::DiagSignalType* message) const
	{
		Q_ASSERT(message);

		Proto::Write(message->mutable_uuid(), uuid);

		message->set_signaltypeid(signalTypeId.toStdString());
		message->set_systemsignaltype(systemSignalType);
		message->set_type(static_cast<int>(type));
		message->set_byteorder(static_cast<int>(byteOrder));

		message->set_inversevalue(inverseValue);
		message->set_normalstate(normalState);
		message->set_normalstatestring0(normalStateString0.toStdString());
		message->set_normalstatestring1(normalStateString1.toStdString());

		message->set_analogformat(static_cast<int>(analogFormat));
		message->set_adchighlimit(adcHighLimit);
		message->set_adclowlimit(adcLowLimit);
		message->set_valuehighlimit(valueHighLimit);
		message->set_valuelowlimit(valueLowLimit);
		message->set_valuemultiplier(valueMultiplier);
		message->set_valueoffset(valueOffset);
		message->set_uselimits(useLimits);
		message->set_units(units.toStdString());

		return;
	}

	bool DiagSignalType::load(const Proto::DiagSignalType& message)
	{
		uuid = Proto::Read(message.uuid());
		Q_ASSERT(uuid.isNull() == false);

		signalTypeId = QString::fromStdString(message.signaltypeid());
		systemSignalType = message.systemsignaltype();
		type = static_cast<E::DiagSignalType>(message.type());
		byteOrder = static_cast<E::DiagByteOrder>(message.byteorder());

		inverseValue = message.inversevalue();
		normalState = message.normalstate();
		normalStateString0 = QString::fromStdString(message.normalstatestring0());
		normalStateString1 = QString::fromStdString(message.normalstatestring1());

		analogFormat = static_cast<E::DiagAnalogFormat>(message.analogformat());
		adcHighLimit = message.adchighlimit();
		adcLowLimit = message.adclowlimit();
		valueHighLimit = message.valuehighlimit();
		valueLowLimit = message.valuelowlimit();
		valueMultiplier = message.valuemultiplier();
		valueOffset = message.valueoffset();
		useLimits = message.uselimits();
		units = QString::fromStdString(message.units());

		return true;
	}

	//
	// DiagSignalTypes
	//
	void DiagSignalTypes::clear()
	{
		m_types.clear();
	}

	std::vector<DiagSignalType>* DiagSignalTypes::mutableDiagSignalTypes()
	{
		return &m_types;
	}

	void DiagSignalTypes::writeToXml(XmlWriteHelper& xml) const
	{
		xml.writeStartDocument();

		xml.writeStartElement(XmlElement::DIAG_SIGNAL_TYPES);
		xml.writeIntAttribute(XmlAttribute::COUNT, static_cast<int>(m_types.size()));

		for(const auto& dst : m_types)
		{
			dst.writeToXml(xml);
		}

		xml.writeEndElement();		//	XmlElement::DIAG_SIGNAL_TYPES

		xml.writeEndDocument();
	}

	bool DiagSignalTypes::readFromXml(XmlReadHelper& xml)
	{
		m_types.clear();

		bool result = true;

		result &= xml.findElement(XmlElement::DIAG_SIGNAL_TYPES);

		RETURN_IF_FALSE(result);

		int count = 0;
		result &= xml.readIntAttribute(XmlAttribute::COUNT, &count);

		m_types.resize(count);

		for(int i = 0; i < count; i++)
		{
			result &= m_types[i].readFromXml(xml);
		}

		return result;
	}

	//
	// DiagSignalType
	//
	const char* DiagSignalTypeObject::mimeType = "application/x-radiydiagsignaltype";

	DiagSignalTypeObject::DiagSignalTypeObject(QObject* parent) :
		PropertyObject(parent)
	{
		addProperty<QUuid, DiagSignalTypeObject, &DiagSignalTypeObject::uuid, &DiagSignalTypeObject::setUuid>(PropertyNames::uuid, {}, true)
			->setReadOnly(true)
			.setExpert(true)
			.setViewOrder(10);

		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::diagSignalTypeId, PropertyNames::categoryDiagSignal, true, DiagSignalTypeObject::signalTypeId, DiagSignalTypeObject::setSignalTypeId)
			->setEssential(true)
			.setViewOrder(11);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::systemSignalType, PropertyNames::categoryDiagSignal, true, DiagSignalTypeObject::systemSignalType, DiagSignalTypeObject::setSystemSignalType)
			->setExpert(true)
			.setDescription(QStringLiteral("System signal types are predefined and cannot be changed or deleted."))
			.setViewOrder(12);
		ADD_PROPERTY_GET_SET_CAT(E::DiagSignalType, PropertyNames::type, PropertyNames::categoryDiagSignal, true, DiagSignalTypeObject::type, DiagSignalTypeObject::setType)
			->setEssential(true)
			.setViewOrder(13);
		ADD_PROPERTY_GET_SET_CAT(E::DiagByteOrder, PropertyNames::byteOrder, PropertyNames::categoryDiagSignal, true, DiagSignalTypeObject::byteOrder, DiagSignalTypeObject::setByteOrder)
			->setViewOrder(14);

		// categoryDiscrete
		//
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::inverseValue, PropertyNames::categoryDiscrete, true, DiagSignalTypeObject::inverseValue, DiagSignalTypeObject::setInverseValue)
			->setViewOrder(200);
		ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::normalState, PropertyNames::categoryDiscrete, true, DiagSignalTypeObject::normalState, DiagSignalTypeObject::setNormalState)
			->setViewOrder(201);
		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::normalStateString0, PropertyNames::categoryDiscrete, true, DiagSignalTypeObject::normalStateString0, DiagSignalTypeObject::setNormalStateString0)
			->setViewOrder(202);
		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::normalStateString1, PropertyNames::categoryDiscrete, true, DiagSignalTypeObject::normalStateString1, DiagSignalTypeObject::setNormalStateString1)
			->setViewOrder(203);

		// categoryAnalog
		//
		ADD_PROPERTY_GET_SET_CAT(E::DiagAnalogFormat, PropertyNames::analogFormat, PropertyNames::categoryAnalog, true, DiagSignalTypeObject::analogFormat, DiagSignalTypeObject::setAnalogFormat)
			->setViewOrder(100);
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::adcHighLimit, PropertyNames::categoryAnalog, true, DiagSignalTypeObject::adcHighLimit, DiagSignalTypeObject::setAdcHighLimit)
			->setViewOrder(101);
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::adcLowLimit, PropertyNames::categoryAnalog, true, DiagSignalTypeObject::adcLowLimit, DiagSignalTypeObject::setAdcLowLimit)
			->setViewOrder(102);
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::valueHighLimit, PropertyNames::categoryAnalog, true, DiagSignalTypeObject::valueHighLimit, DiagSignalTypeObject::setValueHighLimit)
			->setViewOrder(103);
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::valueLowLimit, PropertyNames::categoryAnalog, true, DiagSignalTypeObject::valueLowLimit, DiagSignalTypeObject::setValueLowLimit)
			->setViewOrder(104);
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::valueMultiplier, PropertyNames::categoryAnalog, true, DiagSignalTypeObject::valueMultiplier, DiagSignalTypeObject::setValueMultiplier)
			->setViewOrder(105);
		ADD_PROPERTY_GET_SET_CAT(double, PropertyNames::valueOffset, PropertyNames::categoryAnalog, true, DiagSignalTypeObject::valueOffset, DiagSignalTypeObject::setValueOffset)
			->setViewOrder(106);
		ADD_PROPERTY_GET_SET_CAT(bool, PropertyNames::useLimits, PropertyNames::categoryAnalog, true, DiagSignalTypeObject::useLimits, DiagSignalTypeObject::setUseLimits)
			->setViewOrder(107);
		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::units, PropertyNames::categoryAnalog, true, DiagSignalTypeObject::units, DiagSignalTypeObject::setUnits)
			->setViewOrder(108);

		return;
	}

	std::shared_ptr<DiagSignalTypeObject> DiagSignalTypeObject::CreateObject(QObject* parent)
	{
		return std::shared_ptr<DiagSignalTypeObject>(new DiagSignalTypeObject{parent}); // cannot use make_shared as constructor is protected ((
	}

	std::shared_ptr<DiagSignalTypeObject> DiagSignalTypeObject::CreateObject(const Proto::Envelope& message)
	{
		auto dst = DiagSignalTypeObject::CreateObject();
		bool ok = dst->LoadData(message);

		return ok ? dst : std::shared_ptr<DiagSignalTypeObject>();
	}

	bool DiagSignalTypeObject::SaveData(Proto::Envelope* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		const std::string& className = this->metaObject()->className();
		quint32 classnameHash = ::ClassNameHashCode(className);

		message->set_classnamehash(classnameHash);

		auto m = message->MutableExtension(::Proto::diagSignalType);
		m_data.save(m);

		return true;
	}

	bool DiagSignalTypeObject::LoadData(const Proto::Envelope& message)
	{
		if (message.HasExtension(::Proto::diagSignalType) == false)
		{
			Q_ASSERT(message.HasExtension(::Proto::diagSignalType));
			return false;
		}

		return m_data.load(message.GetExtension(::Proto::diagSignalType));
	}

	QUuid DiagSignalTypeObject::uuid() const
	{
		return m_data.uuid;
	}

	void DiagSignalTypeObject::setUuid(QUuid uuid)
	{
		m_data.uuid = uuid;
	}

	bool DiagSignalTypeObject::isSystemSignalType() const
	{
		return systemSignalType();
	}

	bool DiagSignalTypeObject::systemSignalType() const
	{
		return m_data.systemSignalType;
	}

	void DiagSignalTypeObject::setSystemSignalType(bool value)
	{
		m_data.systemSignalType = value;
	}

	const QString& DiagSignalTypeObject::signalTypeId() const
	{
		return m_data.signalTypeId;
	}

	void DiagSignalTypeObject::setSignalTypeId(const QString& value)
	{
		m_data.signalTypeId = value;
	}

	E::DiagSignalType DiagSignalTypeObject::type() const
	{
		return m_data.type;
	}

	void DiagSignalTypeObject::setType(E::DiagSignalType value)
	{
		m_data.type = value;
	}

	E::DiagByteOrder DiagSignalTypeObject::byteOrder() const
	{
		return m_data.byteOrder;
	}

	void DiagSignalTypeObject::setByteOrder(E::DiagByteOrder value)
	{
		m_data.byteOrder = value;
	}

	bool DiagSignalTypeObject::inverseValue() const
	{
		return m_data.inverseValue;
	}

	void DiagSignalTypeObject::setInverseValue(bool value)
	{
		m_data.inverseValue = value;
	}

	int DiagSignalTypeObject::normalState() const
	{
		return m_data.normalState;
	}

	void DiagSignalTypeObject::setNormalState(int value)
	{
		m_data.normalState = value;
	}

	const QString& DiagSignalTypeObject::normalStateString0() const
	{
		return m_data.normalStateString0;
	}

	void DiagSignalTypeObject::setNormalStateString0(const QString& value)
	{
		m_data.normalStateString0 = value;
	}

	const QString& DiagSignalTypeObject::normalStateString1() const
	{
		return m_data.normalStateString1;
	}

	void DiagSignalTypeObject::setNormalStateString1(const QString& value)
	{
		m_data.normalStateString1 = value;
	}

	E::DiagAnalogFormat DiagSignalTypeObject::analogFormat() const
	{
		return m_data.analogFormat;
	}

	void DiagSignalTypeObject::setAnalogFormat(E::DiagAnalogFormat value)
	{
		m_data.analogFormat = value;
	}

	double DiagSignalTypeObject::adcHighLimit() const
	{
		return m_data.adcHighLimit;
	}

	void DiagSignalTypeObject::setAdcHighLimit(double value)
	{
		m_data.adcHighLimit = value;
	}

	double DiagSignalTypeObject::adcLowLimit() const
	{
		return m_data.adcLowLimit;
	}

	void DiagSignalTypeObject::setAdcLowLimit(double value)
	{
		m_data.adcLowLimit = value;
	}

	double DiagSignalTypeObject::valueHighLimit() const
	{
		return m_data.valueHighLimit;
	}

	void DiagSignalTypeObject::setValueHighLimit(double value)
	{
		m_data.valueHighLimit = value;
	}

	double DiagSignalTypeObject::valueLowLimit() const
	{
		return m_data.valueLowLimit;
	}

	void DiagSignalTypeObject::setValueLowLimit(double value)
	{
		m_data.valueLowLimit = value;
	}

	double DiagSignalTypeObject::valueMultiplier() const
	{
		return m_data.valueMultiplier;
	}

	void DiagSignalTypeObject::setValueMultiplier(double value)
	{
		m_data.valueMultiplier = value;
	}

	double DiagSignalTypeObject::valueOffset() const
	{
		return m_data.valueOffset;
	}

	void DiagSignalTypeObject::setValueOffset(double value)
	{
		m_data.valueOffset = value;
	}

	bool DiagSignalTypeObject::useLimits() const
	{
		return m_data.useLimits;
	}

	void DiagSignalTypeObject::setUseLimits(bool value)
	{
		m_data.useLimits = value;
	}

	const QString& DiagSignalTypeObject::units() const
	{
		return m_data.units;
	}

	void DiagSignalTypeObject::setUnits(const QString& value)
	{
		m_data.units = value;
	}

	const DiagSignalType DiagSignalTypeObject::diagSignalType() const
	{
		return m_data;
	}

} // namespace Hardware
