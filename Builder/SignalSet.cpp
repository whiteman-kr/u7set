#include "SignalSet.h"
#include "../UtilsLib/WUtils.h"

namespace Builder
{

	SignalSet::SignalSet(VFrame30::BusSet* busSet, std::shared_ptr<BuildResultWriter> resultWriter, IssueLogger* log) :
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

		return m_busses.writeReport(m_resultWriter.get());
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
			AppSignal& s = (*this)[i];

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

				if (s.coarseAperture() >= 100 ||
					s.fineAperture() >= 100)
				{
					// Aperture of signal %1 should be less then 100.
					//
					m_log->errALC5157(s.appSignalID());
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

			// check tunable signals properties
			//
			if (s.enableTuning() == true)
			{
				if (s.isInternal() == false && s.isOutput() == false)
				{
					LOG_INTERNAL_ERROR_MSG(m_log, QString("%1 - only Internal and Output signals can be tunable").
													arg(s.appSignalID()));
					result = false;
					continue;
				}

				if (s.tuningLowBound() >= s.tuningHighBound())
				{
					// TuningHighBound property of tunable signal '%1' must be greate than TuningLowBound.
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
						// TuningDefaultValue property of tunable signal %1 must be in range from TuningLowBound to TuningHighBound.
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

		m_signalToLm.clear();

		for(int i = 0; i < signalCount; i++)
		{
			AppSignal& s = (*this)[i];

			// check EquipmentID
			//
			if (s.equipmentID().isEmpty() == true)
			{
				// Application signal '%1' is not bound to any device object.
				//
				m_log->wrnALC5012(s.appSignalID());
				continue;
			}

			std::shared_ptr<Hardware::DeviceObject> device = equipment->deviceObject(s.equipmentID());

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
						linkSignalToLm(&s, module);
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

			case Hardware::DeviceType::AppSignal:
				{
					Hardware::DeviceChassis* chassis = const_cast<Hardware::DeviceChassis*>(device->getParentChassis());

					if (chassis == nullptr)
					{
						assert(false);
						continue;
					}

					std::shared_ptr<Hardware::DeviceModule> module = chassis->findLogicModule();

					if (module != nullptr && module->isLogicModule() == true)
					{
						linkSignalToLm(&s, module);
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
			AppSignal& s = (*this)[i];

			s.initCalculatedProperties();
		}
	}

	void SignalSet::cacheSpecPropValues()
	{
		int signalsCount = count();

		for(int i = 0; i < signalsCount; i++)
		{
			AppSignal& s = (*this)[i];

			s.cacheSpecPropValues();
		}
	}

	bool SignalSet::expandTemplates(Hardware::EquipmentSet* equipment)
	{
		int signalCount = count();

		if (signalCount == 0)
		{
			return true;
		}

		QHash<QString, AppSignal*> expandedCustomAppSignalIDs;

		expandedCustomAppSignalIDs.reserve(signalCount);

		bool result = true;

		for(int i = 0; i < signalCount; i++)
		{
			AppSignal& s = (*this)[i];

			if (s.customAppSignalIDContainsMacro() == false &&
					s.captionContainsMacro() == false)
			{
				continue;
			}

			const Hardware::DeviceObject* deviceObject = equipment->deviceObject(s.equipmentID()).get();

			if (deviceObject == nullptr)
			{
				// Application signal %1 is bound to unknown device object %2.
				//
				m_log->errALC5013(s.appSignalID(), s.equipmentID());
				result = false;
				continue;
			}

			if (s.customAppSignalIDContainsMacro() == true)
			{
				QString errMsg;

				QString expandedCustomID = Hardware::expandDeviceSignalTemplate(*deviceObject, s.customAppSignalID(), &errMsg);

				if (errMsg.isEmpty() == false)
				{
					// App signal %1 macro expanding error: %2
					//
					m_log->errALC5182(s.appSignalID(), errMsg);
					result = false;
					continue;
				}

				AppSignal* existsSignal = expandedCustomAppSignalIDs.value(expandedCustomID, nullptr);

				if (existsSignal != nullptr)
				{
					// Non unique CustomAppSignalID after macro expansion in signals %1 and %2
					//
					m_log->errALC5183(existsSignal->appSignalID(), s.appSignalID());
					result = false;
					continue;
				}

				s.setCustomAppSignalID(expandedCustomID);

				expandedCustomAppSignalIDs.insert(expandedCustomID, &s);
			}

			if (s.captionContainsMacro() == true)
			{
				QString errMsg;

				QString expandedCaption = Hardware::expandDeviceSignalTemplate(*deviceObject, s.caption(), &errMsg);

				if (errMsg.isEmpty() == false)
				{
					// App signal %1 macro expanding error: %2
					//
					m_log->errALC5182(s.appSignalID(), errMsg);
					result = false;
					continue;
				}

				s.setCaption(expandedCaption);
			}
		}

		if (result == true)
		{
			LOG_MESSAGE(m_log, QString("App signal macrosses are successfully expanded - %1").
							arg(expandedCustomAppSignalIDs.count()));
		}

		return result;
	}

	AppSignal* SignalSet::appendBusChildSignal(const AppSignal& s, BusShared bus, const BusSignal& busSignal, DeviceModuleShared lm)
	{
		AppSignal* newSignal = createBusChildSignal(s, bus, busSignal);

		if (newSignal != nullptr)
		{
			AppSignalSet::append(newSignal);

			linkSignalToLm(newSignal, lm);
		}

		return newSignal;
	}

	AppSignal* SignalSet::createBusChildSignal(const AppSignal& busParentSignal, BusShared bus, const BusSignal& busSignal)
	{
		AppSignal* newSignal = new AppSignal();

		newSignal->setAppSignalID(QString(busParentSignal.appSignalID() + BusSignal::BUS_SIGNAL_ID_SEPARATOR + busSignal.signalID));
		newSignal->setCustomAppSignalID(QString(busParentSignal.customAppSignalID() + BusSignal::BUS_SIGNAL_ID_SEPARATOR + busSignal.signalID));

		QString caption = expandBusSignalCaptionTemplate(busParentSignal, bus, busSignal);

		newSignal->setCaption(caption);
		newSignal->setEquipmentID(busParentSignal.equipmentID());

		newSignal->setSignalType(busSignal.signalType);
		newSignal->setInOutType(busParentSignal.inOutType());

		newSignal->setByteOrder(E::ByteOrder::BigEndian);

		switch(newSignal->signalType())
		{
		case E::SignalType::Analog:

			newSignal->setUnit(busSignal.units);
			newSignal->setDataSize(SIZE_32BIT);
			newSignal->setAnalogSignalFormat(busSignal.analogFormat);

			newSignal->setSpecPropStruct(AppSignalDefaultSpecPropStruct::BUS_CHILD_ANALOG);
			newSignal->createSpecPropValues();

			newSignal->setLowADC(static_cast<int>(busSignal.inbusAnalogLowLimit));
			newSignal->setHighADC(static_cast<int>(busSignal.inbusAnalogHighLimit));

			newSignal->setLowEngineeringUnits(busSignal.busAnalogLowLimit);
			newSignal->setHighEngineeringUnits(busSignal.busAnalogHighLimit);

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

	void SignalSet::findAndRemoveExcludedFromBuildSignals()
	{
		QVector<int> excludedFromBuidSignalsIDs;

		int signalCount = count();

		for(int i = 0; i < signalCount; i++)
		{
			const AppSignal& s = (*this)[i];

			if (s.excludeFromBuild() == true)
			{
				excludedFromBuidSignalsIDs.append(s.ID());

				// Signal %1 is excluded from build.
				//
				m_log->wrnALC5167(s.appSignalID());											// Signal %1 is excluded from build.
			}
		}

		for(int id : excludedFromBuidSignalsIDs)
		{
			remove(id);
		}
	}

	void SignalSet::linkSignalToLm(AppSignal* appSignal, DeviceModuleShared lm)
	{
		TEST_PTR_RETURN(appSignal);
		TEST_PTR_RETURN(lm);

		if (m_signalToLm.contains(appSignal) == true)
		{
			Q_ASSERT(false);
			return;
		}

		m_signalToLm.insert({appSignal, lm});

		appSignal->setLmEquipmentID(lm->equipmentIdTemplate());
	}

	void SignalSet::append(AppSignal* appSignal, DeviceModuleShared lm)
	{
		TEST_PTR_RETURN(appSignal);
		TEST_PTR_RETURN(lm);

		//
		// Here SignalSet take on ownership of appSignal object
		//

		AppSignalSet::append(appSignal);
		linkSignalToLm(appSignal, lm);
	}

	DeviceModuleShared SignalSet::getAppSignalLm(const AppSignal* appSignal) const
	{
		if (appSignal == nullptr)
		{
			return nullptr;
		}

		std::map<const AppSignal*, DeviceModuleShared>::const_iterator pos = m_signalToLm.find(appSignal);

		if (pos == m_signalToLm.end())
		{
			return nullptr;
		}

		return pos->second;
	}

	DeviceModuleShared SignalSet::getAppSignalLm(const QString& appSignalID) const
	{
		const AppSignal* appSignal = getSignal(appSignalID);

		return getAppSignalLm(appSignal);
	}

	QString SignalSet::expandBusSignalCaptionTemplate(const AppSignal& busParentSignal, BusShared bus, const BusSignal& busSignal) const
	{
		QString caption = busSignal.caption;

		caption.replace(BusSignal::BUS_TYPE, bus->busTypeID());
		caption.replace(BusSignal::BUS_APP_SIGNAL_ID, busParentSignal.appSignalID());
		caption.replace(BusSignal::BUS_CUSTOM_APP_SIGNAL_ID, busParentSignal.customAppSignalID());
		caption.replace(BusSignal::BUS_CAPTION, busParentSignal.caption());

		return caption;
	}

	bool SignalSet::isSignalExists(const QString& appSignalID) const
	{
		return (getSignal(appSignalID) != nullptr);
	}

	bool SignalSet::checkSignalPropertiesRanges(const AppSignal& s)
	{
		if (s.isAnalog() == false)
		{
			return true;
		}

		bool result = true;

		result &= checkSignalPropertyRanges(s, AppSignalPropNames::LOW_ENGINEERING_UNITS);
		result &= checkSignalPropertyRanges(s, AppSignalPropNames::HIGH_ENGINEERING_UNITS);
		result &= checkSignalPropertyRanges(s, AppSignalPropNames::LOW_VALID_RANGE);
		result &= checkSignalPropertyRanges(s, AppSignalPropNames::HIGH_VALID_RANGE);

		result &= checkSignalTuningValuesRanges(s, s.tuningDefaultValue(), AppSignalPropNames::TUNING_DEFAULT_VALUE);
		result &= checkSignalTuningValuesRanges(s, s.tuningLowBound(), AppSignalPropNames::TUNING_LOW_BOUND);
		result &= checkSignalTuningValuesRanges(s, s.tuningHighBound(), AppSignalPropNames::TUNING_HIGH_BOUND);

		return result;
	}

	bool SignalSet::checkSignalPropertyRanges(const AppSignal& s, const QString& propertyName)
	{
		if (s.isAnalog() == false)
		{
			return true;
		}

		if (s.isSpecPropExists(propertyName) == false)
		{
			return true;
		}

		double properyValue = s.getSpecPropDouble(propertyName, nullptr);

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

	bool SignalSet::checkSignalTuningValuesRanges(const AppSignal& s, const TuningValue& tuningValue, const QString& propertyName)
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
