#ifndef OPTIONS_H
#define OPTIONS_H

#include <assert.h>

#include <QObject>
#include <QMutex>
#include <QVector>
#include <QFont>
#include <QColor>

#include "MeasureViewHeader.h"
#include "ObjectVector.h"

// ==============================================================================================

#define					WINDOW_GEOMETRY_OPTIONS_KEY		"Options/Window/"

// ==============================================================================================

#define					PROJECT_INFO_KEY				"Options/ProjectInfo/"

// ----------------------------------------------------------------------------------------------

class ProjectInfo: public QObject
{
	Q_OBJECT

public:

	explicit ProjectInfo(QObject *parent = nullptr);
	explicit ProjectInfo(const ProjectInfo& from, QObject *parent = nullptr);
	virtual ~ProjectInfo();

private:

	QString				m_projectName;
	int					m_id = -1;
	QString				m_release;
	QString				m_date;
	int					m_changeset = 0;
	QString				m_user;
	QString				m_workstation;
	int					m_dbVersion = 0;
	int					m_cfgFileVersion = 0;

public:

	QString				projectName() { return m_projectName; }
	int					id() { return m_id; }
	QString				release() { return m_release; }
	QString				date() { return m_date; }
	int					changeset() { return m_changeset; }
	QString				user() { return m_user; }
	QString				workstation() { return m_workstation; }
	int					dbVersion() { return m_dbVersion; }
	int					cfgFileVersion() { return m_cfgFileVersion; }
	void				setCfgFileVersion(int version) { m_cfgFileVersion = version; }

	void				save();

	bool				readFromXml(const QByteArray& fileData);

	ProjectInfo&		operator=(const ProjectInfo& from);
};

// ==============================================================================================

#define					SOCKET_OPTIONS_KEY				"Options/Socket/"

// ----------------------------------------------------------------------------------------------

const char* const		SocketServerType[] =
{
						QT_TRANSLATE_NOOP("Options.h", "Primary"),
						QT_TRANSLATE_NOOP("Options.h", "Reserve"),
};

const int				SOCKET_SERVER_TYPE_COUNT = sizeof(SocketServerType)/sizeof(SocketServerType[0]);

const int				SOCKET_SERVER_TYPE_PRIMARY = 0,
						SOCKET_SERVER_TYPE_RESERVE = 1;

// ----------------------------------------------------------------------------------------------

const char* const		SocketClientParamNameForAll[] =
{
						QT_TRANSLATE_NOOP("Options.h", "EquipmentID"),
						QT_TRANSLATE_NOOP("Options.h", "IP"),
						QT_TRANSLATE_NOOP("Options.h", "Port"),
						QT_TRANSLATE_NOOP("Options.h", "EquipmentID"),
						QT_TRANSLATE_NOOP("Options.h", "IP"),
						QT_TRANSLATE_NOOP("Options.h", "Port"),
};

const int				SOCKET_CLIENT_PARAM_COUNT			= sizeof(SocketClientParamNameForAll)/sizeof(SocketClientParamNameForAll[0]);

const int				SOCKET_CLIENT_PARAM_EQUIPMENT_ID1	= 0,
						SOCKET_CLIENT_PARAM_SERVER_IP1		= 1,
						SOCKET_CLIENT_PARAM_SERVER_PORT1	= 2,
						SOCKET_CLIENT_PARAM_EQUIPMENT_ID2	= 3,
						SOCKET_CLIENT_PARAM_SERVER_IP2		= 4,
						SOCKET_CLIENT_PARAM_SERVER_PORT2	= 5;

// ----------------------------------------------------------------------------------------------

struct CONNECTION_OPTION
{
	bool				readFromCfgSrv = false;

	QString				equipmentID;
	QString				serverIP;
	int					serverPort = 0;
};

// ----------------------------------------------------------------------------------------------

class SocketClientOption
{
public:

	SocketClientOption();
	virtual ~SocketClientOption() {}

private:

	int					m_type = -1;

	CONNECTION_OPTION	m_connectOption[SOCKET_SERVER_TYPE_COUNT];

public:

	int					socketType() const { return m_type; }
	void				setSocketType(int socketType) { m_type = socketType; }

	QString				equipmentID(int serverType) const;
	void				setEquipmentID(int serverType, const QString& equipmentID);

	QString				serverIP(int serverType) const;
	void				setServerIP(int serverType, const QString& ip);

	int					serverPort(int serverType) const;
	void				setServerPort(int serverType, int port);

	HostAddressPort		address(int serverType) const;

	void				load();
	void				save();

	bool				readOptionsFromXml(const QByteArray& fileData);
};

// ==============================================================================================

const char* const		SocketType[] =
{
						QT_TRANSLATE_NOOP("Options.h", "ConfigSocket"),
						QT_TRANSLATE_NOOP("Options.h", "SignalSocket"),
						QT_TRANSLATE_NOOP("Options.h", "TuningSocket"),
};

const int				SOCKET_TYPE_COUNT = sizeof(SocketType)/sizeof(SocketType[0]);

const int				SOCKET_TYPE_CONFIG = 0,
						SOCKET_TYPE_SIGNAL = 1,
						SOCKET_TYPE_TUNING = 2;

const char* const		SocketDefaultID[SOCKET_TYPE_COUNT] =
{
						QT_TRANSLATE_NOOP("Options.h", "_METROLOGY"),	// for ConfigSocket
						QT_TRANSLATE_NOOP("Options.h", "_ADS"),			// for SignalSocket
						QT_TRANSLATE_NOOP("Options.h", "_METROLOGY"),	// for TuningSocket
};

const int				SocketDefaultPort[SOCKET_TYPE_COUNT] =
{
						PORT_CONFIGURATION_SERVICE_CLIENT_REQUEST,		// ConfigSocket
						PORT_APP_DATA_SERVICE_CLIENT_REQUEST,			// SignalSocket
						PORT_TUNING_SERVICE_CLIENT_REQUEST,				// TuningSocket
};

const char* const		SocketClientParamName[SOCKET_TYPE_COUNT][SOCKET_CLIENT_PARAM_COUNT] =
{
					{
						QT_TRANSLATE_NOOP("Options.h", "EquipmentID of software \"Metrology\""),
						QT_TRANSLATE_NOOP("Options.h", "Configuration Service IP"),
						QT_TRANSLATE_NOOP("Options.h", "Configuration Service Port"),
						QT_TRANSLATE_NOOP("Options.h", "EquipmentID of software \"Metrology\""),
						QT_TRANSLATE_NOOP("Options.h", "Configuration Service IP"),
						QT_TRANSLATE_NOOP("Options.h", "Configuration Service Port"),
					},
					{
						QT_TRANSLATE_NOOP("Options.h", "EquipmentID of software \"Application Data Service\""),
						QT_TRANSLATE_NOOP("Options.h", "Application Data Service IP"),
						QT_TRANSLATE_NOOP("Options.h", "Application Data Service Port"),
						QT_TRANSLATE_NOOP("Options.h", "EquipmentID of software \"Application Data Service\""),
						QT_TRANSLATE_NOOP("Options.h", "Application Data Service IP"),
						QT_TRANSLATE_NOOP("Options.h", "Application Data Service Port"),
					},
					{
						QT_TRANSLATE_NOOP("Options.h", "EquipmentID of software \"Metrology\""),
						QT_TRANSLATE_NOOP("Options.h", "Tuning Service IP"),
						QT_TRANSLATE_NOOP("Options.h", "Tuning Service Port"),
						QT_TRANSLATE_NOOP("Options.h", "EquipmentID of software \"Metrology\""),
						QT_TRANSLATE_NOOP("Options.h", "Tuning Service IP"),
						QT_TRANSLATE_NOOP("Options.h", "Tuning Service Port"),
					},
};

// ----------------------------------------------------------------------------------------------

class SocketOption : public QObject
{
	Q_OBJECT

public:

	explicit SocketOption(QObject *parent = nullptr);
	explicit SocketOption(const SocketOption& from, QObject *parent = nullptr);
	virtual ~SocketOption();

private:

	SocketClientOption	m_client[SOCKET_TYPE_COUNT];

public:

	SocketClientOption	client(int socketType) const;
	void				setClient(int socketType, const SocketClientOption& client);

	void				load();
	void				save();

	SocketOption&		operator=(const SocketOption& from);
};


// ==============================================================================================

#define					TOOLBAR_OPTIONS_KEY			 "Options/ToolBar/"

// ----------------------------------------------------------------------------------------------

class ToolBarOption : public QObject
{
	Q_OBJECT

public:

	explicit ToolBarOption(QObject *parent = nullptr);
	explicit ToolBarOption(const ToolBarOption& from, QObject *parent = nullptr);
	virtual ~ToolBarOption();

private:

	int					m_measureTimeout = 0;									// in milliseconds, timeout between the time when the calibrator is set value and the time when the application is save measurement
	int					m_measureKind = MEASURE_KIND_ONE;						// measure kind: each channel separately - 0 or for all channels together - 1
	int					m_outputSignalType = OUTPUT_SIGNAL_TYPE_UNUSED;			// selected type of output signal

public:

	int					measureTimeout() const { return m_measureTimeout; }
	void				setMeasureTimeout(int timeout) { m_measureTimeout = timeout; }

	int					measureKind() const { return m_measureKind; }
	void				setMeasureKind(int kind) { m_measureKind = kind; }

	int					outputSignalType() const { return m_outputSignalType; }
	void				setOutputSignalType(int type) { m_outputSignalType = type; }


	void				load();
	void				save();

	ToolBarOption&		operator=(const ToolBarOption& from);
};

// ==============================================================================================

#define					MEASURE_VIEW_OPTIONS_KEY		"Options/MeasureView/"

// ----------------------------------------------------------------------------------------------

const char* const		MeasureViewParam[] =
{
						QT_TRANSLATE_NOOP("Options.h", "Font of measurements list"),
						QT_TRANSLATE_NOOP("Options.h", "Color measurement that has not error"),
						QT_TRANSLATE_NOOP("Options.h", "Color measurement over limit error"),
						QT_TRANSLATE_NOOP("Options.h", "Color measurement over control error"),
};

const int				MWO_PARAM_COUNT					= sizeof(MeasureViewParam)/sizeof(MeasureViewParam[0]);

const int				MWO_PARAM_FONT					= 0,
						MWO_PARAM_COLOR_NOT_ERROR		= 1,
						MWO_PARAM_COLOR_LIMIT_ERROR		= 2,
						MWO_PARAM_COLOR_CONTROL_ERROR	= 3;

// ----------------------------------------------------------------------------------------------

const char* const		TypeSignalID[] =
{
						QT_TRANSLATE_NOOP("Options.h", "AppSignalID"),
						QT_TRANSLATE_NOOP("Options.h", "CustomAppSignalID"),
						QT_TRANSLATE_NOOP("Options.h", "EquipmentID"),
};

const int				SIGNAL_ID_TYPE_COUNT		= sizeof(TypeSignalID)/sizeof(TypeSignalID[0]);

const int				SIGNAL_ID_TYPE_APP			= 0,
						SIGNAL_ID_TYPE_CUSTOM		= 1,
						SIGNAL_ID_TYPE_EQUIPMENT	= 2;

// ----------------------------------------------------------------------------------------------

#define					COLOR_NOT_ERROR				QColor(0xA0, 0xFF, 0xA0)
#define					COLOR_OVER_LIMIT_ERROR		QColor(0xFF, 0xA0, 0xA0)
#define					COLOR_OVER_CONTROL_ERROR	QColor(0xFF, 0xD0, 0xA0)

// ----------------------------------------------------------------------------------------------

class MeasureViewOption : public QObject
{
	Q_OBJECT

public:
	explicit MeasureViewOption(QObject *parent = nullptr);
	explicit MeasureViewOption(const MeasureViewOption& from, QObject *parent = nullptr);
	virtual ~MeasureViewOption();

//private:
public:

	MeasureViewColumn	m_column[MEASURE_TYPE_COUNT][MEASURE_VIEW_COLUMN_COUNT];

private:

	int					m_measureType = MEASURE_TYPE_UNKNOWN;						// current, active ViewID

	QFont				m_font;
	QFont				m_fontBold;

	QColor				m_colorNotError = COLOR_NOT_ERROR;
	QColor				m_colorErrorLimit = COLOR_OVER_LIMIT_ERROR;
	QColor				m_colorErrorControl = COLOR_OVER_CONTROL_ERROR;

public:

	int					measureType() const { return m_measureType; }
	void				setMeasureType(int measureType) { m_measureType = measureType; }

	QFont&				font() { return m_font; }
	void				setFont(QFont font) { m_font = font; }

	QFont				fontBold() const { return m_fontBold; }
	void				setFontBold(QFont font) { m_fontBold = font; }

	QColor				colorNotError() const { return m_colorNotError; }
	void				setColorNotError(QColor color) { m_colorNotError = color; }

	QColor				colorErrorLimit() const { return m_colorErrorLimit; }
	void				setColorErrorLimit(QColor color) { m_colorErrorLimit = color; }

	QColor				colorErrorControl() const { return m_colorErrorControl; }
	void				setColorErrorControl(QColor color) { m_colorErrorControl = color; }

	void				init();

	void				load();
	void				save();

	MeasureViewOption&	operator=(const MeasureViewOption& from);
};

// ==============================================================================================

#define					SIGNAL_INFO_OPTIONS_KEY		"Options/SignalInfo/"

// ----------------------------------------------------------------------------------------------

const char* const		SignalInfoParam[] =
{
						QT_TRANSLATE_NOOP("Options.h", "Font of signal information list"),
						QT_TRANSLATE_NOOP("Options.h", "Show electric state"),
						QT_TRANSLATE_NOOP("Options.h", "Show ADC state"),
						QT_TRANSLATE_NOOP("Options.h", "Show ADC (hex) state"),
						QT_TRANSLATE_NOOP("Options.h", "Color flag no validity"),
						QT_TRANSLATE_NOOP("Options.h", "Color flag overflow"),
						QT_TRANSLATE_NOOP("Options.h", "Color flag underflow"),
};

const int				SIO_PARAM_COUNT					= sizeof(SignalInfoParam)/sizeof(SignalInfoParam[0]);

const int				SIO_PARAM_FONT					= 0,
						SIO_PARAM_ELECTRIC_STATE		= 1,
						SIO_PARAM_ADC_STATE				= 2,
						SIO_PARAM_ADC_HEX_STATE			= 3,
						SIO_PARAM_COLOR_FLAG_VALID		= 4,
						SIO_PARAM_COLOR_FLAG_OVERFLOW	= 5,
						SIO_PARAM_COLOR_FLAG_UNDERFLOW	= 6;

// ----------------------------------------------------------------------------------------------

#define					COLOR_FLAG_VALID		QColor(0xFF, 0xA0, 0xA0)
#define					COLOR_FLAG_OVERFLOW		QColor(0xFF, 0xD0, 0xA0)
#define					COLOR_FLAG_OVERBREAK	QColor(0xFF, 0xD0, 0xA0)

// ----------------------------------------------------------------------------------------------

class SignalInfoOption : public QObject
{
	Q_OBJECT

public:

	explicit SignalInfoOption(QObject *parent = nullptr);
	explicit SignalInfoOption(const SignalInfoOption& from, QObject *parent = nullptr);
	virtual ~SignalInfoOption();

private:

	QFont				m_font;

	bool				m_showElectricState = false;

	QColor				m_colorFlagValid = COLOR_FLAG_VALID;
	QColor				m_colorFlagOverflow = COLOR_FLAG_OVERFLOW;
	QColor				m_colorFlagUnderflow = COLOR_FLAG_OVERBREAK;

public:

	QFont&				font() { return m_font; }
	void				setFont(QFont font) { m_font = font; }

	bool				showElectricState() const { return m_showElectricState; }
	void				setShowElectricState(bool show) { m_showElectricState = show; }

	QColor				colorFlagValid() const { return m_colorFlagValid; }
	void				setColorFlagValid(QColor color) { m_colorFlagValid = color; }

	QColor				colorFlagOverflow() const { return m_colorFlagOverflow; }
	void				setColorFlagOverflow(QColor color) { m_colorFlagOverflow = color; }

	QColor				colorFlagUnderflow() const { return m_colorFlagUnderflow; }
	void				setColorFlagUnderflow(QColor color) { m_colorFlagUnderflow = color; }

	void				load();
	void				save();

	SignalInfoOption&	operator=(const SignalInfoOption& from);
};


// ==============================================================================================

#define					DATABASE_OPTIONS_REG_KEY		"Options/Database/"

// ----------------------------------------------------------------------------------------------

const char* const		DatabaseParam[] =
{
						QT_TRANSLATE_NOOP("Options.h", "Path"),
						QT_TRANSLATE_NOOP("Options.h", "Type"),
};

const int				DBO_PARAM_COUNT	= sizeof(DatabaseParam)/sizeof(DatabaseParam[0]);

const int				DBO_PARAM_PATH	= 0,
						DBO_PARAM_TYPE	= 1;

// ----------------------------------------------------------------------------------------------

const char* const		DatabaseType[] =
{
						QT_TRANSLATE_NOOP("Options.h", "SQLite"),
};

const int				DATABASE_TYPE_COUNT		= sizeof(DatabaseType)/sizeof(DatabaseType[0]);

const int				DATABASE_TYPE_SQLITE	= 0;


// ----------------------------------------------------------------------------------------------

class DatabaseOption : public QObject
{
	Q_OBJECT

public:

	explicit DatabaseOption(QObject *parent = nullptr);
	explicit DatabaseOption(const DatabaseOption& from, QObject *parent = nullptr);
	virtual ~DatabaseOption();

private:

	QString				m_path;
	int					m_type;

public:

	QString				path() const { return m_path; }
	void				setPath(const QString& path) { m_path = path; }

	int					type() const { return m_type; }
	void				setType(int type) { m_type = type; }


	bool				create();
	void				remove();

	void				load();
	void				save();

	DatabaseOption&		operator=(const DatabaseOption& from);
};

// ==============================================================================================

#define					MODULE_OPTIONS_KEY			"Options/Module/"

// ----------------------------------------------------------------------------------------------

const char* const		ModuleParamName[] =
{
						QT_TRANSLATE_NOOP("Options.h", "Measure all signals of module in series"),
						QT_TRANSLATE_NOOP("Options.h", "Show warning if signal is already measured"),
						QT_TRANSLATE_NOOP("Options.h", "Suffix to identify signal of module serial number"),
};

const int				MO_PARAM_COUNT					= sizeof(ModuleParamName)/sizeof(ModuleParamName[0]);

const int				MO_PARAM_MEASURE_ENTIRE_MODULE	= 0,
						MO_PARAM_WARN_IF_MEASURED		= 1,
						MO_PARAM_SUFFIX_SN				= 2;

// ----------------------------------------------------------------------------------------------

class ModuleOption : public QObject
{
	Q_OBJECT

public:

	explicit ModuleOption(QObject *parent = nullptr);
	explicit ModuleOption(const ModuleOption& from, QObject *parent = nullptr);
	virtual ~ModuleOption();

private:

	bool				m_measureEntireModule = false;								// measure all inputs of module in series
	bool				m_warningIfMeasured = true;									// show warning if signal is already measured
	QString				m_suffixSN;													// suffix to identify the signal of module serial number

public:

	bool				measureEntireModule() const { return m_measureEntireModule; }
	void				setMeasureEntireModule(bool measure) { m_measureEntireModule = measure; }

	bool				warningIfMeasured() const { return m_warningIfMeasured; }
	void				setWarningIfMeasured(bool enable) { m_warningIfMeasured = enable; }

	QString				suffixSN() const { return m_suffixSN; }
	void				setSuffixSN(const QString& suffixSN) { m_suffixSN = suffixSN; }

public:

	void				load();
	void				save();

	ModuleOption&		operator=(const ModuleOption& from);
};

// ==============================================================================================

const char* const		LinearityPointSensor[] =
{
						QT_TRANSLATE_NOOP("Options.h", "%"),
						QT_TRANSLATE_NOOP("Options.h", "0 .. 5 V"),
						QT_TRANSLATE_NOOP("Options.h", "-10 .. 10 V"),
						QT_TRANSLATE_NOOP("Options.h", "0 - 5 mA"),
						QT_TRANSLATE_NOOP("Options.h", "4 - 20 mA"),
						QT_TRANSLATE_NOOP("Options.h", "0 - 100 °C"),
						QT_TRANSLATE_NOOP("Options.h", "0 - 150 °C"),
						QT_TRANSLATE_NOOP("Options.h", "0 - 200 °C"),
						QT_TRANSLATE_NOOP("Options.h", "0 - 400 °C"),
};

const int				POINT_SENSOR_COUNT			= sizeof(LinearityPointSensor)/sizeof(LinearityPointSensor[0]);

const int				POINT_SENSOR_UNKNOWN		= -1,
						POINT_SENSOR_PERCENT		= 0,
						POINT_SENSOR_U_0_5_V		= 1,
						POINT_SENSOR_U_m10_10_V		= 2,
						POINT_SENSOR_I_0_5_MA		= 3,
						POINT_SENSOR_I_4_20_MA		= 4,
						POINT_SENSOR_T_0_100_C		= 5,
						POINT_SENSOR_T_0_150_C		= 6,
						POINT_SENSOR_T_0_200_C		= 7,
						POINT_SENSOR_T_0_400_C		= 8;

// ----------------------------------------------------------------------------------------------

class LinearityPoint
{
public:

	LinearityPoint() { setPercent(0); }
	explicit LinearityPoint(double percent) { setPercent(percent); }
	virtual ~LinearityPoint() {}

private:

	int					m_index = -1;

	double				m_percentValue = 0;
	double				m_sensorValue[POINT_SENSOR_COUNT];

public:

	int					Index() const { return m_index; }
	void				setIndex(int index) { m_index = index; }

	double				percent() const {return m_percentValue; }
	void				setPercent(double value);

	double				sensorValue(int sensor);
};

// ==============================================================================================

class LinearityPointBase : public ObjectVector<LinearityPoint>
{

public:

	LinearityPointBase();
	virtual ~LinearityPointBase() { clear(); }

	QString				text();

	virtual void		initEmptyData(QVector<LinearityPoint> &data);
};

// ==============================================================================================

#define					LINEARITY_OPTIONS_KEY			"Options/Linearity/"

// ----------------------------------------------------------------------------------------------

const char* const		LinearityParamName[] =
{
						QT_TRANSLATE_NOOP("Options.h", "Limit of error"),
						QT_TRANSLATE_NOOP("Options.h", "Type of error"),
						QT_TRANSLATE_NOOP("Options.h", "Show error from limit"),
						QT_TRANSLATE_NOOP("Options.h", "Measure time in a point, (sec)"),
						QT_TRANSLATE_NOOP("Options.h", "Count of measurements in a point"),
						QT_TRANSLATE_NOOP("Options.h", "Division of the measure range"),
						QT_TRANSLATE_NOOP("Options.h", "Count of points"),
						QT_TRANSLATE_NOOP("Options.h", "Lower limit of the measure range, (%)"),
						QT_TRANSLATE_NOOP("Options.h", "High limit of the measure range, (%)"),
						QT_TRANSLATE_NOOP("Options.h", "Points of range"),
						QT_TRANSLATE_NOOP("Options.h", "Type of measurements list"),
						QT_TRANSLATE_NOOP("Options.h", "Show columns of engineering values"),
};

const int				LO_PARAM_COUNT					= sizeof(LinearityParamName)/sizeof(LinearityParamName[0]);

const int				LO_PARAM_ERROR_LIMIT			= 0,
						LO_PARAM_ERROR_TYPE				= 1,
						LO_PARAM_SHOW_ERROR_FROM_LIMIT	= 2,
						LO_PARAM_MEASURE_TIME			= 3,
						LO_PARAM_MEASURE_IN_POINT		= 4,
						LO_PARAM_RANGE_TYPE				= 5,
						LO_PARAM_POINT_COUNT			= 6,
						LO_PARAM_LOW_RANGE				= 7,
						LO_PARAM_HIGH_RANGE				= 8,
						LO_PARAM_VALUE_POINTS			= 9,
						LO_PARAM_LIST_TYPE				= 10,
						LO_PARAM_SHOW_ENGINEERING_VALUE	= 11;

// ----------------------------------------------------------------------------------------------

const char* const		LinearityRangeTypeStr[] =
{
						QT_TRANSLATE_NOOP("Options.h", "Manual division of the measure range"),
						QT_TRANSLATE_NOOP("Options.h", "Automatic division of the measure range"),
};

const int				LO_RANGE_TYPE_COUNT			= sizeof(LinearityRangeTypeStr)/sizeof(LinearityRangeTypeStr[0]);

const int				LO_RANGE_TYPE_MANUAL		= 0,
						LO_RANGE_TYPE_AUTOMATIC		= 1;

// ----------------------------------------------------------------------------------------------

const char* const		LinearityViewTypeStr[] =
{
						QT_TRANSLATE_NOOP("Options.h", "Simple"),
						QT_TRANSLATE_NOOP("Options.h", "Extended (show columns for metrological certification)"),
						QT_TRANSLATE_NOOP("Options.h", "Detail electric (show all measurements at one point)"),
						QT_TRANSLATE_NOOP("Options.h", "Detail engineering (show all measurements at one point)"),
};

const int				LO_VIEW_TYPE_COUNT				= sizeof(LinearityViewTypeStr)/sizeof(LinearityViewTypeStr[0]);

const int				LO_VIEW_TYPE_UNKNOWN			= -1,
						LO_VIEW_TYPE_SIMPLE				= 0,
						LO_VIEW_TYPE_EXTENDED			= 1,
						LO_VIEW_TYPE_DETAIL_ELRCTRIC	= 2,
						LO_VIEW_TYPE_DETAIL_ENGINEERING	= 3;

// ----------------------------------------------------------------------------------------------

class LinearityOption : public QObject
{
	Q_OBJECT

public:

	explicit LinearityOption(QObject *parent = nullptr);
	explicit LinearityOption(const LinearityOption& from, QObject *parent = nullptr);
	virtual ~LinearityOption();

private:

	LinearityPointBase	m_pointBase;												// list of measurement points

	double				m_errorLimit = 0.2;											// permissible error is given by specified documents
	int					m_errorType = MEASURE_ERROR_TYPE_REDUCE;					// type of error absolute or reduced
	int					m_showErrorFromLimit = MEASURE_LIMIT_TYPE_ELECTRIC;			// type of displaing error denend on limit

	int					m_measureTimeInPoint = 1;									// time, in seconds, during which will be made ​​N measurements at each point
	int					m_measureCountInPoint = 20;									// count of measurements in a point, according to GOST MI-2002 application 7

	int					m_rangeType = LO_RANGE_TYPE_MANUAL;							// type of division measure range: manual - 0 or automatic - 1
	double				m_lowLimitRange = 0;										// lower limit of the range for automatic division
	double				m_highLimitRange = 100;										// high limit of the range for automatic division

	int					m_viewType = LO_VIEW_TYPE_SIMPLE;							// type of measurements list extended or simple
	bool				m_showEngeneeringValueColumn = true;						// show columns of engeneering values

public:

	LinearityPointBase& points() { return m_pointBase; }

	double				errorLimit() const { return m_errorLimit; }
	void				setErrorLimit(double errorLimit) { m_errorLimit = errorLimit; }

	int					errorType() const { return m_errorType; }
	void				setErrorType(int type) { m_errorType = type; }

	int					showErrorFromLimit() const { return m_showErrorFromLimit; }
	void				setShowErrorFromLimit(int type) { m_showErrorFromLimit = type; }

	int					measureTimeInPoint() const { return m_measureTimeInPoint; }
	void				setMeasureTimeInPoint(int sec) { m_measureTimeInPoint = sec; }

	int					measureCountInPoint();
	void				setMeasureCountInPoint(int measureCount);

	int					rangeType() const { return m_rangeType; }
	void				setRangeType(int type) { m_rangeType = type; }

	double				lowLimitRange() const { return m_lowLimitRange; }
	void				setLowLimitRange(double lowLimit) { m_lowLimitRange = lowLimit; }

	double				highLimitRange() const { return m_highLimitRange; }
	void				setHighLimitRange(double highLimit) { m_highLimitRange = highLimit; }

	int					viewType() const { return m_viewType; }
	void				setViewType(int type) { m_viewType = type; }

	bool				showEngeneeringValueColumn() const { return m_showEngeneeringValueColumn; }
	void				setShowPhyscalValueColumn(bool show) { m_showEngeneeringValueColumn = show; }

public:

	void				recalcPoints(int count = -1);

	void				load();
	void				save();

	LinearityOption&	operator=(const LinearityOption& from);
};

// ==============================================================================================

#define					COMPARATOR_OPTIONS_KEY			"Options/Comparator/"

// ----------------------------------------------------------------------------------------------

const char* const		ComparatorParamName[] =
{
						QT_TRANSLATE_NOOP("Options.h", "Limit of error"),
						QT_TRANSLATE_NOOP("Options.h", "Start value"),
						QT_TRANSLATE_NOOP("Options.h", "Error type"),
						QT_TRANSLATE_NOOP("Options.h", "Enable measure hysteresis"),
						QT_TRANSLATE_NOOP("Options.h", "Start measurement from the сomparator"),
						QT_TRANSLATE_NOOP("Options.h", "Additional check on the switch сomparator"),
};

const int				CO_PARAM_COUNT				= sizeof(ComparatorParamName)/sizeof(ComparatorParamName[0]);

const int				CO_PARAM_ERROR_LIMIT		= 0,
						CO_PARAM_START_VALUE		= 1,
						CO_PARAM_ERROR_TYPE			= 2,
						CO_PARAM_ENABLE_HYSTERESIS	= 3,
						CO_PARAM_COMPARATOR_INDEX	= 4,
						CO_PARAM_ADDITIONAL_CHECK	= 5;

// ----------------------------------------------------------------------------------------------

class ComparatorOption : public QObject
{
	Q_OBJECT

public:

	explicit ComparatorOption(QObject *parent = nullptr);
	explicit ComparatorOption(const ComparatorOption& from, QObject *parent = nullptr);
	virtual ~ComparatorOption();

//private:
public:

	double				m_errorValue = 0.2;								// permissible error is given by specified documents
	double				m_startValue = 0.1;								// start value is given by metrologists
	int					m_errorType = MEASURE_ERROR_TYPE_REDUCE;		// type of error absolute or reduced

	bool				m_enableMeasureHysteresis = false;				// enable flag to measure hysteresis of сomparator
	int					m_startComparatorIndex = 0;						// start the measurement with the сomparators under the number ...
	bool				m_additionalCheck = true;						// additional check on the stitch сomparator

public:

	void				load();
	void				save();

	ComparatorOption&	operator=(const ComparatorOption& from);
};

// ==============================================================================================

#define					BACKUP_OPTIONS_REG_KEY			"Options/BackupMeasure/"

// ----------------------------------------------------------------------------------------------

const char* const		BackupParam[] =
{
						QT_TRANSLATE_NOOP("Options.h", "On start application"),
						QT_TRANSLATE_NOOP("Options.h", "On exit application"),
						QT_TRANSLATE_NOOP("Options.h", "Path"),
};

const int				BUO_PARAM_COUNT		= sizeof(BackupParam)/sizeof(BackupParam[0]);

const int				BUO_PARAM_ON_START	= 0,
						BUO_PARAM_ON_EXIT	= 1,
						BUO_PARAM_PATH		= 2;

// ----------------------------------------------------------------------------------------------

class BackupOption : public QObject
{
	Q_OBJECT

public:

	explicit BackupOption(QObject *parent = nullptr);
	explicit BackupOption(const BackupOption& from, QObject *parent = nullptr);
	virtual ~BackupOption();

private:

	bool				m_onStart = false;
	bool				m_onExit = true;
	QString				m_path;

public:

	bool				onStart () const { return m_onStart; }
	void				setOnStart(bool onStart) { m_onStart = onStart; }

	bool				onExit() const { return m_onExit; }
	void				setOnExit(bool onExit) { m_onExit = onExit; }

	QString				path() const { return m_path; }
	void				setPath(const QString& path) { m_path = path; }


	bool				createBackup();
	void				createBackupOnStart();
	void				createBackupOnExit();

	void				load();
	void				save();

	BackupOption&		operator=(const BackupOption& from);
};

// ==============================================================================================

class Options : public QObject
{
	Q_OBJECT

public:

	explicit Options(QObject *parent = nullptr);
	explicit Options(const Options& from, QObject *parent = nullptr);
	virtual ~Options();

public:

	int					channelCount();

	bool				m_updateColumnView[MEASURE_TYPE_COUNT];			 // determined the need to update the view after changing settings

private:

	QMutex				m_mutex;

	ProjectInfo			m_projectInfo;
	ToolBarOption		m_toolBar;
	SocketOption		m_socket;
	MeasureViewOption	m_measureView;
	SignalInfoOption	m_signalInfo;
	DatabaseOption		m_database;
	ModuleOption		m_module;
	LinearityOption		m_linearity;
	ComparatorOption	m_comparator;
	BackupOption		m_backup;

public:

	ProjectInfo&		projectInfo() { return m_projectInfo; }
	void				setProjectInfo(const ProjectInfo& projectInfo) { m_projectInfo = projectInfo; }

	ToolBarOption&		toolBar() { return m_toolBar; }
	void				setToolBar(const ToolBarOption& toolBar) { m_toolBar = toolBar; }

	SocketOption&		socket() { return m_socket; }
	void				setSocket(const SocketOption& socket) { m_socket = socket; }

	MeasureViewOption&	measureView() { return m_measureView; }
	void				setMeasureView(const MeasureViewOption& measureView) { m_measureView = measureView; }

	SignalInfoOption&	signalInfo() { return m_signalInfo; }
	void				setSignalInfo(const SignalInfoOption& signalInfo) { m_signalInfo = signalInfo; }

	DatabaseOption&		database() { return m_database; }
	void				setDatabase(const DatabaseOption& database) { m_database = database; }

	ModuleOption&		module() { return m_module; }
	void				setModule(const ModuleOption& module) { m_module = module; }

	LinearityOption&	linearity() { return m_linearity; }
	void				setLinearity(const LinearityOption& linearity) { m_linearity = linearity; }

	ComparatorOption&	comparator() { return m_comparator; }
	void				setComparator(const ComparatorOption& comparator) { m_comparator = comparator; }

	BackupOption&		backup() { return m_backup; }
	void				etBackup(const BackupOption& backup) { m_backup = backup; }

	void				load();
	void				save();
	void				unload();

	bool				readFromXml(const QByteArray& fileData);

	Options&			operator=(const Options& from);
};

// ==============================================================================================

extern Options			theOptions;

// ==============================================================================================

#endif // OPTIONS_H
