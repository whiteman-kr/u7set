#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QtSql>
#include <QMutex>

#include "Measure.h"

// ==============================================================================================

#define                 DATABASE_NAME       "Metrology.db"
#define                 DATABASE_VERSION    0

// ==============================================================================================
//
// Is a list of fields SQL tables.
//
class SqlFieldBase : public QSqlRecord
{
public:

                        SqlFieldBase();
                        ~SqlFieldBase();

    int                 init(int objectType, int version);

    void                append(const QSqlField& field);
    void                append(QString name, QVariant::Type type = QVariant::Invalid, int length = 0);

    QString             extFieldName(int index);
};

// ==============================================================================================

const char* const		SqlTabletName[] =
{
                        QT_TRANSLATE_NOOP("Database.h", "DatabaseInfo"),
                        QT_TRANSLATE_NOOP("Database.h", "History"),

                        QT_TRANSLATE_NOOP("Database.h", "LinearityMeasure"),
                        QT_TRANSLATE_NOOP("Database.h", "LinearityMeasureAddVal"),
                        QT_TRANSLATE_NOOP("Database.h", "LinearityMeasure20El"),
                        QT_TRANSLATE_NOOP("Database.h", "LinearityMeasure20Ph"),
                        QT_TRANSLATE_NOOP("Database.h", "LinearityPoint"),

                        QT_TRANSLATE_NOOP("Database.h", "ComparatorMeasure"),
                        QT_TRANSLATE_NOOP("Database.h", "ComparatorHysteresis"),

                        QT_TRANSLATE_NOOP("Database.h", "ComplexComparatorMeasure"),
                        QT_TRANSLATE_NOOP("Database.h", "ComplexComparatorHysteresis"),
                        QT_TRANSLATE_NOOP("Database.h", "ComplexComparatorPoint"),
                        QT_TRANSLATE_NOOP("Database.h", "ComplexComparatorSignal"),

                        QT_TRANSLATE_NOOP("Database.h", "OutputSignal"),

                        QT_TRANSLATE_NOOP("Database.h", "ReportOption"),
};

const int               SQL_TABLE_COUNT                    = sizeof(SqlTabletName)/sizeof(SqlTabletName[0]);

const int               SQL_TABLE_UNKNONW                       = -1,
                        SQL_TABLE_DATABASE_INFO                 = 0,
                        SQL_TABLE_HISTORY                       = 1,
                        SQL_TABLE_LINEARITY                     = 2,
                        SQL_TABLE_LINEARITY_ADD_VAL             = 3,
                        SQL_TABLE_LINEARITY_20_EL               = 4,
                        SQL_TABLE_LINEARITY_20_PH               = 5,
                        SQL_TABLE_LINEARITY_POINT               = 6,
                        SQL_TABLE_COMPARATOR                    = 7,
                        SQL_TABLE_COMPARATOR_HYSTERESIS         = 8,
                        SQL_TABLE_COMPLEX_COMPARATOR            = 9,
                        SQL_TABLE_COMPLEX_COMPARATOR_HYSTERESIS = 10,
                        SQL_TABLE_COMPLEX_COMPARATOR_POINT      = 11,
                        SQL_TABLE_COMPLEX_COMPARATOR_SIGNAL     = 12,
                        SQL_TABLE_OUTPUT_SIGNAL                 = 13,
                        SQL_TABLE_REPORT_HEADER                 = 14;

// ----------------------------------------------------------------------------------------------

#define					ERR_SQL_TABLE(table) (table < 0 || table >= SQL_TABLE_COUNT)
#define                 TEST_SQL_TABLE(table)			if (ERR_SQL_TABLE(table)) { return; }
#define					TEST_SQL_TABLE1(table, retVal)	if (ERR_SQL_TABLE(table)) { return retVal; }


// ==============================================================================================

const int              SQL_TABLE_VER_UNKNONW = -1;

// ----------------------------------------------------------------------------------------------
//
// current versions of tables or views
//
const int				SqlTableVersion[SQL_TABLE_COUNT] =
{
                        DATABASE_VERSION,   //    SQL_TABLE_DATABASE_INFO
                        0,                  //    SQL_TABLE_HISTORY

                        0,                  //    SQL_TABLE_LINEARITY
                        0,                  //    SQL_TABLE_LINEARITY_ADD_VAL
                        0,                  //    SQL_TABLE_LINEARITY_20_EL
                        0,                  //    SQL_TABLE_LINEARITY_20_PH
                        0,                  //    SQL_TABLE_LINEARITY_POINT

                        0,                  //    SQL_TABLE_COMPARATOR
                        0,                  //    SQL_TABLE_COMPARATOR_HYSTERESIS

                        0,                  //    SQL_TABLE_COMPLEX_COMPARATOR
                        0,                  //    SQL_TABLE_COMPLEX_COMPARATOR_HYSTERESIS
                        0,                  //    SQL_TABLE_COMPLEX_COMPARATOR_POINT
                        0,                  //    SQL_TABLE_COMPLEX_COMPARATOR_SIGNAL

                        0,                  //    SQL_TABLE_OUTPUT_SIGNAL

                        0,                  //    SQL_TABLE_REPORT_HEADER
};

// ==============================================================================================

const int              SQL_OBJECT_ID_UNKNONW = -1;

// ----------------------------------------------------------------------------------------------
//
// unique object ID in the database
// x00 - measurements, x10 - points, x20 - signals
//
const int               SqlObjectID[SQL_TABLE_COUNT] =
{
                        0,          //    SQL_TABLE_DATABASE_INFO
                        1,          //    SQL_TABLE_HISTORY

                        100,        //    SQL_TABLE_LINEARITY
                        101,        //    SQL_TABLE_LINEARITY_ADD_VAL
                        102,        //    SQL_TABLE_LINEARITY_20_EL
                        103,        //    SQL_TABLE_LINEARITY_20_PH
                        110,        //    SQL_TABLE_LINEARITY_POINT

                        200,        //    SQL_TABLE_COMPARATOR
                        201,        //    SQL_TABLE_COMPARATOR_HYSTERESIS

                        300,        //    SQL_TABLE_COMPLEX_COMPARATOR
                        301,        //    SQL_TABLE_COMPLEX_COMPARATOR_HYSTERESIS
                        310,        //    SQL_TABLE_COMPLEX_COMPARATOR_POINT
                        320,        //    SQL_TABLE_COMPLEX_COMPARATOR_SIGNAL

                        420,        //    SQL_TABLE_OUTPUT_SIGNAL

                        500,        //    SQL_TABLE_REPORT_HEADER
};

// ==============================================================================================

const int               SQL_TABLE_IS_MAIN   = 0,
                        SQL_TABLE_IS_SUB    = 1,
                        SQL_TABLE_IS_CONFIG = 2;

// ----------------------------------------------------------------------------------------------

const int               SqlTableByMeasureType[SQL_TABLE_COUNT] =
{
                        MEASURE_TYPE_UNKNOWN,               //    SQL_TABLE_DATABASE_INFO                       // SQL_TABLE_CONFIG
                        MEASURE_TYPE_UNKNOWN,               //    SQL_TABLE_HISTORY                             // SQL_TABLE_CONFIG

                        MEASURE_TYPE_LINEARITY,             //    SQL_TABLE_LINEARITY                           // SQL_TABLE_MEASURE_MAIN
                        MEASURE_TYPE_LINEARITY,             //    SQL_TABLE_LINEARITY_ADD_VAL                   // SQL_TABLE_MEASURE_SUB
                        MEASURE_TYPE_LINEARITY,             //    SQL_TABLE_LINEARITY_20_EL                     // SQL_TABLE_MEASURE_SUB
                        MEASURE_TYPE_LINEARITY,             //    SQL_TABLE_LINEARITY_20_PH                     // SQL_TABLE_MEASURE_SUB
                        MEASURE_TYPE_UNKNOWN,               //    SQL_TABLE_LINEARITY_POINT                     // SQL_TABLE_CONFIG

                        MEASURE_TYPE_COMPARATOR,            //    SQL_TABLE_COMPARATOR                          // SQL_TABLE_MEASURE_MAIN
                        MEASURE_TYPE_COMPARATOR,            //    SQL_TABLE_COMPARATOR_HYSTERESIS               // SQL_TABLE_MEASURE_SUB

                        MEASURE_TYPE_UNKNOWN,               //    SQL_TABLE_COMPLEX_COMPARATOR                  // SQL_TABLE_MEASURE_MAIN
                        MEASURE_TYPE_UNKNOWN,               //    SQL_TABLE_COMPLEX_COMPARATOR_HYSTERESIS       // SQL_TABLE_MEASURE_SUB
                        MEASURE_TYPE_UNKNOWN,               //    SQL_TABLE_COMPLEX_COMPARATOR_POINT            // SQL_TABLE_CONFIG
                        MEASURE_TYPE_UNKNOWN,               //    SQL_TABLE_COMPLEX_COMPARATOR_SIGNAL           // SQL_TABLE_CONFIG

                        MEASURE_TYPE_UNKNOWN,               //    SQL_TABLE_OUTPUT_SIGNAL                       // SQL_TABLE_CONFIG

                        MEASURE_TYPE_UNKNOWN,               //    SQL_TABLE_REPORT_HEADER                       // SQL_TABLE_CONFIG
};

// ==============================================================================================

const int				SQL_FIELD_OBJECT_ID = 0;            // zero column is unique identifier of the table (or other object) in the database
const int				SQL_FIELD_KEY       = 1;            // first column is key in the table, for example:: RecordID, PointID, SignalID, ReportID и т.д.

// ==============================================================================================

const int				SQL_INVALID_INDEX   = -1;
const int				SQL_INVALID_KEY		= -1;
const int				SQL_INVALID_RECORD  = -1;

// ==============================================================================================
//
// Represents the structure determines the version of the object (tables, databases, etc.) in the database
//
class SqlObjectInfo
{
public:

                        SqlObjectInfo();
                        ~SqlObjectInfo();

private:

    int                 m_objectType = SQL_TABLE_UNKNONW;           // type of table
    int                 m_objectID = SQL_OBJECT_ID_UNKNONW;			// unique identifier of table in the database
    QString             m_name;                                     // наименование таблицы
    int                 m_version = SQL_TABLE_VER_UNKNONW;          // table version, is read when the database initialization

public:

    bool                init(int objectType);
    void                clear();

    int                 objectType() const { return m_objectType; }
    void                setObjectType(int type) { m_objectType = type; }

    int                 objectID() const { return m_objectID; }
    void                setObjectID(int objectID) { m_objectID = objectID; }

    QString             name() const { return m_name; }
    void                setName(const QString& name) { m_name = name; }

    int                 version() const { return m_version; }
    void                setVersion(int verison) { m_version = verison; }

    SqlObjectInfo&      operator=(SqlObjectInfo& from);
};

// ==============================================================================================

class SqlHistoryDatabase
{

public:

                        SqlHistoryDatabase();
                        SqlHistoryDatabase(int objectID, int version, const QString& event, const QString& time);
                        ~SqlHistoryDatabase();

private:

    int                 m_objectID = SQL_OBJECT_ID_UNKNONW;
    int                 m_version = SQL_TABLE_VER_UNKNONW;
    QString             m_event;
    QString             m_time;

public:

    int                 objectID() const { return m_objectID; }
    void                setObjectID(int objectID) { m_objectID = objectID; }

    int                 version() const { return m_version; }
    void                setVersion(int verison) { m_version = verison; }

    QString             event() const { return m_event; }
    void                setEvent(const QString& event) { m_event = event; }

    QString             time() const { return m_time; }
    void                setTime(const QString& time) { m_time = time; }

    SqlHistoryDatabase& operator=(SqlHistoryDatabase& from);
};

// ==============================================================================================

class SqlTable
{
public:

                        SqlTable();
                        ~SqlTable();

private:

    QSqlDatabase*       m_pDatabase;
    SqlObjectInfo       m_info;
    SqlFieldBase        m_fieldBase;

public:

    SqlObjectInfo&      info() { return m_info; }
    void                setInfo(SqlObjectInfo info) { m_info = info; }

    bool                isEmpty() { return recordCount() == 0; }
    int                 recordCount() const;
    int                 lastKey() const;

    bool                init(int objectType, QSqlDatabase* pDatabase);

    bool                isExist() const;
    bool                isOpen() const { return m_fieldBase.count() != 0; }
    bool                open();
    void                close();

    bool                create();
    bool                drop();
    bool                clear();

    int                 read(void* pRecord, int key) { return read(pRecord, &key, 1); }
    int                 read(void* pRecord, int* key = nullptr, int keyCount = 0);             // read record form table, if key == nullptr in the array pRecord will be record all records of table

    int                 write(void* pRecord) { return write(pRecord, 1); }
    int                 write(void* pRecord, int count, int key) { return write(pRecord, count, &key); }
    int                 write(void* pRecord, int recordCount, int* key = nullptr);             // insert or update records (depend from key) in a table, pRecord - array of record, count - amount records

    int                 remove(int key) { return remove(&key, 1); }                                  // remove records by key
    int                 remove(const int* key, int keyCount) const;

    SqlTable&           operator=(SqlTable& from);
};

// ==============================================================================================

class Database : public QObject
{
    Q_OBJECT

public:

    explicit            Database(QObject* parent = 0);
                        ~Database();

    bool                isOpen() const { return m_database.isOpen(); }
    bool                open();
    void                close();

    SqlTable*           openTable(int objectType);

    bool                appendMeasure(Measurement* pMeasurement);
    bool                removeMeasure(int measuteType, const QVector<int>& keyList);

private:

    QSqlDatabase        m_database;
    SqlTable            m_table[SQL_TABLE_COUNT];

    static SqlHistoryDatabase m_history[DATABASE_VERSION + 1];

    void                initVersion();
    void                createTables();
};

// ==============================================================================================

extern Database*         thePtrDB;

// ==============================================================================================

#endif // DATABASE_H
