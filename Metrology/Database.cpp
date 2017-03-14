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

SqlFieldBase::~SqlFieldBase()
{
}

// -------------------------------------------------------------------------------------------------------------------

int SqlFieldBase::init(int objectType, int)
{
	switch(objectType)
	{
		case SQL_TABLE_DATABASE_INFO:

			append("ID",							QVariant::Int);
			append("ObjectID",						QVariant::Int);
			append("Name",							QVariant::String, 256);
			append("Version",						QVariant::Int);

			break;

		case SQL_TABLE_HISTORY:

			append("ObjectID",						QVariant::Int);
			append("Version",						QVariant::Int);
			append("Event",							QVariant::String, 256);
			append("Time",							QVariant::String, 64);

			break;

		case SQL_TABLE_LINEARITY:

			append("ObjectID",						QVariant::Int);
			append("MeasureID",						QVariant::Int);

			append("Filter",						QVariant::Bool);

			append("AppSignalID",					QVariant::String, 64);
			append("CustomAppSignalID",				QVariant::String, 64);
			append("Caption",						QVariant::String, 256);

			append("EquipmentID",					QVariant::String, 256);
			append("RackIndex",						QVariant::Int);
			append("RackCaption",					QVariant::String, 64);
			append("Channel",						QVariant::Int);
			append("Chassis",						QVariant::Int);
			append("Module",						QVariant::Int);
			append("Place",							QVariant::Int);

			append("PrecentFormLimit",				QVariant::Double);

			append("InElectricNominal",				QVariant::Double);
			append("InElectricMeasure",				QVariant::Double);

			append("PhysicalNominal",				QVariant::Double);
			append("PhysicalMeasure",				QVariant::Double);

			append("OutElectricNominal",			QVariant::Double);
			append("OutElectricMeasure",			QVariant::Double);

			append("InElectricHasLimit",			QVariant::Bool);
			append("InElectricLowLimit",			QVariant::Double);
			append("InElectricHighLimit",			QVariant::Double);
			append("InElectricUnit",				QVariant::String, 32);
			append("InElectricPrecision",			QVariant::Int);

			append("PhysicalHasLimit",				QVariant::Bool);
			append("PhysicalLowLimit",				QVariant::Double);
			append("PhysicalHighLimit",				QVariant::Double);
			append("PhysicalUnit",					QVariant::String, 32);
			append("PhysicalPrecision",				QVariant::Int);

			append("OutElectricHasLimit",			QVariant::Bool);
			append("OutElectricLowLimit",			QVariant::Double);
			append("OutElectricHighLimit",			QVariant::Double);
			append("OutElectricUnit",				QVariant::String, 32);
			append("OutElectricPrecision",			QVariant::Int);

			append("InElectricErrorAbsolute",		QVariant::Double);
			append("InElectricErrorReduce",			QVariant::Double);
			append("InElectricLimitErrorAbsolute",	QVariant::Double);
			append("InElectricLimitErrorReduce",	QVariant::Double);

			append("PhysicalErrorAbsolute",			QVariant::Double);
			append("PhysicalErrorReduce",			QVariant::Double);
			append("PhysicalLimitErrorAbsolute",	QVariant::Double);
			append("PhysicalLimitErrorReduce",		QVariant::Double);

			append("OutElectricErrorAbsolute",		QVariant::Double);
			append("OutElectricErrorReduce",		QVariant::Double);
			append("OutElectricLimitErrorAbsolute",	QVariant::Double);
			append("OutElectricLimitErrorReduce",	QVariant::Double);

			append("MeasureTime",					QVariant::String, 64);

			break;

		case SQL_TABLE_LINEARITY_20_EL:
		case SQL_TABLE_LINEARITY_20_PH:

			append("ObjectID",						QVariant::Int);
			append("MeasureID",						QVariant::Int);

			append(QString("MeasurementCount"),		QVariant::Int);

			append(QString("Measurement0"),			QVariant::Double);
			append(QString("Measurement1"),			QVariant::Double);
			append(QString("Measurement2"),			QVariant::Double);
			append(QString("Measurement3"),			QVariant::Double);
			append(QString("Measurement4"),			QVariant::Double);
			append(QString("Measurement5"),			QVariant::Double);
			append(QString("Measurement6"),			QVariant::Double);
			append(QString("Measurement7"),			QVariant::Double);
			append(QString("Measurement8"),			QVariant::Double);
			append(QString("Measurement9"),			QVariant::Double);
			append(QString("Measurement10"),		QVariant::Double);
			append(QString("Measurement11"),		QVariant::Double);
			append(QString("Measurement12"),		QVariant::Double);
			append(QString("Measurement13"),		QVariant::Double);
			append(QString("Measurement14"),		QVariant::Double);
			append(QString("Measurement15"),		QVariant::Double);
			append(QString("Measurement16"),		QVariant::Double);
			append(QString("Measurement17"),		QVariant::Double);
			append(QString("Measurement18"),		QVariant::Double);
			append(QString("Measurement19"),		QVariant::Double);

			break;

		case SQL_TABLE_LINEARITY_ADD_VAL:

			append("ObjectID",						QVariant::Int);
			append("MeasureID",						QVariant::Int);

			append(QString("ValueCount"),			QVariant::Int);

			append(QString("Value0"),				QVariant::Double);
			append(QString("Value1"),				QVariant::Double);
			append(QString("Value2"),				QVariant::Double);
			append(QString("Value3"),				QVariant::Double);
			append(QString("Value4"),				QVariant::Double);
			append(QString("Value5"),				QVariant::Double);
			append(QString("Value6"),				QVariant::Double);
			append(QString("Value7"),				QVariant::Double);
			append(QString("Value8"),				QVariant::Double);
			append(QString("Value9"),				QVariant::Double);
			append(QString("Value10"),				QVariant::Double);
			append(QString("Value11"),				QVariant::Double);
			append(QString("Value12"),				QVariant::Double);
			append(QString("Value13"),				QVariant::Double);
			append(QString("Value14"),				QVariant::Double);
			append(QString("Value15"),				QVariant::Double);

			break;

		case SQL_TABLE_LINEARITY_POINT:

			append("ObjectID",						QVariant::Int);
			append("PointID",						QVariant::Int);

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
			append("PointID",						QVariant::Int);

			break;

		case SQL_TABLE_COMPLEX_COMPARATOR_SIGNAL:

			append("ObjectID",						QVariant::Int);
			append("SignalID",						QVariant::Int);

			break;

		case SQL_TABLE_REPORT_HEADER:

			append("ObjectID",						QVariant::Int);
			append("ReportID",						QVariant::Int);


			append("DocumentTitle",					QVariant::String, 256);
			append("ReportTitle",					QVariant::String, 256);
			append("Date",							QVariant::String, 32);
			append("TableTitle",					QVariant::String, 256);
			append("Conclusion",					QVariant::String, 256);

			append("Temperature",					QVariant::Double);
			append("Pressure",						QVariant::Double);
			append("Humidity",						QVariant::Double);
			append("Voltage",						QVariant::Double);
			append("Frequency",						QVariant::Double);

			append("Calibrator0",					QVariant::String, 64);
			append("Calibrator1",					QVariant::String, 64);
			append("Calibrator2",					QVariant::String, 64);
			append("Calibrator3",					QVariant::String, 64);
			append("Calibrator4",					QVariant::String, 64);
			append("Calibrator5",					QVariant::String, 64);

			append("LinkObjectID",					QVariant::Int);
			append("ReportFile",					QVariant::String, 256);

			append("Param",							QVariant::Int);

			break;

		case SQL_TABLE_RACK_GROUP:

			append("ObjectID",						QVariant::Int);
			append("GroupIndex",					QVariant::Int);

			append("Caption",						QVariant::String, 64);

			append("RackID0",						QVariant::String, 64);
			append("RackID1",						QVariant::String, 64);
			append("RackID2",						QVariant::String, 64);
			append("RackID3",						QVariant::String, 64);
			append("RackID4",						QVariant::String, 64);
			append("RackID5",						QVariant::String, 64);

			break;

		case SQL_TABLE_OUTPUT_SIGNAL:

			append("ObjectID",						QVariant::Int);
			append("SignalID",						QVariant::Int);

			append("OutputSignalType",				QVariant::Int);

			append("InputAppSignalID",				QVariant::String, 64);
			append("OutputAppSignalID",				QVariant::String, 64);

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

	if (type == QVariant::Double)
	{
		field.setPrecision(9);
	}

	if (type == QVariant::String)
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
		case QVariant::Bool:	result = QString("%1 BOOL").arg(f.name());									break;
		case QVariant::Int:		result = QString("%1 INTEGER").arg(f.name());								break;
		case QVariant::Double:	result = QString("%1 DOUBLE(0, %2)").arg(f.name()).arg(f.precision());		break;
		case QVariant::String:	result = QString("%1 VARCHAR(%2)").arg(f.name()).arg(f.length());			break;
		default:				result.clear();
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

SqlObjectInfo::~SqlObjectInfo()
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
	m_caption = SqlTabletName[objectType];
	m_version = SqlTableVersion[objectType];

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void SqlObjectInfo::clear()
{
	m_objectType = SQL_TABLE_UNKNONW;
	m_objectID = SQL_OBJECT_ID_UNKNONW;
	m_caption.clear();
	m_version = SQL_TABLE_VER_UNKNONW;
}

// -------------------------------------------------------------------------------------------------------------------

SqlObjectInfo& SqlObjectInfo::operator=(SqlObjectInfo& from)
{
	m_objectType = from.m_objectType;
	m_objectID = from.m_objectID;
	m_caption = from.m_caption;
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

SqlHistoryDatabase::~SqlHistoryDatabase()
{
}

// -------------------------------------------------------------------------------------------------------------------

SqlHistoryDatabase::SqlHistoryDatabase(int objectID, int version, const QString& event,  const QString& time)
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

SqlTable::~SqlTable()
{
}

// -------------------------------------------------------------------------------------------------------------------

int SqlTable::recordCount() const
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

	QSqlQuery query(QString("SELECT count(*) FROM %1").arg(m_info.caption()));
	if (query.next() == false)
	{
		return 0;
	}

	return query.value(0).toInt();
}

// -------------------------------------------------------------------------------------------------------------------

int SqlTable::lastKey() const
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

	QSqlQuery query(QString("SELECT max(%1) FROM %2").arg(m_fieldBase.field(SQL_FIELD_KEY).name()).arg(m_info.caption()));
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

bool SqlTable::isExist() const
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
		if (m_pDatabase->tables().at(et).compare(SqlTabletName[type]) == 0)
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

	QString request = QString("CREATE TABLE if not exists %1 (").arg(m_info.caption());

	int filedCount = m_fieldBase.count();
	for(int field = 0; field < filedCount; field++)
	{
		request.append(m_fieldBase.extFieldName(field));

		if (field == SQL_FIELD_KEY)
		{
			request.append(" PRIMARY KEY NOT NULL");

			switch(m_info.objectType())
			{
				case SQL_TABLE_LINEARITY_20_EL:
				case SQL_TABLE_LINEARITY_20_PH:
				case SQL_TABLE_LINEARITY_ADD_VAL:
					request.append(QString(" REFERENCES %1(MeasureID) ON DELETE CASCADE").arg(SqlTabletName[SQL_TABLE_LINEARITY]));
					break;
			}
		}

		if (field != filedCount - 1)
		{
			request.append(", ");
		}
	}

	request.append(");");

	return query.exec(request);
}

// -------------------------------------------------------------------------------------------------------------------

bool SqlTable::drop()
{
	QSqlQuery query;
	if (query.exec(QString("DROP TABLE %1").arg(m_info.caption())) == false)
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

	if (query.exec(QString("DELETE FROM %1").arg(m_info.caption())) == false)
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
	QString request = QString("SELECT * FROM %1").arg(m_info.caption());

	if (key != nullptr && keyCount != 0)
	{
		request.append(" WHERE ");
		QString keyFieldName = m_fieldBase.field(SQL_FIELD_KEY).name();

		for(int k = 0; k < keyCount; k++)
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
					info->setCaption(query.value(field++).toString());
					info->setVersion(query.value(field++).toInt());
				}
				break;

			case SQL_TABLE_HISTORY:
				{
					SqlHistoryDatabase* history = static_cast<SqlHistoryDatabase*> (pRecord) + readedCount;

					history->setObjectID(objectID);
					history->setVersion(query.value(field++).toInt());
					history->setEvent(query.value(field++).toString());
					history->setTime(query.value(field++).toString());
				}
				break;

			case SQL_TABLE_LINEARITY:
				{
					LinearityMeasurement* measure = static_cast<LinearityMeasurement*> (pRecord) + readedCount;

					measure->setMeasureID(query.value(field++).toInt());

					measure->setFilter(query.value(field++).toBool());

					measure->setAppSignalID(query.value(field++).toString());
					measure->setCustomAppSignalID(query.value(field++).toString());
					measure->setCaption(query.value(field++).toString());

					measure->location().setEquipmentID(query.value(field++).toString());
					measure->location().rack().setIndex(query.value(field++).toInt());
					measure->location().rack().setCaption(query.value(field++).toString());
					measure->location().rack().setChannel(query.value(field++).toInt());
					measure->location().setChassis(query.value(field++).toInt());
					measure->location().setModule(query.value(field++).toInt());
					measure->location().setPlace(query.value(field++).toInt());

					measure->setPercent(query.value(field++).toDouble());

					measure->setNominal(VALUE_TYPE_IN_ELECTRIC, query.value(field++).toDouble());
					measure->setMeasure(VALUE_TYPE_IN_ELECTRIC, query.value(field++).toDouble());

					measure->setNominal(VALUE_TYPE_PHYSICAL, query.value(field++).toDouble());
					measure->setMeasure(VALUE_TYPE_PHYSICAL, query.value(field++).toDouble());

					measure->setNominal(VALUE_TYPE_OUT_ELECTRIC, query.value(field++).toDouble());
					measure->setMeasure(VALUE_TYPE_OUT_ELECTRIC, query.value(field++).toDouble());

					measure->setHasLimit(VALUE_TYPE_IN_ELECTRIC, query.value(field++).toBool());
					measure->setLowLimit(VALUE_TYPE_IN_ELECTRIC, query.value(field++).toDouble());
					measure->setHighLimit(VALUE_TYPE_IN_ELECTRIC, query.value(field++).toDouble());
					measure->setUnit(VALUE_TYPE_IN_ELECTRIC, query.value(field++).toString());
					measure->setLimitPrecision(VALUE_TYPE_IN_ELECTRIC, query.value(field++).toInt());

					measure->setHasLimit(VALUE_TYPE_PHYSICAL, query.value(field++).toBool());
					measure->setLowLimit(VALUE_TYPE_PHYSICAL, query.value(field++).toDouble());
					measure->setHighLimit(VALUE_TYPE_PHYSICAL, query.value(field++).toDouble());
					measure->setUnit(VALUE_TYPE_PHYSICAL, query.value(field++).toString());
					measure->setLimitPrecision(VALUE_TYPE_PHYSICAL, query.value(field++).toInt());

					measure->setHasLimit(VALUE_TYPE_OUT_ELECTRIC, query.value(field++).toBool());
					measure->setLowLimit(VALUE_TYPE_OUT_ELECTRIC, query.value(field++).toDouble());
					measure->setHighLimit(VALUE_TYPE_OUT_ELECTRIC, query.value(field++).toDouble());
					measure->setUnit(VALUE_TYPE_OUT_ELECTRIC, query.value(field++).toString());
					measure->setLimitPrecision(VALUE_TYPE_OUT_ELECTRIC, query.value(field++).toInt());

					measure->setError(VALUE_TYPE_IN_ELECTRIC, MEASURE_ERROR_TYPE_ABSOLUTE, query.value(field++).toDouble());
					measure->setError(VALUE_TYPE_IN_ELECTRIC, MEASURE_ERROR_TYPE_REDUCE, query.value(field++).toDouble());
					measure->setErrorLimit(VALUE_TYPE_IN_ELECTRIC, MEASURE_ERROR_TYPE_ABSOLUTE, query.value(field++).toDouble());
					measure->setErrorLimit(VALUE_TYPE_IN_ELECTRIC, MEASURE_ERROR_TYPE_REDUCE, query.value(field++).toDouble());

					measure->setError(VALUE_TYPE_PHYSICAL, MEASURE_ERROR_TYPE_ABSOLUTE, query.value(field++).toDouble());
					measure->setError(VALUE_TYPE_PHYSICAL, MEASURE_ERROR_TYPE_REDUCE, query.value(field++).toDouble());
					measure->setErrorLimit(VALUE_TYPE_PHYSICAL, MEASURE_ERROR_TYPE_ABSOLUTE, query.value(field++).toDouble());
					measure->setErrorLimit(VALUE_TYPE_PHYSICAL, MEASURE_ERROR_TYPE_REDUCE, query.value(field++).toDouble());

					measure->setError(VALUE_TYPE_OUT_ELECTRIC, MEASURE_ERROR_TYPE_REDUCE, query.value(field++).toDouble());
					measure->setError(VALUE_TYPE_OUT_ELECTRIC, MEASURE_ERROR_TYPE_ABSOLUTE, query.value(field++).toDouble());
					measure->setErrorLimit(VALUE_TYPE_OUT_ELECTRIC, MEASURE_ERROR_TYPE_ABSOLUTE, query.value(field++).toDouble());
					measure->setErrorLimit(VALUE_TYPE_OUT_ELECTRIC, MEASURE_ERROR_TYPE_REDUCE, query.value(field++).toDouble());

					measure->setMeasureTime(QDateTime::fromString(query.value(field++).toString(), MEASURE_TIME_FORMAT));
				}
				break;

			case SQL_TABLE_LINEARITY_20_EL:
			case SQL_TABLE_LINEARITY_20_PH:
				{
					int valueType = VALUE_TYPE_UNKNOWN;

					switch(m_info.objectType())
					{
						case SQL_TABLE_LINEARITY_20_EL:	valueType = VALUE_TYPE_IN_ELECTRIC;	break;
						case SQL_TABLE_LINEARITY_20_PH:	valueType = VALUE_TYPE_PHYSICAL;	break;
						default:						valueType = VALUE_TYPE_UNKNOWN;		break;
					}

					if (valueType == VALUE_TYPE_UNKNOWN)
					{
						break;
					}

					LinearityMeasurement* measure = static_cast<LinearityMeasurement*> (pRecord) + readedCount;

					measure->setMeasureID(query.value(field++).toInt());

					measure->setMeasureCount(query.value(field++).toInt());

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

			case SQL_TABLE_LINEARITY_ADD_VAL:
				{
					LinearityMeasurement* measure = static_cast<LinearityMeasurement*> (pRecord) + readedCount;

					measure->setMeasureID(query.value(field++).toInt());

					measure->setAdditionalParamCount(query.value(field++).toInt());

					measure->setAdditionalParam(MEASURE_ADDITIONAL_PARAM_MAX_VALUE, query.value(field++).toDouble());
					measure->setAdditionalParam(MEASURE_ADDITIONAL_PARAM_SYSTEM_ERROR, query.value(field++).toDouble());
					measure->setAdditionalParam(MEASURE_ADDITIONAL_PARAM_MSE, query.value(field++).toDouble());
					measure->setAdditionalParam(MEASURE_ADDITIONAL_PARAM_LOW_HIGH_BORDER, query.value(field++).toDouble());
				}
				break;

			case SQL_TABLE_LINEARITY_POINT:
				{
					LinearityPoint* point = static_cast<LinearityPoint*> (pRecord) + readedCount;

					point->setIndex(query.value(field++).toInt());
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

					header->m_calibrator[CHANNEL_0] = query.value(field++).toString();
					header->m_calibrator[CHANNEL_1] = query.value(field++).toString();
					header->m_calibrator[CHANNEL_2] = query.value(field++).toString();
					header->m_calibrator[CHANNEL_3] = query.value(field++).toString();
					header->m_calibrator[CHANNEL_4] = query.value(field++).toString();
					header->m_calibrator[CHANNEL_5] = query.value(field++).toString();

					header->m_linkObjectID = query.value(field++).toInt();
					header->m_reportFile = query.value(field++).toString();

					header->m_param = query.value(field++).toInt();
				}
				break;

			case SQL_TABLE_RACK_GROUP:
				{
					RackGroup* group = static_cast<RackGroup*> (pRecord) + readedCount;

					group->setIndex(query.value(field++).toInt());
					group->setCaption(query.value(field++).toString());

					group->setRackID(CHANNEL_0, query.value(field++).toString());
					group->setRackID(CHANNEL_1, query.value(field++).toString());
					group->setRackID(CHANNEL_2, query.value(field++).toString());
					group->setRackID(CHANNEL_3, query.value(field++).toString());
					group->setRackID(CHANNEL_4, query.value(field++).toString());
					group->setRackID(CHANNEL_5, query.value(field++).toString());
				}
				break;

			case SQL_TABLE_OUTPUT_SIGNAL:
				{
					OutputSignal* signal = static_cast<OutputSignal*> (pRecord) + readedCount;

					signal->setSignalID(query.value(field++).toInt());

					signal->setType(query.value(field++).toInt());

					signal->setAppSignalID(MEASURE_IO_SIGNAL_TYPE_INPUT, query.value(field++).toString());
					signal->setAppSignalID(MEASURE_IO_SIGNAL_TYPE_OUTPUT, query.value(field++).toString());
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
		request = QString("INSERT INTO %1 (").arg(m_info.caption());

		int filedCount = m_fieldBase.count();
		for(int f = 0; f < filedCount; f++)
		{
			request.append(m_fieldBase.field(f).name());

			if (f != filedCount - 1)
			{
				request.append(", ");
			}
		}

		request.append(") VALUES (");

		for(int f = 0; f < filedCount; f++)
		{
			request.append("?");

			if (f != filedCount - 1)
			{
				request.append(", ");
			}
		}

		request.append(");");
	}
	else
	{
		request = QString("UPDATE %1 SET ").arg(m_info.caption());

		int filedCount = m_fieldBase.count();
		for(int f = 0; f < filedCount; f++)
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
					query.bindValue(field++, info->caption());
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

			case SQL_TABLE_LINEARITY:
				{
					LinearityMeasurement* measure = static_cast<LinearityMeasurement*> (pRecord) + r;

					measure->setMeasureID(lastKey() + 1);

					query.bindValue(field++, measure->measureID());

					query.bindValue(field++, measure->filter());

					query.bindValue(field++, measure->appSignalID());
					query.bindValue(field++, measure->customAppSignalID());
					query.bindValue(field++, measure->caption());

					query.bindValue(field++, measure->location().equipmentID());
					query.bindValue(field++, measure->location().rack().index());
					query.bindValue(field++, measure->location().rack().caption());
					query.bindValue(field++, measure->location().rack().channel());
					query.bindValue(field++, measure->location().chassis());
					query.bindValue(field++, measure->location().module());
					query.bindValue(field++, measure->location().place());

					query.bindValue(field++, measure->percent());

					query.bindValue(field++, measure->nominal(VALUE_TYPE_IN_ELECTRIC));
					query.bindValue(field++, measure->measure(VALUE_TYPE_IN_ELECTRIC));

					query.bindValue(field++, measure->nominal(VALUE_TYPE_PHYSICAL));
					query.bindValue(field++, measure->measure(VALUE_TYPE_PHYSICAL));

					query.bindValue(field++, measure->nominal(VALUE_TYPE_OUT_ELECTRIC));
					query.bindValue(field++, measure->measure(VALUE_TYPE_OUT_ELECTRIC));

					query.bindValue(field++, measure->hasLimit(VALUE_TYPE_IN_ELECTRIC));
					query.bindValue(field++, measure->lowLimit(VALUE_TYPE_IN_ELECTRIC));
					query.bindValue(field++, measure->highLimit(VALUE_TYPE_IN_ELECTRIC));
					query.bindValue(field++, measure->unit(VALUE_TYPE_IN_ELECTRIC));
					query.bindValue(field++, measure->limitPrecision(VALUE_TYPE_IN_ELECTRIC));

					query.bindValue(field++, measure->hasLimit(VALUE_TYPE_PHYSICAL));
					query.bindValue(field++, measure->lowLimit(VALUE_TYPE_PHYSICAL));
					query.bindValue(field++, measure->highLimit(VALUE_TYPE_PHYSICAL));
					query.bindValue(field++, measure->unit(VALUE_TYPE_PHYSICAL));
					query.bindValue(field++, measure->limitPrecision(VALUE_TYPE_PHYSICAL));

					query.bindValue(field++, measure->hasLimit(VALUE_TYPE_OUT_ELECTRIC));
					query.bindValue(field++, measure->lowLimit(VALUE_TYPE_OUT_ELECTRIC));
					query.bindValue(field++, measure->highLimit(VALUE_TYPE_OUT_ELECTRIC));
					query.bindValue(field++, measure->unit(VALUE_TYPE_OUT_ELECTRIC));
					query.bindValue(field++, measure->limitPrecision(VALUE_TYPE_OUT_ELECTRIC));

					query.bindValue(field++, measure->error(VALUE_TYPE_IN_ELECTRIC, MEASURE_ERROR_TYPE_ABSOLUTE));
					query.bindValue(field++, measure->error(VALUE_TYPE_IN_ELECTRIC, MEASURE_ERROR_TYPE_REDUCE));
					query.bindValue(field++, measure->errorLimit(VALUE_TYPE_IN_ELECTRIC, MEASURE_ERROR_TYPE_ABSOLUTE));
					query.bindValue(field++, measure->errorLimit(VALUE_TYPE_IN_ELECTRIC, MEASURE_ERROR_TYPE_REDUCE));

					query.bindValue(field++, measure->error(VALUE_TYPE_PHYSICAL, MEASURE_ERROR_TYPE_ABSOLUTE));
					query.bindValue(field++, measure->error(VALUE_TYPE_PHYSICAL, MEASURE_ERROR_TYPE_REDUCE));
					query.bindValue(field++, measure->errorLimit(VALUE_TYPE_PHYSICAL, MEASURE_ERROR_TYPE_ABSOLUTE));
					query.bindValue(field++, measure->errorLimit(VALUE_TYPE_PHYSICAL, MEASURE_ERROR_TYPE_REDUCE));

					query.bindValue(field++, measure->error(VALUE_TYPE_OUT_ELECTRIC, MEASURE_ERROR_TYPE_ABSOLUTE));
					query.bindValue(field++, measure->error(VALUE_TYPE_OUT_ELECTRIC, MEASURE_ERROR_TYPE_REDUCE));
					query.bindValue(field++, measure->errorLimit(VALUE_TYPE_OUT_ELECTRIC, MEASURE_ERROR_TYPE_ABSOLUTE));
					query.bindValue(field++, measure->errorLimit(VALUE_TYPE_OUT_ELECTRIC, MEASURE_ERROR_TYPE_REDUCE));

					measure->setMeasureTime(QDateTime::currentDateTime());

					query.bindValue(field++, measure->measureTime().toString(MEASURE_TIME_FORMAT));

				}
				break;

			case SQL_TABLE_LINEARITY_20_EL:
			case SQL_TABLE_LINEARITY_20_PH:
				{
					int valueType = VALUE_TYPE_UNKNOWN;

					switch(m_info.objectType())
					{
						case SQL_TABLE_LINEARITY_20_EL:	valueType = VALUE_TYPE_IN_ELECTRIC;	break;
						case SQL_TABLE_LINEARITY_20_PH:	valueType = VALUE_TYPE_PHYSICAL;	break;
						default:						valueType = VALUE_TYPE_UNKNOWN;		break;
					}

					if (valueType == VALUE_TYPE_UNKNOWN)
					{
						break;
					}

					LinearityMeasurement* measure = static_cast<LinearityMeasurement*> (pRecord) + r;

					query.bindValue(field++, measure->measureID());

					query.bindValue(field++, measure->measureCount());

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

			case SQL_TABLE_LINEARITY_ADD_VAL:
				{
					LinearityMeasurement* measure = static_cast<LinearityMeasurement*> (pRecord) + r;

					query.bindValue(field++, measure->measureID());

					query.bindValue(field++, measure->additionalParamCount());

					query.bindValue(field++, measure->additionalParam(MEASURE_ADDITIONAL_PARAM_MAX_VALUE));
					query.bindValue(field++, measure->additionalParam(MEASURE_ADDITIONAL_PARAM_SYSTEM_ERROR));
					query.bindValue(field++, measure->additionalParam(MEASURE_ADDITIONAL_PARAM_MSE));
					query.bindValue(field++, measure->additionalParam(MEASURE_ADDITIONAL_PARAM_LOW_HIGH_BORDER));
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
					query.bindValue(field++, 0);
					query.bindValue(field++, 0);
				}
				break;

			case SQL_TABLE_LINEARITY_POINT:
				{
					LinearityPoint* point = static_cast<LinearityPoint*> (pRecord) + r;

					query.bindValue(field++, point->Index());
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

					query.bindValue(field++, header->m_calibrator[CHANNEL_0]);
					query.bindValue(field++, header->m_calibrator[CHANNEL_1]);
					query.bindValue(field++, header->m_calibrator[CHANNEL_2]);
					query.bindValue(field++, header->m_calibrator[CHANNEL_3]);
					query.bindValue(field++, header->m_calibrator[CHANNEL_4]);
					query.bindValue(field++, header->m_calibrator[CHANNEL_5]);

					query.bindValue(field++, header->m_linkObjectID);
					query.bindValue(field++, header->m_reportFile);

					query.bindValue(field++, header->m_param);
				}
				break;

			case SQL_TABLE_RACK_GROUP:
				{
					RackGroup* group = static_cast<RackGroup*> (pRecord) + r;

					query.bindValue(field++, group->Index());
					query.bindValue(field++, group->caption());

					query.bindValue(field++, group->rackID(CHANNEL_0));
					query.bindValue(field++, group->rackID(CHANNEL_1));
					query.bindValue(field++, group->rackID(CHANNEL_2));
					query.bindValue(field++, group->rackID(CHANNEL_3));
					query.bindValue(field++, group->rackID(CHANNEL_4));
					query.bindValue(field++, group->rackID(CHANNEL_5));
				}
				break;

			case SQL_TABLE_OUTPUT_SIGNAL:
				{
					OutputSignal* signal = static_cast<OutputSignal*> (pRecord) + r;

					signal->setSignalID(lastKey() + 1);
					query.bindValue(field++, signal->signalID());

					query.bindValue(field++, signal->type());

					query.bindValue(field++, signal->appSignalID(MEASURE_IO_SIGNAL_TYPE_INPUT));
					query.bindValue(field++, signal->appSignalID(MEASURE_IO_SIGNAL_TYPE_OUTPUT));
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

int SqlTable::remove(const int* key, int keyCount) const
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

	QString request = QString("DELETE FROM %1 WHERE ").arg(m_info.caption());
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

	if (query.exec(request) == false)
	{
		query.exec("END TRANSACTION");

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
	QString path = theOptions.database().path();
	if (path.isEmpty() == true)
	{
		QMessageBox::critical(nullptr, tr("Database"), tr("Invalid path!"));
		return false;
	}

	switch(theOptions.database().type())
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

	if (table.isExist() == false)
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
						m_table[t].info().setVersion(info[i].version());
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
					QMessageBox::critical(nullptr, tr("Database"), tr("Cannot create table: %1").arg(table.info().caption()));
				}
			}
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool Database::appendMeasure(Measurement* pMeasurement)
{
	if (pMeasurement == nullptr)
	{
		return false;
	}

	int measureType = pMeasurement->measureType();
	if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
	{
		return false;
	}

	bool result = false;

	for (int type = 0; type < SQL_TABLE_COUNT; type++)
	{
		if (SqlTableByMeasureType[type] != measureType)
		{
			continue;
		}

		SqlTable& table = m_table[type];

		if (table.open() == true)
		{
			if (table.write(pMeasurement) == 1)
			{
				result = true;
			}

			table.close();
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

bool Database::removeMeasure(int measuteType, const QVector<int>& keyList)
{
	bool result = false;

	for (int type = 0; type < SQL_TABLE_COUNT; type++)
	{
		if (SqlTableByMeasureType[type] != measuteType)
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

