#include "MeasureBase.h"

// -------------------------------------------------------------------------------------------------------------------

MeasureBase::MeasureBase(QObject *parent) :
    QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

int MeasureBase::count() const
{
    int count = 0;

    m_mutex.lock();

        count = m_measureList.count();

    m_mutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

int MeasureBase::append(MeasureItem* pMeasure)
{
    if (pMeasure == nullptr)
    {
        return -1;
    }

    int type = pMeasure->measureType();
    if (type < 0 || type >= MEASURE_TYPE_COUNT)
    {
        return -1;
    }

    int index = -1;

    switch (type)
    {
        case MEASURE_TYPE_LINEARITY:            index = formatLinearityMeasure(pMeasure);           break;
        case MEASURE_TYPE_COMPARATOR:           index = formatComparatorMeasure(pMeasure);          break;
        case MEASURE_TYPE_COMPLEX_COMPARATOR:   index = formatComplexComparatorMeasure(pMeasure);   break;
        default:                                assert(0);                                          break;
    }

    if (index == -1)
    {
        return -1;
    }

    m_mutex.lock();

        m_measureList.append(pMeasure);

    m_mutex.unlock();

    return pMeasure->baseIndex();
}

// -------------------------------------------------------------------------------------------------------------------

MeasureItem* MeasureBase::at(int index) const
{
    if (index < 0 || index >= count())
    {
        return nullptr;
    }

    MeasureItem* pMeasure = nullptr;

    m_mutex.lock();

        pMeasure = m_measureList.at(index);

    m_mutex.unlock();

    return pMeasure;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureBase::removeAt(int index)
{
    if (index < 0 || index >= count())
    {
        return false;
    }

    m_mutex.lock();

        MeasureItem* pMeasure = m_measureList.at(index);
        if (pMeasure != nullptr)
        {
            delete pMeasure;
        }

        m_measureList.removeAt(index);

    m_mutex.unlock();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureBase::clear()
{
    m_mutex.lock();

        int count = m_measureList.count();
        for(int m = 0; m < count; m++)
        {
            MeasureItem* pMeasure = m_measureList.at(m) ;
            if (pMeasure == nullptr)
            {
                continue;
            }

            delete pMeasure;
        }

        m_measureList.clear();

    m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int MeasureBase::formatLinearityMeasure(MeasureItem* pMeasure)
{
    if (pMeasure == nullptr)
    {
        return -1;
    }

    if (pMeasure->measureType() != MEASURE_TYPE_LINEARITY)
    {
        return -1;
    }

    LinearetyMeasureItem* m = static_cast<LinearetyMeasureItem*> (pMeasure);
    if (m == nullptr)
    {
        return -1;
    }

    m->setBaseIndex(count());

    // features
    //
    m->setStrID("#IDMPS");
    m->setExtStrID("IDMPS");
    m->setName("This is signal of the block MPS");

    m->position().setCaseNo(0);
    m->position().setCaseType("CASE-1");
    m->position().setChannel(0);
    m->position().setBlock(0);
    m->position().setSubblock(0);
    m->position().setEntry(0);

    m->setHasOutput(true);

    m->setLowLimit(VALUE_TYPE_ELECTRIC, 50);
    m->setHighLimit(VALUE_TYPE_ELECTRIC, 100);

    m->setLowLimit(VALUE_TYPE_PHYSICAL, 0);
    m->setHighLimit(VALUE_TYPE_PHYSICAL, 100);

    m->setLowLimit(VALUE_TYPE_OUTPUT, 4);
    m->setHighLimit(VALUE_TYPE_OUTPUT, 20);

    m->setUnit(VALUE_TYPE_ELECTRIC, "Ohm");
    m->setUnit(VALUE_TYPE_PHYSICAL, "°С");
    m->setUnit(VALUE_TYPE_OUTPUT, "mA");

    m->setValuePrecision(VALUE_TYPE_ELECTRIC, 3);
    m->setValuePrecision(VALUE_TYPE_PHYSICAL, 2);
    m->setValuePrecision(VALUE_TYPE_OUTPUT, 3);

    m->setAdjustment(0);

    // nominal
    //
    m->setNominal(VALUE_TYPE_ELECTRIC, 0);
    m->setNominal(VALUE_TYPE_PHYSICAL, 0);
    m->setNominal(VALUE_TYPE_OUTPUT, 0);

    m->setPercent(0);

    // measure
    //
    m->setMeasureArrayCount(0);

    for(int index = 0; index < MEASUREMENT_IN_POINT; index++)
    {
        m->setMeasureItemArray(VALUE_TYPE_ELECTRIC, index, 0);
        m->setMeasureItemArray(VALUE_TYPE_PHYSICAL, index, 0);
        m->setMeasureItemArray(VALUE_TYPE_OUTPUT, index, 0);
    }

    m->setMeasure(VALUE_TYPE_ELECTRIC, 0);
    m->setMeasure(VALUE_TYPE_PHYSICAL, 0);
    m->setMeasure(VALUE_TYPE_OUTPUT, 0);

    // calc errors
    //
    m->setErrorInput(ERROR_TYPE_ABSOLUTE, 0);
    m->setErrorInput(ERROR_TYPE_REDUCE, 0);

    m->setErrorOutput(ERROR_TYPE_ABSOLUTE, 0);
    m->setErrorOutput(ERROR_TYPE_REDUCE, 0);

    m->setErrorLimit(ERROR_TYPE_ABSOLUTE, 0);
    m->setErrorLimit(ERROR_TYPE_REDUCE, 0);

    m->setErrorPrecision(ERROR_TYPE_ABSOLUTE, 0);
    m->setErrorPrecision(ERROR_TYPE_REDUCE, 2);

    m->setErrorAddional(ADDITIONAL_ERROR_SYSTEM, 0);
    m->setErrorAddional(ADDITIONAL_ERROR_MSE, 0);
    m->setErrorAddional(ADDITIONAL_ERROR_LOW_BORDER, 0);
    m->setErrorAddional(ADDITIONAL_ERROR_HIGH_BORDER, 0);


    return m->baseIndex();
}

// -------------------------------------------------------------------------------------------------------------------

int MeasureBase::formatComparatorMeasure(MeasureItem* pMeasure)
{
    if (pMeasure == nullptr)
    {
        return -1;
    }

    if (pMeasure->measureType() != MEASURE_TYPE_COMPARATOR)
    {
        return -1;
    }

    MeasureItem* m = static_cast<LinearetyMeasureItem*> (pMeasure);
    if (m == nullptr)
    {
        return -1;
    }

    m->setBaseIndex(count());

    return m->baseIndex();
}

// -------------------------------------------------------------------------------------------------------------------

int MeasureBase::formatComplexComparatorMeasure(MeasureItem* pMeasure)
{
    if (pMeasure == nullptr)
    {
        return -1;
    }

    if (pMeasure->measureType() != MEASURE_TYPE_COMPLEX_COMPARATOR)
    {
        return -1;
    }

    MeasureItem* m = static_cast<LinearetyMeasureItem*> (pMeasure);
    if (m == nullptr)
    {
        return -1;
    }

    m->setBaseIndex(count());

    return m->baseIndex();
}

// -------------------------------------------------------------------------------------------------------------------
