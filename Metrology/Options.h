#ifndef OPTIONS_H
#define OPTIONS_H

#include <QObject>
#include <QMutex>
#include "Measure.h"


// ==============================================================================================

const char* const LinearityPointSensor[] =
{
                QT_TRANSLATE_NOOP("Options.h", "0 - 5 mA"),
                QT_TRANSLATE_NOOP("Options.h", "4 - 20 mA"),
};

const int		POINT_SENSOR_COUNT           = sizeof(LinearityPointSensor)/sizeof(char*);

const int		POINT_SENSOR_UNKNOWN         = -1,
                POINT_SENSOR_I_0_5_MA        = 0,
                POINT_SENSOR_I_4_20_MA       = 1;

// ----------------------------------------------------------------------------------------------

class LinearityPoint
{
public:

    explicit    LinearityPoint();
    explicit    LinearityPoint(double percent);

private:

    int         m_pointID = -1;

    double      m_percentValue = 0;
    double      m_sensorValue[POINT_SENSOR_COUNT];

public:

    void        setID(int id);

    void        setPercent(double value);
    double      getPrecent() ;

    double      getSensorValue(int sensor);
};


// ==============================================================================================

class LinearityPointBase : public QObject
{
    Q_OBJECT

private:

    QMutex                  m_mutex;

    QList<LinearityPoint*>	m_pointList;                                  // list of measurement points

public:

    explicit                LinearityPointBase(QObject *parent = 0);
    explicit                LinearityPointBase(const LinearityPointBase& from, QObject *parent = 0);
                            ~LinearityPointBase();

    int                     count();
    bool                    isEmpty() { return count() == 0; }

    int                     append(LinearityPoint* point);

    bool                    removeAt(int index);
    bool                    removeAt(LinearityPoint* point);

    void                    clear();

    LinearityPoint*         at(int index);

    void                    swap(int i, int j);

    QString                 text();

    void					load();
    void					save();

    LinearityPointBase&     operator=(const LinearityPointBase& from);
};


// ==============================================================================================

#define						LINEARITY_OPTIONS_KEY		"Options/Linearity/"

// ----------------------------------------------------------------------------------------------

const char* const			LinearityParamName[] =
{
                            QT_TRANSLATE_NOOP("Options.h", "Permissible error"),
                            QT_TRANSLATE_NOOP("Options.h", "Control error"),
                            QT_TRANSLATE_NOOP("Options.h", "Error type"),
                            QT_TRANSLATE_NOOP("Options.h", "Calculate error by standard deviation"),
                            QT_TRANSLATE_NOOP("Options.h", "Measure time in a point, (sec)"),
                            QT_TRANSLATE_NOOP("Options.h", "Count of measurements in a point"),
                            QT_TRANSLATE_NOOP("Options.h", "Division of the measure range"),
                            QT_TRANSLATE_NOOP("Options.h", "Count of points"),
                            QT_TRANSLATE_NOOP("Options.h", "Lower limit of the measure range, (%)"),
                            QT_TRANSLATE_NOOP("Options.h", "High limit of the measure range, (%)"),
                            QT_TRANSLATE_NOOP("Options.h", "Points of range"),
                            QT_TRANSLATE_NOOP("Options.h", "Show column of output values"),
                            QT_TRANSLATE_NOOP("Options.h", "Consider correction for output signals"),
};

const int					LO_PARAM_COUNT				= sizeof(LinearityParamName)/sizeof(char*);

const int					LO_PARAM_ERROR				= 0,
                            LO_PARAM_ERROR_CTRL			= 1,
                            LO_PARAM_ERROR_TYPE			= 2,
                            LO_PARAM_ERROR_BY_SCO		= 3,
                            LO_PARAM_MEASURE_TIME		= 4,
                            LO_PARAM_MEASURE_IN_POINT	= 5,
                            LO_PARAM_RANGE_TYPE			= 6,
                            LO_PARAM_POINT_COUNT		= 7,
                            LO_PARAM_LOW_RANGE			= 8,
                            LO_PARAM_HIGH_RANGE			= 9,
                            LO_PARAM_VALUE_POINTS       = 10,
                            LO_PARAM_OUTPUT_RANGE		= 11,
                            LO_PARAM_CORRECT_OUTPUT		= 12;

// ----------------------------------------------------------------------------------------------

const char* const			LinearityRangeTypeStr[] =
{
                            QT_TRANSLATE_NOOP("Options.h", "Manual division of the measure range"),
                            QT_TRANSLATE_NOOP("Options.h", "Automatic division of the measure range"),
};

const int					LO_RANGE_TYPE_COUNT			= sizeof(LinearityRangeTypeStr)/sizeof(char*);

const int					LO_RANGE_TYPE_MANUAL		= 0,
                            LO_RANGE_TYPE_AUTOMATIC		= 1;

// ----------------------------------------------------------------------------------------------

class LinearityOption : public QObject
{
    Q_OBJECT

public:
    explicit    LinearityOption(QObject *parent = 0);
    explicit    LinearityOption(const LinearityOption& from, QObject *parent = 0);
               ~LinearityOption();

    LinearityPointBase      m_pointBase;                                  // list of measurement points

    double					m_errorValue = 0.5;                           // permissible error is given by specified documents
    double					m_errorCtrl = 0.1;                            // control error is given by metrologists
    int						m_errorType = MEASURE_ERROR_TYPE_REDUCE;      // type of error absolute or reduced
    bool					m_errorCalcBySCO = false;                     // сalculate error by standard deviation

    int						m_measureTimeInPoint = 1;                     // time, in seconds, during which will be made ​​N measurements at each point
    int						m_measureCountInPoint = 20;                   // количество измерений в точке, согласно госту МИ 2002-89 приложение 7

    int						m_rangeType = LO_RANGE_TYPE_MANUAL;           // type of division measure range: manual - 0 or automatic - 1
    double                  m_lowLimitRange = 0;                          // lower limit of the range for automatic division
    double					m_highLimitRange = 100;                       // high limit of the range for automatic division

    bool					m_showOutputRangeColumn = false;              // show column output values
    bool					m_considerCorrectOutput = false;              // take into account the correction for signals with output range

    void					recalcPoints(int count = -1);

    void					load();
    void					save();

    LinearityOption&        operator=(const LinearityOption& from);
};

// ==============================================================================================

class Options : public QObject
{
    Q_OBJECT

public:
    explicit Options(QObject *parent = 0);
    explicit Options(const Options& from, QObject *parent = 0);
            ~Options();

private:

    LinearityOption     m_linearity;

public:

    void                setLinearity(const LinearityOption& linearity)  { m_linearity = linearity; }
    LinearityOption&    getLinearity()                                  { return m_linearity; }

    void				load();
    void				save();

    inline Options&     operator=(const Options& from);
};


// ==============================================================================================

#endif // OPTIONS_H
