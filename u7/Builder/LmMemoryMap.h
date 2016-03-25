#pragma once

#include "../include/Address16.h"
#include "../include/Signal.h"
#include "../include/OutputLog.h"

#include "../Builder/LmLimits.h"


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
            MemoryArea regDiscretSignals;
            MemoryArea nonRegDiscretSignals;

        } m_appBitAdressed;

        struct
        {
            MemoryArea memory;

        } m_tuningInterface;

        struct
        {
            MemoryArea memory;

            MemoryArea lmDiagnostics;					// copying from this->lmDiagnostics
            MemoryArea lmInputs;						// reads from this->m_lmInOuts area
            MemoryArea lmOutputs;						// writes to this->m_lmInOuts ares
            MemoryArea module[MODULES_COUNT];			// depends from chassis	configuration
            MemoryArea regAnalogSignals;
            MemoryArea regDiscreteSignals;				// copying from this->appBitAdressed.regDiscretSignals
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

        OutputLog* m_log = nullptr;

        void addSection(QStringList& memFile, MemoryArea& memArea, const QString& title);
        void addRecord(QStringList& memFile, MemoryArea& memArea, const QString& title);
        void addSignals(QStringList& memFile, MemoryArea& memArea);

    public:

        LmMemoryMap(OutputLog* log);

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

        int regDiscreteSignalsAddress() const { return m_appBitAdressed.regDiscretSignals.startAddress(); }
        int regDiscreteSignalsSizeW() const { return m_appBitAdressed.regDiscretSignals.sizeW(); }

        int bitAddressedMemoryAddress() const { return m_appBitAdressed.memory.startAddress(); }
        int bitAccumulatorAddress() const { return m_appBitAdressed.bitAccumulator.startAddress(); }
        int wordAddressedMemoryAddress() const { return m_appWordAdressed.memory.startAddress(); }


        // rb_* - adrresses and sizes in Registration Buffer
        //
        int rb_lmDiagnosticsAddress() const { return m_appWordAdressed.lmDiagnostics.startAddress(); }
        int rb_regDiscreteSignalsAddress() const { return m_appWordAdressed.regDiscreteSignals.startAddress(); }

        int rb_lmInputsAddress() const { return m_appWordAdressed.lmInputs.startAddress(); }
        int rb_lmOutputsAddress() const { return m_appWordAdressed.lmOutputs.startAddress(); }

        //

        int getModuleDataOffset(int place);

        int getModuleRegDataOffset(int place);

        int addModule(int place, int moduleAppRegDataSize);

        void getFile(QStringList& memFile);

        Address16 addRegDiscreteSignal(const Signal& signal);
        Address16 addRegDiscreteSignalToRegBuffer(const Signal& signal);
        Address16 addNonRegDiscreteSignal(const Signal& signal);
        Address16 addRegAnalogSignal(const Signal& signal);
        Address16 addNonRegAnalogSignal(const Signal& signal);

        double bitAddressedMemoryUsed();
        double wordAddressedMemoryUsed();
    };

}
