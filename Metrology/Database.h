#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QtSql>

#include "MeasureBase.h"
#include "Options.h"

// ==============================================================================================

#define					DATABASE_VERSION		2
#define					DATABASE_NAME			"Metrology"

#define					DATABASE_SQLLITE_EXT	".db"

// ==============================================================================================
//
// !!! Any changing must change version of database and version of appropriate table !!!
//

const char* const migration[DATABASE_VERSION] =
{
	"ALTER TABLE History ADD Description VARCHAR(256);",	// 1
	"DROP TABLE History;",									// 2
};

// ==============================================================================================
//
// list of fields SQL tables.
//
class SqlFieldBase : public QSqlRecord
{
public:

	SqlFieldBase();
	virtual ~SqlFieldBase() {}

public:

	int					init(int objectType, int version);

	void				append(const QSqlField& field);
	void				append(QString name, QMetaType::Type type = QMetaType::UnknownType, int length = 0);

	QString				extFieldName(int index);
};

// ==============================================================================================

const char* const		SqlTableName[] =
{
						"DatabaseInfo",

						"LinearityMeasure",
						"LinearityMeasureAddValEl",
						"LinearityMeasureAddValEn",
						"LinearityMeasure20El",
						"LinearityMeasure20En",
						"LinearityPoint",

						"ComparatorMeasure",

						"ReportOption",
						"RackGroup",
};

const int				SQL_TABLE_COUNT							= sizeof(SqlTableName)/sizeof(SqlTableName[0]);

const int				SQL_TABLE_UNKNONW						= -1,
						SQL_TABLE_DATABASE_INFO					= 0,
						SQL_TABLE_LINEARITY						= 1,
						SQL_TABLE_LINEARITY_ADD_VAL_EL			= 2,
						SQL_TABLE_LINEARITY_ADD_VAL_EN			= 3,
						SQL_TABLE_LINEARITY_20_EL				= 4,
						SQL_TABLE_LINEARITY_20_EN				= 5,
						SQL_TABLE_LINEARITY_POINT				= 6,
						SQL_TABLE_COMPARATOR					= 7,
						SQL_TABLE_REPORT_HEADER					= 8,
						SQL_TABLE_RACK_GROUP					= 9;


// ==============================================================================================

const int				SQL_TABLE_VER_UNKNONW = -1;

// ----------------------------------------------------------------------------------------------
//
// current versions of tables or views
//
const int				SqlTableVersion[SQL_TABLE_COUNT] =
{
						DATABASE_VERSION,	//	SQL_TABLE_DATABASE_INFO

						0,					//	SQL_TABLE_LINEARITY
						0,					//	SQL_TABLE_LINEARITY_ADD_EL_VAL
						0,					//	SQL_TABLE_LINEARITY_ADD_EN_VAL
						0,					//	SQL_TABLE_LINEARITY_20_EL
						0,					//	SQL_TABLE_LINEARITY_20_EN
						0,					//	SQL_TABLE_LINEARITY_POINT

						0,					//	SQL_TABLE_COMPARATOR

						0,					//	SQL_TABLE_REPORT_HEADER
						0,					//	SQL_TABLE_RACK_GROUP
};

// ==============================================================================================

const int				SQL_OBJECT_ID_UNKNONW = -1;

// ----------------------------------------------------------------------------------------------
//
// unique object ID in the database
//
const int				SqlObjectID[SQL_TABLE_COUNT] =
{
						0,			//	SQL_TABLE_DATABASE_INFO

						100,		//	SQL_TABLE_LINEARITY
						110,		//	SQL_TABLE_LINEARITY_ADD_EL_VAL
						111,		//	SQL_TABLE_LINEARITY_ADD_EN_VAL
						120,		//	SQL_TABLE_LINEARITY_20_EL
						121,		//	SQL_TABLE_LINEARITY_20_EN
						130,		//	SQL_TABLE_LINEARITY_POINT

						200,		//	SQL_TABLE_COMPARATOR

						400,		//	SQL_TABLE_REPORT_HEADER

						500,		//	SQL_TABLE_RACK_GROUP
};

// ==============================================================================================

const int				SQL_TABLE_IS_MAIN	= 0,
						SQL_TABLE_IS_SUB	= 1,
						SQL_TABLE_IS_CONFIG	= 2;

// ----------------------------------------------------------------------------------------------

const int				SqlTableByMeasureType[SQL_TABLE_COUNT] =
{
						Measure::Type::NoMeasureType,			//	SQL_TABLE_DATABASE_INFO						// SQL_TABLE_CONFIG

						Measure::Type::Linearity,				//	SQL_TABLE_LINEARITY							// SQL_TABLE_MEASURE_MAIN
						Measure::Type::Linearity,				//	SQL_TABLE_LINEARITY_ADD_EL_VAL				// SQL_TABLE_MEASURE_SUB
						Measure::Type::Linearity,				//	SQL_TABLE_LINEARITY_ADD_EN_VAL				// SQL_TABLE_MEASURE_SUB
						Measure::Type::Linearity,				//	SQL_TABLE_LINEARITY_20_EL					// SQL_TABLE_MEASURE_SUB
						Measure::Type::Linearity,				//	SQL_TABLE_LINEARITY_20_EN					// SQL_TABLE_MEASURE_SUB

						Measure::Type::NoMeasureType,			//	SQL_TABLE_LINEARITY_POINT					// SQL_TABLE_CONFIG

						Measure::Type::Comparators,				//	SQL_TABLE_COMPARATOR						// SQL_TABLE_MEASURE_MAIN

						Measure::Type::NoMeasureType,			//	SQL_TABLE_REPORT_HEADER						// SQL_TABLE_CONFIG
						Measure::Type::NoMeasureType,			//	SQL_TABLE_RACK_GROUP						// SQL_TABLE_CONFIG
};

// ----------------------------------------------------------------------------------------------

const int				SqlTableAppointType[SQL_TABLE_COUNT] =
{
						SQL_TABLE_IS_CONFIG,				//	SQL_TABLE_DATABASE_INFO

						SQL_TABLE_IS_MAIN,					//	SQL_TABLE_LINEARITY
						SQL_TABLE_IS_SUB,					//	SQL_TABLE_LINEARITY_ADD_EL_VAL
						SQL_TABLE_IS_SUB,					//	SQL_TABLE_LINEARITY_ADD_EN_VAL
						SQL_TABLE_IS_SUB,					//	SQL_TABLE_LINEARITY_20_EL
						SQL_TABLE_IS_SUB,					//	SQL_TABLE_LINEARITY_20_EN

						SQL_TABLE_IS_CONFIG,				//	SQL_TABLE_LINEARITY_POINT

						SQL_TABLE_IS_MAIN,					//	SQL_TABLE_COMPARATOR

						SQL_TABLE_IS_CONFIG,				//	SQL_TABLE_REPORT_HEADER
						SQL_TABLE_IS_CONFIG,				//	SQL_TABLE_RACK_GROUP
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

private:

	int					m_objectType = SQL_TABLE_UNKNONW;			// type of table
	int					m_objectID = SQL_OBJECT_ID_UNKNONW;			// unique identifier of table in the database
	QString				m_caption;									// caption of table
	int					m_version = SQL_TABLE_VER_UNKNONW;			// table version, is read when the database initialization
};

// ==============================================================================================

const int				REMOVE_TRANSACTION_RECORD_COUNT = 500;

// ----------------------------------------------------------------------------------------------

class SqlTable
{
public:

	SqlTable();
	virtual ~SqlTable();

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

private:

	QSqlDatabase*		m_pDatabase = nullptr;
	SqlObjectInfo		m_info;
	SqlFieldBase		m_fieldBase;
};

// ==============================================================================================

class Database : public QObject
{
	Q_OBJECT

public:

	explicit Database(QObject* parent = nullptr);
	virtual ~Database() override;

public:

	void				setDatabaseOption(const DatabaseOption& option) { m_databaseOption = option; }

	bool				isOpen() const { return m_database.isOpen(); }
	bool				open();
	bool				openSQLite(const QString& databaseName);
	bool				openPostgres(const QString& databaseName);
	void				close();

	SqlTable*			openTable(int objectType);

	bool				appendMeasure(Measure::Item* pMeasurement);
	bool				removeMeasure(Measure::Type measuteType, const std::vector<int>& keyList);
	bool				updateMeasure(Measure::Type measuteType, const std::vector<Measure::Item*>& list);

private:

	QSqlDatabase		m_database;
	SqlTable			m_table[SQL_TABLE_COUNT];

	DatabaseOption		m_databaseOption;

	int					m_currentVersion = 0;

	bool				createBackup();

	int					initVersion();
	void				createTables();

	bool				applyMigrations();

public slots:

	void				appendToBase(Measure::Item* pMeasurement);
	void				removeFromBase(Measure::Type measureType, const std::vector<int>& keyList);
	void				updateInBase(Measure::Type measureType, const std::vector<Measure::Item*>& list);
};

// ==============================================================================================

extern Database theDatabase;

// ==============================================================================================

#endif // DATABASE_H
