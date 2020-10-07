#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QtSql>

#include "MeasureBase.h"

// ==============================================================================================

#define					DATABASE_NAME		"Metrology.db"
#define					DATABASE_VERSION	0

// ==============================================================================================
//
// Is a list of fields SQL tables.
//
class SqlFieldBase : public QSqlRecord
{
public:

	SqlFieldBase();
	virtual ~SqlFieldBase() {}

public:

	int					init(int objectType, int version);

	void				append(const QSqlField& field);
	void				append(QString name, QVariant::Type type = QVariant::Invalid, int length = 0);

	QString				extFieldName(int index);
};

// ==============================================================================================

const char* const		SqlTabletName[] =
{
						QT_TRANSLATE_NOOP("Database.h", "DatabaseInfo"),
						QT_TRANSLATE_NOOP("Database.h", "History"),

						QT_TRANSLATE_NOOP("Database.h", "LinearityMeasure"),
						QT_TRANSLATE_NOOP("Database.h", "LinearityMeasureAddValEl"),
						QT_TRANSLATE_NOOP("Database.h", "LinearityMeasureAddValEn"),
						QT_TRANSLATE_NOOP("Database.h", "LinearityMeasure20El"),
						QT_TRANSLATE_NOOP("Database.h", "LinearityMeasure20En"),
						QT_TRANSLATE_NOOP("Database.h", "LinearityPoint"),

						QT_TRANSLATE_NOOP("Database.h", "ComparatorMeasure"),

						QT_TRANSLATE_NOOP("Database.h", "ComplexComparatorMeasure"),
						QT_TRANSLATE_NOOP("Database.h", "ComplexComparatorHysteresis"),
						QT_TRANSLATE_NOOP("Database.h", "ComplexComparatorPoint"),
						QT_TRANSLATE_NOOP("Database.h", "ComplexComparatorSignal"),

						QT_TRANSLATE_NOOP("Database.h", "ReportOption"),
						QT_TRANSLATE_NOOP("Database.h", "RackGroup"),
						QT_TRANSLATE_NOOP("Database.h", "SignalConnection"),
};

const int				SQL_TABLE_COUNT							= sizeof(SqlTabletName)/sizeof(SqlTabletName[0]);

const int				SQL_TABLE_UNKNONW						= -1,
						SQL_TABLE_DATABASE_INFO					= 0,
						SQL_TABLE_HISTORY						= 1,
						SQL_TABLE_LINEARITY						= 2,
						SQL_TABLE_LINEARITY_ADD_VAL_EL			= 3,
						SQL_TABLE_LINEARITY_ADD_VAL_EN			= 4,
						SQL_TABLE_LINEARITY_20_EL				= 5,
						SQL_TABLE_LINEARITY_20_EN				= 6,
						SQL_TABLE_LINEARITY_POINT				= 7,
						SQL_TABLE_COMPARATOR					= 8,
						SQL_TABLE_COMPLEX_COMPARATOR			= 9,
						SQL_TABLE_COMPLEX_COMPARATOR_HYSTERESIS	= 10,
						SQL_TABLE_COMPLEX_COMPARATOR_POINT		= 11,
						SQL_TABLE_COMPLEX_COMPARATOR_SIGNAL		= 12,
						SQL_TABLE_REPORT_HEADER					= 13,
						SQL_TABLE_RACK_GROUP					= 14,
						SQL_TABLE_SIGNAL_CONNECTION				= 15;

// ==============================================================================================

const int				SQL_TABLE_VER_UNKNONW = -1;

// ----------------------------------------------------------------------------------------------
//
// current versions of tables or views
//
const int				SqlTableVersion[SQL_TABLE_COUNT] =
{
						DATABASE_VERSION,	//	SQL_TABLE_DATABASE_INFO
						0,					//	SQL_TABLE_HISTORY

						0,					//	SQL_TABLE_LINEARITY
						0,					//	SQL_TABLE_LINEARITY_ADD_EL_VAL
						0,					//	SQL_TABLE_LINEARITY_ADD_EN_VAL
						0,					//	SQL_TABLE_LINEARITY_20_EL
						0,					//	SQL_TABLE_LINEARITY_20_EN
						0,					//	SQL_TABLE_LINEARITY_POINT

						0,					//	SQL_TABLE_COMPARATOR

						0,					//	SQL_TABLE_COMPLEX_COMPARATOR
						0,					//	SQL_TABLE_COMPLEX_COMPARATOR_HYSTERESIS
						0,					//	SQL_TABLE_COMPLEX_COMPARATOR_POINT
						0,					//	SQL_TABLE_COMPLEX_COMPARATOR_SIGNAL

						0,					//	SQL_TABLE_REPORT_HEADER
						0,					//	SQL_TABLE_RACK_GROUP
						0,					//	SQL_TABLE_SIGNAL_CONNECTION
};

// ==============================================================================================

const int				SQL_OBJECT_ID_UNKNONW = -1;

// ----------------------------------------------------------------------------------------------
//
// unique object ID in the database
// x00 - measurements, x10 - points, x20 - signals, , x30 - racks
//
const int				SqlObjectID[SQL_TABLE_COUNT] =
{
						0,			//	SQL_TABLE_DATABASE_INFO
						1,			//	SQL_TABLE_HISTORY

						100,		//	SQL_TABLE_LINEARITY
						110,		//	SQL_TABLE_LINEARITY_ADD_EL_VAL
						111,		//	SQL_TABLE_LINEARITY_ADD_EN_VAL
						120,		//	SQL_TABLE_LINEARITY_20_EL
						121,		//	SQL_TABLE_LINEARITY_20_EN
						130,		//	SQL_TABLE_LINEARITY_POINT

						200,		//	SQL_TABLE_COMPARATOR

						300,		//	SQL_TABLE_COMPLEX_COMPARATOR
						301,		//	SQL_TABLE_COMPLEX_COMPARATOR_HYSTERESIS
						310,		//	SQL_TABLE_COMPLEX_COMPARATOR_POINT
						320,		//	SQL_TABLE_COMPLEX_COMPARATOR_SIGNAL

						400,		//	SQL_TABLE_REPORT_HEADER
						530,		//	SQL_TABLE_RACK_GROUP
						620,		//	SQL_TABLE_SIGNAL_CONNECTION
};

// ==============================================================================================

const int				SQL_TABLE_IS_MAIN	= 0,
						SQL_TABLE_IS_SUB	= 1,
						SQL_TABLE_IS_CONFIG	= 2;

// ----------------------------------------------------------------------------------------------

const int				SqlTableByMeasureType[SQL_TABLE_COUNT] =
{
						MEASURE_TYPE_UNKNOWN,				//	SQL_TABLE_DATABASE_INFO						// SQL_TABLE_CONFIG
						MEASURE_TYPE_UNKNOWN,				//	SQL_TABLE_HISTORY							// SQL_TABLE_CONFIG

						MEASURE_TYPE_LINEARITY,				//	SQL_TABLE_LINEARITY							// SQL_TABLE_MEASURE_MAIN
						MEASURE_TYPE_LINEARITY,				//	SQL_TABLE_LINEARITY_ADD_EL_VAL				// SQL_TABLE_MEASURE_SUB
						MEASURE_TYPE_LINEARITY,				//	SQL_TABLE_LINEARITY_ADD_EN_VAL				// SQL_TABLE_MEASURE_SUB
						MEASURE_TYPE_LINEARITY,				//	SQL_TABLE_LINEARITY_20_EL					// SQL_TABLE_MEASURE_SUB
						MEASURE_TYPE_LINEARITY,				//	SQL_TABLE_LINEARITY_20_EN					// SQL_TABLE_MEASURE_SUB
						MEASURE_TYPE_UNKNOWN,				//	SQL_TABLE_LINEARITY_POINT					// SQL_TABLE_CONFIG

						MEASURE_TYPE_COMPARATOR,			//	SQL_TABLE_COMPARATOR						// SQL_TABLE_MEASURE_MAIN

						MEASURE_TYPE_UNKNOWN,				//	SQL_TABLE_COMPLEX_COMPARATOR				// SQL_TABLE_MEASURE_MAIN
						MEASURE_TYPE_UNKNOWN,				//	SQL_TABLE_COMPLEX_COMPARATOR_HYSTERESIS		// SQL_TABLE_MEASURE_SUB
						MEASURE_TYPE_UNKNOWN,				//	SQL_TABLE_COMPLEX_COMPARATOR_POINT			// SQL_TABLE_CONFIG
						MEASURE_TYPE_UNKNOWN,				//	SQL_TABLE_COMPLEX_COMPARATOR_SIGNAL			// SQL_TABLE_CONFIG

						MEASURE_TYPE_UNKNOWN,				//	SQL_TABLE_REPORT_HEADER						// SQL_TABLE_CONFIG
						MEASURE_TYPE_UNKNOWN,				//	SQL_TABLE_RACK_GROUP						// SQL_TABLE_CONFIG
						MEASURE_TYPE_UNKNOWN,				//	SQL_TABLE_SIGNAL_CONNECTION					// SQL_TABLE_CONFIG
};

// ----------------------------------------------------------------------------------------------

const int				SqlTableAppointType[SQL_TABLE_COUNT] =
{
						SQL_TABLE_IS_CONFIG,				//	SQL_TABLE_DATABASE_INFO
						SQL_TABLE_IS_CONFIG,				//	SQL_TABLE_HISTORY

						SQL_TABLE_IS_MAIN,					//	SQL_TABLE_LINEARITY
						SQL_TABLE_IS_SUB,					//	SQL_TABLE_LINEARITY_ADD_EL_VAL
						SQL_TABLE_IS_SUB,					//	SQL_TABLE_LINEARITY_ADD_EN_VAL
						SQL_TABLE_IS_SUB,					//	SQL_TABLE_LINEARITY_20_EL
						SQL_TABLE_IS_SUB,					//	SQL_TABLE_LINEARITY_20_EN
						SQL_TABLE_IS_CONFIG,				//	SQL_TABLE_LINEARITY_POINT

						SQL_TABLE_IS_MAIN,					//	SQL_TABLE_COMPARATOR

						SQL_TABLE_IS_MAIN,					//	SQL_TABLE_COMPLEX_COMPARATOR
						SQL_TABLE_IS_SUB,					//	SQL_TABLE_COMPLEX_COMPARATOR_HYSTERESIS
						SQL_TABLE_IS_CONFIG,				//	SQL_TABLE_COMPLEX_COMPARATOR_POINT
						SQL_TABLE_IS_CONFIG,				//	SQL_TABLE_COMPLEX_COMPARATOR_SIGNAL

						SQL_TABLE_IS_CONFIG,				//	SQL_TABLE_REPORT_HEADER
						SQL_TABLE_IS_CONFIG,				//	SQL_TABLE_RACK_GROUP
						SQL_TABLE_IS_CONFIG,				//	SQL_TABLE_SIGNAL_CONNECTION
};

// ==============================================================================================

const int				SQL_FIELD_OBJECT_ID = 0;			// zero column is unique identifier of the table (or other object) in the database
const int				SQL_FIELD_KEY		= 1;			// first column is key in the table, for example:: RecordID, PointID, SignalID, ReportID и т.д.

// ==============================================================================================

const int				SQL_INVALID_INDEX	= -1;
const int				SQL_INVALID_KEY		= -1;
const int				SQL_INVALID_RECORD	= -1;

// ==============================================================================================
//
// Represents the structure determines the version of the object (tables, databases, etc.) in the database
//
class SqlObjectInfo
{
public:

	SqlObjectInfo();
	virtual ~SqlObjectInfo() {}

private:

	int					m_objectType = SQL_TABLE_UNKNONW;			// type of table
	int					m_objectID = SQL_OBJECT_ID_UNKNONW;			// unique identifier of table in the database
	QString				m_caption;									// caption of table
	int					m_version = SQL_TABLE_VER_UNKNONW;			// table version, is read when the database initialization

public:

	bool				init(int objectType);
	void				clear();

	int					objectType() const { return m_objectType; }
	void				setObjectType(int type) { m_objectType = type; }

	int					objectID() const { return m_objectID; }
	void				setObjectID(int objectID) { m_objectID = objectID; }

	QString				caption() const { return m_caption; }
	void				setCaption(const QString& caption) { m_caption = caption; }

	int					version() const { return m_version; }
	void				setVersion(int verison) { m_version = verison; }

	SqlObjectInfo&		operator=(SqlObjectInfo& from);
};

// ==============================================================================================

class SqlHistoryDatabase
{

public:

	SqlHistoryDatabase();
	SqlHistoryDatabase(int objectID, int version, const QString& event, const QString& time);
	virtual ~SqlHistoryDatabase();

private:

	int					m_objectID = SQL_OBJECT_ID_UNKNONW;
	int					m_version = SQL_TABLE_VER_UNKNONW;
	QString				m_event;
	QString				m_time;

public:

	int					objectID() const { return m_objectID; }
	void				setObjectID(int objectID) { m_objectID = objectID; }

	int					version() const { return m_version; }
	void				setVersion(int verison) { m_version = verison; }

	QString				event() const { return m_event; }
	void				setEvent(const QString& event) { m_event = event; }

	QString				time() const { return m_time; }
	void				setTime(const QString& time) { m_time = time; }

	SqlHistoryDatabase& operator=(SqlHistoryDatabase& from);
};

// ==============================================================================================

const int				REMOVE_TRANSACTION_RECORD_COUNT = 500;

// ----------------------------------------------------------------------------------------------

class SqlTable
{
public:

	SqlTable();
	virtual ~SqlTable();

private:

	QSqlDatabase*		m_pDatabase;
	SqlObjectInfo		m_info;
	SqlFieldBase		m_fieldBase;

public:

	SqlObjectInfo&		info() { return m_info; }
	void				setInfo(SqlObjectInfo info) { m_info = info; }

	bool				isEmpty() { return recordCount() == 0; }
	int					recordCount() const;
	int					lastKey() const;

	bool				init(int objectType, QSqlDatabase* pDatabase);

	bool				isExist() const;
	bool				isOpen() const { return m_fieldBase.count() != 0; }
	bool				open();
	void				close();

	bool				create();
	bool				drop();
	bool				clear();

	int					read(void* pRecord, int key) { return read(pRecord, &key, 1); }
	int					read(void* pRecord, int* key = nullptr, int keyCount = 0);			// read record form table, if key == nullptr in the array pRecord will be record all records of table

	int					write(void* pRecord) { return write(pRecord, 1); }
	int					write(void* pRecord, int count, int key) { return write(pRecord, count, &key); }
	int					write(void* pRecord, int recordCount, int* key = nullptr);			// insert or update records (depend from key) in a table, pRecord - array of record, count - amount records

	int					remove(int key) { return remove(&key, 1); }							// remove records by key
	int					remove(const int* key, int keyCount) const;

	SqlTable&			operator=(SqlTable& from);
};

// ==============================================================================================

class Database : public QObject
{
	Q_OBJECT

public:

	explicit Database(QObject* parent = nullptr);
	virtual ~Database();

private:

	QSqlDatabase		m_database;
	SqlTable			m_table[SQL_TABLE_COUNT];

	static SqlHistoryDatabase m_history[DATABASE_VERSION + 1];

	bool				createBackup();

	void				initVersion();
	void				createTables();

public:

	bool				isOpen() const { return m_database.isOpen(); }
	bool				open();
	void				close();

	SqlTable*			openTable(int objectType);

	bool				appendMeasure(Measurement* pMeasurement);
	bool				removeMeasure(int measuteType, const QVector<int>& keyList);
};

// ==============================================================================================

extern Database theDatabase;

// ==============================================================================================

#endif // DATABASE_H
