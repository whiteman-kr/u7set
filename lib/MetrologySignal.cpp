#include "../lib/MetrologySignal.h"
#include "../lib/UnitsConvertor.h"

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

	SignalLocation::SignalLocation(Hardware::DeviceObject* pDeviceObject, bool shownOnSchemas)
		: m_shownOnSchemas(shownOnSchemas)
	{
		if (pDeviceObject == nullptr)
		{
			assert(pDeviceObject);
			return;
		}

		getParentObject(pDeviceObject);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalLocation::clear()
	{
		m_rack.clear();

		m_chassisID.clear();
		m_chassis = -1;

		m_moduleID.clear();
		m_module = -1;

		m_place = -1;
		m_contact.clear();

		m_shownOnSchemas = false;

		m_moduleSerialNoID.clear();
		m_moduleSerialNo = 0;
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
				setChassisID(pDeviceObject->equipmentId());
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

	QString SignalLocation::shownOnSchemasStr() const
	{
		return m_shownOnSchemas == true ? QString("Yes") : QString();
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString	SignalLocation::moduleSerialNoStr() const
	{
		return m_moduleSerialNo == 0 ? QString("N/A") : QString::number(m_moduleSerialNo);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalLocation::serializeTo(Proto::MetrologySignalLocation *l) const
	{
		if (l == nullptr)
		{
			assert(false);
			return;
		}

		l->set_rackid(m_rack.equipmentID().toStdString());

		l->set_chassisid(m_chassisID.toStdString());
		l->set_chassis(m_chassis);

		l->set_moduleid(m_moduleID.toStdString());
		l->set_module(m_module);

		l->set_place(m_place);
		l->set_contact(m_contact.toStdString());

		l->set_shownonschemas(m_shownOnSchemas);
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool SignalLocation::serializeFrom(const Proto::MetrologySignalLocation& l)
	{
		m_rack.setEquipmentID(QString::fromStdString(l.rackid()));

		m_chassisID = QString::fromStdString(l.chassisid());
		m_chassis = l.chassis();

		m_moduleID = QString::fromStdString(l.moduleid());
		m_module = l.module();

		m_place = l.place();
		m_contact = QString::fromStdString(l.contact());

		m_shownOnSchemas = l.shownonschemas();

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	SignalParam::SignalParam(const ::Signal& signal, const SignalLocation& location)
	{
		setParam(signal, location);
	}

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
		// init AppSignal
		//
		::Signal* pSignal = dynamic_cast<::Signal*>(this);
		if (pSignal == nullptr)
		{
			assert(false);
			return;
		}

		*pSignal = signal;

		// init location of signal
		//
		m_location = location;

		// init empty electricUnits
		//
		m_electricLowLimit = 0;
		m_electricHighLimit = 0;
		m_electricUnitID = E::ElectricUnit::NoUnit;
		m_electricSensorType = E::SensorType::NoSensor;
		m_electricRLoad = 0;
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

			if (signal.isInput() == true || signal.isOutput() == true)
			{
				if (signal.isSpecPropExists(SignalProperties::electricLowLimitCaption) == true && signal.isSpecPropExists(SignalProperties::electricHighLimitCaption) == true)
				{
					m_electricLowLimit = signal.electricLowLimit();
					m_electricHighLimit = signal.electricHighLimit();
					m_electricPrecision = 4;
				}

				if (signal.isSpecPropExists(SignalProperties::electricUnitCaption) == true)
				{
					m_electricUnitID = signal.electricUnit();
				}

				if (signal.isSpecPropExists(SignalProperties::sensorTypeCaption) == true)
				{
					m_electricSensorType = signal.sensorType();
				}

				switch (signal.inOutType())
				{
					case E::SignalInOutType::Input:

						switch (m_electricUnitID)
						{
							case E::ElectricUnit::V:

								qpl = uc.electricToPhysical_Input(signal.electricLowLimit(), signal.electricLowLimit(), signal.electricHighLimit(), m_electricUnitID, m_electricSensorType, m_electricRLoad);
								qph = uc.electricToPhysical_Input(signal.electricHighLimit(), signal.electricLowLimit(), signal.electricHighLimit(), m_electricUnitID, m_electricSensorType, m_electricRLoad);

								break;

							case E::ElectricUnit::mA:

								if (signal.isSpecPropExists(SignalProperties::rload_OhmCaption) == true)
								{
									m_electricRLoad = signal.rload_Ohm();
								}

								qpl = uc.electricToPhysical_Input(signal.electricLowLimit(), signal.electricLowLimit(), signal.electricHighLimit(), m_electricUnitID, m_electricSensorType, m_electricRLoad);
								qph = uc.electricToPhysical_Input(signal.electricHighLimit(), signal.electricLowLimit(), signal.electricHighLimit(), m_electricUnitID,m_electricSensorType, m_electricRLoad);

								break;

							case E::ElectricUnit::mV:

								qpl = uc.electricToPhysical_ThermoCouple(signal.electricLowLimit(), signal.electricLowLimit(), signal.electricHighLimit(), m_electricUnitID, m_electricSensorType);
								qph = uc.electricToPhysical_ThermoCouple(signal.electricHighLimit(), signal.electricLowLimit(), signal.electricHighLimit(), m_electricUnitID, m_electricSensorType);

								break;

							case E::ElectricUnit::Ohm:

								m_electricR0 = uc.r0_from_signal(signal);

								qpl = uc.electricToPhysical_ThermoResistor(signal.electricLowLimit(), signal.electricLowLimit(), signal.electricHighLimit(), m_electricUnitID, m_electricSensorType, m_electricR0);
								qph = uc.electricToPhysical_ThermoResistor(signal.electricHighLimit(), signal.electricLowLimit(), signal.electricHighLimit(), m_electricUnitID, m_electricSensorType, m_electricR0);

								break;
						}

						break;

					case E::SignalInOutType::Output:

						if (signal.isSpecPropExists(SignalProperties::outputModeCaption) == false)
						{
							break;
						}

						qpl = uc.electricToPhysical_Output(signal.electricLowLimit(), signal.electricLowLimit(), signal.electricHighLimit(), m_electricUnitID, signal.outputMode());
						qph = uc.electricToPhysical_Output(signal.electricHighLimit(), signal.electricLowLimit(), signal.electricHighLimit(), m_electricUnitID, signal.outputMode());

						break;
				}
			}

			if (qpl.ok() == true && qph.ok() == true)
			{
				m_physicalLowLimit = qpl.toDouble();
				m_physicalHighLimit = qph.toDouble();
			}
			else
			{
				m_physicalLowLimit = m_electricLowLimit;
				m_physicalHighLimit = m_electricHighLimit;
			}
		}
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalParam::serializeTo(Proto::MetrologySignal *ms) const
	{
		if (ms == nullptr)
		{
			assert(false);
			return;
		}

		const Signal* pSignal = dynamic_cast<const Signal*>(this);
		if (pSignal == nullptr)
		{
			assert(false);
			return;
		}

		Proto::AppSignal* protoAppSignal = ms->mutable_appsignal();
		pSignal->serializeTo(protoAppSignal);

		Proto::MetrologySignalLocation* protoLocation = ms->mutable_location();
		m_location.serializeTo(protoLocation);

		ms->set_electriclowlimit(m_electricLowLimit);
		ms->set_electrichighlimit(m_electricHighLimit);

		ms->set_electricunitid(TO_INT(m_electricUnitID));
		ms->set_electricsensortype(TO_INT(m_electricSensorType));
		ms->set_electricrload(m_electricRLoad);

		ms->set_electricr0(m_electricR0);
		ms->set_electricprecision(m_electricPrecision);

		ms->set_physicallowlimit(m_physicalLowLimit);
		ms->set_physicalhighlimit(m_physicalHighLimit);
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool SignalParam::serializeFrom(const Proto::MetrologySignal& ms)
	{
		Signal* pSignal = dynamic_cast<Signal*>(this);
		if (pSignal == nullptr)
		{
			assert(false);
			return false;
		}

		const Proto::AppSignal& protoAppSignal = ms.appsignal();
		pSignal->serializeFrom(protoAppSignal);

		const Proto::MetrologySignalLocation& protoLocation = ms.location();
		m_location.serializeFrom(protoLocation);

		m_electricLowLimit = ms.electriclowlimit();
		m_electricHighLimit = ms.electrichighlimit();

		m_electricUnitID = static_cast<E::ElectricUnit>(ms.electricunitid());
		m_electricSensorType = static_cast<E::SensorType>(ms.electricsensortype());
		m_electricRLoad = ms.electricrload();

		m_electricR0 = ms.electricr0();
		m_electricPrecision = ms.electricprecision();

		m_physicalLowLimit = ms.physicallowlimit();
		m_physicalHighLimit = ms.physicalhighlimit();

		return true;
	}
	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::signalTypeStr() const
	{
		int type = inOutTypeInt();
		if (type < 0 || type >= SIGANL_TYPE_COUNT)
		{
			return QString();
		}

		return  qApp->translate("MeasureSignal.h", SignalTypeStr[type]);
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::adcRangeStr(bool showHex) const
	{
		QString range;

		if (showHex == false)
		{
			range = QString::asprintf("%d .. %d", lowADC(), highADC());
		}
		else
		{
			range = QString::asprintf("0x%04X .. 0x%04X", lowADC(), highADC());
		}

		return range;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalParam::setElectricLowLimit(double lowLimit)
	{
		m_electricLowLimit = lowLimit;

		if (isSpecPropExists(SignalProperties::electricLowLimitCaption) == false)
		{
			return;
		}

		Signal::setElectricLowLimit(lowLimit);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalParam::setElectricHighLimit(double highLimit)
	{
		m_electricHighLimit = highLimit;

		if (isSpecPropExists(SignalProperties::electricHighLimitCaption) == false)
		{
			return;
		}

		Signal::setElectricHighLimit(highLimit);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalParam::setElectricUnitID(E::ElectricUnit unitID)
	{
		m_electricUnitID = unitID;

		if (isSpecPropExists(SignalProperties::electricUnitCaption) == false)
		{
			return;
		}

		Signal::setElectricUnit(unitID);
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::electricUnitStr() const
	{
		return QMetaEnum::fromType<E::ElectricUnit>().key(m_electricUnitID);
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalParam::setElectricSensorType(E::SensorType sensorType)
	{
		m_electricSensorType = sensorType;

		if (isSpecPropExists(SignalProperties::sensorTypeCaption) == false)
		{
			return;
		}

		Signal::setSensorType(sensorType);
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::electricSensorTypeStr() const
	{
		if (isInternal() == true)
		{
			return QString();
		}

		QString typeStr = QMetaEnum::fromType<E::SensorType>().key(m_electricSensorType);

		switch(m_electricUnitID)
		{
			case E::ElectricUnit::mA:

				if (m_electricSensorType == E::SensorType::V_0_5)
				{
					typeStr += " " + electricRLoadStr();
				}

				break;

			case E::ElectricUnit::Ohm:

				if (	m_electricSensorType != E::SensorType::NoSensor && m_electricSensorType != E::SensorType::Ohm_Raw &&
						m_electricSensorType != E::SensorType::Ohm_Pt21 && m_electricSensorType != E::SensorType::Ohm_Cu23)
				{
					typeStr += " " + electricR0Str();
				}

				break;
		}


		return typeStr;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalParam::setElectricRLoad(double rload)
	{
		m_electricRLoad = rload;

		if (isSpecPropExists(SignalProperties::rload_OhmCaption) == false)
		{
			return;
		}

		Signal::setRload_Ohm(rload);
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::electricRLoadStr() const
	{
		QString r0;
		r0 = QString::asprintf("R=%0.0f", m_electricRLoad);
		return r0;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalParam::setElectricR0(double r0)
	{
		m_electricR0 = r0;

		if (isSpecPropExists(SignalProperties::R0_OhmCaption) == false)
		{
			return;
		}

		Signal::setR0_Ohm(r0);
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString SignalParam::electricR0Str() const
	{
		QString r0;
		r0 = QString::asprintf("R0=%0.2f", m_electricR0);
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
		if (isInternal() == true)
		{
			return QString();
		}

		QString range, formatStr;

		formatStr = QString::asprintf("%%.%df", m_electricPrecision);

		range = QString::asprintf(formatStr.toLocal8Bit() + " .. " + formatStr.toLocal8Bit(), m_electricLowLimit, m_electricHighLimit);

		QString unit = electricUnitStr();

		if (unit.isEmpty() == false)
		{
			range.append(" " + unit);
		}

		return range;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool SignalParam::isLinearRange() const
	{
		if (	(m_electricUnitID == E::ElectricUnit::mV && m_electricSensorType != E::SensorType::mV_Raw_Mul_8 && m_electricSensorType != E::SensorType::mV_Raw_Mul_32) ||
				(m_electricUnitID == E::ElectricUnit::Ohm && m_electricSensorType != E::SensorType::Ohm_Raw) )
		{
			return false;	// for non-linear
		}

		return true;		// for linear
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

		formatStr = QString::asprintf("%%.%df", decimalPlaces());

		range = QString::asprintf(formatStr.toLocal8Bit() + " .. " + formatStr.toLocal8Bit(), m_physicalLowLimit, m_physicalHighLimit);

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

		formatStr = QString::asprintf("%%.%df", decimalPlaces());

		range = QString::asprintf(formatStr.toLocal8Bit() + " .. " + formatStr.toLocal8Bit(), lowEngineeringUnits(), highEngineeringUnits());

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

				formatStr = QString::asprintf("%%.%df", decimalPlaces());

				stateStr = QString::asprintf(formatStr.toLocal8Bit(), tuningDefaultValue().toDouble());

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

		formatStr = QString::asprintf("%%.%df", decimalPlaces());

		range = QString::asprintf(formatStr.toLocal8Bit() + " .. " + formatStr.toLocal8Bit(), tuningLowBound().toDouble(), tuningHighBound().toDouble());

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

	// -------------------------------------------------------------------------------------------------------------------

	std::shared_ptr<ComparatorEx> SignalParam::comparator(int index) const
	{
		if (index < 0 || index >= m_comparatorCount)
		{
			return nullptr;
		}

		return m_comparatorList[index];
	}

	// -------------------------------------------------------------------------------------------------------------------

	void SignalParam::setComparatorList(const QVector<std::shared_ptr<ComparatorEx>>& comparators)
	{
		m_comparatorList = comparators;
		m_comparatorCount = m_comparatorList.count();
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	Signal::Signal(const SignalParam& param)
	{
		setParam(param);
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

	ComparatorEx::ComparatorEx(::Comparator* pComparator)
	{
		if (pComparator == nullptr)
		{
			return;
		}

		::Comparator* pBaseComparator =  dynamic_cast<::Comparator*>(this);
		if (pBaseComparator == nullptr)
		{
			return;
		}

		*pBaseComparator = *pComparator;
	}

	// -------------------------------------------------------------------------------------------------------------------

	void ComparatorEx::clear()
	{
		m_index = -1;

		m_inputSignal = nullptr;
		m_compareSignal = nullptr;
		m_hysteresisSignal = nullptr;
		m_outputSignal = nullptr;

		m_compareValue = 0;
		m_hysteresisValue = 0;
	}

	// -------------------------------------------------------------------------------------------------------------------

	bool ComparatorEx::signalsIsValid() const
	{
		if (m_inputSignal == nullptr || m_inputSignal->param().isValid() == false)
		{
			return false;
		}

		if (compare().isConst() == false)
		{
			if (m_compareSignal == nullptr || m_compareSignal->param().isValid() == false)
			{
				return false;
			}
		}

		if (hysteresis().isConst() == false)
		{
			if (m_hysteresisSignal == nullptr || m_hysteresisSignal->param().isValid() == false)
			{
				return false;
			}
		}

		if (m_outputSignal == nullptr || m_outputSignal->param().isValid() == false)
		{
			return false;
		}

		return true;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString ComparatorEx::cmpTypeStr() const
	{
		QString typeStr;

		switch (cmpType())
		{
			case E::CmpType::Greate:	typeStr = QString(">");	break;
			case E::CmpType::Less:		typeStr = QString("<");	break;
		}

		return typeStr;
	}

	// -------------------------------------------------------------------------------------------------------------------

	int ComparatorEx::valuePrecision() const
	{
		int result = 0;

		switch (inAnalogSignalFormat())
		{
			case E::AnalogAppSignalFormat::Float32:		result = precision();	break;
			case E::AnalogAppSignalFormat::SignedInt32:	result = 0;				break;
		}

		return result;
	}

	// -------------------------------------------------------------------------------------------------------------------

	double ComparatorEx::compareOnlineValue(int cmpValueType)
	{
		if (cmpValueType < 0 || cmpValueType >= CmpValueTypeCount)
		{
			return 0.0;
		}

		double hysteresisValue = hysteresisOnlineValue();	// get hysteresis value

		//
		//
		double deviation = 0;

		switch (m_deviationType)
		{
			case DeviationType::Unused:													// for comparators: Less and Greate

				if (cmpValueType == CmpValueTypeHysteresis)
				{
					switch (cmpType())
					{
						case E::CmpType::Less:		deviation = hysteresisValue;	break;
						case E::CmpType::Greate:	deviation -= hysteresisValue;	break;
					}
				}

				break;

			case DeviationType::Down:													// for comparators: Equal and NotEqual

				deviation = -hysteresisValue / 2;

				break;

			case DeviationType::Up:														// for comparators: Equal and NotEqual

				deviation = hysteresisValue / 2;

				break;
		}

		//
		//
		if (compare().isConst() == true)
		{
			m_compareValue = compare().constValue() + deviation;
		}
		else
		{
			if (m_compareSignal != nullptr)
			{
				if (m_compareSignal->param().isValid() == true && m_compareSignal->state().valid() == true)
				{
					m_compareValue = m_compareSignal->state().value() + deviation;
				}
			}
		}

		return m_compareValue;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString ComparatorEx::compareOnlineValueStr(int cmpValueType)
	{
		if (cmpValueType < 0 || cmpValueType >= CmpValueTypeCount)
		{
			return QString();
		}

		return QString::number(compareOnlineValue(cmpValueType), 'f', valuePrecision());
	}

	// -------------------------------------------------------------------------------------------------------------------

	double ComparatorEx::compareConstValue() const
	{
		double value = 0;

		// if compare value is const then hysteresis also always const
		//
		switch (m_deviationType)
		{
			case DeviationType::Unused:	value = compare().constValue();									break;
			case DeviationType::Down:	value = compare().constValue() - hysteresis().constValue() / 2;	break;
			case DeviationType::Up:		value= compare().constValue() + hysteresis().constValue() / 2;	break;
		}

		return value;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString ComparatorEx::compareDefaultValueStr() const
	{
		QString value;

		value += cmpTypeStr() + " ";

		if (compare().isConst() == true)
		{
			value += QString::number(compareConstValue(), 'f', valuePrecision());			// if compare is const then hysteresis also const
		}
		else
		{
			value += compare().appSignalID();
		}

		return value;
	}

	// -------------------------------------------------------------------------------------------------------------------

	double ComparatorEx::hysteresisOnlineValue()
	{
		if (hysteresis().isConst() == true)
		{
			m_hysteresisValue = hysteresis().constValue();
		}
		else
		{
			if (m_hysteresisSignal != nullptr)
			{
				if (m_hysteresisSignal->param().isValid() == true && m_hysteresisSignal->state().valid() == true)
				{
					m_hysteresisValue = m_hysteresisSignal->state().value();
				}
			}
		}

		return m_hysteresisValue;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString ComparatorEx::hysteresisOnlineValueStr()
	{
		return QString::number(hysteresisOnlineValue(), 'f', valuePrecision());
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString ComparatorEx::hysteresisDefaultValueStr() const
	{
		QString value;

		if (hysteresis().isConst() == true)
		{
			value = QString::number(hysteresis().constValue(), 'f', valuePrecision());
		}
		else
		{
			value = hysteresis().appSignalID();
		}

		if (m_deviationType != DeviationType::Unused)
		{
			value = QT_TRANSLATE_NOOP("MetrologySignal.cpp", "Unused");
		}

		return value;
	}


	// -------------------------------------------------------------------------------------------------------------------

	bool ComparatorEx::outputState() const
	{
		if (m_outputSignal == nullptr)
		{
			return false;
		}

		if (m_outputSignal->param().isValid() == false || m_outputSignal->state().valid() == false)
		{
			return false;
		}

		return m_outputSignal->state().value() != 0.0;
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString ComparatorEx::outputStateStr() const
	{
		return outputStateStr("True", "False");
	}

	// -------------------------------------------------------------------------------------------------------------------

	QString ComparatorEx::outputStateStr(const QString& forTrue, const QString& forFalse) const
	{
		QString stateStr;

		if (outputState() == true)
		{
			stateStr = forTrue;
		}
		else
		{
			stateStr = forFalse;
		}

		return stateStr;
	}

	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------
	// -------------------------------------------------------------------------------------------------------------------

}
