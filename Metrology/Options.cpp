 #include "Options.h"

#include <QSettings>
#include <QWidget>
#include <QMainWindow>
#include "CalibratorBase.h"

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

LinearityPoint::LinearityPoint()
{
    setPercent(0);
}

// -------------------------------------------------------------------------------------------------------------------

LinearityPoint::LinearityPoint(double percent)
{
    setPercent(percent);
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityPoint::setID(int id)
{
    m_pointID = id;
}

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

double LinearityPoint::getPrecent()
{
    return m_percentValue;
}

// -------------------------------------------------------------------------------------------------------------------

double LinearityPoint::getSensorValue(int sensor)
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

int LinearityPointBase::append(LinearityPoint* point)
{
    if (point == nullptr)
    {
        return -1;
    }

    int index = -1;

    m_mutex.lock();

        m_pointList.append(point);
        index = m_pointList.count() - 1;

    m_mutex.unlock();

    return index;
}

// -------------------------------------------------------------------------------------------------------------------

int LinearityPointBase::insert(int index, LinearityPoint* point)
{
    if (index < 0 || index > count())
    {
        return -1;
    }

    if (point == nullptr)
    {
        return -1;
    }

    m_mutex.lock();

        m_pointList.insert(index, point);

    m_mutex.unlock();

    return index;
}

// -------------------------------------------------------------------------------------------------------------------

bool LinearityPointBase::removeAt(int index)
{
    if (index < 0 || index >= count())
    {
        return false;
    }

    m_mutex.lock();

        LinearityPoint* point = m_pointList.at(index);
        if (point != nullptr)
        {
            delete point;
        }
        m_pointList.removeAt(index);

    m_mutex.unlock();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool LinearityPointBase::removeAt(LinearityPoint* point)
{
    if (point == nullptr)
    {
        return false;
    }

    m_mutex.lock();

        int index = m_pointList.indexOf(point);
        if (index != -1)
        {
            LinearityPoint* point = m_pointList.at(index);
            if (point != nullptr)
            {
                delete point;
            }
            m_pointList.removeAt(index);
        }

    m_mutex.unlock();

    return true;
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityPointBase::clear()
{
    m_mutex.lock();

        int count = m_pointList.count();
        for(int index = 0; index < count ; index ++)
        {
            LinearityPoint* point = m_pointList.at(index);
            if (point != nullptr)
            {
                delete point;
            }
        }

        m_pointList.clear();

    m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

LinearityPoint* LinearityPointBase::at(int index)
{
    if (index < 0 || index >= count())
    {
        return nullptr;
    }

    LinearityPoint* point = nullptr;

    m_mutex.lock();

        point = m_pointList[index];

    m_mutex.unlock();

    return point;
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityPointBase::swap(int i, int j)
{
    m_mutex.lock();

        m_pointList.swap(i, j);

    m_mutex.unlock();

}

// -------------------------------------------------------------------------------------------------------------------

QString LinearityPointBase::text()
{
    QString retText;

    m_mutex.lock();

        if (m_pointList.isEmpty() == true)
        {
            retText = tr("The measurement points are not set");
        }
        else
        {
            int count = m_pointList.count();
            for(int index = 0; index < count; index++)
            {
                LinearityPoint* point = m_pointList.at(index);
                if (point != nullptr)
                {
                    retText.append(QString("%1%").arg(QString::number(point->getPrecent(), 10, 1)));
                }
                else
                {
                    retText.append(QString("?%"));
                }

                if (index != count - 1 )
                {
                    retText.append(QString(", "));
                }
            }
        }

    m_mutex.unlock();

    return retText;
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityPointBase::load()
{
    // load from table of database

    m_mutex.lock();

        if (m_pointList.isEmpty() == true)
        {
            const int count = 7;
            double value[count] = {2, 20, 40, 50, 60, 80, 98};

            for(int index = 0; index < count; index++)
            {
                LinearityPoint* point = new LinearityPoint( value[index] );
                if (point != nullptr)
                {
                    m_pointList.append( point );
                }
            }
        }

    m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityPointBase::save()
{
    m_mutex.lock();

        int count = m_pointList.count();
        for(int index = 0; index < count; index++)
        {
            LinearityPoint* point = m_pointList.at(index);
            if (point != nullptr)
            {
                point->setID(index);
            }
        }

    m_mutex.unlock();

    // save to table of database
}


// -------------------------------------------------------------------------------------------------------------------

LinearityPointBase& LinearityPointBase::operator=(const LinearityPointBase& from)
{
    clear();

    m_mutex.lock();

        int count = from.m_pointList.count();
        for(int index = 0; index < count; index++)
        {
            LinearityPoint* from_point = from.m_pointList.at(index);
            if (from_point != nullptr)
            {
                LinearityPoint* point = new LinearityPoint( from_point->getPrecent() );
                if (point != nullptr)
                {
                    m_pointList.append(point);
                }
            }
        }

    m_mutex.unlock();

    return *this;
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
        LinearityPoint* point = new LinearityPoint( (m_lowLimitRange + m_highLimitRange) / 2 );
        m_pointBase.append( point );
    }
    else
    {
        double value = (double) (m_highLimitRange - m_lowLimitRange ) / (count - 1);

        for (int p = 0; p < count ; p++)
        {
            LinearityPoint* point = new LinearityPoint( m_lowLimitRange + ( p * value ) );
            m_pointBase.append( point );
        }
    }
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityOption::load()
{
    QSettings s;

    m_pointBase.load();

    m_errorValue = s.value( QString("%1ErrorValue").arg(LINEARITY_OPTIONS_KEY), 0.5).toDouble();
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

int Options::getChannelCount()
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
    m_linearity.load();
}

// -------------------------------------------------------------------------------------------------------------------

void Options::save()
{
    m_toolBar.save();
    m_connectTcpIp.save();
    m_measureView.save();
    m_linearity.save();
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
        m_linearity = from.m_linearity;

    m_mutex.unlock();

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
