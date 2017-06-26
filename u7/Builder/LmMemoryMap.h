#pragma once

#include "../lib/Address16.h"
#include "../lib/Signal.h"
#include "../lib/OutputLog.h"
#include "../lib/LmLimits.h"
#include "IssueLogger.h"


namespace Builder
{

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

	private:
		int m_startAddress = 0;
		int m_sizeW = 0;

		bool m_locked = false;

		Address16 m_nextSignalAddress;
		QVector<SignalAddress16> m_signals;

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

		Address16 appendSignal(const Signal &signal);

		bool hasSignals() const { return m_signals.size() > 0; }

		QVector<SignalAddress16>& getSignals() { return m_signals; }
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
			MemoryArea result;

		} m_optoInterface;

		struct
		{
			MemoryArea memory;

			MemoryArea bitAccumulator;

			MemoryArea acquiredDiscreteSignals;

			MemoryArea nonAcquiredDiscreteSignals;

		} m_appBitAdressed;

		struct
		{
			MemoryArea memory;

		} m_tuningInterface;

		struct
		{
			MemoryArea memory;

			MemoryArea regRawData;							// registered raw data
			MemoryArea acquiredAnalogSignals;
			MemoryArea acquiredAnalogTuningSignals;
			MemoryArea acquiredBusses;
			MemoryArea acquiredDiscreteSignals;				// copying from this->appBitAdressed.regDiscretSignals
			MemoryArea regTuningSignals;
			MemoryArea nonRegAnalogSignals;

		} m_appWordAdressed;

		struct
		{
			MemoryArea memory;
		} m_lmDiagnostics;

		struct
		{
			MemoryArea memory;
		} m_lmInOuts;

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
		void addRecord(QStringList& memFile, MemoryArea& memArea, const QString& title);
		void addSignals(QStringList& memFile, MemoryArea& memArea);

	public:

		LmMemoryMap(IssueLogger* log);

		bool init(	const MemoryArea& moduleData,
					const MemoryArea& optoInterfaceData,
					const MemoryArea& appLogicBitData,
					const MemoryArea& tuningData,
					const MemoryArea& appLogicWordData,
					const MemoryArea& lmDiagData,
					const MemoryArea& lmIntOutData);

		bool recalculateAddresses();

		int lmDiagnosticsAddress() const { return m_lmDiagnostics.memory.startAddress(); }
		int lmDiagnosticsSizeW() const { return m_lmDiagnostics.memory.sizeW(); }

		int lmInOutsAddress() const { return m_lmInOuts.memory.startAddress(); }
		int lmInOutsSizeW() const { return m_lmInOuts.memory.sizeW(); }

		int regDiscreteSignalsAddress() const { return m_appBitAdressed.acquiredDiscreteSignals.startAddress(); }
		int regDiscreteSignalsSizeW() const { return m_appBitAdressed.acquiredDiscreteSignals.sizeW(); }

		int appBitMemoryStart() const { return m_appBitAdressed.memory.startAddress(); }
		int appBitMemorySizeW() const { return m_appBitAdressed.memory.sizeW(); }

		int bitAccumulatorAddress() const { return m_appBitAdressed.bitAccumulator.startAddress(); }

		int appWordMemoryStart() const { return m_appWordAdressed.memory.startAddress(); }
		int appWordMemorySizeW() const { return m_appWordAdressed.memory.sizeW(); }

		// rb_* - adrresses and sizes in Registration Buffer
		//
		int rb_regDiscreteSignalsAddress() const { return m_appWordAdressed.acquiredDiscreteSignals.startAddress(); }

		int rb_lmInputsAddress() const { return m_appWordAdressed.lmInputs.startAddress(); }
		int rb_lmOutputsAddress() const { return m_appWordAdressed.lmOutputs.startAddress(); }

		//

		int getModuleDataOffset(int place) const;
		int getModuleRegDataOffset(int place) const;

		int getRegBufStartAddr() const;

		int addModule(int place, int moduleAppRegDataSize);

		void getFile(QStringList& memFile);

		Address16 appendAcquiredDiscreteSignal(const Signal& signal);
		Address16 appendNonAcquiredDiscreteSignal(const Signal& signal);

		Address16 setRegRawDataSize(int sizeW);

		Address16 appendAcquiredAnalogSignal(const Signal& signal);
		Address16 appendAcquiredAnalogTuningSignal(const Signal& signal);
		Address16 appendAcquiredBus(const Signal& signal);

		Address16 appendAcquiredDiscreteSignalToRegBuf(const Signal& signal);
		Address16 addRegTuningSignal(const Signal& signal);
		Address16 addNonRegAnalogSignal(const Signal& signal);
		Address16 appendNonAcquiredBus(const Signal& signal);

		double bitAddressedMemoryUsed();
		double wordAddressedMemoryUsed();

		int getAppDataSize() const { return m_appWordAdressed.nonRegAnalogSignals.startAddress() - m_appWordAdressed.memory.startAddress(); }

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
