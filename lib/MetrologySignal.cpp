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

	SignalLocation::SignalLocation(Hardware::DeviceObject* pDeviceObject)
	{
		if (pDeviceObject == nullptr)
		{
			assert(pDeviceObject);
			return;
		}

		setEquipmentID(pDeviceObject->equipmentId());

		getParentObject(pDeviceObject);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalLocation::clear()
	{

		m_equipmentID.clear();

		m_rack.clear();
		m_chassis = -1;
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
		return m_chassis == -1 ? QString("N/A") : QString::number(m_chassis + 1);
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalLocation::moduleStr() const
	{
		return m_module == -1 ? QString("N/A") : QString::number(m_module + 1);
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalLocation::placeStr() const
	{
		return m_place == -1 ? QString("N/A") : QString::number(m_place + 1);
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

		result &= xml.readStringAttribute("EquipmentID", &m_equipmentID);

		result &= xml.readStringAttribute("RackID", &rackID);
		result &= xml.readIntAttribute("Chassis", &m_chassis);
		result &= xml.readIntAttribute("Module", &m_module);
		result &= xml.readIntAttribute("Place", &m_place);

		result &= xml.readStringAttribute("Contact", &m_contact);

		rack().setEquipmentID(rackID);

		return result;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalLocation::writeToXml(XmlWriteHelper& xml)
	{
		xml.writeStringAttribute("EquipmentID", equipmentID());			// signal equipmentID

		xml.writeStringAttribute("RackID", rack().equipmentID());		// signal rack		(other info about rack ref. in RackParam by rack hash)
		xml.writeIntAttribute("Chassis", chassis());					// signal chassis
		xml.writeIntAttribute("Module", module());						// signal module
		xml.writeIntAttribute("Place", place());						// signal place

		xml.writeStringAttribute("Contact", contact());					// signal contact for input: _IN00A or _IN00B, for output: only _OUT00
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	bool SignalParam::isValid() const
	{
		if (m_appSignalID.isEmpty() == true || m_hash == 0)
		{
			return false;
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalParam::setAppSignalID(const QString& appSignalID)
	{
		m_appSignalID = appSignalID;

		if (appSignalID.isEmpty() == true)
		{
			m_hash = 0;
			return;
		}

		m_hash = calcHash(appSignalID);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalParam::setParam(const ::Signal& signal, const SignalLocation& location)
	{
		m_appSignalID = signal.appSignalID();
		m_customAppSignalID = signal.customAppSignalID();
		m_caption = signal.caption();

		m_signalType = signal.signalType();
		m_inOutType = signal.inOutType();

		m_location = location;

		m_lowADC = signal.lowADC();
		m_highADC = signal.highADC();

		m_physicalLowLimit = signal.lowEngeneeringUnits();
		m_physicalHighLimit = signal.highEngeneeringUnits();
		m_physicalUnit = signal.unit();
		m_physicalPrecision = signal.decimalPlaces();

		switch(m_inOutType)
		{
			case E::SignalInOutType::Input:

				m_electricLowLimit = signal.electricLowLimit();
				m_electricHighLimit = signal.electricHighLimit();
				m_electricUnitID = signal.electricUnit();

				m_electricSensorType = signal.sensorType();
				m_electricPrecision = 3;

				break;

			case E::SignalInOutType::Internal:

				m_electricLowLimit = 0;
				m_electricHighLimit = 0;
				m_electricUnitID = E::ElectricUnit::NoUnit;

				m_electricSensorType = E::SensorType::NoSensorType;
				m_electricPrecision = 0;

				break;

			case E::SignalInOutType::Output:

				switch(signal.outputMode())
				{
					case E::OutputMode::Plus0_Plus5_V:		m_electricLowLimit = 0;		m_electricHighLimit = 5;	m_electricUnitID = E::ElectricUnit::V;	break;
					case E::OutputMode::Plus4_Plus20_mA:	m_electricLowLimit = 4;		m_electricHighLimit = 20;	m_electricUnitID = E::ElectricUnit::mA;	break;
					case E::OutputMode::Minus10_Plus10_V:	m_electricLowLimit = -10;	m_electricHighLimit = 10;	m_electricUnitID = E::ElectricUnit::V;	break;
					case E::OutputMode::Plus0_Plus5_mA:		m_electricLowLimit = 0;		m_electricHighLimit = 5;	m_electricUnitID = E::ElectricUnit::mA;	break;
					default:								assert(0);
				}

				m_electricSensorType = signal.sensorType();
				m_electricPrecision = 3;

				break;

			default: assert(0);
		}

		m_enableTuning = signal.enableTuning();
		m_tuningDefaultValue = signal.tuningDefaultValue().toDouble();
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool SignalParam::readFromXml(XmlReadHelper& xml)
	{
		bool result = true;

		if (xml.name() != "Signal")
		{
			return false;
		}

		result &= xml.readStringAttribute("AppSignalID", &m_appSignalID);
		if (m_appSignalID.isEmpty() == true)
		{
			return false;
		}

		m_hash = calcHash(m_appSignalID);
		if (m_hash == 0)
		{
			return false;
		}

		result &= xml.readStringAttribute("CustomAppSignalID", &m_customAppSignalID);
		result &= xml.readStringAttribute("Caption", &m_caption);

		int type = 0;

		result &= xml.readIntAttribute("SignalType", &type);
		m_signalType = static_cast<E::SignalType>(type);

		result &= xml.readIntAttribute("InOutType", &type);
		m_inOutType  = static_cast<E::SignalInOutType>(type);

		result &= m_location.readFromXml(xml);

		result &= xml.readIntAttribute("LowADC", &m_lowADC);
		result &= xml.readIntAttribute("HighADC", &m_highADC);

		result &= xml.readDoubleAttribute("PhysicalLowLimit", &m_physicalLowLimit);
		result &= xml.readDoubleAttribute("PhysicalHighLimit", &m_physicalHighLimit);
		result &= xml.readStringAttribute("PhysicalUnit", &m_physicalUnit);
		result &= xml.readIntAttribute("PhysicalPrecision", &m_physicalPrecision);

		result &= xml.readDoubleAttribute("ElectricLowLimit", &m_electricLowLimit);
		result &= xml.readDoubleAttribute("ElectricHighLimit", &m_electricHighLimit);
		result &= xml.readIntAttribute("ElectricUnitID", &type);
		m_electricUnitID = static_cast<E::ElectricUnit>(type);
		result &= xml.readIntAttribute("ElectricSensorType", &type);
		m_electricSensorType = static_cast<E::SensorType>(type);
		result &= xml.readIntAttribute("ElectricPrecision", &m_electricPrecision);

		result &= xml.readBoolAttribute("EnableTuning", &m_enableTuning);
		result &= xml.readDoubleAttribute("TuningDefaultValue", &m_tuningDefaultValue);

		return result;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalParam::writeToXml(XmlWriteHelper& xml)
	{
		xml.writeStartElement("Signal");
		{
			xml.writeStringAttribute("AppSignalID", appSignalID());
			xml.writeStringAttribute("CustomAppSignalID", customAppSignalID());
			xml.writeStringAttribute("Caption", caption());

			xml.writeIntAttribute("SignalType", signalType());
			xml.writeIntAttribute("InOutType", TO_INT(inOutType()));

			location().writeToXml(xml);

			xml.writeIntAttribute("LowADC", lowADC());
			xml.writeIntAttribute("HighADC", highADC());

			xml.writeDoubleAttribute("PhysicalLowLimit", physicalLowLimit());
			xml.writeDoubleAttribute("PhysicalHighLimit", physicalHighLimit());
			xml.writeStringAttribute("PhysicalUnit", physicalUnit());
			xml.writeIntAttribute("PhysicalPrecision", physicalPrecision());

			xml.writeDoubleAttribute("ElectricLowLimit", electricLowLimit());
			xml.writeDoubleAttribute("ElectricHighLimit", electricHighLimit());
			xml.writeIntAttribute("ElectricUnitID", electricUnitID());
			xml.writeIntAttribute("ElectricSensorType", electricSensorType());
			xml.writeIntAttribute("ElectricPrecision", electricPrecision());

			xml.writeBoolAttribute("EnableTuning", enableTuning());
			xml.writeDoubleAttribute("TuningDefaultValue", tuningDefaultValue());
		}
		xml.writeEndElement();
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::adcRangeStr(bool showHex) const
	{
		QString range;

		if (showHex == false)
		{
			range.sprintf("%d .. %d", m_lowADC, m_highADC);
		}
		else
		{
			range.sprintf("0x%04X .. 0x%04X", m_lowADC, m_highADC);
		}

		return range;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool SignalParam::physicalRangeIsValid() const
	{
		if (m_physicalLowLimit == 0 && m_physicalHighLimit == 0)
		{
			return false;
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::physicalRangeStr() const
	{
		QString range, formatStr;

		formatStr.sprintf("%%.%df", m_physicalPrecision);

		range.sprintf(formatStr.toAscii() + " .. " + formatStr.toAscii(), m_physicalLowLimit, m_physicalHighLimit);

		if (m_physicalUnit.isEmpty() == false)
		{
			range.append(" " + m_physicalUnit);
		}

		return range;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool SignalParam::electricRangeIsValid() const
	{
		if (m_electricLowLimit == 0 && m_electricHighLimit == 0)
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

		range.sprintf(formatStr.toAscii() + " .. " + formatStr.toAscii(), m_electricLowLimit, m_electricHighLimit);

		if (m_electricUnit.isEmpty() == false)
		{
			range.append(" " + m_electricUnit);
		}

		return range;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::enableTuningStr() const
	{
		if (m_enableTuning == false)
		{
			return QString();
		}

		return QString("Yes");
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::tuningDefaultValueStr() const
	{
		if (m_enableTuning == false)
		{
			return QString();
		}

		QString stateStr, formatStr;

		switch (m_signalType)
		{
			case E::SignalType::Analog:

				formatStr.sprintf("%%.%df", m_physicalPrecision);

				stateStr.sprintf(formatStr.toAscii(), m_tuningDefaultValue);

				if (m_physicalUnit.isEmpty() == false)
				{
					stateStr.append(" " + m_physicalUnit);
				}

				break;

			case E::SignalType::Discrete:

				stateStr = m_tuningDefaultValue == 0 ? QString("No") : QString("Yes");

				break;

			default:
				assert(0);
		}

		return stateStr;
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
