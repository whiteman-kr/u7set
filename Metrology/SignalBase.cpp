#include "SignalBase.h"

#include "Measure.h"
#include "CalibratorBase.h"

// -------------------------------------------------------------------------------------------------------------------

SignalBase theSignalBase;

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

#include "ctype.h"

void DevicePosition::setFromID(const QString& equipmentID)
{
    if (equipmentID.isEmpty() == true)
    {
        assert(equipmentID.isEmpty() != true);
        return;
    }

    m_equipmentID = equipmentID;

    // parse position from equipmentID
    //

    QString value;
    int begPos, endPos;

    // CaseIndex
    //

    value = equipmentID;

    begPos = value.indexOf("_");
    if (begPos == -1)
    {
        return;
    }

    endPos = begPos + 1;

    while( isalpha( value[ endPos ].toAscii() ) == 0)
    {
        endPos++;
    }

    value.remove(0, begPos + 1);

    value.remove(endPos - begPos - 1, value.count());

    m_caseNo = value.toInt() - 1;

    // CaseType
    //

    value = equipmentID;

    begPos = value.indexOf("_");
    if (begPos == -1)
    {
        return;
    }

    endPos = begPos + 1;

    while( isalpha( value[ endPos ].toAscii() ) == 0)
    {
        endPos++;
    }

    value.remove(0, begPos + (endPos - begPos) );

    endPos = value.indexOf("_", 0);
    if (endPos == -1)
    {
        return;
    }

    value.remove(endPos, value.count());

    m_caseCaption = value;

    // Shassis
    //

    value = equipmentID;

    begPos = value.indexOf("_", begPos + 1);
    if (begPos == -1)
    {
        return;
    }

    value.remove(0, begPos + 3);

    endPos = value.indexOf("_", 0);
    if (endPos == -1)
    {
        return;
    }

    value.remove(endPos, value.count());

    m_subblock = value.toInt() - 1;

    // Module
    //

    value = equipmentID;

    begPos = value.indexOf("_", begPos + 1);
    if (begPos == -1)
    {
        return;
    }

    value.remove(0, begPos + 3);

    endPos = value.indexOf("_", 0);
    if (endPos == -1)
    {
        return;
    }

    value.remove(endPos, value.count());

    m_block = value.toInt() - 1;

    // Input
    //

    value = equipmentID;

    begPos = value.indexOf("_", begPos + 1);
    if (begPos == -1)
    {
        return;
    }

    begPos = value.indexOf("_", begPos + 1);
    if (begPos == -1)
    {
        return;
    }

    value.remove(0, begPos + 3);

    value.remove(2, value.count());

    m_entry = value.toInt() - 1;
}

// -------------------------------------------------------------------------------------------------------------------

QString DevicePosition::caseString() const
{
    QString caseNo = m_caseNo == -1 ? "" : QString::number(m_caseNo + 1);

    QString result;

    if (caseNo.isEmpty() == false && m_caseCaption.isEmpty() == false)
    {
        result = caseNo + " - " + m_caseCaption;
    }
    else
    {
        result = caseNo + m_caseCaption;
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString DevicePosition::channelString() const
{
    return m_channel == -1 ? QT_TRANSLATE_NOOP("Measure", "No") : QString::number(m_channel + 1);
}

// -------------------------------------------------------------------------------------------------------------------

QString DevicePosition::subblockString() const
{
    return m_subblock == -1 ? QT_TRANSLATE_NOOP("Measure", "No") : QString::number(m_subblock + 1);
}

// -------------------------------------------------------------------------------------------------------------------

QString DevicePosition::blockString() const
{
    return m_block == -1 ? QT_TRANSLATE_NOOP("Measure", "No") : QString::number(m_block + 1);
}

// -------------------------------------------------------------------------------------------------------------------

QString DevicePosition::entryString() const
{
    return m_entry == -1 ? QT_TRANSLATE_NOOP("Measure", "No") : QString::number(m_entry + 1);
}

// -------------------------------------------------------------------------------------------------------------------

DevicePosition& DevicePosition::operator=(const DevicePosition& from)
{
    m_equipmentID = from.m_equipmentID;

    m_caseNo = from.m_caseNo;

    m_caseType = from.m_caseType;
    m_caseCaption = from.m_caseCaption;

    m_channel = from.m_channel;
    m_subblock = from.m_subblock;
    m_block = from.m_block;
    m_entry = from.m_entry;

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureSignal::MeasureSignal()
{
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal::MeasureSignal(Signal param)
{
    m_param = param;

    // temporary solution
    // because u7 can not set electric range
    //
    if (param.equipmentID().isEmpty() == false && param.hash() != 0)
    {
        if (param.isAnalog() == true && param.isInput() == true)
        {
            m_position.setFromID(param.equipmentID());
        }
    }

    m_param.setInputLowLimit(4);
    m_param.setInputHighLimit(20);
    m_param.setInputUnitID(15);
    //
    // temporary solution

}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal::~MeasureSignal()
{
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal& MeasureSignal::operator=(const MeasureSignal& from)
{
    m_param = from.m_param;
    m_state = from.m_state;
    m_position = from.m_position;

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureSignal::stateString()
{
    QString state, formatStr;

    formatStr.sprintf( ("%%.%df"), m_param.decimalPlaces() );

    state.sprintf( formatStr.toAscii() + " ", m_state.value );

    state.append( theSignalBase.unit( m_param.unitID() ) );

    if (m_state.flags.underflow != 0)
    {
        state.append(" - Underflow");
    }

    if (m_state.flags.overflow != 0)
    {
        state.append(" - Overflow");
    }

    if (m_state.flags.valid == 0)
    {
        state = "No valid";
    }

    return state;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureSignal::adcRange(const bool& showInHex)
{
    QString range;

    if (showInHex == true)
    {
        range.sprintf("0x%04X .. 0x%04X", m_param.lowADC(), m_param.highADC());
    }
    else
    {
        range.sprintf("%d .. %d", m_param.lowADC(), m_param.highADC());
    }

    return range;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureSignal::inputPhysicalRange()
{
    QString range, formatStr;

    formatStr.sprintf( ("%%.%df"), m_param.decimalPlaces() );

    range.sprintf( formatStr.toAscii() + " .. " + formatStr.toAscii() + " ", m_param.lowEngeneeringUnits(), m_param.highEngeneeringUnits());

    range.append( theSignalBase.unit( m_param.unitID() ) );

    return range;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureSignal::inputElectricRange()
{
    QString range;

    range.sprintf("%.3f .. %.3f ", m_param.inputLowLimit(), m_param.inputHighLimit());

    range.append( theSignalBase.unit( m_param.inputUnitID() ) );

    int inputSensorID = m_param.inputSensorID();

    if (inputSensorID >= 1 && inputSensorID < SENSOR_TYPE_COUNT)
    {
        range.append( " (" );
        range.append( SensorTypeStr[ inputSensorID ]  );
        range.append( ")" );
    }

    return range;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureSignal::outputPhysicalRange()
{
    QString range, formatStr;

    formatStr.sprintf( ("%%.%df"), m_param.decimalPlaces() );

    range.sprintf( formatStr.toAscii() + " .. " + formatStr.toAscii() + " ", m_param.outputLowLimit(), m_param.outputHighLimit());

    range.append( theSignalBase.unit( m_param.outputUnitID() ) );

    return range;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureSignal::outputElectricRange()
{
    int mode = m_param.outputMode();

    if (mode < 0 || mode >= OUTPUT_MODE_COUNT)
    {
        assert(false);
        return "";
    }

    return OutputModeStr[ m_param.outputMode() ];
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureSignal::calibratorIndexString(const int& index)
{
    CalibratorManager* pManager = theCalibratorBase.сalibratorForMeasure(index);
    if (pManager == nullptr)
    {
        return "";
    }

    if (pManager->calibratorIsConnected() == false)
    {
        return "not connected";
    }

    return QString("Calibrator %1 (%2)").arg( pManager->index() + 1).arg(pManager->portName());
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureMultiSignal::MeasureMultiSignal()
{
    clear();
}

// -------------------------------------------------------------------------------------------------------------------

MeasureMultiSignal::~MeasureMultiSignal()
{
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureMultiSignal::clear()
{
    m_mutex.lock();

        for(int i = 0; i < MEASURE_MULTI_SIGNAL_COUNT; i++)
        {
            m_signalHash[i] = 0;
        }

         m_caseNo = -1;
         m_subblock = -1;
         m_block = -1;
         m_entry = -1;

    m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureMultiSignal::isEmpty() const
{
    bool empty = true;

    m_mutex.lock();

        for(int i = 0; i < MEASURE_MULTI_SIGNAL_COUNT; i++)
        {
            if (m_signalHash[i] != 0)
            {
                empty = false;

                break;
            }
        }

     m_mutex.unlock();

    return empty;
}

// -------------------------------------------------------------------------------------------------------------------

Hash MeasureMultiSignal::hash(const int& index) const
{
    if (index < 0 || index >= MEASURE_MULTI_SIGNAL_COUNT)
    {
        assert(false);
        return 0;
    }

    Hash hash = 0;

    m_mutex.lock();

        hash = m_signalHash[index];

    m_mutex.unlock();

    return hash;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureMultiSignal::setSignal(const int& index, const MeasureSignal& signal)
{
    if (index < 0 || index >= MEASURE_MULTI_SIGNAL_COUNT)
    {
        return;
    }

    if (signal.param().appSignalID().isEmpty() == true || signal.param().hash() == 0)
    {
        return;
    }

    m_mutex.lock();

        m_signalHash[index] = signal.param().hash();

        setCaseNo( signal.position().caseNo() );
        setSubblock( signal.position().subblock() );
        setBlock( signal.position().block() );
        setEntry( signal.position().entry() );


    m_mutex.unlock();

    return;
}

// -------------------------------------------------------------------------------------------------------------------

MeasureMultiSignal& MeasureMultiSignal::operator=(const MeasureMultiSignal& from)
{
    m_mutex.lock();

        for(int i = 0; i < MEASURE_MULTI_SIGNAL_COUNT; i++)
        {
            m_signalHash[i] = from.m_signalHash[i];
        }

        m_caseNo = from.m_caseNo;
        m_subblock = from.m_subblock;
        m_block = from.m_block;
        m_entry = from.m_entry;

    m_mutex.unlock();

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalBase::SignalBase(QObject *parent) :
    QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

 SignalBase::~SignalBase()
 {
 }

 // -------------------------------------------------------------------------------------------------------------------

void SignalBase::clear()
{
    m_activeSignal.clear();

    m_signalForMesaureMutex.lock();

        m_signalForMesaureList.clear();

    m_signalForMesaureMutex.unlock();

    m_caseMutex.lock();

        m_caseTypeList.clear();

    m_caseMutex.unlock();

    m_stateMutex.lock();

        m_requestStateList.clear();

    m_stateMutex.unlock();

    m_unitMutex.lock();

        m_unitMap.clear();

    m_unitMutex.unlock();

    m_signalMutex.lock();

        m_signalHashMap.clear();

        m_signalList.clear();

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::signalCount() const
{
    int count = 0;

    m_signalMutex.lock();

        count = m_signalList.size();

    m_signalMutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::hashIsValid(const Hash& hash)
{
    if (hash == 0)
    {
        assert(false);
        return false;
    }

    bool valid = false;

    m_signalMutex.lock();

        if (m_signalHashMap.contains(hash) == true)
        {
            valid = true;
        }

    m_signalMutex.unlock();

    return valid;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::appendSignal(const Signal& param)
{
    if (param.appSignalID().isEmpty() == true || param.hash() == 0)
    {
        assert(false);
        return -1;
    }

    int index = -1;

    m_signalMutex.lock();

        if (m_signalHashMap.contains(param.hash()) == false)
        {
            MeasureSignal signal(param);

            m_signalList.append(signal);
            index = m_signalList.size() - 1;

            m_signalHashMap.insert(param.hash(), index);
        }

     m_signalMutex.unlock();

     return index;
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal SignalBase::signal(const Hash& hash)
{
    if (hashIsValid(hash) == false)
    {
        return MeasureSignal();
    }

    MeasureSignal signal;

    m_signalMutex.lock();

        int index = m_signalHashMap[hash];

        if (index >= 0 && index < m_signalList.size())
        {
            signal = m_signalList[index];
        }

    m_signalMutex.unlock();

    return signal;
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal SignalBase::signal(const int& index)
{
    MeasureSignal signal;

    m_signalMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            signal = m_signalList[index];
        }

    m_signalMutex.unlock();

    return signal;
}

// -------------------------------------------------------------------------------------------------------------------

Signal SignalBase::signalParam(const Hash& hash)
{
    if (hashIsValid(hash) == false)
    {
        assert(false);
        return Signal();
    }

    Signal param;

    m_signalMutex.lock();

        int index = m_signalHashMap[hash];

        if (index >= 0 && index < m_signalList.size())
        {
            param = m_signalList[index].param();
        }

    m_signalMutex.unlock();

    return param;
}

// -------------------------------------------------------------------------------------------------------------------

Signal SignalBase::signalParam(const int& index)
{
    Signal param;

    m_signalMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            param = m_signalList[index].param();
        }

    m_signalMutex.unlock();

    return param;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignalParam(const Hash& hash, const Signal& param)
{
    if (hashIsValid(hash) == false)
    {
        assert(false);
        return;
    }

    int index = -1;

    m_signalMutex.lock();

        index = m_signalHashMap[hash];

        if (index >= 0 && index < m_signalList.size())
        {
            m_signalList[index].setParam(param);
        }

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignalParam(const int& index, const Signal& param)
{
    m_signalMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            m_signalList[index].setParam(param);
        }

    m_signalMutex.unlock();
}


// -------------------------------------------------------------------------------------------------------------------

AppSignalState SignalBase::signalState(const Hash& hash)
{
    if (hashIsValid(hash) == false)
    {
        assert(false);
        return AppSignalState();
    }

    AppSignalState state;

    m_signalMutex.lock();

        int index = m_signalHashMap[hash];

        if (index >= 0 && index < m_signalList.size())
        {
            state = m_signalList[index].state();
        }

    m_signalMutex.unlock();

    return state;
}

// -------------------------------------------------------------------------------------------------------------------

AppSignalState SignalBase::signalState(const int& index)
{
    AppSignalState state;

    m_signalMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            state = m_signalList[index].state();
        }

    m_signalMutex.unlock();

    return state;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignalState(const Hash& hash, const AppSignalState &state)
{
    if (hashIsValid(hash) == false)
    {
        assert(false);
    }

    int index = -1;

    m_signalMutex.lock();

        index = m_signalHashMap[hash];

        if (index >= 0 && index < m_signalList.size())
        {
            m_signalList[index].setState(state);
        }

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignalState(const int& index, const AppSignalState &state)
{
    m_signalMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            m_signalList[index].setState(state);
        }

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::appendUnit(const int& unitID, const QString& unit)
{
    m_unitMutex.lock();

        if (m_unitMap.contains(unitID) == false)
        {
            m_unitMap.insert(unitID, unit);
        }

    m_unitMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::unitCount() const
{
    int count = 0;

    m_unitMutex.lock();

        count = m_unitMap.size();

    m_unitMutex.unlock();

    return count;
}


// -------------------------------------------------------------------------------------------------------------------

QString SignalBase::unit(const int& unitID)
{
    QString strUnit;

    m_unitMutex.lock();

        if (m_unitMap.contains(unitID) == true)
        {
            strUnit = m_unitMap[unitID];
        }

    m_unitMutex.unlock();

    return strUnit;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::hashForRequestStateCount() const
{
    int count = 0;

    m_stateMutex.lock();

        count = m_requestStateList.size();

    m_stateMutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

Hash SignalBase::hashForRequestState(int index)
{
    Hash hash;

    m_stateMutex.lock();

        if (index >= 0 && index < m_requestStateList.size())
        {
            hash = m_requestStateList[index];
        }

    m_stateMutex.unlock();

    return hash;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::createCaseTypeList()
{
    int caseType = 0;

    QMap<QString, int> caseTypeMap;

    m_caseMutex.lock();

        m_caseTypeList.clear();

        int count = m_signalList.size();

        for(int i = 0; i < count; i ++)
        {
            MeasureSignal& signal = m_signalList[i];

            if (signal.param().appSignalID().isEmpty() == true || signal.param().hash() == 0)
            {
                continue;
            }

            if (signal.param().isAnalog() == false || signal.param().isInput() == false )
            {
                continue;
            }

            if (signal.position().subblock() == -1 || signal.position().block() == -1 || signal.position().entry() == -1)
            {
                continue;
            }

            if (signal.position().caseCaption().isEmpty() == true)
            {
                continue;
            }

            if (caseTypeMap.contains(signal.position().caseCaption()) == true)
            {
                signal.setCaseType( caseTypeMap[signal.position().caseCaption()] );

                continue;
            }

            caseTypeMap.insert(signal.position().caseCaption(), caseType);

            m_caseTypeList.append(signal.position().caseCaption());

            signal.setCaseType(caseType);

            caseType++;
        }

    m_caseMutex.unlock();

    return caseType;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::caseTypeCount() const
{
    int count = 0;

    m_caseMutex.lock();

        count = m_caseTypeList.size();

    m_caseMutex.unlock();

    return count;
}


// -------------------------------------------------------------------------------------------------------------------

QString SignalBase::caseTypeCaption(const int& type)
{
    QString caption;

    m_caseMutex.lock();

        if (type >= 0 && type < m_caseTypeList.size())
        {
            caption = m_caseTypeList[type];
        }

    m_caseMutex.unlock();

    return caption;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::caseNoCount() const
{
    int count = 0;

    m_caseMutex.lock();

        count = m_caseNoList.size();

    m_caseMutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::caseNoByCaseIndex(const int& caseIndex)
{
    int caseNo = -1;

    m_caseMutex.lock();

        if (caseIndex >= 0 && caseIndex < m_caseNoList.size())
        {
            caseNo = m_caseNoList[caseIndex];
        }

    m_caseMutex.unlock();

    return caseNo;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::createSignalListForMeasure(const int& caseType, const int& measureKind)
{
    if (caseType < 0 || caseType >= caseTypeCount())
    {
        assert(false);
        return 0;
    }

    if (measureKind < 0 || measureKind >= MEASURE_KIND_COUNT)
    {
        assert(false);
        return 0;
    }

    int signalCount = 0;

    int caseNoIndex = 0;
    QMap<int, int>  caseNoMap;

    // find all caseNo for selected caseType and create map of caseNo
    //
    m_caseMutex.lock();

        m_caseNoList.clear();

        signalCount = m_signalList.size();

        for(int i = 0; i < signalCount; i ++)
        {
            MeasureSignal& signal = m_signalList[i];

            if (signal.param().appSignalID().isEmpty() == true || signal.param().hash() == 0)
            {
                continue;
            }

            if (signal.param().isAnalog() == false || signal.param().isInput() == false )
            {
                continue;
            }

            if (signal.position().caseType() != caseType)
            {
                continue;
            }

            if (caseNoMap.contains(signal.position().caseNo()) == true)
            {
                continue;
            }

            caseNoMap.insert(signal.position().caseNo(), caseNoIndex);

            m_caseNoList.append(signal.position().caseNo());

            caseNoIndex ++;
        }

    m_caseMutex.unlock();

    int                 signalIndex = 0;
    MeasureMultiSignal  multiSignal;
    QMap<Hash, int>  signalForMesaureMap;

    // find all signals for selected caseType
    //
    m_signalForMesaureMutex.lock();

        multiSignal.clear();
        m_signalForMesaureList.clear();

        signalCount = m_signalList.size();

        for(int i = 0; i < signalCount; i ++)
        {
            MeasureSignal& signal = m_signalList[i];

            if (signal.param().appSignalID().isEmpty() == true || signal.param().hash() == 0)
            {
                continue;
            }

            if (signal.param().isAnalog() == false || signal.param().isInput() == false )
            {
                continue;
            }

            if (signal.position().subblock() == -1 || signal.position().block() == -1 || signal.position().entry() == -1)
            {
                continue;
            }

            if (signal.position().caseType()!= caseType)
            {
                continue;
            }

            switch(measureKind)
            {

                case MEASURE_KIND_ONE:
                    {
                        multiSignal.setSignal(MEASURE_MULTI_SIGNAL_0, signal);
                    }
                    break;

                case MEASURE_KIND_MULTI:
                    {
                        QString id;
                        id.sprintf("%d - %d - %d - %d - %d",
                                    signal.position().caseType(),
                                    signal.position().channel() + 1,
                                    signal.position().subblock() + 1,
                                    signal.position().block() + 1,
                                    signal.position().entry() + 1);

                        Hash hashid = calcHash(id);

                        if (signalForMesaureMap.contains(hashid) == true)
                        {
                            int index = signalForMesaureMap[hashid];

                            if (index >= 0 && index < m_signalForMesaureList.size())
                            {
                                MeasureMultiSignal& ms = m_signalForMesaureList[index];

                                if (caseNoMap.contains(signal.position().caseNo()) == true)  // find index of case by caseNo
                                {
                                    int caseIndex = caseNoMap[signal.position().caseNo()];

                                    if ( caseIndex >= 0 && caseIndex < MEASURE_MULTI_SIGNAL_COUNT )
                                    {
                                        ms.setSignal( caseIndex, signal );
                                    }
                                }
                            }

                            continue;
                        }

                        signalForMesaureMap.insert(hashid, signalIndex);

                        if (caseNoMap.contains(signal.position().caseNo()) == true)  // find index of case by caseNo
                        {
                            int caseIndex = caseNoMap[signal.position().caseNo()];

                            if ( caseIndex >= 0 && caseIndex < MEASURE_MULTI_SIGNAL_COUNT )
                            {
                                multiSignal.setSignal( caseIndex, signal );
                            }
                        }
                    }
                    break;

                default:
                    assert(false);
            }

            if(multiSignal.isEmpty() == true)
            {
                assert(false);
                continue;
            }

            m_signalForMesaureList.append(multiSignal);

            signalIndex++;

        }

    m_signalForMesaureMutex.unlock();

    return signalIndex;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::signalForMeasureCount() const
{
    int count;

    m_signalForMesaureMutex.lock();

        count = m_signalForMesaureList.size();

    m_signalForMesaureMutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::signalForMeasure(const int& index, MeasureMultiSignal& multiSignal)
{
    bool found = false;

    m_signalForMesaureMutex.lock();

        if (index >= 0 && index < m_signalForMesaureList.size())
        {
            multiSignal = m_signalForMesaureList[index];

            found = true;
        }

    m_signalForMesaureMutex.unlock();

    return found;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setActiveSignal(const MeasureMultiSignal& multiSignal)
{
    m_activeSignalMutex.lock();

        m_activeSignal = multiSignal;

        m_stateMutex.lock();

            m_requestStateList.clear();

            for(int i = 0; i < MEASURE_MULTI_SIGNAL_COUNT; i++)
            {
                Hash hash = m_activeSignal.hash(i);
                if (hash != 0)
                {
                    m_requestStateList.append(hash);
                }
            }

        m_stateMutex.unlock();

    m_activeSignalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------
