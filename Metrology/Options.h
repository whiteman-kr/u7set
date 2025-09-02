#ifndef OPTIONS_H
#define OPTIONS_H

#include <cassert>

#include <QObject>
#include <QMutex>
#include <QFont>
#include <QColor>

#include "../OnlineLib/SoftwareSettings.h"
#include "../OnlineLib/SocketIO.h"

#include "MetrologySignal.h"
#include "MeasureViewHeader.h"
#include "MeasureBase.h"
#include "MeasurePointBase.h"

// ==============================================================================================

#define WINDOW_GEOMETRY_OPTIONS_KEY "Options/Window/"

// ==============================================================================================

#define CALIBRATOR_OPTIONS_KEY "Options/Calibrators/"

// ----------------------------------------------------------------------------------------------

class CalibratorOption
{

public:

	CalibratorOption();
	CalibratorOption(const QString& port, CalibratorType type);
	virtual ~CalibratorOption() {}

public:

	bool isValid() const;

	QString port() const { return m_port; }
	void setPort(const QString& port) { m_port = port; }

	CalibratorType type() const { return m_type; }
	void setType(CalibratorType type) { m_type = type; }

private:

	QString m_port;
	CalibratorType m_type = CalibratorType::Calys75;
};

// ----------------------------------------------------------------------------------------------

class CalibratorsOption : public QObject
{
	Q_OBJECT

public:

	explicit CalibratorsOption(QObject* parent = nullptr);
	explicit CalibratorsOption(const CalibratorsOption& from, QObject* parent = nullptr);
	virtual ~CalibratorsOption() override;

public:

	CalibratorOption calibrator(int channel) const;
	void setCalibrator(int channel, const CalibratorOption& calibrator);

	//
	//
	void load();
	void save();

	//
	//
	CalibratorsOption& operator=(const CalibratorsOption& from);

private:

	CalibratorOption m_calibrator[Metrology::CHANNEL_COUNT];
};


// ==============================================================================================

#define		SOCKET_OPTIONS_KEY				"Options/Socket/"

// ----------------------------------------------------------------------------------------------

namespace OT
{
	Q_NAMESPACE

	// ----------------------------------------------------------------------------------------------

	enum ServerType
	{
		ConfigurationService = 0,
		AppDataService = 1,
		TuningService = 2,
	};
	Q_ENUM_NS(ServerType)

	const int ServerTypeCount = 3;

	#define ERR_SERVER_TYPE(type) (static_cast<int>(type) < 0 || static_cast<int>(type) >= OT::ServerTypeCount)

	QString serverCaption(OT::ServerType type);
	QString serverDefaultID(OT::ServerType type);
	int serverDefaultPort(OT::ServerType type);

	// ----------------------------------------------------------------------------------------------

	enum ServerPriority
	{
		Primary = 0,
		Reserve = 1,
	};
	Q_ENUM_NS(ServerPriority)

	const int ServerPriorityCount = 2;

	#define ERR_SERVER_PRIORITY(priority) (static_cast<int>(priority) < 0 || static_cast<int>(priority) >= OT::ServerPriorityCount)

	QString serverPriorityCaption(ServerPriority priority);

	// ----------------------------------------------------------------------------------------------

	enum ServerConnectionParam
	{
		sco_Type = 0,
		sco_Priority = 1,
		sco_EquipmentID = 2,
		sco_ServerIP = 3,
		sco_ServerPort = 4,
	};
	Q_ENUM_NS(ServerConnectionParam)

	const int ServerConnectionParamCount = 5;

	#define ERR_SERVER_CONNECTION_PARAM(param) (static_cast<int>(param) < 0 || static_cast<int>(param) >= OT::ServerConnectionParamCount)

	QString serverConnectionParamCaption(OT::ServerConnectionParam param);

	// ----------------------------------------------------------------------------------------------
}

// ----------------------------------------------------------------------------------------------

class ServerConnection
{
public:

	ServerConnection();
	virtual ~ServerConnection() {}

public:

	//
	//
	OT::ServerPriority serverPriority() const { return m_priority; }
	void setServerPriority(OT::ServerPriority priority) { m_priority = priority; }

	//
	//
	bool isValid() const { return m_isValid; }
	void setIsValid(bool valid) { m_isValid = valid; }

	QString equipmentID() const { return m_equipmentID; }
	void setEquipmentID(const QString& equipmentID) { m_equipmentID = equipmentID; }

	QString serverIP() const { return m_serverIP; }
	void setServerIP(const QString& ip) { m_serverIP = ip; }

	int serverPort() const { return m_serverPort; }
	void setServerPort(int port) { m_serverPort = port; }

	//
	//
	void load(OT::ServerType serverType);
	void save(OT::ServerType serverType);

private:

	//
	//
	OT::ServerPriority m_priority = OT::ServerPriority::Primary;

	//
	//
	bool m_isValid = false;
	QString m_equipmentID;

	QString m_serverIP;
	int m_serverPort = Socket::PORT_LOWEST;
};

// ----------------------------------------------------------------------------------------------

class ServerOption
{
public:

	ServerOption();
	virtual ~ServerOption() {}

public:

	OT::ServerType serverType() const { return m_type; }
	void setServerType(OT::ServerType serverType) { m_type = serverType; }

	QString equipmentID(OT::ServerPriority priority) const;
	void setEquipmentID(OT::ServerPriority priority, const QString& equipmentID);

	QString serverIP(OT::ServerPriority priority) const;
	void setServerIP(OT::ServerPriority priority, const QString& ip);

	int serverPort(OT::ServerPriority priority) const;
	void setServerPort(OT::ServerPriority priority, int port);

	//
	//
	HostAddressPort address(OT::ServerPriority priority) const;

	ServerConnection* connection(OT::ServerPriority priority) const;

	//
	//
	void load();
	void save();

	bool init(const MetrologySettings& settings);

private:

	OT::ServerType m_type = OT::ServerType::ConfigurationService;

	ServerConnection m_connection[OT::ServerPriorityCount];
};

// ----------------------------------------------------------------------------------------------

class SocketOption : public QObject
{
	Q_OBJECT

public:

	explicit SocketOption(QObject* parent = nullptr);
	explicit SocketOption(const SocketOption& from, QObject* parent = nullptr);
	virtual ~SocketOption() override;

public:

	OT::ServerType type() const { return m_type; };
	void setType(OT::ServerType serverType) { m_type = serverType; };

	OT::ServerPriority priority() const { return m_priority; }
	void setPriority(OT::ServerPriority priority) { m_priority = priority; }

	//
	//
	ServerOption server(OT::ServerType serverType) const;
	void setServer(OT::ServerType serverType, const ServerOption& server);

	//
	//
	QString equipmentID() const;
	void setEquipmentID(const QString& equipmentID);

	QString serverIP() const;
	void setServerIP(const QString& ip);

	int serverPort() const;
	void setServerPort(int port);

	//
	//
	void load();
	void save();

	//
	//
	SocketOption& operator=(const SocketOption& from);

private:

	OT::ServerType m_type = OT::ServerType::ConfigurationService;
	OT::ServerPriority m_priority = OT::ServerPriority::Primary;

	ServerOption m_server[OT::ServerTypeCount];
};

// ==============================================================================================

#define PROJECT_INFO_KEY "Options/ProjectInfo/"

// ----------------------------------------------------------------------------------------------

class ProjectInfo: public QObject
{
	Q_OBJECT

public:

	explicit ProjectInfo(QObject* parent = nullptr);
	explicit ProjectInfo(const ProjectInfo& from, QObject* parent = nullptr);
	virtual ~ProjectInfo() override;

public:

	QString projectName() { return m_projectName; }
	int id() { return m_id; }
	QString date() { return m_date; }
	int changeset() { return m_changeset; }
	QString user() { return m_user; }
	QString workstation() { return m_workstation; }
	int dbVersion() { return m_dbVersion; }
	int cfgFileVersion() { return m_cfgFileVersion; }
	void setCfgFileVersion(int version) { m_cfgFileVersion = version; }

	//
	//
	void save();
	bool readFromXml(const QByteArray& fileData);

	//
	//
	ProjectInfo& operator=(const ProjectInfo& from);

private:

	void appendProperties();

	QString m_projectName;
	int m_id = -1;
	QString m_date;
	int m_changeset = 0;
	QString m_user;
	QString m_workstation;
	int m_dbVersion = 0;
	int m_cfgFileVersion = 0;
};

// ==============================================================================================

#define MODULE_OPTIONS_KEY "Options/Module/"

// ----------------------------------------------------------------------------------------------

namespace OT
{
	Q_NAMESPACE

	enum ModuleParam
	{
		NoModuleParam = -1,
		mo_SuffixSN = 0,
		mo_MeasureInterInsteadIn = 1,
		mo_MeasureLinAdnCmp = 2,
		mo_MeasureEntireModule = 3,
		mo_ShowOnSchemas = 4,
		mo_WarningIfMeasured = 5,
		mo_MaxInputs = 6,
	};
	Q_ENUM_NS(ModuleParam)

	const int ModuleParamCount = 7;

	#define ERR_MO_PARAM_TYPE(param) (static_cast<int>(param) < 0 || static_cast<int>(param) >= OT::ModuleParamCount)

	QString ModuleParamCaption(ModuleParam param);
}

// ----------------------------------------------------------------------------------------------

class ModuleOption : public QObject
{
	Q_OBJECT

public:

	explicit ModuleOption(QObject* parent = nullptr);
	explicit ModuleOption(const ModuleOption& from, QObject* parent = nullptr);
	virtual ~ModuleOption() override;

public:

	QString suffixSN() const { return m_suffixSN; }
	void setSuffixSN(const QString& suffixSN) { m_suffixSN = suffixSN; }

	bool measureInterInsteadIn() const { return m_measureInterInsteadIn; }
	void setMeasureInterInsteadIn(bool measure) { m_measureInterInsteadIn = measure; }

	bool measureLinAndCmp() const { return m_measureLinAndCmp; }
	void setMeasureLinAndCmp(bool measure) { m_measureLinAndCmp = measure; }

	bool measureEntireModule() const { return m_measureEntireModule; }
	void setMeasureEntireModule(bool measure) { m_measureEntireModule = measure; }

	bool measureShownOnSchemas() const { return m_measureShownOnSchemas; }
	void setMeasureShownOnSchemas(bool measure) { m_measureShownOnSchemas = measure; }

	bool warningIfMeasured() const { return m_warningIfMeasured; }
	void setWarningIfMeasured(bool enable) { m_warningIfMeasured = enable; }

	int maxInputCount() const { return m_maxInputCount; }
	void setMaxInputCount(int count);

	//
	//
	void load();
	void save();

	//
	//
	ModuleOption& operator=(const ModuleOption& from);

private:

	QString m_suffixSN;												// suffix to identify the signal of module serial number

	bool m_measureInterInsteadIn = false;							// measure internal signal instead input signal
	bool m_measureLinAndCmp = false;								// measure linearity and comparators together
	bool m_measureEntireModule = false;								// measure all inputs of module in series
	bool m_measureShownOnSchemas = false;							// measure only signals that are displayed in schemas
	bool m_warningIfMeasured = true;								// show warning if signal is already measured

	int m_maxInputCount = Metrology::InputCount;					// maximum number of inputs for input mofule
};

// ==============================================================================================

#define LINEARITY_OPTIONS_KEY "Options/Linearity/"

// ----------------------------------------------------------------------------------------------

namespace OT
{
	Q_NAMESPACE

	// ----------------------------------------------------------------------------------------------

	enum LinearityParam
	{
		NoLinearityParam = -1,
		lo_ErrorLimit = 0,
		lo_ErrorType = 1,
		lo_CalcErrorByRange = 2,
		lo_MeasureTime = 3,
		lo_MaxMeasuresInPoint = 4,
		lo_DivisionType = 5,
		lo_PointCount = 6,
		lo_LowLimit = 7,
		lo_HighLimit = 8,
		lo_ValuesOfPoints = 9,
		lo_ViewType = 10,
	};
	Q_ENUM_NS(LinearityParam)

	const int LinearityParamCount = 11;

	#define ERR_LO_PARAM(param) (static_cast<int>(param) < 0 || static_cast<int>(param) >= OT::LinearityParamCount)

	QString LinearityParamCaption(LinearityParam param);

	// ----------------------------------------------------------------------------------------------

	enum LinearityViewType
	{
		Simple				= 0,
		Extended			= 1,
		Detail_Electric		= 2,
		Detail_Engineering	= 3,
	};
	Q_ENUM_NS(LinearityViewType)

	const int LinearityViewTypeCount = 4;

	#define ERR_LINEARITY_VIEW_TYPE(type) (static_cast<int>(type) < 0 || static_cast<int>(type) >= OT::LinearityViewTypeCount)

	QString LinearityViewTypeCaption(LinearityViewType type);
	QString LinearityViewTypeCaptionTr(LinearityViewType type);

	// ----------------------------------------------------------------------------------------------
}

// ----------------------------------------------------------------------------------------------

class LinearityOption : public QObject
{
	Q_OBJECT

public:

	explicit LinearityOption(QObject* parent = nullptr);
	explicit LinearityOption(const LinearityOption& from, QObject* parent = nullptr);
	virtual ~LinearityOption() override;

public:

	Measure::PointBase&	points() { return m_pointBase; }

	double errorLimit() const { return m_errorLimit; }
	void setErrorLimit(double errorLimit) { m_errorLimit = errorLimit; }

	Measure::MT::ErrorType errorType() const { return m_errorType; }
	void setErrorType(Measure::MT::ErrorType type) { m_errorType = type; }

	Measure::MT::CalcErrorRange calcErrorByRange() const { return m_calcErrorByRange; }
	void setCalcErrorByRange(Measure::MT::CalcErrorRange byRange) { m_calcErrorByRange = byRange; }

	int measureTimeInPoint() const { return m_measureTimeInPoint; }
	void setMeasureTimeInPoint(int sec) { m_measureTimeInPoint = sec; }

	int measureCountInPoint();
	void setMeasureCountInPoint(int measureCount);

	Measure::LT::LinearityDivision divisionType() const { return m_divisionType; }
	void setDivisionType(Measure::LT::LinearityDivision divisionType) { m_divisionType = divisionType; }

	double lowLimitRange() const { return m_lowLimitRange; }
	void setLowLimitRange(double lowLimit, bool updateMeasurePoints = true);

	double highLimitRange() const { return m_highLimitRange; }
	void setHighLimitRange(double highLimit, bool updateMeasurePoints = true);

	OT::LinearityViewType viewType() const { return m_viewType; }
	void setViewType(OT::LinearityViewType viewType) { m_viewType = viewType; }

	//
	//
	int measurePointsCount() { return m_pointBase.count(); }
	void setMeasurePointsCount(int count = -1);
	QString measurePointsText() {return m_pointBase.text();  };

	void load();
	void save();

	//
	//
	LinearityOption& operator=(const LinearityOption& from);

private:

	Measure::PointBase m_pointBase;																		// list of measurement points

	double m_errorLimit = 0.1;																			// permissible error is given by specified documents
	Measure::MT::ErrorType m_errorType = Measure::MT::ErrorType::Reduce;								// type of error absolute or reduced
	Measure::MT::CalcErrorRange m_calcErrorByRange = Measure::MT::CalcErrorRange::By_Electric_Range;	// error is calculated by the range

	int m_measureTimeInPoint = 1;																		// time, in seconds, during which will be made ​​N measurements at each point
	int m_measureCountInPoint = Measure::MAX_MEASUREMENT_IN_POINT;											// count of measurements in a point, according to GOST MI-2002 application 7

	Measure::LT::LinearityDivision m_divisionType = Measure::LT::LinearityDivision::Manual;				// type of division measure range: manual - 0 or automatic - 1
	double m_lowLimitRange = Measure::LinearityRangeLow;												// lower limit of the range for automatic division
	double m_highLimitRange = Measure::LinearityRangeHigh;												// high limit of the range for automatic division

	OT::LinearityViewType m_viewType = OT::LinearityViewType::Simple;									// type of measurements list: simple or extended and etc
};

// ==============================================================================================

#define COMPARATOR_OPTIONS_KEY "Options/Comparator/"

// ----------------------------------------------------------------------------------------------

namespace OT
{
	Q_NAMESPACE

	enum ComparatorParam
	{
		NoComparatorParam = -1,
		co_ErrorLimit = 0,
		co_ErrorType = 1,
		co_CalcErrorByRange = 2,
		co_StartValue = 3,
		co_StartFromComparator = 4,
		co_MeasureHysteresis = 5,
	};
	Q_ENUM_NS(ComparatorParam)

	const int ComparatorParamCount = 6;

	#define ERR_CO_PARAM_TYPE(param) (static_cast<int>(param) < 0 || static_cast<int>(param) >= OT::ComparatorParamCount)

	QString ComparatorParamCaption(ComparatorParam param);
}

// ----------------------------------------------------------------------------------------------

class ComparatorOption : public QObject
{
	Q_OBJECT

public:

	explicit ComparatorOption(QObject* parent = nullptr);
	explicit ComparatorOption(const ComparatorOption& from, QObject* parent = nullptr);
	virtual ~ComparatorOption() override;


public:

	double errorLimit() const { return m_errorLimit; }
	void setErrorLimit(double errorLimit) { m_errorLimit = errorLimit; }

	Measure::MT::ErrorType errorType() const { return m_errorType; }
	void setErrorType(Measure::MT::ErrorType type) { m_errorType = type; }

	Measure::MT::CalcErrorRange calcErrorByRange() const { return m_calcErrorByRange; }
	void setCalcErrorByRange(Measure::MT::CalcErrorRange byRange) { m_calcErrorByRange = byRange; }

	double startValueForCompare() const { return m_startValueForCompare; }
	void setStartValueForCompare(double value) { m_startValueForCompare = value; }

	int startFromComparator() const { return m_startFromComparator + 1; }
	void setStartFromComparator(int index);

	bool enableMeasureHysteresis() const { return m_enableMeasureHysteresis; }
	void setEnableMeasureHysteresis(bool enable) { m_enableMeasureHysteresis = enable; }

	//
	//
	void load();
	void save();

	//
	//
	ComparatorOption& operator=(const ComparatorOption& from);

private:

	double m_errorLimit = 0.1;																			// permissible error is given by specified documents
	Measure::MT::ErrorType m_errorType = Measure::MT::ErrorType::Reduce;								// type of error absolute or reduced
	Measure::MT::CalcErrorRange m_calcErrorByRange = Measure::MT::CalcErrorRange::By_Electric_Range;	// error is calculated by the range
	double m_startValueForCompare = 0.1;																// start value is given by metrologists

	int m_startFromComparator = 0;																		// start the measurement with the сomparators under the number ...
	bool m_enableMeasureHysteresis = false;																// enable flag to measure hysteresis of сomparator
};

// ==============================================================================================

#define TOOLBAR_OPTIONS_KEY "Options/ToolBar/"

// ----------------------------------------------------------------------------------------------

class ToolBarOption : public QObject
{
	Q_OBJECT

public:

	explicit ToolBarOption(QObject* parent = nullptr);
	explicit ToolBarOption(const ToolBarOption& from, QObject* parent = nullptr);
	virtual ~ToolBarOption() override;

public:

	int measureTimeout() const { return m_measureTimeout; }
	void setMeasureTimeout(int timeout) { m_measureTimeout = timeout; }

	int measureKind() const { return m_measureKind; }
	void setMeasureKind(int kind) { m_measureKind = kind; }

	int connectionType() const { return m_connectionType; }
	void setConnectionType(int type) { m_connectionType = type; }

	QString defaultRack() const { return m_defaultRack; }
	void setDefaultRack(const QString& rack) { m_defaultRack = rack; }

	QString defaultSignalId() const { return m_defaultSignalId; }
	void setDefaultSignalId(const QString& signalId) { m_defaultSignalId = signalId; }

	//
	//
	void load();
	void save();

	//
	//
	ToolBarOption& operator=(const ToolBarOption& from);

private:

	int m_measureTimeout = 0;											// in milliseconds, timeout between the time when the calibrator is set value and the time when the application is save measurement
	int m_measureKind = Measure::Kind::OneRack;							// measure kind: each channel separately - 0 or for all channels together - 1
	int m_connectionType = Metrology::ConnectionType::Unused;			// selected type of connection

	QString m_defaultRack;
	QString m_defaultSignalId;
};

// ==============================================================================================

#define MEASURE_VIEW_OPTIONS_KEY "Options/MeasureView/"

// ----------------------------------------------------------------------------------------------

namespace OT
{
	Q_NAMESPACE

	// ----------------------------------------------------------------------------------------------

	enum LanguageType
	{
		English	= 0,
		Russian	= 1,
		Ukrainian = 2,
	};
	Q_ENUM_NS(LanguageType)

	const int LanguageTypeCount = 3;

	#define ERR_LANGUAGE_TYPE(type) (static_cast<int>(type) < 0 || static_cast<int>(type) >= OT::LanguageTypeCount)

	QString LanguageTypeCaptionEn(OT::LanguageType type);
	QString LanguageTypeCaptionTr(OT::LanguageType type);

	// ----------------------------------------------------------------------------------------------

	enum MeasureViewParam
	{
		NoMeasureViewParam = -1,
		mwo_Font = 0,
		mwo_ColorNoError = 1,
		mwo_ColorErrorOfLimit = 2,
		mwo_ColorErrorOfControl = 3,
		mwo_ShowNoValid = 4,
		mwo_PrecesionByCalibrator = 5,
	};
	Q_ENUM_NS(MeasureViewParam)

	const int MeasureViewParamCount = 6;

	#define ERR_MWO_PARAM_TYPE(param) (static_cast<int>(param) < 0 || static_cast<int>(param) >= OT::MeasureViewParamCount)

	QString MeasureViewParamCaption(MeasureViewParam param);

	// ----------------------------------------------------------------------------------------------
}

// ----------------------------------------------------------------------------------------------

#define					COLOR_NOT_ERROR				QColor(0xA0, 0xFF, 0xA0)
#define					COLOR_OVER_LIMIT_ERROR		QColor(0xFF, 0xA0, 0xA0)
#define					COLOR_OVER_CONTROL_ERROR	QColor(0xFF, 0xD0, 0xA0)

// ----------------------------------------------------------------------------------------------

class MeasureViewOption : public QObject
{
	Q_OBJECT

public:
	explicit MeasureViewOption(QObject* parent = nullptr);
	explicit MeasureViewOption(const MeasureViewOption& from, QObject* parent = nullptr);
	virtual ~MeasureViewOption() override;

//private:
public:

	Measure::HeaderColumn m_column[Measure::TYPE_COUNT][OT::LanguageTypeCount][Measure::MaxColumnCount];

public:

	int measureType() const { return m_measureType; }
	void setMeasureType(int measureType) { m_measureType = measureType; }

	bool updateColumnView(Measure::Type measureType) const;
	void setUpdateColumnView(Measure::Type measureType, bool state);

	QFont font() const { return m_font; }
	void setFont(const QFont& font)	{ m_font = font; }
	void setFont(const QString& fontStr) { m_font.fromString(fontStr); }

	QFont fontBold() const { return m_fontBold; }
	void setFontBold(const QFont& font)	{ m_fontBold = font; }
	void setFontBold(const QString& fontStr) { m_fontBold.fromString(fontStr); }

	QColor colorNotError() const { return m_colorNotError; }
	void setColorNotError(QColor color) { m_colorNotError = color; }

	QColor colorErrorLimit() const { return m_colorErrorLimit; }
	void setColorErrorLimit(QColor color) { m_colorErrorLimit = color; }

	QColor colorErrorControl() const { return m_colorErrorControl; }
	void setColorErrorControl(QColor color) { m_colorErrorControl = color; }

	bool showNoValid() const { return m_showNoValid; }
	void setShowNoValid(bool enable) { m_showNoValid = enable; }

	bool precesionByCalibrator() const { return m_precesionByCalibrator; }
	void setPrecesionByCalibrator(bool enable) { m_precesionByCalibrator = enable; }

	//
	//
	void load();
	void save();

	void saveColumnWidth(Measure::Type measureType, const Measure::HeaderColumn& c);

	//
	//
	MeasureViewOption& operator=(const MeasureViewOption& from);

private:

	int m_measureType = Measure::Type::NoMeasureType;				// current, active ViewID

//	bool m_updateColumnView[Measure::TYPE_COUNT];              // determined the need to update the view after changing settings - Galytskyi code, warning C26495, Variable 'MeasureViewOption::m_updateColumnView' is uninitialized. Always initialize a member variable (type.6). u7set D:\u7set\Metrology\Options.cpp 1352
	std::array<bool, Measure::TYPE_COUNT> m_updateColumnView{}; // Ovcharenko 29.08.25 without warning C26495

	QFont m_font;
	QFont m_fontBold;

	QColor m_colorNotError = COLOR_NOT_ERROR;
	QColor m_colorErrorLimit = COLOR_OVER_LIMIT_ERROR;
	QColor m_colorErrorControl = COLOR_OVER_CONTROL_ERROR;

	bool m_showNoValid = false;										// show measuring value if signal is not valid
	bool m_precesionByCalibrator = false;							// show accuracy for measure value and nominal value from calibrator
};

// ==============================================================================================

#define SIGNAL_INFO_OPTIONS_KEY "Options/SignalInfo/"

// ----------------------------------------------------------------------------------------------

namespace OT
{
	Q_NAMESPACE

	enum SignalInfoParam
	{
		NoSignalInfoParam = -1,
		sio_Font = 0,
		sio_ShowNoValid = 1,
		sio_ShowElectricState = 2,
		sio_ColorFlagNoValid = 3,
		sio_ColorFlagSim = 4,
		sio_ColorFlagLock = 5,
		sio_ColorFlagOverflow = 6,
		sio_ColorFlagUnderflow = 7,
		sio_TimeForUpdate = 8,
	};
	Q_ENUM_NS(SignalInfoParam)

	const int SignalInfoParamCount = 9;

	#define ERR_SIO_PARAM(param) (static_cast<int>(param) < 0 || static_cast<int>(param) >= OT::SignalInfoParamCount)

	QString SignalInfoParamCaption(SignalInfoParam param);
}

// ----------------------------------------------------------------------------------------------

#define COLOR_FLAG_VALID		QColor(0xFF, 0xA0, 0xA0)
#define COLOR_FLAG_SIM			QColor(0x50, 0xA0, 0xFF)
#define COLOR_FLAG_LOCK			QColor(0xA0, 0xA0, 0xA0)
#define COLOR_FLAG_OVERFLOW		QColor(0xFF, 0xD0, 0xA0)
#define COLOR_FLAG_OVERBREAK	QColor(0xFF, 0xD0, 0xA0)

// ----------------------------------------------------------------------------------------------

class SignalInfoOption : public QObject
{
	Q_OBJECT

public:

	explicit SignalInfoOption(QObject* parent = nullptr);
	explicit SignalInfoOption(const SignalInfoOption& from, QObject* parent = nullptr);
	virtual ~SignalInfoOption() override;

public:

	QFont font() const { return m_font; }
	void setFont(const QFont& font)	{ m_font = font; }
	void setFont(const QString& fontStr) { m_font.fromString(fontStr); }

	bool showNoValid() const { return m_showNoValid; }
	void setShowNoValid(bool enable) { m_showNoValid = enable; }

	bool showElectricState() const { return m_showElectricState; }
	void setShowElectricState(bool show) { m_showElectricState = show; }

	QColor colorFlagValid() const { return m_colorFlagValid; }
	void setColorFlagValid(QColor color) { m_colorFlagValid = color; }

	QColor colorFlagSim() const { return m_colorFlagSim; }
	void setColorFlagSim(QColor color) { m_colorFlagSim = color; }

	QColor colorFlagLock() const { return m_colorFlagLock; }
	void setColorFlagLock(QColor color) { m_colorFlagLock = color; }

	QColor colorFlagOverflow() const { return m_colorFlagOverflow; }
	void setColorFlagOverflow(QColor color) { m_colorFlagOverflow = color; }

	QColor colorFlagUnderflow() const { return m_colorFlagUnderflow; }
	void setColorFlagUnderflow(QColor color) { m_colorFlagUnderflow = color; }

	int timeForUpdate() const { return m_timeForUpdate; }
	void setTimeForUpdate(int ms) { m_timeForUpdate = ms; }

	QMap<QString, int> columnsWidth() const { return m_columnsWidth; }
	void setColumnsWidth(const QMap<QString, int>& map) { m_columnsWidth = map; }

	//
	//
	void load();
	void save();

	void loadColumnsWidth();
	void saveColumnsWidth();

	//
	//
	SignalInfoOption& operator=(const SignalInfoOption& from);

private:

	QFont m_font;

	bool m_showNoValid = false;										// show measuring value if signal is not valid
	bool m_showElectricState = false;

	QColor m_colorFlagValid = COLOR_FLAG_VALID;
	QColor m_colorFlagSim = COLOR_FLAG_SIM;
	QColor m_colorFlagLock = COLOR_FLAG_LOCK;
	QColor m_colorFlagOverflow = COLOR_FLAG_OVERFLOW;
	QColor m_colorFlagUnderflow = COLOR_FLAG_OVERBREAK;

	int m_timeForUpdate = 250; // 250 ms

	QMap<QString, int> m_columnsWidth;
};

// ==============================================================================================

#define COMPARATOR_INFO_OPTIONS_KEY "Options/ComparatorInfo/"

// ----------------------------------------------------------------------------------------------

namespace OT
{
	Q_NAMESPACE

	enum ComparatorInfoParam
	{
		NoComparatorInfoParam = -1,
		cio_Font = 0,
		cio_ColorFlagSim = 1,
		cio_ColorFlagLock = 2,
		cio_ColorStateFalse = 3,
		cio_ColorStateTrue = 4,
		cio_TimeForUpdate = 5,
	};
	Q_ENUM_NS(ComparatorInfoParam)

	const int ComparatorInfoParamCount = 6;

	#define ERR_CIO_PARAM(param) (static_cast<int>(param) < 0 || static_cast<int>(param) >= OT::ComparatorInfoParamCount)

	QString ComparatorInfoParamCaption(ComparatorInfoParam param);
}

// ----------------------------------------------------------------------------------------------

#define COLOR_COMPARATOR_FLAG_SIM		QColor(0x50, 0xA0, 0xFF)
#define COLOR_COMPARATOR_FLAG_LOCK		QColor(0xA0, 0xA0, 0xA0)
#define COLOR_COMPARATOR_STATE_FALSE	QColor(0xFF, 0xFF, 0xFF)
#define COLOR_COMPARATOR_STATE_TRUE		QColor(0xA0, 0xFF, 0xA0)

// ----------------------------------------------------------------------------------------------

class ComparatorInfoOption : public QObject
{
	Q_OBJECT

public:

	explicit ComparatorInfoOption(QObject* parent = nullptr);
	explicit ComparatorInfoOption(const ComparatorInfoOption& from, QObject* parent = nullptr);
	virtual ~ComparatorInfoOption() override;

public:

	QFont font() const { return m_font; }
	void setFont(const QFont& font)	{ m_font = font; }
	void setFont(const QString& fontStr) { m_font.fromString(fontStr); }

	QColor colorFlagSim() const { return m_colorFlagSim; }
	void setColorFlagSim(QColor color) { m_colorFlagSim = color; }

	QColor colorFlagLock() const { return m_colorFlagLock; }
	void setColorFlagLock(QColor color) { m_colorFlagLock = color; }

	QColor colorStateFalse() const { return m_colorStateFalse; }
	void setColorStateFalse(QColor color) { m_colorStateFalse = color; }

	QColor colorStateTrue() const { return m_colorStateTrue; }
	void setColorStateTrue(QColor color) { m_colorStateTrue = color; }

	int timeForUpdate() const { return m_timeForUpdate; }
	void setTimeForUpdate(int ms) { m_timeForUpdate = ms; }

	//
	//
	void load();
	void save();

	//
	//
	ComparatorInfoOption& operator=(const ComparatorInfoOption& from);

private:

	QFont m_font;

	QColor m_colorFlagSim = COLOR_COMPARATOR_FLAG_SIM;
	QColor m_colorFlagLock = COLOR_COMPARATOR_FLAG_LOCK;

	QColor m_colorStateFalse = COLOR_COMPARATOR_STATE_FALSE;
	QColor m_colorStateTrue = COLOR_COMPARATOR_STATE_TRUE;

	int m_timeForUpdate = 250; // 250 ms
};

// ==============================================================================================

#define STATISTICS_OPTIONS_KEY "Options/Statistics/"

// ----------------------------------------------------------------------------------------------

class StatisticsOption : public QObject
{
	Q_OBJECT

public:

	explicit StatisticsOption(QObject* parent = nullptr);
	explicit StatisticsOption(const StatisticsOption& from, QObject* parent = nullptr);
	virtual ~StatisticsOption() override;

public:

	QMap<QString, int> columnsWidth() const { return m_columnsWidth; }
	void setColumnsWidth(const QMap<QString, int>& map) { m_columnsWidth = map; }

	//
	//
	void load();
	void save();

	//
	//
	StatisticsOption& operator=(const StatisticsOption& from);

private:

	QMap<QString, int> m_columnsWidth;
};

// ==============================================================================================

#define DATABASE_OPTIONS_REG_KEY "Options/Database/"

// ----------------------------------------------------------------------------------------------

namespace OT
{
	Q_NAMESPACE

	// ----------------------------------------------------------------------------------------------

	enum DatabaseParam
	{
		NoDatabaseParam = -1,
		dbo_Type = 0,
		dbo_LocationPath = 1,
		dbo_Ip = 2,
		dbo_Port = 3,
		dbo_User = 4,
		dbo_Password = 5,
		dbo_OnStart = 6,
		dbo_OnExit = 7,
		dbo_CopyPath = 8,
	};
	Q_ENUM_NS(DatabaseParam)

	const int DatabaseParamCount = 9;

	#define ERR_DBO_PARAM(param) (static_cast<int>(param) < 0 || static_cast<int>(param) >= OT::DatabaseParamCount)

	QString DatabaseParamCaption(DatabaseParam param);

	// ----------------------------------------------------------------------------------------------

	enum DatabaseType
	{
		SQLite = 0,
		PostgreSQL = 1,
	};
	Q_ENUM_NS(DatabaseType)

	const int DatabaseTypeCount = 2;

	#define ERR_DATABASE_TYPE(type) (static_cast<int>(type) < 0 || static_cast<int>(type) >= OT::DatabaseTypeCount)

	QString DatabaseTypeCaption(OT::DatabaseType type);

	// ----------------------------------------------------------------------------------------------
}

// ----------------------------------------------------------------------------------------------

const QString DEFAULT_DB_IP = "localhost";
const int DEFAULT_DB_PORT = 5432;
const QString DEFAULT_DB_USER = "postgres";
const QString DEFAULT_DB_PASSWORD = "password";

// ----------------------------------------------------------------------------------------------

class DatabaseOption : public QObject
{
	Q_OBJECT

public:

	explicit DatabaseOption(QObject* parent = nullptr);
	explicit DatabaseOption(const DatabaseOption& from, QObject* parent = nullptr);
	virtual ~DatabaseOption() override;

public:

	OT::DatabaseType type() const { return m_type; }
	void setType(OT::DatabaseType type) { m_type = type; }

	QString locationPath() const { return m_locationPath; }
	void setLocationPath(const QString& path) { m_locationPath = path; }


	QString ip() const { return m_ip; }
	void setIp(const QString& ip) { m_ip = ip; }

	int port() const { return m_port; }
	void setPort(int port) { m_port = port; }

	QString user() const { return m_user; }
	void setUser(const QString& user) { m_user = user; }

	QString password() const { return m_password; }
	void setPassword(const QString& password) { m_password = password; }


	bool onStart () const { return m_onStart; }
	void setOnStart(bool onStart) { m_onStart = onStart; }

	bool onExit() const { return m_onExit; }
	void setOnExit(bool onExit) { m_onExit = onExit; }

	QString backupPath() const { return m_backupPath; }
	void setBackupPath(const QString& path) { m_backupPath = path; }

	//
	//
	void load();
	void save();

	//
	//
	DatabaseOption& operator=(const DatabaseOption& from);

private:

	QString m_locationPath;
	OT::DatabaseType m_type = OT::SQLite;

	QString m_ip = DEFAULT_DB_IP;
	int m_port = DEFAULT_DB_PORT;
	QString m_user = DEFAULT_DB_IP;
	QString m_password = DEFAULT_DB_IP;

	bool m_onStart = false;
	bool m_onExit = true;
	QString m_backupPath;
};

// ==============================================================================================

#define LANGUAGE_OPTIONS_REG_KEY "Options/Language/"

// ----------------------------------------------------------------------------------------------

#define LANGUAGE_OPTIONS_DIR		"/translations"
#define LANGUAGE_OPTIONS_FILE_RU	"Metrology_ru.qm"
#define LANGUAGE_OPTIONS_FILE_UK	"Metrology_uk.qm"

// ----------------------------------------------------------------------------------------------

namespace OT
{
	Q_NAMESPACE

	enum LanguageParam
	{
		NoLanguageParam = -1,
		lno_LanguageType = 0,
	};
	Q_ENUM_NS(LanguageParam)

	const int LanguageParamCount = 1;

	#define ERR_LNO_PARAM(param) (static_cast<int>(param) < 0 || static_cast<int>(param) >= OT::LanguageParamCount)

	QString LanguageParamCaption(LanguageParam param);
}

// ----------------------------------------------------------------------------------------------

class LanguageOption : public QObject
{
	Q_OBJECT

public:

	explicit LanguageOption(QObject* parent = nullptr);
	explicit LanguageOption(const LanguageOption& from, QObject* parent = nullptr);
	virtual ~LanguageOption() override;

public:

	OT::LanguageType languageType() const { return m_languageType; }
	void setLanguageType(OT::LanguageType type) { m_languageType = type; }

	//
	//
	void load();
	void save();

	//
	//
	LanguageOption& operator=(const LanguageOption& from);

private:

	OT::LanguageType m_languageType = OT::English;
};

// ==============================================================================================

bool compareDouble(double lDouble, double rDouble);

// ==============================================================================================

class Options : public QObject
{
	Q_OBJECT

public:

	explicit Options(QObject* parent = nullptr);
	explicit Options(const Options& from, QObject* parent = nullptr);
	virtual ~Options() override;

public:

	CalibratorsOption& calibrators() { return m_calibrators; }
	void setCalibrators(const CalibratorsOption& calibrators) { m_calibrators = calibrators; }

	SocketOption& socket() { return m_socket; }
	void setSocket(const SocketOption& socket) { m_socket = socket; }

	ProjectInfo& projectInfo() { return m_projectInfo; }
	void setProjectInfo(const ProjectInfo& projectInfo) { m_projectInfo = projectInfo; }

	ModuleOption& module() { return m_module; }
	void setModule(const ModuleOption& module) { m_module = module; }

	LinearityOption& linearity() { return m_linearity; }
	void setLinearity(const LinearityOption& linearity) { m_linearity = linearity; }

	ComparatorOption& comparator() { return m_comparator; }
	void setComparator(const ComparatorOption& comparator) { m_comparator = comparator; }

	ToolBarOption& toolBar() { return m_toolBar; }
	void setToolBar(const ToolBarOption& toolBar) { m_toolBar = toolBar; }

	MeasureViewOption& measureView() { return m_measureView; }
	void setMeasureView(const MeasureViewOption& measureView) { m_measureView = measureView; }

	SignalInfoOption& signalInfo() { return m_signalInfo; }
	void setSignalInfo(const SignalInfoOption& signalInfo) { m_signalInfo = signalInfo; }

	ComparatorInfoOption& comparatorInfo() { return m_comparatorInfo; }
	void setComparatorInfo(const ComparatorInfoOption& comparatorInfo) { m_comparatorInfo = comparatorInfo; }

	StatisticsOption& statistics() { return m_statistics; }
	void setStatistics(const StatisticsOption& statistics) { m_statistics = statistics; }

	DatabaseOption& database() { return m_database; }
	void setDatabase(const DatabaseOption& database) { m_database = database; }

	LanguageOption& language() { return m_language; }
	void setLanguage(const LanguageOption& language) { m_language = language; }

	//
	//
	void load();
	void save();

	bool setMetrologySettings(std::shared_ptr<const SoftwareSettings> curSettingsProfile);
	bool readFromXml(const QByteArray& fileData);

	//
	//
	Options& operator=(const Options& from);

private:

	QMutex m_mutex;

	CalibratorsOption m_calibrators;

	SocketOption m_socket;
	ProjectInfo m_projectInfo;

	MetrologySettings m_settings;

	ModuleOption m_module;
	LinearityOption m_linearity;
	ComparatorOption m_comparator;

	ToolBarOption m_toolBar;
	MeasureViewOption m_measureView;

	SignalInfoOption m_signalInfo;
	ComparatorInfoOption m_comparatorInfo;
	StatisticsOption m_statistics;

	DatabaseOption m_database;

	LanguageOption m_language;
};

// ==============================================================================================

extern Options theOptions;

// ==============================================================================================

#endif // OPTIONS_H
