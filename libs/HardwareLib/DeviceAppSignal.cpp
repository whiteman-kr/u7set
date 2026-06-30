#include <HardwareLib/DeviceAppSignal.h>
#include <HardwareLib/PropertyNames.h>

namespace Hardware
{
	//
	//
	// DeviceAppSignal
	//
	//
	DeviceAppSignal::DeviceAppSignal(bool preset /*= false*/, QObject* parent /*= nullptr*/) noexcept :
		DeviceObject(DeviceType::AppSignal, preset, parent)
	{
		// These properties are used in setType()
		// So they don take part in PropertyOnDemand
		//
		addProperty<E::AnalogAppSignalFormat,
					DeviceAppSignal,
					&DeviceAppSignal::appSignalDataFormat,
					&DeviceAppSignal::setAppSignalDataFormat>(PropertyNames::appSignalDataFormat, PropertyNames::categoryAppSignal, true)
			->setUpdateFromPreset(true)
			.setExpert(preset);

		addProperty<QString, DeviceAppSignal, &DeviceAppSignal::appSignalBusTypeId, &DeviceAppSignal::setAppSignalBusTypeId>(
			PropertyNames::appSignalBusTypeId,
			PropertyNames::categoryAppSignal,
			true)
			->setUpdateFromPreset(true)
			.setExpert(preset);

		// Show/Hide analog signal properties
		//
		setSignalType(signalType());

		return;
	}

	void DeviceAppSignal::propertyDemand(const QString& prop)
	{
		DeviceObject::propertyDemand(prop);

		if (prop.isEmpty() == true || prop == PropertyNames::isInstantiable)
		{
			auto instantiableProp = addProperty<bool, DeviceAppSignal, &DeviceAppSignal::isInstantiable, &DeviceAppSignal::setInstantiable>(
				PropertyNames::isInstantiable,
				PropertyNames::categoryAppSignal,
				true);
			instantiableProp->setUpdateFromPreset(true);
			instantiableProp->setExpert(isPreset());
		}

		if (prop.isEmpty() == true || prop == PropertyNames::type)
		{
			auto typeProp = addProperty<E::SignalType, DeviceAppSignal, &DeviceAppSignal::signalType, &DeviceAppSignal::setSignalType>(
				PropertyNames::type,
				PropertyNames::categoryAppSignal,
				true);
			typeProp->setUpdateFromPreset(true);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::function)
		{
			auto functionProp = addProperty<E::SignalFunction, DeviceAppSignal, &DeviceAppSignal::function, &DeviceAppSignal::setFunction>(
				PropertyNames::function,
				PropertyNames::categoryAppSignal,
				true);
			functionProp->setUpdateFromPreset(true);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::byteOrder)
		{
			auto byteOrderProp = addProperty<E::ByteOrder, DeviceAppSignal, &DeviceAppSignal::byteOrder, &DeviceAppSignal::setByteOrder>(
				PropertyNames::byteOrder,
				PropertyNames::categoryAppSignal,
				true);
			byteOrderProp->setUpdateFromPreset(true);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::format)
		{
			auto formatProp = addProperty<E::DataFormat, DeviceAppSignal, &DeviceAppSignal::format, &DeviceAppSignal::setFormat>(
				PropertyNames::format,
				PropertyNames::categoryAppSignal,
				true);
			formatProp->setUpdateFromPreset(true);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::memoryArea)
		{
			auto memoryAreaProp =
				addProperty<E::MemoryArea, DeviceAppSignal, &DeviceAppSignal::memoryArea, &DeviceAppSignal::setMemoryArea>(
					PropertyNames::memoryArea,
					PropertyNames::categoryAppSignal,
					true);
			memoryAreaProp->setUpdateFromPreset(true);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::size)
		{
			auto sizeProp =
				addProperty<int, DeviceAppSignal, &DeviceAppSignal::size, &DeviceAppSignal::setSize>(PropertyNames::size,
																									 PropertyNames::categoryAppSignal,
																									 true);
			sizeProp->setUpdateFromPreset(true);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::valueOffset)
		{
			auto valueOffsetProp = addProperty<int, DeviceAppSignal, &DeviceAppSignal::valueOffset, &DeviceAppSignal::setValueOffset>(
				PropertyNames::valueOffset,
				PropertyNames::categoryAppSignal,
				true);
			valueOffsetProp->setUpdateFromPreset(true);
			valueOffsetProp->setExpert(isPreset());
		}

		if (prop.isEmpty() == true || prop == PropertyNames::valueBit)
		{
			auto valueBitProp = addProperty<int, DeviceAppSignal, &DeviceAppSignal::valueBit, &DeviceAppSignal::setValueBit>(
				PropertyNames::valueBit,
				PropertyNames::categoryAppSignal,
				true);
			valueBitProp->setUpdateFromPreset(true);
			valueBitProp->setExpert(isPreset());
		}

		if (prop.isEmpty() == true || prop == PropertyNames::validitySignalId)
		{
			auto validitySignalId =
				addProperty<QString, DeviceAppSignal, &DeviceAppSignal::validitySignalId, &DeviceAppSignal::setValiditySignalId>(
					PropertyNames::validitySignalId,
					PropertyNames::categoryAppSignal,
					true);
			validitySignalId->setUpdateFromPreset(true);
		}

		if (prop.isEmpty() == true || prop == PropertyNames::signalSpecificProperties)
		{
			auto signalSpecPropsStructProp =
				addProperty<QString, DeviceAppSignal, &DeviceAppSignal::signalSpecPropsStruct, &DeviceAppSignal::setSignalSpecPropsStruct>(
					PropertyNames::signalSpecificProperties,
					PropertyNames::categoryAppSignal,
					true);

			signalSpecPropsStructProp->setUpdateFromPreset(true);
			signalSpecPropsStructProp->setExpert(isPreset());
			signalSpecPropsStructProp->setSpecificEditor(E::PropertySpecificEditor::SpecificPropertyStruct);
		}

		return;
	}

	bool DeviceAppSignal::SaveData(Proto::Envelope* message, bool saveTree, const std::function<bool(const DeviceObject&)>& predicate) const
	{
		bool result = DeviceObject::SaveData(message, saveTree, predicate);
		if (result == false || message->HasExtension(::Proto::deviceobject) == false)
		{
			Q_ASSERT(result);
			Q_ASSERT(message->HasExtension(::Proto::deviceobject));
			return false;
		}

		// --
		//
		auto signalMessage = message->MutableExtension(::Proto::deviceobject)->mutable_appsignal();

		signalMessage->set_instantiable(m_instantiable);

		signalMessage->set_type(static_cast<int>(m_signalType));
		signalMessage->set_function(static_cast<int>(m_function));

		signalMessage->set_byteorder(static_cast<int>(m_byteOrder));
		signalMessage->set_format(static_cast<int>(m_format));
		signalMessage->set_memoryarea(static_cast<int>(m_memoryArea));

		signalMessage->set_size(static_cast<int>(m_size));

		signalMessage->set_valueoffset(static_cast<int>(m_valueOffset));
		signalMessage->set_valuebit(static_cast<int>(m_valueBit));

		signalMessage->set_validitysignalid(m_validitySignalId.toUtf8());

		signalMessage->set_appsignallowadc(m_appSignalLowAdc);
		signalMessage->set_appsignalhighadc(m_appSignalHighAdc);

		signalMessage->set_appsignallowengunits(m_appSignalLowEngUnits);
		signalMessage->set_appsignalhighengunits(m_appSignalHighEngUnits);

		signalMessage->set_appsignaldataformat(static_cast<int>(m_appSignalDataFormat));

		signalMessage->set_appsignalbustypeid(m_appSignalBusTypeId.toStdString());

		signalMessage->set_signalspecpropsstruct(m_signalSpecPropsStruct.toUtf8());
		signalMessage->set_signalspecpropsstructwasfixed(true); // m_signalSpecPropsStruct was fixed on loading

		return true;
	}

	bool DeviceAppSignal::LoadData(const Proto::Envelope& message)
	{
		if (message.HasExtension(::Proto::deviceobject) == false)
		{
			Q_ASSERT(message.HasExtension(::Proto::deviceobject));
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		const auto& deviceobject = message.GetExtension(::Proto::deviceobject);

		if (deviceobject.has_appsignal() == false)
		{
			Q_ASSERT(deviceobject.has_appsignal());
			return false;
		}

		const auto& signalMessage = deviceobject.appsignal();

		m_instantiable = signalMessage.instantiable();

		if (signalMessage.has_obsoletetype() == true)
		{
			Q_ASSERT(signalMessage.has_type() == false);
			Q_ASSERT(signalMessage.has_function() == false);

			enum ObsoleteSignalType
			{
				DiagDiscrete,
				DiagAnalog,
				InputDiscrete,
				InputAnalog,
				OutputDiscrete,
				OutputAnalog,
			};

			ObsoleteSignalType obsoleteType = static_cast<ObsoleteSignalType>(signalMessage.obsoletetype());

			switch (obsoleteType)
			{
			case ObsoleteSignalType::DiagDiscrete:
				setSignalType(E::SignalType::Discrete);
				m_function = E::SignalFunction::Diagnostics;
				break;
			case ObsoleteSignalType::DiagAnalog:
				setSignalType(E::SignalType::Analog);
				m_function = E::SignalFunction::Diagnostics;
				break;
			case ObsoleteSignalType::InputDiscrete:
				setSignalType(E::SignalType::Discrete);
				m_function = E::SignalFunction::Input;
				break;
			case ObsoleteSignalType::InputAnalog:
				setSignalType(E::SignalType::Analog);
				m_function = E::SignalFunction::Input;
				break;
			case ObsoleteSignalType::OutputDiscrete:
				setSignalType(E::SignalType::Discrete);
				m_function = E::SignalFunction::Output;
				break;
			case ObsoleteSignalType::OutputAnalog:
				setSignalType(E::SignalType::Analog);
				m_function = E::SignalFunction::Output;
				break;
			default:
				Q_ASSERT(false);
			}
		}
		else
		{
			setSignalType(static_cast<E::SignalType>(signalMessage.type())); // Show hide some props
			m_function = static_cast<E::SignalFunction>(signalMessage.function());
		}

		m_byteOrder = static_cast<E::ByteOrder>(signalMessage.byteorder());
		m_format = static_cast<E::DataFormat>(signalMessage.format());
		m_memoryArea = static_cast<E::MemoryArea>(signalMessage.memoryarea());

		m_size = signalMessage.size();

		m_valueOffset = signalMessage.valueoffset();
		m_valueBit = signalMessage.valuebit();

		m_validitySignalId = QString::fromUtf8(signalMessage.validitysignalid().data());

		m_appSignalLowAdc = signalMessage.appsignallowadc();
		m_appSignalHighAdc = signalMessage.appsignalhighadc();

		m_appSignalLowEngUnits = signalMessage.appsignallowengunits();
		m_appSignalHighEngUnits = signalMessage.appsignalhighengunits();

		m_appSignalDataFormat = static_cast<E::AnalogAppSignalFormat>(signalMessage.appsignaldataformat());

		m_appSignalBusTypeId = QString::fromStdString(signalMessage.appsignalbustypeid());

		m_signalSpecPropsStruct = QString::fromStdString(signalMessage.signalspecpropsstruct());

		if (signalMessage.signalspecpropsstructwasfixed() == false)
		{
			// RPCT-2622, RPCT-2621
			// Shit happens. We had a situaltion when misprinting was detected (EngEneering vs EngIneering).
			// To avoid manual replacement of this typo for non platform modules, the replace just made.
			//
			m_signalSpecPropsStruct = replaceEngeneeringToEngineering(m_signalSpecPropsStruct);
		}

		if (isPreset() == true)
		{
			setExpertToProperty(PropertyNames::type, true);
			setExpertToProperty(PropertyNames::function, true);
			setExpertToProperty(PropertyNames::byteOrder, true);
			setExpertToProperty(PropertyNames::format, true);
			setExpertToProperty(PropertyNames::memoryArea, true);
			setExpertToProperty(PropertyNames::size, true);
			setExpertToProperty(PropertyNames::valueOffset, true);
			setExpertToProperty(PropertyNames::valueBit, true);
			setExpertToProperty(PropertyNames::validitySignalId, true);
			setExpertToProperty(PropertyNames::appSignalDataFormat, true);
			setExpertToProperty(PropertyNames::signalSpecificProperties, true);
		}

		return true;
	}

	void DeviceAppSignal::expandEquipmentId()
	{
		if (m_validitySignalId.isEmpty() == false)
		{
			if (hasParent() == true)
			{
				m_validitySignalId.replace(QLatin1String("$(PARENT)"), parent()->equipmentIdTemplate(), Qt::CaseInsensitive);
			}

			m_validitySignalId.replace(QLatin1String("$(PLACE)"), QString::number(place()).rightJustified(2, '0'), Qt::CaseInsensitive);
		}

		DeviceObject::expandEquipmentId();

		return;
	}

	bool DeviceAppSignal::isInstantiable() const
	{
		return m_instantiable;
	}
	void DeviceAppSignal::setInstantiable(bool value)
	{
		m_instantiable = value;
	}

	E::SignalType DeviceAppSignal::signalType() const
	{
		return m_signalType;
	}

	void DeviceAppSignal::setSignalType(E::SignalType value)
	{
		m_signalType = value;

		if (function() == E::SignalFunction::Input || function() == E::SignalFunction::Output)
		{
			bool analogSignalProps = false;
			bool busSignalProps = false;

			switch (m_signalType)
			{
			case E::SignalType::Analog:
				analogSignalProps = true;
				break;
			case E::SignalType::Discrete:
				break;
			case E::SignalType::Bus:
				busSignalProps = true;
				break;
			default:
				Q_ASSERT(false);
			}

			bool propertiesWereChanged = false;

			if (auto p = propertyByCaption(PropertyNames::appSignalDataFormat); p != nullptr && p->visible() != analogSignalProps)
			{
				p->setVisible(analogSignalProps);
				propertiesWereChanged = true;
			}
			else
			{
				Q_ASSERT(p);
			}

			if (auto p = propertyByCaption(PropertyNames::appSignalBusTypeId); p != nullptr && p->visible() != busSignalProps)
			{
				p->setVisible(busSignalProps);
				propertiesWereChanged = true;
			}
			else
			{
				Q_ASSERT(p);
			}

			if (propertiesWereChanged == true)
			{
				emit propertyListChanged();
			}
		}
	}

	E::SignalFunction DeviceAppSignal::function() const
	{
		return m_function;
	}

	void DeviceAppSignal::setFunction(E::SignalFunction value)
	{
		m_function = value;
	}

	E::ByteOrder DeviceAppSignal::byteOrder() const
	{
		return m_byteOrder;
	}

	void DeviceAppSignal::setByteOrder(E::ByteOrder value)
	{
		m_byteOrder = value;
	}

	E::DataFormat DeviceAppSignal::format() const
	{
		return m_format;
	}

	void DeviceAppSignal::setFormat(E::DataFormat value)
	{
		m_format = value;
	}

	E::MemoryArea DeviceAppSignal::memoryArea() const
	{
		return m_memoryArea;
	}

	void DeviceAppSignal::setMemoryArea(E::MemoryArea value)
	{
		m_memoryArea = value;
	}

	int DeviceAppSignal::size() const
	{
		return m_size;
	}

	void DeviceAppSignal::setSize(int value)
	{
		m_size = value;
	}

	int DeviceAppSignal::valueOffset() const
	{
		return m_valueOffset;
	}

	void DeviceAppSignal::setValueOffset(int value)
	{
		m_valueOffset = value;
	}

	int DeviceAppSignal::valueBit() const
	{
		return m_valueBit;
	}

	void DeviceAppSignal::setValueBit(int value)
	{
		m_valueBit = value;
	}

	QString DeviceAppSignal::validitySignalId() const
	{
		return m_validitySignalId;
	}

	void DeviceAppSignal::setValiditySignalId(QString value)
	{
		m_validitySignalId = value.trimmed();
	}

	bool DeviceAppSignal::isInputSignal() const
	{
		return m_function == E::SignalFunction::Input;
	}

	bool DeviceAppSignal::isOutputSignal() const
	{
		return m_function == E::SignalFunction::Output;
	}

	bool DeviceAppSignal::isValiditySignal() const
	{
		return m_function == E::SignalFunction::Validity;
	}

	bool DeviceAppSignal::isDiagSignal() const
	{
		return m_function == E::SignalFunction::Diagnostics;
	}

	bool DeviceAppSignal::isSoftwareCalculatedSignal() const
	{
		return m_function == E::SignalFunction::SoftwareCalculated;
	}

	bool DeviceAppSignal::isAnalogSignal() const
	{
		return m_signalType == E::SignalType::Analog;
	}

	bool DeviceAppSignal::isDiscreteSignal() const
	{
		return m_signalType == E::SignalType::Discrete;
	}

	int DeviceAppSignal::appSignalLowAdc() const
	{
		return m_appSignalLowAdc;
	}

	void DeviceAppSignal::setAppSignalLowAdc(int value)
	{
		m_appSignalLowAdc = value;
	}

	int DeviceAppSignal::appSignalHighAdc() const
	{
		return m_appSignalHighAdc;
	}

	void DeviceAppSignal::setAppSignalHighAdc(int value)
	{
		m_appSignalHighAdc = value;
	}

	double DeviceAppSignal::appSignalLowEngUnits() const
	{
		return m_appSignalLowEngUnits;
	}

	void DeviceAppSignal::setAppSignalLowEngUnits(double value)
	{
		m_appSignalLowEngUnits = value;
	}

	double DeviceAppSignal::appSignalHighEngUnits() const
	{
		return m_appSignalHighEngUnits;
	}

	void DeviceAppSignal::setAppSignalHighEngUnits(double value)
	{
		m_appSignalHighEngUnits = value;
	}

	E::AnalogAppSignalFormat DeviceAppSignal::appSignalDataFormat() const
	{
		return m_appSignalDataFormat;
	}

	void DeviceAppSignal::setAppSignalDataFormat(E::AnalogAppSignalFormat value)
	{
		m_appSignalDataFormat = value;
	}

	QString DeviceAppSignal::appSignalBusTypeId() const
	{
		return m_appSignalBusTypeId;
	}

	void DeviceAppSignal::setAppSignalBusTypeId(QString value)
	{
		m_appSignalBusTypeId = value;
	}

	QString DeviceAppSignal::signalSpecPropsStruct() const
	{
		return m_signalSpecPropsStruct;
	}

	void DeviceAppSignal::setSignalSpecPropsStruct(QString value)
	{
		m_signalSpecPropsStruct = value;
	}
} // namespace Hardware
