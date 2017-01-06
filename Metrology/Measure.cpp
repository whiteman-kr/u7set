#include "Measure.h"

#include "Options.h"
#include "Conversion.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
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

    value.remove(0, begPos + 1);

    value.remove(1, value.count());

    m_caseNo = value.toInt() - 1;

    // CaseType
    //

    value = equipmentID;

    begPos = value.indexOf("_");
    if (begPos == -1)
    {
        return;
    }

    value.remove(0, begPos + 2);

    endPos = value.indexOf("_", 0);
    if (endPos == -1)
    {
        return;
    }

    value.remove(endPos, value.count());

    m_caseType = value;

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

DevicePosition& DevicePosition::operator=(const DevicePosition& from)
{
    m_equipmentID = from.m_equipmentID;

    m_caseNo = from.m_caseNo;
    m_caseType = from.m_caseType;

    m_channel = from.m_channel;
    m_subblock = from.m_subblock;
    m_block = from.m_block;
    m_entry = from.m_entry;

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureItem::MeasureItem(int type)
{
    m_measureType = type;
}

// -------------------------------------------------------------------------------------------------------------------

MeasureItem* MeasureItem::at(int index)
{
    MeasureItem* pMeasure = nullptr;

    switch(m_measureType)
    {
        case MEASURE_TYPE_LINEARITY:            pMeasure = static_cast<LinearetyMeasureItem*> (this) + index;           break;
        case MEASURE_TYPE_COMPARATOR:           pMeasure = static_cast<ComparatorMeasureItem*> (this) + index;          break;
        case MEASURE_TYPE_COMPLEX_COMPARATOR:   pMeasure = static_cast<ComplexComparatorMeasureItem*> (this) + index;   break;
        default:                                assert(0);                                                              break;
    }

    return pMeasure;
}

// -------------------------------------------------------------------------------------------------------------------

MeasureItem& MeasureItem::operator=(MeasureItem& from)
{
    m_measureType = from.m_measureType;

    m_measureID = from.m_measureID;
    m_filter = from.m_filter;

    m_measureTime = from.m_measureTime;
    m_reportType = from.m_reportType;

    switch(m_measureType)
    {
        case MEASURE_TYPE_LINEARITY:            *static_cast<LinearetyMeasureItem*> (this) = *static_cast <LinearetyMeasureItem*> (&from);                  break;
        case MEASURE_TYPE_COMPARATOR:           *static_cast<ComparatorMeasureItem*> (this) = *static_cast <ComparatorMeasureItem*> (&from);                break;
        case MEASURE_TYPE_COMPLEX_COMPARATOR:   *static_cast<ComplexComparatorMeasureItem*> (this) = *static_cast <ComplexComparatorMeasureItem*> (&from);  break;
        default:                                assert(0);                                                                                                  break;
    }

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

LinearetyMeasureItem::LinearetyMeasureItem() :
    MeasureItem(MEASURE_TYPE_LINEARITY)
{
    for(int v = 0; v < VALUE_TYPE_COUNT; v++)
    {
        m_nominal[v] = 0;
        m_measure[v] = 0;

        for(int m = 0; m < MEASUREMENT_IN_POINT; m++)
        {
            m_measureArray[v][m] = 0;
        }

        m_lowLimit[v] = 0;
        m_highLimit[v] = 0;

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

LinearetyMeasureItem::LinearetyMeasureItem(Calibrator* pCalibrator)
{
    if (pCalibrator == nullptr)
    {
        return;
    }

    setMeasureType(MEASURE_TYPE_LINEARITY);

    // features
    //
    setAppSignalID("#IDMPS");
    setCustomAppSignalID("IDMPS");
    setCaption("This is signal of the block MPS");

    position().setCaseNo(0);
    position().setCaseType("CASE-1");
    position().setChannel(0);
    position().setBlock(0);
    position().setSubblock(0);
    position().setEntry(0);

    setValuePrecision(VALUE_TYPE_ELECTRIC, 3);
    setValuePrecision(VALUE_TYPE_PHYSICAL, 2);
    setValuePrecision(VALUE_TYPE_OUTPUT, 3);

    // nominal
    //

    double electric = pCalibrator->sourceValue();
    double physical = conversion(electric, CT_ELECTRIC_TO_PHYSICAL, INPUT_UNIT_OHM, OHM_PT_50_W_1391);

    setNominal(VALUE_TYPE_ELECTRIC, electric);
    setNominal(VALUE_TYPE_PHYSICAL, physical);
    setNominal(VALUE_TYPE_OUTPUT, 0);

    setPercent(0);

    // measure
    //
    int measureCount = theOptions.linearity().m_measureCountInPoint > MEASUREMENT_IN_POINT ? MEASUREMENT_IN_POINT : theOptions.linearity().m_measureCountInPoint;

    setMeasureArrayCount(measureCount);

    for(int index = 0; index < measureCount; index++)
    {
        setMeasureItemArray(VALUE_TYPE_ELECTRIC, index, 0);
        setMeasureItemArray(VALUE_TYPE_PHYSICAL, index, 0);
        setMeasureItemArray(VALUE_TYPE_OUTPUT, index, pCalibrator->measureValue());
    }

    setMeasure(VALUE_TYPE_ELECTRIC, 0);
    setMeasure(VALUE_TYPE_PHYSICAL, 0);
    setMeasure(VALUE_TYPE_OUTPUT, pCalibrator->measureValue());

    // limits
    //
    setLowLimit(VALUE_TYPE_ELECTRIC, 50);
    setHighLimit(VALUE_TYPE_ELECTRIC, 100);
    setUnit(VALUE_TYPE_ELECTRIC, CalibratorUnit[pCalibrator->sourceUnit()]);

    setLowLimit(VALUE_TYPE_PHYSICAL, 0);
    setHighLimit(VALUE_TYPE_PHYSICAL, 100);
    setUnit(VALUE_TYPE_PHYSICAL, "°С");

    setLowLimit(VALUE_TYPE_OUTPUT, 4);
    setHighLimit(VALUE_TYPE_OUTPUT, 20);
    setUnit(VALUE_TYPE_OUTPUT, CalibratorUnit[pCalibrator->measureUnit()]);

    setHasOutput(true);
    setAdjustment(0);

    // calc errors
    //
    setErrorInput(ERROR_TYPE_ABSOLUTE, 0);
    setErrorInput(ERROR_TYPE_REDUCE, 0);

    setErrorOutput(ERROR_TYPE_ABSOLUTE, 0);
    setErrorOutput(ERROR_TYPE_REDUCE, 0);

    setErrorLimit(ERROR_TYPE_ABSOLUTE, 0);
    setErrorLimit(ERROR_TYPE_REDUCE, 0);

    setErrorPrecision(ERROR_TYPE_ABSOLUTE, 0);
    setErrorPrecision(ERROR_TYPE_REDUCE, 2);

    setAdditionalValue(ADDITIONAL_VALUE_MEASURE_MIN, 0);
    setAdditionalValue(ADDITIONAL_VALUE_MEASURE_MAX, 0);
    setAdditionalValue(ADDITIONAL_VALUE_SYSTEM_ERROR, 0);
    setAdditionalValue(ADDITIONAL_VALUE_MSE, 0);
    setAdditionalValue(ADDITIONAL_VALUE_LOW_BORDER, 0);
    setAdditionalValue(ADDITIONAL_VALUE_HIGH_BORDER, 0);
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

void LinearetyMeasureItem::updateMeasureArray(int valueType, MeasureItem* pMeasure)
{
    if (pMeasure == nullptr)
    {
        return;
    }

    int measureType = pMeasure->measureType();
    if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    LinearetyMeasureItem* pLinearetyMeasureItem = static_cast <LinearetyMeasureItem*> (pMeasure);

    m_measureArrayCount = pLinearetyMeasureItem->measureArrayCount();

    for(int m = 0; m < MEASUREMENT_IN_POINT; m++)
    {
        m_measureArray[valueType][m] = pLinearetyMeasureItem->measureItemArray(valueType, m);
    }
}

// -------------------------------------------------------------------------------------------------------------------

void LinearetyMeasureItem::updateAdditionalValue(MeasureItem* pMeasure)
{
    if (pMeasure == nullptr)
    {
        return;
    }

    int measureType = pMeasure->measureType();
    if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
    {
        return;
    }

    LinearetyMeasureItem* pLinearetyMeasureItem = static_cast <LinearetyMeasureItem*> (pMeasure);

    for(int a = 0; a < ADDITIONAL_VALUE_COUNT; a++)
    {
        m_additionalValue[a] = pLinearetyMeasureItem->additionalValue(a);
    }
}

// -------------------------------------------------------------------------------------------------------------------

LinearetyMeasureItem& LinearetyMeasureItem::operator=(const LinearetyMeasureItem& from)
{
    m_appSignalID = from.m_appSignalID;
    m_customAppSignalID = from.m_customAppSignalID;
    m_caption = from.m_caption;

    m_position = from.m_position;

    for(int v = 0; v < VALUE_TYPE_COUNT; v++)
    {
        m_nominal[v] = from.m_nominal[v];
        m_measure[v] = from.m_measure[v];

        for(int m = 0; m < MEASUREMENT_IN_POINT; m++)
        {
            m_measureArray[v][m] = from.m_measureArray[v][m];
        }

        m_lowLimit[v] = from.m_lowLimit[v];;
        m_highLimit[v] = from.m_highLimit[v];;
        m_unit[v] = from.m_unit[v];;

        m_valuePrecision[v] = from.m_valuePrecision[v];
    }

    m_percent = from.m_percent;
    m_measureArrayCount = from.m_measureArrayCount;

    m_hasOutput = from.m_hasOutput;
    m_adjustment = from.m_adjustment;

    for(int e = 0; e < ERROR_TYPE_COUNT; e++)
    {
        m_errorInput[e] = from.m_errorInput[e];;
        m_errorOutput[e] = from.m_errorOutput[e];;
        m_errorLimit[e] = from.m_errorLimit[e];;

        m_errorPrecision[e] = from.m_errorPrecision[e];;
    }

    m_additionalValueCount = from.m_additionalValueCount;

    for(int a = 0; a < ADDITIONAL_VALUE_COUNT; a++)
    {
        m_additionalValue[a] = from.m_additionalValue[a];;
    }

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ComparatorMeasureItem::ComparatorMeasureItem() :
    MeasureItem(MEASURE_TYPE_COMPARATOR)
{
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorMeasureItem::ComparatorMeasureItem(Calibrator* pCalibrator)
{
    if (pCalibrator == nullptr)
    {
        return;
    }

    setMeasureType(MEASURE_TYPE_COMPARATOR);

    // features
    //
    setStrID("#IDMPS");
    setExtStrID("IDMPS");
    setName("This is signal of the block MPS");

    position().setCaseNo(0);
    position().setCaseType("CASE-1");
    position().setChannel(0);
    position().setBlock(0);
    position().setSubblock(0);
    position().setEntry(0);
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorMeasureItem::updateHysteresis(MeasureItem* pMeasure)
{
    if (pMeasure == nullptr)
    {
        return;
    }
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------


ComplexComparatorMeasureItem::ComplexComparatorMeasureItem() :
    MeasureItem(MEASURE_TYPE_COMPLEX_COMPARATOR)
{
}

// -------------------------------------------------------------------------------------------------------------------

ComplexComparatorMeasureItem::ComplexComparatorMeasureItem(Calibrator* pMainCalibrator, Calibrator* pSubCalibrator)
{
    if (pMainCalibrator == nullptr || pSubCalibrator == nullptr)
    {
        return;
    }

    setMeasureType(MEASURE_TYPE_COMPLEX_COMPARATOR);

    // features
    //
    setStrID("#IDMPS");
    setExtStrID("IDMPS");
    setName("This is signal of the block MPS");

    position().setCaseNo(0);
    position().setCaseType("CASE-1");
    position().setChannel(0);
    position().setBlock(0);
    position().setSubblock(0);
    position().setEntry(0);
}

// -------------------------------------------------------------------------------------------------------------------

void ComplexComparatorMeasureItem::updateHysteresis(MeasureItem* pMeasure)
{
    if (pMeasure == nullptr)
    {
        return;
    }
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

