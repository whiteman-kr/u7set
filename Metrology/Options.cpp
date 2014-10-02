 #include "Options.h"

#include <QSettings>
#include <QWidget>

// -------------------------------------------------------------------------------------------------------------------

Options theOptions;

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

void restoreWindowPosition(QWidget* pWidget)
{
    if (pWidget == nullptr)
    {
        return;
    }

    QSettings s;

    QString className = pWidget->metaObject()->className();
    QByteArray geometry = s.value(QString("%1%2/geometry").arg(WINDOW_GEOMETRY_OPTIONS_KEY).arg(className)).toByteArray();

    pWidget->restoreGeometry( geometry );
}

// -------------------------------------------------------------------------------------------------------------------

void saveWindowPosition(QWidget* pWidget)
{
    if (pWidget == nullptr)
    {
        return;
    }

    QSettings s;

    QString className = pWidget->metaObject()->className();

    s.setValue(QString("%1%2/geometry").arg(WINDOW_GEOMETRY_OPTIONS_KEY).arg(className), pWidget->saveGeometry());
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

    m_measureKind = s.value( QString("%1MeasureKind").arg(TOOLBAR_OPTIONS_KEY), MEASURE_KIND_ONE).toInt();
    m_measureTimeout = s.value( QString("%1MeasureTimeout").arg(TOOLBAR_OPTIONS_KEY), 0).toInt();
    m_outputSignalType = s.value( QString("%1OutputSignalType").arg(TOOLBAR_OPTIONS_KEY), OUTPUT_SIGNAL_TYPE_UNKNOWN).toInt();
}

// -------------------------------------------------------------------------------------------------------------------

void ToolBarOption::save()
{
    QSettings s;

    s.setValue( QString("%1MeasureKind").arg(TOOLBAR_OPTIONS_KEY), m_measureKind);
    s.setValue( QString("%1MeasureTimeout").arg(TOOLBAR_OPTIONS_KEY), m_measureTimeout);
    s.setValue( QString("%1OutputSignalType").arg(TOOLBAR_OPTIONS_KEY), m_outputSignalType);
}

// -------------------------------------------------------------------------------------------------------------------

ToolBarOption& ToolBarOption::operator=(const ToolBarOption& from)
{
    m_measureKind = from.m_measureKind;
    m_measureTimeout = from.m_measureTimeout;
    m_outputSignalType = from.m_outputSignalType;

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
            const int count = 6;
            double value[count] = {2, 20, 40, 60, 80, 98};

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
    m_errorType  = s.value( QString("%1ErrorType").arg(LINEARITY_OPTIONS_KEY), MEASURE_ERROR_TYPE_REDUCE).toInt();
    m_errorCalcBySCO  = s.value( QString("%1ErrorCalcBySCO ").arg(LINEARITY_OPTIONS_KEY), false).toBool();

    m_measureTimeInPoint = s.value( QString("%1MeasureTimeInPoint").arg(LINEARITY_OPTIONS_KEY), 1).toInt();
    m_measureCountInPoint = s.value( QString("%1MeasureCountInPoint").arg(LINEARITY_OPTIONS_KEY), 20).toInt();

    m_rangeType = s.value( QString("%1RangeType").arg(LINEARITY_OPTIONS_KEY), LO_RANGE_TYPE_MANUAL).toInt();
    m_lowLimitRange = s.value( QString("%1LowLimitRange").arg(LINEARITY_OPTIONS_KEY), 0).toDouble();
    m_highLimitRange = s.value( QString("%1HighLimitRange").arg(LINEARITY_OPTIONS_KEY), 100).toDouble();

    m_showOutputRangeColumn = s.value( QString("%1ShowOutputRangeColumn").arg(LINEARITY_OPTIONS_KEY), false).toBool();
    m_considerCorrectOutput = s.value( QString("%1ConsiderCorrectOutput").arg(LINEARITY_OPTIONS_KEY), false).toBool();
}

// -------------------------------------------------------------------------------------------------------------------

void LinearityOption::save()
{
    QSettings s;

    m_pointBase.save();

    s.setValue( QString("%1ErrorValue").arg(LINEARITY_OPTIONS_KEY), m_errorValue);
    s.setValue( QString("%1ErrorCtrl").arg(LINEARITY_OPTIONS_KEY), m_errorCtrl);
    s.setValue( QString("%1ErrorType").arg(LINEARITY_OPTIONS_KEY), m_errorType);
    s.setValue( QString("%1ErrorCalcBySCO").arg(LINEARITY_OPTIONS_KEY), m_errorCalcBySCO);

    s.setValue( QString("%1MeasureTimeInPoint").arg(LINEARITY_OPTIONS_KEY), m_measureTimeInPoint);
    s.setValue( QString("%1MeasureCountInPoint").arg(LINEARITY_OPTIONS_KEY), m_measureCountInPoint);

    s.setValue( QString("%1RangeType").arg(LINEARITY_OPTIONS_KEY), m_rangeType);
    s.setValue( QString("%1LowLimitRange").arg(LINEARITY_OPTIONS_KEY), m_lowLimitRange);
    s.setValue( QString("%1HighLimitRange").arg(LINEARITY_OPTIONS_KEY), m_highLimitRange);

    s.setValue( QString("%1ShowOutputRangeColumn").arg(LINEARITY_OPTIONS_KEY), m_showOutputRangeColumn);
    s.setValue( QString("%1ConsiderCorrectOutput").arg(LINEARITY_OPTIONS_KEY), m_considerCorrectOutput);
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

    m_showOutputRangeColumn = from.m_showOutputRangeColumn;
    m_considerCorrectOutput = from.m_considerCorrectOutput;

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

void Options::load()
{
    m_linearity.load();
}

// -------------------------------------------------------------------------------------------------------------------

void Options::save()
{
    m_linearity.save();
}

// -------------------------------------------------------------------------------------------------------------------

Options& Options::operator=(const Options& from)
{
    m_mutex.lock();

        m_linearity = from.m_linearity;

    m_mutex.unlock();

    return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
