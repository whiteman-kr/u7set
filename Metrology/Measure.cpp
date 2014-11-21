#include "Measure.h"

// -------------------------------------------------------------------------------------------------------------------

QString DevicePosition::caseString() const
{
    QString caseNo = m_caseNo == -1 ? "" : QString::number(m_caseNo + 1);

    QString result;

    if (caseNo.isEmpty() == false && m_caseType.isEmpty() == false)
    {
        result = caseNo + " - " + m_caseType;
    }
    else
    {
        result = caseNo + m_caseType;
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

MeasureItem::MeasureItem(int type)
{
    m_measureType = type;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureItem::strID(bool external)
{
    if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
    {
        return "";
    }

    QString result;

    switch(m_measureType)
    {
        case MEASURE_TYPE_LINEARITY:
            {
                LinearetyMeasureItem* measure = static_cast<LinearetyMeasureItem*> (this);
                if (measure == nullptr)
                {
                    break;
                }

                if  (external == true)
                {
                    result = measure->extStrID();
                }
                else
                {
                    result = measure->strID();
                }
            }
            break;

        case MEASURE_TYPE_COMPARATOR:
            break;

        case MEASURE_TYPE_COMPLEX_COMPARATOR:
            break;

        default:
            assert(0);
            break;

    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureItem::measureTimeString() const
{
    return m_measureTime.date().toString("dd-mm-yyyy ") + m_measureTime.time().toString("hh:mm:ss");
}

// -------------------------------------------------------------------------------------------------------------------

LinearetyMeasureItem::LinearetyMeasureItem() :
    MeasureItem(MEASURE_TYPE_LINEARITY)
{
    for(int v = 0; v < VALUE_TYPE_COUNT; v++)
    {
        m_lowLimit[v] = 0;
        m_highLimit[v] = 0;

        m_nominal[v] = 0;
        m_measure[v] = 0;

        for(int m = 0; m < MEASUREMENT_IN_POINT; m++)
        {
            m_measureArray[v][m] = 0;
        }

        m_valuePrecision[v] = 0;
    }

    for(int e = 0; e < ERROR_TYPE_COUNT; e++)
    {
        m_errorInput[e] = 0;
        m_errorOutput[e] = 0;
        m_errorLimit[e] = 0;

        m_errorPrecision[e] = 0;
    }

    for(int a = 0; a < ADDITIONAL_VALUE_COUNT; a++)
    {
        m_additionalValue[a] = 0;
    }
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearetyMeasureItem::limitString(int type) const
{
    if (type < 0 || type >= VALUE_TYPE_COUNT)
    {
        assert(0);
        return "";
    }

    QString low = QString::number(m_lowLimit[type], 10, m_valuePrecision[type]);
    QString high = QString::number(m_highLimit[type], 10, m_valuePrecision[type]);

    return QString("%1 .. %2 %3").arg(low).arg(high).arg(m_unit[type]);;
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearetyMeasureItem::nominalString(int type) const
{
    if (type < 0 || type >= VALUE_TYPE_COUNT)
    {
        assert(0);
        return "";
    }

    return QString("%1 %2").arg(QString::number(m_nominal[type], 10, m_valuePrecision[type])).arg(m_unit[type]);
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearetyMeasureItem::measureString(int type) const
{
    if (type < 0 || type >= VALUE_TYPE_COUNT)
    {
        assert(0);
        return "";
    }

    return QString("%1 %2").arg(QString::number(m_measure[type], 10, m_valuePrecision[type])).arg(m_unit[type]);
}

// -------------------------------------------------------------------------------------------------------------------

QString LinearetyMeasureItem::measureItemString(int type, int index) const
{
    if (type < 0 || type >= VALUE_TYPE_COUNT)
    {
        assert(0);
        return "";
    }

    if (index < 0 || index >= MEASUREMENT_IN_POINT)
    {
        assert(0);
        return "";
    }

    return QString::number(m_measureArray[type][index], 10, m_valuePrecision[type]);
}

// -------------------------------------------------------------------------------------------------------------------
