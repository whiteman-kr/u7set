#pragma once

#include "../lib/Signal.h"
#include "../lib/AppSignal.h"
#include "../lib/DeviceObject.h"
#include "../lib/XmlHelper.h"
#include "../lib/UnitsConvertor.h"
#include "../lib/ComparatorSet.h"
#include "../lib/SignalProperties.h"
#include "../Builder/CfgFiles.h"
#include "../Proto/serialization.pb.h"
#include "../lib/ProtoSerialization.h"

// Attention !!!
// If you want to change any function writeToXml you must change CFG_FILE_VER_METROLOGY_ITEMS
// and write log history about changing

const int			CFG_FILE_VER_METROLOGY_ITEMS_XML	= 1;

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

	const int	InputCount	= 32;

	const int	ComparatorCount	= 16;

	// ==============================================================================================

	class ComparatorEx;

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

			// serialize
			//

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

			RackParam			m_rack;					// rack EquipmentID

			QString				m_chassisID;			// chassis EquipmentID
			int					m_chassis = -1;			// number of chassis

			QString				m_moduleID;				// module EquipmentID
			int					m_module = -1;			// number of module

			int					m_place = -1;			// number of place
			QString				m_contact;				// for input: _IN00A or _IN00B, for output: only _OUT00

			QString				m_moduleSerialNoID;		// AppSignalID serial number of module	- write online
			int					m_moduleSerialNo = 0;	// serial number of module				- write online

			void				getParentObject(Hardware::DeviceObject* pDeviceObject);

		public:

			void				clear();

			RackParam&			rack() { return m_rack; }
			QString				rackCaption(bool showIndex = false) const;
			void				setRack(const RackParam& rack) { m_rack = rack; }

			QString				chassisID() const { return m_chassisID; }
			void				setChassisID(const QString& chassisID) { m_chassisID = chassisID; }

			int					chassis() const { return m_chassis; }
			QString				chassisStr() const;
			void				setChassis(int chassis) { m_chassis = chassis; }

			QString				moduleID() const { return m_moduleID; }
			void				setModuleID(const QString& moduleID) { m_moduleID = moduleID; }

			int					module() const { return m_module; }
			QString				moduleStr() const;
			void				setModule(int module) { m_module = module; }

			int					place() const { return m_place; }
			QString				placeStr() const;
			void				setPlace(int place) { m_place = place; }

			QString				contact() const { return m_contact; }
			void				setContact(const QString& contact) { m_contact = contact; }

			QString				moduleSerialNoID() const { return m_moduleSerialNoID; }
			void				setModuleSerialNoID(const QString& appSignalID) { m_moduleSerialNoID = appSignalID; }

			int					moduleSerialNo() const { return m_moduleSerialNo; }
			QString				moduleSerialNoStr() const;
			void				setModuleSerialNo(int serialNo) { m_moduleSerialNo = serialNo; }

			// serialize
			//
			void				serializeTo(Proto::MetrologySignalLocation *l) const;
			bool				serializeFrom(const Proto::MetrologySignalLocation& l);
	};

	// ==============================================================================================

	class SignalParam : public ::Signal
	{
	public:

		SignalParam() {}
		SignalParam(const ::Signal& signal, const SignalLocation& location);
		virtual ~SignalParam() {}

	private:

		SignalLocation			m_location;

		double					m_electricLowLimit = 0;
		double					m_electricHighLimit = 0;

		E::ElectricUnit			m_electricUnitID = E::ElectricUnit::NoUnit;
		E::SensorType			m_electricSensorType = E::SensorType::NoSensor;

		double					m_electricR0 = 0;
		int						m_electricPrecision = 4;

		double					m_physicalLowLimit = 0;
		double					m_physicalHighLimit = 0;

		QVector<std::shared_ptr<ComparatorEx>> m_comparatorList;
		int						m_comparatorCount = 0;

	public:

		bool					isValid() const;

		void					setParam(const ::Signal& signal, const SignalLocation& location);

		void					setAppSignalID(const QString& appSignalID);

		SignalLocation&			location() { return m_location; }
		SignalLocation			location() const { return m_location; }
		void					setLocation(const SignalLocation& location) { m_location = location; }

		void					setRack(const Metrology::RackParam& rack) { m_location.setRack(rack); }
		void					setPlace(int place) { m_location.setPlace(place); }

		QString					adcRangeStr(bool showHex) const;

		double					electricLowLimit() const { return m_electricLowLimit; }
		void					setElectricLowLimit(double lowLimit) { m_electricLowLimit = lowLimit; }

		double					electricHighLimit() const { return m_electricHighLimit; }
		void					setElectricHighLimit(double highLimit) { m_electricHighLimit = highLimit; }

		E::ElectricUnit			electricUnitID() const { return m_electricUnitID; }
		void					setElectricUnitID(E::ElectricUnit unitID) { m_electricUnitID = unitID; }
		QString					electricUnitStr() const;

		E::SensorType			electricSensorType() const { return m_electricSensorType; }
		void					setElectricSensorType(E::SensorType sensorType) { m_electricSensorType = sensorType; }
		QString					electricSensorTypeStr() const;

		double					electricR0() const { return m_electricR0; }
		void					setElectricR0(double r0) { m_electricR0 = r0; }
		QString					electricR0Str() const;

		int						electricPrecision() const { return m_electricPrecision; }
		void					setElectricPrecision(int precision) { m_electricPrecision = precision; }

		bool					electricRangeIsValid() const;
		QString					electricRangeStr() const;

		double					physicalLowLimit() const { return m_physicalLowLimit; }
		void					setPhysicalLowLimit(double lowLimit) { m_physicalLowLimit = lowLimit; }

		double					physicalHighLimit() const { return m_physicalHighLimit; }
		void					setPhysicalHighLimit(double highLimit) { m_physicalHighLimit = highLimit; }

		bool					physicalRangeIsValid() const;
		QString					physicalRangeStr() const;

		bool					engineeringRangeIsValid() const;
		QString					engineeringRangeStr() const;

		// tuning
		//
		QString					enableTuningStr() const;
		QString					tuningDefaultValueStr() const;
		bool					tuningRangeIsValid() const;
		QString					tuningRangeStr() const;
		TuningValueType			tuningValueType() const;

		// comparators
		//
		std::shared_ptr<ComparatorEx> comparator(int index) const;
		void					setComparatorList(const QVector<std::shared_ptr<ComparatorEx>>& comparators);
		int						comparatorCount() const { return m_comparatorCount; }
		bool					hasComparators() const { return m_comparatorCount != 0; }

		// serialize
		//
		void					serializeTo(Proto::MetrologySignal *ms) const;
		bool					serializeFrom(const Proto::MetrologySignal& ms);
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

		double m_value = 0;

		AppSignalStateFlags m_flags;

	public:

		void setState(const AppSignalState& state) { m_value = state.m_value; m_flags = state.m_flags; }

		double value() const { return m_value; }
		void setValue(double value) { m_value = value; }

		AppSignalStateFlags flags() { return m_flags; }
		const AppSignalStateFlags& flags() const { return m_flags; }
		void setFlags(const AppSignalStateFlags& flags) { m_flags = flags; }

		bool valid() const { return m_flags.valid; }
		void setValid(bool valid) { m_flags.valid = valid; }
	};


	// ==============================================================================================

	class SignalStatistic
	{
	public:

		SignalStatistic() {}
		virtual ~SignalStatistic() {}

		enum State
		{
			Failed,
			Success,
		};

	private:

		int m_measureCount = 0;
		State m_state = State::Success;

	public:

		int measureCount() const { return m_measureCount; }
		void setMeasureCount(int count) { m_measureCount = count; }
		QString measureCountStr() const;

		State state() const { return m_state; }
		void setState(State state) { m_state = state; }
		QString stateStr() const;

		bool isMeasured() const { return m_measureCount != 0; }
	};

	// ==============================================================================================

	class Signal
	{
	public:

		Signal() {}
		explicit Signal(const SignalParam& param);
		virtual ~Signal() {}

	private:

		SignalParam m_param;
		SignalState m_state;

		SignalStatistic m_statistic;

	public:

		SignalParam& param() { return m_param; }
		const SignalParam& param() const { return m_param; }
		void setParam(const Metrology::SignalParam& param) { m_param = param; }

		SignalState& state() { return m_state; }
		const SignalState& state() const { return m_state; }
		void setState(const Metrology::SignalState& state) { m_state = state; }

		SignalStatistic& statistic() { return m_statistic; }
		const SignalStatistic& statistic() const { return m_statistic; }
		void setStatistic(const SignalStatistic& statistic) { m_statistic = statistic; }
	};

	// ==============================================================================================



	// ----------------------------------------------------------------------------------------------
	class ComparatorEx : public ::Comparator
	{
	public:

		ComparatorEx() {}
		explicit ComparatorEx(Comparator* pComparator);
		virtual ~ComparatorEx() {}

		enum DeviationType
		{
			NoUsed,
			Down,
			Up,
		};

	private:

		int m_index = -1;

		Metrology::Signal* m_inputSignal = nullptr;
		Metrology::Signal* m_compareSignal = nullptr;
		Metrology::Signal* m_hysteresisSignal = nullptr;
		Metrology::Signal* m_outputSignal = nullptr;

		DeviationType m_deviationType = DeviationType::NoUsed;		// for comparators Equal and NotEqual; for comparators Less and Greate deviationType = DeviationType::NoUsed

	public:

		void clear();
		bool signalsIsValid() const;

		int index() const { return m_index; }
		void setIndex(int index) { m_index = index; }

		Metrology::Signal* inputSignal() const { return m_inputSignal; }
		void setInputSignal(Metrology::Signal* pSignal) { m_inputSignal = pSignal; }

		Metrology::Signal* compareSignal() const { return m_compareSignal; }
		void setCompareSignal(Metrology::Signal* pSignal) { m_compareSignal = pSignal; }

		Metrology::Signal* hysteresisSignal() const { return m_hysteresisSignal; }
		void setHysteresisSignal(Metrology::Signal* pSignal) { m_hysteresisSignal = pSignal; }

		Metrology::Signal* outputSignal() const { return m_outputSignal; }
		void setOutputSignal(Metrology::Signal* pSignal) { m_outputSignal = pSignal; }

		DeviationType deviation() const { return m_deviationType; }
		void setDeviation(DeviationType type) { m_deviationType = type; }

		QString cmpTypeStr() const;

		int valuePrecision() const;

		double compareOnlineValue() const;				// current online (run time) value
		QString compareOnlineValueStr() const;			// str current oline (run time) value
		double compareConstValue() const;				// default offine value
		QString compareDefaultValueStr() const;			// str default offine value

		double hysteresisOnlineValue() const;			// current oline (run time) value
		QString hysteresisOnlineValueStr() const;		// str current oline (run time) value
		QString hysteresisDefaultValueStr() const;		// str default offine value

		bool outputState() const;
		QString outputStateStr() const;
		QString outputStateStr(const QString& forTrue, const QString& forFalse) const;
	};


	// ==============================================================================================
}
