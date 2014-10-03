#ifndef OPTIONS_H
#define OPTIONS_H

#include <QObject>
#include <QMutex>
#include <assert.h>
#include "Measure.h"


// ==============================================================================================

#define                     WINDOW_GEOMETRY_OPTIONS_KEY		"Options/Window/"

// ----------------------------------------------------------------------------------------------

void restoreWindowPosition(QWidget* pWidget);
void saveWindowPosition(QWidget* pWidget);

// ==============================================================================================

#define						TCPIP_OPTIONS_KEY               "Options/TcpIp/"

// ----------------------------------------------------------------------------------------------

const char* const			TcpIpParamName[] =
{
                            QT_TRANSLATE_NOOP("Options.h", "IP"),
                            QT_TRANSLATE_NOOP("Options.h", "Port"),
};

const int					TCPIP_PARAM_COUNT				= sizeof(TcpIpParamName)/sizeof(char*);

const int					TCPIP_PARAM_SERVER_IP			= 0,
                            TCPIP_PARAM_SERVER_PORT         = 1;

// ----------------------------------------------------------------------------------------------

class TcpIpOption : public QObject
{
    Q_OBJECT

public:
    explicit    TcpIpOption(QObject *parent = 0);
    explicit    TcpIpOption(const TcpIpOption& from, QObject *parent = 0);
               ~TcpIpOption();

    QString     m_serverIP = "127.0.0.1";
    int         m_serverPort = 2000;

    void        load();
    void        save();

    TcpIpOption&  operator=(const TcpIpOption& from);
};

// ==============================================================================================

#define						TOOLBAR_OPTIONS_KEY             "Options/ToolBar/"

// ----------------------------------------------------------------------------------------------

class ToolBarOption : public QObject
{
    Q_OBJECT

public:
    explicit    ToolBarOption(QObject *parent = 0);
    explicit    ToolBarOption(const ToolBarOption& from, QObject *parent = 0);
               ~ToolBarOption();

    int         m_measureTimeout = 0;                               // in milliseconds, timeout between the time when the calibrator is set value and the time when the application is save measurement
    int			m_measureKind = MEASURE_KIND_ONE;                   // measure kind: each channel separately - 0 or for all channels together - 1
    int         m_outputSignalType = OUTPUT_SIGNAL_TYPE_DONT_USED;  // selected type of output signal

    void        load();
    void        save();

    ToolBarOption&  operator=(const ToolBarOption& from);
};


// ==============================================================================================

const char* const LinearityPointSensor[] =
{
                QT_TRANSLATE_NOOP("Options.h", "%"),
                QT_TRANSLATE_NOOP("Options.h", "0 - 5 V"),
                QT_TRANSLATE_NOOP("Options.h", "0 - 5 mA"),
                QT_TRANSLATE_NOOP("Options.h", "4 - 20 mA"),
                QT_TRANSLATE_NOOP("Options.h", "0 - 100 °C"),
                QT_TRANSLATE_NOOP("Options.h", "0 - 150 °C"),
                QT_TRANSLATE_NOOP("Options.h", "0 - 200 °C"),
                QT_TRANSLATE_NOOP("Options.h", "0 - 400 °C"),
};

const int		POINT_SENSOR_COUNT           = sizeof(LinearityPointSensor)/sizeof(char*);

const int		POINT_SENSOR_UNKNOWN        = -1,
                POINT_SENSOR_PERCENT        = 0,
                POINT_SENSOR_U_0_5_V        = 1,
                POINT_SENSOR_I_0_5_MA       = 2,
                POINT_SENSOR_I_4_20_MA      = 3,
                POINT_SENSOR_T_0_100_C      = 4,
                POINT_SENSOR_T_0_150_C      = 5,
                POINT_SENSOR_T_0_200_C      = 6,
                POINT_SENSOR_T_0_400_C      = 7;


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
    int                     insert(int index, LinearityPoint* point);

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

#define						LINEARITY_OPTIONS_KEY           "Options/Linearity/"

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

    QMutex              m_mutex;

    ToolBarOption       m_toolBar;
    TcpIpOption         m_connectTcpIp;
    LinearityOption     m_linearity;

public:

    void                setToolBar(const ToolBarOption& toolBar)            { m_toolBar = toolBar; }
    ToolBarOption&      getToolBar()                                        { return m_toolBar; }

    void                setTcpIp(const TcpIpOption& connectTcpIp)           { m_connectTcpIp = connectTcpIp; }
    TcpIpOption&        getTcpIp()                                          { return m_connectTcpIp; }

    void                setLinearity(const LinearityOption& linearity)      { m_linearity = linearity; }
    LinearityOption&    getLinearity()                                      { return m_linearity; }

    void				load();
    void				save();

    inline Options&     operator=(const Options& from);
};


// ==============================================================================================

extern Options theOptions;

// ==============================================================================================

#endif // OPTIONS_H
