#pragma once

#include "../lib/Address16.h"
#include "../VFrame30/Bus.h"
#include "IssueLogger.h"
#include "BuildResultWriter.h"


namespace Builder
{
	class Busses;

	class BusSignal
	{
	public:
		QString signalID;
		QString caption;
		E::SignalType signalType = E::SignalType::Discrete;
		Address16 inbusAddr;
		QString busTypeID;

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
		void init(const Busses& busses, const VFrame30::BusSignal& bs);
		bool isOverlaped(const BusSignal& bs);

		bool isValid() const;
	};

	class Bus
	{
	public:
		static QString INVALUD_BUS_SIGNAL_ID;

	public:
		Bus(const Busses& busses, const VFrame30::Bus bus, IssueLogger* log);

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

		const VFrame30::Bus& srcBus() const { return m_srcBus; }
		VFrame30::BusSignal& getBusSignal(const QString& signalID);

		QStringList getChildBussesIDs();

		bool isInitialized() const { return m_isInitialized; }

	private:
		bool buildInBusSignalsMap();
		bool placeSignals();
		bool buildSignalsOrder();
		bool checkSignalsOffsets();
		void buildSignalIndexesArrays();

	private:
		const Busses& m_busses;
		VFrame30::Bus m_srcBus;
		IssueLogger* m_log = nullptr;

		//

		QHash<QString, int>	m_inBusSignalsMap;	// in bus signalID => signal index in m_srcBus.signals

		E::BusDataFormat m_busDataFormat = E::BusDataFormat::Mixed;
		int m_sizeW = -1;

		QVector<BusSignal> m_signals;

		std::vector<int> m_analogSignalIndexes;
		std::vector<int> m_busSignalIndexes;
		std::map<int, std::vector<int>> m_discreteSignalIndexes;		// discrete signals offset => discrete signal indexes

		static VFrame30::BusSignal m_invalidBusSignal;
		static BusSignal m_invalidSignal;

		bool m_isInitialized = false;
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
		int getBusSizeBits(const QString& busTypeID) const;

	private:
		bool getBusInitOrder(QVector<BusShared>* busInitOrder);

	private:
		VFrame30::BusSet* m_busSet = nullptr;
		IssueLogger* m_log = nullptr;

		QHash<QString, BusShared> m_busses;
	};
}
