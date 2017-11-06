#pragma once

#include "../lib/Address16.h"
#include "../lib/Signal.h"
#include "../lib/OutputLog.h"
#include "../lib/LmLimits.h"
#include "IssueLogger.h"


namespace Builder
{

	class UalSignal;

	struct MemoryArea
	{
	public:
		struct SignalAddress16
		{
		private:
			Address16 m_address;
			QString m_signalStrID;
			int m_sizeW = 0;
			bool m_isDiscrete = false;

		public:
			SignalAddress16() {}

			SignalAddress16(const QString& strID, const Address16& address, int sizeW, bool isDiscrete) :
				m_address(address),
				m_signalStrID(strID),
				m_sizeW(sizeW),
				m_isDiscrete(isDiscrete)
			{
			}

			SignalAddress16(const SignalAddress16& sa)
			{
				m_signalStrID = sa.m_signalStrID;
				m_address = sa.m_address;
				m_sizeW = sa.m_sizeW;
				m_isDiscrete = sa.m_isDiscrete;
			}

			void setSignalStrID(const QString& strID) { m_signalStrID = strID; }
			QString signalStrID() const { return m_signalStrID; }

			void setAddress(const Address16& address) { m_address = address; }
			Address16 address() const { return m_address; }

			void setSizeW(int sizeW) { m_sizeW = sizeW; }
			int sizeW() const { return m_sizeW; }

			void setDiscrete(bool discrete) { m_isDiscrete = discrete; }
			bool isDiscrete() const { return m_isDiscrete; }
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

		bool hasSignals() const { return m_signals.size() > 0; }

		QVector<SignalAddress16>& getSignals() { return m_signals; }

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

	private:

		struct
		{
			MemoryArea memory;

			MemoryArea module[MODULES_COUNT];
		} m_modules;

		struct
		{
			MemoryArea memory;

			MemoryArea channel[OPTO_INTERFACE_COUNT];
			MemoryArea reserv;

		} m_optoInterface;

		struct
		{
			MemoryArea memory;

			MemoryArea bitAccumulator;

			MemoryArea acquiredDiscreteOutputSignals;
			MemoryArea acquiredDiscreteInternalSignals;

			MemoryArea nonAcquiredDiscreteOutputSignals;
			MemoryArea nonAcquiredDiscreteInternalSignals;

		} m_appBitAdressed;

		struct
		{
			MemoryArea memory;

		} m_tuningInterface;

		struct
		{
			MemoryArea memory;

			MemoryArea acquiredRawData;							// registered raw data

			MemoryArea acquiredAnalogInputSignals;
			MemoryArea acquiredAnalogOutputSignals;
			MemoryArea acquiredAnalogInternalSignals;
			MemoryArea acquiredAnalogOptoSignals;
			MemoryArea acquiredAnalogBusChildSignals;
			MemoryArea acquiredAnalogTuningSignals;
			MemoryArea acquiredAnalogConstSignals;

			MemoryArea acquiredBuses;

			MemoryArea acquiredDiscreteInputSignals;
			MemoryArea acquiredDiscreteOutputSignals;				// copying from this->appBitAdressed.acquiredDiscreteOutputSignals
			MemoryArea acquiredDiscreteInternalSignals;				// copying from this->appBitAdressed.acquiredDiscreteInternalSignals
			MemoryArea acquiredDiscreteOptoAndBusChildSignals;
			MemoryArea acquiredDiscreteTuningSignals;
			MemoryArea acquiredDiscreteConstSignals;

			MemoryArea nonAcquiredAnalogInputSignals;
			MemoryArea nonAcquiredAnalogOutputSignals;
			MemoryArea nonAcquiredAnalogInternalSignals;

			MemoryArea nonAcquiredBuses;

			MemoryArea wordAccumulator;

		} m_appWordAdressed;

		struct ReadWriteAccess
		{
			int readCount = 0;
			int writeCount = 0;
		};

		QVector<ReadWriteAccess> m_memory;

		IssueLogger* m_log = nullptr;

		// meory reporting functions nad variables

		int m_sectionStartAddrW = -1;

		void addSection(QStringList& memFile, MemoryArea& memArea, const QString& title, int sectionStartAddrW = -1);
		void addRecordSignals(QStringList& memFile, MemoryArea& memArea, const QString& title);
		void addRecord(QStringList& memFile, MemoryArea& memArea, const QString& title);
		void addSignals(QStringList& memFile, MemoryArea& memArea);


	public:

		LmMemoryMap(IssueLogger* log);

		bool init(const MemoryArea& moduleData,
					const MemoryArea& optoInterfaceData,
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

		int bitAccumulatorAddress() const { return m_appBitAdressed.bitAccumulator.startAddress(); }
		int wordAccumulatorAddress() const { return m_appWordAdressed.wordAccumulator.startAddress(); }

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
		int getModuleRegDataOffset(int place) const;

		int getRegBufStartAddr() const;

		void getFile(QStringList& memFile);

		bool appendUalSignals(MemoryArea& memArea, const QVector<UalSignal*>& ualSignals);
		bool appendRegSignals(MemoryArea& memArea, const QVector<UalSignal*>& ualSignals, bool setUalAddrEqualToRegBufAddr);
		bool appendRegAnalogConstSignals(MemoryArea& memArea, const QVector<UalSignal*>& ualSignals);

		bool appendAcquiredDiscreteStrictOutputSignals(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredDiscreteInternalSignals(const QVector<UalSignal*>& ualSignals);
		bool appendNonAcquiredDiscreteStrictOutputSignals(const QVector<UalSignal*>& ualSignals);
		bool appendNonAcquiredDiscreteInternalSignals(const QVector<UalSignal*>& ualSignals);

		bool appendAcquiredDiscreteInputSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredDiscreteStrictOutputSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredDiscreteInternalSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredDiscreteOptoAndBusChildSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);

		bool appendAcquiredDiscreteTuningSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredAnalogTuningSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredDiscreteConstSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);

		bool appendAcquiredAnalogInputSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredAnalogStrictOutputSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredAnalogInternalSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredAnalogOptoSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredAnalogBusChildSignalsInRegBuf(const QVector<UalSignal*>& ualSignals);
		bool appendAcquiredAnalogConstSignalsInRegBuf(const QHash<int, UalSignal *>& acquiredAnalogConstIntSignals,
													  const QHash<float, UalSignal *>& acquiredAnalogConstFloatSignals);
		bool appendAcquiredBussesInRegBuf(const QVector<UalSignal*>& ualSignals);

		bool appendNonAcquiredAnalogInputSignals(const QVector<UalSignal*>& ualSignals);
		bool appendNonAcquiredAnalogStrictOutputSignals(const QVector<UalSignal*>& ualSignals);
		bool appendNonAcquiredAnalogInternalSignals(const QVector<UalSignal*>& ualSignals);

		bool appendNonAcquiredBusses(const QVector<UalSignal*>& ualSignals);

		Address16 setAcquiredRawDataSize(int sizeW);

/*		Address16 appendAcquiredAnalogInputSignal(const Signal& signal);
		Address16 appendAcquiredAnalogOutputSignal(const Signal& signal);
		Address16 appendAcquiredAnalogInternalSignal(const Signal& signal);
		Address16 appendAcquiredAnalogTuningSignal(const Signal& signal);

		Address16 appendAcquiredBus(const Signal& signal);

		Address16 appendAcquiredDiscreteOutputSignalInRegBuf(const Signal& signal);
		Address16 appendAcquiredDiscreteInternalSignalInRegBuf(const Signal& signal);

		Address16 appendAcquiredDiscreteTuningSignal(const Signal& signal);

		Address16 appendNonAcquiredAnalogInputSignal(const Signal& signal);
		Address16 appendNonAcquiredAnalogOutputSignal(const Signal& signal);
		Address16 appendNonAcquiredAnalogInternalSignal(const Signal& signal);

		Address16 appendNonAcquiredBus(const Signal& signal);*/

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
	};

}
