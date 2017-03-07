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
				setContact( pDeviceObject->equipmentId().remove( pDeviceObject->parent()->equipmentId() ) );
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

		xml.writeStringAttribute("RackID", rack().equipmentID());		// signal rack      ( other info about rack ref. in RackParam by rack hash)
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

	void SignalParam::setParam(const Signal& signal, const SignalLocation& location)
	{
		m_appSignalID = signal.appSignalID();
		m_customAppSignalID = signal.customAppSignalID();
		m_caption = signal.caption();

		m_signalType = signal.signalType();
		m_inOutType = signal.inOutType();

		m_location = location;

		m_lowADC = signal.lowADC();
		m_highADC = signal.highADC();

		m_inputElectricLowLimit = signal.inputLowLimit();
		m_inputElectricHighLimit = signal.inputHighLimit();
		m_inputElectricUnitID = signal.inputUnitID();
		m_inputElectricSensorType = signal.inputSensorType();
		m_inputElectricPrecision = 3;

		m_inputPhysicalLowLimit = signal.lowEngeneeringUnits();
		m_inputPhysicalHighLimit = signal.highEngeneeringUnits();
		m_inputPhysicalUnitID = signal.unitID();
		m_inputPhysicalPrecision = signal.decimalPlaces();

		switch(signal.outputMode())
		{
			case E::OutputMode::Plus0_Plus5_V:		m_outputElectricLowLimit = 0;	m_outputElectricHighLimit = 5;	m_outputElectricUnitID = E::InputUnit::V;	break;
			case E::OutputMode::Plus4_Plus20_mA:	m_outputElectricLowLimit = 4;	m_outputElectricHighLimit = 20;	m_outputElectricUnitID = E::InputUnit::mA;	break;
			case E::OutputMode::Minus10_Plus10_V:	m_outputElectricLowLimit = -10;	m_outputElectricHighLimit = 10;	m_outputElectricUnitID = E::InputUnit::V;	break;
			case E::OutputMode::Plus0_Plus5_mA:		m_outputElectricLowLimit = 0;	m_outputElectricHighLimit = 5;	m_outputElectricUnitID = E::InputUnit::mA;	break;
			default:								assert(0);
		}
		m_outputElectricSensorType = signal.outputSensorType();
		m_outputElectricPrecision = 3;

		m_outputPhysicalLowLimit = signal.outputLowLimit();
		m_outputPhysicalHighLimit = signal.outputHighLimit();
		m_outputPhysicalUnitID = signal.outputUnitID();
		m_outputPhysicalPrecision = signal.decimalPlaces();

		m_enableTuning = signal.enableTuning();
		m_tuningDefaultValue = signal.tuningDefaultValue();
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

		result &= xml.readDoubleAttribute("InputElectricLowLimit", &m_inputElectricLowLimit);
		result &= xml.readDoubleAttribute("InputElectricHighLimit", &m_inputElectricHighLimit);
		result &= xml.readIntAttribute("InputElectricUnitID", &type);
		m_inputElectricUnitID = static_cast<E::InputUnit>(type);
		result &= xml.readIntAttribute("InputElectricSensorType", &type);
		m_inputElectricSensorType = static_cast<E::SensorType>(type);
		result &= xml.readIntAttribute("InputElectricPrecision", &m_inputElectricPrecision);

		result &= xml.readDoubleAttribute("InputPhysicalLowLimit", &m_inputPhysicalLowLimit);
		result &= xml.readDoubleAttribute("InputPhysicalHighLimit", &m_inputPhysicalHighLimit);
		result &= xml.readIntAttribute("InputPhysicalUnitID", &m_inputPhysicalUnitID);
		result &= xml.readIntAttribute("InputPhysicalPrecision", &m_inputPhysicalPrecision);

		result &= xml.readDoubleAttribute("OutputElectricLowLimit", &m_outputElectricLowLimit);
		result &= xml.readDoubleAttribute("OutputElectricHighLimit", &m_outputElectricHighLimit);
		result &= xml.readIntAttribute("OutputElectricUnitID", &type);
		m_outputElectricUnitID = static_cast<E::InputUnit>(type);
		result &= xml.readIntAttribute("OutputElectricSensorType", &type);
		m_outputElectricSensorType = static_cast<E::SensorType>(type);
		result &= xml.readIntAttribute("OutputElectricPrecision", &m_outputElectricPrecision);

		result &= xml.readDoubleAttribute("OutputPhysicalLowLimit", &m_outputPhysicalLowLimit);
		result &= xml.readDoubleAttribute("OutputPhysicalHighLimit", &m_outputPhysicalHighLimit);
		result &= xml.readIntAttribute("OutputPhysicalUnitID", &m_outputPhysicalUnitID);
		result &= xml.readIntAttribute("OutputPhysicalPrecision", &m_outputPhysicalPrecision);

		result &= xml.readBoolAttribute("EnableTuning", &m_enableTuning);
		result &= xml.readDoubleAttribute("TuningDefaultValue", &m_tuningDefaultValue);

		return result;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalParam::writeToXml(XmlWriteHelper& xml)
	{
		xml.writeStartElement("Signal");    // <Signal>
		{
			xml.writeStringAttribute("AppSignalID", appSignalID());
			xml.writeStringAttribute("CustomAppSignalID", customAppSignalID());
			xml.writeStringAttribute("Caption", caption());

			xml.writeIntAttribute("SignalType", signalType());
			xml.writeIntAttribute("InOutType", TO_INT(inOutType()));

			location().writeToXml(xml);

			xml.writeIntAttribute("LowADC", lowADC());
			xml.writeIntAttribute("HighADC", highADC());

			xml.writeDoubleAttribute("InputElectricLowLimit", inputElectricLowLimit());
			xml.writeDoubleAttribute("InputElectricHighLimit", inputElectricHighLimit());
			xml.writeIntAttribute("InputElectricUnitID", inputElectricUnitID());
			xml.writeIntAttribute("InputElectricSensorType", inputElectricSensorType());
			xml.writeIntAttribute("InputElectricPrecision", inputElectricPrecision());

			xml.writeDoubleAttribute("InputPhysicalLowLimit", inputPhysicalLowLimit());
			xml.writeDoubleAttribute("InputPhysicalHighLimit", inputPhysicalHighLimit());
			xml.writeIntAttribute("InputPhysicalUnitID", inputPhysicalUnitID());
			xml.writeIntAttribute("InputPhysicalPrecision", inputPhysicalPrecision());

			xml.writeDoubleAttribute("OutputElectricLowLimit", outputElectricLowLimit());
			xml.writeDoubleAttribute("OutputElectricHighLimit", outputElectricHighLimit());
			xml.writeIntAttribute("OutputElectricUnitID", outputElectricUnitID());
			xml.writeIntAttribute("OutputElectricSensorType", outputElectricSensorType());
			xml.writeIntAttribute("OutputElectricPrecision", outputElectricPrecision());

			xml.writeDoubleAttribute("OutputPhysicalLowLimit", outputPhysicalLowLimit());
			xml.writeDoubleAttribute("OutputPhysicalHighLimit", outputPhysicalHighLimit());
			xml.writeIntAttribute("OutputPhysicalUnitID", outputPhysicalUnitID());
			xml.writeIntAttribute("OutputPhysicalPrecision", outputPhysicalPrecision());

			xml.writeBoolAttribute("EnableTuning", enableTuning());
			xml.writeDoubleAttribute("TuningDefaultValue", tuningDefaultValue());
		}
		xml.writeEndElement();  // </Signal>
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

	QString SignalParam::inputElectricRangeStr() const
	{
		QString range, formatStr;

		formatStr.sprintf(("%%.%df"), m_inputElectricPrecision);

		range.sprintf(formatStr.toAscii() + " .. " + formatStr.toAscii(), m_inputElectricLowLimit, m_inputElectricHighLimit);

		if (m_inputElectricUnit.isEmpty() == false)
		{
			range.append(" " + m_inputElectricUnit);
		}

		return range;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::inputPhysicalRangeStr() const
	{
		QString range, formatStr;

		formatStr.sprintf(("%%.%df"), m_inputPhysicalPrecision);

		range.sprintf(formatStr.toAscii() + " .. " + formatStr.toAscii(), m_inputPhysicalLowLimit, m_inputPhysicalHighLimit);

		if (m_inputPhysicalUnit.isEmpty() == false)
		{
			range.append(" " + m_inputPhysicalUnit );
		}

		return range;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::outputElectricRangeStr() const
	{
		QString range, formatStr;

		formatStr.sprintf(("%%.%df"), m_outputElectricPrecision);

		range.sprintf(formatStr.toAscii() + " .. " + formatStr.toAscii(), m_outputElectricLowLimit, m_outputElectricHighLimit);

		if (m_outputElectricUnit.isEmpty() == false)
		{
			range.append(" " + m_outputElectricUnit);
		}

		return range;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::outputPhysicalRangeStr() const
	{
		QString range, formatStr;

		formatStr.sprintf(("%%.%df"), m_outputPhysicalPrecision );

		range.sprintf(formatStr.toAscii() + " .. " + formatStr.toAscii(), m_outputPhysicalLowLimit, m_outputPhysicalHighLimit);

		if (m_outputPhysicalUnit.isEmpty() == false)
		{
			range.append(" " + m_outputPhysicalUnit);
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

				formatStr.sprintf(("%%.%df"), m_inputPhysicalPrecision);

				stateStr.sprintf(formatStr.toAscii(), m_tuningDefaultValue);

				if (m_inputPhysicalUnit.isEmpty() == false)
				{
					stateStr.append(" " + m_inputPhysicalUnit);
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
}
