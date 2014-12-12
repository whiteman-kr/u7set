#ifndef OPTIONS_H
#define OPTIONS_H

#include <assert.h>

#include <QObject>
#include <QMutex>
#include <QVector>
#include <QColor>
#include <QFont>
#include <QTimer>

#include "CalibratorBase.h"
#include "Measure.h"
#include "MeasureViewHeader.h"

// ==============================================================================================

#define                 WINDOW_GEOMETRY_OPTIONS_KEY		"Options/Window/"

// ==============================================================================================

#define                 TCPIP_OPTIONS_KEY               "Options/TcpIp/"

// ----------------------------------------------------------------------------------------------

const char* const       TcpIpParamName[] =
{
                        QT_TRANSLATE_NOOP("Options.h", "IP"),
                        QT_TRANSLATE_NOOP("Options.h", "Port"),
};

const int               TCPIP_PARAM_COUNT       = sizeof(TcpIpParamName)/sizeof(char*);

const int               TCPIP_PARAM_SERVER_IP   = 0,
                        TCPIP_PARAM_SERVER_PORT = 1;

// ----------------------------------------------------------------------------------------------

class TcpIpOption : public QObject
{
    Q_OBJECT

public:
    explicit            TcpIpOption(QObject *parent = 0);
    explicit            TcpIpOption(const TcpIpOption& from, QObject *parent = 0);
                        ~TcpIpOption();

    QString             m_serverIP = "127.0.0.1";
    int                 m_serverPort = 2000;

    void                load();
    void                save();

    TcpIpOption&        operator=(const TcpIpOption& from);
};

// ==============================================================================================

#define                 TOOLBAR_OPTIONS_KEY             "Options/ToolBar/"

// ----------------------------------------------------------------------------------------------

class ToolBarOption : public QObject
{
    Q_OBJECT

public:
    explicit            ToolBarOption(QObject *parent = 0);
    explicit            ToolBarOption(const ToolBarOption& from, QObject *parent = 0);
                        ~ToolBarOption();

    int                 m_measureTimeout = 0;                               // in milliseconds, timeout between the time when the calibrator is set value and the time when the application is save measurement
    int                 m_measureKind = MEASURE_KIND_ONE;                   // measure kind: each channel separately - 0 or for all channels together - 1
    int                 m_outputSignalType = OUTPUT_SIGNAL_TYPE_DONT_USED;  // selected type of output signal

    void                load();
    void                save();

    ToolBarOption&      operator=(const ToolBarOption& from);
};

// ==============================================================================================

#define                 MEASURE_VIEW_OPTIONS_KEY        "Options/MeasureView/"

// ----------------------------------------------------------------------------------------------

const char* const		MeasureViewParam[] =
{
                        QT_TRANSLATE_NOOP("Options.h", "Font of measurements list"),
                        QT_TRANSLATE_NOOP("Options.h", "Show external ID"),
                        QT_TRANSLATE_NOOP("Options.h", "Displaying value"),
                        QT_TRANSLATE_NOOP("Options.h", "Measurement over limit error"),
                        QT_TRANSLATE_NOOP("Options.h", "Measurement over control error"),
};

const int				MWO_PARAM_COUNT					= sizeof(MeasureViewParam)/sizeof(char*);

const int				MWO_PARAM_FONT					= 0,
                        MWO_PARAM_ID					= 1,
                        MWO_PARAM_DISPLAYING_VALUE      = 2,
                        MWO_PARAM_COLOR_LIMIT_ERROR     = 3,
                        MWO_PARAM_COLOR_CONTROL_ERROR   = 4;

// ----------------------------------------------------------------------------------------------

#define                 COLOR_LIMIT_ERROR               QColor(0xFF, 0xD0, 0xD0)
#define					COLOR_CONTROL_ERROR             QColor(0xFF, 0xFF, 0x99)

// ----------------------------------------------------------------------------------------------

const char* const       DisplayingValueType[] =
{
                        QT_TRANSLATE_NOOP("Options.h", "Show in physical units"),
                        QT_TRANSLATE_NOOP("Options.h", "Show in electrical units"),
                        QT_TRANSLATE_NOOP("Options.h", "Displayed as a percentage (%) of the range"),
};

const int				DISPLAYING_VALUE_TYPE_COUNT     = sizeof(DisplayingValueType)/sizeof(char*);

const int				DISPLAYING_VALUE_TYPE_PHYSICAL  = 0,
                        DISPLAYING_VALUE_TYPE_ELECTRIC  = 1,
                        DISPLAYING_VALUE_TYPE_PERCENT   = 2;

// ----------------------------------------------------------------------------------------------

class MeasureViewOption : public QObject
{
    Q_OBJECT

public:
    explicit            MeasureViewOption(QObject *parent = 0);
    explicit            MeasureViewOption(const MeasureViewOption& from, QObject *parent = 0);
                        ~MeasureViewOption();

    MeasureViewColumn   m_column[MEASURE_TYPE_COUNT][MEASURE_VIEW_COLUMN_COUNT];

    int					m_measureType = MEASURE_TYPE_UNKNOWN;						// current, active ViewID

    QFont				m_font;
    QFont				m_fontBold;

    bool				m_showExternalID = true;
    int					m_showDisplayingValueType = DISPLAYING_VALUE_TYPE_PHYSICAL;

    QColor              m_colorLimitError = COLOR_LIMIT_ERROR;
    QColor              m_colorControlError = COLOR_CONTROL_ERROR;

    void                init();

    void                load();
    void                save();

    MeasureViewOption&  operator=(const MeasureViewOption& from);
};

// ==============================================================================================

#define                 DATABASE_OPTIONS_REG_KEY		"Options/Database/"

// ----------------------------------------------------------------------------------------------

const char* const		DatabaseParam[] =
{
                        QT_TRANSLATE_NOOP("Options.h", "Path"),
                        QT_TRANSLATE_NOOP("Options.h", "Type"),
};

const int				DBO_PARAM_COUNT = sizeof(DatabaseParam)/sizeof(char*);

const int				DBO_PARAM_PATH  = 0,
                        DBO_PARAM_TYPE  = 1;

// ----------------------------------------------------------------------------------------------

const char* const       DatabaseType[] =
{
                        QT_TRANSLATE_NOOP("Options.h", "SQLite"),
};

const int				DATABASE_TYPE_COUNT     = sizeof(DatabaseType)/sizeof(char*);

const int				DATABASE_TYPE_SQLITE    = 0;


// ----------------------------------------------------------------------------------------------

class DatabaseOption : public QObject
{
    Q_OBJECT

public:
    explicit            DatabaseOption(QObject *parent = 0);
    explicit            DatabaseOption(const DatabaseOption& from, QObject *parent = 0);
                        ~DatabaseOption();

    QString             m_path;
    int					m_type;

    void                load();
    void                save();

    DatabaseOption&     operator=(const DatabaseOption& from);
};

// ==============================================================================================

#define                 REPORT_OPTIONS_REG_KEY		"Options/Reports/"

// ----------------------------------------------------------------------------------------------

const char* const		ReportParam[] =
{
                        QT_TRANSLATE_NOOP("Options.h", "Path"),
                        QT_TRANSLATE_NOOP("Options.h", "Type"),
                        QT_TRANSLATE_NOOP("Options.h", "Document title"),
                        QT_TRANSLATE_NOOP("Options.h", "Report title"),
                        QT_TRANSLATE_NOOP("Options.h", "Date of measuring"),
                        QT_TRANSLATE_NOOP("Options.h", "Table title"),
                        QT_TRANSLATE_NOOP("Options.h", "Conclusion"),
                        QT_TRANSLATE_NOOP("Options.h", "Environment temperature, °С"),
                        QT_TRANSLATE_NOOP("Options.h", "Atmospheric pressure, kPa"),
                        QT_TRANSLATE_NOOP("Options.h", "Relative humidity, %"),
                        QT_TRANSLATE_NOOP("Options.h", "Power voltage, V"),
                        QT_TRANSLATE_NOOP("Options.h", "Power frequency, Hz"),
                        QT_TRANSLATE_NOOP("Options.h", "Calibrator 1"),
                        QT_TRANSLATE_NOOP("Options.h", "Calibrator 2"),
                        QT_TRANSLATE_NOOP("Options.h", "Calibrator 3"),
                        QT_TRANSLATE_NOOP("Options.h", "Calibrator 4"),
                        QT_TRANSLATE_NOOP("Options.h", "Calibrator 5"),
                        QT_TRANSLATE_NOOP("Options.h", "Calibrator 6"),
                        QT_TRANSLATE_NOOP("Options.h", "File name"),
};

const int				RO_PARAM_COUNT          = sizeof(ReportParam)/sizeof(char*);

const int				RO_PARAM_PATH           = 0,
                        RO_PARAM_TYPE           = 1,
                        RO_PARAM_DOCUMENT_TITLE = 2,
                        RO_PARAM_REPORT_TITLE   = 3,
                        RO_PARAM_DATE			= 4,
                        RO_PARAM_TABLE_TITLE    = 5,
                        RO_PARAM_CONCLUSION		= 6,
                        RO_PARAM_T				= 7,
                        RO_PARAM_P              = 8,
                        RO_PARAM_H				= 9,
                        RO_PARAM_V				= 10,
                        RO_PARAM_F				= 11,
                        RO_PARAM_CALIBRATOR_0   = 12,
                        RO_PARAM_CALIBRATOR_1	= 13,
                        RO_PARAM_CALIBRATOR_2	= 14,
                        RO_PARAM_CALIBRATOR_3	= 15,
                        RO_PARAM_CALIBRATOR_4	= 16,
                        RO_PARAM_CALIBRATOR_5	= 17,
                        RO_PARAM_REPORT_FILE    = 18;


// ----------------------------------------------------------------------------------------------

const char* const       ReportType[] =
{
                        QT_TRANSLATE_NOOP("Options.h", "Linearity"),
                        QT_TRANSLATE_NOOP("Options.h", "Linearity metrological certification"),
                        QT_TRANSLATE_NOOP("Options.h", "Linearity detail electric"),
                        QT_TRANSLATE_NOOP("Options.h", "Linearity detail physical"),
                        QT_TRANSLATE_NOOP("Options.h", "Comparators"),
                        QT_TRANSLATE_NOOP("Options.h", "Complex comparators"),
};

const int               REPORT_TYPE_COUNT                       = sizeof(ReportType)/sizeof(char*);

const int               REPORT_TYPE_UNKNOWN                     = -1,
                        REPORT_TYPE_LINEARITY                   = 0,
                        REPORT_TYPE_LINEARITY_CERTIFICATION     = 1,
                        REPORT_TYPE_LINEARITY_DETAIL_ELRCTRIC   = 2,
                        REPORT_TYPE_LINEARITY_DETAIL_PHYSICAL   = 3,
                        REPORT_TYPE_COMPARATOR                  = 4,
                        REPORT_TYPE_COMPLEX_COMPARATOR          = 5;

const char* const       ReportFileName[REPORT_TYPE_COUNT] =
{
                        QT_TRANSLATE_NOOP("Options.h", "Linearity.ncr"),
                        QT_TRANSLATE_NOOP("Options.h", "LinearityCertification.ncr"),
                        QT_TRANSLATE_NOOP("Options.h", "LinearityDetailEl.ncr"),
                        QT_TRANSLATE_NOOP("Options.h", "LinearityDetailPh.ncr"),
                        QT_TRANSLATE_NOOP("Options.h", "Comparators.ncr"),
                        QT_TRANSLATE_NOOP("Options.h", "ComplexComparators.ncr"),
};

// ==============================================================================================

struct REPORT_HEADER
{
    int                 m_type = REPORT_TYPE_UNKNOWN;

    QString             m_documentTitle;
    QString             m_reportTitle;
    QString             m_date;
    QString             m_tableTitle;
    QString             m_conclusion;

    double              m_T = 0;
    double              m_P = 0;
    double              m_H = 0;
    double              m_V = 0;
    double              m_F = 0;

    QString             m_calibrator[MAX_CALIBRATOR_COUNT];

    int                 m_linkObjectID;
    QString             m_reportFile;

    int                 m_param = 0;

    void                init(int type);
};


// ==============================================================================================

class ReportHeaderBase : public QObject
{
    Q_OBJECT

private:

    QMutex              m_mutex;

    QVector<REPORT_HEADER>	m_headerList;

public:
    explicit            ReportHeaderBase(QObject *parent = 0);
    explicit            ReportHeaderBase(const ReportHeaderBase& from, QObject *parent = 0);
                        ~ReportHeaderBase();

    int                 count();
    bool                isEmpty() { return count() == 0; }

    void                clear();

    bool                reportsIsExist();

    void                load();
    void                save();

    ReportHeaderBase&   operator=(const ReportHeaderBase& from);
    REPORT_HEADER&      operator[](int index);
};

// ==============================================================================================

class ReportOption : public QObject
{
    Q_OBJECT

public:
    explicit            ReportOption(QObject *parent = 0);
    explicit            ReportOption(const ReportOption& from, QObject *parent = 0);
                        ~ReportOption();

    QString             m_path;
    int					m_type = REPORT_TYPE_LINEARITY;

    ReportHeaderBase    m_headerBase;

    void                init();

    int                 reportTypeByMeasureType(int measureType);

    void                load();
    void                save();

    ReportOption&       operator=(const ReportOption& from);

};

// ==============================================================================================

const char* const       LinearityPointSensor[] =
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

const int               POINT_SENSOR_COUNT          = sizeof(LinearityPointSensor)/sizeof(char*);

const int               POINT_SENSOR_UNKNOWN        = -1,
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

    explicit            LinearityPoint() { setPercent(0); }
    explicit            LinearityPoint(double percent) { setPercent(percent); }

private:

    int                 m_pointID = -1;

    double              m_percentValue = 0;
    double              m_sensorValue[POINT_SENSOR_COUNT];

public:

    int                 pointID() { return m_pointID; }
    void                setPointID(int id) { m_pointID = id; }

    double              percent() {return m_percentValue; }
    void                setPercent(double value);

    double              sensorValue(int sensor);
};

// ==============================================================================================

class LinearityPointBase : public QObject
{
    Q_OBJECT

public:

    explicit            LinearityPointBase(QObject *parent = 0);
    explicit            LinearityPointBase(const LinearityPointBase& from, QObject *parent = 0);
                        ~LinearityPointBase();

private:

    QMutex              m_mutex;

    QVector<LinearityPoint>	m_pointList;                                  // list of measurement points

public:

    int                 count();
    bool                isEmpty() { return count() == 0; }

    int                 append(LinearityPoint point);
    int                 insert(int index, LinearityPoint point);
    bool                remove(int index);

    void                clear();

    void                swap(int i, int j);

    QString             text();

    void                load();
    void                save();

    LinearityPointBase& operator=(const LinearityPointBase& from);
    LinearityPoint&     operator[](int index);
};


// ==============================================================================================

#define                 LINEARITY_OPTIONS_KEY           "Options/Linearity/"

// ----------------------------------------------------------------------------------------------

const char* const       LinearityParamName[] =
{
                        QT_TRANSLATE_NOOP("Options.h", "Limit error"),
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
                        QT_TRANSLATE_NOOP("Options.h", "Type of measurements list"),
                        QT_TRANSLATE_NOOP("Options.h", "Show column of output values"),
};

const int               LO_PARAM_COUNT				= sizeof(LinearityParamName)/sizeof(char*);

const int               LO_PARAM_ERROR				= 0,
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
                        LO_PARAM_LIST_TYPE          = 11,
                        LO_PARAM_OUTPUT_RANGE		= 12;


// ----------------------------------------------------------------------------------------------

const char* const       LinearityRangeTypeStr[] =
{
                        QT_TRANSLATE_NOOP("Options.h", "Manual division of the measure range"),
                        QT_TRANSLATE_NOOP("Options.h", "Automatic division of the measure range"),
};

const int               LO_RANGE_TYPE_COUNT			= sizeof(LinearityRangeTypeStr)/sizeof(char*);

const int               LO_RANGE_TYPE_MANUAL		= 0,
                        LO_RANGE_TYPE_AUTOMATIC		= 1;

// ----------------------------------------------------------------------------------------------

const char* const       LinearityViewTypeStr[] =
{
                        QT_TRANSLATE_NOOP("Options.h", "Simple"),
                        QT_TRANSLATE_NOOP("Options.h", "Extended (show columns for metrological certification)"),
                        QT_TRANSLATE_NOOP("Options.h", "Detail electric (show all measurements at one point)"),
                        QT_TRANSLATE_NOOP("Options.h", "Detail physical (show all measurements at one point)"),
};

const int               LO_VIEW_TYPE_COUNT              = sizeof(LinearityViewTypeStr)/sizeof(char*);

const int               LO_VIEW_TYPE_UNKNOWN            = -1,
                        LO_VIEW_TYPE_SIMPLE             = 0,
                        LO_VIEW_TYPE_EXTENDED           = 1,
                        LO_VIEW_TYPE_DETAIL_ELRCTRIC    = 2,
                        LO_VIEW_TYPE_DETAIL_PHYSICAL    = 3;

// ----------------------------------------------------------------------------------------------

class LinearityOption : public QObject
{
    Q_OBJECT

public:
    explicit            LinearityOption(QObject *parent = 0);
    explicit            LinearityOption(const LinearityOption& from, QObject *parent = 0);
                        ~LinearityOption();

    LinearityPointBase  m_pointBase;                               // list of measurement points

    double              m_errorValue = 0.5;                           // permissible error is given by specified documents
    double              m_errorCtrl = 0.1;                            // control error is given by metrologists
    int                 m_errorType = ERROR_TYPE_REDUCE;      // type of error absolute or reduced
    bool                m_errorCalcBySCO = false;                     // сalculate error by standard deviation

    int                 m_measureTimeInPoint = 1;                     // time, in seconds, during which will be made ​​N measurements at each point
    int                 m_measureCountInPoint = 20;                   // количество измерений в точке, согласно госту МИ 2002-89 приложение 7

    int                 m_rangeType = LO_RANGE_TYPE_MANUAL;           // type of division measure range: manual - 0 or automatic - 1
    double              m_lowLimitRange = 0;                          // lower limit of the range for automatic division
    double              m_highLimitRange = 100;                       // high limit of the range for automatic division

    int                 m_viewType = LO_VIEW_TYPE_SIMPLE;             // type of measurements list extended or simple
    bool                m_showOutputRangeColumn = false;              // show column output values

    void                recalcPoints(int count = -1);

    void                load();
    void                save();

    LinearityOption&    operator=(const LinearityOption& from);
};

// ==============================================================================================

#define					BACKUP_OPTIONS_REG_KEY		"Options/BackupMeasure/"

// ----------------------------------------------------------------------------------------------

const char* const		BackupParam[] =
{
                        QT_TRANSLATE_NOOP("Options.h", "On start application"),
                        QT_TRANSLATE_NOOP("Options.h", "On exit application"),
                        QT_TRANSLATE_NOOP("Options.h", "Path"),
};

const int				BUO_PARAM_COUNT         = sizeof(BackupParam)/sizeof(char*);

const int				BUO_PARAM_ON_START      = 0,
                        BUO_PARAM_ON_EXIT       = 1,
                        BUO_PARAM_PATH          = 2;

// ----------------------------------------------------------------------------------------------

class BackupOption : public QObject
{
    Q_OBJECT

public:
    explicit            BackupOption(QObject *parent = 0);
    explicit            BackupOption(const BackupOption& from, QObject *parent = 0);
                        ~BackupOption();

    bool				m_onStart = false;
    bool                m_onExit = true;
    QString				m_path;

    bool				createBackup();
    void				createBackupOnStart();
    void				createBackupOnExit();

    void				load();
    void				save();

    BackupOption&       operator=(const BackupOption& from);
};

// ==============================================================================================

class Options : public QObject
{
    Q_OBJECT

public:

    explicit            Options(QObject *parent = 0);
    explicit            Options(const Options& from, QObject *parent = 0);
                        ~Options();

    int                 channelCount();

    bool                m_updateColumnView[MEASURE_TYPE_COUNT];             // determined the need to update the view after changing settings

private:

    QMutex              m_mutex;

    ToolBarOption       m_toolBar;
    TcpIpOption         m_connectTcpIp;
    MeasureViewOption   m_measureView;
    DatabaseOption      m_database;
    ReportOption        m_report;
    LinearityOption     m_linearity;
    BackupOption        m_backup;

public:

    ToolBarOption&      toolBar() { return m_toolBar; }
    void                setToolBar(const ToolBarOption& toolBar) { m_toolBar = toolBar; }

    TcpIpOption&        connectTcpIp() { return m_connectTcpIp; }
    void                setConnectTcpIp(const TcpIpOption& connectTcpIp) { m_connectTcpIp = connectTcpIp; }

    MeasureViewOption&  measureView() { return m_measureView; }
    void                setMeasureView(const MeasureViewOption& measureView) { m_measureView = measureView; }

    DatabaseOption&     database() { return m_database; }
    void                setDatabase(const DatabaseOption& database) { m_database = database; }

    ReportOption&       report() { return m_report; }
    void                setReport(const ReportOption& report) { m_report = report; }

    LinearityOption&    linearity() { return m_linearity; }
    void                setLinearity(const LinearityOption& linearity) { m_linearity = linearity; }

    BackupOption&       backup() { return m_backup; }
    void                etBackup(const BackupOption& backup) { m_backup = backup; }

    void                load();
    void                save();

    inline Options&     operator=(const Options& from);
};

// ==============================================================================================

extern Options          theOptions;

// ==============================================================================================

#endif // OPTIONS_H
