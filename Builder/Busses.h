#pragma once

#include <vector>

#include "../UtilsLib/Address16.h"
#include "../AppSignalLib/Bus.h"
#include "IssueLogger.h"
#include "BuildResultWriter.h"
#include <CommonLib/ConstStrings.h>

namespace Builder
{
	class Busses;

	struct InbusConvDescription
	{
		// Input/Output analog bus child signal format
		//
		E::AnalogAppSignalFormat busSignalFormat = E::AnalogAppSignalFormat::SignedInt32;

		// Inbus analog signal format
		//
		E::DataFormat inbusAnalogFormat  = E::DataFormat::SignedInt;
		int inbusSizeBits = 0;

		// Inbus conversion chain description
		//

		// Scaling AFB, if scaling required
		//
		QString inbusScalingAfb;
		int inbusScalingResultSizeBits = 0;
		bool inbusTypeConvAfterScalingRequired = false;

		// Inbus type conversion AFB, if type conversion required
		//
		QString inbusTypeConversionAfb;
		int inbusTypeConversionResultSizeBits = 0;

		// Frombus conversion chain description
		//

		// Type conversion AFB, if type conversion required
		//
		bool frombusTypeConvBeforeScalingRequired = false;
		QString frombusTypeConversionAfb;
		int frombusTypeConversionResultSizeBits = 0;

		QString frombusScalingAfb;
		int frombusScalingResultSizeBits = 0;

		bool isValid() const { return inbusSizeBits != 0; }
	};

	class BusSignal
	{
	private:

		static const std::vector<InbusConvDescription> m_inbusConvDescriptions;

	public:
		QString signalID;
		QString caption;
		E::SignalType signalType = E::SignalType::Discrete;
		Address16 inbusAddr;
		QString busTypeID;
		QString units;

		// Bus child analog signal params (In/Out signal for BusComposer and BusExtractor respectively)
		//
		E::AnalogAppSignalFormat inOutAnalogFormat = E::AnalogAppSignalFormat::Float32;
		double inOutAnalogLowLimit = 0.0;
		double inOutAnalogHighLimit = 65535.0;

		// Params of analog signal stored in bus
		//
		E::DataFormat inbusAnalogFormat  = E::DataFormat::SignedInt;
		int inbusSizeBits = 0;
		E::ByteOrder inbusAnalogByteOrder = E::ByteOrder::BigEndian;

		double inbusAnalogLowLimit = 0.0;
		double inbusAnalogHighLimit = 65535.0;

		// Order of inbus signal conversion:
		//
		// Scaling -> Type conversion -> ByteOrder conversion
		//
		// Order of frombus signal conversion:
		//
		// ByteOrder conversion -> Type conversion -> Scaling
		//
		// In some cases Scaling and Type conversion can be performed in time of Scaling
		//
		bool conversionRequired() const;

		bool scalingRequired() const;
		bool typeConversionRequired() const;
		bool byteOrderConversionRequired() const;

		void getInbusScalingCoefficients(double* k, double* b) const;
		void getFrombusScalingCoefficients(double* k, double* b) const;

		InbusConvDescription getInbusConvDescription() const;

		void init(const Busses& busses, const AppSignalLib::BusSignal& bs);
		bool isOverlaped(const BusSignal& bs);

		bool isValid() const;

		int inbusOffset() const;
	};

	class Bus
	{
	public:
		static const QString INVALUD_BUS_SIGNAL_ID;

	public:
		Bus(const Busses& busses, const AppSignalLib::Bus& bus, IssueLogger* log);

		bool init();

		void writeReport(QStringList& list);

		int sizeW() const;
		int sizeB() const;
		int sizeBit() const;

		QString busTypeID() const { return m_srcBus.busTypeId(); }

		E::BusDataFormat busDataFormat() const { return m_busDataFormat; }

		const BusSignal& signalByID(const QString& signalID) const;
		const BusSignal& signalByIndex(int index) const;
		const std::vector<int>& analogSignalIndexes() const { return m_analogSignalIndexes; }
		const std::map<int, std::vector<int>>& discreteSignalIndexes() const { return m_discreteSignalIndexes; }

		const QVector<BusSignal>& busSignals() const { return m_signals; }

		const AppSignalLib::Bus& srcBus() const { return m_srcBus; }
		AppSignalLib::BusSignal& getBusSignal(const QString& signalID);

		QStringList getChildBussesIDs();

		bool isInitialized() const { return m_isInitialized; }

		const Busses& busses() const { return m_busses; }

	private:
		bool buildInBusSignalsMap();
		bool autoPlaceSignals();
		bool buildSignalsOrder();
		bool calcBusSizeW();
		bool checkSignalsOffsets();
		void buildSignalIndexesArrays();

	private:
		const Busses& m_busses;
		AppSignalLib::Bus m_srcBus;
		IssueLogger* m_log = nullptr;

		//

		QHash<QString, int>	m_inBusSignalsMap;	// in bus signalID => signal index in m_srcBus.signals

		E::BusDataFormat m_busDataFormat = E::BusDataFormat::Mixed;
		int m_sizeW = -1;

		QVector<BusSignal> m_signals;

		std::vector<int> m_analogSignalIndexes;
		std::vector<int> m_busSignalIndexes;
		std::map<int, std::vector<int>> m_discreteSignalIndexes;		// discrete signals offset => discrete signal indexes

		AppSignalLib::BusSignal m_invalidBusSignal;
		BusSignal m_invalidSignal;

		bool m_isInitialized = false;
	};

	typedef std::shared_ptr<Bus> BusShared;

	class Busses
	{
	public:
		Busses(AppSignalLib::BusSet* busSet, IssueLogger* log);
		virtual ~Busses();

		bool prepare();
		bool writeReport(BuildResultWriter* resultWriter);

		BusShared getBus(const QString& busTypeID) const;
		int getBusSizeBits(const QString& busTypeID) const;

		int count() const { return static_cast<int>(m_busses.count()); }

	private:
		bool getBusInitOrder(QVector<BusShared>* busInitOrder);

	private:
		AppSignalLib::BusSet* m_busSet = nullptr;
		IssueLogger* m_log = nullptr;

		QHash<QString, BusShared> m_busses;
	};
}
