#include "SignalSet.h"
#include "../lib/WUtils.h"
#include "../lib/SignalProperties.h"

namespace Builder
{

	SignalSet::SignalSet(VFrame30::BusSet* busSet, BuildResultWriter* resultWriter, IssueLogger* log) :
		m_busSet(busSet),
		m_resultWriter(resultWriter),
		m_log(log),
		m_busses(busSet, log)
	{
		assert(busSet != nullptr);
		assert(resultWriter != nullptr);
		assert(log != nullptr);
	}

	SignalSet::~SignalSet()
	{
	}

	bool SignalSet::prepareBusses()
	{
		bool result =  m_busses.prepare();

		if (result == false)
		{
			return false;
		}

		if (m_busses.count() == 0)
		{
			return true;
		}

		return m_busses.writeReport(m_resultWriter);
	}

	bool SignalSet::checkSignals()
	{
		bool result = true;

		int signalCount = count();

		if (signalCount == 0)
		{
			return true;
		}

		LOG_EMPTY_LINE(m_log);
		LOG_MESSAGE(m_log, QString(tr("Checking application signals...")));

		QHash<QString, int> appSignalIDs;
		QHash<QString, int> customAppSignalIDs;

		appSignalIDs.reserve(static_cast<int>(signalCount * 1.3));

		m_busSignals.clear();

		for(int i = 0; i < signalCount; i++)
		{
			Signal& s = (*this)[i];

			if (s.ID() > m_maxSignalID)
			{
				m_maxSignalID = s.ID();
			}

			// check AppSignalID
			//
			if (appSignalIDs.contains(s.appSignalID()) == true)
			{
				// Application signal identifier '%1' is not unique.
				//
				m_log->errALC5016(s.appSignalID());
				result = false;
				continue;
			}

			appSignalIDs.insert(s.appSignalID(), i);

			// check CustomAppSignalID
			//
			if (customAppSignalIDs.contains(s.customAppSignalID()) == true)
			{
				// Custom application signal identifier '%1' is not unique.
				//
				m_log->errALC5017(s.customAppSignalID());
				result = false;
				continue;
			}

			customAppSignalIDs.insert(s.customAppSignalID(), i);

			// check other signal properties
			//
			if (s.isAnalog() == true && s.byteOrder() != E::ByteOrder::BigEndian)
			{
				// Signal '%1' has Little Endian byte order.
				//
				m_log->wrnALC5070(s.appSignalID());
			}

			switch(s.signalType())
			{
			case E::SignalType::Discrete:
				if (s.dataSize() != 1)
				{
					// Discrete signal '%1' must have DataSize equal to 1.
					//
					m_log->errALC5014(s.appSignalID());
					result = false;
				}
				break;

			case E::SignalType::Analog:
				if (s.dataSize() != 32)
				{
					// Analog signal '%1' must have DataSize equal to 32.
					//
					m_log->errALC5015(s.appSignalID());
					result = false;
				}

				if (s.coarseAperture() <= 0 || s.fineAperture() <= 0)
				{
					// Analog signal '%1' aperture should be greate then 0.
					//
					m_log->errALC5090(s.appSignalID());
					result = false;
				}

				if (s.coarseAperture() < s.fineAperture())
				{
					// Coarse aperture of signal '%1' less then fine aperture.
					//
					m_log->wrnALC5093(s.appSignalID());
				}

				result &= checkSignalPropertiesRanges(s);

				break;

			case E::SignalType::Bus:
				{
					BusShared bus = getBus(s.busTypeID());

					if (bus == nullptr)
					{
						//  Bus type ID '%1' of signal '%2' is undefined.
						//
						m_log->errALC5092(s.busTypeID(), s.appSignalID());
						result = false;
					}
					else
					{
						m_busSignals.insert(i, s.appSignalID());

						s.setDataSize(bus->sizeW() * SIZE_16BIT);
					}
				}
				break;

			default:
				assert(false);
				LOG_INTERNAL_ERROR(m_log);
				return false;
			}

			// check tuningable signals properties
			//
			if (s.enableTuning() == true)
			{
				if (s.isInternal() == false)
				{
					LOG_INTERNAL_ERROR(m_log);				// only Internals can be tuningable
					return false;
				}

				if (s.tuningLowBound() >= s.tuningHighBound())
				{
					// TuningHighBound property of tuningable signal '%1' must be greate than TuningLowBound.
					//
					m_log->errALC5068(s.appSignalID());
					result = false;
				}
				else
				{
					// bounds OK, defaultValue checking
					//
					if (s.tuningDefaultValue() < s.tuningLowBound() ||
						s.tuningDefaultValue() > s.tuningHighBound())
					{
						// TuningDefaultValue property of tuningable signal %1 must be in range from TuningLowBound to TuningHighBound.
						//
						m_log->errALC5069(s.appSignalID());
						result = false;
					}
				}
			}
		}

		return result;
	}

	bool SignalSet::bindSignalsToLMs(Hardware::EquipmentSet* equipment)
	{
		TEST_PTR_RETURN_FALSE(equipment);

		int signalCount = count();

		bool result = true;

		for(int i = 0; i < signalCount; i++)
		{
			Signal& s = (*this)[i];

			// check EquipmentID
			//
			if (s.equipmentID().isEmpty() == true)
			{
				// Application signal '%1' is not bound to any device object.
				//
				m_log->wrnALC5012(s.appSignalID());
				continue;
			}

			std::shared_ptr<Hardware::DeviceObject> device = equipment->deviceObjectSharedPointer(s.equipmentID());

			if (device == nullptr)
			{
				// Application signal '%1' is bound to unknown device object '%2'.
				//
				m_log->errALC5013(s.appSignalID(), s.equipmentID());
				result = false;
				continue;
			}

			switch(device->deviceType())
			{
			case Hardware::DeviceType::Module:
				{
					std::shared_ptr<Hardware::DeviceModule> module = std::dynamic_pointer_cast<Hardware::DeviceModule>(device);

					if (module != nullptr && (module->isLogicModule() == true || module->isBvb() == true))
					{
						s.setLm(module);
					}
					else
					{
						// The signal %1 can be bind only to Logic Module or Equipment Signal.
						//
						m_log->errALC5031(s.appSignalID());
						result = false;
					}
				}
				break;

			case Hardware::DeviceType::Signal:
				{
					Hardware::DeviceChassis* chassis = const_cast<Hardware::DeviceChassis*>(device->getParentChassis());

					if (chassis == nullptr)
					{
						assert(false);
						continue;
					}

					std::shared_ptr<Hardware::DeviceModule> module = chassis->getLogicModuleSharedPointer();

					if (module != nullptr && module->isLogicModule() == true)
					{
						s.setLm(module);
					}
					else
					{
						// Associated logic module is not found. Signal %1 cannot be processed.
						//
						m_log->errALC5154(s.appSignalID());
						result = false;
					}
				}
				break;

			default:

				if (s.isInput() == true || s.isOutput() == true)
				{
					// The input (or output) signal %1 can be bind to Equipment Signal only.
					//
					m_log->errALC5136(s.appSignalID());
					result = false;
				}
				else
				{
					// The signal %1 can be bind only to Logic Module or Equipment Signal.
					//
					m_log->errALC5031(s.appSignalID());
					result = false;
				}
			}
		}

		return result;
	}

	void SignalSet::initCalculatedSignalsProperties()
	{
		int signalsCount = count();

		for(int i = 0; i < signalsCount; i++)
		{
			Signal& s = (*this)[i];

			s.initCalculatedProperties();
		}
	}

	Signal* SignalSet::appendBusChildSignal(const Signal& s, BusShared bus, const BusSignal& busSignal)
	{
		m_maxSignalID++;

		Signal* newSignal = createBusChildSignal(s, bus, busSignal);

		append(m_maxSignalID, newSignal);

		return newSignal;
	}

	Signal* SignalSet::createBusChildSignal(const Signal& busParentSignal, BusShared bus, const BusSignal& busSignal)
	{
		Signal* newSignal = new Signal();

		newSignal->setAppSignalID(QString(busParentSignal.appSignalID() + Signal::BUS_SIGNAL_ID_SEPARATOR + busSignal.signalID));
		newSignal->setCustomAppSignalID(QString(busParentSignal.customAppSignalID() + Signal::BUS_SIGNAL_ID_SEPARATOR + busSignal.signalID));

		QString caption = buildBusSignalCaption(busParentSignal, bus, busSignal);

		newSignal->setCaption(caption);
		newSignal->setEquipmentID(busParentSignal.equipmentID());
		newSignal->setLm(busParentSignal.lm());
//		newSignal->setBusTypeID(s.busTypeID());

		newSignal->setSignalType(busSignal.signalType);
		newSignal->setInOutType(E::SignalInOutType::Internal);

		newSignal->setByteOrder(E::ByteOrder::BigEndian);

		switch(newSignal->signalType())
		{
		case E::SignalType::Analog:

			newSignal->setUnit(busSignal.units);
			newSignal->setDataSize(SIZE_32BIT);
			newSignal->setAnalogSignalFormat(busSignal.analogFormat);

			newSignal->setSpecPropStruct(SignalProperties::defaultBusChildAnalogSpecPropStruct);
			newSignal->createSpecPropValues();

			newSignal->setLowADC(busSignal.inbusAnalogLowLimit);
			newSignal->setHighADC(busSignal.inbusAnalogHighLimit);

			newSignal->setLowEngeneeringUnits(busSignal.busAnalogLowLimit);
			newSignal->setHighEngeneeringUnits(busSignal.busAnalogHighLimit);

			newSignal->setLowValidRange(busSignal.busAnalogLowLimit);
			newSignal->setHighValidRange(busSignal.busAnalogHighLimit);
			break;

		case E::SignalType::Discrete:
			newSignal->setUnit("");
			newSignal->setDataSize(SIZE_1BIT);
			break;

		case E::SignalType::Bus:
			newSignal->setBusTypeID(busSignal.busTypeID);
			newSignal->setDataSize(busSignal.inbusSizeBits);
			break;

		default:
			assert(false);
		}

		newSignal->setEnableTuning(false);

		newSignal->setAcquire(busParentSignal.acquire());
		newSignal->setDecimalPlaces(2);				// !!!
		newSignal->setCoarseAperture(1);
		newSignal->setFineAperture(0.5);
		newSignal->setAdaptiveAperture(false);

		return newSignal;
	}

	QString SignalSet::buildBusSignalCaption(const Signal& s, BusShared bus, const BusSignal& busSignal)
	{
		QString caption = s.caption();

		caption.replace(Signal::BUS_SIGNAL_MACRO_BUSTYPEID, bus->busTypeID());
		caption.replace(Signal::BUS_SIGNAL_MACRO_BUSID, s.customAppSignalID());
		caption.replace(Signal::BUS_SIGNAL_MACRO_BUSSIGNALID, busSignal.signalID);
		caption.replace(Signal::BUS_SIGNAL_MACRO_BUSSIGNALCAPTION, busSignal.caption);

		return caption;
	}

	QString SignalSet::buildBusSignalCaption(const QString& busParentSignalCaption,
											 const QString& busTypeID,
											 const QString& busParentSignalCustomID,
											 const QString& busChildSignalID,
											 const QString& busChildSignalCaption)
	{
		QString caption = busParentSignalCaption;

		caption.replace(Signal::BUS_SIGNAL_MACRO_BUSTYPEID, busTypeID);
		caption.replace(Signal::BUS_SIGNAL_MACRO_BUSID, busParentSignalCustomID);
		caption.replace(Signal::BUS_SIGNAL_MACRO_BUSSIGNALID, busChildSignalID);
		caption.replace(Signal::BUS_SIGNAL_MACRO_BUSSIGNALCAPTION, busChildSignalCaption);

		return caption;
	}

	bool SignalSet::checkSignalPropertiesRanges(const Signal& s)
	{
		if (s.isAnalog() == false)
		{
			return true;
		}

		bool result = true;

		result &= checkSignalPropertyRanges(s, s.lowEngeneeringUnits(), "LowEngeneeringUnits");
		result &= checkSignalPropertyRanges(s, s.highEngeneeringUnits(), "HighEngeneeringUnits");

		result &= checkSignalPropertyRanges(s, s.lowValidRange(), "LowValidRange");
		result &= checkSignalPropertyRanges(s, s.highValidRange(), "HighValidRange");

		result &= checkSignalTuningValuesRanges(s, s.tuningDefaultValue(), "TuningDefaultValue");
		result &= checkSignalTuningValuesRanges(s, s.tuningLowBound(), "TuningLowBound");
		result &= checkSignalTuningValuesRanges(s, s.tuningHighBound(), "TuningHighBound");

		return result;
	}

	bool SignalSet::checkSignalPropertyRanges(const Signal& s, double properyValue, const QString& propertyName)
	{
		if (s.isAnalog() == false)
		{
			return true;
		}

		bool result = true;

		switch(s.analogSignalFormat())
		{
		case E::AnalogAppSignalFormat::SignedInt32:

			if (properyValue > static_cast<double>(std::numeric_limits<qint32>::max()) ||
				properyValue < static_cast<double>(std::numeric_limits<qint32>::lowest()))
			{
				m_log->errALC5137(s.appSignalID(), propertyName);
				result = false;
			}

			break;

		case E::AnalogAppSignalFormat::Float32:

			if (properyValue > static_cast<double>(std::numeric_limits<float>::max()) ||
				properyValue < static_cast<double>(std::numeric_limits<float>::lowest()))
			{
				m_log->errALC5138(s.appSignalID(), propertyName);
				result = false;
			}

			break;

		default:
			assert(false);
		}

		return result;
	}

	bool SignalSet::checkSignalTuningValuesRanges(const Signal& s, const TuningValue& tuningValue, const QString& propertyName)
	{
		if (s.isAnalog() == false)
		{
			return true;
		}

		bool result = true;

		switch(s.analogSignalFormat())
		{
		case E::AnalogAppSignalFormat::SignedInt32:

			assert(tuningValue.type() == TuningValueType::SignedInt32);

			if (tuningValue.rawInt64() > static_cast<qint64>(std::numeric_limits<qint32>::max()) ||
				tuningValue.rawInt64() < static_cast<qint64>(std::numeric_limits<qint32>::lowest()))
			{
				m_log->errALC5137(s.appSignalID(), propertyName);
				result = false;
			}

			break;

		case E::AnalogAppSignalFormat::Float32:

			if (tuningValue.rawDouble() > static_cast<double>(std::numeric_limits<float>::max()) ||
				tuningValue.rawDouble() < static_cast<double>(std::numeric_limits<float>::lowest()))
			{
				m_log->errALC5138(s.appSignalID(), propertyName);
				result = false;
			}

			break;

		default:
			assert(false);
		}

		return result;
	}




}