#ifndef CALIBRATOR_H
#define CALIBRATOR_H

#include <QObject>
#include <QtGlobal>
#include <QtSerialPort/QSerialPort>
#include <QStringList>

// ==============================================================================================

#define			CALIBRATOR_OPTIONS_KEY		"Options/Calibrators/"

// ==============================================================================================

const int		CALIBRATOR_TIMEOUT			= 5000,		// 5 seconds
				CALIBRATOR_TIMEOUT_STEP		= 10;		// 10 milliseconds

// ----------------------------------------------------------------------------------------------

const char* const CalibratorType[] =
{
				QT_TRANSLATE_NOOP("Calibrator.h", "TRX-II"),
				QT_TRANSLATE_NOOP("Calibrator.h", "CALYS-75"),
};

const int		CALIBRATOR_TYPE_COUNT			= sizeof(CalibratorType)/sizeof(CalibratorType[0]);

const int		CALIBRATOR_TYPE_UNKNOWN		= -1,
				CALIBRATOR_TYPE_TRXII		= 0,
				CALIBRATOR_TYPE_CALYS75		= 1;

// ----------------------------------------------------------------------------------------------

const char* const CalibratorMode[] =
{
				QT_TRANSLATE_NOOP("Calibrator.h", "Measure"),
				QT_TRANSLATE_NOOP("Calibrator.h", "Source"),
};

const int		CALIBRATOR_MODE_COUNT			= sizeof(CalibratorMode)/sizeof(CalibratorMode[0]);

const int		CALIBRATOR_MODE_UNKNOWN		= -1,
				CALIBRATOR_MODE_MEASURE		= 0,
				CALIBRATOR_MODE_SOURCE		= 1;

// ----------------------------------------------------------------------------------------------

const char* const CalibratorUnit[] =
{
				QT_TRANSLATE_NOOP("Calibrator.h", "mV"),
				QT_TRANSLATE_NOOP("Calibrator.h", "mA"),
				QT_TRANSLATE_NOOP("Calibrator.h", "V"),
				QT_TRANSLATE_NOOP("Calibrator.h", "kHz"),
				QT_TRANSLATE_NOOP("Calibrator.h", "Ohm (Low)"),
				QT_TRANSLATE_NOOP("Calibrator.h", "Ohm (High)"),
};

const int		CALIBRATOR_UNIT_COUNT		= sizeof(CalibratorUnit)/sizeof(CalibratorUnit[0]);

const int		CALIBRATOR_UNIT_UNKNOWN		= -1,
				CALIBRATOR_UNIT_MV			= 0,
				CALIBRATOR_UNIT_MA			= 1,
				CALIBRATOR_UNIT_V			= 2,
				CALIBRATOR_UNIT_KHZ			= 3,
				CALIBRATOR_UNIT_LOW_OHM		= 4,
				CALIBRATOR_UNIT_HIGH_OHM	= 5;

// ----------------------------------------------------------------------------------------------

const char* const CalibratorStep[] =
{
				QT_TRANSLATE_NOOP("Calibrator.h", "Step down"),
				QT_TRANSLATE_NOOP("Calibrator.h", "Step up"),
};

const int		CALIBRATOR_STEP_COUNT			= sizeof(CalibratorStep)/sizeof(CalibratorStep[0]);

const int		CALIBRATOR_STEP_UNKNOWN		= -1,
				CALIBRATOR_STEP_DOWN		= 0,
				CALIBRATOR_STEP_UP			= 1;

// ----------------------------------------------------------------------------------------------

const char* const CalibratorReset[] =
{
				QT_TRANSLATE_NOOP("Calibrator.h", "Hard reset"),
				QT_TRANSLATE_NOOP("Calibrator.h", "Soft reset"),
};

const int		CALIBRATOR_RESET_COUNT		  = sizeof(CalibratorStep)/sizeof(CalibratorReset[0]);

const int		CALIBRATOR_RESET_UNKNOWN	= -1,
				CALIBRATOR_RESET_HARD		= 0,
				CALIBRATOR_RESET_SOFT		= 1;

// ----------------------------------------------------------------------------------------------

const char* const CalibratorConvert[] =
{
				QT_TRANSLATE_NOOP("Calibrator.h", "Hz to kHz"),
				QT_TRANSLATE_NOOP("Calibrator.h", "kHz to Hz"),
};

const int		CALIBRATOR_CONVERT_COUNT		= sizeof(CalibratorConvert)/sizeof(CalibratorConvert[0]);

const int		CALIBRATOR_CONVERT_HZ_TO_KHZ	= 0,
				CALIBRATOR_CONVERT_KHZ_TO_HZ	= 1;


// ----------------------------------------------------------------------------------------------

const int		CalibratorBaudRate[CALIBRATOR_TYPE_COUNT] =
{
				QSerialPort::Baud9600,
				QSerialPort::Baud115200,
};

// ----------------------------------------------------------------------------------------------

const int		CALIBRATION_TS_AC0			= 0,
				CALIBRATION_TS_AC1			= 1,
				CALIBRATION_TS_RANGE		= 2;

const int		CALIBRATION_TS_COUNT		= 3;

const double	CalibratorTS[CALIBRATOR_TYPE_COUNT][CALIBRATOR_UNIT_COUNT][CALIBRATION_TS_COUNT] =
{
	// CALIBRATION_TYPE_TRXII	= 0,
	{
		{0.01,	0.005,	100},		// CALIBRATION_UNIT_MV			= 0,
		{0.01,	0.02,	24},		// CALIBRATION_UNIT_MA			= 1,
		{0.01,	0.005,	12},		// CALIBRATION_UNIT_V			= 2,
		{0.005,	0.000,	20000},		// CALIBRATION_UNIT_KHZ			= 3,
		{0.005,	0.02,	400},		// CALIBRATION_UNIT_LOW_OHM		= 4,
		{0.02,	0.015,	2000},		// CALIBRATION_UNIT_HIGH_OHM	= 5;
	},

	// CALIBRATION_TYPE_CALYS75	= 1,
	{
		{0.013,	0.003,	100},		// CALIBRATION_UNIT_MV			= 0,
		{0.018,	0.002,	24},		// CALIBRATION_UNIT_MA			= 1,
		{0.015,	0.004,	50},		// CALIBRATION_UNIT_V			= 2,
		{0.005,	0.000,	10000},		// CALIBRATION_UNIT_KHZ			= 3,
		{0.012,	0.0025,	400},		// CALIBRATION_UNIT_LOW_OHM		= 4,
		{0.012,	0.0025,	4000},		// CALIBRATION_UNIT_HIGH_OHM	= 5;
	},
};

// ----------------------------------------------------------------------------------------------

const int		CALIBRATOR_TERMINATOR_LEN = 2;

const char* const CalibratorTerminator[CALIBRATOR_TYPE_COUNT] =
{
				"\n\r",
				"\r\n",
};

// ==============================================================================================

const int		INVALID_CALIBRATOR_CHANNEL = -1;

// ==============================================================================================
// common commands

#define CALIBRATOR_IDN					"*IDN?"
#define CALIBRATOR_RESET				"*RST"
#define CALIBRATOR_BEEP					":SYSTEM:BEEP"


// ==============================================================================================
// Commands - model TRX-II

#define TRXII_RESET_SOFT				":SYSTEM:RESET"

#define TRXII_MEASURE_UNIT_MV			":MEASURE:MV"
#define TRXII_MEASURE_UNIT_MA			":MEASURE:MA"
#define TRXII_MEASURE_UNIT_V			":MEASURE:VOLT"
#define TRXII_MEASURE_UNIT_KHZ			":MEASURE:FREQ Hz,5.00"
#define TRXII_MEASURE_UNIT_OHM			":MEASURE:OHM"

#define TRXII_SOURCE_UNIT_MV			":SOURCE:MV:DIRECT"
#define TRXII_SOURCE_UNIT_MA			":SOURCE:MA:DIRECT"
#define TRXII_SOURCE_UNIT_V				":SOURCE:VOLT:DIRECT"
#define TRXII_SOURCE_UNIT_KHZ			":SOURCE:FREQ 20kHz,10.0"
#define TRXII_SOURCE_UNIT_LOW_OHM		":SOURCE:OHM LO:DIRECT"
#define TRXII_SOURCE_UNIT_HIGH_OHM		":SOURCE:OHM HI:DIRECT"

#define TRXII_GET_VALUE					":SAMPLE"
#define TRXII_SET_VALUE					":PT "

#define TRXII_KEY_UP					":SYSTEM:KEY UP"
#define TRXII_KEY_DOWN					":SYSTEM:KEY DOWN"


// ==============================================================================================
// Commands - model Calys 75

#define CALYS75_RESET_SOFT				"*RST"

#define CALYS75_MEASURE_UNIT_MV			"SENS:FUNC VOLT"
#define CALYS75_MEASURE_RANG_MV			"SENS:VOLT:RANG 100mV"
#define CALYS75_MEASURE_UNIT_MA			"SENS:FUNC CURR"
#define CALYS75_MEASURE_RANG_MA			"SENS:CURR:RANG 50mA"
#define CALYS75_MEASURE_UNIT_V			"SENS:FUNC VOLT"
#define CALYS75_MEASURE_RANG_V			"SENS:VOLT:RANG 50V"
#define CALYS75_MEASURE_UNIT_KHZ		"SENS:FUNC FREQ"
#define CALYS75_MEASURE_RANG_KHZ		"SENS:FREQ:RANG 10KHZ"
#define CALYS75_MEASURE_UNIT_OHM		"SENS:FUNC RES"
#define CALYS75_MEASURE_LOW_RANG_OHM	"SENS:RES:RANG 400"
#define CALYS75_MEASURE_HIGH_RANG_OHM	"SENS:RES:RANG 4000"

#define CALYS75_SOURCE_UNIT_MV			"SOUR:FUNC VOLT"
#define CALYS75_SOURCE_RANG_MV			"SOUR:VOLT:RANG 100mV"
#define CALYS75_SOURCE_UNIT_MA			"SOUR:FUNC CURR"
#define CALYS75_SOURCE_RANG_MA			"SOUR:CURR:RANG 24mA"
#define CALYS75_SOURCE_UNIT_V			"SOUR:FUNC VOLT"
#define CALYS75_SOURCE_RANG_V			"SOUR:VOLT:RANG 50V"
#define CALYS75_SOURCE_UNIT_KHZ			"SOUR:FUNC FREQ"
#define CALYS75_SOURCE_RANG_KHZ			"SOUR:FREQ:RANG 10KHZ"
#define CALYS75_SOURCE_UNIT_OM			"SOUR:FUNC RES"
#define CALYS75_SOURCE_LOW_RANG_OHM		"SOUR:RES:RANG 400,1mA"
#define CALYS75_SOURCE_HIGH_RANG_OHM	"SOUR:RES:RANG 4000,1mA"

#define CALYS75_GET_VALUE				"DISP?"
#define CALYS75_SET_VALUE				"SOUR "

#define CALYS75_REMOTE_CONTROL			"REM"
#define CALYS75_MANUAL_CONTROL			"LOC"

// ==============================================================================================

class Calibrator : public QObject
{
	Q_OBJECT

public:

	explicit Calibrator(int channel = INVALID_CALIBRATOR_CHANNEL, QObject *parent = nullptr);
	virtual ~Calibrator();

private:

	bool		m_connected = false;

	int			m_channel = INVALID_CALIBRATOR_CHANNEL;									// index calibrator in a common base calibrators CalibratorBase

	QSerialPort m_port;																	// object serial port for management of the calibrator

	QString		m_portName;																// string containing the name of the serial port
	int			m_type = CALIBRATOR_TYPE_UNKNOWN;										// calibrator type: 0 - CALIBRATOR_TYPE_TRXII or 1 - CALIBRATOR_TYPE_CALYS75
	QString		m_caption;																// name of calibrator
	QString		m_serialNo;																// serial number of calibrator

	int			m_timeout = 0;															// time counter waits for a response from the calibrator

	int			m_mode = CALIBRATOR_MODE_UNKNOWN;										// calibrator mode: 0 - CALIBRATOR_MODE_MEASURE or 1 - CALIBRATOR_MODE_SOURCE
	int			m_measureUnit = 0;														// measure unit: mA, mV and etc.
	int			m_sourceUnit = 0;														// source unit: mA, mV and etc.

	double		m_measureValue = 0;														// contains measured electrical value of the calibrator
	double		m_sourceValue = 0;														// contains installed electrical value of the calibrator

	QString		m_lastResponse;															// string containing the last response data from the calibrator
	QString		m_lastError;															// in the case of an error of the calibrator, this string contains the description

	bool		m_enableWaitResponse = false;											// enbale wait response from calibrator after open port

	bool		m_busy = false;

	void		clear();																// erases all information on the calibrator: SerialNo, Name and etc.

	void		setConnected(bool connect);												// function changes status calibrator: connected or disconnected

	bool		openPort();																// open the serial port to manage the calibrator
	bool		getIDN();																// identify the calibrator, get: SerialNo, Type and etc.

	bool		send(QString cmd);														// sending commands to the calibrator
	bool		recv();																	// receiving a response from the calibrator

	void		parseResponse();														// extracts from the string of the last response from the calibrator current electrical values
	void		convert(double& val, int mode, int order);								// translation from Kilo to Mega, from Hz to kHz, etc.

public:

	bool		isConnected() const { return m_connected; }

	int			channel() const{ return m_channel; }

	bool		portIsOpen() const { return m_port.isOpen(); }

	QString		portName() const { return m_portName; }
	void		setPortName(const QString& portName) { m_portName = portName; }

	int			type() const { return m_type; }
	QString		typeStr() const;
	void		setType(int type) { m_type = type; }

	QString		caption() const { return m_caption; }
	QString		serialNo() const { return m_serialNo; }

	int			timeout() const { return m_timeout;	}

	int			mode() const { return m_mode; }
	int			measureUnit() const { return m_measureUnit; }
	int			sourceUnit() const { return m_sourceUnit; }

	double		measureValue() const { return m_measureValue; }
	double		sourceValue() const { return m_sourceValue; }

	QString		lastError() const { return m_lastError; }

	void		setWaitResponse(bool enable) { m_enableWaitResponse = enable; }

	bool		isBusy() const { return m_busy; }
	void		setBusy(bool busy);

signals:

	void		connected();
	void		disconnected();

	void		unitIsChanged();
	void		commandIsSent(QString);
	void		responseIsReceived(QString);
	void		valueIsRequested();
	void		valueIsReceived();

	void		error(QString);

public slots:

	bool		open();																	// initialization of the calibrator

	bool		setUnit(int mode, int unit);											// select mode: measure - 0 (CALIBRATOR_MODE_MEASURE) or source - 1 (CALIBRATOR_MODE_SOURCE)
																						// select unit: mA, mV and etc.
	bool		setValue(double value);													// set value
	bool		stepDown();																// decrease the value on the calibrator
	bool		stepUp();																// increasing the value on the calibrator
	bool		step(int stepType);														// imitation of the "step"

	double		getValue();																// get electrical values ​​with calibrator

	bool		beep();																	// beep
	bool		reset(int resetType);	  												// reset: hard: 0 - CALIBRATOR_RESET_HARD or soft: 1 - CALIBRATOR_RESET_SOFT

	bool		setRemoteControl(bool enable);											// allow remote control of the calibrator (only model CALYS75)

	void		close();																// the end of the session with a calibrator
};

// ==============================================================================================

#endif // CALIBRATOR_H
