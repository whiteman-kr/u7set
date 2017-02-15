#include "SignalBase.h"

#include "Measure.h"
#include "CalibratorBase.h"
#include "MeasurementBase.h"
#include "Database.h"

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
        return QString("Not measured");
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

SignalPosition::SignalPosition()
{

}

// -------------------------------------------------------------------------------------------------------------------

SignalPosition::SignalPosition(const QString& equipmentID) :
    m_equipmentID (equipmentID)
{
    if (equipmentID.isEmpty() == true)
    {
        return;
    }

    setFromID(equipmentID);
}

// -------------------------------------------------------------------------------------------------------------------

SignalPosition::~SignalPosition()
{

}

// -------------------------------------------------------------------------------------------------------------------

void SignalPosition::clear()
{
    m_equipmentID = QString();

    m_caseNo = -1;

    m_caseType = -1;
    m_caseCaption = QString();

    m_channel = -1;
    m_subblock = -1;
    m_block = -1;
    m_entry = -1;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalPosition::setFromID(const QString& equipmentID)
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

    //m_caseNo = value.toInt() - 1;
    m_caseNo = value.toInt();

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

    //m_subblock = value.toInt() - 1;
    m_subblock = value.toInt();

    // Module
    //

    value = equipmentID;

    begPos = value.indexOf("_", begPos + 1);
    if (begPos == -1)
    {
        return;
    }

    value.remove(0, begPos + 3);

    m_block = value.toInt();

    endPos = value.indexOf("_", 0);
    if (endPos == -1)
    {
        return;
    }

    value.remove(endPos, value.count());

    //m_block = value.toInt() - 1;
    m_block = value.toInt();

    // Input
    //

    value = equipmentID;

    begPos = value.indexOf("_", begPos + 1);
    if (begPos == -1)
    {
        return;
    }

    endPos = begPos + 1;

    while( isdigit( value[ endPos ].toAscii() ) == 0)
    {
        endPos++;

        if (endPos >= value.count())
        {
            break;
        }
    }

    value.remove(0, endPos);

    value.remove(2, value.count());

    m_entry = value.toInt() - 1;
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalPosition::caseStr() const
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

QString SignalPosition::channelStr() const
{
    return m_channel == -1 ? QT_TRANSLATE_NOOP("SignalBase.cpp", "No") : QString::number(m_channel + 1);
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalPosition::subblockStr() const
{
    return m_subblock == -1 ? QT_TRANSLATE_NOOP("SignalBase.cpp", "No") : QString::number(m_subblock + 1);
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalPosition::blockStr() const
{
    return m_block == -1 ? QT_TRANSLATE_NOOP("SignalBase.cpp", "No") : QString::number(m_block + 1);
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalPosition::entryStr() const
{
    return m_entry == -1 ? QT_TRANSLATE_NOOP("SignalBase.cpp", "No") : QString::number(m_entry + 1);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalParam::SignalParam()
{
}

// -------------------------------------------------------------------------------------------------------------------

SignalParam::SignalParam(const Signal& signal)
{
    setParam(signal);
}

// -------------------------------------------------------------------------------------------------------------------

SignalParam::~SignalParam()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalParam::isValid() const
{
    if (m_appSignalID.isEmpty() == true || m_hash == 0)
    {
        return false;
    }

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

int analogTuningSignalCount = 0;
int discreteTuningSignalCount = 0;

void SignalParam::setParam(const Signal& signal)
{
    setAppSignalID(signal.appSignalID());
    setCustomAppSignalID(signal.customAppSignalID());
    setCaption(signal.caption());

    setSignalType(signal.signalType());
    setInOutType(signal.inOutType());

    m_position.setFromID(signal.equipmentID());

    setLowADC(signal.lowADC());
    setHighADC(signal.highADC());

    setInputElectricLowLimit(signal.inputLowLimit());
    setInputElectricHighLimit(signal.inputHighLimit());
    setInputElectricUnitID(signal.inputUnitID());
    setInputElectricSensorType(signal.inputSensorType());
    setInputElectricPrecision(3);

    setInputPhysicalLowLimit(signal.lowEngeneeringUnits());
    setInputPhysicalHighLimit(signal.highEngeneeringUnits());
    setInputPhysicalUnitID(signal.unitID());
    setInputPhysicalPrecision(signal.decimalPlaces());
    setAdjustment(0);

    setHasOutput(false);

    switch(signal.outputMode())
    {
        case E::OutputMode::Plus0_Plus5_V:     setOutputElectricLowLimit(0);    setOutputElectricHighLimit(5);  setOutputElectricUnitID(E::InputUnit::V);   break;
        case E::OutputMode::Plus4_Plus20_mA:   setOutputElectricLowLimit(4);    setOutputElectricHighLimit(20); setOutputElectricUnitID(E::InputUnit::mA);  break;
        case E::OutputMode::Minus10_Plus10_V:  setOutputElectricLowLimit(-10);  setOutputElectricHighLimit(10); setOutputElectricUnitID(E::InputUnit::V);   break;
        case E::OutputMode::Plus0_Plus5_mA:    setOutputElectricLowLimit(0);    setOutputElectricHighLimit(5);  setOutputElectricUnitID(E::InputUnit::mA);  break;
    }
    setOutputElectricSensorType(signal.outputSensorType());
    setOutputElectricPrecision(3);

    setOutputPhysicalLowLimit(signal.outputLowLimit());
    setOutputPhysicalHighLimit(signal.outputHighLimit());
    setOutputPhysicalUnitID(signal.outputUnitID());
    setOutputPhysicalPrecision(signal.decimalPlaces());

    setNormalState(signal.normalState());

    setEnableTuning(signal.enableTuning());
    setTuningDefaultValue(signal.tuningDefaultValue());

    if (signal.enableTuning() == true)
    {
        switch (signalType())
        {
            case E::SignalType::Analog:     m_position.setEntry( analogTuningSignalCount++ );   break;
            case E::SignalType::Discrete:   m_position.setEntry( discreteTuningSignalCount++ ); break;
        }
    }

}

// -------------------------------------------------------------------------------------------------------------------

void SignalParam::setAppSignalID(const QString& appSignalID)
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

QString SignalParam::adcRangeStr(bool showHex) const
{
    QString range;

    if (showHex == false)
    {
        range.sprintf("%d .. %d", m_lowADC, m_highADC);
    }
    else
    {
        range.sprintf("0x%04X .. 0x%04X", m_lowADC, m_highADC);
    }

    return range;
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalParam::inputElectricRangeStr() const
{
    QString range, formatStr;

    formatStr.sprintf( ("%%.%df"), m_inputElectricPrecision );

    range.sprintf(formatStr.toAscii() + " .. " + formatStr.toAscii(), m_inputElectricLowLimit, m_inputElectricHighLimit);

    if (theUnitBase.hasUnit( m_inputElectricUnitID ) == true)
    {
        range.append( " " + theUnitBase.unit( m_inputElectricUnitID ) );
    }

    return range;
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalParam::inputElectricSensorStr() const
{
    if (theUnitBase.hasSensorType( m_inputElectricSensorType ) == false)
    {
        return QString();
    }

    return SensorTypeStr[ m_inputElectricSensorType ];
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalParam::inputPhysicalRangeStr() const
{
    QString range, formatStr;

    formatStr.sprintf( ("%%.%df"), m_inputPhysicalPrecision );

    range.sprintf( formatStr.toAscii() + " .. " + formatStr.toAscii(), m_inputPhysicalLowLimit, m_inputPhysicalHighLimit);

    if (theUnitBase.hasUnit( m_inputPhysicalUnitID ) == true)
    {
        range.append( " " + theUnitBase.unit( m_inputPhysicalUnitID ) );
    }

    return range;
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalParam::outputElectricRangeStr() const
{
    QString range, formatStr;

    formatStr.sprintf( ("%%.%df"), m_outputElectricPrecision );

    range.sprintf(formatStr.toAscii() + " .. " + formatStr.toAscii(), m_outputElectricLowLimit, m_outputElectricHighLimit);

    if (theUnitBase.hasUnit( m_outputElectricUnitID ) == true)
    {
        range.append( " " + theUnitBase.unit( m_outputElectricUnitID ) );
    }

    return range;
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalParam::outputElectricSensorStr() const
{
    if (theUnitBase.hasSensorType( m_outputElectricSensorType ) == false)
    {
        return QString();
    }

    return SensorTypeStr[ m_outputElectricSensorType ];
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalParam::outputPhysicalRangeStr() const
{
    QString range, formatStr;

    formatStr.sprintf( ("%%.%df"), m_outputPhysicalPrecision );

    range.sprintf( formatStr.toAscii() + " .. " + formatStr.toAscii(), m_outputPhysicalLowLimit, m_outputPhysicalHighLimit);

    if (theUnitBase.hasUnit( m_outputPhysicalUnitID ) == true)
    {
        range.append( " " + theUnitBase.unit( m_outputPhysicalUnitID ) );
    }

    return range;
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalParam::enableTuningStr() const
{
    if (m_enableTuning == false)
    {
        return QString();
    }

    return QString("Yes");
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalParam::tuningDefaultValueStr() const
{
    if (m_enableTuning == false)
    {
        return QString();
    }

    QString stateStr, formatStr;

    switch (m_signalType)
    {
        case E::SignalType::Analog:

            formatStr.sprintf( ("%%.%df"), m_inputPhysicalPrecision );

            stateStr.sprintf( formatStr.toAscii(), m_tuningDefaultValue);

            if (theUnitBase.hasUnit( m_inputPhysicalUnitID ) == true)
            {
                stateStr.append(" " + theUnitBase.unit( m_inputPhysicalUnitID ) );
            }

            break;

        case E::SignalType::Discrete:

            stateStr = m_tuningDefaultValue == 0 ? QString("No") : QString("Yes");

            break;

        default:
            assert(0);
    }


    return stateStr;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MetrologySignal::MetrologySignal()
{
}

// -------------------------------------------------------------------------------------------------------------------

MetrologySignal::MetrologySignal(const Signal& param)
{
    m_param.setParam(param);

    // temporary solution
    // because u7 can not set electric range
    //
    m_param.setInputElectricLowLimit(0);
    m_param.setInputElectricHighLimit(5);
    m_param.setInputElectricUnitID( E::InputUnit::V );
    //
    // temporary solution
}

// -------------------------------------------------------------------------------------------------------------------

MetrologySignal::~MetrologySignal()
{
}

// -------------------------------------------------------------------------------------------------------------------

MetrologySignal& MetrologySignal::operator=(const MetrologySignal& from)
{
    m_param = from.m_param;
    m_state = from.m_state;

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MetrologyMultiSignal::MetrologyMultiSignal()
{
}

// -------------------------------------------------------------------------------------------------------------------

MetrologyMultiSignal::MetrologyMultiSignal(const MetrologyMultiSignal& from)
{
    *this = from;
}

// -------------------------------------------------------------------------------------------------------------------

MetrologyMultiSignal::~MetrologyMultiSignal()
{
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyMultiSignal::clear()
{
    m_mutex.lock();

        for(int c = 0; c < MAX_CHANNEL_COUNT; c++)
        {
            m_signalHash[c] = 0;
        }

        m_position.clear();

        m_strID = QString();

    m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

bool MetrologyMultiSignal::isEmpty() const
{
    bool empty = true;

    m_mutex.lock();

        for(int c = 0; c < MAX_CHANNEL_COUNT; c++)
        {
            if (m_signalHash[c] != 0)
            {
                empty = false;

                break;
            }
        }

     m_mutex.unlock();

    return empty;
}

// -------------------------------------------------------------------------------------------------------------------

Hash MetrologyMultiSignal::hash(int channel) const
{
    if (channel < 0 || channel >= MAX_CHANNEL_COUNT)
    {
        assert(0);
        return 0;
    }

    Hash hash = 0;

    m_mutex.lock();

        hash = m_signalHash[channel];

    m_mutex.unlock();

    return hash;
}

// -------------------------------------------------------------------------------------------------------------------

bool MetrologyMultiSignal::setSignal(int channel, int measureKind, const SignalParam& param)
{
    if (channel < 0 || channel >= MAX_CHANNEL_COUNT)
    {
        assert(0);
        return false;
    }

    if (measureKind < 0 || measureKind >= MEASURE_KIND_COUNT)
    {
        assert(0);
        return false;
    }

    if (param.isValid() == false)
    {
        assert(false);
        return false;
    }

    m_mutex.lock();

        m_signalHash[channel] = param.hash();

        m_position.setCaseNo( param.position().caseNo() );
        m_position.setChannel( param.position().channel() );
        m_position.setSubblock( param.position().subblock() );
        m_position.setBlock( param.position().block() );
        m_position.setEntry( param.position().entry() );

        switch(measureKind)
        {
            case MEASURE_KIND_ONE:      m_strID = param.customAppSignalID(); break;
            case MEASURE_KIND_MULTI:    m_strID.sprintf( "CH %02d _ MD %02d _ IN %02d",  m_position.subblock() + 1, m_position.block() + 1, m_position.entry() + 1); break;
            default:                    assert(false);
        }

    m_mutex.unlock();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

MetrologyMultiSignal& MetrologyMultiSignal::operator=(const MetrologyMultiSignal& from)
{
    m_mutex.lock();

        for(int c = 0; c < MAX_CHANNEL_COUNT; c++)
        {
            m_signalHash[c] = from.m_signalHash[c];
        }

        m_position = from.m_position;

        m_strID = from.m_strID;

    m_mutex.unlock();

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureParam::MeasureParam()
{
}

// -------------------------------------------------------------------------------------------------------------------

MeasureParam::MeasureParam(const MeasureParam& from)
{
    *this = from;
}

// -------------------------------------------------------------------------------------------------------------------

MeasureParam::~MeasureParam()
{
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureParam::clear()
{
    m_mutex.lock();

        for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
        {
            m_param[type].setAppSignalID(QString());
        }

        m_outputSignalType = OUTPUT_SIGNAL_TYPE_UNUSED;

        m_equalPhysicalRange = false;

        m_pCalibratorManager = nullptr;

    m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureParam::isValid() const
{
    bool valid = true;

    m_mutex.lock();

        if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
        {
            valid = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT].isValid();
        }
        else
        {
            for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
            {
                if (m_param[type].isValid() == false )
                {
                    valid = false;

                    break;
                }
            }
        }

     m_mutex.unlock();

    return valid;
}

// -------------------------------------------------------------------------------------------------------------------

SignalParam MeasureParam::param(int type) const
{
    if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
    {
        return SignalParam();
    }

    SignalParam param;

    m_mutex.lock();

        param = m_param[type];

    m_mutex.unlock();

    return param;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureParam::setParam(int type, const SignalParam& param)
{
    if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
    {
        return false;
    }

    if (param.isValid() == false)
    {
        return false;
    }

    m_mutex.lock();

        m_param[type] = param;

        m_equalPhysicalRange = testPhysicalRange();

    m_mutex.unlock();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureParam::testPhysicalRange()
{
    if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
    {
        return true;
    }

    const SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
    const SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];

    if (inParam.isValid() == false || outParam.isValid() == false)
    {
        return false;
    }

    if (inParam.inputPhysicalLowLimit() != outParam.inputPhysicalLowLimit())
    {
        return false;
    }

    if (inParam.inputPhysicalHighLimit() != outParam.inputPhysicalHighLimit())
    {
        return false;
    }

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureParam::caseStr() const
{
    QString strResult;

    m_mutex.lock();

        if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
        {
            const SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
            if (param.isValid() == true)
            {
                strResult = param.position().caseStr();
            }
        }
        else
        {
            const SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
            if (param.isValid() == true)
            {
                strResult = param.position().caseStr();
            }
        }

    m_mutex.unlock();

    return strResult;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureParam::signalID(bool showCustomID, const QString& divider) const
{
    QString strResult;

    m_mutex.lock();

        if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
        {
            const SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
            if (param.isValid() == true)
            {
                strResult = showCustomID == true ? param.customAppSignalID() : param.appSignalID();
            }
        }
        else
        {
            const SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
            if (inParam.isValid() == true)
            {
                strResult = (showCustomID == true ? inParam.customAppSignalID() : inParam.appSignalID()) + divider;
            }

            const SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
            if (outParam.isValid() == true)
            {
                strResult += (showCustomID == true ? outParam.customAppSignalID() : outParam.appSignalID());
            }
        }

    m_mutex.unlock();

    return strResult;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureParam::subblockStr() const
{
    QString strResult;

    m_mutex.lock();

        if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
        {
            const SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
            if (param.isValid() == true)
            {
                strResult = param.position().subblockStr();
            }
        }
        else
        {
            const SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
            if (param.isValid() == true)
            {
                strResult = param.position().subblockStr();
            }
        }

    m_mutex.unlock();

    return strResult;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureParam::blockStr() const
{
    QString strResult;

    m_mutex.lock();

        if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
        {
            const SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
            if (param.isValid() == true)
            {
                strResult = param.position().blockStr();
            }
        }
        else
        {
            const SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
            if (param.isValid() == true)
            {
                strResult = param.position().blockStr();
            }
        }

    m_mutex.unlock();

    return strResult;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureParam::entryStr() const
{
    QString strResult;

    m_mutex.lock();

        if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
        {
            const SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
            if (param.isValid() == true)
            {
                strResult = param.position().entryStr();
            }
        }
        else
        {
            const SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
            if (param.isValid() == true)
            {
                strResult = param.position().entryStr();
            }
        }

    m_mutex.unlock();

    return strResult;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureParam::caption(const QString& divider) const
{
    QString strResult;

    m_mutex.lock();

        if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
        {
            const SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
            if (param.isValid() == true)
            {
                strResult = param.caption();
            }
        }
        else
        {
            const SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
            const SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];

            QString inСaptionStr, outСaptionStr;

            if (inParam.isValid() == true)
            {
                inСaptionStr = inParam.caption();
            }

            if (outParam.isValid() == true)
            {
                outСaptionStr = outParam.caption();
            }

            strResult = inСaptionStr + divider + outСaptionStr;
        }

    m_mutex.unlock();

    return strResult;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureParam::physicalRangeStr(const QString& divider) const
{
    QString strResult;

    m_mutex.lock();

        if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
        {
            const SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
            if (param.isValid() == true)
            {
                strResult = param.inputPhysicalRangeStr();
            }
        }
        else
        {
            const SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
            const SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];

            QString inRangeStr, outRangeStr;

            if (inParam.isValid() == true)
            {
                inRangeStr = inParam.inputPhysicalRangeStr();
            }

            if (outParam.isValid() == true)
            {
                outRangeStr = outParam.inputPhysicalRangeStr();
            }

            if (m_equalPhysicalRange == true)
            {
                strResult = outRangeStr;
            }
            else
            {
                strResult = inRangeStr + divider + outRangeStr;
            }
        }

    m_mutex.unlock();

    return strResult;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureParam::electricRangeStr(const QString& divider) const
{
    QString strResult;

    m_mutex.lock();

        if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
        {
            const SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
            if (param.isValid() == true)
            {
                strResult = param.inputElectricRangeStr();
            }
        }
        else
        {
            const SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
            const SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];

            QString inRangeStr, outRangeStr;

            if (inParam.isValid() == true && inParam.isInput() == true)
            {
                inRangeStr = inParam.inputElectricRangeStr();
            }

            if (outParam.isValid() == true && outParam.isOutput() == true)
            {
                outRangeStr = outParam.outputElectricRangeStr();
            }

            strResult = inRangeStr + divider + outRangeStr;
        }

    m_mutex.unlock();

    return strResult;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureParam::electricSensorStr(const QString& divider) const
{
    QString strResult;

    m_mutex.lock();

        if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
        {
            const SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
            if (param.isValid() == true)
            {
                strResult = param.inputElectricSensorStr();
            }
        }
        else
        {
            const SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
            const SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];

            QString inSensorStr, outSensorStr;

            if (inParam.isValid() == true && inParam.isInput() == true)
            {
                inSensorStr = inParam.inputElectricSensorStr();
            }

            if (outParam.isValid() == true && outParam.isOutput() == true)
            {
                outSensorStr = outParam.outputElectricSensorStr();
            }

            strResult = inSensorStr + divider + outSensorStr;
        }

    m_mutex.unlock();

    return strResult;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureParam::calibratorStr() const
{
    if (m_pCalibratorManager == nullptr || m_pCalibratorManager->calibratorIsConnected() == false)
    {
        return QString("Not connected");
    }

    return QString("Calibrator %1 (%2)").arg( m_pCalibratorManager->channel() + 1).arg(m_pCalibratorManager->portName());
}


// -------------------------------------------------------------------------------------------------------------------

MeasureParam& MeasureParam::operator=(const MeasureParam& from)
{
    m_mutex.lock();

        for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
        {
            m_param[type] = from.m_param[type];
        }

        m_outputSignalType = from.m_outputSignalType;

        m_equalPhysicalRange = from.m_equalPhysicalRange;

        m_pCalibratorManager = from.m_pCalibratorManager;

    m_mutex.unlock();

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureSignal::MeasureSignal()
{
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal::MeasureSignal(const MeasureSignal& from)
{
    *this = from;
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal::~MeasureSignal()
{
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureSignal::clear()
{
    m_mutex.lock();

        m_outputSignalType = OUTPUT_SIGNAL_TYPE_UNUSED;

        for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
        {
            m_signal[type].clear();
        }

    m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureSignal::isEmpty() const
{
    bool empty = true;

    m_mutex.lock();

        for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
        {
            if (m_signal[type].isEmpty() == false )
            {
                empty = false;

                break;
            }
        }

     m_mutex.unlock();

    return empty;
}

// -------------------------------------------------------------------------------------------------------------------

MetrologyMultiSignal MeasureSignal::signal(int type) const
{
    if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
    {
        return MetrologyMultiSignal();
    }

    MetrologyMultiSignal signal;

    m_mutex.lock();

        signal = m_signal[type];

    m_mutex.unlock();

    return signal;
}

// -------------------------------------------------------------------------------------------------------------------

Hash MeasureSignal::signalHash(int type, int channel) const
{
    if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
    {
        return 0;
    }

    if (channel < 0 || channel >= MAX_CHANNEL_COUNT)
    {
        return 0;
    }

    Hash hash;

    m_mutex.lock();

        hash = m_signal[type].hash(channel);

    m_mutex.unlock();

    return hash;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureSignal::setSignal(int type, const MetrologyMultiSignal& signal)
{
    if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
    {
        return false;
    }

    m_mutex.lock();

        m_signal[type] = signal;

    m_mutex.unlock();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureSignal::setSignal(int channel, int measureKind, int outputSignalType, const SignalParam& param)
{
    if (channel < 0 || channel >= MAX_CHANNEL_COUNT)
    {
        assert(0);
        return false;
    }

    if (measureKind < 0 || measureKind >= MEASURE_KIND_COUNT)
    {
        assert(0);
        return false;
    }

    if (outputSignalType < 0 || outputSignalType >= OUTPUT_SIGNAL_TYPE_COUNT)
    {
        assert(0);
        return false;
    }

    if (param.isValid() == false)
    {
        assert(false);
        return false;
    }

    bool result = true;

    switch (outputSignalType)
    {
        case OUTPUT_SIGNAL_TYPE_UNUSED:
            {
                m_mutex.lock();

                    m_outputSignalType = OUTPUT_SIGNAL_TYPE_UNUSED;

                    result = m_signal[MEASURE_IO_SIGNAL_TYPE_INPUT].setSignal(channel, measureKind, param);

                m_mutex.unlock();
            }
            break;

        case OUTPUT_SIGNAL_TYPE_FROM_INPUT:
        case OUTPUT_SIGNAL_TYPE_FROM_TUNING:
            {
                int index = theOutputSignalBase.find(MEASURE_IO_SIGNAL_TYPE_INPUT, param.hash(), outputSignalType);
                if (index == -1)
                {
                    result = false;
                    break;
                }

                OutputSignal outputSignal = theOutputSignalBase.signal(index);
                if (outputSignal.isValid() == false)
                {
                    result = false;
                    break;
                }

                outputSignal.updateParam();

                m_mutex.lock();

                    m_outputSignalType = outputSignalType;

                    for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type ++)
                    {
                        SignalParam paramFromOutputSignal = outputSignal.param(type);
                        if (paramFromOutputSignal.isValid() == false)
                        {
                            result = false;
                            break;
                        }

                        if (m_signal[type].setSignal(channel, measureKind, paramFromOutputSignal) == false)
                        {
                            result = false;
                            break;
                        }
                    }

                m_mutex.unlock();
            }

            break;

        default:
            assert(0);
    }

    if (result == false)
    {
        clear();
    }

    return result;

}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal& MeasureSignal::operator=(const MeasureSignal& from)
{
    m_mutex.lock();

        m_outputSignalType = from.m_outputSignalType;

        for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
        {
            m_signal[type] = from.m_signal[type];
        }

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
    emit activeSignalChanged(m_activeSignal);

    m_signalMesaureMutex.lock();

        m_signalMesaureList.clear();

    m_signalMesaureMutex.unlock();

    m_caseMutex.lock();

        m_caseTypeList.clear();

    m_caseMutex.unlock();

    m_stateMutex.lock();

        m_requestStateList.clear();

    m_stateMutex.unlock();

    m_signalMutex.lock();

        m_signalHashMap.clear();
        m_signalList.clear();

        analogTuningSignalCount = 0;
        discreteTuningSignalCount = 0;

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
                if ( m_signalList[i].param().position().equipmentID() > m_signalList[j].param().position().equipmentID() )
                {
                    MetrologySignal signal	= m_signalList[ i ];
                    m_signalList[ i ]       = m_signalList[ j ];
                    m_signalList[ j ]       = signal;
                }
            }
        }

    m_signalMutex.unlock();

    qDebug() << __FUNCTION__ <<  responseTime.elapsed() << " ms";
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::appendSignal(const Signal& signal)
{
    if (signal.appSignalID().isEmpty() == true || signal.hash() == 0)
    {
        assert(false);
        return -1;
    }

    int index = -1;

    m_signalMutex.lock();

        if (m_signalHashMap.contains(signal.hash()) == false)
        {
            MetrologySignal metrologySignal(signal);

            m_signalList.append(metrologySignal);
            index = m_signalList.size() - 1;

            m_signalHashMap.insert(signal.hash(), index);
        }

     m_signalMutex.unlock();

     return index;
}

// -------------------------------------------------------------------------------------------------------------------

MetrologySignal SignalBase::signal(const QString& appSignalID)
{
    if (appSignalID.isEmpty() == true)
    {
        assert(false);
        return MetrologySignal();
    }

    return signal( calcHash(appSignalID) );
}

// -------------------------------------------------------------------------------------------------------------------

MetrologySignal SignalBase::signal(const Hash& hash)
{
    if (hash == 0)
    {
        assert(hash != 0);
        return MetrologySignal();
    }

    MetrologySignal signal;

    m_signalMutex.lock();

        if (m_signalHashMap.contains(hash) == true)
        {
            int index = m_signalHashMap[hash];

            if (index >= 0 && index < m_signalList.size())
            {
                signal = m_signalList[index];
            }
        }

    m_signalMutex.unlock();

    return signal;
}

// -------------------------------------------------------------------------------------------------------------------

MetrologySignal SignalBase::signal(int index)
{
    MetrologySignal signal;

    m_signalMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            signal = m_signalList[index];
        }

    m_signalMutex.unlock();

    return signal;
}

// -------------------------------------------------------------------------------------------------------------------

SignalParam SignalBase::signalParam(const QString& appSignalID)
{
    if (appSignalID.isEmpty() == true)
    {
        assert(false);
        return SignalParam();
    }

    return signalParam( calcHash(appSignalID) );
}

// -------------------------------------------------------------------------------------------------------------------

SignalParam SignalBase::signalParam(const Hash& hash)
{
    if (hash == 0)
    {
        assert(hash != 0);
        return SignalParam();
    }

    SignalParam param;

    m_signalMutex.lock();

        if (m_signalHashMap.contains(hash) == true)
        {
            int index = m_signalHashMap[hash];

            if (index >= 0 && index < m_signalList.size())
            {
                param = m_signalList[index].param();
            }
        }

    m_signalMutex.unlock();

    return param;
}

// -------------------------------------------------------------------------------------------------------------------

SignalParam SignalBase::signalParam(int index)
{
    SignalParam param;

    m_signalMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            param = m_signalList[index].param();
        }

    m_signalMutex.unlock();

    return param;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignalParam(const Hash& hash, const SignalParam& param)
{
    if (hash == 0)
    {
        assert(hash != 0);
        return;
    }

    m_signalMutex.lock();

        if (m_signalHashMap.contains(hash) == true)
        {
            int index = m_signalHashMap[hash];

            if (index >= 0 && index < m_signalList.size())
            {
                m_signalList[index].setParam(param);

                emit updatedSignalParam(param.hash());
            }
        }

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignalParam(int index, const SignalParam& param)
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

AppSignalState SignalBase::signalState(const QString& appSignalID)
{
    if (appSignalID.isEmpty() == true)
    {
        assert(false);
        return AppSignalState();
    }

    return signalState(calcHash(appSignalID));
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

        if (m_signalHashMap.contains(hash) == true)
        {
            int index = m_signalHashMap[hash];

            if (index >= 0 && index < m_signalList.size())
            {
                state = m_signalList[index].state();
            }
        }

    m_signalMutex.unlock();

    return state;
}

// -------------------------------------------------------------------------------------------------------------------

AppSignalState SignalBase::signalState(int index)
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

void SignalBase::setSignalState(const QString& appSignalID, const AppSignalState &state)
{
    if (appSignalID.isEmpty() == true)
    {
        assert(false);
        return;
    }

    setSignalState(calcHash(appSignalID), state);
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

        if (m_signalHashMap.contains(hash) == true)
        {
            index = m_signalHashMap[hash];

            if (index >= 0 && index < m_signalList.size())
            {
                m_signalList[index].setState(state);
            }
        }

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignalState(int index, const AppSignalState &state)
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

int SignalBase::createCaseTypeList(int outputSignalType)
{
    if (outputSignalType < 0 || outputSignalType >= OUTPUT_SIGNAL_TYPE_COUNT)
    {
        assert(false);
        return 0;
    }

    int caseTypeCount = 0;
    QMap<QString, int> caseTypeMap;


    // find all type of cases for selected outputSignalType and create caseTypeList for ToolBar
    //
    m_caseMutex.lock();

        m_caseTypeList.clear();

        int count = m_signalList.size();

        for(int i = 0; i < count; i ++)
        {
            SignalParam& param = m_signalList[i].param();
            if (param.isValid() == false)
            {
                continue;
            }

            if (param.isAnalog() == false)
            {
                continue;
            }

            if (param.position().subblock() == -1 || param.position().block() == -1)
            {
                continue;
            }

            switch (outputSignalType)
            {
                case OUTPUT_SIGNAL_TYPE_UNUSED:
                case OUTPUT_SIGNAL_TYPE_FROM_INPUT:

                    if (param.isInput() == false)
                    {
                        continue;
                    }

                    if (param.position().entry() == -1)
                    {
                        continue;
                    }

                    break;

                case OUTPUT_SIGNAL_TYPE_FROM_TUNING:

                    if (param.isInternal() == false)
                    {
                        continue;
                    }

                    break;

                default:
                    assert(0);
                    continue;
            }

            const QString& caseCaption = param.position().caseCaption();

            if (caseCaption.isEmpty() == true)
            {
                continue;
            }

            if (caseTypeMap.contains( caseCaption ) == true)
            {
                int caseType = caseTypeMap[ caseCaption ];
                param.setCaseType( caseType );
            }
            else
            {
                caseTypeMap.insert(caseCaption, caseTypeCount);
                param.setCaseType(caseTypeCount);

                caseTypeCount++;

                m_caseTypeList.append(caseCaption);
            }
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

QString SignalBase::caseTypeCaption(int type)
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

int SignalBase::caseNoByCaseIndex(int caseIndex)
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

int SignalBase::createMeasureSignalList(int caseType, int measureKind, int outputSignalType)
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

    if (outputSignalType < 0 || outputSignalType >= OUTPUT_SIGNAL_TYPE_COUNT)
    {
        assert(false);
        return 0;
    }

    int signalCount = 0;

    int caseNoIndex = 0;
    QMap<int, int>  caseNoMap;

    // find all caseNo for selected caseType and create caseNo map for ToolBar
    //
    m_caseMutex.lock();

        m_caseNoList.clear();

        signalCount = m_signalList.size();

        for(int i = 0; i < signalCount; i ++)
        {
            SignalParam param = m_signalList[i].param();
            if (param.isValid() == false)
            {
                continue;
            }

            if (param.isAnalog() == false)
            {
                continue;
            }

            switch (outputSignalType)
            {
                case OUTPUT_SIGNAL_TYPE_UNUSED:
                case OUTPUT_SIGNAL_TYPE_FROM_INPUT:

                    if (param.isInput() == false)
                    {
                        continue;
                    }

                    break;

                case OUTPUT_SIGNAL_TYPE_FROM_TUNING:

                    if (param.isInternal() == false)
                    {
                        continue;
                    }

                    break;

                default:
                    assert(0);
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

    int             signalIndex = 0;
    MeasureSignal   measureSignal;
    QMap<Hash, int> mesaureSignalMap;

    // find all signals for selected caseType and create Measure Signal List map for ToolBar
    //
    m_signalMesaureMutex.lock();

        m_signalMesaureList.clear();

        signalCount = m_signalList.size();

        for(int i = 0; i < signalCount; i ++)
        {
            SignalParam param = m_signalList[i].param();
            if (param.isValid() == false)
            {
                continue;
            }

            if (param.isAnalog() == false)
            {
                continue;
            }

            if (param.position().subblock() == -1 || param.position().block() == -1)
            {
                continue;
            }

            switch (outputSignalType)
            {
                case OUTPUT_SIGNAL_TYPE_UNUSED:
                case OUTPUT_SIGNAL_TYPE_FROM_INPUT:

                    if (param.isInput() == false)
                    {
                        continue;
                    }

                    if (param.position().entry() == -1)
                    {
                        continue;
                    }

                    break;

                case OUTPUT_SIGNAL_TYPE_FROM_TUNING:

                    if (param.isInternal() == false)
                    {
                        continue;
                    }

                    break;

                default:
                    assert(0);
                    continue;
            }

            if (param.position().caseType()!= caseType)
            {
                continue;
            }

            measureSignal.clear();

            switch(measureKind)
            {

                case MEASURE_KIND_ONE:
                    {
                        measureSignal.setSignal(CHANNEL_0, measureKind, outputSignalType, param);
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

                        if (mesaureSignalMap.contains(hashid) == true)
                        {
                            int index = mesaureSignalMap[hashid];
                            if (index >= 0 && index < m_signalMesaureList.size())
                            {
                                MeasureSignal& ms = m_signalMesaureList[index];

                                if (caseNoMap.contains(param.position().caseNo()) == true)  // find index of case by caseNo
                                {
                                    int caseIndex = caseNoMap[param.position().caseNo()];
                                    if ( caseIndex >= 0 && caseIndex < MAX_CHANNEL_COUNT )
                                    {
                                        ms.setSignal(caseIndex, measureKind, outputSignalType, param);
                                    }
                                }
                            }

                            continue;
                        }

                        mesaureSignalMap.insert(hashid, signalIndex);

                        if (caseNoMap.contains(param.position().caseNo()) == true)  // find index of case by caseNo
                        {
                            int caseIndex = caseNoMap[param.position().caseNo()];
                            if ( caseIndex >= 0 && caseIndex < MAX_CHANNEL_COUNT )
                            {
                                measureSignal.setSignal(caseIndex, measureKind, outputSignalType, param);
                            }
                        }

                    }
                    break;

                default:
                    assert(false);
                    continue;
            }

            if(measureSignal.isEmpty() == true)
            {
                //assert(false);
                continue;
            }

            m_signalMesaureList.append(measureSignal);

            signalIndex++;
        }

    m_signalMesaureMutex.unlock();

    return signalIndex;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::measureSignalCount() const
{
    int count;

    m_signalMesaureMutex.lock();

        count = m_signalMesaureList.size();

    m_signalMesaureMutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal SignalBase::measureSignal(int index)
{
    MeasureSignal signal;

    m_signalMesaureMutex.lock();

        if (index >= 0 && index < m_signalMesaureList.size())
        {
            signal = m_signalMesaureList[index];
        }

    m_signalMesaureMutex.unlock();

    return signal;
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal SignalBase::activeSignal() const
{
    MeasureSignal signal;

    m_activeSignalMutex.lock();

        signal = m_activeSignal;

    m_activeSignalMutex.unlock();

    return signal;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setActiveSignal(const MeasureSignal& multiSignal)
{
    m_activeSignalMutex.lock();

        m_activeSignal = multiSignal;

        m_stateMutex.lock();

            m_requestStateList.clear();

            for(int i = 0; i < MAX_CHANNEL_COUNT; i++)
            {
                Hash hash = m_activeSignal.signal(MEASURE_IO_SIGNAL_TYPE_INPUT).hash(i);
                if (hash == 0)
                {
                    continue;
                }

                m_requestStateList.append(hash);

                if (m_activeSignal.outputSignalType() == OUTPUT_SIGNAL_TYPE_UNUSED)
                {
                    continue;
                }

                hash = m_activeSignal.signal(MEASURE_IO_SIGNAL_TYPE_OUTPUT).hash(i);
                if (hash == 0)
                {
                    continue;
                }

                m_requestStateList.append(hash);
            }

        m_stateMutex.unlock();

    m_activeSignalMutex.unlock();

    emit activeSignalChanged(m_activeSignal);
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

void UnitBase::appendUnit(int unitID, const QString& unit)
{
    m_unitMutex.lock();

        if (m_unitMap.contains(unitID) == false)
        {
            m_unitMap[unitID] = unit;
        }

    m_unitMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

bool UnitBase::hasUnit(int unitID)
{
    if (unitID == NO_UNIT_ID)
    {
        return false;
    }

    bool has;

    m_unitMutex.lock();

        has = m_unitMap.contains(unitID);

    m_unitMutex.unlock();

    return has;
}

// -------------------------------------------------------------------------------------------------------------------

bool UnitBase::hasSensorType(int sensorType)
{
    if (sensorType < 0 || sensorType >= SENSOR_TYPE_COUNT)
    {
        return false;
    }

    if (sensorType == E::SensorType::NoSensorType)
    {
        return false;
    }

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

QString UnitBase::unit(int unitID)
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
    m_hash = 0;

    m_type = OUTPUT_SIGNAL_TYPE_UNUSED;

    for(int k = 0; k < MEASURE_IO_SIGNAL_TYPE_COUNT; k++)
    {
        m_param[k].setAppSignalID(QString());
    }
}

// -------------------------------------------------------------------------------------------------------------------

bool OutputSignal::isValid() const
{
    if (m_hash == 0)
    {
        return false;
    }

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool OutputSignal::setHash()
{
    QString strID;

    for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
    {
        strID.append(m_appSignalID[type]);
    }

    if (strID.isEmpty() == true)
    {
        return false;
    }

    m_hash = calcHash( strID );
    if (m_hash == 0)
    {
        return false;
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

QString OutputSignal::appSignalID(int type) const
{
    if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
    {
        return QString();
    }

    QString appSignalID;

    m_signalMutex.lock();

        appSignalID = m_appSignalID[type];

    m_signalMutex.unlock();

    return appSignalID;
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignal::setAppSignalID(int type, const QString& appSignalID)
{
    if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
    {
        return;
    }

    m_signalMutex.lock();

        m_appSignalID[type] = appSignalID;

        setHash();

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

SignalParam OutputSignal::param(int type) const
{
    if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
    {
        return SignalParam();
    }

    SignalParam param;

    m_signalMutex.lock();

        param = m_param[type];

    m_signalMutex.unlock();

    return param;
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignal::setParam(int type, const SignalParam& param)
{
    if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
    {
        return;
    }

    if (param.isValid() == false)\
    {
        return;
    }

    m_signalMutex.lock();

        m_appSignalID[type] = param.appSignalID();

        setHash();

        m_param[type] = param;

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignal::updateParam()
{
    m_signalMutex.lock();

        for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
        {
            if (m_appSignalID[type].isEmpty() == true)
            {
                continue;
            }

            Hash signalHash = calcHash(m_appSignalID[type]);
            if (signalHash == 0)
            {
                continue;
            }

            m_param[type] = theSignalBase.signalParam(signalHash);
        }

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignal& OutputSignal::operator=(const OutputSignal& from)
{
    m_signalID = from.m_signalID;
    m_hash = from.m_signalID;

    m_type = from.m_type;

    for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
    {
        m_appSignalID[type] = from.m_appSignalID[type];

        m_param[type] = from.m_param[type];
    }

    return *this;
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
            index = m_signalList.size() - 1;

    m_signalMutex.unlock();

    return index;
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignal OutputSignalBase::signal(int index) const
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

void OutputSignalBase::setSignal(int index, const OutputSignal& signal)
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

void OutputSignalBase::remove(int index)
{
    m_signalMutex.lock();

        if (index >= 0 && index < m_signalList.size())
        {
            m_signalList.remove(index);
        }

    m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int OutputSignalBase::find(int measureIoType, const Hash& hash, int outputSignalType)
{
    if (measureIoType < 0 || measureIoType >= MEASURE_IO_SIGNAL_TYPE_COUNT)
    {
        assert(0);
        return -1;
    }

    if (hash == 0)
    {
        assert(hash != 0);
        return -1;
    }

    if (outputSignalType < 0 || outputSignalType >= OUTPUT_SIGNAL_TYPE_COUNT)
    {
        assert(0);
        return -1;
    }

    int foundIndex = -1;

    m_signalMutex.lock();

        int count = m_signalList.size();

        for(int i = 0; i < count; i ++)
        {
            const OutputSignal& signal = m_signalList[i];

            if (calcHash(m_signalList[i].appSignalID(measureIoType)) != hash)
            {
                continue;
            }

            if (signal.type() != outputSignalType)
            {
                continue;
            }

            foundIndex = i;
            break;
        }

    m_signalMutex.unlock();

    return foundIndex;

}

// -------------------------------------------------------------------------------------------------------------------

int OutputSignalBase::find(const OutputSignal& signal)
{
    int foundIndex = -1;

        m_signalMutex.lock();

        int count = m_signalList.size();

        for(int i = 0; i < count; i ++)
        {
            if (m_signalList[i].hash() == signal.hash())
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

        m_signalList = from.m_signalList;

    m_signalMutex.unlock();

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------



