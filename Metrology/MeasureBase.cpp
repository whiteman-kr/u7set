#include "MeasureBase.h"

#include "Database.h"

// -------------------------------------------------------------------------------------------------------------------

MeasureBase::MeasureBase(QObject *parent) :
    QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------
// each measurement is located in several tables,
// firstly read data from the main table, and additional sub tables in memory
// later update the data in the main table from sub tables
//
int MeasureBase::load(int measureType)
{
    if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
    {
        return -1;
    }

    if (thepDatabase == nullptr)
    {
        return -1;
    }

    m_measureType = measureType;

    struct tableData
    {
        int             objectType;
        MeasureItem*    pMeasureItem;
        int             count;
    };

    QVector<tableData> tableList;

    // read all table of current MEASURE_TYPE in memory
    //
    for(int objectType = 0; objectType < SQL_TABLE_COUNT; objectType++)
    {
        if (SqlTableByMeasureType[objectType] == measureType)
        {
            SqlTable* table = thepDatabase->openTable(objectType);
            if (table != nullptr)
            {
                tableData data;

                data.objectType = objectType;
                data.pMeasureItem = nullptr;
                data.count = table->recordCount();

                switch(measureType)
                {
                    case MEASURE_TYPE_LINEARITY:            data.pMeasureItem = new LinearetyMeasureItem[data.count];           break;
                    case MEASURE_TYPE_COMPARATOR:           data.pMeasureItem = new ComparatorMeasureItem[data.count];          break;
                    case MEASURE_TYPE_COMPLEX_COMPARATOR:   data.pMeasureItem = new ComplexComparatorMeasureItem[data.count];   break;
                    default:                                assert(0);                                                          break;
                }

                if (data.pMeasureItem != nullptr)
                {
                    if (table->read(data.pMeasureItem) == data.count)
                    {
                        tableList.append( data );
                    }
                }

                table->close();
            }
        }
    }

    // if tables of current MEASURE_TYPE is not exist, then exit
    //
    int tableCount = tableList.count();
    if (tableCount == 0)
    {
        return 0;
    }

    // get main table, afterwards from sub tables update data in main table
    // addpend data-measurement in MeasureBase
    //

    tableData mainTable = tableList[SQL_TABLE_MEASURE_MAIN];

    for(int mainIndex = 0; mainIndex < mainTable.count; mainIndex++)
    {
        MeasureItem* pMainMeasure = mainTable.pMeasureItem->at(mainIndex);
        if (pMainMeasure == nullptr)
        {
            continue;
        }

        for(int sub_table = 1; sub_table < tableCount; sub_table++)
        {
            tableData subTable = tableList[sub_table];

            for(int subIndex = 0; subIndex < subTable.count; subIndex++)
            {
                MeasureItem* pSubMeasure = subTable.pMeasureItem->at(subIndex);
                if (pSubMeasure == nullptr)
                {
                    continue;
                }

                // update main measurement from sub measurement
                //
                if (pMainMeasure->measureID() == pSubMeasure->measureID())
                {
                    switch(subTable.objectType)
                    {
                        case SQL_TABLE_LINEARETY_20_EL:                 static_cast<LinearetyMeasureItem*>(pMainMeasure)->updateMeasureArray(VALUE_TYPE_ELECTRIC, pSubMeasure); break;
                        case SQL_TABLE_LINEARETY_20_PH:                 static_cast<LinearetyMeasureItem*>(pMainMeasure)->updateMeasureArray(VALUE_TYPE_PHYSICAL, pSubMeasure); break;
                        case SQL_TABLE_LINEARETY_ADD_VAL:               static_cast<LinearetyMeasureItem*>(pMainMeasure)->updateAdditionalValue(pSubMeasure);                   break;
                        case SQL_TABLE_COMPARATOR_HYSTERESIS:           static_cast<ComparatorMeasureItem*>(pMainMeasure)->updateHysteresis(pSubMeasure);                       break;
                        case SQL_TABLE_COMPLEX_COMPARATOR_HYSTERESIS:   static_cast<ComplexComparatorMeasureItem*>(pMainMeasure)->updateHysteresis(pSubMeasure);                break;
                    }

                    break;
                }
            }
        }
    }

    // append measuremets to MeasureBase from updated main table
    //
    m_mutex.lock();

        for(int index = 0; index < mainTable.count; index++)
        {
            MeasureItem* pMeasureTable = mainTable.pMeasureItem->at(index);
            if (pMeasureTable == nullptr)
            {
                continue;
            }

            MeasureItem* pMeasureAppend = nullptr;

            switch(measureType)
            {
                case MEASURE_TYPE_LINEARITY:            pMeasureAppend  = new LinearetyMeasureItem;         break;
                case MEASURE_TYPE_COMPARATOR:           pMeasureAppend  = new ComparatorMeasureItem;        break;
                case MEASURE_TYPE_COMPLEX_COMPARATOR:   pMeasureAppend  = new ComplexComparatorMeasureItem; break;
                default:                                assert(0);                                          break;
            }

            if (pMeasureAppend == nullptr)
            {
                continue;
            }

            *pMeasureAppend = *pMeasureTable;

            m_measureList.append(pMeasureAppend);
        }

    m_mutex.unlock();

    // if measurement is nonexistentin in main table, but exist in sub table,
    // need remove this measurement in sub table
    // remove nonexistent indexes-measurements-ID in sub tables
    //
    for(int sub_table = 1; sub_table < tableCount; sub_table++)
    {
        tableData subTable = tableList[sub_table];

        QVector<int> removeKeyList;

        for(int subIndex = 0; subIndex < subTable.count; subIndex++)
        {
            MeasureItem* pSubMeasure = subTable.pMeasureItem->at(subIndex);
            if (pSubMeasure == nullptr)
            {
                continue;
            }

            bool foundMeasure = false;

            for(int mainIndex = 0; mainIndex < mainTable.count; mainIndex++)
            {
                MeasureItem* pMainMeasure = mainTable.pMeasureItem->at(mainIndex);
                if (pMainMeasure == nullptr)
                {
                    continue;
                }

                if (pMainMeasure->measureID() == pSubMeasure->measureID())
                {
                    foundMeasure = true;
                    break;
                }
            }

            // if measurement is not found in main table then need remove it in sub table
            //
            if (foundMeasure == false)
            {
                removeKeyList.append(pSubMeasure->measureID());
            }
        }

        // remove unnecessary measurement from sub table
        //
        SqlTable* table = thepDatabase->openTable(subTable.objectType);
        if (table != nullptr)
        {
            table->remove(removeKeyList.data(), removeKeyList.count());
            table->close();
        }
    }

    // remove table data from memory
    //
    for(int t = 0; t < tableCount; t++)
    {
        tableData table = tableList[t];

        if (table.pMeasureItem == nullptr)
        {
            continue;
        }

        switch(measureType)
        {
            case MEASURE_TYPE_LINEARITY:            delete [] static_cast<LinearetyMeasureItem*> (table.pMeasureItem);          break;
            case MEASURE_TYPE_COMPARATOR:           delete [] static_cast<ComparatorMeasureItem*> (table.pMeasureItem);         break;
            case MEASURE_TYPE_COMPLEX_COMPARATOR:   delete [] static_cast<ComplexComparatorMeasureItem*> (table.pMeasureItem);  break;
            default:                                assert(0);                                                                  break;
        }
    }


    return count();
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

    m_mutex.lock();

        m_measureList.append(pMeasure);

    m_mutex.unlock();

    return count();
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

bool MeasureBase::remove(int index)
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
            if (pMeasure != nullptr)
            {
                delete pMeasure;
            }
        }

        m_measureList.clear();

    m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------


