#include "DiagSignal.h"
#include "DiagSignalType.h"


namespace Hardware
{
	//
	//
	// DiagSignal
	//
	//
	DiagSignal::DiagSignal(bool preset /*= false*/, QObject* parent /*= nullptr*/) noexcept :
		DeviceObject(DeviceType::DiagSignal, preset, parent)
	{
	}

	void DiagSignal::propertyDemand(const QString& prop)
	{
		DeviceObject::propertyDemand(prop);

		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::diagSignalTypeId, PropertyNames::categoryDiagSignal, true, DiagSignal::signalTypeId, DiagSignal::setSignalTypeId)
			->setUpdateFromPreset(true)
			.setExpert(isPreset());

		ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::valueOffset, PropertyNames::categoryDiagSignal, true, DiagSignal::valueOffset, DiagSignal::setValueOffset)
			->setUpdateFromPreset(true)
			.setExpert(isPreset());

		ADD_PROPERTY_GET_SET_CAT(int, PropertyNames::valueBit, PropertyNames::categoryDiagSignal, true, DiagSignal::valueBit, DiagSignal::setValueBit)
			->setUpdateFromPreset(true)
			.setExpert(isPreset());

		ADD_PROPERTY_GET_SET_CAT(QString, PropertyNames::validitySignalId, PropertyNames::categoryDiagSignal, true, DiagSignal::validitySignalId, DiagSignal::setValiditySignalId)
			->setUpdateFromPreset(true)
			.setExpert(isPreset());

		//		auto typeProp = addProperty<E::SignalType, DeviceAppSignal, &DeviceAppSignal::signalType, &DeviceAppSignal::setSignalType>(PropertyNames::type, QLatin1String(), true);
		//		auto functionProp = addProperty<E::SignalFunction, DeviceAppSignal, &DeviceAppSignal::function, &DeviceAppSignal::setFunction>(PropertyNames::function, QLatin1String(), true);
		//		auto byteOrderProp = addProperty<E::ByteOrder, DeviceAppSignal, &DeviceAppSignal::byteOrder, &DeviceAppSignal::setByteOrder>(PropertyNames::byteOrder, QLatin1String(), true);
		//		auto formatProp = addProperty<E::DataFormat, DeviceAppSignal, &DeviceAppSignal::format, &DeviceAppSignal::setFormat>(PropertyNames::format, QLatin1String(), true);
		//		auto memoryAreaProp = addProperty<E::MemoryArea, DeviceAppSignal, &DeviceAppSignal::memoryArea, &DeviceAppSignal::setMemoryArea>(PropertyNames::memoryArea, QLatin1String(), true);
		//		auto sizeProp = addProperty<int, DeviceAppSignal, &DeviceAppSignal::size, &DeviceAppSignal::setSize>(PropertyNames::size, QLatin1String(), true);
		//		auto signalSpecPropsStructProp = addProperty<QString, DeviceAppSignal, &DeviceAppSignal::signalSpecPropsStruct, &DeviceAppSignal::setSignalSpecPropsStruct>(PropertyNames::signalSpecificProperties, PropertyNames::categoryAppSignal, true);

		//		typeProp->setUpdateFromPreset(true);
		//		typeProp->setExpert(m_preset);

		//		functionProp->setUpdateFromPreset(true);
		//		functionProp->setExpert(m_preset);

		//		byteOrderProp->setUpdateFromPreset(true);
		//		byteOrderProp->setExpert(m_preset);

		//		formatProp->setUpdateFromPreset(true);
		//		formatProp->setExpert(m_preset);

		//		memoryAreaProp->setUpdateFromPreset(true);
		//		memoryAreaProp->setExpert(m_preset);

		//		sizeProp->setUpdateFromPreset(true);
		//		sizeProp->setExpert(m_preset);

		//		signalSpecPropsStructProp->setUpdateFromPreset(true);
		//		signalSpecPropsStructProp->setExpert(m_preset);
		//		signalSpecPropsStructProp->setSpecificEditor(E::PropertySpecificEditor::SpecificPropertyStruct);

		return;
	}

	bool DiagSignal::SaveData(Proto::Envelope* message, bool saveTree) const
	{
		bool result = DeviceObject::SaveData(message, saveTree);
		if (result == false || message->has_deviceobject() == false)
		{
			Q_ASSERT(result);
			Q_ASSERT(message->has_deviceobject());
			return false;
		}

		// --
		//
		::Proto::DeviceDiagSignal* signalMessage = message->mutable_deviceobject()->mutable_diagsignal();

		signalMessage->set_signaltypeid(m_signalTypeId.toUtf8());
		signalMessage->set_valueoffset(static_cast<int>(m_valueOffset));
		signalMessage->set_valuebit(static_cast<int>(m_valueBit));
		signalMessage->set_validitysignalid(m_validitySignalId.toUtf8());

		return true;
	}

	bool DiagSignal::LoadData(const Proto::Envelope& message)
	{
		if (message.has_deviceobject() == false)
		{
			Q_ASSERT(message.has_deviceobject());
			return false;
		}

		bool result = DeviceObject::LoadData(message);
		if (result == false)
		{
			return false;
		}

		// --
		//
		if (message.deviceobject().has_diagsignal() == false)
		{
			Q_ASSERT(message.deviceobject().has_diagsignal());
			return false;
		}

		const Proto::DeviceDiagSignal& signalMessage = message.deviceobject().diagsignal();

		m_signalTypeId = QString::fromUtf8(signalMessage.signaltypeid().data());
		m_valueOffset = signalMessage.valueoffset();
		m_valueBit = signalMessage.valuebit();
		m_validitySignalId = QString::fromUtf8(signalMessage.validitysignalid().data());

		if (isPreset() == true)
		{
			setExpertToProperty(PropertyNames::diagSignalTypeId, true);
			setExpertToProperty(PropertyNames::valueOffset, true);
			setExpertToProperty(PropertyNames::valueBit, true);
			setExpertToProperty(PropertyNames::validitySignalId, true);
		}

		return true;
	}

	void DiagSignal::expandEquipmentId()
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

	const QString& DiagSignal::signalTypeId() const
	{
		return m_signalTypeId;
	}

	void DiagSignal::setSignalTypeId(const QString& value)
	{
		m_signalTypeId = value;
	}

	int DiagSignal::valueOffset() const
	{
		return m_valueOffset;
	}

	void DiagSignal::setValueOffset(int value)
	{
		m_valueOffset = value;
	}

	int DiagSignal::valueBit() const
	{
		return m_valueBit;
	}

	void DiagSignal::setValueBit(int value)
	{
		m_valueBit = value;
	}

	const QString& DiagSignal::validitySignalId() const
	{
		return m_validitySignalId;
	}

	void DiagSignal::setValiditySignalId(const QString& value)
	{
		m_validitySignalId = value;
	}

	std::shared_ptr<Hardware::DiagSignalType> DiagSignal::diagSignalType() const
	{
		return m_diagSignalType;
	}

	void DiagSignal::setDiagSignalType(std::shared_ptr<DiagSignalType> value)
	{
		Q_ASSERT(m_signalTypeId == value->signalTypeId());
		m_diagSignalType = value;
	}
} // namespace Hardware