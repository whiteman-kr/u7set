#include "../Builder/LmMemoryMap.h"

namespace Builder
{

    // ---------------------------------------------------------------------------------
    //
    //	MemoryArea class implementation
    //
    // ---------------------------------------------------------------------------------

    void MemoryArea::setStartAddress(int startAddress)
    {
        assert(m_locked == false);
        m_startAddress = startAddress;
        m_nextSignalAddress.set(startAddress, 0);
    }


    MemoryArea& MemoryArea::operator = (const MemoryArea& ma)
    {
        assert(m_locked == false);

        m_startAddress = ma.m_startAddress;
        m_sizeW = ma.m_sizeW;

        return *this;
    }


    Address16 MemoryArea::appendSignal(const Signal& signal)
    {
        Address16 signalAddress;

        if (signal.isAnalog())
        {
            // do word-align
            //
            m_nextSignalAddress.wordAlign();
        }

        signalAddress = m_nextSignalAddress;

        if (signal.isAnalog())
        {
            m_nextSignalAddress.addWord(signal.sizeW());
        }
        else
        {
            m_nextSignalAddress.add1Bit();
        }

        m_signals.append(SignalAddress16(signal.strID(), signalAddress, signal.sizeW(), signal.isDiscrete()));

        m_sizeW = m_nextSignalAddress.offset() - m_startAddress;

        if (m_nextSignalAddress.bit() > 0)
        {
            m_sizeW += 1;
        }

        return signalAddress;
    }


    // ---------------------------------------------------------------------------------
    //
    //	LmMemoryMap class implementation
    //
    // ---------------------------------------------------------------------------------

    LmMemoryMap::LmMemoryMap(OutputLog *log) :
        m_log(log)
    {
        assert(m_log != nullptr);
    }


    bool LmMemoryMap::init(	const MemoryArea& moduleData,
                            const MemoryArea& optoInterfaceData,
                            const MemoryArea& appLogicBitData,
                            const MemoryArea& tuningData,
                            const MemoryArea& appLogicWordData,
                            const MemoryArea& lmDiagData,
                            const MemoryArea& lmIntOutData)
    {

        // init modules memory mapping
        //
        m_modules.memory.setStartAddress(moduleData.startAddress());
        m_modules.memory.setSizeW(moduleData.sizeW() * MODULES_COUNT);
        m_modules.memory.lock();

        for(int i = 0; i < MODULES_COUNT; i++)
        {
            m_modules.module[i].setStartAddress(m_modules.memory.startAddress() + i * moduleData.sizeW());
            m_modules.module[i].setSizeW(moduleData.sizeW());
        }

        // init opto interface memory mapping
        //
        m_optoInterface.memory.setStartAddress(optoInterfaceData.startAddress());
        m_optoInterface.memory.setSizeW(optoInterfaceData.sizeW() * (OPTO_INTERFACE_COUNT + 1));
        m_optoInterface.memory.lock();

        for(int i = 0; i < OPTO_INTERFACE_COUNT; i++)
        {
            if (i == 0)
            {
                m_optoInterface.channel[i].setStartAddress(m_optoInterface.memory.startAddress());
            }
            else
            {
                m_optoInterface.channel[i].setStartAddress(m_optoInterface.channel[i-1].nextAddress());
            }

            m_optoInterface.channel[i].setSizeW(optoInterfaceData.sizeW());
        }

        m_optoInterface.result.setStartAddress(m_optoInterface.channel[OPTO_INTERFACE_COUNT -1].nextAddress());
        m_optoInterface.result.setSizeW(optoInterfaceData.sizeW());

        // init application bit-addressed memory mapping
        //
        m_appBitAdressed.memory = appLogicBitData;
        m_appBitAdressed.memory.lock();

        m_appBitAdressed.bitAccumulator.setStartAddress(appLogicBitData.startAddress());
        m_appBitAdressed.bitAccumulator.setSizeW(1);        // bit accumulator has 1 word (16 bit) size

        m_appBitAdressed.regDiscretSignals.setStartAddress(appLogicBitData.startAddress());
        m_appBitAdressed.nonRegDiscretSignals.setStartAddress(appLogicBitData.startAddress());

        // init tuning interface memory mapping
        //
        m_tuningInterface.memory = tuningData;
        m_tuningInterface.memory.lock();

        // init application word-addressed memory mapping
        //
        m_appWordAdressed.memory = appLogicWordData;
        m_appWordAdressed.memory.lock();

        m_appWordAdressed.lmDiagnostics.setStartAddress(appLogicWordData.startAddress());
        m_appWordAdressed.lmInputs.setStartAddress(appLogicWordData.startAddress());
        m_appWordAdressed.lmOutputs.setStartAddress(appLogicWordData.startAddress());
        m_appWordAdressed.regAnalogSignals.setStartAddress(appLogicWordData.startAddress());
        m_appWordAdressed.regDiscreteSignals.setStartAddress(appLogicWordData.startAddress());
        m_appWordAdressed.nonRegAnalogSignals.setStartAddress(appLogicWordData.startAddress());

        // init LM diagnostics memory mapping
        //
        m_lmDiagnostics.memory = lmDiagData;
        m_lmDiagnostics.memory.lock();

        // init LM in/out controller memory mapping
        //
        m_lmInOuts.memory = lmIntOutData;
        m_lmInOuts.memory.lock();

        return recalculateAddresses();
    }


    bool LmMemoryMap::recalculateAddresses()
    {
        // recalc application bit-addressed memory mapping
        //

        m_appBitAdressed.regDiscretSignals.setStartAddress(m_appBitAdressed.bitAccumulator.nextAddress());

        m_appBitAdressed.nonRegDiscretSignals.setStartAddress(m_appBitAdressed.regDiscretSignals.nextAddress());

        if (m_appBitAdressed.nonRegDiscretSignals.nextAddress() > m_appBitAdressed.memory.nextAddress())
        {
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, tr("Out of bit-addressed memory range!"));

            return false;
        }

        // recalc application word-addressed memory mapping
        //

        // LM diagnostics

        m_appWordAdressed.lmDiagnostics.setStartAddress(m_appWordAdressed.memory.startAddress());
        m_appWordAdressed.lmDiagnostics.setSizeW(m_lmDiagnostics.memory.sizeW());

        // LM input discrete signals

        m_appWordAdressed.lmInputs.setStartAddress(m_appWordAdressed.lmDiagnostics.nextAddress());
        m_appWordAdressed.lmInputs.setSizeW(m_lmInOuts.memory.sizeW());

        // LM output discrete signals

        m_appWordAdressed.lmOutputs.setStartAddress(m_appWordAdressed.lmInputs.nextAddress());
        m_appWordAdressed.lmOutputs.setSizeW(m_lmInOuts.memory.sizeW());

        // modules data

        for(int i = 0; i < MODULES_COUNT; i++)
        {
            if (i == 0)
            {
                m_appWordAdressed.module[0].setStartAddress(m_appWordAdressed.lmOutputs.nextAddress());
            }
            else
            {
                m_appWordAdressed.module[i].setStartAddress(m_appWordAdressed.module[i-1].nextAddress());
            }
        }

        // registered analog signals

        m_appWordAdressed.regAnalogSignals.setStartAddress(m_appWordAdressed.module[MODULES_COUNT - 1].nextAddress());

        // registered discrete signals

        m_appWordAdressed.regDiscreteSignals.setStartAddress(m_appWordAdressed.regAnalogSignals.nextAddress());
        m_appWordAdressed.regDiscreteSignals.setSizeW(m_appBitAdressed.regDiscretSignals.sizeW());

        // non registered analog signals

        m_appWordAdressed.nonRegAnalogSignals.setStartAddress(m_appWordAdressed.regDiscreteSignals.nextAddress());

        if (m_appWordAdressed.nonRegAnalogSignals.nextAddress() > m_appWordAdressed.memory.nextAddress())
        {
            LOG_ERROR_OBSOLETE(m_log, Builder::IssueType::NotDefined, tr("Out of word-addressed memory range!"));

            return false;
        }

        return true;
    }


    int LmMemoryMap::getModuleDataOffset(int place)
    {
        assert(place >= FIRST_MODULE_PLACE && place <= LAST_MODULE_PLACE);

        return m_modules.module[place - 1].startAddress();;
    }


    int LmMemoryMap::getModuleRegDataOffset(int place)
    {
        assert(place >= FIRST_MODULE_PLACE && place <= LAST_MODULE_PLACE);

        return m_appWordAdressed.module[place - 1].startAddress();;
    }


    int LmMemoryMap::addModule(int place, int moduleAppRegDataSize)
    {
        assert(place >= FIRST_MODULE_PLACE && place <= LAST_MODULE_PLACE);

        m_appWordAdressed.module[place - 1].setSizeW(moduleAppRegDataSize);

        recalculateAddresses();

        return getModuleRegDataOffset(place);
    }


    void LmMemoryMap::getFile(QStringList& memFile)
    {
        memFile.append(QString(" LM's memory map"));
        memFile.append("");

        //

        addSection(memFile, m_modules.memory, "I/O modules controller memory");

        for(int i = 0; i < MODULES_COUNT; i++)
        {
            addRecord(memFile, m_modules.module[i], QString().sprintf("I/O module %02d", i + 1));
        }

        memFile.append("");

        addSection(memFile, m_optoInterface.memory, "Opto interfaces memory");

        //

        for(int i = 0; i < OPTO_INTERFACE_COUNT; i++)
        {
            addRecord(memFile, m_optoInterface.channel[i], QString().sprintf("opto interface %02d", i + 1));
        }

        memFile.append("");

        addRecord(memFile, m_optoInterface.result, "opto interfaces data processing result");

        memFile.append("");

        //

        addSection(memFile, m_appBitAdressed.memory, "Application logic bit-addressed memory");

        addRecord(memFile, m_appBitAdressed.bitAccumulator, "bit accumulator");

        addRecord(memFile, m_appBitAdressed.regDiscretSignals, "registrated discrete signals");

        memFile.append("");

        addSignals(memFile, m_appBitAdressed.regDiscretSignals);

        addRecord(memFile, m_appBitAdressed.nonRegDiscretSignals, "non-registrated discrete signals");

        memFile.append("");

        addSignals(memFile, m_appBitAdressed.nonRegDiscretSignals);

        //

        addSection(memFile, m_tuningInterface.memory, "Tuning interface memory");

        memFile.append("");

        //

        addSection(memFile, m_appWordAdressed.memory, "Application logic word-addressed memory");

        addRecord(memFile, m_appWordAdressed.lmDiagnostics, "LM's diagnostics data");
        addRecord(memFile, m_appWordAdressed.lmInputs, "LM's inputs state");
        addRecord(memFile, m_appWordAdressed.lmOutputs, "LM's outputs state");

        memFile.append("");

        for(int i = 0; i < MODULES_COUNT; i++)
        {
            addRecord(memFile, m_appWordAdressed.module[i], QString().sprintf("I/O module %02d data", i + 1));
        }

        memFile.append("");

        addRecord(memFile, m_appWordAdressed.regAnalogSignals, "registrated analogs signals");

        memFile.append("");

        addSignals(memFile, m_appWordAdressed.regAnalogSignals);

        addRecord(memFile, m_appWordAdressed.regDiscreteSignals, "registrated discrete signals (from bit-addressed memory)");

        memFile.append("");

        addSignals(memFile, m_appWordAdressed.regDiscreteSignals);

        addRecord(memFile, m_appWordAdressed.nonRegAnalogSignals, "non-registrated analogs signals");

        memFile.append("");

        addSignals(memFile, m_appWordAdressed.nonRegAnalogSignals);

        //

        addSection(memFile, m_lmDiagnostics.memory, "LM's diagnostics memory");
        addSection(memFile, m_lmInOuts.memory, "LM's inputs/outputs memory");
    }


    void LmMemoryMap::addSection(QStringList& memFile, MemoryArea& memArea, const QString& title)
    {
        memFile.append(QString().rightJustified(80, '-'));
        memFile.append(QString(" Address    Size      Description"));
        memFile.append(QString().rightJustified(80, '-'));

        QString str;
        str.sprintf(" %05d      %05d     %s", memArea.startAddress(), memArea.sizeW(), C_STR(title));

        memFile.append(str);
        memFile.append(QString().rightJustified(80, '-'));
        memFile.append("");
    }


    void LmMemoryMap::addRecord(QStringList& memFile, MemoryArea& memArea, const QString& title)
    {
        QString str;

        str.sprintf(" %05d      %05d     %s", memArea.startAddress(), memArea.sizeW(), C_STR(title));

        memFile.append(str);
    }


    void LmMemoryMap::addSignals(QStringList& memFile, MemoryArea& memArea)
    {
        if (memArea.hasSignals() == false)
        {
            return;
        }

        QVector<MemoryArea::SignalAddress16>& signalsArray = memArea.getSignals();

        for(MemoryArea::SignalAddress16& signal : signalsArray)
        {
            QString str;

            if (signal.isDiscrete())
            {
                str.sprintf(" %05d.%02d   00000.01  %s",
                            signal.address().offset(), signal.address().bit(),
                            C_STR(signal.signalStrID()));

            }
            else
            {
                str.sprintf(" %05d      %05d     %s",
                            signal.address().offset(),
                            signal.sizeW(),
                            C_STR(signal.signalStrID()));
            }

            memFile.append(str);
        }

        memFile.append("");
    }


    Address16 LmMemoryMap::addRegDiscreteSignal(const Signal& signal)
    {
        assert(signal.isInternal() && signal.isRegistered() && signal.isDiscrete());

        return m_appBitAdressed.regDiscretSignals.appendSignal(signal);
    }

    Address16 LmMemoryMap::addRegDiscreteSignalToRegBuffer(const Signal& signal)
    {
        assert(signal.isInternal() && signal.isRegistered() && signal.isDiscrete());

        return m_appWordAdressed.regDiscreteSignals.appendSignal(signal);
    }

    Address16 LmMemoryMap::addNonRegDiscreteSignal(const Signal& signal)
    {
        assert(signal.isInternal() && !signal.isRegistered() && signal.isDiscrete());

        return m_appBitAdressed.nonRegDiscretSignals.appendSignal(signal);
    }


    Address16 LmMemoryMap::addRegAnalogSignal(const Signal& signal)
    {
        assert(signal.isInternal() && signal.isRegistered() && signal.isAnalog());

        return m_appWordAdressed.regAnalogSignals.appendSignal(signal);
    }


    Address16 LmMemoryMap::addNonRegAnalogSignal(const Signal& signal)
    {
        assert(signal.isInternal() && !signal.isRegistered() && signal.isAnalog());

        return m_appWordAdressed.nonRegAnalogSignals.appendSignal(signal);
    }

    double LmMemoryMap::bitAddressedMemoryUsed()
    {
        return double((m_appBitAdressed.regDiscretSignals.sizeW() + m_appBitAdressed.nonRegDiscretSignals.sizeW()) * 100) /
                double(m_appBitAdressed.memory.sizeW());
    }

    double LmMemoryMap::wordAddressedMemoryUsed()
    {
        return double((m_appWordAdressed.nonRegAnalogSignals.nextAddress() - m_appWordAdressed.memory.startAddress()) * 100) /
                double(m_appWordAdressed.memory.sizeW());
    }


}

