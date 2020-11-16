#ifndef OPTIONS_H
#define OPTIONS_H

#include <assert.h>

#include <QObject>
#include <QMutex>
#include <QVector>
#include <QFont>
#include <QColor>

#include "../lib/SocketIO.h"
#include "../lib/MetrologySignal.h"

#include "MeasureViewHeader.h"
#include "MeasureBase.h"
#include "MeasurePointBase.h"

// ==============================================================================================

#define					WINDOW_GEOMETRY_OPTIONS_KEY		"Options/Window/"

// ==============================================================================================

#define					CALIBRATOR_OPTIONS_KEY			"Options/Calibrators/"

// ----------------------------------------------------------------------------------------------

class CalibratorOption
{

public:

	CalibratorOption();
	CalibratorOption(const QString& port, int type);
	virtual ~CalibratorOption() {}

private:

	QString				m_port;
	int					m_type = CALIBRATOR_TYPE_CALYS75;

public:

	bool				isValid() const;

	QString				port() const { return m_port; }
	void				setPort(const QString& port) { m_port = port; }

	int					type() const { return m_type; }
	void				setType(int type) { m_type = type; }
};

// ----------------------------------------------------------------------------------------------

class CalibratorsOption : public QObject
{
	Q_OBJECT

public:

	explicit CalibratorsOption(QObject *parent = nullptr);
	explicit CalibratorsOption(const CalibratorsOption& from, QObject *parent = nullptr);
	virtual ~CalibratorsOption();

private:

	CalibratorOption	m_calibrator[Metrology::ChannelCount];

public:

	CalibratorOption	calibrator(int channel) const;
	void				setCalibrator(int channel, const CalibratorOption& calibrator);

	void				load();
	void				save();

	CalibratorsOption&	operator=(const CalibratorsOption& from);
};


// ==============================================================================================

#define					SOCKET_OPTIONS_KEY				"Options/SocketConnection/"

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
	QString				m_date;
	int					m_changeset = 0;
	QString				m_user;
	QString				m_workstation;
	int					m_dbVersion = 0;
	int					m_cfgFileVersion = 0;

public:

	QString				projectName() { return m_projectName; }
	int					id() { return m_id; }
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

#define					MODULE_OPTIONS_KEY			"Options/Module/"

// ----------------------------------------------------------------------------------------------

const char* const		ModuleParamName[] =
{
						QT_TRANSLATE_NOOP("Options.h", "Suffix to identify signal of module serial number"),
						QT_TRANSLATE_NOOP("Options.h", "Measure all signals of module in series"),
						QT_TRANSLATE_NOOP("Options.h", "Show warning if signal is already measured"),
						QT_TRANSLATE_NOOP("Options.h", "Maximum number of inputs for input mofule"),
};

const int				MO_PARAM_COUNT					= sizeof(ModuleParamName)/sizeof(ModuleParamName[0]);

const int				MO_PARAM_SUFFIX_SN				= 0,
						MO_PARAM_MEASURE_ENTIRE_MODULE	= 1,
						MO_PARAM_WARN_IF_MEASURED		= 2,
						MO_PARAM_MAX_IMPUT_COUNT		= 3;

// ----------------------------------------------------------------------------------------------

class ModuleOption : public QObject
{
	Q_OBJECT

public:

	explicit ModuleOption(QObject *parent = nullptr);
	explicit ModuleOption(const ModuleOption& from, QObject *parent = nullptr);
	virtual ~ModuleOption();

private:

	QString				m_suffixSN;													// suffix to identify the signal of module serial number

	bool				m_measureEntireModule = false;								// measure all inputs of module in series
	bool				m_warningIfMeasured = true;									// show warning if signal is already measured

	int					m_maxInputCount = Metrology::InputCount;					// maximum number of inputs for input mofule

public:

	QString				suffixSN() const { return m_suffixSN; }
	void				setSuffixSN(const QString& suffixSN) { m_suffixSN = suffixSN; }

	bool				measureEntireModule() const { return m_measureEntireModule; }
	void				setMeasureEntireModule(bool measure) { m_measureEntireModule = measure; }

	bool				warningIfMeasured() const { return m_warningIfMeasured; }
	void				setWarningIfMeasured(bool enable) { m_warningIfMeasured = enable; }

	int					maxInputCount() const { return m_maxInputCount; }
	void				setMaxInputCount(int count);

public:

	void				load();
	void				save();

	ModuleOption&		operator=(const ModuleOption& from);
};

// ==============================================================================================

#define					LINEARITY_OPTIONS_KEY			"Options/Linearity/"

// ----------------------------------------------------------------------------------------------

const char* const		LinearityParamName[] =
{
						QT_TRANSLATE_NOOP("Options.h", "Limit of error (%)"),
						QT_TRANSLATE_NOOP("Options.h", "Type of error"),
						QT_TRANSLATE_NOOP("Options.h", "Show error from limit"),
						QT_TRANSLATE_NOOP("Options.h", "Measure time in a point (sec)"),
						QT_TRANSLATE_NOOP("Options.h", "Count of measurements in a point"),
						QT_TRANSLATE_NOOP("Options.h", "Division of the measure range"),
						QT_TRANSLATE_NOOP("Options.h", "Count of points"),
						QT_TRANSLATE_NOOP("Options.h", "Lower limit of the measure range (%)"),
						QT_TRANSLATE_NOOP("Options.h", "High limit of the measure range (%)"),
						QT_TRANSLATE_NOOP("Options.h", "Points of range"),
						QT_TRANSLATE_NOOP("Options.h", "Type of measurements list"),
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
						LO_PARAM_LIST_TYPE				= 10;

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

const int				LO_VIEW_TYPE_UNDEFINED			= -1,
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

	MeasurePointBase	m_pointBase;												// list of measurement points

	double				m_errorLimit = 0.2;											// permissible error is given by specified documents
	int					m_errorType = MEASURE_ERROR_TYPE_REDUCE;					// type of error absolute or reduced
	int					m_limitType = MEASURE_LIMIT_TYPE_ELECTRIC;					// type of displaing error denend on limit

	int					m_measureTimeInPoint = 1;									// time, in seconds, during which will be made ​​N measurements at each point
	int					m_measureCountInPoint = 20;									// count of measurements in a point, according to GOST MI-2002 application 7

	int					m_rangeType = LO_RANGE_TYPE_MANUAL;							// type of division measure range: manual - 0 or automatic - 1
	double				m_lowLimitRange = 5;										// lower limit of the range for automatic division
	double				m_highLimitRange = 95;										// high limit of the range for automatic division

	int					m_viewType = LO_VIEW_TYPE_SIMPLE;							// type of measurements list extended or simple

public:

	MeasurePointBase&	points() { return m_pointBase; }

	double				errorLimit() const { return m_errorLimit; }
	void				setErrorLimit(double errorLimit) { m_errorLimit = errorLimit; }

	int					errorType() const { return m_errorType; }
	void				setErrorType(int type) { m_errorType = type; }

	int					limitType() const { return m_limitType; }
	void				setLimitType(int type) { m_limitType = type; }

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
						QT_TRANSLATE_NOOP("Options.h", "Limit of error (%)"),
						QT_TRANSLATE_NOOP("Options.h", "Start value (%)"),
						QT_TRANSLATE_NOOP("Options.h", "Error type"),
						QT_TRANSLATE_NOOP("Options.h", "Show error from limit"),
						QT_TRANSLATE_NOOP("Options.h", "Start measurement from the сomparator"),
						QT_TRANSLATE_NOOP("Options.h", "Enable to measure hysteresis of comparators"),
};

const int				CO_PARAM_COUNT					= sizeof(ComparatorParamName)/sizeof(ComparatorParamName[0]);

const int				CO_PARAM_ERROR_LIMIT			= 0,
						CO_PARAM_START_VALUE			= 1,
						CO_PARAM_ERROR_TYPE				= 2,
						CO_PARAM_SHOW_ERROR_FROM_LIMIT	= 3,
						CO_PARAM_COMPARATOR_INDEX		= 4,
						CO_PARAM_ENABLE_HYSTERESIS		= 5;

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

	double				m_errorLimit = 0.2;										// permissible error is given by specified documents
	double				m_startValueForCompare = 0.1;							// start value is given by metrologists
	int					m_errorType = MEASURE_ERROR_TYPE_REDUCE;				// type of error absolute or reduced
	int					m_limitType = MEASURE_LIMIT_TYPE_ELECTRIC;				// type of displaing error denend on limit

	int					m_startComparatorIndex = 0;								// start the measurement with the сomparators under the number ...
	bool				m_enableMeasureHysteresis = false;						// enable flag to measure hysteresis of сomparator

public:

	double				errorLimit() const { return m_errorLimit; }
	void				setErrorLimit(double errorLimit) { m_errorLimit = errorLimit; }

	double				startValueForCompare() const { return m_startValueForCompare; }
	void				setStartValueForCompare(double value) { m_startValueForCompare = value; }

	int					errorType() const { return m_errorType; }
	void				setErrorType(int type) { m_errorType = type; }

	int					limitType() const { return m_limitType; }
	void				setLimitType(int type) { m_limitType = type; }

	int					startComparatorIndex() const { return m_startComparatorIndex; }
	void				setStartComparatorIndex(int index) { m_startComparatorIndex = index; }

	bool				enableMeasureHysteresis() const { return m_enableMeasureHysteresis; }
	void				setEnableMeasureHysteresis(bool enable) { m_enableMeasureHysteresis = enable; }

	void				load();
	void				save();

	ComparatorOption&	operator=(const ComparatorOption& from);
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
	int					m_measureKind = MEASURE_KIND_ONE_RACK;					// measure kind: each channel separately - 0 or for all channels together - 1
	int					m_signalConnectionType = SIGNAL_CONNECTION_TYPE_UNUSED;	// selected type of connection

	QString				m_defaultRack;
	QString				m_defaultSignalId;

public:

	int					measureTimeout() const { return m_measureTimeout; }
	void				setMeasureTimeout(int timeout) { m_measureTimeout = timeout; }

	int					measureKind() const { return m_measureKind; }
	void				setMeasureKind(int kind) { m_measureKind = kind; }

	int					signalConnectionType() const { return m_signalConnectionType; }
	void				setSignalConnectionType(int type) { m_signalConnectionType = type; }

	QString				defaultRack() const { return m_defaultRack; }
	void				setDefaultRack(const QString& rack) { m_defaultRack = rack; }

	QString				defaultSignalId() const { return m_defaultSignalId; }
	void				setDefaultSignalId(const QString& signalId) { m_defaultSignalId = signalId; }

	void				load();
	void				save();

	ToolBarOption&		operator=(const ToolBarOption& from);
};

// ==============================================================================================

#define					MEASURE_VIEW_OPTIONS_KEY		"Options/MeasureView/"

// ----------------------------------------------------------------------------------------------

const char* const		LanguageTypeStr[] =
{
						QT_TRANSLATE_NOOP("Options.h", "English"),
						QT_TRANSLATE_NOOP("Options.h", "Russian"),
};

const int				LANGUAGE_TYPE_COUNT	= sizeof(LanguageTypeStr)/sizeof(LanguageTypeStr[0]);

const int				LANGUAGE_TYPE_EN	= 0,
						LANGUAGE_TYPE_RU	= 1;

// ----------------------------------------------------------------------------------------------

const char* const		MeasureViewParam[] =
{
						QT_TRANSLATE_NOOP("Options.h", "Font of measurements list"),
						QT_TRANSLATE_NOOP("Options.h", "Color measurement that has not error"),
						QT_TRANSLATE_NOOP("Options.h", "Color measurement over limit error"),
						QT_TRANSLATE_NOOP("Options.h", "Color measurement over control error"),
						QT_TRANSLATE_NOOP("Options.h", "Show measuring value if signal is not valid"),
						QT_TRANSLATE_NOOP("Options.h", "Show accuracy for measure value and nominal value from calibrator"),
};

const int				MWO_PARAM_COUNT						= sizeof(MeasureViewParam)/sizeof(MeasureViewParam[0]);

const int				MWO_PARAM_FONT						= 0,
						MWO_PARAM_COLOR_NOT_ERROR			= 1,
						MWO_PARAM_COLOR_LIMIT_ERROR			= 2,
						MWO_PARAM_COLOR_CONTROL_ERROR		= 3,
						MWO_PARAM_SHOW_NO_VALID				= 4,
						MWO_PARAM_PRECESION_BY_CALIBRATOR	= 5;

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

	MeasureViewColumn	m_column[MEASURE_TYPE_COUNT][LANGUAGE_TYPE_COUNT][MEASURE_VIEW_COLUMN_COUNT];

private:

	int					m_measureType = MEASURE_TYPE_UNDEFINED;						// current, active ViewID

	bool				m_updateColumnView[MEASURE_TYPE_COUNT];						// determined the need to update the view after changing settings

	QFont				m_font;
	QFont				m_fontBold;

	QColor				m_colorNotError = COLOR_NOT_ERROR;
	QColor				m_colorErrorLimit = COLOR_OVER_LIMIT_ERROR;
	QColor				m_colorErrorControl = COLOR_OVER_CONTROL_ERROR;

	bool				m_showNoValid = false;										// show measuring value if signal is not valid
	bool				m_precesionByCalibrator = false;							// show accuracy for measure value and nominal value from calibrator

public:

	int					measureType() const { return m_measureType; }
	void				setMeasureType(int measureType) { m_measureType = measureType; }

	bool				updateColumnView(int measureType) const;
	void				setUpdateColumnView(int measureType, bool state);

	QFont				font() const { return m_font; }
	void				setFont(const QString& fontStr)	{ m_font.fromString(fontStr); }

	QFont				fontBold() const { return m_fontBold; }
	void				setFontBold(const QString& fontStr) { m_fontBold.fromString(fontStr); }

	QColor				colorNotError() const { return m_colorNotError; }
	void				setColorNotError(QColor color) { m_colorNotError = color; }

	QColor				colorErrorLimit() const { return m_colorErrorLimit; }
	void				setColorErrorLimit(QColor color) { m_colorErrorLimit = color; }

	QColor				colorErrorControl() const { return m_colorErrorControl; }
	void				setColorErrorControl(QColor color) { m_colorErrorControl = color; }

	bool				showNoValid() const { return m_showNoValid; }
	void				setShowNoValid(bool enable) { m_showNoValid = enable; }

	bool				precesionByCalibrator() const { return m_precesionByCalibrator; }
	void				setPrecesionByCalibrator(bool enable) { m_precesionByCalibrator = enable; }

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
						QT_TRANSLATE_NOOP("Options.h", "Show measuring value if signal is not valid"),
						QT_TRANSLATE_NOOP("Options.h", "Show electric state"),
						QT_TRANSLATE_NOOP("Options.h", "Color flag no validity"),
						QT_TRANSLATE_NOOP("Options.h", "Color flag overflow"),
						QT_TRANSLATE_NOOP("Options.h", "Color flag underflow"),
						QT_TRANSLATE_NOOP("Options.h", "Time for updating state of signal (ms)"),
};

const int				SIO_PARAM_COUNT					= sizeof(SignalInfoParam)/sizeof(SignalInfoParam[0]);

const int				SIO_PARAM_FONT					= 0,
						SIO_PARAM_SHOW_NO_VALID			= 1,
						SIO_PARAM_ELECTRIC_STATE		= 2,
						SIO_PARAM_COLOR_FLAG_VALID		= 3,
						SIO_PARAM_COLOR_FLAG_OVERFLOW	= 4,
						SIO_PARAM_COLOR_FLAG_UNDERFLOW	= 5,
						SIO_PARAM_TIME_FOR_UPDATE		= 6;

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

	bool				m_showNoValid = false;										// show measuring value if signal is not valid
	bool				m_showElectricState = false;

	QColor				m_colorFlagValid = COLOR_FLAG_VALID;
	QColor				m_colorFlagOverflow = COLOR_FLAG_OVERFLOW;
	QColor				m_colorFlagUnderflow = COLOR_FLAG_OVERBREAK;

	int					m_timeForUpdate = 250; // 250 ms

public:

	QFont				font() const { return m_font; }
	void				setFont(const QString& fontStr)	{ m_font.fromString(fontStr); }

	bool				showNoValid() const { return m_showNoValid; }
	void				setShowNoValid(bool enable) { m_showNoValid = enable; }

	bool				showElectricState() const { return m_showElectricState; }
	void				setShowElectricState(bool show) { m_showElectricState = show; }

	QColor				colorFlagValid() const { return m_colorFlagValid; }
	void				setColorFlagValid(QColor color) { m_colorFlagValid = color; }

	QColor				colorFlagOverflow() const { return m_colorFlagOverflow; }
	void				setColorFlagOverflow(QColor color) { m_colorFlagOverflow = color; }

	QColor				colorFlagUnderflow() const { return m_colorFlagUnderflow; }
	void				setColorFlagUnderflow(QColor color) { m_colorFlagUnderflow = color; }

	int					timeForUpdate() const { return m_timeForUpdate; }
	void				setTimeForUpdate(int ms) { m_timeForUpdate = ms; }

	void				load();
	void				save();

	SignalInfoOption&	operator=(const SignalInfoOption& from);
};

// ==============================================================================================

#define					COMPARATOR_INFO_OPTIONS_KEY		"Options/ComparatorInfo/"

// ----------------------------------------------------------------------------------------------

const char* const		ComparatorInfoParam[] =
{
						QT_TRANSLATE_NOOP("Options.h", "Font of signal information list"),
						QT_TRANSLATE_NOOP("Options.h", "Displaying text, if comparator has state \"logical 0\""),
						QT_TRANSLATE_NOOP("Options.h", "Displaying text, if comparator has state \"logical 1\""),
						QT_TRANSLATE_NOOP("Options.h", "Color, if comparator has state \"logical 0\""),
						QT_TRANSLATE_NOOP("Options.h", "Color, if comparator has state \"logical 1\""),
						QT_TRANSLATE_NOOP("Options.h", "Time for updating state of comparator (ms)"),
};

const int				CIO_PARAM_COUNT						= sizeof(ComparatorInfoParam)/sizeof(ComparatorInfoParam[0]);

const int				CIO_PARAM_FONT						= 0,
						CIO_PARAM_DISPLAYING_STATE_FALSE	= 1,
						CIO_PARAM_DISPLAYING_STATE_TRUE		= 2,
						CIO_PARAM_COLOR_STATE_FALSE			= 3,
						CIO_PARAM_COLOR_STATE_TRUE			= 4,
						CIO_PARAM_TIME_FOR_UPDATE			= 5;

// ----------------------------------------------------------------------------------------------

#define					COLOR_COMPARATOR_STATE_FALSE		QColor(0xFF, 0xFF, 0xFF)
#define					COLOR_COMPARATOR_STATE_TRUE			QColor(0xA0, 0xFF, 0xA0)

// ----------------------------------------------------------------------------------------------

class ComparatorInfoOption : public QObject
{
	Q_OBJECT

public:

	explicit ComparatorInfoOption(QObject *parent = nullptr);
	explicit ComparatorInfoOption(const ComparatorInfoOption& from, QObject *parent = nullptr);
	virtual ~ComparatorInfoOption();

private:

	QFont				m_font;

	QString				m_displayingStateFalse;
	QString				m_displayingStateTrue;

	QColor				m_colorStateFalse = COLOR_COMPARATOR_STATE_FALSE;
	QColor				m_colorStateTrue = COLOR_COMPARATOR_STATE_TRUE;

	int					m_timeForUpdate = 250; // 250 ms

public:

	QFont				font() const { return m_font; }
	void				setFont(const QString& fontStr)	{ m_font.fromString(fontStr); }

	QString				displayingStateFalse() const { return m_displayingStateFalse; }
	void				setDisplayingStateFalse(const QString& state) { m_displayingStateFalse = state; }

	QString				displayingStateTrue() const { return m_displayingStateTrue; }
	void				setDisplayingStateTrue(const QString& state) { m_displayingStateTrue = state; }

	QColor				colorStateFalse() const { return m_colorStateFalse; }
	void				setColorStateFalse(QColor color) { m_colorStateFalse = color; }

	QColor				colorStateTrue() const { return m_colorStateTrue; }
	void				setColorStateTrue(QColor color) { m_colorStateTrue = color; }

	int					timeForUpdate() const { return m_timeForUpdate; }
	void				setTimeForUpdate(int ms) { m_timeForUpdate = ms; }

	void				load();
	void				save();

	ComparatorInfoOption&	operator=(const ComparatorInfoOption& from);
};

// ==============================================================================================

#define					DATABASE_OPTIONS_REG_KEY		"Options/Database/"

// ----------------------------------------------------------------------------------------------

const char* const		DatabaseParam[] =
{
						QT_TRANSLATE_NOOP("Options.h", "Location path"),
						QT_TRANSLATE_NOOP("Options.h", "Type"),
						QT_TRANSLATE_NOOP("Options.h", "On start application"),
						QT_TRANSLATE_NOOP("Options.h", "On exit application"),
						QT_TRANSLATE_NOOP("Options.h", "Path for copy"),
};

const int				DBO_PARAM_COUNT			= sizeof(DatabaseParam)/sizeof(DatabaseParam[0]);

const int				DBO_PARAM_LOCATION_PATH	= 0,
						DBO_PARAM_TYPE			= 1,
						DBO_PARAM_ON_START		= 2,
						DBO_PARAM_ON_EXIT		= 3,
						DBO_PARAM_COPY_PATH		= 4;

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

	QString				m_locationPath;
	int					m_type;

	bool				m_onStart = false;
	bool				m_onExit = true;
	QString				m_copyPath;

public:

	QString				locationPath() const { return m_locationPath; }
	void				setLocationPath(const QString& path) { m_locationPath = path; }

	int					type() const { return m_type; }
	void				setType(int type) { m_type = type; }

	bool				onStart () const { return m_onStart; }
	void				setOnStart(bool onStart) { m_onStart = onStart; }

	bool				onExit() const { return m_onExit; }
	void				setOnExit(bool onExit) { m_onExit = onExit; }

	QString				copyPath() const { return m_copyPath; }
	void				setCopyPath(const QString& path) { m_copyPath = path; }

	void				load();
	void				save();

	DatabaseOption&		operator=(const DatabaseOption& from);
};

// ==============================================================================================

#define					LANGUAGE_OPTIONS_REG_KEY			"Options/Language/"

// ----------------------------------------------------------------------------------------------

const char* const		LanguageParam[] =
{
						QT_TRANSLATE_NOOP("Options.h", "Language"),
};

const int				LNO_PARAM_COUNT		= sizeof(LanguageParam)/sizeof(LanguageParam[0]);

const int				LNO_PARAM_LANGUAGE_TYPE	= 0;

// ----------------------------------------------------------------------------------------------

#define					LANGUAGE_OPTIONS_DIR		"/languages"
#define					LANGUAGE_OPTIONS_FILE_RU	"Metrology_ru.qm"

// ----------------------------------------------------------------------------------------------

class LanguageOption : public QObject
{
	Q_OBJECT

public:

	explicit LanguageOption(QObject *parent = nullptr);
	explicit LanguageOption(const LanguageOption& from, QObject *parent = nullptr);
	virtual ~LanguageOption();

private:

	int					m_languageType = LANGUAGE_TYPE_EN;

public:

	int					languageType() const { return m_languageType; }
	void				setLanguageType(int type) { m_languageType = type; }

	void				load();
	void				save();

	LanguageOption&		operator=(const LanguageOption& from);
};

// ==============================================================================================

bool compareDouble(double lDouble, double rDouble);

// ==============================================================================================

class Options : public QObject
{
	Q_OBJECT

public:

	explicit Options(QObject *parent = nullptr);
	explicit Options(const Options& from, QObject *parent = nullptr);
	virtual ~Options();

private:

	QMutex					m_mutex;

	CalibratorsOption		m_calibrators;

	SocketOption			m_socket;
	ProjectInfo				m_projectInfo;

	ModuleOption			m_module;
	LinearityOption			m_linearity;
	ComparatorOption		m_comparator;

	ToolBarOption			m_toolBar;
	MeasureViewOption		m_measureView;

	SignalInfoOption		m_signalInfo;
	ComparatorInfoOption	m_comparatorInfo;

	DatabaseOption			m_database;

	LanguageOption			m_language;


public:

	CalibratorsOption&		calibrators() { return m_calibrators; }
	void					setCalibrators(const CalibratorsOption& calibrators) { m_calibrators = calibrators; }

	SocketOption&			socket() { return m_socket; }
	void					setSocket(const SocketOption& socket) { m_socket = socket; }

	ProjectInfo&			projectInfo() { return m_projectInfo; }
	void					setProjectInfo(const ProjectInfo& projectInfo) { m_projectInfo = projectInfo; }

	ModuleOption&			module() { return m_module; }
	void					setModule(const ModuleOption& module) { m_module = module; }

	LinearityOption&		linearity() { return m_linearity; }
	void					setLinearity(const LinearityOption& linearity) { m_linearity = linearity; }

	ComparatorOption&		comparator() { return m_comparator; }
	void					setComparator(const ComparatorOption& comparator) { m_comparator = comparator; }

	ToolBarOption&			toolBar() { return m_toolBar; }
	void					setToolBar(const ToolBarOption& toolBar) { m_toolBar = toolBar; }

	MeasureViewOption&		measureView() { return m_measureView; }
	void					setMeasureView(const MeasureViewOption& measureView) { m_measureView = measureView; }

	SignalInfoOption&		signalInfo() { return m_signalInfo; }
	void					setSignalInfo(const SignalInfoOption& signalInfo) { m_signalInfo = signalInfo; }

	ComparatorInfoOption&	comparatorInfo() { return m_comparatorInfo; }
	void					setComparatorInfo(const ComparatorInfoOption& comparatorInfo) { m_comparatorInfo = comparatorInfo; }

	DatabaseOption&			database() { return m_database; }
	void					setDatabase(const DatabaseOption& database) { m_database = database; }

	LanguageOption&			language() { return m_language; }
	void					setLanguage(const LanguageOption& language) { m_language = language; }

	void					load();
	void					save();

	bool					readFromXml(const QByteArray& fileData);

	Options&				operator=(const Options& from);
};

// ==============================================================================================

extern Options			theOptions;

// ==============================================================================================

#endif // OPTIONS_H
