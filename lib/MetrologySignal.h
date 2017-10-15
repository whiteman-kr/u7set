#pragma once

#include "../lib/Signal.h"
#include "../lib/AppSignal.h"
#include "../lib/DeviceObject.h"
#include "../lib/XmlHelper.h"

// Attention !!!
// If you want to change any function writeToXml you must change CFG_FILE_VER_METROLOGY_SIGNALS
// and write log history about changing

const int	CFG_FILE_VER_METROLOGY_SIGNALS = 2;

// Historty of version
//
// version 1 - it is base version
// version 2 - deleted a few fields SignalParam::writeToXml (story about removing redundant ranges)
//

namespace Metrology
{

	// ==============================================================================================

	const char* const ChannelLetter [] = {"A", "B", "C", "D", "E", "F"};

	const int	ChannelCount	= sizeof(ChannelLetter)/sizeof(ChannelLetter[0]);

	const int	Channel_A		= 0,
				Channel_B		= 1,
				Channel_C		= 2,
				Channel_D		= 3,
				Channel_E		= 4,
				Channel_F		= 5;

	const int	Channel_0		= 0,
				Channel_1		= 1,
				Channel_2		= 2,
				Channel_3		= 3,
				Channel_4		= 4,
				Channel_5		= 5;

	// ==============================================================================================

	class RackParam
	{
		public:
			RackParam() {}
			RackParam(int index, const QString& equipmentID, const QString& caption);
			virtual ~RackParam() {}

		private:

			int					m_index = -1;		// index of rack in the m_equipmentID

			Hash				m_hash = 0;			// hash calcHash from rack m_equipmentID

			QString				m_equipmentID;		// rack equipmentID
			QString				m_caption;			// rack caption

			int					m_groupIndex = -1;	// for multichannel measuring - index rack group (several racks can be combine in groups)
			int					m_channel = -1;		// for multichannel measuring - index channel in rack group

		public:

			bool				isValid() const;
			void				clear();

			int					index() const { return m_index; }
			void				setIndex(int index) { m_index = index; }

			Hash				hash() const { return m_hash; }

			QString				equipmentID() const { return m_equipmentID; }
			void				setEquipmentID(const QString& equipmentID);

			QString				caption() const { return m_caption; }
			void				setCaption(const QString& caption) { m_caption = caption; }

			int					groupIndex() const { return m_groupIndex; }
			void				setGroupIndex(int index) { m_groupIndex = index; }

			int					channel() const { return m_channel; }
			QString				channelStr() const;
			void				setChannel(int channel) { m_channel = channel; }

			bool				readFromXml(XmlReadHelper& xml);
			void				writeToXml(XmlWriteHelper& xml);
	};

	// ==============================================================================================

	class SignalLocation
	{
		public:
			SignalLocation() {}
			SignalLocation(Hardware::DeviceObject* pDeviceObject);
			virtual ~SignalLocation() {}

		private:

			QString				m_equipmentID;

			RackParam			m_rack;
			int					m_chassis = -1;
			int					m_module = -1;
			int					m_place = -1;

			QString				m_contact;				// for input: _IN00A or _IN00B, for output: only _OUT00

			void				getParentObject(Hardware::DeviceObject* pDeviceObject);

		public:

			void				clear();

			QString				equipmentID() const { return m_equipmentID; }
			void				setEquipmentID(const QString& equipmentID) { m_equipmentID = equipmentID; }

			RackParam&			rack() { return m_rack; }
			QString				rackCaption(bool showIndex = false) const;
			void				setRack(const RackParam& rack) { m_rack = rack; }

			int					chassis() const { return m_chassis; }
			QString				chassisStr() const;
			void				setChassis(int chassis) { m_chassis = chassis; }

			int					module() const { return m_module; }
			QString				moduleStr() const;
			void				setModule(int module) { m_module = module; }

			int					place() const { return m_place; }
			QString				placeStr() const;
			void				setPlace(int place) { m_place = place; }

			QString				contact() const { return m_contact; }
			void				setContact(const QString& contact) { m_contact = contact; }

			bool				readFromXml(XmlReadHelper& xml);
			void				writeToXml(XmlWriteHelper& xml);
	};

	// ==============================================================================================

	class SignalParam
	{
	public:

		SignalParam() {}
		SignalParam(const Signal& signal, const SignalLocation& location) { setParam(signal, location); }
		virtual ~SignalParam() {}

	private:

		Hash					m_hash = 0;							// hash calcHash from AppSignalID

		QString					m_appSignalID;
		QString					m_customAppSignalID;
		QString					m_caption;

		E::SignalType			m_signalType = E::SignalType::Analog;
		E::SignalInOutType		m_inOutType = E::SignalInOutType::Internal;

		SignalLocation			m_location;

		int						m_lowADC = 0;
		int						m_highADC = 0;

		double					m_physicalLowLimit = 0;
		double					m_physicalHighLimit = 0;
		QString					m_physicalUnit;
		int						m_physicalPrecision = 2;

		double					m_electricLowLimit = 0;
		double					m_electricHighLimit = 0;
		E::ElectricUnit			m_electricUnitID = E::ElectricUnit::NoUnit;
		QString					m_electricUnit;
		E::SensorType			m_electricSensorType = E::SensorType::NoSensorType;
		QString					m_electricSensor;
		int						m_electricPrecision = 3;

		bool					m_enableTuning = false;
		double					m_tuningDefaultValue = 0;

	public:

		bool					isValid() const;

		void					setParam(const ::Signal& signal, const SignalLocation& location);

		Hash					hash() const { return m_hash; }

		QString					appSignalID() const { return m_appSignalID; }
		void					setAppSignalID(const QString& appSignalID);

		QString					customAppSignalID() const { return m_customAppSignalID; }
		void					setCustomAppSignalID(const QString& customAppSignalID) { m_customAppSignalID = customAppSignalID; }

		QString					caption() const { return m_caption; }
		void					setCaption(const QString& caption) { m_caption = caption; }

		E::SignalType			signalType() const { return m_signalType; }
		void					setSignalType(E::SignalType type) { m_signalType = type; }

		E::SignalInOutType		inOutType() const { return m_inOutType; }
		void					setInOutType(E::SignalInOutType inOutType) { m_inOutType = inOutType; }

		bool					isAnalog() const { return m_signalType == E::SignalType::Analog; }
		bool					isDiscrete() const { return m_signalType == E::SignalType::Discrete; }
		bool					isBus() const { return m_signalType == E::SignalType::Bus; }

		bool					isInput() const { return m_inOutType == E::SignalInOutType::Input; }
		bool					isOutput() const { return m_inOutType == E::SignalInOutType::Output; }
		bool					isInternal() const { return m_inOutType == E::SignalInOutType::Internal; }

		SignalLocation			location() const { return m_location; }
		void					setLocation(const SignalLocation& location) { m_location = location; }

		void					setRack(const Metrology::RackParam& rack) { m_location.setRack(rack); }
		void					setPlace(int place) { m_location.setPlace(place); }

		int						lowADC() const { return m_lowADC; }
		void					setLowADC(int lowADC) { m_lowADC = lowADC; }

		int						highADC() const { return m_highADC; }
		void					setHighADC(int highADC) { m_highADC = highADC;}

		QString					adcRangeStr(bool showHex) const;

		double					physicalLowLimit() const { return m_physicalLowLimit; }
		void					setPhysicalLowLimit(double lowLimit) { m_physicalLowLimit = lowLimit; }

		double					physicalHighLimit() const { return m_physicalHighLimit; }
		void					setPhysicalHighLimit(double highLimit) { m_physicalHighLimit = highLimit; }

		QString					physicalUnit() const { return m_physicalUnit; }
		void					setPhysicalUnit(const QString& unit) { m_physicalUnit = unit; }

		int						physicalPrecision() const { return m_physicalPrecision; }
		void					setPhysicalPrecision(int precision) { m_physicalPrecision = precision; }

		bool					physicalRangeIsValid() const;
		QString					physicalRangeStr() const;

		double					electricLowLimit() const { return m_electricLowLimit; }
		void					setElectricLowLimit(double lowLimit) { m_electricLowLimit = lowLimit; }

		double					electricHighLimit() const { return m_electricHighLimit; }
		void					setElectricHighLimit(double highLimit) { m_electricHighLimit = highLimit; }

		E::ElectricUnit			electricUnitID() const { return m_electricUnitID; }
		void					setElectricUnitID(E::ElectricUnit unitID) { m_electricUnitID = unitID; }

		QString					electricUnit() const { return m_electricUnit; }
		void					setElectricUnit(const QString& unit) { m_electricUnit = unit; }

		E::SensorType			electricSensorType() const { return m_electricSensorType; }
		void					setElectricSensorType(E::SensorType sensorType) { m_electricSensorType = sensorType; }

		QString					electricSensor() const { return m_electricSensor; }
		void					setElectricSensor(const QString& sensor) { m_electricSensor = sensor; }

		int						electricPrecision() const { return m_electricPrecision; }
		void					setElectricPrecision(int precision) { m_electricPrecision = precision; }

		bool					electricRangeIsValid() const;
		QString					electricRangeStr() const;

		bool					enableTuning() const { return m_enableTuning; }
		QString					enableTuningStr() const;
		void					setEnableTuning(bool enableTuning) { m_enableTuning = enableTuning; }

		double					tuningDefaultValue() const { return m_tuningDefaultValue; }
		QString					tuningDefaultValueStr() const;
		void					setTuningDefaultValue(double value) { m_tuningDefaultValue = value; }

		bool					readFromXml(XmlReadHelper& xml);
		void					writeToXml(XmlWriteHelper& xml);
	};

	// ==============================================================================================

	class SignalState
	{
	public:

		SignalState()  {}
		SignalState(const AppSignalState& state) : m_value(state.m_value), m_flags(state.m_flags)  {}
		SignalState(double value, const AppSignalStateFlags& flags) : m_value(value), m_flags(flags)  {}
		virtual ~SignalState() {}

	private:

		double					m_value = 0;

		AppSignalStateFlags		m_flags;

	public:

		void					setState(const AppSignalState& state) { m_value = state.m_value; m_flags = state.m_flags; }

		double					value() const { return m_value; }
		void					setValue(double value) { m_value = value; }

		AppSignalStateFlags		flags() const { return m_flags; }
		void					setFlags(const AppSignalStateFlags& flags) { m_flags = flags; }

		bool					valid() const { return m_flags.valid; }
		void					setValid(bool valid) { m_flags.valid = valid; }
	};

	// ==============================================================================================

	const char* const StatisticState[] =
	{
				QT_TRANSLATE_NOOP("MetrologySignal.h", "Invalid"),
				QT_TRANSLATE_NOOP("MetrologySignal.h", "Ok"),
	};

	const int	StatisticStateCount		= sizeof(StatisticState)/sizeof(StatisticState[0]);

	const int	StatisticStateInvalid	= 0,
				StatisticStateSuccess	= 1;

	// ----------------------------------------------------------------------------------------------

	class SignalStatistic
	{
	public:

		SignalStatistic() {}
		explicit SignalStatistic(const Hash& signalHash) : m_signalHash (signalHash) {}
		virtual ~SignalStatistic() {}

	private:

		Hash					m_signalHash = 0;

		int						m_measureCount = 0;
		int						m_state = StatisticStateSuccess;

	public:

		Hash					signalHash() const { return m_signalHash; }
		void					setSignalHash(const Hash& hash) { m_signalHash = hash; }

		int&					measureCount() { return m_measureCount; }
		QString					measureCountStr() const;

		int						state() const { return m_state; }
		QString					stateStr() const;
		void					setState(bool state) { m_state = state; }
	};

	// ==============================================================================================

	class Signal
	{
	public:

		Signal() {}
		explicit Signal(const SignalParam& param)
		{
			setParam(param);

			// temporary solution
			// because u7 can not set electric range
			//
			if (param.electricLowLimit() == 0 && param.electricHighLimit() == 0)
			{
				m_param.setElectricLowLimit(0);
				m_param.setElectricHighLimit(5);
			}

			if (param.electricUnitID() == E::ElectricUnit::NoUnit)
			{
				m_param.setElectricUnitID(E::ElectricUnit::V);
			}
			//
			//
		}

		virtual ~Signal() {}

	private:

		SignalParam				m_param;
		SignalState				m_state;

		SignalStatistic			m_statistic;

	public:

		SignalParam&			param() { return m_param; }
		void					setParam(const Metrology::SignalParam& param) { m_param = param; }

		SignalState&			state() { return m_state; }
		void					setState(const Metrology::SignalState& state) { m_state = state; }

		SignalStatistic&		statistic() { return m_statistic; }
		void					setStatistic(const SignalStatistic& statistic) { m_statistic = statistic; }
	};

	// ==============================================================================================
}
