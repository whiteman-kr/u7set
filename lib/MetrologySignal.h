#pragma once

#include "../lib/Signal.h"
#include "../lib/AppSignal.h"
#include "../lib/DeviceObject.h"
#include "../lib/XmlHelper.h"

// Attention !!!
// If you want to change any function writeToXml you must change CFG_FILE_VER_METROLOGY_SIGNALS
// and write log history about changing

const int	CFG_FILE_VER_METROLOGY_SIGNALS = 1;

// Historty of version
//
// version 1 - it is base version
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

		double					m_inputElectricLowLimit = 0;
		double					m_inputElectricHighLimit = 0;
		E::InputUnit			m_inputElectricUnitID = E::InputUnit::NoInputUnit;
		QString					m_inputElectricUnit;
		E::SensorType			m_inputElectricSensorType = E::SensorType::NoSensorType;
		QString					m_inputElectricSensor;
		int						m_inputElectricPrecision = 3;

		double					m_inputPhysicalLowLimit = 0;
		double					m_inputPhysicalHighLimit = 0;
		int						m_inputPhysicalUnitID = NO_UNIT_ID;
		QString					m_inputPhysicalUnit;
		int						m_inputPhysicalPrecision = 2;

		double					m_outputElectricLowLimit = 0;
		double					m_outputElectricHighLimit = 0;
		E::InputUnit			m_outputElectricUnitID  = E::InputUnit::NoInputUnit;
		QString					m_outputElectricUnit;
		E::SensorType			m_outputElectricSensorType = E::SensorType::NoSensorType;
		QString					m_outputElectricSensor;
		int						m_outputElectricPrecision = 3;

		double					m_outputPhysicalLowLimit = 0;
		double					m_outputPhysicalHighLimit = 0;
		int						m_outputPhysicalUnitID = NO_UNIT_ID;
		QString					m_outputPhysicalUnit;
		int						m_outputPhysicalPrecision = 2;

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

		double					inputElectricLowLimit() const { return m_inputElectricLowLimit; }
		void					setInputElectricLowLimit(double lowLimit) { m_inputElectricLowLimit = lowLimit; }

		double					inputElectricHighLimit() const { return m_inputElectricHighLimit; }
		void					setInputElectricHighLimit(double highLimit) { m_inputElectricHighLimit = highLimit; }

		E::InputUnit			inputElectricUnitID() const { return m_inputElectricUnitID; }
		void					setInputElectricUnitID(E::InputUnit unitID) { m_inputElectricUnitID = unitID; }

		QString					inputElectricUnit() const { return m_inputElectricUnit; }
		void					setInputElectricUnit(const QString& unit) { m_inputElectricUnit = unit; }

		E::SensorType			inputElectricSensorType() const { return m_inputElectricSensorType; }
		void					setInputElectricSensorType(E::SensorType sensorType) { m_inputElectricSensorType = sensorType; }

		QString					inputElectricSensor() const { return m_inputElectricSensor; }
		void					setInputElectricSensor(const QString& sensor) { m_inputElectricSensor = sensor; }

		int						inputElectricPrecision() const { return m_inputElectricPrecision; }
		void					setInputElectricPrecision(int precision) { m_inputElectricPrecision = precision; }

		QString					inputElectricRangeStr() const;

		double					inputPhysicalLowLimit() const { return m_inputPhysicalLowLimit; }
		void					setInputPhysicalLowLimit(double lowLimit) { m_inputPhysicalLowLimit = lowLimit; }

		double					inputPhysicalHighLimit() const { return m_inputPhysicalHighLimit; }
		void					setInputPhysicalHighLimit(double highLimit) { m_inputPhysicalHighLimit = highLimit; }

		int						inputPhysicalUnitID() const { return m_inputPhysicalUnitID; }
		void					setInputPhysicalUnitID(int unitID) { m_inputPhysicalUnitID = unitID; }

		QString					inputPhysicalUnit() const { return m_inputPhysicalUnit; }
		void					setInputPhysicalUnit(const QString& unit) { m_inputPhysicalUnit = unit; }

		int						inputPhysicalPrecision() const { return m_inputPhysicalPrecision; }
		void					setInputPhysicalPrecision(int precision) { m_inputPhysicalPrecision = precision; }

		QString					inputPhysicalRangeStr() const;

		double					outputElectricLowLimit() const { return m_outputElectricLowLimit; }
		void					setOutputElectricLowLimit(double lowLimit) { m_outputElectricLowLimit = lowLimit; }

		double					outputElectricHighLimit() const { return m_outputElectricHighLimit; }
		void					setOutputElectricHighLimit(double highLimit) { m_outputElectricHighLimit = highLimit; }

		E::InputUnit			outputElectricUnitID() const { return m_outputElectricUnitID; }
		void					setOutputElectricUnitID(const E::InputUnit unitID) { m_outputElectricUnitID = unitID; }

		QString					outputElectricUnit() const { return m_outputElectricUnit; }
		void					setOutputElectricUnit(const QString& unit) { m_outputElectricUnit = unit; }

		E::SensorType			outputElectricSensorType() const { return m_outputElectricSensorType; }
		void					setOutputElectricSensorType(const E::SensorType sensorType) { m_outputElectricSensorType = sensorType; }

		QString					outputElectricSensor() const { return m_outputElectricSensor; }
		void					setOutputElectricSensor(const QString& sensor) { m_outputElectricSensor = sensor; }

		int						outputElectricPrecision() const { return m_outputElectricPrecision; }
		void					setOutputElectricPrecision(int precision) { m_outputElectricPrecision = precision; }

		QString					outputElectricRangeStr() const;

		double					outputPhysicalLowLimit() const { return m_outputPhysicalLowLimit; }
		void					setOutputPhysicalLowLimit(double lowLimit) { m_outputPhysicalLowLimit = lowLimit; }

		double					outputPhysicalHighLimit() const { return m_outputPhysicalHighLimit; }
		void					setOutputPhysicalHighLimit(double highLimit) { m_outputPhysicalHighLimit = highLimit; }

		int						outputPhysicalUnitID() const { return m_outputPhysicalUnitID; }
		void					setOutputPhysicalUnitID(int unitID) { m_outputPhysicalUnitID = unitID; }

		QString					outputPhysicalUnit() const { return m_outputPhysicalUnit; }
		void					setOutputPhysicalUnit(const QString& unit) { m_outputPhysicalUnit = unit; }

		int						outputPhysicalPrecision() const { return m_outputPhysicalPrecision; }
		void					setOutputPhysicalPrecision(int precision) { m_outputPhysicalPrecision = precision; }

		QString					outputPhysicalRangeStr() const;

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
		SignalState(const AppSignalState& state) : m_value(state.value), m_flags(state.flags)  {}
		SignalState(double value, const AppSignalStateFlags& flags) : m_value(value), m_flags(flags)  {}
		virtual ~SignalState() {}

	private:

		double					m_value = 0;

		AppSignalStateFlags		m_flags;

	public:

		void					setState(const AppSignalState& state) { m_value = state.value; m_flags = state.flags; }

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
			m_param.setInputElectricLowLimit(0);
			m_param.setInputElectricHighLimit(5);
			m_param.setInputElectricUnitID(E::InputUnit::V);
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
