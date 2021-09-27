#pragma once

#include <vector>

#include "../UtilsLib/Address16.h"
#include "../VFrame30/Bus.h"
#include "IssueLogger.h"
#include "BuildResultWriter.h"
#include "../lib/ConstStrings.h"

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

		inline static const std::vector<InbusConvDescription> m_inbusConvDescriptions =
		{
			// SI32 -> SI32
			//
			{ E::AnalogAppSignalFormat::SignedInt32, E::DataFormat::SignedInt, SIZE_32BIT,
				Afb::SCALE_SI32_SI32, SIZE_32BIT, false, Afb::NO_AFB, SIZE_32BIT,
				false, Afb::NO_AFB, SIZE_32BIT, Afb::SCALE_SI32_SI32, SIZE_32BIT },

			// SI32 -> FP32

			{ E::AnalogAppSignalFormat::SignedInt32, E::DataFormat::Float, SIZE_32BIT,
				Afb::SCALE_SI32_FP32, SIZE_32BIT, false, Afb::TCONV_SI32_FP32, SIZE_32BIT,
				false, Afb::TCONV_SI32_FP32, SIZE_32BIT, Afb::SCALE_FP32_SI32, SIZE_32BIT },

			// SI32 -> SI16
			//
			{ E::AnalogAppSignalFormat::SignedInt32, E::DataFormat::SignedInt, SIZE_16BIT,
				Afb::SCALE_SI32_SI32, SIZE_32BIT, true, Afb::SW_TCONV_SI32_SI16, SIZE_16BIT,
				true, Afb::TCONV_SI16_SI32 + Afb::OR + Afb::SW_TCONV_SI16_SI32, SIZE_32BIT, Afb::SCALE_SI32_SI32, SIZE_32BIT },

			// SI32 -> UI16
			//
			{ E::AnalogAppSignalFormat::SignedInt32, E::DataFormat::UnsignedInt, SIZE_16BIT,
				Afb::SCALE_SI32_UI16, SIZE_32BIT, false, Afb::SW_TCONV_SI32_UI16, SIZE_16BIT,
				false, Afb::TCONV_UI16_SI32 + Afb::OR + Afb::SW_TCONV_UI16_SI32, SIZE_32BIT, Afb::SCALE_UI16_SI32, SIZE_32BIT },

/*

			{ E::AnalogAppSignalFormat::Float32, E::DataFormat::SignedInt, SIZE_32, "scale_fp_si", SIZE_32BIT, false, "tconv_fp_si", SIZE_32BIT },
			{ E::AnalogAppSignalFormat::Float32, E::DataFormat::Float, SIZE_32, "scale_fp_fp", SIZE_32BIT, false, "", SIZE_32BIT },
			{ E::AnalogAppSignalFormat::Float32, E::DataFormat::SignedInt, SIZE_16, "scale_fp_si", SIZE_32BIT, true, "sw_tconv_si_16si", SIZE_16BIT },
			{ E::AnalogAppSignalFormat::Float32, E::DataFormat::UnsignedInt, SIZE_16, "scale_fp_si", SIZE_32BIT, true, "sw_tconv_si_16ui", SIZE_16BIT },*/

		};

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

		//

		bool conversionRequired() const;

//		bool is_SInt32_To_UInt16_BE_NoScale_conversion() const;
//		bool is_SInt32_To_SInt16_BE_NoScale_conversion() const;

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
		bool scalingRequired() const;
		bool typeConversionRequired() const;
		bool byteOrderConversionRequired() const;

		void getInbusScalingCoefficients(double* k, double* b) const;
		void getFrombusScalingCoefficients(double* k, double* b) const;

		InbusConvDescription getInbusConvDescription() const;

		void init(const Busses& busses, const VFrame30::BusSignal& bs);
		bool isOverlaped(const BusSignal& bs);

		bool isValid() const;
	};

	class Bus
	{
	public:
		static const QString INVALUD_BUS_SIGNAL_ID;

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

		VFrame30::BusSignal m_invalidBusSignal;
		BusSignal m_invalidSignal;

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

		int count() const { return m_busses.count(); }

	private:
		bool getBusInitOrder(QVector<BusShared>* busInitOrder);

	private:
		VFrame30::BusSet* m_busSet = nullptr;
		IssueLogger* m_log = nullptr;

		QHash<QString, BusShared> m_busses;
	};
}
