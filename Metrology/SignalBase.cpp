#include "SignalBase.h"

// -------------------------------------------------------------------------------------------------------------------

SignalBase theSignalBase;

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
    //m_param.setInputSensorID(1);
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

QString MeasureSignal::adcRangeString()
{
    QString range;

    range.sprintf("%04X .. %04X", m_param.lowADC(), m_param.highADC());

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
            m_signal[i] = nullptr;
        }

         m_caseNo = -1;
         m_subblock = -1;
         m_block = -1;
         m_entry = -1;

    m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureMultiSignal::isEmpty()
{
    bool empty = true;

    m_mutex.lock();

        for(int i = 0; i < MEASURE_MULTI_SIGNAL_COUNT; i++)
        {
            if (m_signal[i] != nullptr)
            {
                empty = false;

                break;
            }
        }

     m_mutex.unlock();

    return empty;
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal* MeasureMultiSignal::signal(const int &index) const
{
    if (index < 0 || index >= MEASURE_MULTI_SIGNAL_COUNT)
    {
        assert(false);
        return nullptr;
    }

    return m_signal[index];
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureMultiSignal::setSignal(const int& index, MeasureSignal *pSignal)
{
    if (index < 0 || index >= MEASURE_MULTI_SIGNAL_COUNT)
    {
        return;
    }

    m_mutex.lock();

        m_signal[index] = pSignal;

        if (pSignal != nullptr)
        {
            if (pSignal->param().appSignalID().isEmpty() == false && pSignal->param().hash() != 0)
            {
                setCaseNo( pSignal->position().caseNo() );
                setSubblock( pSignal->position().subblock() );
                setBlock( pSignal->position().block() );
                setEntry( pSignal->position().entry() );
            }
        }

    m_mutex.unlock();

    return;
}

// -------------------------------------------------------------------------------------------------------------------

MeasureMultiSignal& MeasureMultiSignal::operator=(const MeasureMultiSignal& from)
{
    m_mutex.lock();

        for(int i = 0; i < MEASURE_MULTI_SIGNAL_COUNT; i++)
        {
            m_signal[i] = from.m_signal[i];
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
        assert(hash != 0);
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

MeasureSignal SignalBase::operator [](int index)
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

Signal* SignalBase::signalParam(const Hash& hash)
{
    if (hashIsValid(hash) == false)
    {
        assert(false);
        return nullptr;
    }

    int index = -1;

    m_signalMutex.lock();

        index = m_signalHashMap[hash];

    m_signalMutex.unlock();

    return signalParam(index);
}

// -------------------------------------------------------------------------------------------------------------------

Signal* SignalBase::signalParam(const int& index)
{
    Signal* param = nullptr;

    m_signalMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            param = &m_signalList[index].param();
        }

    m_signalMutex.unlock();

    return param;
}

// -------------------------------------------------------------------------------------------------------------------

AppSignalState* SignalBase::signalState(const Hash& hash)
{
    if (hashIsValid(hash) == false)
    {
        assert(false);
        return nullptr;
    }

    int index = -1;

    m_signalMutex.lock();

        index = m_signalHashMap[hash];

    m_signalMutex.unlock();

    return signalState(index);
}

// -------------------------------------------------------------------------------------------------------------------

AppSignalState* SignalBase::signalState(const int& index)
{
    AppSignalState* state = nullptr;

    m_signalMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            state = &m_signalList[index].state();
        }

    m_signalMutex.unlock();

    return state;
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::setSignalState(const Hash& hash, const AppSignalState &state)
{
    if (hashIsValid(hash) == false)
    {
        assert(false);
        return false;
    }

    int index = -1;

    m_signalMutex.lock();

        index = m_signalHashMap[hash];

    m_signalMutex.unlock();

    return setSignalState(index, state);
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::setSignalState(const int& index, const AppSignalState &state)
{
    m_signalMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            m_signalList[index].setState(state);
        }

    m_signalMutex.unlock();

    return true;
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

//int SignalBase::caseIndexByCaseNo(const int& caseNo)
//{
//    int caseIndex = -1;

//    m_caseMutex.lock();

//        int count = m_caseNoList.size();

//        for (int i = 0; i < count; i ++)
//        {
//             if (m_caseNoList[i] == caseNo)
//             {
//                 caseIndex = i;

//                 break;
//             }
//        }

//    m_caseMutex.unlock();

//    return caseIndex;
//}

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
                        multiSignal.setSignal(MEASURE_MULTI_SIGNAL_0, &signal);
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
                                        ms.setSignal( caseIndex, &signal );
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
                                multiSignal.setSignal( caseIndex, &signal );
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

bool SignalBase::signalForMeasure(const int& index, MeasureMultiSignal& signal)
{
    bool found = false;

    m_signalForMesaureMutex.lock();

        if (index >= 0 && index < m_signalForMesaureList.size())
        {
            signal = m_signalForMesaureList[index];

            found = true;
        }

    m_signalForMesaureMutex.unlock();

    return found;
}

// -------------------------------------------------------------------------------------------------------------------

//MeasureMultiSignal SignalBase::activeSignal()
//{
//    return MeasureMultiSignal();
//}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setActiveSignal(const MeasureMultiSignal& multiSignal)
{
    m_activeSignalMutex.lock();

        m_activeSignal = multiSignal;

        m_stateMutex.lock();

            m_requestStateList.clear();

            for(int i = 0; i < MEASURE_MULTI_SIGNAL_COUNT; i++)
            {
                MeasureSignal* pSignal = m_activeSignal.signal(i);
                if (pSignal != nullptr)
                {
                    if (pSignal->param().appSignalID().isEmpty() == false && pSignal->param().hash() != 0)
                    {
                       m_requestStateList.append(pSignal->param().hash());
                    }
                }
            }

        m_stateMutex.unlock();

    m_activeSignalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------
