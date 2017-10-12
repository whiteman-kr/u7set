#pragma once

#include "../lib/Address16.h"
#include "../VFrame30/Bus.h"
#include "IssueLogger.h"
#include "BuildResultWriter.h"


namespace Builder
{
	class BusSignal
	{
	public:
		QString signalID;
		E::SignalType signalType = E::SignalType::Discrete;
		Address16 inbusAddr;

		// analog signals conversion parameters
		//
		E::AnalogAppSignalFormat analogFormat = E::AnalogAppSignalFormat::Float32;

		int inbusSizeBits = 0;

		E::DataFormat inbusAnalogFormat  = E::DataFormat::SignedInt;
		E::ByteOrder inbusAnalogByteOrder = E::ByteOrder::BigEndian;

		double busAnalogLowLimit = 0.0;
		double busAnalogHighLimit = 65535.0;

		double inbusAnalogLowLimit = 0.0;
		double inbusAnalogHighLimit = 65535.0;

		//

		bool conversionRequired() const;
		void init(const VFrame30::BusSignal& bs);
		bool isOverlaped(const BusSignal& bs);

		bool isValid() const;
	};

	class Bus
	{
	public:
		static QString INVALUD_BUS_SIGNAL_ID;

	public:
		Bus(const VFrame30::Bus bus, IssueLogger* log);

		bool init();

		void writeReport(QStringList& list);

		int sizeW() const { return m_sizeW; }
		int sizeB() const { return m_sizeW * WORD_SIZE_IN_BYTES; }

		QString busTypeID() const { return m_srcBus.busTypeId(); }

		const BusSignal& signalByID(const QString& signalID) const;
		const BusSignal& signalByIndex(int index) const;
		const std::vector<int>& analogSignalIndexes() const { return m_analogSignalIndexes; }
		const std::map<int, std::vector<int>>& discreteSignalIndexes() const { return m_discreteSignalIndexes; }

	private:
		bool buildInBusSignalsMap();
		bool placeSignals();
		bool buildSignalsOrder();
		bool checkSignalsOffsets();
		void buildSignalIndexesArrays();

		VFrame30::BusSignal& getBusSignal(const QString& signalID);

	private:
		VFrame30::Bus m_srcBus;
		IssueLogger* m_log = nullptr;

		//

		QHash<QString, int>	m_inBusSignalsMap;	// in bus signalID => signal index in m_srcBus.signals

		int m_sizeW = 0;

		QVector<BusSignal> m_signals;

		std::vector<int> m_analogSignalIndexes;
		std::map<int, std::vector<int>> m_discreteSignalIndexes;		// discrete signals offset => discrete signal indexes

		static VFrame30::BusSignal m_invalidBusSignal;
		static BusSignal m_invalidSignal;
	};

	typedef std::shared_ptr<Bus> BusShared;

	class Busses
	{
	public:
		Busses(VFrame30::BusSet* busSet, IssueLogger* log);
		virtual ~Busses();

		bool prepare();
		bool writeReport(BuildResultWriter* resultWriter);

		BusShared getBus(const QString& busTypeID) const;

	private:
		VFrame30::BusSet* m_busSet = nullptr;
		IssueLogger* m_log = nullptr;

		QHash<QString, BusShared> m_busses;
	};
}
