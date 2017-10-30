#include "SignalSet.h"
#include "../lib/WUtils.h"

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
					result = false;
				}

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
			if (s.enableTuning() == false)
			{
				continue;
			}

			if (s.isInternal() == false)
			{
				LOG_INTERNAL_ERROR(m_log);				// only Internals can be tuningable
				return false;
			}

			if (s.isAnalog() == true)
			{
				if (s.lowEngeneeringUnits() >= s.highEngeneeringUnits())
				{
					// LowEngeneeringUnits property of tuningable signal '%1' must be greate than HighEngeneeringUnits.
					//
					m_log->errALC5068(s.appSignalID());
					result = false;
				}
				else
				{
					// limits OK
					//
					if (s.tuningDefaultValue() < s.lowEngeneeringUnits() ||
						s.tuningDefaultValue() > s.highEngeneeringUnits())
					{
						// TuningDefaultValue property of tuningable signal '%1' must be in range from LowEngeneeringUnits toHighEngeneeringUnits.
						//
						m_log->errALC5069(s.appSignalID());
						result = false;
					}
				}
			}
		}

		return result;
	}

	bool SignalSet::expandBusSignals()
	{
		if (m_busSignals.count() == 0)
		{
			return true;
		}

		bool result = true;

		for(QString busSignalID : m_busSignals)
		{
			Signal* s = getSignal(busSignalID);

			if (s == nullptr)
			{
				result = false;
				LOG_INTERNAL_ERROR(m_log);
				continue;
			}

			const VFrame30::Bus& bus = m_busSet->bus(s->busTypeID());

			const std::vector<VFrame30::BusSignal>& busSignals = bus.busSignals();

			for(const VFrame30::BusSignal& busSignal : busSignals)
			{
				result &= appendBusSignal(*s, bus, busSignal);
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
			s.setLm(nullptr);

			if (s.equipmentID().isEmpty() == true)
			{
				// Application signal '%1' is not bound to any device object.
				//
				m_log->wrnALC5012(s.appSignalID());
			}
			else
			{
				std::shared_ptr<Hardware::DeviceObject> device = equipment->deviceObjectSharedPointer(s.equipmentID());

				if (device == nullptr)
				{
					// Application signal '%1' is bound to unknown device object '%2'.
					//
					m_log->errALC5013(s.appSignalID(), s.equipmentID());
					result = false;
					continue;
				}

				bool deviceOK = false;

				switch(device->deviceType())
				{
				case Hardware::DeviceType::Module:
					{
						std::shared_ptr<Hardware::DeviceModule> module = std::dynamic_pointer_cast<Hardware::DeviceModule>(device);

						if (module != nullptr && module->isLogicModule() == true)
						{
							s.setLm(module);
							deviceOK = true;
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
							deviceOK = true;
						}
					}
					break;
				}

				if (deviceOK == false)
				{
					// The signal '%1' can be bind only to Logic Module or Equipment Signal.
					//
					m_log->errALC5031(s.appSignalID());
					result = false;
					continue;
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

	bool SignalSet::appendBusSignal(const Signal& s, const VFrame30::Bus& bus, const VFrame30::BusSignal& busSignal)
	{
		bool result = true;

		Signal* newSignal = new Signal();

		newSignal->setAppSignalID(QString(s.appSignalID() + Signal::BUS_SIGNAL_ID_SEPARATOR + busSignal.signalId()));
		newSignal->setCustomAppSignalID(QString(s.customAppSignalID() + Signal::BUS_SIGNAL_ID_SEPARATOR + busSignal.signalId()));

		QString caption = buildBusSignalCaption(s, bus, busSignal);

		newSignal->setCaption(caption);
		newSignal->setEquipmentID(s.equipmentID());
		newSignal->setBusTypeID(s.busTypeID());
		//newSignal->setChannel(s.channel());

		newSignal->setSignalType(busSignal.type());
		newSignal->setInOutType(E::SignalInOutType::Internal);

		newSignal->setByteOrder(E::ByteOrder::BigEndian);

		switch(newSignal->signalType())
		{
		case E::SignalType::Analog:
			newSignal->setUnit(busSignal.units());
			newSignal->setDataSize(SIZE_32BIT);
			newSignal->setAnalogSignalFormat(busSignal.analogFormat());

			newSignal->setLowADC(busSignal.busAnalogLowLimit());
			newSignal->setHighADC(busSignal.busAnalogHighLimit());

			newSignal->setLowEngeneeringUnits(busSignal.busAnalogLowLimit());
			newSignal->setHighEngeneeringUnits(busSignal.busAnalogHighLimit());

			newSignal->setLowValidRange(busSignal.busAnalogLowLimit());
			newSignal->setHighValidRange(busSignal.busAnalogHighLimit());

			break;

		case E::SignalType::Discrete:
			newSignal->setUnit("");
			newSignal->setDataSize(SIZE_1BIT);

			break;

		default:
			assert(false);
		}

		newSignal->setEnableTuning(false);

		newSignal->setAcquire(s.acquire());
		newSignal->setDecimalPlaces(2);				// !!!
		newSignal->setCoarseAperture(1);
		newSignal->setFineAperture(0.5);
		newSignal->setAdaptiveAperture(false);

	/*
		// Signal fields from database
		//
		int m_ID = 0;
		int m_signalGroupID = 0;
		int m_signalInstanceID = 0;
		int m_changesetID = 0;
		bool m_checkedOut = false;
		int m_userID = 0;
		QDateTime m_created;
		bool m_deleted = false;
		QDateTime m_instanceCreated;
		VcsItemAction m_instanceAction = VcsItemAction::Added;*/

		if (result == true)
		{
			m_maxSignalID++;

			append(m_maxSignalID, newSignal);
		}
		else
		{
			delete newSignal;
		}

		return result;
	}

	QString SignalSet::buildBusSignalCaption(const Signal& s, const VFrame30::Bus& bus, const VFrame30::BusSignal& busSignal)
	{
		QString caption = s.caption();

		caption.replace(Signal::BUS_SIGNAL_MACRO_BUSTYPEID, bus.busTypeId());
		caption.replace(Signal::BUS_SIGNAL_MACRO_BUSID, s.customAppSignalID());
		caption.replace(Signal::BUS_SIGNAL_MACRO_BUSSIGNALID, busSignal.signalId());
		caption.replace(Signal::BUS_SIGNAL_MACRO_BUSSIGNALCAPTION, busSignal.caption());

		return caption;
	}

	QString SignalSet::buildBusSignalCaption(const QString& busParentSignalCaption,
											 const QString& busTypeID,
											 const QString& busParentSignalCustomID,
											 const QString& busChildSignalID,
											 const QString& busChildSignalCaption) const
	{
		QString caption = busParentSignalCaption;

		caption.replace(Signal::BUS_SIGNAL_MACRO_BUSTYPEID, busTypeID);
		caption.replace(Signal::BUS_SIGNAL_MACRO_BUSID, busParentSignalCustomID);
		caption.replace(Signal::BUS_SIGNAL_MACRO_BUSSIGNALID, busChildSignalID);
		caption.replace(Signal::BUS_SIGNAL_MACRO_BUSSIGNALCAPTION, busChildSignalCaption);

		return caption;
	}

}
