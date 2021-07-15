#pragma once

#include "../UtilsLib/Address16.h"
#include "IssueLogger.h"

namespace Builder
{

	class UalSignal;

	struct MemoryArea								// Memory area with variable address and size disposed in MemoryDomain
	{
	public:
		struct SignalAddress16
		{
		public:
			SignalAddress16() {}

			SignalAddress16(const QString& strID, const Address16& address, int sizeW, E::SignalType signalType) :
				m_address(address),
				m_signalID(strID),
				m_sizeW(sizeW),
				m_signalType(signalType)
			{
			}

			SignalAddress16(const SignalAddress16& sa)
			{
				m_signalID = sa.m_signalID;
				m_address = sa.m_address;
				m_sizeW = sa.m_sizeW;
				m_signalType = sa.m_signalType;
			}

			void setSignalStrID(const QString& strID) { m_signalID = strID; }
			QString signalStrID() const { return m_signalID; }
			void clearSignalID() { m_signalID.clear(); }
			void appendSignalID(const QString &signalID);

			void setAddress(const Address16& address) { m_address = address; }
			Address16 address() const { return m_address; }

			void setSizeW(int sizeW) { m_sizeW = sizeW; }
			int sizeW() const { return m_sizeW; }

			bool isDiscrete() const { return m_signalType == E::SignalType::Discrete; }

		private:
			Address16 m_address;
			QString m_signalID;
			int m_sizeW = 0;
			E::SignalType m_signalType = E::SignalType::Discrete;
		};

	public:
		void setStartAddress(int startAddress);

		void setSizeW(int sizeW) { assert(m_locked == false); m_sizeW = sizeW; }

		int startAddress() const { return m_startAddress; }
		int sizeW() const { return m_sizeW; }

		int* ptrStartAddress() { return &m_startAddress; }
		int* ptrSizeW() { return &m_sizeW; }

		void lock() { m_locked = true; }
		void unlock() { m_locked = false; }

		int nextAddress() const { return m_startAddress + m_sizeW; }

		MemoryArea& operator = (const MemoryArea& ma);

		Address16 appendSignal(const UalSignal* ualSignal, bool appendAcquiredOnly);
		bool appendSignalWithFixedAddress(const UalSignal* ualSignal);

		bool hasSignals() const { return m_signals.size() > 0; }

		QVector<SignalAddress16>& getSignals() { return m_signals; }
		QVector<SignalAddress16> getSignalsJoined() const;

		void appendUalRefSignals(const Address16& addr16, const UalSignal* ualSignal, bool appendAcquiredOnly);

	private:
		int m_startAddress = 0;
		int m_sizeW = 0;

		bool m_locked = false;

		Address16 m_nextSignalAddress;
		QVector<SignalAddress16> m_signals;

	};

	class LmMemoryMap : public QObject
	{
		Q_OBJECT

	public:
		LmMemoryMap(IssueLogger* log);

		bool init(int appMemorySize,
					const MemoryArea& ioModuleData,
					int ioModulesCount,
					const MemoryArea& optoInterfaceData,
					int optoInterfaceCount,
					const MemoryArea& appLogicBitData,
					const MemoryArea& tuningData,
					const MemoryArea& appLogicWordData);

		bool recalculateAddresses();

		int acquiredDiscreteOutputSignalsAddress() const { return m_appBitAdressed.acquiredDiscreteOutputSignals.startAddress(); }
		int acquiredDiscreteInternalSignalsAddress() const { return m_appBitAdressed.acquiredDiscreteInternalSignals.startAddress(); }

		int acquiredDiscreteOutputSignalsSizeW() const { return m_appBitAdressed.acquiredDiscreteOutputSignals.sizeW(); }
		int acquiredDiscreteInternalSignalsSizeW() const { return m_appBitAdressed.acquiredDiscreteInternalSignals.sizeW(); }

		int appBitMemoryStart() const { return m_appBitAdressed.memory.startAddress(); }
		int appBitMemorySizeW() const { return m_appBitAdressed.memory.sizeW(); }

		int appBitMemoryDiscreteSignalsHeapStart() const { return m_appBitAdressed.discreteSignalsHeap.startAddress(); }
		void setAppBitMemoryDiscreteSignalsHeapSizeW(int sizeW) { m_appBitAdressed.discreteSignalsHeap.setSizeW(sizeW); }

		int bitAccumulatorAddress() const { return m_appBitAdressed.bitAccumulator.startAddress(); }
		int wordAccumulatorAddress() const { return m_appWordAdressed.wordAccumulator.startAddress(); }

		int appWordMemoryAnalogAndBusSignalsHeapStart() const { return m_appWordAdressed.analogAndBusSignalsHeap.startAddress(); }
		void setAppWordMemoryAnalogAndBusSignalsHeapSizeW(int sizeW) { m_appWordAdressed.analogAndBusSignalsHeap.setSizeW(sizeW); }

		int appWordMemoryStart() const { return m_appWordAdressed.memory.startAddress(); }
		int appWordMemorySizeW() const { return m_appWordAdressed.memory.sizeW(); }

		int regBufStartAddr() const { return m_appWordAdressed.acquiredRawData.startAddress(); }
		int regBufSizeW() const;

		int acquiredRawDataAddress() const { return m_appWordAdressed.acquiredRawData.startAddress(); }

		int acquiredDiscreteInputSignalsAddressInRegBuf() const { return m_appWordAdressed.acquiredDiscreteInputSignals.startAddress(); }
		int acquiredDiscreteOutputSignalsAddressInRegBuf() const { return m_appWordAdressed.acquiredDiscreteOutputSignals.startAddress(); }
		int acquiredDiscreteInternalSignalsAddressInRegBuf() const { return m_appWordAdressed.acquiredDiscreteInternalSignals.startAddress(); }
		int acquiredDiscreteConstSignalsAddressInRegBuf() const { return m_appWordAdressed.acquiredDiscreteConstSignals.startAddress(); }

		int acquiredDiscreteOutputSignalsInRegBufSizeW() const {  return m_appWordAdressed.acquiredDiscreteOutputSignals.sizeW(); }
		int acquiredDiscreteInternalSignalsInRegBufSizeW() const {  return m_appWordAdressed.acquiredDiscreteInternalSignals.sizeW(); }
		int acquiredDiscreteConstSignalsInRegBufSizeW() const { return m_appWordAdressed.acquiredDiscreteConstSignals.sizeW(); }

		//

		int getModuleDataOffset(int place) const;

		void getFile(QStringList& memFile,
					 const std::vector<std::tuple<QString, Address16, int>>& discreteSignalHeapItems,
					 const std::vector<std::tuple<QString, Address16, int>>& analogAndBusSignalHeapItems);

		bool appendUalSignals(MemoryArea& memArea, const QVector<UalSignal*>& ualSignals);
		bool appendRegSignals(MemoryArea& memArea, const QVector<UalSignal*>& ualSignals, bool setUalAddrEqualToRegBufAddr);
		bool appendRegAnalogConstSignals(MemoryArea& memArea, const QVector<UalSignal*>& ualSignals);
		bool appendTuningSignal(const UalSignal* tuningSignal);

		bool appendAcquiredDiscreteStrictOutputSignals(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredDiscreteInternalSignals(const QVector<UalSignal*>& ualSignals);
		bool appendNonAcquiredDiscreteStrictOutputSignals(const QVector<UalSignal*>& ualSignals);
		bool appendNonAcquiredDiscreteInternalSignals(const QVector<UalSignal*>& ualSignals);

		bool appendAcquiredDiscreteInputSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredDiscreteStrictOutputSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredDiscreteInternalSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredDiscreteOptoSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredDiscreteBusChildSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);

		bool appendAcquiredDiscreteTuningSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredAnalogTuningSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredDiscreteConstSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);

		bool appendAcquiredAnalogInputSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredAnalogStrictOutputSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredAnalogInternalSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredAnalogOptoSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredAnalogBusChildSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredAnalogConstSignalsInRegBuf(const QMultiHash<int, UalSignal *>& acquiredAnalogConstIntSignals,
													  const QMultiHash<float, UalSignal *>& acquiredAnalogConstFloatSignals);

		bool appendAcquiredInputBusesInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredOutputBusesInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredInternalBusesInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredBusChildBusesInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredOptoBusesInRegBuf(const QVector<UalSignal*>& ualSignals);

		bool appendNonAcquiredAnalogInputSignals(const QVector<UalSignal*>& ualSignals);
		bool appendNonAcquiredAnalogStrictOutputSignals(const QVector<UalSignal*>& ualSignals);
		bool appendNonAcquiredAnalogInternalSignals(const QVector<UalSignal*>& ualSignals);

		bool appendNonAcquiredOutputBusses(const QVector<UalSignal*>& ualSignals);
		bool appendNonAcquiredInternalBusses(const QVector<UalSignal*>& ualSignals);

		Address16 setAcquiredRawDataSize(int sizeW);

		double bitAddressedMemoryUsed();
		double wordAddressedMemoryUsed();

		bool read16(int address);
		bool read32(int address);
		bool readArea(int startAddress, int size);

		bool write16(int address);
		bool write32(int address);
		bool writeArea(int startAddress, int size);

		int getMemoryReadCount(int address) const;
		int getMemoryWriteCount(int address) const;

		Address16 constBit0Addr() const;
		Address16 constBit1Addr() const;

		bool addressInBitMemory(int address) const;
		bool addressInWordMemory(int address) const;

	private:
		void addSection(QStringList& memFile, MemoryArea& memArea, const QString& title, int sectionStartAddrW = -1);
		void addRecordSignals(QStringList& memFile, MemoryArea& memArea, const QString& title);
		void addRecord(QStringList& memFile, MemoryArea& memArea, const QString& title);
		void addSignals(QStringList& memFile, MemoryArea& memArea);

	private:

		struct
		{
			MemoryArea memory;

			std::vector<MemoryArea> module;
		} m_modules;

		struct
		{
			MemoryArea memory;

			std::vector<MemoryArea> channel;
			MemoryArea reserv;

		} m_optoInterface;

		struct
		{
			MemoryArea memory;

			MemoryArea bitAccumulator;
			MemoryArea constBits;

			MemoryArea acquiredDiscreteOutputSignals;
			MemoryArea acquiredDiscreteInternalSignals;

			MemoryArea nonAcquiredDiscreteOutputSignals;
			MemoryArea nonAcquiredDiscreteInternalSignals;

			MemoryArea discreteSignalsHeap;

		} m_appBitAdressed;

		struct
		{
			MemoryArea memory;

		} m_tuningInterface;

		struct
		{
			MemoryArea memory;

			MemoryArea acquiredRawData;								// registered raw data

			MemoryArea acquiredAnalogInputSignals;
			MemoryArea acquiredAnalogOutputSignals;
			MemoryArea acquiredAnalogInternalSignals;
			MemoryArea acquiredAnalogOptoSignals;
			MemoryArea acquiredAnalogBusChildSignals;				// Child signals of Input, Output and Internal busses
			MemoryArea acquiredAnalogTuningSignals;
			MemoryArea acquiredAnalogConstSignals;

			MemoryArea acquiredInputBuses;
			MemoryArea acquiredOutputBuses;
			MemoryArea acquiredInternalBuses;
			MemoryArea acquiredBusChildBuses;
			MemoryArea acquiredOptoBuses;

			MemoryArea acquiredDiscreteInputSignals;
			MemoryArea acquiredDiscreteOutputSignals;				// copying from this->appBitAdressed.acquiredDiscreteOutputSignals
			MemoryArea acquiredDiscreteInternalSignals;				// copying from this->appBitAdressed.acquiredDiscreteInternalSignals
			MemoryArea acquiredDiscreteOptoSignals;
			MemoryArea acquiredDiscreteBusChildSignals;
			MemoryArea acquiredDiscreteTuningSignals;
			MemoryArea acquiredDiscreteConstSignals;

			MemoryArea nonAcquiredAnalogInputSignals;
			MemoryArea nonAcquiredAnalogOutputSignals;
			MemoryArea nonAcquiredAnalogInternalSignals;

			// non-Acquired Input Buses are disposed in IO modules memory
			MemoryArea nonAcquiredOutputBuses;
			MemoryArea nonAcquiredInternalBuses;

			MemoryArea wordAccumulator;

			MemoryArea analogAndBusSignalsHeap;

		} m_appWordAdressed;

		struct ReadWriteAccess
		{
			int readCount = 0;
			int writeCount = 0;
		};

		int m_appMemorySize = 0;
		QVector<ReadWriteAccess> m_memory;

		IssueLogger* m_log = nullptr;

		int m_sectionStartAddrW = -1;
	};

}
