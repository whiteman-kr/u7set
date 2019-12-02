#include "../lib/MetrologySignal.h"

namespace Metrology
{

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	RackParam::RackParam(int index, const QString& equipmentID, const QString& caption) :
		m_index (index),
		m_equipmentID (equipmentID),
		m_caption (caption)
	{
		if (equipmentID.isEmpty() == false)
		{
			m_hash = calcHash(equipmentID);
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool RackParam::isValid() const
	{
		if (m_index == -1 || m_equipmentID.isEmpty() == true)
		{
			return false;
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void RackParam::clear()
	{
		m_index = -1;

		m_hash = 0;

		m_equipmentID.clear();
		m_caption.clear();
	}

	// -------------------------------------------------------------------------------------------------------------------

	void RackParam::setEquipmentID(const QString& equipmentID)
	{
		m_equipmentID = equipmentID;

		if (equipmentID.isEmpty() == true)
		{
			m_hash = 0;
			return;
		}

		m_hash = calcHash(equipmentID);
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString RackParam::channelStr() const
	{
		return m_channel == -1 ? QString() : QString::number(m_channel + 1);
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool RackParam::readFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		result &= xml.readIntAttribute("Index", &m_index);
		result &= xml.readStringAttribute("EquipmentID", &m_equipmentID);
		result &= xml.readStringAttribute("Caption", &m_caption);

		if (m_equipmentID.isEmpty() == false)
		{
			m_hash = calcHash(m_equipmentID);
		}

		return result;
	}


	// -------------------------------------------------------------------------------------------------------------------

	void RackParam::writeToXml(XmlWriteHelper& xml)
	{
		xml.writeStartElement("Rack");
		{
			xml.writeIntAttribute("Index", index());
			xml.writeStringAttribute("EquipmentID", equipmentID());
			xml.writeStringAttribute("Caption", caption());
		}
		xml.writeEndElement();
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	SignalLocation::SignalLocation(const QString& appSignalID, Hardware::DeviceObject* pDeviceObject)
	{
		if (pDeviceObject == nullptr)
		{
			assert(pDeviceObject);
			return;
		}

		setAppSignalID(appSignalID);
		setEquipmentID(pDeviceObject->equipmentId());

		getParentObject(pDeviceObject);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalLocation::clear()
	{
		m_appSignalID.clear();
		m_equipmentID.clear();

		m_rack.clear();
		m_chassis = -1;
		m_moduleID.clear();
		m_module = -1;
		m_place = -1;

		m_contact.clear();
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalLocation::getParentObject(Hardware::DeviceObject* pDeviceObject)
	{
		if (pDeviceObject == nullptr || pDeviceObject->isRoot() == true)
		{
			return;
		}

		switch(pDeviceObject->deviceType())
		{
			case Hardware::DeviceType::Rack:
				rack().setEquipmentID(pDeviceObject->equipmentId());
				break;

			case Hardware::DeviceType::Chassis:
				setChassis(pDeviceObject->place());
				break;

			case Hardware::DeviceType::Module:
				setModuleID(pDeviceObject->equipmentId());
				setModule(pDeviceObject->place());
				break;

			case Hardware::DeviceType::Signal:
				setPlace(pDeviceObject->place());
				setContact(pDeviceObject->equipmentId().remove(pDeviceObject->parent()->equipmentId()));
				break;
		}

		getParentObject(pDeviceObject->parent());
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalLocation::rackCaption(bool showIndex) const
	{
		QString result = m_rack.caption();

		if (showIndex == true && m_rack.index() != -1)
		{
			result.insert(0, QString::number(m_rack.index() + 1) + " - ");
		}

		return result;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalLocation::chassisStr() const
	{
		return m_chassis == -1 ? QString("N/A") : QString::number(m_chassis);
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalLocation::moduleStr() const
	{
		return m_module == -1 ? QString("N/A") : QString::number(m_module);
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalLocation::placeStr() const
	{
		return m_place == -1 ? QString("N/A") : QString::number(m_place);
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool SignalLocation::readFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		if (xml.name() != "Signal")
		{
			return false;
		}

		QString rackID;

		result &= xml.readStringAttribute("AppSignalID", &m_appSignalID);
		result &= xml.readStringAttribute("EquipmentID", &m_equipmentID);

		result &= xml.readStringAttribute("RackID", &rackID);
		result &= xml.readIntAttribute("Chassis", &m_chassis);
		result &= xml.readStringAttribute("ModuleID", &m_moduleID);
		result &= xml.readIntAttribute("Module", &m_module);
		result &= xml.readIntAttribute("Place", &m_place);
		result &= xml.readStringAttribute("Contact", &m_contact);

		rack().setEquipmentID(rackID);

		return result;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalLocation::writeToXml(XmlWriteHelper& xml)
	{
		xml.writeStringAttribute("AppSignalID", appSignalID());				// signal appSignalID
		xml.writeStringAttribute("EquipmentID", equipmentID());				// signal equipmentID

		xml.writeStringAttribute("RackID", rack().equipmentID());			// signal rack		(other info about rack ref. in RackParam by rack hash)
		xml.writeIntAttribute("Chassis", chassis());						// signal chassis
		xml.writeStringAttribute("ModuleID", moduleID());					// signal module EquipmentID
		xml.writeIntAttribute("Module", module());							// signal module
		xml.writeIntAttribute("Place", place());							// signal place
		xml.writeStringAttribute("Contact", contact());						// signal contact for input: _IN00A or _IN00B, for output: only _OUT00
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	bool SignalParam::isValid() const
	{
		if (appSignalID().isEmpty() == true || hash() == 0)
		{
			return false;
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalParam::setAppSignalID(const QString& appSignalID)
	{
		Signal::setAppSignalID(appSignalID);

		if (appSignalID.isEmpty() == true)
		{
			setHash(UNDEFINED_HASH);
			return;
		}

		setHash(calcHash(appSignalID));
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalParam::setParam(const ::Signal& signal, const SignalLocation& location)
	{
		m_location = location;

		// init empty electricUnits
		//
		m_electricLowLimit = 0;
		m_electricHighLimit = 0;
		m_electricUnitID = E::ElectricUnit::NoUnit;
		m_electricSensorType = E::SensorType::NoSensor;
		m_electricR0 = 0;
		m_electricPrecision = 0;

		// set electricUnits and physicalUnits
		//
		m_physicalLowLimit = 0;
		m_physicalHighLimit = 0;

		if (signal.isAnalog() == true)
		{
			UnitsConvertor uc;

			UnitsConvertResult qpl;
			UnitsConvertResult qph;

			switch (signal.inOutType())
			{
				case E::SignalInOutType::Input:

					if (signal.isSpecPropExists(SignalProperties::electricLowLimitCaption) == false || signal.isSpecPropExists(SignalProperties::electricHighLimitCaption) == false)
					{
						break;
					}

					m_electricLowLimit = signal.electricLowLimit();
					m_electricHighLimit = signal.electricHighLimit();
					m_electricPrecision = 4;

					if (signal.isSpecPropExists(SignalProperties::electricUnitCaption) == false || signal.isSpecPropExists(SignalProperties::sensorTypeCaption) == false)
					{
						break;
					}

					m_electricUnitID = signal.electricUnit();
					m_electricSensorType = signal.sensorType();

					switch (signal.electricUnit())
					{
						case E::ElectricUnit::V:
						case E::ElectricUnit::mA:
							{
								qpl = uc.electricToPhysical_Input(signal.electricLowLimit(), signal.electricLowLimit(), signal.electricHighLimit(), m_electricUnitID, m_electricSensorType);
								qph = uc.electricToPhysical_Input(signal.electricHighLimit(), signal.electricLowLimit(), signal.electricHighLimit(), m_electricUnitID, m_electricSensorType);
							}
							break;

						case E::ElectricUnit::mV:
							{
								qpl = uc.electricToPhysical_ThermoCouple(signal.electricLowLimit(), signal.electricLowLimit(), signal.electricHighLimit(), m_electricUnitID, m_electricSensorType);
								qph = uc.electricToPhysical_ThermoCouple(signal.electricHighLimit(), signal.electricLowLimit(), signal.electricHighLimit(), m_electricUnitID, m_electricSensorType);
							}
							break;

						case E::ElectricUnit::Ohm:
							{
								QVariant qv;
								bool isEnum;

								if (signal.isSpecPropExists(SignalProperties::R0_OhmCaption) == true)
								{
									if (signal.getSpecPropValue(SignalProperties::R0_OhmCaption, &qv, &isEnum) == true)
									{
										m_electricR0 = qv.toDouble();
									}
								}

								qpl = uc.electricToPhysical_ThermoResistor(signal.electricLowLimit(), signal.electricLowLimit(), signal.electricHighLimit(), m_electricUnitID, m_electricSensorType, m_electricR0);
								qph = uc.electricToPhysical_ThermoResistor(signal.electricHighLimit(), signal.electricLowLimit(), signal.electricHighLimit(), m_electricUnitID, m_electricSensorType, m_electricR0);
							}
							break;
					}

					break;

				case E::SignalInOutType::Output:

					if (signal.isSpecPropExists(SignalProperties::electricLowLimitCaption) == false || signal.isSpecPropExists(SignalProperties::electricHighLimitCaption) == false)
					{
						break;
					}

					m_electricLowLimit = signal.electricLowLimit();
					m_electricHighLimit = signal.electricHighLimit();
					m_electricPrecision = 4;

					if (signal.isSpecPropExists(SignalProperties::electricUnitCaption) == false)
					{
						break;
					}

					m_electricUnitID = signal.electricUnit();

					if (signal.isSpecPropExists(SignalProperties::outputModeCaption) == false)
					{
						break;
					}

					qpl = uc.electricToPhysical_Output(signal.electricLowLimit(), signal.electricLowLimit(), signal.electricHighLimit(), m_electricUnitID, signal.outputMode());
					qph = uc.electricToPhysical_Output(signal.electricHighLimit(), signal.electricLowLimit(), signal.electricHighLimit(), m_electricUnitID, signal.outputMode());

					break;
			}

			if (qpl.ok() == true && qph.ok() == true)
			{
				m_physicalLowLimit = qpl.toDouble();
				m_physicalHighLimit = qph.toDouble();
			}
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalParam::updateParam(const SignalParam& param)
	{
		m_location = param.location();

		// electricUnits
		//

		m_electricLowLimit = param.electricLowLimit();
		m_electricHighLimit = param.electricHighLimit();
		m_electricUnitID = param.electricUnitID();
		m_electricSensorType = param.electricSensorType();
		m_electricR0 = param.electricR0();
		m_electricPrecision = param.electricPrecision();

		// physicalUnits
		//

		m_physicalLowLimit = param.physicalLowLimit();
		m_physicalHighLimit = param.physicalHighLimit();

		// if class have been created separately
		//
		//setSpecPropStruct("5;R0_Ohm;5 Electric parameters;double;0;200;100;2;false;false;R0 (R@0C, Ohm);true;None;65535");
		//createSpecPropValues();
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool SignalParam::readFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		if (xml.name() != "Signal")
		{
			return false;
		}

		int unit = 0;

		result &= m_location.readFromXml(xml);

		result &= xml.readDoubleAttribute("ElectricLowLimit", &m_electricLowLimit);
		result &= xml.readDoubleAttribute("ElectricHighLimit", &m_electricHighLimit);
		result &= xml.readIntAttribute("ElectricUnitID", &unit); m_electricUnitID = static_cast<E::ElectricUnit>(unit);
		result &= xml.readIntAttribute("ElectricSensorType", &unit); m_electricSensorType = static_cast<E::SensorType>(unit);
		result &= xml.readDoubleAttribute("ElectricR0", &m_electricR0);
		result &= xml.readIntAttribute("ElectricPrecision", &m_electricPrecision);

		result &= xml.readDoubleAttribute("PhysicalLowLimit", &m_physicalLowLimit);
		result &= xml.readDoubleAttribute("PhysicalHighLimit", &m_physicalHighLimit);

		return result;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalParam::writeToXml(XmlWriteHelper& xml)
	{
		xml.writeStartElement("Signal");
		{
			location().writeToXml(xml);

			xml.writeDoubleAttribute("ElectricLowLimit", electricLowLimit());
			xml.writeDoubleAttribute("ElectricHighLimit", electricHighLimit());
			xml.writeIntAttribute("ElectricUnitID", electricUnitID());
			xml.writeIntAttribute("ElectricSensorType", electricSensorType());
			xml.writeDoubleAttribute("ElectricR0", electricR0());
			xml.writeIntAttribute("ElectricPrecision", electricPrecision());

			xml.writeDoubleAttribute("PhysicalLowLimit", physicalLowLimit());
			xml.writeDoubleAttribute("PhysicalHighLimit", physicalHighLimit());
		}
		xml.writeEndElement();
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::adcRangeStr(bool showHex) const
	{
		QString range;

		if (showHex == false)
		{
			range.sprintf("%d .. %d", lowADC(), highADC());
		}
		else
		{
			range.sprintf("0x%04X .. 0x%04X", lowADC(), highADC());
		}

		return range;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::electricUnitStr() const
	{
		return QMetaEnum::fromType<E::ElectricUnit>().key(m_electricUnitID);
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::electricSensorTypeStr() const
	{
		QString typeStr = QMetaEnum::fromType<E::SensorType>().key(m_electricSensorType);

		if (m_electricUnitID == E::ElectricUnit::Ohm && m_electricSensorType != E::SensorType::Ohm_Raw)
		{
			typeStr += " " + electricR0Str();
		}

		return typeStr;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::electricR0Str() const
	{
		QString r0;
		r0.sprintf("R0=%0.2f", m_electricR0);
		return r0;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool SignalParam::electricRangeIsValid() const
	{
		if (m_electricLowLimit == 0.0 && m_electricHighLimit == 0.0)
		{
			return false;
		}

		if (m_electricUnitID == E::ElectricUnit::NoUnit)
		{
			return false;
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::electricRangeStr() const
	{
		QString range, formatStr;

		formatStr.sprintf("%%.%df", m_electricPrecision);

		range.sprintf(formatStr.toLocal8Bit() + " .. " + formatStr.toLocal8Bit(), m_electricLowLimit, m_electricHighLimit);

		QString unit = electricUnitStr();

		if (unit.isEmpty() == false)
		{
			range.append(" " + unit);
		}

		return range;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool SignalParam::physicalRangeIsValid() const
	{
		if (m_physicalLowLimit == 0.0 && m_physicalHighLimit == 0.0)
		{
			return false;
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::physicalRangeStr() const
	{
		QString range, formatStr;

		formatStr.sprintf("%%.%df", decimalPlaces());

		range.sprintf(formatStr.toLocal8Bit() + " .. " + formatStr.toLocal8Bit(), m_physicalLowLimit, m_physicalHighLimit);

		return range;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool SignalParam::engineeringRangeIsValid() const
	{
		if (lowEngineeringUnits() == 0.0 && highEngineeringUnits() == 0.0)
		{
			return false;
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::engineeringRangeStr() const
	{
		QString range, formatStr;

		formatStr.sprintf("%%.%df", decimalPlaces());

		range.sprintf(formatStr.toLocal8Bit() + " .. " + formatStr.toLocal8Bit(), lowEngineeringUnits(), highEngineeringUnits());

		if (unit().isEmpty() == false)
		{
			range.append(" " + unit());
		}

		return range;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::enableTuningStr() const
	{
		if (enableTuning() == false)
		{
			return QString();
		}

		return QString("Yes");
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::tuningDefaultValueStr() const
	{
		if (enableTuning() == false)
		{
			return QString();
		}

		QString stateStr, formatStr;

		switch (signalType())
		{
			case E::SignalType::Analog:

				formatStr.sprintf("%%.%df", decimalPlaces());

				stateStr.sprintf(formatStr.toLocal8Bit(), tuningDefaultValue().toDouble());

				break;

			case E::SignalType::Discrete:

				stateStr = tuningDefaultValue().toDouble() == 0.0 ? QString("No") : QString("Yes");

				break;

			default:
				assert(0);
		}

		return stateStr;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool SignalParam::tuningRangeIsValid() const
	{
		if (enableTuning() == false)
		{
			return true;
		}

		if (tuningLowBound().toDouble() == 0.0 && tuningHighBound().toDouble() == 0.0)
		{
			return false;
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::tuningRangeStr() const
	{
		if (enableTuning() == false)
		{
			return QString();
		}

		QString range, formatStr;

		formatStr.sprintf("%%.%df", decimalPlaces());

		range.sprintf(formatStr.toLocal8Bit() + " .. " + formatStr.toLocal8Bit(), tuningLowBound().toDouble(), tuningHighBound().toDouble());

		return range;
	}

	// -------------------------------------------------------------------------------------------------------------------

	TuningValueType	SignalParam::tuningValueType() const
	{
		TuningValueType type = TuningValueType::Float;

		switch (signalType())
		{
			case E::SignalType::Analog:

				switch (analogSignalFormat())
				{
					case E::AnalogAppSignalFormat::SignedInt32:	type = TuningValueType::SignedInt32;	break;
					case E::AnalogAppSignalFormat::Float32:		type = TuningValueType::Float;			break;
				}

				break;

			case E::SignalType::Discrete:						type = TuningValueType::Discrete;		break;
		}

		return type;
	}

	void SignalParam::setComparatorList(const QVector<std::shared_ptr<Comparator>>& comparators)
	{
		m_comparatorList = comparators;
		m_comparatorCount = m_comparatorList.count();
	}

	std::shared_ptr<Comparator> SignalParam::comparator(int index) const
	{
		if (index < 0 || index >= m_comparatorCount)
		{
			return nullptr;
		}

		return m_comparatorList[index];
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	QString SignalStatistic::measureCountStr() const
	{
		if (m_measureCount == 0)
		{
			return QString();
		}

		return QString::number(m_measureCount);
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalStatistic::stateStr() const
	{
		if (m_measureCount == 0)
		{
			return QString("Not measured");
		}

		if (m_state < 0 || m_state >= StatisticStateCount)
		{
			return QString();
		}

		return StatisticState[m_state];
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
}
