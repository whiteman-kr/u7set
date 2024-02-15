#include "Database.h"
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlError>

// -------------------------------------------------------------------------------------------------------------------

Database theDatabase;

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

			append("ID",							QMetaType::Int);
			append("ObjectID",						QMetaType::Int);
			append("Name",							QMetaType::QString, 256);
			append("Version",						QMetaType::Int);

			break;

		case SQL_TABLE_LINEARITY:

			append("ObjectID",						QMetaType::Int);
			append("MeasureID",						QMetaType::Int);

			append("Filter",						QMetaType::Bool);
			append("Valid",							QMetaType::Bool);

			append("ConnectSignalID",				QMetaType::QString, 64);
			append("ConnectType",					QMetaType::Int);

			append("AppSignalID",					QMetaType::QString, 64);
			append("CustomAppSignalID",				QMetaType::QString, 64);
			append("EquipmentID",					QMetaType::QString, 256);
			append("Caption",						QMetaType::QString, 256);

			append("ModuleSN",						QMetaType::Int);
			append("ModuleCaption",					QMetaType::QString, 64);
			append("RackIndex",						QMetaType::Int);
			append("RackCaption",					QMetaType::QString, 64);
			append("Channel",						QMetaType::Int);
			append("Chassis",						QMetaType::Int);
			append("Module",						QMetaType::Int);
			append("Place",							QMetaType::Int);

			append("CalibratorPrecision",			QMetaType::Int);

			append("PrecentFormLimit",				QMetaType::Double);

			append("ElectricNominal",				QMetaType::Double);
			append("ElectricMeasure",				QMetaType::Double);

			append("EngineeringNominal",			QMetaType::Double);
			append("EngineeringMeasure",			QMetaType::Double);

			append("ElectricLowLimit",				QMetaType::Double);
			append("ElectricHighLimit",				QMetaType::Double);
			append("ElectricUnit",					QMetaType::QString, 32);
			append("ElectricPrecision",				QMetaType::Int);

			append("EngineeringLowLimit",			QMetaType::Double);
			append("EngineeringHighLimit",			QMetaType::Double);
			append("EngineeringUnit",				QMetaType::QString, 32);
			append("EngineeringPrecision",			QMetaType::Int);

			append("ElectricErrorAbsolute",			QMetaType::Double);
			append("ElectricErrorReduce",			QMetaType::Double);
			append("ElectricErrorRelative",			QMetaType::Double);
			append("ElectricLimitErrorAbsolute",	QMetaType::Double);
			append("ElectricLimitErrorReduce",		QMetaType::Double);
			append("ElectricLimitErrorRelative",	QMetaType::Double);

			append("EngineeringErrorAbsolute",		QMetaType::Double);
			append("EngineeringErrorReduce",		QMetaType::Double);
			append("EngineeringErrorRelative",		QMetaType::Double);
			append("EngineeringLimitErrorAbsolute",	QMetaType::Double);
			append("EngineeringLimitErrorReduce",	QMetaType::Double);
			append("EngineeringLimitErrorRelative",	QMetaType::Double);

			append("MeasureTime",					QMetaType::QTime);
			append("Calibrator",					QMetaType::QString, 64);

			break;

		case SQL_TABLE_LINEARITY_ADD_VAL_EL:
		case SQL_TABLE_LINEARITY_ADD_VAL_EN:

			append("ObjectID",						QMetaType::Int);
			append("MeasureID",						QMetaType::Int);

			append(QString("ValueCount"),			QMetaType::Int);

			append(QString("Value0"),				QMetaType::Double);
			append(QString("Value1"),				QMetaType::Double);
			append(QString("Value2"),				QMetaType::Double);
			append(QString("Value3"),				QMetaType::Double);
			append(QString("Value4"),				QMetaType::Double);
			append(QString("Value5"),				QMetaType::Double);
			append(QString("Value6"),				QMetaType::Double);
			append(QString("Value7"),				QMetaType::Double);
			append(QString("Value8"),				QMetaType::Double);
			append(QString("Value9"),				QMetaType::Double);
			append(QString("Value10"),				QMetaType::Double);
			append(QString("Value11"),				QMetaType::Double);
			append(QString("Value12"),				QMetaType::Double);
			append(QString("Value13"),				QMetaType::Double);
			append(QString("Value14"),				QMetaType::Double);
			append(QString("Value15"),				QMetaType::Double);

			break;

		case SQL_TABLE_LINEARITY_20_EL:
		case SQL_TABLE_LINEARITY_20_EN:

			append("ObjectID",						QMetaType::Int);
			append("MeasureID",						QMetaType::Int);

			append(QString("MeasurementCount"),		QMetaType::Int);

			append(QString("Measurement0"),			QMetaType::Double);
			append(QString("Measurement1"),			QMetaType::Double);
			append(QString("Measurement2"),			QMetaType::Double);
			append(QString("Measurement3"),			QMetaType::Double);
			append(QString("Measurement4"),			QMetaType::Double);
			append(QString("Measurement5"),			QMetaType::Double);
			append(QString("Measurement6"),			QMetaType::Double);
			append(QString("Measurement7"),			QMetaType::Double);
			append(QString("Measurement8"),			QMetaType::Double);
			append(QString("Measurement9"),			QMetaType::Double);
			append(QString("Measurement10"),		QMetaType::Double);
			append(QString("Measurement11"),		QMetaType::Double);
			append(QString("Measurement12"),		QMetaType::Double);
			append(QString("Measurement13"),		QMetaType::Double);
			append(QString("Measurement14"),		QMetaType::Double);
			append(QString("Measurement15"),		QMetaType::Double);
			append(QString("Measurement16"),		QMetaType::Double);
			append(QString("Measurement17"),		QMetaType::Double);
			append(QString("Measurement18"),		QMetaType::Double);
			append(QString("Measurement19"),		QMetaType::Double);

			break;

		case SQL_TABLE_LINEARITY_POINT:

			append("ObjectID",						QMetaType::Int);
			append("PointID",						QMetaType::Int);

			append("PercentValue",					QMetaType::Double);

			break;

		case SQL_TABLE_COMPARATOR:

			append("ObjectID",						QMetaType::Int);
			append("MeasureID",						QMetaType::Int);

			append("Filter",						QMetaType::Bool);
			append("Valid",							QMetaType::Bool);

			append("ConnectSignalID",				QMetaType::QString, 64);
			append("ConnectType",					QMetaType::Int);

			append("AppSignalID",					QMetaType::QString, 64);
			append("CustomAppSignalID",				QMetaType::QString, 64);
			append("EquipmentID",					QMetaType::QString, 256);
			append("Caption",						QMetaType::QString, 256);

			append("ModuleSN",						QMetaType::Int);
			append("ModuleCaption",					QMetaType::QString, 64);
			append("RackIndex",						QMetaType::Int);
			append("RackCaption",					QMetaType::QString, 64);
			append("Channel",						QMetaType::Int);
			append("Chassis",						QMetaType::Int);
			append("Module",						QMetaType::Int);
			append("Place",							QMetaType::Int);

			append("CalibratorPrecision",			QMetaType::Int);

			append("CompareAppSignalID"	,			QMetaType::QString, 64);
			append("OutputAppSignalID",				QMetaType::QString, 64);

			append("CmpValueType",					QMetaType::Int);
			append("CmpType",						QMetaType::Int);

			append("ElectricNominal",				QMetaType::Double);
			append("ElectricMeasure",				QMetaType::Double);

			append("EngineeringNominal",			QMetaType::Double);
			append("EngineeringMeasure",			QMetaType::Double);

			append("ElectricLowLimit",				QMetaType::Double);
			append("ElectricHighLimit",				QMetaType::Double);
			append("ElectricUnit",					QMetaType::QString, 32);
			append("ElectricPrecision",				QMetaType::Int);

			append("EngineeringLowLimit",			QMetaType::Double);
			append("EngineeringHighLimit",			QMetaType::Double);
			append("EngineeringUnit",				QMetaType::QString, 32);
			append("EngineeringPrecision",			QMetaType::Int);

			append("ElectricErrorAbsolute",			QMetaType::Double);
			append("ElectricErrorReduce",			QMetaType::Double);
			append("ElectricErrorRelative",			QMetaType::Double);
			append("ElectricLimitErrorAbsolute",	QMetaType::Double);
			append("ElectricLimitErrorReduce",		QMetaType::Double);
			append("ElectricLimitErrorRelative",	QMetaType::Double);

			append("EngineeringErrorAbsolute",		QMetaType::Double);
			append("EngineeringErrorReduce",		QMetaType::Double);
			append("EngineeringErrorRelative",		QMetaType::Double);
			append("EngineeringLimitErrorAbsolute",	QMetaType::Double);
			append("EngineeringLimitErrorReduce",	QMetaType::Double);
			append("EngineeringLimitErrorRelative",	QMetaType::Double);

			append("MeasureTime",					QMetaType::QTime);
			append("Calibrator",					QMetaType::QString, 64);

			break;

		case SQL_TABLE_REPORT_HEADER:

			append("ObjectID",						QMetaType::Int);
			append("ReportID",						QMetaType::Int);

			break;

		case SQL_TABLE_RACK_GROUP:

			append("ObjectID",						QMetaType::Int);
			append("GroupIndex",					QMetaType::Int);

			append("Caption",						QMetaType::QString, 64);

			append("RackID0",						QMetaType::QString, 64);
			append("RackID1",						QMetaType::QString, 64);
			append("RackID2",						QMetaType::QString, 64);
			append("RackID3",						QMetaType::QString, 64);

			break;

		default:
			assert(0);
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

void SqlFieldBase::append(QString name, QMetaType::Type type, int length)
{
	if (name.isEmpty() == true)
	{
		return;
	}

	if (type == QMetaType::UnknownType)
	{
		return;
	}

	QSqlField field(name, QMetaType(type));

	if (type == QMetaType::Double)
	{
		field.setPrecision(9);
	}

	if (type == QMetaType::QString)
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

	switch(f.metaType().id())
	{
		case QMetaType::Bool:		result = QString("%1 BOOLEAN").arg(f.name());								break;	// bool
		case QMetaType::Int:		result = QString("%1 INTEGER").arg(f.name());								break;	// qint32
		case QMetaType::Long:		result = QString("%1 BIGINT").arg(f.name());								break;	// qint64
		case QMetaType::Double:		result = QString("%1 DOUBLE PRECISION").arg(f.name());						break;	// double
		case QMetaType::QString:	result = QString("%1 VARCHAR(%2)").arg(f.name()).arg(f.length());			break;	// string
		case QMetaType::QVariant:	result = QString("%1 BYTEA").arg(f.name());									break;	// blob
		case QMetaType::QTime:		result = QString("%1 TIMESTAMP").arg(f.name());								break;	// date and time

		default:
			assert(0);
			result.clear();
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
	m_caption = SqlTableName[objectType];
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

	QSqlQuery query(QString("SELECT max(%1) FROM %2").arg(m_fieldBase.field(SQL_FIELD_KEY).name(), m_info.caption()));
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

	int existTableCount = static_cast<int>(m_pDatabase->tables().count());
	for(int et = 0; et < existTableCount; et++)
	{
		if (m_pDatabase->tables().at(et).compare(SqlTableName[type], Qt::CaseInsensitive) == 0)
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
				case SQL_TABLE_LINEARITY_ADD_VAL_EL:
				case SQL_TABLE_LINEARITY_ADD_VAL_EN:
				case SQL_TABLE_LINEARITY_20_EL:
				case SQL_TABLE_LINEARITY_20_EN:
					request.append(QString(" REFERENCES %1(MeasureID) ON DELETE CASCADE").arg(SqlTableName[SQL_TABLE_LINEARITY]));
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
					if (info == nullptr)
					{
						break;
					}

					info->setObjectID(query.value(field++).toInt());
					info->setCaption(query.value(field++).toString());
					info->setVersion(query.value(field++).toInt());
				}
				break;

			case SQL_TABLE_LINEARITY:
				{
					Measure::LinearityItem* measure = static_cast<Measure::LinearityItem*> (pRecord) + readedCount;
					if (measure == nullptr)
					{
						break;
					}

					measure->setMeasureID(query.value(field++).toInt());

					measure->setFilter(query.value(field++).toBool());
					measure->setSignalValid(query.value(field++).toBool());

					measure->setConnectionSignalID(query.value(field++).toString());
					measure->setConnectionType(query.value(field++).toInt());

					measure->setAppSignalID(query.value(field++).toString());
					measure->setCustomAppSignalID(query.value(field++).toString());
					measure->setEquipmentID(query.value(field++).toString());
					measure->setCaption(query.value(field++).toString());

					measure->location().setModuleSerialNo(query.value(field++).toInt());
					measure->location().setModuleCaption(query.value(field++).toString());
					measure->location().rack().setIndex(query.value(field++).toInt());
					measure->location().rack().setCaption(query.value(field++).toString());
					measure->location().rack().setChannel(query.value(field++).toInt());
					measure->location().setChassis(query.value(field++).toInt());
					measure->location().setModule(query.value(field++).toInt());
					measure->location().setPlace(query.value(field++).toInt());

					measure->setCalibratorPrecision(query.value(field++).toInt());

					measure->setPercent(query.value(field++).toDouble());

					measure->setNominal(Measure::LimitType::Electric, query.value(field++).toDouble());
					measure->setMeasure(Measure::LimitType::Electric, query.value(field++).toDouble());

					measure->setNominal(Measure::LimitType::Engineering, query.value(field++).toDouble());
					measure->setMeasure(Measure::LimitType::Engineering, query.value(field++).toDouble());

					measure->setLowLimit(Measure::LimitType::Electric, query.value(field++).toDouble());
					measure->setHighLimit(Measure::LimitType::Electric, query.value(field++).toDouble());
					measure->setUnit(Measure::LimitType::Electric, query.value(field++).toString());
					measure->setLimitPrecision(Measure::LimitType::Electric, query.value(field++).toInt());

					measure->setLowLimit(Measure::LimitType::Engineering, query.value(field++).toDouble());
					measure->setHighLimit(Measure::LimitType::Engineering, query.value(field++).toDouble());
					measure->setUnit(Measure::LimitType::Engineering, query.value(field++).toString());
					measure->setLimitPrecision(Measure::LimitType::Engineering, query.value(field++).toInt());

					measure->setError(Measure::LimitType::Electric, Measure::MT::ErrorType::Absolute, query.value(field++).toDouble());
					measure->setError(Measure::LimitType::Electric, Measure::MT::ErrorType::Reduce, query.value(field++).toDouble());
					measure->setError(Measure::LimitType::Electric, Measure::MT::ErrorType::Relative, query.value(field++).toDouble());
					measure->setErrorLimit(Measure::LimitType::Electric, Measure::MT::ErrorType::Absolute, query.value(field++).toDouble());
					measure->setErrorLimit(Measure::LimitType::Electric, Measure::MT::ErrorType::Reduce, query.value(field++).toDouble());
					measure->setErrorLimit(Measure::LimitType::Electric, Measure::MT::ErrorType::Relative, query.value(field++).toDouble());

					measure->setError(Measure::LimitType::Engineering, Measure::MT::ErrorType::Absolute, query.value(field++).toDouble());
					measure->setError(Measure::LimitType::Engineering, Measure::MT::ErrorType::Reduce, query.value(field++).toDouble());
					measure->setError(Measure::LimitType::Engineering, Measure::MT::ErrorType::Relative, query.value(field++).toDouble());
					measure->setErrorLimit(Measure::LimitType::Engineering, Measure::MT::ErrorType::Absolute, query.value(field++).toDouble());
					measure->setErrorLimit(Measure::LimitType::Engineering, Measure::MT::ErrorType::Reduce, query.value(field++).toDouble());
					measure->setErrorLimit(Measure::LimitType::Engineering, Measure::MT::ErrorType::Relative, query.value(field++).toDouble());

					measure->setMeasureTime(query.value(field++).toDateTime());
					measure->setCalibrator(query.value(field++).toString());
				}
				break;

			case SQL_TABLE_LINEARITY_ADD_VAL_EL:
			case SQL_TABLE_LINEARITY_ADD_VAL_EN:
				{
					Measure::LimitType limitType = Measure::LimitType::NoLimitType;

					switch(m_info.objectType())
					{
						case SQL_TABLE_LINEARITY_ADD_VAL_EL:	limitType = Measure::LimitType::Electric;		break;
						case SQL_TABLE_LINEARITY_ADD_VAL_EN:	limitType = Measure::LimitType::Engineering;	break;

						default:
							assert(0);
							limitType = Measure::LimitType::NoLimitType;
					}

					if (limitType == Measure::LimitType::NoLimitType)
					{
						break;
					}

					Measure::LinearityItem* measure = static_cast<Measure::LinearityItem*> (pRecord) + readedCount;
					if (measure == nullptr)
					{
						break;
					}

					measure->setMeasureID(query.value(field++).toInt());

					measure->setAdditionalParamCount(query.value(field++).toInt());

					measure->setAdditionalParam(limitType, Measure::AdditionalParam::MaxDeviation, query.value(field++).toDouble());
					measure->setAdditionalParam(limitType, Measure::AdditionalParam::SystemDeviation, query.value(field++).toDouble());
					measure->setAdditionalParam(limitType, Measure::AdditionalParam::StandardDeviation, query.value(field++).toDouble());
					measure->setAdditionalParam(limitType, Measure::AdditionalParam::LowBorder, query.value(field++).toDouble());
					measure->setAdditionalParam(limitType, Measure::AdditionalParam::HighBorder, query.value(field++).toDouble());
					measure->setAdditionalParam(limitType, Measure::AdditionalParam::Uncertainty, query.value(field++).toDouble());
				}
				break;

			case SQL_TABLE_LINEARITY_20_EL:
			case SQL_TABLE_LINEARITY_20_EN:
				{
					Measure::LimitType limitType = Measure::LimitType::NoLimitType;

					switch(m_info.objectType())
					{
						case SQL_TABLE_LINEARITY_20_EL:	limitType = Measure::LimitType::Electric;		break;
						case SQL_TABLE_LINEARITY_20_EN:	limitType = Measure::LimitType::Engineering;	break;

						default:
							assert(0);
							limitType = Measure::LimitType::NoLimitType;
					}

					if (limitType == Measure::LimitType::NoLimitType)
					{
						break;
					}

					Measure::LinearityItem* measure = static_cast<Measure::LinearityItem*> (pRecord) + readedCount;
					if (measure == nullptr)
					{
						break;
					}

					measure->setMeasureID(query.value(field++).toInt());

					measure->setMeasureInPoint(query.value(field++).toInt());

					measure->setMeasureItemArray(limitType, 0, query.value(field++).toDouble());
					measure->setMeasureItemArray(limitType, 1, query.value(field++).toDouble());
					measure->setMeasureItemArray(limitType, 2, query.value(field++).toDouble());
					measure->setMeasureItemArray(limitType, 3, query.value(field++).toDouble());
					measure->setMeasureItemArray(limitType, 4, query.value(field++).toDouble());
					measure->setMeasureItemArray(limitType, 5, query.value(field++).toDouble());
					measure->setMeasureItemArray(limitType, 6, query.value(field++).toDouble());
					measure->setMeasureItemArray(limitType, 7, query.value(field++).toDouble());
					measure->setMeasureItemArray(limitType, 8, query.value(field++).toDouble());
					measure->setMeasureItemArray(limitType, 9, query.value(field++).toDouble());
					measure->setMeasureItemArray(limitType, 10, query.value(field++).toDouble());
					measure->setMeasureItemArray(limitType, 11, query.value(field++).toDouble());
					measure->setMeasureItemArray(limitType, 12, query.value(field++).toDouble());
					measure->setMeasureItemArray(limitType, 13, query.value(field++).toDouble());
					measure->setMeasureItemArray(limitType, 14, query.value(field++).toDouble());
					measure->setMeasureItemArray(limitType, 15, query.value(field++).toDouble());
					measure->setMeasureItemArray(limitType, 16, query.value(field++).toDouble());
					measure->setMeasureItemArray(limitType, 17, query.value(field++).toDouble());
					measure->setMeasureItemArray(limitType, 18, query.value(field++).toDouble());
					measure->setMeasureItemArray(limitType, 19, query.value(field++).toDouble());
				}
				break;

			case SQL_TABLE_LINEARITY_POINT:
				{
					Measure::Point* point = static_cast<Measure::Point*> (pRecord) + readedCount;
					if (point == nullptr)
					{
						break;
					}

					point->setIndex(query.value(field++).toInt());
					point->setPercent(query.value(field++).toDouble());
				}
				break;

			case SQL_TABLE_COMPARATOR:
				{
					Measure::ComparatorItem* measure = static_cast<Measure::ComparatorItem*> (pRecord) + readedCount;
					if (measure == nullptr)
					{
						break;
					}

					measure->setMeasureID(query.value(field++).toInt());

					measure->setFilter(query.value(field++).toBool());
					measure->setSignalValid(query.value(field++).toBool());

					measure->setConnectionSignalID(query.value(field++).toString());
					measure->setConnectionType(query.value(field++).toInt());

					measure->setAppSignalID(query.value(field++).toString());
					measure->setCustomAppSignalID(query.value(field++).toString());
					measure->setEquipmentID(query.value(field++).toString());
					measure->setCaption(query.value(field++).toString());

					measure->location().setModuleSerialNo(query.value(field++).toInt());
					measure->location().setModuleCaption(query.value(field++).toString());
					measure->location().rack().setIndex(query.value(field++).toInt());
					measure->location().rack().setCaption(query.value(field++).toString());
					measure->location().rack().setChannel(query.value(field++).toInt());
					measure->location().setChassis(query.value(field++).toInt());
					measure->location().setModule(query.value(field++).toInt());
					measure->location().setPlace(query.value(field++).toInt());

					measure->setCalibratorPrecision(query.value(field++).toInt());

					measure->setCompareAppSignalID(query.value(field++).toString());
					measure->setOutputAppSignalID(query.value(field++).toString());

					measure->setCmpValueType(query.value(field++).toInt());
					measure->setCmpType(query.value(field++).toInt());

					measure->setNominal(Measure::LimitType::Electric, query.value(field++).toDouble());
					measure->setMeasure(Measure::LimitType::Electric, query.value(field++).toDouble());

					measure->setNominal(Measure::LimitType::Engineering, query.value(field++).toDouble());
					measure->setMeasure(Measure::LimitType::Engineering, query.value(field++).toDouble());

					measure->setLowLimit(Measure::LimitType::Electric, query.value(field++).toDouble());
					measure->setHighLimit(Measure::LimitType::Electric, query.value(field++).toDouble());
					measure->setUnit(Measure::LimitType::Electric, query.value(field++).toString());
					measure->setLimitPrecision(Measure::LimitType::Electric, query.value(field++).toInt());

					measure->setLowLimit(Measure::LimitType::Engineering, query.value(field++).toDouble());
					measure->setHighLimit(Measure::LimitType::Engineering, query.value(field++).toDouble());
					measure->setUnit(Measure::LimitType::Engineering, query.value(field++).toString());
					measure->setLimitPrecision(Measure::LimitType::Engineering, query.value(field++).toInt());

					measure->setError(Measure::LimitType::Electric, Measure::MT::ErrorType::Absolute, query.value(field++).toDouble());
					measure->setError(Measure::LimitType::Electric, Measure::MT::ErrorType::Reduce, query.value(field++).toDouble());
					measure->setError(Measure::LimitType::Electric, Measure::MT::ErrorType::Relative, query.value(field++).toDouble());
					measure->setErrorLimit(Measure::LimitType::Electric, Measure::MT::ErrorType::Absolute, query.value(field++).toDouble());
					measure->setErrorLimit(Measure::LimitType::Electric, Measure::MT::ErrorType::Reduce, query.value(field++).toDouble());
					measure->setErrorLimit(Measure::LimitType::Electric, Measure::MT::ErrorType::Relative, query.value(field++).toDouble());

					measure->setError(Measure::LimitType::Engineering, Measure::MT::ErrorType::Absolute, query.value(field++).toDouble());
					measure->setError(Measure::LimitType::Engineering, Measure::MT::ErrorType::Reduce, query.value(field++).toDouble());
					measure->setError(Measure::LimitType::Engineering, Measure::MT::ErrorType::Relative, query.value(field++).toDouble());
					measure->setErrorLimit(Measure::LimitType::Engineering, Measure::MT::ErrorType::Absolute, query.value(field++).toDouble());
					measure->setErrorLimit(Measure::LimitType::Engineering, Measure::MT::ErrorType::Reduce, query.value(field++).toDouble());
					measure->setErrorLimit(Measure::LimitType::Engineering, Measure::MT::ErrorType::Relative, query.value(field++).toDouble());

					measure->setMeasureTime(query.value(field++).toDateTime());
					measure->setCalibrator(query.value(field++).toString());
				}
				break;

			case SQL_TABLE_REPORT_HEADER:
				{
				}
				break;

			case SQL_TABLE_RACK_GROUP:
				{
					RackGroup* group = static_cast<RackGroup*> (pRecord) + readedCount;
					if (group == nullptr)
					{
						break;
					}

					group->setIndex(query.value(field++).toInt());
					group->setCaption(query.value(field++).toString());

					group->setRackID(Metrology::Channel_0, query.value(field++).toString());
					group->setRackID(Metrology::Channel_1, query.value(field++).toString());
					group->setRackID(Metrology::Channel_2, query.value(field++).toString());
					group->setRackID(Metrology::Channel_3, query.value(field++).toString());
				}
				break;

			default:
				assert(0);
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
		// for append record - request, or for update record - request + QString("%1").arg(key[r]
		//
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
					if (info == nullptr)
					{
						break;
					}

					query.bindValue(field++, info->objectID());
					query.bindValue(field++, info->caption());
					query.bindValue(field++, info->version());
				}
				break;

			case SQL_TABLE_LINEARITY:
				{
					Measure::LinearityItem* measure = static_cast<Measure::LinearityItem*> (pRecord) + r;
					if (measure == nullptr)
					{
						break;
					}

					if (key == nullptr)
					{
						// for append record
						//
						measure->setMeasureID(lastKey() + 1);
					}

					query.bindValue(field++, measure->measureID());

					query.bindValue(field++, measure->filter());
					query.bindValue(field++, measure->isSignalValid());

					query.bindValue(field++, measure->connectionSignalID());
					query.bindValue(field++, measure->connectionType());

					query.bindValue(field++, measure->appSignalID());
					query.bindValue(field++, measure->customAppSignalID());
					query.bindValue(field++, measure->equipmentID());
					query.bindValue(field++, measure->caption());

					query.bindValue(field++, measure->location().moduleSerialNo());
					query.bindValue(field++, measure->location().moduleCaption());
					query.bindValue(field++, measure->location().rack().index());
					query.bindValue(field++, measure->location().rack().caption());
					query.bindValue(field++, measure->location().rack().channel());
					query.bindValue(field++, measure->location().chassis());
					query.bindValue(field++, measure->location().module());
					query.bindValue(field++, measure->location().place());

					query.bindValue(field++, measure->calibratorPrecision());

					query.bindValue(field++, measure->percent());

					query.bindValue(field++, measure->nominal(Measure::LimitType::Electric));
					query.bindValue(field++, measure->measure(Measure::LimitType::Electric));

					query.bindValue(field++, measure->nominal(Measure::LimitType::Engineering));
					query.bindValue(field++, measure->measure(Measure::LimitType::Engineering));

					query.bindValue(field++, measure->lowLimit(Measure::LimitType::Electric));
					query.bindValue(field++, measure->highLimit(Measure::LimitType::Electric));
					query.bindValue(field++, measure->unit(Measure::LimitType::Electric));
					query.bindValue(field++, measure->limitPrecision(Measure::LimitType::Electric));

					query.bindValue(field++, measure->lowLimit(Measure::LimitType::Engineering));
					query.bindValue(field++, measure->highLimit(Measure::LimitType::Engineering));
					query.bindValue(field++, measure->unit(Measure::LimitType::Engineering));
					query.bindValue(field++, measure->limitPrecision(Measure::LimitType::Engineering));

					query.bindValue(field++, measure->error(Measure::LimitType::Electric, Measure::MT::ErrorType::Absolute));
					query.bindValue(field++, measure->error(Measure::LimitType::Electric, Measure::MT::ErrorType::Reduce));
					query.bindValue(field++, measure->error(Measure::LimitType::Electric, Measure::MT::ErrorType::Relative));
					query.bindValue(field++, measure->errorLimit(Measure::LimitType::Electric, Measure::MT::ErrorType::Absolute));
					query.bindValue(field++, measure->errorLimit(Measure::LimitType::Electric, Measure::MT::ErrorType::Reduce));
					query.bindValue(field++, measure->errorLimit(Measure::LimitType::Electric, Measure::MT::ErrorType::Relative));

					query.bindValue(field++, measure->error(Measure::LimitType::Engineering, Measure::MT::ErrorType::Absolute));
					query.bindValue(field++, measure->error(Measure::LimitType::Engineering, Measure::MT::ErrorType::Reduce));
					query.bindValue(field++, measure->error(Measure::LimitType::Engineering, Measure::MT::ErrorType::Relative));
					query.bindValue(field++, measure->errorLimit(Measure::LimitType::Engineering, Measure::MT::ErrorType::Absolute));
					query.bindValue(field++, measure->errorLimit(Measure::LimitType::Engineering, Measure::MT::ErrorType::Reduce));
					query.bindValue(field++, measure->errorLimit(Measure::LimitType::Engineering, Measure::MT::ErrorType::Relative));

					query.bindValue(field++, measure->measureTime());
					query.bindValue(field++, measure->calibrator());
				}
				break;

			case SQL_TABLE_LINEARITY_ADD_VAL_EL:
			case SQL_TABLE_LINEARITY_ADD_VAL_EN:
				{
					Measure::LimitType limitType = Measure::LimitType::NoLimitType;

					switch(m_info.objectType())
					{
						case SQL_TABLE_LINEARITY_ADD_VAL_EL:	limitType = Measure::LimitType::Electric;		break;
						case SQL_TABLE_LINEARITY_ADD_VAL_EN:	limitType = Measure::LimitType::Engineering;	break;

						default:
							assert(0);
							limitType = Measure::LimitType::NoLimitType;
					}

					if (limitType == Measure::LimitType::NoLimitType)
					{
						break;
					}

					Measure::LinearityItem* measure = static_cast<Measure::LinearityItem*> (pRecord) + r;
					if (measure == nullptr)
					{
						break;
					}

					query.bindValue(field++, measure->measureID());

					query.bindValue(field++, measure->additionalParamCount());

					query.bindValue(field++, measure->additionalParam(limitType, Measure::AdditionalParam::MaxDeviation));
					query.bindValue(field++, measure->additionalParam(limitType, Measure::AdditionalParam::SystemDeviation));
					query.bindValue(field++, measure->additionalParam(limitType, Measure::AdditionalParam::StandardDeviation));
					query.bindValue(field++, measure->additionalParam(limitType, Measure::AdditionalParam::LowBorder));
					query.bindValue(field++, measure->additionalParam(limitType, Measure::AdditionalParam::HighBorder));
					query.bindValue(field++, measure->additionalParam(limitType, Measure::AdditionalParam::Uncertainty));
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

			case SQL_TABLE_LINEARITY_20_EL:
			case SQL_TABLE_LINEARITY_20_EN:
				{
					Measure::LimitType limitType = Measure::LimitType::NoLimitType;

					switch(m_info.objectType())
					{
						case SQL_TABLE_LINEARITY_20_EL:	limitType = Measure::LimitType::Electric;		break;
						case SQL_TABLE_LINEARITY_20_EN:	limitType = Measure::LimitType::Engineering;	break;

						default:
							assert(0);
							limitType = Measure::LimitType::NoLimitType;
					}

					if (limitType == Measure::LimitType::NoLimitType)
					{
						break;
					}

					Measure::LinearityItem* measure = static_cast<Measure::LinearityItem*> (pRecord) + r;
					if (measure == nullptr)
					{
						break;
					}

					query.bindValue(field++, measure->measureID());

					query.bindValue(field++, measure->measureInPoint());

					query.bindValue(field++, measure->measureItemArray(limitType, 0));
					query.bindValue(field++, measure->measureItemArray(limitType, 1));
					query.bindValue(field++, measure->measureItemArray(limitType, 2));
					query.bindValue(field++, measure->measureItemArray(limitType, 3));
					query.bindValue(field++, measure->measureItemArray(limitType, 4));
					query.bindValue(field++, measure->measureItemArray(limitType, 5));
					query.bindValue(field++, measure->measureItemArray(limitType, 6));
					query.bindValue(field++, measure->measureItemArray(limitType, 7));
					query.bindValue(field++, measure->measureItemArray(limitType, 8));
					query.bindValue(field++, measure->measureItemArray(limitType, 9));
					query.bindValue(field++, measure->measureItemArray(limitType, 10));
					query.bindValue(field++, measure->measureItemArray(limitType, 11));
					query.bindValue(field++, measure->measureItemArray(limitType, 12));
					query.bindValue(field++, measure->measureItemArray(limitType, 13));
					query.bindValue(field++, measure->measureItemArray(limitType, 14));
					query.bindValue(field++, measure->measureItemArray(limitType, 15));
					query.bindValue(field++, measure->measureItemArray(limitType, 16));
					query.bindValue(field++, measure->measureItemArray(limitType, 17));
					query.bindValue(field++, measure->measureItemArray(limitType, 18));
					query.bindValue(field++, measure->measureItemArray(limitType, 19));
				}
				break;

			case SQL_TABLE_LINEARITY_POINT:
				{
					Measure::Point* point = static_cast<Measure::Point*> (pRecord) + r;
					if (point == nullptr)
					{
						break;
					}

					query.bindValue(field++, point->Index());
					query.bindValue(field++, point->percent());
				}
				break;

			case SQL_TABLE_COMPARATOR:
				{
					Measure::ComparatorItem* measure = static_cast<Measure::ComparatorItem*> (pRecord) + r;
					if (measure == nullptr)
					{
						break;
					}

					if (key == nullptr)
					{
						// for append record
						//
						measure->setMeasureID(lastKey() + 1);
					}

					query.bindValue(field++, measure->measureID());

					query.bindValue(field++, measure->filter());
					query.bindValue(field++, measure->isSignalValid());

					query.bindValue(field++, measure->connectionSignalID());
					query.bindValue(field++, measure->connectionType());

					query.bindValue(field++, measure->appSignalID());
					query.bindValue(field++, measure->customAppSignalID());
					query.bindValue(field++, measure->equipmentID());
					query.bindValue(field++, measure->caption());

					query.bindValue(field++, measure->location().moduleSerialNo());
					query.bindValue(field++, measure->location().moduleCaption());
					query.bindValue(field++, measure->location().rack().index());
					query.bindValue(field++, measure->location().rack().caption());
					query.bindValue(field++, measure->location().rack().channel());
					query.bindValue(field++, measure->location().chassis());
					query.bindValue(field++, measure->location().module());
					query.bindValue(field++, measure->location().place());

					query.bindValue(field++, measure->calibratorPrecision());

					query.bindValue(field++, measure->compareAppSignalID());
					query.bindValue(field++, measure->outputAppSignalID());

					query.bindValue(field++, measure->cmpValueType());
					query.bindValue(field++, measure->cmpTypeInt());

					query.bindValue(field++, measure->nominal(Measure::LimitType::Electric));
					query.bindValue(field++, measure->measure(Measure::LimitType::Electric));

					query.bindValue(field++, measure->nominal(Measure::LimitType::Engineering));
					query.bindValue(field++, measure->measure(Measure::LimitType::Engineering));

					query.bindValue(field++, measure->lowLimit(Measure::LimitType::Electric));
					query.bindValue(field++, measure->highLimit(Measure::LimitType::Electric));
					query.bindValue(field++, measure->unit(Measure::LimitType::Electric));
					query.bindValue(field++, measure->limitPrecision(Measure::LimitType::Electric));

					query.bindValue(field++, measure->lowLimit(Measure::LimitType::Engineering));
					query.bindValue(field++, measure->highLimit(Measure::LimitType::Engineering));
					query.bindValue(field++, measure->unit(Measure::LimitType::Engineering));
					query.bindValue(field++, measure->limitPrecision(Measure::LimitType::Engineering));

					query.bindValue(field++, measure->error(Measure::LimitType::Electric, Measure::MT::ErrorType::Absolute));
					query.bindValue(field++, measure->error(Measure::LimitType::Electric, Measure::MT::ErrorType::Reduce));
					query.bindValue(field++, measure->error(Measure::LimitType::Electric, Measure::MT::ErrorType::Relative));
					query.bindValue(field++, measure->errorLimit(Measure::LimitType::Electric, Measure::MT::ErrorType::Absolute));
					query.bindValue(field++, measure->errorLimit(Measure::LimitType::Electric, Measure::MT::ErrorType::Reduce));
					query.bindValue(field++, measure->errorLimit(Measure::LimitType::Electric, Measure::MT::ErrorType::Relative));

					query.bindValue(field++, measure->error(Measure::LimitType::Engineering, Measure::MT::ErrorType::Absolute));
					query.bindValue(field++, measure->error(Measure::LimitType::Engineering, Measure::MT::ErrorType::Reduce));
					query.bindValue(field++, measure->error(Measure::LimitType::Engineering, Measure::MT::ErrorType::Relative));
					query.bindValue(field++, measure->errorLimit(Measure::LimitType::Engineering, Measure::MT::ErrorType::Absolute));
					query.bindValue(field++, measure->errorLimit(Measure::LimitType::Engineering, Measure::MT::ErrorType::Reduce));
					query.bindValue(field++, measure->errorLimit(Measure::LimitType::Engineering, Measure::MT::ErrorType::Relative));

					query.bindValue(field++, measure->measureTime());
					query.bindValue(field++, measure->calibrator());
				}
				break;

			case SQL_TABLE_REPORT_HEADER:
				{
				}
				break;

			case SQL_TABLE_RACK_GROUP:
				{
					RackGroup* group = static_cast<RackGroup*> (pRecord) + r;
					if (group == nullptr)
					{
						break;
					}

					query.bindValue(field++, group->index());
					query.bindValue(field++, group->caption());

					query.bindValue(field++, group->rackID(Metrology::Channel_0));
					query.bindValue(field++, group->rackID(Metrology::Channel_1));
					query.bindValue(field++, group->rackID(Metrology::Channel_2));
					query.bindValue(field++, group->rackID(Metrology::Channel_3));
				}
				break;

			default:
				assert(0);
		}

		if (query.exec() == false)
		{
			qDebug() << __FUNCTION__ << query.lastError().text();

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

	QString keyFieldName = m_fieldBase.field(SQL_FIELD_KEY).name();
	if (keyFieldName.isEmpty() == true)
	{
		return 0;
	}

	int count = recordCount();
	if (count == 0)
	{
		return 0;
	}

	int transactionCount = keyCount / REMOVE_TRANSACTION_RECORD_COUNT;

	if (keyCount % REMOVE_TRANSACTION_RECORD_COUNT != 0)
	{
		transactionCount++;
	}

	int record = 0;

	for (int t = 0; t < transactionCount; t++)
	{
		QString request = QString("DELETE FROM %1 WHERE ").arg(m_info.caption());

		for (int k = 0; k < REMOVE_TRANSACTION_RECORD_COUNT; k++)
		{
			request.append(QString("%1=%2").arg(keyFieldName).arg(key[record++]));

			if (record >= keyCount )
			{
				break;
			}

			if (k != REMOVE_TRANSACTION_RECORD_COUNT - 1)
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

Database::Database(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

Database::~Database()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool Database::open()
{
	bool result = false;

	switch(m_databaseOption.type())
	{
		case OT::DatabaseType::SQLite:		result = openSQLite(DATABASE_NAME);		break;
		case OT::DatabaseType::PostgreSQL:	result = openPostgres(DATABASE_NAME);	break;

		default:
			assert(0);
			QMessageBox::critical(nullptr, tr("Database"), tr("Unknown type of database!"));
			return false;
	}

	if (result == false)
	{
		return false;
	}

	for(int type = 0; type < SQL_TABLE_COUNT; type++)
	{
		m_table[type].init(type, &m_database);
	}

	//
	//
	m_currentVersion = initVersion();
	if (m_currentVersion > DATABASE_VERSION)
	{
		QMessageBox::critical(nullptr, tr("Database"), tr("Loaded database has version %1, latest known version is %2, please update your software!")
							  .arg(m_currentVersion).arg(DATABASE_VERSION));

		close();

		return false;
	}

	createTables();

	//
	//
	result = applyMigrations();
	if (result == false)
	{
		QMessageBox::critical(nullptr, tr("Database"), tr("Error of migrations - Loaded database has version %1, latest known version is %2.")
							  .arg(m_currentVersion).arg(DATABASE_VERSION));

		close();

		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Database::openSQLite(const QString& databaseName)
{
	if (databaseName.isEmpty() == true)
	{
		return false;
	}

	//
	//
	m_database = QSqlDatabase::addDatabase("QSQLITE");
	if (m_database.lastError().isValid() == true)
	{
		return false;
	}

	//
	//
	QString path = m_databaseOption.locationPath();
	if (path.isEmpty() == true)
	{
		QMessageBox::critical(nullptr, tr("Database"), tr("Invalid path to Database!"));
		return false;
	}

	//
	//
	m_database.setDatabaseName(path + QDir::separator() + databaseName + DATABASE_SQLLITE_EXT);
	if (m_database.open() == false)
	{
		qDebug() << m_database.lastError().text();
		QMessageBox::critical(nullptr, tr("Database"), tr("Cannot open database"));
		return false;
	}

	//
	//
	QSqlQuery query;

	if (query.exec("PRAGMA foreign_keys=on") == false)
	{
		QMessageBox::critical(nullptr, tr("Database"), tr("Error set option of database: [foreign keys=on]"));
	}

	if (query.exec("PRAGMA synchronous=normal") == false)
	{
		QMessageBox::critical(nullptr, tr("Database"), tr("Error set option of database: [synchronous=normal]"));
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Database::openPostgres(const QString& databaseName)
{
	if (databaseName.isEmpty() == true)
	{
		return false;
	}

	QString connectionName = "default";
	{
		//
		//
		QSqlDatabase psql_db = QSqlDatabase::addDatabase("QPSQL", connectionName);
		if (psql_db.lastError().isValid() == true)
		{
			return false;
		}

		// set options postgres database
		//
		psql_db.setDatabaseName("postgres");
		psql_db.setHostName(m_databaseOption.ip());
		psql_db.setPort(m_databaseOption.port());
		psql_db.setUserName(m_databaseOption.user());
		psql_db.setPassword(m_databaseOption.password());

		if (psql_db.open() == false)
		{
			qDebug() << psql_db.lastError().text();
			QMessageBox::critical(nullptr, tr("Database"), tr("Cannot open database"));
			return false;
		}

		//
		//
		QSqlQuery query(psql_db);

		bool result = query.exec("SELECT datname FROM pg_database");
		if (result == false)
		{
			qDebug() << query.lastError().text();
			psql_db.close();
			return false;
		}

		// find our database
		//
		bool isExist = false;

		while (query.next())
		{
			QString existDatabaseName = query.value(0).toString();
			if (QString::compare(existDatabaseName, databaseName, Qt::CaseInsensitive) == 0)
			{
				isExist = true;
				break;
			}
		}

		// if our database was not found
		// create our database
		//
		if (isExist == false)
		{
			result = query.exec("CREATE DATABASE " + QString(databaseName));
			if (result == false)
			{
				qDebug() << query.lastError().text();
				psql_db.close();
				return false;
			}
		}

		psql_db.close();
	}
	QSqlDatabase::removeDatabase(connectionName);

	//
	//
	m_database = QSqlDatabase::addDatabase("QPSQL");
	if (m_database.lastError().isValid() == true)
	{
		return false;
	}

	// set options our database
	//
	m_database.setDatabaseName(QString(databaseName).toLower());
	m_database.setHostName(m_databaseOption.ip());
	m_database.setPort(m_databaseOption.port());
	m_database.setUserName(m_databaseOption.user());
	m_database.setPassword(m_databaseOption.password());

	if (m_database.open() == false)
	{
		qDebug() << m_database.lastError().text();
		QMessageBox::critical(nullptr, tr("Database"), tr("Cannot open database"));
		return false;
	}

	return true;
}
// -------------------------------------------------------------------------------------------------------------------

void Database::close()
{
	if (m_databaseOption.onExit() == true)
	{
		createBackup();
	}

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

int Database::initVersion()
{
	SqlTable table;
	if (table.init(SQL_TABLE_DATABASE_INFO, &m_database) == false)
	{
		return 0;
	}

	int databaseVersion = 0;

	std::vector<SqlObjectInfo> info;

	if (table.isExist() == false)
	{
		if (table.create() == true)
		{
			info.resize(SQL_TABLE_COUNT);

			for(int t = 0; t < SQL_TABLE_COUNT; t++)
			{
				info[static_cast<quint64>(t)] = m_table[t].info();
			}

			table.write(info.data(), static_cast<int>(info.size()));

			databaseVersion = DATABASE_VERSION;
		}
	}
	else
	{
		// open table that has data of database
		//
		if (table.open() == true)
		{
			// determine count of obejects in database
			//
			info.resize(static_cast<quint64>(table.recordCount()));

			int objectCount = table.read(info.data());
			for (int i = 0; i < objectCount; i++)
			{
				if (info[i].objectID() == SQL_TABLE_DATABASE_INFO)
				{
					databaseVersion = info[i].version();
					break;
				}
			}
		}
	}

	table.close();

	return databaseVersion;
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

bool Database::createBackup()
{
	if (m_databaseOption.type() != OT::SQLite)
	{
		return false;
	}

	QString sourcePath = m_databaseOption.locationPath() + QDir::separator() + DATABASE_NAME + DATABASE_SQLLITE_EXT;
	if (QFile::exists(sourcePath) == false)
	{
		return false;
	}

	QString path = m_databaseOption.backupPath();

	if (QFile::exists(path) == false)
	{
		path = QDir::tempPath();

		QSettings s;
		s.setValue(QString("%1Path").arg(DATABASE_OPTIONS_REG_KEY), path);
	}

	QDateTime&& currentTime = QDateTime::currentDateTime();
	QDate&& date = currentTime.date();
	QTime&& time = currentTime.time();

	QString destPath = QString("%1%2%3%4%5%6%7%8%9%10")
				.arg(path)
				.arg(QDir::separator())
				.arg(date.year(), 4, 10, QChar('0'))
				.arg(date.month(), 2, 10, QChar('0'))
				.arg(date.day(), 2, 10, QChar('0'))
				.arg(time.hour(), 2, 10, QChar('0'))
				.arg(time.minute(), 2, 10, QChar('0'))
				.arg(time.second(), 2, 10, QChar('0'))
				.arg(DATABASE_NAME)
				.arg(DATABASE_SQLLITE_EXT);

	if (QFile::copy(sourcePath, destPath) == false)
	{
		QMessageBox::critical(nullptr, tr("Backup"), tr("Error reserved copy database (backup of measurements)"));
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool Database::appendMeasure(Measure::Item* pMeasurement)
{
	if (pMeasurement == nullptr)
	{
		return false;
	}

	Measure::Type measureType = pMeasurement->measureType();
	if (ERR_MEASURE_TYPE(measureType) == true)
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

		if (table.open() == false)
		{
			continue;
		}

		if (table.write(pMeasurement) == 1)
		{
			result = true;
		}

		table.close();
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void Database::appendToBase(Measure::Item* pMeasurement)
{
	if (pMeasurement == nullptr)
	{
		return;
	}

	bool result = appendMeasure(pMeasurement);
	if (result == false)
	{
		QMessageBox::critical(nullptr, tr("Save measurements"), tr("Error saving measurements to database"));
		return;
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool Database::removeMeasure(Measure::Type measuteType, const std::vector<int>& keyList)
{
	bool result = false;

	for (int type = 0; type < SQL_TABLE_COUNT; type++)
	{
		if (SqlTableByMeasureType[type] != measuteType)
		{
			continue;
		}

		SqlTable& table = m_table[type];

		if (table.open() == false)
		{
			continue;
		}

		if (table.remove(keyList.data(), TO_INT(keyList.size())) == TO_INT(keyList.size()))
		{
			result = true;
		}

		table.close();

		break;
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

bool Database::updateMeasure(Measure::Type measuteType, const std::vector<Measure::Item*>& list)
{
	bool result = false;

	std::vector<int> keyList;

	for (int type = 0; type < SQL_TABLE_COUNT; type++)
	{
		if (SqlTableByMeasureType[type] != measuteType)
		{
			continue;
		}

		SqlTable& table = m_table[type];

		if (table.open() == false)
		{
			continue;
		}

		switch (measuteType)
		{
			case Measure::Type::Linearity:
				{
					std::vector<Measure::LinearityItem> measurementList;

					for (Measure::Item* pMeasurement : list)
					{
						if (pMeasurement == nullptr)
						{
							continue;
						}

						measurementList.push_back(*dynamic_cast<Measure::LinearityItem*>(pMeasurement));
						keyList.push_back(pMeasurement->measureID());
					}

					if (table.write((void*) measurementList.data(), TO_INT(keyList.size()), keyList.data()) == TO_INT(keyList.size()))
					{
						result = true;
					}
				}

				break;

			case Measure::Type::Comparators:
				{
					std::vector<Measure::ComparatorItem> measurementList;

					for (Measure::Item* pMeasurement : list)
					{
						if (pMeasurement == nullptr)
						{
							continue;
						}

						measurementList.push_back(*dynamic_cast<Measure::ComparatorItem*>(pMeasurement));
						keyList.push_back(pMeasurement->measureID());
					}

					if (table.write((void*) measurementList.data(), TO_INT(keyList.size()), keyList.data()) == TO_INT(keyList.size()))
					{
						result = true;
					}
				}

				break;

			default:
				assert(0);
		}

		table.close();

		break;
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void Database::removeFromBase(Measure::Type measureType, const std::vector<int>& keyList)
{
	bool result = removeMeasure(static_cast<Measure::Type>(measureType), keyList);
	if (result == false)
	{
		QMessageBox::critical(nullptr, tr("Delete measurements"), tr("Error remove measurements from database"));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void Database::updateInBase(Measure::Type measureType, const std::vector<Measure::Item*>& list)
{
	bool result = updateMeasure(static_cast<Measure::Type>(measureType), list);
	if (result == false)
	{
		QMessageBox::critical(nullptr, tr("Update measurements"), tr("Error update measurements from database"));
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool Database::applyMigrations()
{
	if (m_currentVersion == DATABASE_VERSION)
	{
		return true;
	}

	// run all migrations
	//
	bool allMigrationOk = true;

	for(int i = m_currentVersion; i < DATABASE_VERSION; i++)
	{
		QSqlQuery query;
		bool result = query.exec(migration[i]);
		if (result == false)
		{
			qDebug() << query.lastError().text();
			allMigrationOk = false;
		}
	}

	if (allMigrationOk == false)
	{
		return false;
	}

	// update DatabaseInfo
	//
	SqlTable* pTable = openTable(SQL_TABLE_DATABASE_INFO);
	if (pTable != nullptr)
	{
		pTable->clear();

		std::vector<SqlObjectInfo> info;

		info.resize(SQL_TABLE_COUNT);
		for(int t = 0; t < SQL_TABLE_COUNT; t++)
		{
			info[static_cast<quint64>(t)] = m_table[t].info();
		}

		pTable->write(info.data(), static_cast<int>(info.size()));

		pTable->close();
	}

	return true;
}


// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

