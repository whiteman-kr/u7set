 #include "Options.h"

#include <QSettings>
#include <QTemporaryDir>
#include <QMessageBox>
#include "Database.h"

// -------------------------------------------------------------------------------------------------------------------

Options theOptions;

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

TcpIpOption::TcpIpOption(QObject *parent) :
    QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

TcpIpOption::TcpIpOption(const TcpIpOption& from, QObject *parent) :
    QObject(parent)
{
    *this = from;
}

// -------------------------------------------------------------------------------------------------------------------


TcpIpOption::~TcpIpOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void TcpIpOption::load()
{
    QSettings s;

    m_serverIP = s.value( QString("%1ServerIP").arg(TCPIP_OPTIONS_KEY), "127.0.0.1").toString();
    m_serverPort = s.value( QString("%1ServerPort").arg(TCPIP_OPTIONS_KEY), 2000).toInt();
}

// -------------------------------------------------------------------------------------------------------------------

void TcpIpOption::save()
{
    QSettings s;

    s.setValue( QString("%1ServerIP").arg(TCPIP_OPTIONS_KEY), m_serverIP);
    s.setValue( QString("%1ServerPort").arg(TCPIP_OPTIONS_KEY), m_serverPort);
}

// -------------------------------------------------------------------------------------------------------------------

TcpIpOption& TcpIpOption::operator=(const TcpIpOption& from)
{
    m_serverIP = from.m_serverIP;
    m_serverPort = from.m_serverPort;

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ToolBarOption::ToolBarOption(QObject *parent) :
    QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

ToolBarOption::ToolBarOption(const ToolBarOption& from, QObject *parent) :
    QObject(parent)
{
    *this = from;
}

// -------------------------------------------------------------------------------------------------------------------


ToolBarOption::~ToolBarOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void ToolBarOption::load()
{
    QSettings s;

    m_measureTimeout = s.value( QString("%1MeasureTimeout").arg(TOOLBAR_OPTIONS_KEY), 0).toInt();
    m_measureKind = s.value( QString("%1MeasureKind").arg(TOOLBAR_OPTIONS_KEY), MEASURE_KIND_ONE).toInt();
    m_outputSignalType = s.value( QString("%1OutputSignalType").arg(TOOLBAR_OPTIONS_KEY), OUTPUT_SIGNAL_TYPE_DONT_USED).toInt();
}

// -------------------------------------------------------------------------------------------------------------------

void ToolBarOption::save()
{
    QSettings s;

    s.setValue( QString("%1MeasureTimeout").arg(TOOLBAR_OPTIONS_KEY), m_measureTimeout);
    s.setValue( QString("%1MeasureKind").arg(TOOLBAR_OPTIONS_KEY), m_measureKind);
    s.setValue( QString("%1OutputSignalType").arg(TOOLBAR_OPTIONS_KEY), m_outputSignalType);
}

// -------------------------------------------------------------------------------------------------------------------

ToolBarOption& ToolBarOption::operator=(const ToolBarOption& from)
{
    m_measureTimeout = from.m_measureTimeout;
    m_measureKind = from.m_measureKind;
    m_outputSignalType = from.m_outputSignalType;

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureViewOption::MeasureViewOption(QObject *parent) :
    QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

MeasureViewOption::MeasureViewOption(const MeasureViewOption& from, QObject *parent) :
    QObject(parent)
{
    *this = from;
}

// -------------------------------------------------------------------------------------------------------------------


MeasureViewOption::~MeasureViewOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureViewOption::init()
{
    MeasureViewHeader header;

    for(int type = 0; type < MEASURE_TYPE_COUNT; type ++)
    {
        header.setMeasureType(type);

        for(int column = 0; column < MEASURE_VIEW_COLUMN_COUNT; column++)
        {
            MeasureViewColumn* pColumn = header.column(column);
            if (pColumn != nullptr)
            {
                m_column[type][column] = *pColumn;
            }
        }
    }

    header.setMeasureType(MEASURE_TYPE_UNKNOWN);
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureViewOption::load()
{
    QSettings s;

    for(int type = 0; type < MEASURE_TYPE_COUNT; type ++)
    {
        for(int column = 0; column < MEASURE_VIEW_COLUMN_COUNT; column++)
        {
            MeasureViewColumn c = m_column[type][column];

            if (c.title().isEmpty() == false)
            {
                m_column[type][column].setWidth( s.value(QString("%1/Header/%2/%3/Width").arg(MEASURE_VIEW_OPTIONS_KEY).arg(MeasureType[type]).arg(c.title()), c.width()).toInt() );
                m_column[type][column].setVisible( s.value(QString("%1/Header/%2/%3/Visible").arg(MEASURE_VIEW_OPTIONS_KEY).arg(MeasureType[type]).arg(c.title()), c.enableVisible()).toBool() );
                m_column[type][column].setColor( s.value(QString("%1/Header/%2/%3/Color").arg(MEASURE_VIEW_OPTIONS_KEY).arg(MeasureType[type]).arg(c.title()), c.color().rgb()).toInt() );
            }
        }
    }

    m_font.fromString( s.value( QString("%1Font").arg(MEASURE_VIEW_OPTIONS_KEY), "Segoe UI, 10").toString() );
    m_fontBold = m_font;
    m_fontBold.setBold(true);

    m_showExternalID = s.value( QString("%1ShowExternalID").arg(MEASURE_VIEW_OPTIONS_KEY), true).toBool();
    m_showDisplayingValueType = s.value( QString("%1ShowDisplayingValueType").arg(MEASURE_VIEW_OPTIONS_KEY), DISPLAYING_VALUE_TYPE_PHYSICAL).toInt();

    m_colorLimitError = s.value( QString("%1ColorLimitError").arg(MEASURE_VIEW_OPTIONS_KEY), COLOR_LIMIT_ERROR.rgb()).toInt();
    m_colorControlError = s.value( QString("%1ColorControlError").arg(MEASURE_VIEW_OPTIONS_KEY), COLOR_CONTROL_ERROR.rgb()).toInt();
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureViewOption::save()
{
    QSettings s;

    for(int type = 0; type < MEASURE_TYPE_COUNT; type ++)
    {
        for(int column = 0; column < MEASURE_VIEW_COLUMN_COUNT; column++)
        {
            MeasureViewColumn c = m_column[type][column];

            if (c.title().isEmpty() == false)
            {
                s.setValue(QString("%1/Header/%2/%3/Width").arg(MEASURE_VIEW_OPTIONS_KEY).arg(MeasureType[type]).arg(c.title()), c.width());
                s.setValue(QString("%1/Header/%2/%3/Visible").arg(MEASURE_VIEW_OPTIONS_KEY).arg(MeasureType[type]).arg(c.title()), c.enableVisible());
                s.setValue(QString("%1/Header/%2/%3/Color").arg(MEASURE_VIEW_OPTIONS_KEY).arg(MeasureType[type]).arg(c.title()), c.color().rgb());
            }
        }
    }

    s.setValue( QString("%1Font").arg(MEASURE_VIEW_OPTIONS_KEY), m_font.toString());

    s.setValue( QString("%1ShowExternalID").arg(MEASURE_VIEW_OPTIONS_KEY), m_showExternalID);
    s.setValue( QString("%1ShowDisplayingValueType").arg(MEASURE_VIEW_OPTIONS_KEY), m_showDisplayingValueType);

    s.setValue( QString("%1ColorLimitError").arg(MEASURE_VIEW_OPTIONS_KEY), m_colorLimitError.rgb() );
    s.setValue( QString("%1ColorControlError").arg(MEASURE_VIEW_OPTIONS_KEY), m_colorControlError.rgb() );
}

// -------------------------------------------------------------------------------------------------------------------

MeasureViewOption& MeasureViewOption::operator=(const MeasureViewOption& from)
{
    for(int type = 0; type < MEASURE_TYPE_COUNT; type ++)
    {
        for(int column = 0; column < MEASURE_VIEW_COLUMN_COUNT; column++)
        {
            m_column[type][column] = from.m_column[type][column];
        }
    }

    m_font.fromString( from.m_font.toString() );
    m_fontBold = m_font;
    m_fontBold.setBold(true);

    m_showExternalID = from.m_showExternalID;
    m_showDisplayingValueType = from.m_showDisplayingValueType;

    m_colorControlError = from.m_colorControlError;
    m_colorLimitError = from.m_colorLimitError;

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

DatabaseOption::DatabaseOption(QObject *parent) :
    QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

DatabaseOption::DatabaseOption(const DatabaseOption& from, QObject *parent) :
    QObject(parent)
{
    *this = from;
}

// -------------------------------------------------------------------------------------------------------------------

DatabaseOption::~DatabaseOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void DatabaseOption::load()
{
    QSettings s;

    m_path = s.value( QString("%1Path").arg(DATABASE_OPTIONS_REG_KEY), QDir::currentPath()).toString();
    m_type = s.value( QString("%1Type").arg(DATABASE_OPTIONS_REG_KEY), DATABASE_TYPE_SQLITE).toInt();
}

// -------------------------------------------------------------------------------------------------------------------

void DatabaseOption::save()
{
    QSettings s;

    s.setValue( QString("%1Path").arg(DATABASE_OPTIONS_REG_KEY), m_path );
    s.setValue( QString("%1Type").arg(DATABASE_OPTIONS_REG_KEY), m_type );
}

// -------------------------------------------------------------------------------------------------------------------

DatabaseOption& DatabaseOption::operator=(const DatabaseOption& from)
{
    m_path = from.m_path;
    m_type = from.m_type;

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

void REPORT_HEADER::init(int type)
{
    if ( type < 0 || type >=  REPORT_TYPE_COUNT)
    {
        return;
    }

    m_type = type;

    m_documentTitle = QString("%1 %2").arg(QT_TRANSLATE_NOOP("Options.cpp", "Protocol №")).arg(m_type + 1);
    m_reportTitle = QString("%1 %2").arg(QT_TRANSLATE_NOOP("Options.cpp", "Report of ")).arg(ReportType[m_type]);;
    m_date = QDate::currentDate().toString("dd.MM.yyyy");
    m_tableTitle = QString("%1 %2").arg(QT_TRANSLATE_NOOP("Options.cpp", "Table: ")).arg(ReportType[m_type]);
    m_conclusion = QT_TRANSLATE_NOOP("Options.cpp", "This is report has not problems");

    m_T = 20;
    m_P = 100;
    m_H = 70;
    m_V = 220;
    m_F = 50;

    int objectID = SQL_TABLE_UNKNONW;

    switch(m_type)
    {
        case REPORT_TYPE_LINEARITY:                 objectID = SqlObjectID[SQL_TABLE_LINEARETY];            break;
        case REPORT_TYPE_LINEARITY_CERTIFICATION:   objectID = SqlObjectID[SQL_TABLE_LINEARETY_ADD_VAL];    break;
        case REPORT_TYPE_LINEARITY_DETAIL_ELRCTRIC: objectID = SqlObjectID[SQL_TABLE_LINEARETY_20_EL];      break;
        case REPORT_TYPE_LINEARITY_DETAIL_PHYSICAL: objectID = SqlObjectID[SQL_TABLE_LINEARETY_20_PH];      break;
        case REPORT_TYPE_COMPARATOR:                objectID = SqlObjectID[SQL_TABLE_COMPARATOR];           break;
        case REPORT_TYPE_COMPLEX_COMPARATOR:        objectID = SqlObjectID[SQL_TABLE_COMPLEX_COMPARATOR];   break;
        default:                                    assert(0);                                              break;
    }

    m_linkObjectID = objectID;

    m_reportFile = ReportFileName[m_type];
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ReportHeaderBase::ReportHeaderBase(QObject *parent) :
    QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

ReportHeaderBase::ReportHeaderBase(const ReportHeaderBase& from, QObject *parent) :
    QObject(parent)
{
    *this = from;
}

// -------------------------------------------------------------------------------------------------------------------

ReportHeaderBase::~ReportHeaderBase()
{
    clear();
}

// -------------------------------------------------------------------------------------------------------------------

int ReportHeaderBase::count()
{
    int count = 0;

    m_mutex.lock();

        count = m_headerList.count();

    m_mutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

void ReportHeaderBase::clear()
{
    m_mutex.lock();

        m_headerList.clear();

    m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

bool ReportHeaderBase::reportsIsExist()
{
    QString dontExistReports;

    int reportCount = count();
    for (int r = 0; r < reportCount; r++)
    {
        REPORT_HEADER header = m_headerList[r];

        QString path = theOptions.report().m_path + QDir::separator() + header.m_reportFile;
        if (QFile::exists(path) == false)
        {
            dontExistReports.append("- " + header.m_reportFile + "\n");
        }
    }

    if (dontExistReports.isEmpty() == true)
    {
        return true;
    }

    dontExistReports.insert(0, "The application can not detect the following reports, functions of the application will be limited.\n\n");

    QMessageBox::information(nullptr, tr("Reports"), dontExistReports);

    return false;

}

// -------------------------------------------------------------------------------------------------------------------

void ReportHeaderBase::load()
{
    if (theDatabase.isOpen() == false)
    {
        return;
    }

    SqlTable* pTable = theDatabase.openTable(SQL_TABLE_REPORT_HEADER);
    if (pTable == nullptr)
    {
        return;
    }

    m_mutex.lock();

        int recordCount = pTable->recordCount();
        if (recordCount == REPORT_TYPE_COUNT)
        {
            m_headerList.resize(recordCount);
            pTable->read(m_headerList.data());
        }
        else
        {
            for(int type = 0; type < REPORT_TYPE_COUNT; type++)
            {
                REPORT_HEADER header;

                header.init(type);
                m_headerList.append(header);
            }

            if (pTable->clear() == true)
            {
                pTable->write(m_headerList.data(), m_headerList.count());
            }

        }

    m_mutex.unlock();

    pTable->close();

    reportsIsExist();
}

// -------------------------------------------------------------------------------------------------------------------

void ReportHeaderBase::save()
{
    if (theDatabase.isOpen() == false)
    {
        return;
    }

    SqlTable* pTable = theDatabase.openTable(SQL_TABLE_REPORT_HEADER);
    if (pTable == nullptr)
    {
        return;
    }

    if (pTable->clear() == true)
    {
        m_mutex.lock();

            pTable->write(m_headerList.data(), m_headerList.count());

        m_mutex.unlock();
    }

    pTable->close();
}


// -------------------------------------------------------------------------------------------------------------------

ReportHeaderBase& ReportHeaderBase::operator=(const ReportHeaderBase& from)
{
    clear();

    m_mutex.lock();

        m_headerList = from.m_headerList;

    m_mutex.unlock();

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------

REPORT_HEADER& ReportHeaderBase::operator[](int index)
{
    if (index < 0 || index >= count())
    {
        assert(0);
    }

    return m_headerList[index];
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ReportOption::ReportOption(QObject *parent) :
    QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

ReportOption::ReportOption(const ReportOption& from, QObject *parent) :
    QObject(parent)
{
    *this = from;
}

// -------------------------------------------------------------------------------------------------------------------

ReportOption::~ReportOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void ReportOption::init()
{

}

// -------------------------------------------------------------------------------------------------------------------

int ReportOption::reportTypeByMeasureType(int measureType)
{
    if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
    {
        return REPORT_TYPE_UNKNOWN;
    }

    int reportType = REPORT_TYPE_UNKNOWN;

    switch (measureType)
    {
        case MEASURE_TYPE_LINEARITY:

                switch(theOptions.linearity().m_viewType)
                {
                    case LO_VIEW_TYPE_SIMPLE:           reportType = REPORT_TYPE_LINEARITY;                 break;
                    case LO_VIEW_TYPE_EXTENDED:         reportType = REPORT_TYPE_LINEARITY_CERTIFICATION;   break;
                    case LO_VIEW_TYPE_DETAIL_ELRCTRIC:  reportType = REPORT_TYPE_LINEARITY_DETAIL_ELRCTRIC; break;
                    case LO_VIEW_TYPE_DETAIL_PHYSICAL:  reportType = REPORT_TYPE_LINEARITY_DETAIL_PHYSICAL; break;
                    default:                            reportType = REPORT_TYPE_UNKNOWN;                   break;
                }

            break;

        case MEASURE_TYPE_COMPARATOR:           reportType = REPORT_TYPE_COMPARATOR;                        break;
        case MEASURE_TYPE_COMPLEX_COMPARATOR:   reportType = REPORT_TYPE_COMPLEX_COMPARATOR;                break;
        default:                                reportType = REPORT_TYPE_UNKNOWN;                           break;
    }

    return reportType;
}

// -------------------------------------------------------------------------------------------------------------------

void ReportOption::load()
{
    QSettings s;

    m_path = s.value( QString("%1Path").arg(REPORT_OPTIONS_REG_KEY), QDir::currentPath() + "/reports").toString();
    m_type = s.value( QString("%1Type").arg(REPORT_OPTIONS_REG_KEY), REPORT_TYPE_LINEARITY).toInt();

    m_headerBase.load();
}

// -------------------------------------------------------------------------------------------------------------------

void ReportOption::save()
{
    QSettings s;

    s.setValue( QString("%1Path").arg(REPORT_OPTIONS_REG_KEY), m_path );
    s.setValue( QString("%1Type").arg(REPORT_OPTIONS_REG_KEY), m_type );

    m_headerBase.save();
}

// -------------------------------------------------------------------------------------------------------------------

ReportOption& ReportOption::operator=(const ReportOption& from)
{
    m_path = from.m_path;
    m_type = from.m_type;

    m_headerBase = from.m_headerBase;

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

void LinearityPoint::setPercent(double value)
{
    m_percentValue = value;

    for(int s = 0; s < POINT_SENSOR_COUNT; s++)
    {
        switch(s)
        {
            case POINT_SENSOR_PERCENT:      m_sensorValue[s] = value;                   break;
            case POINT_SENSOR_U_0_5_V:      m_sensorValue[s] = value * 5 / 100;         break;
            case POINT_SENSOR_I_0_5_MA:     m_sensorValue[s] = value * 5 / 100;         break;
            case POINT_SENSOR_I_4_20_MA:    m_sensorValue[s] = value * 16 / 100 + 4;    break;
            case POINT_SENSOR_T_0_100_C:    m_sensorValue[s] = value * 100 / 100;       break;
            case POINT_SENSOR_T_0_150_C:    m_sensorValue[s] = value * 150 / 100;       break;
            case POINT_SENSOR_T_0_200_C:    m_sensorValue[s] = value * 200 / 100;       break;
            case POINT_SENSOR_T_0_400_C:    m_sensorValue[s] = value * 400 / 100;       break;
            default:                        assert(0);                                  break;
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------

double LinearityPoint::sensorValue(int sensor)
{
    if (sensor < 0 || sensor >= POINT_SENSOR_COUNT)
    {
        return 0;
    }

    return m_sensorValue[sensor];
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

LinearityPointBase::LinearityPointBase(QObject *parent) :
    QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

LinearityPointBase::LinearityPointBase(const LinearityPointBase& from, QObject *parent) :
    QObject(parent)
{
    *this = from;
}

// -------------------------------------------------------------------------------------------------------------------

LinearityPointBase::~LinearityPointBase()
{
    clear();
}

// -------------------------------------------------------------------------------------------------------------------

int LinearityPointBase::count()
{
    int count = 0;

    m_mutex.lock();

        count = m_pointList.count();

    m_mutex.unlock();

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

int LinearityPointBase::append(LinearityPoint point)
{
    int index = -1;

    m_mutex.lock();

        m_pointList.append(point);
        index = m_pointList.count() - 1;

    m_mutex.unlock();

    return index;
}

// -------------------------------------------------------------------------------------------------------------------

int LinearityPointBase::insert(int index, LinearityPoint point)
{
    if (index < 0 || index > count())
    {
        return -1;
    }

    m_mutex.lock();

        m_pointList.insert(index, point);

    m_mutex.unlock();

    return index;
}

// -------------------------------------------------------------------------------------------------------------------

bool LinearityPointBase::remove(int index)
{
    if (index < 0 || index >= count())
    {
        return false;
    }

    m_mutex.lock();

        m_pointList.removeAt(index);

    m_mutex.unlock();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityPointBase::clear()
{
    m_mutex.lock();

        m_pointList.clear();

    m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityPointBase::swap(int i, int j)
{
    m_mutex.lock();

        LinearityPoint point    = m_pointList[j];
        m_pointList[j]          = m_pointList[i];
        m_pointList[i]          = point;

    m_mutex.unlock();

}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityPointBase::text()
{
    QString result;

    m_mutex.lock();

        if (m_pointList.isEmpty() == true)
        {
            result = tr("The measurement points are not set");
        }
        else
        {
            int count = m_pointList.count();
            for(int index = 0; index < count; index++)
            {
                LinearityPoint point = m_pointList.at(index);
                result.append(QString("%1%").arg(QString::number(point.percent(), 10, 1)));

                if (index != count - 1 )
                {
                    result.append(QString(", "));
                }
            }
        }

    m_mutex.unlock();

    return result;
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityPointBase::load()
{
    if (theDatabase.isOpen() == false)
    {
        return;
    }

    SqlTable* pTable = theDatabase.openTable(SQL_TABLE_LINEARETY_POINT);
    if (pTable == nullptr)
    {
        return;
    }

    m_mutex.lock();

        int recordCount = pTable->recordCount();
        if (recordCount != 0)
        {
            m_pointList.resize(recordCount);
            pTable->read(m_pointList.data());
        }
        else
        {
            const int valueCount = 7;
            double value[valueCount] = {2, 20, 40, 50, 60, 80, 98};

            for(int index = 0; index < valueCount; index++)
            {
                m_pointList.append( LinearityPoint( value[index] ) );
            }
        }

    m_mutex.unlock();

    pTable->close();
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityPointBase::save()
{
    if (theDatabase.isOpen() == false)
    {
        return;
    }

    SqlTable* pTable = theDatabase.openTable(SQL_TABLE_LINEARETY_POINT);
    if (pTable == nullptr)
    {
        return;
    }

    if (pTable->clear() == true)
    {
        m_mutex.lock();

            int count = m_pointList.count();
            for(int index = 0; index < count; index++)
            {
                m_pointList[index].setPointID(index);
            }

            pTable->write(m_pointList.data(), m_pointList.count());

        m_mutex.unlock();
    }

    pTable->close();
}


// -------------------------------------------------------------------------------------------------------------------

LinearityPointBase& LinearityPointBase::operator=(const LinearityPointBase& from)
{
    clear();

    m_mutex.lock();

        m_pointList = from.m_pointList;

    m_mutex.unlock();

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------

LinearityPoint& LinearityPointBase::operator[](int index)
{
    if (index < 0 || index >= count())
    {
        assert(0);
    }

    return m_pointList[index];
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

LinearityOption::LinearityOption(QObject *parent) :
    QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

LinearityOption::LinearityOption(const LinearityOption& from, QObject *parent) :
    QObject(parent)
{
    *this = from;
}

// -------------------------------------------------------------------------------------------------------------------


LinearityOption::~LinearityOption()
{
    m_pointBase.clear();
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityOption::recalcPoints(int count)
{
    if (m_rangeType != LO_RANGE_TYPE_AUTOMATIC)
    {
        return;
    }

    if (count == -1)
    {
        count = m_pointBase.count();
    }

     m_pointBase.clear();

    if (count == 0)
    {
        return;
    }

    if (count == 1)
    {
        m_pointBase.append(  LinearityPoint( (m_lowLimitRange + m_highLimitRange) / 2 ) );
    }
    else
    {
        double value = (double) (m_highLimitRange - m_lowLimitRange ) / (count - 1);

        for (int p = 0; p < count ; p++)
        {
            m_pointBase.append( LinearityPoint( m_lowLimitRange + ( p * value ) ) );
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityOption::load()
{
    QSettings s;

    m_pointBase.load();

    m_errorValue = s.value( QString("%1ErrorValue").arg(LINEARITY_OPTIONS_KEY), 0.2).toDouble();
    m_errorCtrl = s.value( QString("%1ErrorCtrl").arg(LINEARITY_OPTIONS_KEY), 0.1).toDouble();
    m_errorType  = s.value( QString("%1ErrorType").arg(LINEARITY_OPTIONS_KEY), ERROR_TYPE_REDUCE).toInt();
    m_errorCalcBySCO  = s.value( QString("%1ErrorCalcByMSE ").arg(LINEARITY_OPTIONS_KEY), false).toBool();

    m_measureTimeInPoint = s.value( QString("%1MeasureTimeInPoint").arg(LINEARITY_OPTIONS_KEY), 1).toInt();
    m_measureCountInPoint = s.value( QString("%1MeasureCountInPoint").arg(LINEARITY_OPTIONS_KEY), 20).toInt();

    m_rangeType = s.value( QString("%1RangeType").arg(LINEARITY_OPTIONS_KEY), LO_RANGE_TYPE_MANUAL).toInt();
    m_lowLimitRange = s.value( QString("%1LowLimitRange").arg(LINEARITY_OPTIONS_KEY), 0).toDouble();
    m_highLimitRange = s.value( QString("%1HighLimitRange").arg(LINEARITY_OPTIONS_KEY), 100).toDouble();

    m_viewType = s.value( QString("%1ViewType").arg(LINEARITY_OPTIONS_KEY), LO_VIEW_TYPE_SIMPLE).toInt();
    m_showOutputRangeColumn = s.value( QString("%1ShowOutputRangeColumn").arg(LINEARITY_OPTIONS_KEY), false).toBool();
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityOption::save()
{
    QSettings s;

    m_pointBase.save();

    s.setValue( QString("%1ErrorValue").arg(LINEARITY_OPTIONS_KEY), m_errorValue);
    s.setValue( QString("%1ErrorCtrl").arg(LINEARITY_OPTIONS_KEY), m_errorCtrl);
    s.setValue( QString("%1ErrorType").arg(LINEARITY_OPTIONS_KEY), m_errorType);
    s.setValue( QString("%1ErrorCalcByMSE").arg(LINEARITY_OPTIONS_KEY), m_errorCalcBySCO);

    s.setValue( QString("%1MeasureTimeInPoint").arg(LINEARITY_OPTIONS_KEY), m_measureTimeInPoint);
    s.setValue( QString("%1MeasureCountInPoint").arg(LINEARITY_OPTIONS_KEY), m_measureCountInPoint);

    s.setValue( QString("%1RangeType").arg(LINEARITY_OPTIONS_KEY), m_rangeType);
    s.setValue( QString("%1LowLimitRange").arg(LINEARITY_OPTIONS_KEY), m_lowLimitRange);
    s.setValue( QString("%1HighLimitRange").arg(LINEARITY_OPTIONS_KEY), m_highLimitRange);

    s.setValue( QString("%1ViewType").arg(LINEARITY_OPTIONS_KEY), m_viewType);
    s.setValue( QString("%1ShowOutputRangeColumn").arg(LINEARITY_OPTIONS_KEY), m_showOutputRangeColumn);
}

// -------------------------------------------------------------------------------------------------------------------

LinearityOption& LinearityOption::operator=(const LinearityOption& from)
{
    m_pointBase = from.m_pointBase;

    m_errorValue = from.m_errorValue;
    m_errorCtrl = from.m_errorCtrl;
    m_errorType = from.m_errorType;
    m_errorCalcBySCO = from.m_errorCalcBySCO;

    m_measureTimeInPoint = from.m_measureTimeInPoint;
    m_measureCountInPoint = from.m_measureCountInPoint;

    m_rangeType = from.m_rangeType;
    m_lowLimitRange = from.m_lowLimitRange;
    m_highLimitRange = from.m_highLimitRange;

    m_viewType = from.m_viewType;
    m_showOutputRangeColumn = from.m_showOutputRangeColumn;

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

ComparatorOption::ComparatorOption(QObject *parent) :
    QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorOption::ComparatorOption(const ComparatorOption& from, QObject *parent) :
    QObject(parent)
{
    *this = from;
}

// -------------------------------------------------------------------------------------------------------------------


ComparatorOption::~ComparatorOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorOption::load()
{
    QSettings s;

    m_errorValue = s.value( QString("%1ErrorValue").arg(COMPARATOR_OPTIONS_KEY), 0.2).toDouble();
    m_errorCtrl = s.value( QString("%1ErrorCtrl").arg(COMPARATOR_OPTIONS_KEY), 0.1).toDouble();
    m_startValue = s.value( QString("%1StartValue").arg(COMPARATOR_OPTIONS_KEY), 0.1).toDouble();
    m_errorType = s.value( QString("%1ErrorType").arg(COMPARATOR_OPTIONS_KEY), ERROR_TYPE_REDUCE).toInt();

    m_enableMeasureHysteresis = s.value( QString("%1EnableMeasureHysteresis").arg(COMPARATOR_OPTIONS_KEY), false).toBool();
    m_startComparatorNo = s.value( QString("%1StartSettingNo").arg(COMPARATOR_OPTIONS_KEY), 0).toInt();
    m_additionalCheck = s.value( QString("%1AdditionalCheck").arg(COMPARATOR_OPTIONS_KEY), true).toBool();
}

// -------------------------------------------------------------------------------------------------------------------

void ComparatorOption::save()
{
    QSettings s;

    s.setValue( QString("%1ErrorValue").arg(COMPARATOR_OPTIONS_KEY), m_errorValue);
    s.setValue( QString("%1ErrorCtrl").arg(COMPARATOR_OPTIONS_KEY), m_errorCtrl);
    s.setValue( QString("%1StartValue").arg(COMPARATOR_OPTIONS_KEY), m_startValue);
    s.setValue( QString("%1ErrorType").arg(COMPARATOR_OPTIONS_KEY), m_errorType);

    s.setValue( QString("%1EnableMeasureHysteresis").arg(COMPARATOR_OPTIONS_KEY), m_enableMeasureHysteresis);
    s.setValue( QString("%1StartSettingNo").arg(COMPARATOR_OPTIONS_KEY), m_startComparatorNo);
    s.setValue( QString("%1AdditionalCheck").arg(COMPARATOR_OPTIONS_KEY), m_additionalCheck);
}

// -------------------------------------------------------------------------------------------------------------------

ComparatorOption& ComparatorOption::operator=(const ComparatorOption& from)
{
    m_errorValue = from.m_errorValue;
    m_errorCtrl = from.m_errorCtrl;
    m_startValue = from.m_startValue;
    m_errorType = from.m_errorType;

    m_enableMeasureHysteresis = from.m_enableMeasureHysteresis;
    m_startComparatorNo = from.m_startComparatorNo;
    m_additionalCheck = from.m_additionalCheck;

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

BackupOption::BackupOption(QObject *parent) :
    QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

BackupOption::BackupOption(const BackupOption& from, QObject *parent) :
    QObject(parent)
{
    *this = from;
}

// -------------------------------------------------------------------------------------------------------------------

BackupOption::~BackupOption()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool BackupOption::createBackup()
{
    QString sourcePath = theOptions.database().m_path + QDir::separator() + DATABASE_NAME;
    QString destPath = m_path + QDir::separator() + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + DATABASE_NAME;

    if (QFile::copy(sourcePath, destPath) == false)
    {
        QMessageBox::critical(nullptr, tr("Backup"), tr("Error reserve copy database"));
    }

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void BackupOption::createBackupOnStart()
{
    if (m_onStart == true)
    {
        createBackup();
    }
}

// -------------------------------------------------------------------------------------------------------------------

void BackupOption::createBackupOnExit()
{
    if (m_onExit == true)
    {
        createBackup();
    }
}

// -------------------------------------------------------------------------------------------------------------------

void BackupOption::load()
{
    QTemporaryDir tmpDir;
    QString path = tmpDir.path().left( tmpDir.path().lastIndexOf(QDir::separator(), -1) );

    QSettings s;

    m_onStart = s.value( QString("%1OnStart").arg(BACKUP_OPTIONS_REG_KEY), false).toBool();
    m_onExit = s.value( QString("%1OnExit").arg(BACKUP_OPTIONS_REG_KEY), true).toBool();
    m_path = s.value( QString("%1Path").arg(BACKUP_OPTIONS_REG_KEY), path).toString();
}

// -------------------------------------------------------------------------------------------------------------------

void BackupOption::save()
{
    QSettings s;

    s.setValue( QString("%1OnStart").arg(BACKUP_OPTIONS_REG_KEY), m_onStart);
    s.setValue( QString("%1OnExit").arg(BACKUP_OPTIONS_REG_KEY), m_onExit);
    s.setValue( QString("%1Path").arg(BACKUP_OPTIONS_REG_KEY), m_path);
}

// -------------------------------------------------------------------------------------------------------------------

BackupOption& BackupOption::operator=(const BackupOption& from)
{
    m_onStart = from.m_onStart;
    m_onExit = from.m_onExit;
    m_path = from.m_path;

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

Options::Options(QObject *parent) :
    QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

Options::Options(const Options& from, QObject *parent) :
    QObject(parent)
{
    *this = from;
}

// -------------------------------------------------------------------------------------------------------------------

Options::~Options()
{
}

// -------------------------------------------------------------------------------------------------------------------

int Options::channelCount()
{
    int count = 0;

    switch(m_toolBar.m_measureKind)
    {
        case MEASURE_KIND_ONE:      count = 1;                      break;
        case MEASURE_KIND_MULTI:    count = MAX_CALIBRATOR_COUNT;   break;
        default:                    assert(0);                      break;
    }

    return count;
}

// -------------------------------------------------------------------------------------------------------------------

void Options::load()
{
    m_toolBar.load();

    m_connectTcpIp.load();

    m_measureView.init();
    m_measureView.load();

    m_database.load();
    theDatabase.open();

    m_report.load();
    m_report.init();

    m_linearity.load();

    m_comparator.load();

    m_backup.load();
}

// -------------------------------------------------------------------------------------------------------------------

void Options::save()
{
    m_toolBar.save();
    m_connectTcpIp.save();
    m_measureView.save();
    m_database.save();
    m_report.save();
    m_linearity.save();
    m_comparator.save();
    m_backup.save();
}

// -------------------------------------------------------------------------------------------------------------------

Options& Options::operator=(const Options& from)
{
    m_mutex.lock();

        for(int type = 0; type < MEASURE_TYPE_COUNT; type++)
        {
            m_updateColumnView[type] = from.m_updateColumnView[type];
        }

        m_toolBar = from.m_toolBar;
        m_connectTcpIp = from.m_connectTcpIp;
        m_measureView = from.m_measureView;
        m_database = from.m_database;
        m_report = from.m_report;
        m_linearity = from.m_linearity;
        m_comparator = from.m_comparator;
        m_backup = from.m_backup;

    m_mutex.unlock();

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
