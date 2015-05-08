#include "Database.h"

#include <assert.h>
#include <QMessageBox>
#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------

Database* thePtrDB = nullptr;

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SqlFieldBase::SqlFieldBase()
{
}

// -------------------------------------------------------------------------------------------------------------------

int SqlFieldBase::init(int objectType, int)
{
    switch(objectType)
    {
        case SQL_TABLE_DATABASE_INFO:

            append("ID",                            QVariant::Int);
            append("ObjectID",						QVariant::Int);
            append("Name",							QVariant::String, 256);
            append("Version",						QVariant::Int);

            break;

        case SQL_TABLE_HISTORY:

            append("ObjectID",						QVariant::Int);
            append("Version",						QVariant::Int);
            append("Event",							QVariant::String, 256);
            append("Time",                          QVariant::String, 64);

            break;

        case SQL_TABLE_LINEARETY:

            append("ObjectID",						QVariant::Int);
            append("MeasureID",						QVariant::Int);

            append("Filter",						QVariant::Bool);

            append("StrID",                         QVariant::String, 64);
            append("ExtStrID",						QVariant::String, 64);
            append("Name",                          QVariant::String, 256);

            append("DevStrID",						QVariant::String, 256);
            append("CaseNo",                        QVariant::Int);
            append("CaseType",						QVariant::String, 64);
            append("Channel",						QVariant::Int);
            append("Subblock",						QVariant::Int);
            append("Block",                         QVariant::Int);
            append("Entry",                         QVariant::Int);

            append("NominalElectricValue",			QVariant::Double);
            append("NominalPhysicalValue",			QVariant::Double);
            append("NominalOutputValue",            QVariant::Double);

            append("PrecentFormRange",				QVariant::Double);

            append("MeasureElectricValue",			QVariant::Double);
            append("MeasurePhysicalValue",			QVariant::Double);
            append("MeasureOutputValue",            QVariant::Double);

            append("LowElectricRange",				QVariant::Double);
            append("HighElectricRange",             QVariant::Double);
            append("ElectricUnit",					QVariant::String, 32);

            append("LowPhysicalRange",				QVariant::Double);
            append("HighPhysicalRange",             QVariant::Double);
            append("PhysicalUnit",					QVariant::String, 32);

            append("LowOutputRange",				QVariant::Double);
            append("HighOutputRange",               QVariant::Double);
            append("OutputUnit",					QVariant::String, 32);

            append("PrecisionElectric",             QVariant::Int);
            append("PrecisionPhysical",             QVariant::Int);
            append("PrecisionOutput",               QVariant::Int);

            append("HasOutput",						QVariant::Bool);
            append("Adjustment",					QVariant::Double);

            append("ErrorInputAbsolute",			QVariant::Double);
            append("ErrorInputReduce",              QVariant::Double);
            append("ErrorOutputAbsolute",			QVariant::Double);
            append("ErrorOutputReduce",             QVariant::Double);
            append("ErrorLimitAbsolute",			QVariant::Double);
            append("ErrorLimitReduce",              QVariant::Double);

            append("ErrorPrecisionAbsolute",        QVariant::Int);
            append("ErrorPrecisionReduce",          QVariant::Int);

            append("MeasureTime",					QVariant::String, 64);

            break;

        case SQL_TABLE_LINEARETY_20_EL:
        case SQL_TABLE_LINEARETY_20_PH:

            append("ObjectID",						QVariant::Int);
            append("MeasureID",						QVariant::Int);

            append(QString("MeasurementCount"),     QVariant::Int);

            append(QString("Measurement0"),         QVariant::Double);
            append(QString("Measurement1"),         QVariant::Double);
            append(QString("Measurement2"),         QVariant::Double);
            append(QString("Measurement3"),         QVariant::Double);
            append(QString("Measurement4"),         QVariant::Double);
            append(QString("Measurement5"),         QVariant::Double);
            append(QString("Measurement6"),         QVariant::Double);
            append(QString("Measurement7"),         QVariant::Double);
            append(QString("Measurement8"),         QVariant::Double);
            append(QString("Measurement9"),         QVariant::Double);
            append(QString("Measurement10"),        QVariant::Double);
            append(QString("Measurement11"),        QVariant::Double);
            append(QString("Measurement12"),        QVariant::Double);
            append(QString("Measurement13"),        QVariant::Double);
            append(QString("Measurement14"),        QVariant::Double);
            append(QString("Measurement15"),        QVariant::Double);
            append(QString("Measurement16"),        QVariant::Double);
            append(QString("Measurement17"),        QVariant::Double);
            append(QString("Measurement18"),        QVariant::Double);
            append(QString("Measurement19"),        QVariant::Double);

            break;

        case SQL_TABLE_LINEARETY_ADD_VAL:

            append("ObjectID",						QVariant::Int);
            append("MeasureID",						QVariant::Int);

            append(QString("ValueCount"),           QVariant::Int);

            append(QString("Value0"),               QVariant::Double);
            append(QString("Value1"),               QVariant::Double);
            append(QString("Value2"),               QVariant::Double);
            append(QString("Value3"),               QVariant::Double);
            append(QString("Value4"),               QVariant::Double);
            append(QString("Value5"),               QVariant::Double);
            append(QString("Value6"),               QVariant::Double);
            append(QString("Value7"),               QVariant::Double);
            append(QString("Value8"),               QVariant::Double);
            append(QString("Value9"),               QVariant::Double);
            append(QString("Value10"),              QVariant::Double);
            append(QString("Value11"),              QVariant::Double);
            append(QString("Value12"),              QVariant::Double);
            append(QString("Value13"),              QVariant::Double);
            append(QString("Value14"),              QVariant::Double);
            append(QString("Value15"),              QVariant::Double);

            break;

        case SQL_TABLE_LINEARETY_POINT:

            append("ObjectID",						QVariant::Int);
            append("PointID",                       QVariant::Int);

            append("PercentValue",					QVariant::Double);

            break;

        case SQL_TABLE_COMPARATOR:

            append("ObjectID",						QVariant::Int);
            append("MeasureID",						QVariant::Int);

            break;

        case SQL_TABLE_COMPARATOR_HYSTERESIS:

            append("ObjectID",						QVariant::Int);
            append("MeasureID",						QVariant::Int);

            break;


        case SQL_TABLE_COMPLEX_COMPARATOR:

            append("ObjectID",						QVariant::Int);
            append("MeasureID",						QVariant::Int);

            break;

        case SQL_TABLE_COMPLEX_COMPARATOR_HYSTERESIS:

            append("ObjectID",						QVariant::Int);
            append("MeasureID",						QVariant::Int);

            break;

        case SQL_TABLE_COMPLEX_COMPARATOR_POINT:

            append("ObjectID",						QVariant::Int);
            append("PointID",                       QVariant::Int);

            break;

        case SQL_TABLE_COMPLEX_COMPARATOR_SIGNAL:

            append("ObjectID",						QVariant::Int);
            append("SignalID",						QVariant::Int);

            break;

        case SQL_TABLE_OUTPUT_SIGNAL:

            append("ObjectID",						QVariant::Int);
            append("SignalID",						QVariant::Int);

            break;

        case SQL_TABLE_REPORT_HEADER:

            append("ObjectID",						QVariant::Int);
            append("ReportID",						QVariant::Int);


            append("DocumentTitle",                 QVariant::String, 256);
            append("ReportTitle",                   QVariant::String, 256);
            append("Date",                          QVariant::String, 32);
            append("TableTitle",                    QVariant::String, 256);
            append("Conclusion",                    QVariant::String, 256);

            append("Temperature",                   QVariant::Double);
            append("Pressure",                      QVariant::Double);
            append("Humidity",                      QVariant::Double);
            append("Voltage",                       QVariant::Double);
            append("Frequency",                     QVariant::Double);

            append("Calibrator0",                   QVariant::String, 64);
            append("Calibrator1",                   QVariant::String, 64);
            append("Calibrator2",                   QVariant::String, 64);
            append("Calibrator3",                   QVariant::String, 64);
            append("Calibrator4",                   QVariant::String, 64);
            append("Calibrator5",                   QVariant::String, 64);

            append("LinkObjectID",                  QVariant::Int);
            append("ReportFile",                    QVariant::String, 256);

            append("Param",                         QVariant::Int);

            break;

        default:
            assert(0);
            break;
    }

    int fieldCount = count();
    assert(fieldCount);

    return fieldCount;
}

// -------------------------------------------------------------------------------------------------------------------


void SqlFieldBase::append(const QSqlField& field)
{
    QSqlRecord::append(field);
}

// -------------------------------------------------------------------------------------------------------------------

void SqlFieldBase::append(QString name, QVariant::Type type, int length)
{
    if (name.isEmpty() == true)
    {
        return;
    }

    if (type == QVariant::Invalid)
    {
        return;
    }

    QSqlField field(name, type);

    if (type == QVariant::Double )
    {
        field.setPrecision(9);
    }

    if (type == QVariant::String )
    {
        field.setLength(length);
    }

    append(field);
}

// -------------------------------------------------------------------------------------------------------------------

QString SqlFieldBase::extFieldName(int index)
{
    if (index < 0 || index >= count())
    {
        return QString();
    }

    QSqlField f = field(index);

    QString result;

    switch(f.type())
    {
        case QVariant::Bool:	result = QString("%1 BOOL").arg( f.name() );                                break;
        case QVariant::Int:     result = QString("%1 INTEGER").arg( f.name() );                             break;
        case QVariant::Double:	result = QString("%1 DOUBLE(0, %2)").arg( f.name() ).arg(f.precision());    break;
        case QVariant::String:	result = QString("%1 VARCHAR(%2)").arg( f.name()).arg(f.length());          break;
        default:                result = "";                                                                break;
    }

    return result;
}


// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SqlObjectInfo::SqlObjectInfo()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool SqlObjectInfo::init(int objectType)
{
    if (objectType < 0 || objectType >= SQL_TABLE_COUNT)
    {
        return false;
    }

    m_objectType = objectType;
    m_objectID = SqlObjectID[objectType];
    m_name = SqlTabletName[objectType];
    m_version = SqlTableVersion[objectType];

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void SqlObjectInfo::clear()
{
    m_objectType = SQL_TABLE_UNKNONW;
    m_objectID = SQL_OBJECT_ID_UNKNONW;
    m_name.clear();
    m_version = SQL_TABLE_VER_UNKNONW;
}

// -------------------------------------------------------------------------------------------------------------------

SqlObjectInfo& SqlObjectInfo::operator=(SqlObjectInfo& from)
{
    m_objectType = from.m_objectType;
    m_objectID = from.m_objectID;
    m_name = from.m_name;
    m_version = from.m_version;

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SqlHistoryDatabase::SqlHistoryDatabase()
{
}

// -------------------------------------------------------------------------------------------------------------------

SqlHistoryDatabase::SqlHistoryDatabase(int objectID, int version, QString event,  QString time)
{
    m_objectID = objectID;
    m_version = version;
    m_event = event;
    m_time = time;
}

// -------------------------------------------------------------------------------------------------------------------

SqlHistoryDatabase& SqlHistoryDatabase::operator=(SqlHistoryDatabase& from)
{
    m_objectID = from.m_objectID;
    m_version = from.m_version;
    m_event = from.m_event;
    m_time = from.m_time;

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SqlTable::SqlTable()
{
}

// -------------------------------------------------------------------------------------------------------------------

int SqlTable::recordCount()
{
    if (isOpen() == false)
    {
        return 0;
    }

    int type = m_info.objectType();
    if (type < 0 || type >= SQL_TABLE_COUNT)
    {
        return 0;
    }

    QSqlQuery query(QString("SELECT count(*) FROM %1").arg(m_info.name()));
    if (query.next() == false)
    {
        return 0;
    }

    return query.value(0).toInt();
}

// -------------------------------------------------------------------------------------------------------------------

int SqlTable::lastKey()
{
    if (isOpen() == false)
    {
        return SQL_INVALID_KEY;
    }

    int type = m_info.objectType();
    if (type < 0 || type >= SQL_TABLE_COUNT)
    {
        return SQL_INVALID_KEY;
    }

    QSqlQuery query(QString("SELECT max(%1) FROM %2").arg(m_fieldBase.field(SQL_FIELD_KEY).name()).arg(m_info.name()));
    if (query.next() == false)
    {
        return SQL_INVALID_KEY;
    }

    return query.value(0).toInt();
}

// -------------------------------------------------------------------------------------------------------------------

bool SqlTable::init(int objectType, QSqlDatabase* pDatabase)
{
    if (objectType < 0 || objectType >= SQL_TABLE_COUNT)
    {
        return false;
    }

    if (pDatabase == nullptr)
    {
        return false;
    }

    if (m_info.init(objectType) == false)
    {
        return false;
    }

    m_pDatabase = pDatabase;

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool SqlTable::isExist()
{
    if (m_pDatabase == nullptr)
    {
        return false;
    }

    if (m_pDatabase->isOpen() == false)
    {
        return false;
    }

    int type = m_info.objectType();
    if (type < 0 || type >= SQL_TABLE_COUNT)
    {
        return false;
    }

    bool tableIsExist = false;

    int existTableCount = m_pDatabase->tables().count();
    for(int et = 0; et < existTableCount; et++)
    {
        if ( m_pDatabase->tables().at(et).compare(SqlTabletName[type]) == 0)
        {
            tableIsExist = true;
            break;
        }
    }

    return tableIsExist;
}


// -------------------------------------------------------------------------------------------------------------------

bool SqlTable::open()
{
    if (isExist() == false)
    {
        return false;
    }

    if (m_fieldBase.init(m_info.objectType(), m_info.version()) == 0)
    {
        return false;
    }

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void SqlTable::close()
{
    m_fieldBase.clear();
}

// -------------------------------------------------------------------------------------------------------------------

bool SqlTable::create()
{
    if (isExist() == true)
    {
        return false;
    }

    if (m_fieldBase.init(m_info.objectType(), m_info.version()) == 0)
    {
        return false;
    }

    QSqlQuery query;

    QString request = QString("CREATE TABLE if not exists %1 (").arg(m_info.name());

    int filedCount = m_fieldBase.count();
    for(int field = 0; field < filedCount; field++ )
    {
        request.append( m_fieldBase.extFieldName(field) );

        if (field == SQL_FIELD_KEY)
        {
            request.append( " PRIMARY KEY NOT NULL" );

            switch(m_info.objectType())
            {
                case SQL_TABLE_LINEARETY_20_EL:
                case SQL_TABLE_LINEARETY_20_PH:
                case SQL_TABLE_LINEARETY_ADD_VAL:
                    request.append(QString(" REFERENCES %1(MeasureID) ON DELETE CASCADE").arg(SqlTabletName[SQL_TABLE_LINEARETY]));
                    break;
            }
        }

        if (field != filedCount - 1)
        {
            request.append(", ");
        }
    }



    request.append(" );");

    return query.exec(request);
}

// -------------------------------------------------------------------------------------------------------------------

bool SqlTable::drop()
{
    QSqlQuery query;
    if (query.exec(QString("DROP TABLE %1").arg(m_info.name())) == false)
    {
        return false;
    }

    close();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool SqlTable::clear()
{
    if (isOpen() == false)
    {
        return false;
    }

    QSqlQuery query;

    if (query.exec("BEGIN TRANSACTION") == false)
    {
        return false;
    }

    if (query.exec(QString("DELETE FROM %1").arg(m_info.name())) == false)
    {
        return false;
    }

    if (query.exec("COMMIT") == false)
    {
        return false;
    }

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

int SqlTable::read(void* pRecord, int* key, int keyCount)
{
    if (isOpen() == false)
    {
        return 0;
    }

    if (pRecord == nullptr)
    {
        return 0;
    }

    // create request
    //
    QString request = QString("SELECT * FROM %1").arg(m_info.name());

    if (key != nullptr && keyCount != 0)
    {
        request.append(" WHERE ");
        QString keyFieldName = m_fieldBase.field(SQL_FIELD_KEY).name();

        for(int k = 0; k < keyCount; k++ )
        {
            request.append(QString("%1=%2").arg(keyFieldName).arg(key[k]));

            if (k != keyCount - 1)
            {
                request.append(" OR ");
            }
        }
    }

    // exec select
    //
    QSqlQuery query;
    if (query.exec(request) == false)
    {
        return 0;
    }

    int field = 0;
    int objectID = SQL_OBJECT_ID_UNKNONW;
    int readedCount = 0;

    // read data
    //
    while (query.next() == true)
    {
        field = 0;

        // check unique ID of table or view, zero field always is ObjectID
        //
        objectID = query.value(field++).toInt();
        if (objectID != m_info.objectID())
        {
            continue;
        }

        // read field's data
        //
        switch(m_info.objectType())
        {
            case SQL_TABLE_DATABASE_INFO:
                {
                    SqlObjectInfo* info = static_cast<SqlObjectInfo*> (pRecord) + readedCount;

                    info->setObjectID(query.value(field++).toInt());
                    info->setName(query.value(field++).toString());
                    info->setVersion(query.value(field++).toInt());
                }
                break;

            case SQL_TABLE_HISTORY:
                {
                    SqlHistoryDatabase* history = static_cast<SqlHistoryDatabase*> (pRecord) + readedCount;

                    history->setObjectID( objectID );
                    history->setVersion(query.value(field++).toInt());
                    history->setEvent(query.value(field++).toString());
                    history->setTime(query.value(field++).toString());
                }
                break;

            case SQL_TABLE_LINEARETY:
                {
                    LinearetyMeasureItem* measure = static_cast<LinearetyMeasureItem*> (pRecord) + readedCount;

                    measure->setMeasureID(query.value(field++).toInt());

                    measure->setFilter(query.value(field++).toBool());

                    measure->setStrID(query.value(field++).toString());
                    measure->setExtStrID(query.value(field++).toString());
                    measure->setName(query.value(field++).toString());

                    measure->position().setDeviceStrID(query.value(field++).toString());
                    measure->position().setCaseNo(query.value(field++).toInt());
                    measure->position().setCaseType(query.value(field++).toString());
                    measure->position().setChannel(query.value(field++).toInt());
                    measure->position().setSubblock(query.value(field++).toInt());
                    measure->position().setBlock(query.value(field++).toInt());
                    measure->position().setEntry(query.value(field++).toInt());

                    measure->setNominal(VALUE_TYPE_ELECTRIC, query.value(field++).toDouble());
                    measure->setNominal(VALUE_TYPE_PHYSICAL, query.value(field++).toDouble());
                    measure->setNominal(VALUE_TYPE_OUTPUT, query.value(field++).toDouble());

                    measure->setPercent(query.value(field++).toDouble());

                    measure->setMeasure(VALUE_TYPE_ELECTRIC, query.value(field++).toDouble());
                    measure->setMeasure(VALUE_TYPE_PHYSICAL, query.value(field++).toDouble());
                    measure->setMeasure(VALUE_TYPE_OUTPUT, query.value(field++).toDouble());

                    measure->setLowLimit(VALUE_TYPE_ELECTRIC, query.value(field++).toDouble());
                    measure->setHighLimit(VALUE_TYPE_ELECTRIC, query.value(field++).toDouble());
                    measure->setUnit(VALUE_TYPE_ELECTRIC, query.value(field++).toString());

                    measure->setLowLimit(VALUE_TYPE_PHYSICAL, query.value(field++).toDouble());
                    measure->setHighLimit(VALUE_TYPE_PHYSICAL, query.value(field++).toDouble());
                    measure->setUnit(VALUE_TYPE_PHYSICAL, query.value(field++).toString());

                    measure->setLowLimit(VALUE_TYPE_OUTPUT, query.value(field++).toDouble());
                    measure->setHighLimit(VALUE_TYPE_OUTPUT, query.value(field++).toDouble());
                    measure->setUnit(VALUE_TYPE_OUTPUT, query.value(field++).toString());

                    measure->setValuePrecision(VALUE_TYPE_ELECTRIC, query.value(field++).toInt());
                    measure->setValuePrecision(VALUE_TYPE_PHYSICAL, query.value(field++).toInt());
                    measure->setValuePrecision(VALUE_TYPE_OUTPUT, query.value(field++).toInt());

                    measure->setHasOutput(query.value(field++).toBool());
                    measure->setAdjustment(query.value(field++).toDouble());

                    measure->setErrorInput(ERROR_TYPE_ABSOLUTE, query.value(field++).toDouble());
                    measure->setErrorInput(ERROR_TYPE_REDUCE, query.value(field++).toDouble());
                    measure->setErrorOutput(ERROR_TYPE_ABSOLUTE, query.value(field++).toDouble());
                    measure->setErrorOutput(ERROR_TYPE_REDUCE, query.value(field++).toDouble());
                    measure->setErrorLimit(ERROR_TYPE_ABSOLUTE, query.value(field++).toDouble());
                    measure->setErrorLimit(ERROR_TYPE_REDUCE, query.value(field++).toDouble());

                    measure->setErrorPrecision(ERROR_TYPE_ABSOLUTE, query.value(field++).toInt());
                    measure->setErrorPrecision(ERROR_TYPE_REDUCE, query.value(field++).toInt());

                    measure->setMeasureTime( QDateTime::fromString( query.value(field++).toString(), MEASURE_TIME_FORMAT));
                }
                break;

            case SQL_TABLE_LINEARETY_20_EL:
            case SQL_TABLE_LINEARETY_20_PH:
                {
                    int valueType = VALUE_TYPE_UNKNOWN;

                    switch(m_info.objectType())
                    {
                        case SQL_TABLE_LINEARETY_20_EL:     valueType = VALUE_TYPE_ELECTRIC;    break;
                        case SQL_TABLE_LINEARETY_20_PH:     valueType = VALUE_TYPE_PHYSICAL;    break;
                        default:                            valueType = VALUE_TYPE_UNKNOWN;     break;
                    }

                    if (valueType == VALUE_TYPE_UNKNOWN)
                    {
                        break;
                    }

                    LinearetyMeasureItem* measure = static_cast<LinearetyMeasureItem*> (pRecord) + readedCount;

                    measure->setMeasureID(query.value(field++).toInt());

                    measure->setMeasureArrayCount(query.value(field++).toInt());

                    measure->setMeasureItemArray(valueType, 0, query.value(field++).toDouble());
                    measure->setMeasureItemArray(valueType, 1, query.value(field++).toDouble());
                    measure->setMeasureItemArray(valueType, 2, query.value(field++).toDouble());
                    measure->setMeasureItemArray(valueType, 3, query.value(field++).toDouble());
                    measure->setMeasureItemArray(valueType, 4, query.value(field++).toDouble());
                    measure->setMeasureItemArray(valueType, 5, query.value(field++).toDouble());
                    measure->setMeasureItemArray(valueType, 6, query.value(field++).toDouble());
                    measure->setMeasureItemArray(valueType, 7, query.value(field++).toDouble());
                    measure->setMeasureItemArray(valueType, 8, query.value(field++).toDouble());
                    measure->setMeasureItemArray(valueType, 9, query.value(field++).toDouble());
                    measure->setMeasureItemArray(valueType, 10, query.value(field++).toDouble());
                    measure->setMeasureItemArray(valueType, 11, query.value(field++).toDouble());
                    measure->setMeasureItemArray(valueType, 12, query.value(field++).toDouble());
                    measure->setMeasureItemArray(valueType, 13, query.value(field++).toDouble());
                    measure->setMeasureItemArray(valueType, 14, query.value(field++).toDouble());
                    measure->setMeasureItemArray(valueType, 15, query.value(field++).toDouble());
                    measure->setMeasureItemArray(valueType, 16, query.value(field++).toDouble());
                    measure->setMeasureItemArray(valueType, 17, query.value(field++).toDouble());
                    measure->setMeasureItemArray(valueType, 18, query.value(field++).toDouble());
                    measure->setMeasureItemArray(valueType, 19, query.value(field++).toDouble());
                }
                break;

            case SQL_TABLE_LINEARETY_ADD_VAL:
                {
                    LinearetyMeasureItem* measure = static_cast<LinearetyMeasureItem*> (pRecord) + readedCount;

                    measure->setMeasureID(query.value(field++).toInt());

                    measure->setAdditionalValueCount(query.value(field++).toInt());

                    measure->setAdditionalValue(ADDITIONAL_VALUE_MEASURE_MIN, query.value(field++).toDouble());
                    measure->setAdditionalValue(ADDITIONAL_VALUE_MEASURE_MAX, query.value(field++).toDouble());
                    measure->setAdditionalValue(ADDITIONAL_VALUE_SYSTEM_ERROR, query.value(field++).toDouble());
                    measure->setAdditionalValue(ADDITIONAL_VALUE_MSE, query.value(field++).toDouble());
                    measure->setAdditionalValue(ADDITIONAL_VALUE_LOW_BORDER, query.value(field++).toDouble());
                    measure->setAdditionalValue(ADDITIONAL_VALUE_HIGH_BORDER, query.value(field++).toDouble());
                }
                break;

            case SQL_TABLE_LINEARETY_POINT:
                {
                    LinearityPoint* point = static_cast<LinearityPoint*> (pRecord) + readedCount;

                    point->setPointID(query.value(field++).toInt());
                    point->setPercent(query.value(field++).toDouble());
                }
                break;

            case SQL_TABLE_COMPARATOR:
                {
                }
                break;

            case SQL_TABLE_COMPARATOR_HYSTERESIS:
                {
                }
                break;


            case SQL_TABLE_COMPLEX_COMPARATOR:
                {
                }
                break;

            case SQL_TABLE_COMPLEX_COMPARATOR_HYSTERESIS:
                {
                }
                break;

            case SQL_TABLE_COMPLEX_COMPARATOR_POINT:
                {
                }
                break;

            case SQL_TABLE_COMPLEX_COMPARATOR_SIGNAL:
                {
                }
                break;

            case SQL_TABLE_OUTPUT_SIGNAL:
                {
                }
                break;

            case SQL_TABLE_REPORT_HEADER:
                {
                    REPORT_HEADER* header = static_cast<REPORT_HEADER*> (pRecord) + readedCount;

                    header->m_type = query.value(field++).toInt();

                    header->m_documentTitle = query.value(field++).toString();
                    header->m_reportTitle = query.value(field++).toString();
                    header->m_date = query.value(field++).toString();
                    header->m_tableTitle = query.value(field++).toString();
                    header->m_conclusion = query.value(field++).toString();

                    header->m_T = query.value(field++).toDouble();
                    header->m_P = query.value(field++).toDouble();
                    header->m_H = query.value(field++).toDouble();
                    header->m_V = query.value(field++).toDouble();
                    header->m_F = query.value(field++).toDouble();

                    header->m_calibrator[CALIBRATOR_0] = query.value(field++).toString();
                    header->m_calibrator[CALIBRATOR_1] = query.value(field++).toString();
                    header->m_calibrator[CALIBRATOR_2] = query.value(field++).toString();
                    header->m_calibrator[CALIBRATOR_3] = query.value(field++).toString();
                    header->m_calibrator[CALIBRATOR_4] = query.value(field++).toString();
                    header->m_calibrator[CALIBRATOR_5] = query.value(field++).toString();

                    header->m_linkObjectID = query.value(field++).toInt();
                    header->m_reportFile = query.value(field++).toString();

                    header->m_param = query.value(field++).toInt();
                }
                break;

            default:
                assert(0);
                break;
        }

       readedCount ++;
    }

    return readedCount;
}

// -------------------------------------------------------------------------------------------------------------------

int SqlTable::write(void* pRecord, int count, int* key)
{
    if (isOpen() == false)
    {
        return 0;
    }

    if (pRecord == nullptr)
    {
        return 0;
    }

    if (count == 0)
    {
        return 0;
    }

    // create request
    //

    QString request;

    if (key == nullptr)
    {
        request = QString("INSERT INTO %1 (").arg(m_info.name());

        int filedCount = m_fieldBase.count();
        for(int f = 0; f < filedCount; f++ )
        {
            request.append( m_fieldBase.field(f).name() );

            if (f != filedCount - 1)
            {
                request.append(", ");
            }
        }

        request.append(") VALUES (");

        for(int f = 0; f < filedCount; f++ )
        {
            request.append( "?" );

            if (f != filedCount - 1)
            {
                request.append(", ");
            }
        }

        request.append(" );");
    }
    else
    {
        request = QString("UPDATE %1 SET ").arg(m_info.name());

        int filedCount = m_fieldBase.count();
        for(int f = 0; f < filedCount; f++ )
        {
            request.append(QString("%1=?").arg(m_fieldBase.field(f).name()));

            if (f != filedCount - 1)
            {
                request.append(", ");
            }
        }

        request.append(QString(" WHERE %1=").arg(m_fieldBase.field(SQL_FIELD_KEY).name()));
    }

    int field = 0;
    int writedCount = 0;

    QSqlQuery query;

    if (query.exec("BEGIN TRANSACTION") == false)
    {
        return 0;
    }

    for (int r = 0; r < count; r++)
    {
        if (query.prepare(key == nullptr ? request :  request + QString("%1").arg(key[r])) == false)
        {
            continue;
        }

        field = 0;

        query.bindValue(field++, m_info.objectID());

        switch(m_info.objectType())
        {
            case SQL_TABLE_DATABASE_INFO:
                {
                    SqlObjectInfo* info = static_cast<SqlObjectInfo*> (pRecord) + r;

                    query.bindValue(field++, info->objectID());
                    query.bindValue(field++, info->name());
                    query.bindValue(field++, info->version());
                }
                break;

            case SQL_TABLE_HISTORY:
                {
                    SqlHistoryDatabase* history = static_cast<SqlHistoryDatabase*> (pRecord) + r;

                    query.bindValue(field++, history->version());
                    query.bindValue(field++, history->event());
                    query.bindValue(field++, history->time());
                }
                break;

            case SQL_TABLE_LINEARETY:
                {
                    LinearetyMeasureItem* measure = static_cast<LinearetyMeasureItem*> (pRecord) + r;

                    measure->setMeasureID( lastKey() + 1);

                    query.bindValue(field++, measure->measureID());

                    query.bindValue(field++, measure->filter());

                    query.bindValue(field++, measure->strID());
                    query.bindValue(field++, measure->extStrID());
                    query.bindValue(field++, measure->name());

                    query.bindValue(field++, measure->position().deviceStrID());
                    query.bindValue(field++, measure->position().caseNo());
                    query.bindValue(field++, measure->position().caseType());
                    query.bindValue(field++, measure->position().channel());
                    query.bindValue(field++, measure->position().subblock());
                    query.bindValue(field++, measure->position().block());
                    query.bindValue(field++, measure->position().entry());

                    query.bindValue(field++, measure->nominal(VALUE_TYPE_ELECTRIC));
                    query.bindValue(field++, measure->nominal(VALUE_TYPE_PHYSICAL));
                    query.bindValue(field++, measure->nominal(VALUE_TYPE_OUTPUT));

                    query.bindValue(field++, measure->percent());

                    query.bindValue(field++, measure->measure(VALUE_TYPE_ELECTRIC));
                    query.bindValue(field++, measure->measure(VALUE_TYPE_PHYSICAL));
                    query.bindValue(field++, measure->measure(VALUE_TYPE_OUTPUT));

                    query.bindValue(field++, measure->lowLimit(VALUE_TYPE_ELECTRIC));
                    query.bindValue(field++, measure->highLimit(VALUE_TYPE_ELECTRIC));
                    query.bindValue(field++, measure->unit(VALUE_TYPE_ELECTRIC));

                    query.bindValue(field++, measure->lowLimit(VALUE_TYPE_PHYSICAL));
                    query.bindValue(field++, measure->highLimit(VALUE_TYPE_PHYSICAL));
                    query.bindValue(field++, measure->unit(VALUE_TYPE_PHYSICAL));

                    query.bindValue(field++, measure->lowLimit(VALUE_TYPE_OUTPUT));
                    query.bindValue(field++, measure->highLimit(VALUE_TYPE_OUTPUT));
                    query.bindValue(field++, measure->unit(VALUE_TYPE_OUTPUT));

                    query.bindValue(field++, measure->valuePrecision(VALUE_TYPE_ELECTRIC));
                    query.bindValue(field++, measure->valuePrecision(VALUE_TYPE_PHYSICAL));
                    query.bindValue(field++, measure->valuePrecision(VALUE_TYPE_OUTPUT));

                    query.bindValue(field++, measure->hasOutput());
                    query.bindValue(field++, measure->adjustment());

                    query.bindValue(field++, measure->errorInput(ERROR_TYPE_ABSOLUTE));
                    query.bindValue(field++, measure->errorInput(ERROR_TYPE_REDUCE));

                    query.bindValue(field++, measure->errorOutput(ERROR_TYPE_ABSOLUTE));
                    query.bindValue(field++, measure->errorOutput(ERROR_TYPE_REDUCE));

                    query.bindValue(field++, measure->errorLimit(ERROR_TYPE_ABSOLUTE));
                    query.bindValue(field++, measure->errorLimit(ERROR_TYPE_REDUCE));

                    query.bindValue(field++, measure->errorPrecision(ERROR_TYPE_ABSOLUTE));
                    query.bindValue(field++, measure->errorPrecision(ERROR_TYPE_REDUCE));

                    measure->setMeasureTime(QDateTime::currentDateTime());

                    query.bindValue(field++, measure->measureTime().toString(MEASURE_TIME_FORMAT));

                }
                break;

            case SQL_TABLE_LINEARETY_20_EL:
            case SQL_TABLE_LINEARETY_20_PH:
                {
                    int valueType = VALUE_TYPE_UNKNOWN;

                    switch(m_info.objectType())
                    {
                        case SQL_TABLE_LINEARETY_20_EL:     valueType = VALUE_TYPE_ELECTRIC;    break;
                        case SQL_TABLE_LINEARETY_20_PH:     valueType = VALUE_TYPE_PHYSICAL;    break;
                        default:                            valueType = VALUE_TYPE_UNKNOWN;     break;
                    }

                    if (valueType == VALUE_TYPE_UNKNOWN)
                    {
                        break;
                    }

                    LinearetyMeasureItem* measure = static_cast<LinearetyMeasureItem*> (pRecord) + r;

                    query.bindValue(field++, measure->measureID());

                    query.bindValue(field++, measure->measureArrayCount());

                    query.bindValue(field++, measure->measureItemArray(valueType, 0));
                    query.bindValue(field++, measure->measureItemArray(valueType, 1));
                    query.bindValue(field++, measure->measureItemArray(valueType, 2));
                    query.bindValue(field++, measure->measureItemArray(valueType, 3));
                    query.bindValue(field++, measure->measureItemArray(valueType, 4));
                    query.bindValue(field++, measure->measureItemArray(valueType, 5));
                    query.bindValue(field++, measure->measureItemArray(valueType, 6));
                    query.bindValue(field++, measure->measureItemArray(valueType, 7));
                    query.bindValue(field++, measure->measureItemArray(valueType, 8));
                    query.bindValue(field++, measure->measureItemArray(valueType, 9));
                    query.bindValue(field++, measure->measureItemArray(valueType, 10));
                    query.bindValue(field++, measure->measureItemArray(valueType, 11));
                    query.bindValue(field++, measure->measureItemArray(valueType, 12));
                    query.bindValue(field++, measure->measureItemArray(valueType, 13));
                    query.bindValue(field++, measure->measureItemArray(valueType, 14));
                    query.bindValue(field++, measure->measureItemArray(valueType, 15));
                    query.bindValue(field++, measure->measureItemArray(valueType, 16));
                    query.bindValue(field++, measure->measureItemArray(valueType, 17));
                    query.bindValue(field++, measure->measureItemArray(valueType, 18));
                    query.bindValue(field++, measure->measureItemArray(valueType, 19));
                }
                break;

            case SQL_TABLE_LINEARETY_ADD_VAL:
                {
                    LinearetyMeasureItem* measure = static_cast<LinearetyMeasureItem*> (pRecord) + r;

                    query.bindValue(field++, measure->measureID());

                    query.bindValue(field++, measure->additionalValueCount());

                    query.bindValue(field++, measure->additionalValue(ADDITIONAL_VALUE_MEASURE_MIN));
                    query.bindValue(field++, measure->additionalValue(ADDITIONAL_VALUE_MEASURE_MAX));
                    query.bindValue(field++, measure->additionalValue(ADDITIONAL_VALUE_SYSTEM_ERROR));
                    query.bindValue(field++, measure->additionalValue(ADDITIONAL_VALUE_MSE));
                    query.bindValue(field++, measure->additionalValue(ADDITIONAL_VALUE_LOW_BORDER));
                    query.bindValue(field++, measure->additionalValue(ADDITIONAL_VALUE_HIGH_BORDER));
                    query.bindValue(field++, 0);
                    query.bindValue(field++, 0);
                    query.bindValue(field++, 0);
                    query.bindValue(field++, 0);
                    query.bindValue(field++, 0);
                    query.bindValue(field++, 0);
                    query.bindValue(field++, 0);
                    query.bindValue(field++, 0);
                    query.bindValue(field++, 0);
                    query.bindValue(field++, 0);
                }
                break;

            case SQL_TABLE_LINEARETY_POINT:
                {
                    LinearityPoint* point = static_cast<LinearityPoint*> (pRecord) + r;

                    query.bindValue(field++, point->pointID());
                    query.bindValue(field++, point->percent());
                }
                break;

            case SQL_TABLE_COMPARATOR:
                {
                }
                break;

            case SQL_TABLE_COMPARATOR_HYSTERESIS:
                {
                }
                break;


            case SQL_TABLE_COMPLEX_COMPARATOR:
                {
                }
                break;

            case SQL_TABLE_COMPLEX_COMPARATOR_HYSTERESIS:
                {
                }
                break;

            case SQL_TABLE_COMPLEX_COMPARATOR_POINT:
                {
                }
                break;

            case SQL_TABLE_COMPLEX_COMPARATOR_SIGNAL:
                {
                }
                break;

            case SQL_TABLE_OUTPUT_SIGNAL:
                {
                }
                break;

            case SQL_TABLE_REPORT_HEADER:
                {
                    REPORT_HEADER* header = static_cast<REPORT_HEADER*> (pRecord) + r;

                    query.bindValue(field++, header->m_type);

                    query.bindValue(field++, header->m_documentTitle);
                    query.bindValue(field++, header->m_reportTitle);
                    query.bindValue(field++, header->m_date);
                    query.bindValue(field++, header->m_tableTitle);
                    query.bindValue(field++, header->m_conclusion);

                    query.bindValue(field++, header->m_T);
                    query.bindValue(field++, header->m_P);
                    query.bindValue(field++, header->m_H);
                    query.bindValue(field++, header->m_V);
                    query.bindValue(field++, header->m_F);

                    query.bindValue(field++, header->m_calibrator[CALIBRATOR_0]);
                    query.bindValue(field++, header->m_calibrator[CALIBRATOR_1]);
                    query.bindValue(field++, header->m_calibrator[CALIBRATOR_2]);
                    query.bindValue(field++, header->m_calibrator[CALIBRATOR_3]);
                    query.bindValue(field++, header->m_calibrator[CALIBRATOR_4]);
                    query.bindValue(field++, header->m_calibrator[CALIBRATOR_5]);

                    query.bindValue(field++, header->m_linkObjectID);
                    query.bindValue(field++, header->m_reportFile);

                    query.bindValue(field++, header->m_param);
                }
                break;

            default:
                assert(0);
                break;
        }

        if (query.exec() == false)
        {
            continue;
        }

        writedCount ++;
    }

    if (query.exec("COMMIT") == false)
    {
        return 0;
    }

    return writedCount;
}

// -------------------------------------------------------------------------------------------------------------------

int SqlTable::remove(int* key, int keyCount)
{
    if (isOpen() == false)
    {
        return 0;
    }

    if (key == nullptr || keyCount == 0)
    {
        return 0;
    }

    int count = recordCount();
    if (count == 0)
    {
        return 0;
    }

    QString request = QString("DELETE FROM %1 WHERE ").arg(m_info.name());
    QString keyFieldName = m_fieldBase.field(SQL_FIELD_KEY).name();

    for (int k = 0; k < keyCount; k++)
    {
        request.append(QString("%1=%2").arg(keyFieldName).arg(key[k]));

        if (k != keyCount - 1)
        {
            request.append(" OR ");
        }

    }

    QSqlQuery query;

    if (query.exec("BEGIN TRANSACTION") == false)
    {
        return 0;
    }

    if(query.exec(request) == false)
    {
        return 0;
    }

    if (query.exec("COMMIT") == false)
    {
        return 0;
    }

    return count - recordCount();
}

// -------------------------------------------------------------------------------------------------------------------

SqlTable& SqlTable::operator=(SqlTable& from)
{
    m_pDatabase = from.m_pDatabase;
    m_info = from.m_info;
    m_fieldBase = from.m_fieldBase;

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SqlHistoryDatabase Database::m_history[] =
{
    SqlHistoryDatabase(SQL_TABLE_HISTORY, 0, "Create database", "11-11-2014 11:11:11"),
};

// -------------------------------------------------------------------------------------------------------------------

Database::Database(QObject* parent) :
    QObject(parent)
{
    for(int type = 0; type < SQL_TABLE_COUNT; type++)
    {
        m_table[type].init(type, &m_database);
    }
}

// -------------------------------------------------------------------------------------------------------------------

Database::~Database()
{
    close();
}

// -------------------------------------------------------------------------------------------------------------------

bool Database::open()
{
    QString path = theOptions.database().m_path;
    if (path.isEmpty() == true)
    {
        QMessageBox::critical(nullptr, tr("Database"), tr("Invalid path!"));
        return false;
    }

    switch(theOptions.database().m_type)
    {
        case DATABASE_TYPE_SQLITE:

            m_database = QSqlDatabase::addDatabase("QSQLITE");
            if (m_database.lastError().isValid() == true)
            {
                return false;
            }

            m_database.setDatabaseName(path + QDir::separator() + DATABASE_NAME);

            break;

        default:
            assert(0);
            break;
    }

    if (m_database.open() == false)
    {
        QMessageBox::critical(nullptr, tr("Database"), tr("Cannot open database"));
        return false;
    }

    QSqlQuery query;

    if (query.exec("PRAGMA foreign_keys=on") == false)
    {
        QMessageBox::critical(nullptr, tr("Database"), tr("Error set option of database: [foreign keys=on]"));
    }

    if (query.exec("PRAGMA synchronous=normal") == false)
    {
        QMessageBox::critical(nullptr, tr("Database"), tr("Error set option of database: [synchronous=normal]"));
    }


    initVersion();
    createTables();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void Database::close()
{
    for(int type = 0; type < SQL_TABLE_COUNT; type++)
    {
        if (m_table[type].isOpen() == true)
        {
            m_table[type].close();
        }

        m_table[type].info().clear();
    }

    if (m_database.isOpen() == true)
    {
        m_database.close();
    }
}

// -------------------------------------------------------------------------------------------------------------------

SqlTable* Database::openTable(int objectType)
{
    if (objectType < 0 || objectType >= SQL_TABLE_COUNT)
    {
        return nullptr;
    }

    if (m_table[objectType].isOpen() == true)
    {
        return nullptr;
    }

    if (m_table[objectType].open() == false)
    {
        return nullptr;
    }

    return &m_table[objectType];
}

// -------------------------------------------------------------------------------------------------------------------

void Database::initVersion()
{
    SqlTable table;
    if (table.init(SQL_TABLE_DATABASE_INFO, &m_database) == false)
    {
        return;
    }

    QVector<SqlObjectInfo> info;

    if (table.isExist() == false )
    {
        if (table.create() == true)
        {
            info.resize(SQL_TABLE_COUNT);

            for(int t = 0; t < SQL_TABLE_COUNT; t++)
            {
                info[t] = m_table[t].info();
            }

            table.write(info.data(), info.count());
        }
    }
    else
    {
        if (table.open() == true)
        {
            info.resize(table.recordCount());

            int count = table.read(info.data());
            for (int i = 0; i < count; i++)
            {
                for(int t = 0; t < SQL_TABLE_COUNT; t++)
                {
                    if (m_table[t].info().objectID() == info[i].objectID())
                    {
                        m_table[t].info().setVersion( info[i].version() );
                        break;
                    }
                }
            }
        }
    }

    table.close();
}

// -------------------------------------------------------------------------------------------------------------------

void Database::createTables()
{
    // find table in database, if table is not exist, then create it
    //
    for(int type = 0; type < SQL_TABLE_COUNT; type++)
    {
        SqlTable table;

        if (table.init(type, &m_database) == true)
        {
            if (table.isExist() == false)
            {
                if (table.create() == false)
                {
                    QMessageBox::critical(nullptr, tr("Database"), tr("Cannot create table: %1").arg(table.info().name()));
                }
            }
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------

bool Database::appendMeasure(MeasureItem* pMeasure)
{
    if (pMeasure == nullptr)
    {
        return false;
    }

    int measureType = pMeasure->measureType();
    if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
    {
        return false;
    }

    bool result = false;

    for (int type = 0; type < SQL_TABLE_COUNT; type++)
    {
        if ( SqlTableByMeasureType[type] != measureType )
        {
            continue;
        }

        SqlTable& table = m_table[type];

        if (table.open() == true)
        {
            if (table.write(pMeasure) == 1)
            {
                result = true;
            }

            table.close();
        }
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

bool Database::removeMeasure(int measuteType, QVector<int> keyList)
{
    bool result = false;

    for (int type = 0; type < SQL_TABLE_COUNT; type++)
    {
        if ( SqlTableByMeasureType[type] != measuteType )
        {
            continue;
        }

        SqlTable& table = m_table[type];

        if (table.open() == true)
        {
            if (table.remove(keyList.data(), keyList.count()) == keyList.count())
            {
                result = true;
            }

            table.close();
        }

        break;
    }

    return result;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

