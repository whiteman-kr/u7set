#include "SignalBase.h"

#include "Measure.h"
#include "CalibratorBase.h"
#include "MeasurementBase.h"
#include "ObjectVector.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DevicePosition::DevicePosition()
{

}

// -------------------------------------------------------------------------------------------------------------------

DevicePosition::DevicePosition(const QString& equipmentID) :
    m_equipmentID (equipmentID)
{
    if (equipmentID.isEmpty() == true)
    {
        return;
    }

    setFromID(equipmentID);
}

// -------------------------------------------------------------------------------------------------------------------

DevicePosition::~DevicePosition()
{

}

// -------------------------------------------------------------------------------------------------------------------

void DevicePosition::clear()
{
    m_equipmentID = QString();

    m_caseNo = -1;

    m_caseType = -1;
    m_caseCaption = QString();

    m_channel = -1;
    m_subblock = -1;
    m_block = -1;
    m_entry = -1;

    m_position.handle = 0;
}

// -------------------------------------------------------------------------------------------------------------------

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

QString DevicePosition::caseStr() const
{
    QString caseNo = m_caseNo == -1 ? QString() : QString::number(m_caseNo + 1);

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

QString DevicePosition::subblockStr() const
{
    return m_subblock == -1 ? QT_TRANSLATE_NOOP("Measure", "No") : QString::number(m_subblock + 1);
}

// -------------------------------------------------------------------------------------------------------------------

QString DevicePosition::blockStr() const
{
    return m_block == -1 ? QT_TRANSLATE_NOOP("Measure", "No") : QString::number(m_block + 1);
}

// -------------------------------------------------------------------------------------------------------------------

QString DevicePosition::entryStr() const
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

StatisticItem::StatisticItem()
{

}

// -------------------------------------------------------------------------------------------------------------------

StatisticItem::StatisticItem(const Hash& signalHash):
    m_signalHash (signalHash)
{
}


// -------------------------------------------------------------------------------------------------------------------

StatisticItem::~StatisticItem()
{

}

// -------------------------------------------------------------------------------------------------------------------

QString StatisticItem::measureCountStr() const
{
    if (m_measureCount == 0)
    {
        return QString();
    }

    return QString::number(m_measureCount);
}

// -------------------------------------------------------------------------------------------------------------------

QString StatisticItem::stateStr() const
{
    if (m_measureCount == 0)
    {
        return QString("no measured");
    }

    if (m_state < 0 || m_state >= STATISTIC_STATE_COUNT)
    {
        return QString();
    }

    return StatisticStateStr[m_state];
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureSignalParam::MeasureSignalParam()
{
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignalParam::MeasureSignalParam(const Signal& param)
{
    setParam(param);
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignalParam::~MeasureSignalParam()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureSignalParam::isValid() const
{
    if (m_appSignalID.isEmpty() == true || m_hash == 0)
    {
        return false;
    }

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureSignalParam::setParam(const Signal& param)
{
    setAppSignalID(param.appSignalID());
    setCustomAppSignalID(param.customAppSignalID());
    setCaption(param.caption());

    setSignalType(param.signalType());
    setInOutType(param.inOutType());

    m_position.setFromID(param.equipmentID());

    setLowADC(param.lowADC());
    setHighADC(param.highADC());

    setInputElectricLowLimit(param.inputLowLimit());
    setInputElectricHighLimit(param.inputHighLimit());
    setInputElectricUnitID(param.inputUnitID());
    setInputElectricSensorType(param.inputSensorType());
    setInputElectricPrecise(3);

    setInputPhysicalLowLimit(param.lowEngeneeringUnits());
    setInputPhysicalHighLimit(param.highEngeneeringUnits());
    setInputPhysicalUnitID(param.unitID());
    setInputPhysicalPrecise(param.decimalPlaces());
    setAdjustment(0);

    setHasOutput(false);

    switch(param.outputMode())
    {
        case E::OutputMode::Plus0_Plus5_V:     setOutputElectricLowLimit(0);    setOutputElectricHighLimit(5);  setOutputElectricUnitID(E::InputUnit::V);   break;
        case E::OutputMode::Plus4_Plus20_mA:   setOutputElectricLowLimit(4);    setOutputElectricHighLimit(20); setOutputElectricUnitID(E::InputUnit::mA);  break;
        case E::OutputMode::Minus10_Plus10_V:  setOutputElectricLowLimit(-10);  setOutputElectricHighLimit(10); setOutputElectricUnitID(E::InputUnit::V);   break;
        case E::OutputMode::Plus0_Plus5_mA:    setOutputElectricLowLimit(0);    setOutputElectricHighLimit(5);  setOutputElectricUnitID(E::InputUnit::mA);  break;
    }
    setOutputElectricSensorType(param.outputSensorType());
    setOutputElectricPrecise(3);

    setOutputPhysicalLowLimit(param.outputLowLimit());
    setOutputPhysicalHighLimit(param.outputHighLimit());
    setOutputPhysicalUnitID(param.outputUnitID());
    setOutputPhysicalPrecise(param.decimalPlaces());

    setAcquire(param.acquire());
    setNormalState(param.normalState());
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureSignalParam::setAppSignalID(const QString& appSignalID)
{
    m_appSignalID = appSignalID;

    if (appSignalID.isEmpty() == true)
    {
        m_hash = 0;
        return;
    }

    m_hash = calcHash(appSignalID);
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureSignalParam::adcRangeStr(const bool showInHex) const
{
    QString range;

    if (showInHex == true)
    {
        range.sprintf("0x%04X .. 0x%04X", m_lowADC, m_highADC);
    }
    else
    {
        range.sprintf("%d .. %d", m_lowADC, m_highADC);
    }

    return range;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureSignalParam::inputElectricRangeStr() const
{
    QString range, formatStr;

    formatStr.sprintf( ("%%.%df"), m_inputElectricPrecise );

    range.sprintf(formatStr.toAscii() + " .. " + formatStr.toAscii() + " ", m_inputElectricLowLimit, m_inputElectricHighLimit);

    if ( m_inputElectricUnitID >= 0 && m_inputElectricUnitID < theUnitBase.unitCount())
    {
        range.append( theUnitBase.unit( m_inputElectricUnitID ) );
    }

    if (m_inputElectricSensorType >= 1 && m_inputElectricSensorType < SENSOR_TYPE_COUNT)
    {
        range.append( " (" );
        range.append( SensorTypeStr[ m_inputElectricSensorType ]  );
        range.append( ")" );
    }

    return range;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureSignalParam::inputPhysicalRangeStr() const
{
    QString range, formatStr;

    formatStr.sprintf( ("%%.%df"), m_inputPhysicalPrecise );

    range.sprintf( formatStr.toAscii() + " .. " + formatStr.toAscii() + " ", m_inputPhysicalLowLimit, m_inputPhysicalHighLimit);

    if ( m_inputPhysicalUnitID >= 0 && m_inputPhysicalUnitID < theUnitBase.unitCount())
    {
        range.append( theUnitBase.unit( m_inputPhysicalUnitID ) );
    }

    return range;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureSignalParam::outputElectricRangeStr() const
{
    QString range, formatStr;

    formatStr.sprintf( ("%%.%df"), m_outputElectricPrecise );

    range.sprintf(formatStr.toAscii() + " .. " + formatStr.toAscii() + " ", m_outputElectricLowLimit, m_outputElectricHighLimit);

    if ( m_outputElectricUnitID >= 0 && m_outputElectricUnitID < theUnitBase.unitCount())
    {
        range.append( theUnitBase.unit( m_outputElectricUnitID ) );
    }

    if (m_outputElectricSensorType >= 1 && m_outputElectricSensorType < SENSOR_TYPE_COUNT)
    {
        range.append( " (" );
        range.append( SensorTypeStr[ m_outputElectricSensorType ]  );
        range.append( ")" );
    }

    return range;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureSignalParam::outputPhysicalRangeStr() const
{
    QString range, formatStr;

    formatStr.sprintf( ("%%.%df"), m_outputPhysicalPrecise );

    range.sprintf( formatStr.toAscii() + " .. " + formatStr.toAscii() + " ", m_outputPhysicalLowLimit, m_outputPhysicalHighLimit);

    if ( m_outputPhysicalUnitID >= 0 && m_outputPhysicalUnitID < theUnitBase.unitCount())
    {
        range.append( theUnitBase.unit( m_outputPhysicalUnitID ) );
    }

    return range;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureSignalParam::calibratorIndexStr(const int index) const
{
    CalibratorManager* pManager = theCalibratorBase.сalibratorForMeasure(index);
    if (pManager == nullptr || pManager->calibratorIsConnected() == false)
    {
        return QString("not connected");
    }

    return QString("Calibrator %1 (%2)").arg( pManager->index() + 1).arg(pManager->portName());
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureSignal::MeasureSignal()
{
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal::MeasureSignal(const Signal& param)
{
    m_param.setParam(param);

    // temporary solution
    // because u7 can not set electric range
    //
    m_param.setInputElectricLowLimit(4);
    m_param.setInputElectricHighLimit(20);
    m_param.setInputElectricUnitID( E::InputUnit::mA );
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

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureMultiSignal::MeasureMultiSignal()
{
    clear();
}

// -------------------------------------------------------------------------------------------------------------------

MeasureMultiSignal::MeasureMultiSignal(const MeasureMultiSignal& from)
{
    *this = from;
}

// -------------------------------------------------------------------------------------------------------------------

MeasureMultiSignal::~MeasureMultiSignal()
{
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureMultiSignal::clear()
{
    m_mutex.lock();

        for(int i = 0; i < MAX_CHANNEL_COUNT; i++)
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

        for(int i = 0; i < MAX_CHANNEL_COUNT; i++)
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

Hash MeasureMultiSignal::hash(const int index) const
{
    if (index < 0 || index >= MAX_CHANNEL_COUNT)
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

void MeasureMultiSignal::setSignal(const int index, const MeasureSignalParam& param)
{
    if (index < 0 || index >= MAX_CHANNEL_COUNT)
    {
        return;
    }

    if (param.isValid() == false)
    {
        return;
    }

    m_mutex.lock();

        m_signalHash[index] = param.hash();

        setCaseNo( param.position().caseNo() );
        setSubblock( param.position().subblock() );
        setBlock( param.position().block() );
        setEntry( param.position().entry() );


    m_mutex.unlock();

    return;
}

// -------------------------------------------------------------------------------------------------------------------

MeasureMultiSignal& MeasureMultiSignal::operator=(const MeasureMultiSignal& from)
{
    m_mutex.lock();

        for(int i = 0; i < MAX_CHANNEL_COUNT; i++)
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

SignalBase theSignalBase;

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

    m_signalMutex.lock();

        m_signalHashMap.clear();

        m_signalList.clear();

    m_signalMutex.unlock();

    theUnitBase.clear();
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

void SignalBase::sortByPosition()
{
    return;

    QTime responseTime;
    responseTime.start();

    m_signalMutex.lock();

        int count = m_signalList.size();

        for( int i = 0; i < count - 1; i++)
        {
            for(int j = i+1; j < count; j++)
            {
                if ( m_signalList[i].param().position().handle() > m_signalList[j].param().position().handle() )
                {
                    MeasureSignal signal	= m_signalList[ i ];
                    m_signalList[ i ]       = m_signalList[ j ];
                    m_signalList[ j ]       = signal;
                }
            }
        }

    m_signalMutex.unlock();

    qDebug() << __FUNCTION__ <<  responseTime.elapsed() << " ms";
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

MeasureSignal SignalBase::signal(const QString& appSignalID)
{
    if (appSignalID.isEmpty() == true)
    {
        assert(false);
        return MeasureSignal();
    }

    return signal( calcHash(appSignalID) );
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal SignalBase::signal(const Hash& hash)
{
    if (hash == 0)
    {
        assert(hash != 0);
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

MeasureSignal SignalBase::signal(const int index)
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

MeasureSignalParam SignalBase::signalParam(const QString& appSignalID)
{
    if (appSignalID.isEmpty() == true)
    {
        assert(false);
        return MeasureSignalParam();
    }

    return signalParam( calcHash(appSignalID) );
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignalParam SignalBase::signalParam(const Hash& hash)
{
    if (hash == 0)
    {
        assert(hash != 0);
        return MeasureSignalParam();
    }

    MeasureSignalParam param;

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

MeasureSignalParam SignalBase::signalParam(const int index)
{
    MeasureSignalParam param;

    m_signalMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            param = m_signalList[index].param();
        }

    m_signalMutex.unlock();

    return param;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignalParam(const Hash& hash, const MeasureSignalParam& param)
{
    if (hash == 0)
    {
        assert(hash != 0);
        return;
    }

    int index = -1;

    m_signalMutex.lock();

        index = m_signalHashMap[hash];

        if (index >= 0 && index < m_signalList.size())
        {
            m_signalList[index].setParam(param);

            emit updatedSignalParam(param.hash());
        }

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignalParam(const int index, const MeasureSignalParam& param)
{
    m_signalMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            m_signalList[index].setParam(param);

            emit updatedSignalParam(param.hash());
        }

    m_signalMutex.unlock();
}


// -------------------------------------------------------------------------------------------------------------------

AppSignalState SignalBase::signalState(const Hash& hash)
{
    if (hash == 0)
    {
        assert(hash != 0);
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

AppSignalState SignalBase::signalState(const int index)
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
    if (hash == 0)
    {
        assert(hash != 0);
        return;
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

void SignalBase::setSignalState(const int index, const AppSignalState &state)
{
    m_signalMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            m_signalList[index].setState(state);
        }

    m_signalMutex.unlock();
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

Hash SignalBase::hashForRequestState(const int index)
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
    int caseTypeCount = 0;

    QMap<QString, int> caseTypeMap;

    m_caseMutex.lock();

        m_caseTypeList.clear();

        int count = m_signalList.size();

        for(int i = 0; i < count; i ++)
        {
            MeasureSignalParam param = m_signalList[i].param();
            if (param.isValid() == false)
            {
                continue;
            }

            if (param.isAnalog() == false || param.isInput() == false )
            {
                continue;
            }

            if (param.position().subblock() == -1 || param.position().block() == -1 || param.position().entry() == -1)
            {
                continue;
            }

            if (param.position().caseCaption().isEmpty() == true)
            {
                continue;
            }

            if (caseTypeMap.contains(param.position().caseCaption()) == true)
            {
                param.setCaseType( caseTypeMap[param.position().caseCaption()] );
                m_signalList[i].setParam(param);

                continue;
            }

            caseTypeMap.insert(param.position().caseCaption(), caseTypeCount);

            m_caseTypeList.append(param.position().caseCaption());


            param.setCaseType(caseTypeCount);
            m_signalList[i].setParam(param);

            caseTypeCount++;
        }

    m_caseMutex.unlock();

    return caseTypeCount;
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

QString SignalBase::caseTypeCaption(const int type)
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

int SignalBase::caseNoByCaseIndex(const int caseIndex)
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

int SignalBase::createSignalListForMeasure(const int caseType, const int measureKind)
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
            MeasureSignalParam param = m_signalList[i].param();
            if (param.isValid() == false)
            {
                continue;
            }

            if (param.isAnalog() == false || param.isInput() == false )
            {
                continue;
            }

            if (param.position().caseType() != caseType)
            {
                continue;
            }

            if (caseNoMap.contains(param.position().caseNo()) == true)
            {
                continue;
            }

            caseNoMap.insert(param.position().caseNo(), caseNoIndex);

            m_caseNoList.append(param.position().caseNo());

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
            MeasureSignalParam param = m_signalList[i].param();
            if (param.isValid() == false)
            {
                continue;
            }

            if (param.isAnalog() == false || param.isInput() == false )
            {
                continue;
            }

            if (param.position().subblock() == -1 || param.position().block() == -1 || param.position().entry() == -1)
            {
                continue;
            }

            if (param.position().caseType()!= caseType)
            {
                continue;
            }

            switch(measureKind)
            {

                case MEASURE_KIND_ONE:
                    {
                        multiSignal.setSignal(CHANNEL_0, param);
                    }
                    break;

                case MEASURE_KIND_MULTI:
                    {
                        QString id;
                        id.sprintf("%d - %d - %d - %d - %d",
                                    param.position().caseType(),
                                    param.position().channel() + 1,
                                    param.position().subblock() + 1,
                                    param.position().block() + 1,
                                    param.position().entry() + 1);

                        Hash hashid = calcHash(id);

                        if (signalForMesaureMap.contains(hashid) == true)
                        {
                            int index = signalForMesaureMap[hashid];

                            if (index >= 0 && index < m_signalForMesaureList.size())
                            {
                                MeasureMultiSignal& ms = m_signalForMesaureList[index];

                                if (caseNoMap.contains(param.position().caseNo()) == true)  // find index of case by caseNo
                                {
                                    int caseIndex = caseNoMap[param.position().caseNo()];

                                    if ( caseIndex >= 0 && caseIndex < MAX_CHANNEL_COUNT )
                                    {
                                        ms.setSignal( caseIndex, param );
                                    }
                                }
                            }

                            continue;
                        }

                        signalForMesaureMap.insert(hashid, signalIndex);

                        if (caseNoMap.contains(param.position().caseNo()) == true)  // find index of case by caseNo
                        {
                            int caseIndex = caseNoMap[param.position().caseNo()];

                            if ( caseIndex >= 0 && caseIndex < MAX_CHANNEL_COUNT )
                            {
                                multiSignal.setSignal( caseIndex, param );
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

MeasureMultiSignal SignalBase::signalForMeasure(const int index)
{
    MeasureMultiSignal signal;

    m_signalForMesaureMutex.lock();

        if (index >= 0 && index < m_signalForMesaureList.size())
        {
            signal = m_signalForMesaureList[index];
        }

    m_signalForMesaureMutex.unlock();

    return signal;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setActiveSignal(const MeasureMultiSignal& multiSignal)
{
    m_activeSignalMutex.lock();

        m_activeSignal = multiSignal;

        m_stateMutex.lock();

            m_requestStateList.clear();

            for(int i = 0; i < MAX_CHANNEL_COUNT; i++)
            {
                Hash hash = m_activeSignal.hash(i);
                if (hash != 0)
                {
                    m_requestStateList.append(hash);
                }
            }

        m_stateMutex.unlock();

    m_activeSignalMutex.unlock();

    emit setActiveSignal();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

UnitBase theUnitBase;

// -------------------------------------------------------------------------------------------------------------------

UnitBase::UnitBase(QObject *parent) :
    QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

UnitBase::~UnitBase()
{
}

 // -------------------------------------------------------------------------------------------------------------------

void UnitBase::clear()
{
    m_unitMutex.lock();

        m_unitMap.clear();

    m_unitMutex.unlock();
}


// -------------------------------------------------------------------------------------------------------------------

int UnitBase::unitCount() const
{
    int count = 0;

    m_unitMutex.lock();

        count = m_unitMap.size();

    m_unitMutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

void UnitBase::appendUnit(const int unitID, const QString& unit)
{
    m_unitMutex.lock();

        if (m_unitMap.contains(unitID) == false)
        {
            m_unitMap.insert(unitID, unit);
        }

    m_unitMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

QString UnitBase::unit(const int unitID)
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
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

OutputSignal::OutputSignal()
{
    clear();
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignal::OutputSignal(const OutputSignal& from)
{
    *this = from;
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignal::~OutputSignal()
{
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignal::clear()
{
    m_type = OUTPUT_SIGNAL_TYPE_NO_USED;

    for(int k = 0; k < OUTPUT_SIGNAL_KIND_COUNT; k++)
    {
        m_param[k].setAppSignalID(QString());
    }
}
// -------------------------------------------------------------------------------------------------------------------

bool OutputSignal::isValid() const
{
    if (m_type < 0 || m_type >= OUTPUT_SIGNAL_TYPE_COUNT)
    {
        return false;
    }

    for(int kind = 0; kind < OUTPUT_SIGNAL_KIND_COUNT; kind++)
    {
        if ( m_appSignalID[kind].isEmpty() == true)
        {
            return false;
        }
    }

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

QString OutputSignal::typeStr() const
{
    if (m_type < 0 || m_type >= OUTPUT_SIGNAL_TYPE_COUNT)
    {
        return QString();
    }

    return OutputSignalType[m_type];
}

// -------------------------------------------------------------------------------------------------------------------

QString OutputSignal::appSignalID(const int kind) const
{
    if (kind < 0 || kind >= OUTPUT_SIGNAL_KIND_COUNT)
    {
        return QString();
    }

    QString appSignalID;

    m_signalMutex.lock();

        appSignalID = m_appSignalID[kind];

    m_signalMutex.unlock();

    return appSignalID;
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignal::setAppSignalID(const int kind, const QString& appSignalID)
{
    if (kind < 0 || kind >= OUTPUT_SIGNAL_KIND_COUNT)
    {
        return;
    }

    m_signalMutex.lock();

        m_appSignalID[kind] = appSignalID;

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignalParam OutputSignal::param(const int kind) const
{
    if (kind < 0 || kind >= OUTPUT_SIGNAL_KIND_COUNT)
    {
        return MeasureSignalParam();
    }

    MeasureSignalParam param;

    m_signalMutex.lock();

        param = m_param[kind];

    m_signalMutex.unlock();

    return param;
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignal::setParam(const int kind, const MeasureSignalParam& param)
{
    if (kind < 0 || kind >= OUTPUT_SIGNAL_KIND_COUNT)
    {
        return;
    }

    if (param.isValid() == false)\
    {
        return;
    }

    m_signalMutex.lock();

        m_appSignalID[kind] = param.appSignalID();

        m_param[kind] = param;

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignal::updateParam()
{
    m_signalMutex.lock();

        for(int kind = 0; kind < OUTPUT_SIGNAL_KIND_COUNT; kind++)
        {
            if (m_appSignalID[kind].isEmpty() == true)
            {
                continue;
            }

            Hash signalHash = calcHash(m_appSignalID[kind]);
            if (signalHash == 0)
            {
                continue;
            }

            m_param[kind] = theSignalBase.signalParam(signalHash);
        }

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignal& OutputSignal::operator=(const OutputSignal& from)
{
    m_signalID = from.m_signalID;

    m_type = from.m_type;

    for(int kind = 0; kind < OUTPUT_SIGNAL_KIND_COUNT; kind++)
    {
        m_appSignalID[kind] = from.m_appSignalID[kind];

        m_param[kind] = from.m_param[kind];
    }

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------

bool OutputSignal::operator==(const OutputSignal& signal)
{
    if (m_signalID != signal.m_signalID)
    {
        return false;
    }

    if (m_type != signal.m_type)
    {
        return false;
    }

    for(int kind = 0; kind < OUTPUT_SIGNAL_KIND_COUNT; kind++)
    {
        if (m_appSignalID[kind] != signal.m_appSignalID[kind] )
        {
            return false;
        }

        if (m_param[kind].hash() != signal.m_param[kind].hash() )
        {
            return false;
        }
    }

    return true;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

OutputSignalBase theOutputSignalBase;

// -------------------------------------------------------------------------------------------------------------------

OutputSignalBase::OutputSignalBase(QObject *parent) :
    QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignalBase::~OutputSignalBase()
{
}

 // -------------------------------------------------------------------------------------------------------------------

void OutputSignalBase::clear()
{
    m_signalMutex.lock();

        m_signalList.clear();

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int OutputSignalBase::signalCount() const
{
    int count = 0;

    m_signalMutex.lock();

        count = m_signalList.size();

    m_signalMutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalBase::sort()
{

}

// -------------------------------------------------------------------------------------------------------------------

int OutputSignalBase::load()
{
    if (thePtrDB == nullptr)
    {
        return 0;
    }

    QTime responseTime;
    responseTime.start();

    SqlTable* table = thePtrDB->openTable(SQL_TABLE_OUTPUT_SIGNAL);
    if (table == nullptr)
    {
        return false;
    }

    m_signalList.resize(table->recordCount());

    int readedRecordCount = 0;

    m_signalMutex.lock();

        readedRecordCount = table->read(m_signalList.data());

    m_signalMutex.unlock();

    table->close();

    qDebug() << "OutputSignalBase::load() - Loaded output signals: " << readedRecordCount << ", Time for load: " << responseTime.elapsed() << " ms";

    return readedRecordCount;
}

// -------------------------------------------------------------------------------------------------------------------

bool OutputSignalBase::save()
{
    if (thePtrDB == nullptr)
    {
        return false;
    }

    SqlTable* table = thePtrDB->openTable(SQL_TABLE_OUTPUT_SIGNAL);
    if (table == nullptr)
    {
        return false;
    }

    if (table->clear() == false)
    {
        table->close();
        return false;
    }

    int writtenRecordCount = 0;

    m_signalMutex.lock();

        writtenRecordCount = table->write(m_signalList.data(), m_signalList.count() );

    m_signalMutex.unlock();

    table->close();

    if (writtenRecordCount != signalCount() )
    {
        return false;
    }

    qDebug() << "OutputSignalBase::load() - Written output signals: " << writtenRecordCount;

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

int OutputSignalBase::appendSignal(const OutputSignal& signal)
{
    int index = -1;

    m_signalMutex.lock();

        m_signalList.append(signal);
        index = m_signalList.size();

    m_signalMutex.unlock();

    return index;
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignal OutputSignalBase::signal(const int index) const
{
    OutputSignal signal;

    m_signalMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            signal = m_signalList[index];
        }

    m_signalMutex.unlock();

    return signal;
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalBase::setSignal(const int index, const OutputSignal& signal)
{
    m_signalMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            m_signalList[index] = signal;
        }

    m_signalMutex.unlock();
}


// -------------------------------------------------------------------------------------------------------------------

void OutputSignalBase::remove(const OutputSignal& signal)
{
    int index = find(signal);

    return remove(index);
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalBase::remove(const int index)
{
    m_signalMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            m_signalList.remove(index);
        }

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int OutputSignalBase::find(const OutputSignal& signal)
{
    int foundIndex = -1;

    m_signalMutex.lock();

        int count = m_signalList.size();

        for(int i = 0; i < count; i ++)
        {
            if (m_signalList[i] == signal)
            {
                foundIndex = i;
                break;
            }
        }

    m_signalMutex.unlock();

    return foundIndex;
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignalBase& OutputSignalBase::operator=(const OutputSignalBase& from)
{
    m_signalMutex.lock();

        m_signalList.clear();

        m_signalList = from.m_signalList;

    m_signalMutex.unlock();

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------



