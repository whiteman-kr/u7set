#ifndef CALIBRATOR_H
#define CALIBRATOR_H

#include <QObject>
#include <QtGlobal>
#include <QtSerialPort/QSerialPort>
#include <QStringList>

// ==============================================================================================

const char* const CalibratorType[] =
{
				"TRX-II",
				"CALYS-75",
				"KEITHLEY-6221",
};

const int		CALIBRATOR_TYPE_COUNT			= sizeof(CalibratorType)/sizeof(CalibratorType[0]);

const int		CALIBRATOR_TYPE_UNDEFINED	= -1,
				CALIBRATOR_TYPE_TRXII		= 0,
				CALIBRATOR_TYPE_CALYS75		= 1,
				CALIBRATOR_TYPE_KTHL6221	= 2;

// ----------------------------------------------------------------------------------------------

const char* const CalibratorMode[] =
{
				QT_TRANSLATE_NOOP("Calibrator.h", "Measure"),
				QT_TRANSLATE_NOOP("Calibrator.h", "Source"),
};

const int		CALIBRATOR_MODE_COUNT		= sizeof(CalibratorMode)/sizeof(CalibratorMode[0]);

const int		CALIBRATOR_MODE_UNDEFINED	= -1,
				CALIBRATOR_MODE_MEASURE		= 0,
				CALIBRATOR_MODE_SOURCE		= 1;

// ----------------------------------------------------------------------------------------------

const char* const CalibratorUnit[] =
{
				QT_TRANSLATE_NOOP("Calibrator.h", "mV"),
				QT_TRANSLATE_NOOP("Calibrator.h", "mA"),
				QT_TRANSLATE_NOOP("Calibrator.h", "μA (micro)"),
				QT_TRANSLATE_NOOP("Calibrator.h", "nA"),
				QT_TRANSLATE_NOOP("Calibrator.h", "V"),
				QT_TRANSLATE_NOOP("Calibrator.h", "kHz"),
				QT_TRANSLATE_NOOP("Calibrator.h", "Ohm (Low)"),
				QT_TRANSLATE_NOOP("Calibrator.h", "Ohm (High)"),
};

const int		CALIBRATOR_UNIT_COUNT		= sizeof(CalibratorUnit)/sizeof(CalibratorUnit[0]);

const int		CALIBRATOR_UNIT_UNDEFINED	= -1,
				CALIBRATOR_UNIT_MV			= 0,
				CALIBRATOR_UNIT_MA			= 1,
				CALIBRATOR_UNIT_McrA		= 2,
				CALIBRATOR_UNIT_NA			= 3,
				CALIBRATOR_UNIT_V			= 4,
				CALIBRATOR_UNIT_KHZ			= 5,
				CALIBRATOR_UNIT_LOW_OHM		= 6,
				CALIBRATOR_UNIT_HIGH_OHM	= 7;

// ----------------------------------------------------------------------------------------------
// Minimal range for calibrators TRX-II and Calys75 this is 400 Ohm
//
const double	CALIBRATOR_MINIMAL_RANGE_OHM	= 400;

// ----------------------------------------------------------------------------------------------

const int		DEFAULT_ECLECTRIC_UNIT_PRECESION = 4;

// ----------------------------------------------------------------------------------------------

const int		CALIBRATOR_TIMEOUT			= 8000,		// 8 seconds
				CALIBRATOR_TIMEOUT_STEP		= 10;		// 10 milliseconds

// ----------------------------------------------------------------------------------------------

const int		INVALID_CALIBRATOR_CHANNEL = -1;

// ==============================================================================================
// common commands

#define CALIBRATOR_IDN					"*IDN?"
#define CALIBRATOR_RESET				"*RST"
#define CALIBRATOR_BEEP					":SYSTEM:BEEP"

// ==============================================================================================
// Commands - model TRX-II

#define TRXII_KEY_UP					":SYSTEM:KEY UP"
#define TRXII_KEY_DOWN					":SYSTEM:KEY DOWN"

// ==============================================================================================
// Commands - model Calys 75

#define CALYS75_REMOTE_CONTROL			"REM"
#define CALYS75_MANUAL_CONTROL			"LOC"

// ==============================================================================================
// Commands - model KEITHLEY-6221

#define KTHL6221_OUTPUT_ON				"OUTP ON"
#define KTHL6221_OUTPUT_OFF				"OUTP OFF"

#define KTHL6221_LIMIT_FOR_SWITCH		21

// ==============================================================================================

struct CalibratorParam
{
	bool isValid() const;

	int type = CALIBRATOR_TYPE_UNDEFINED;

	int baudRate = 0;

	QString cmdGetValue;
	QString cmdSetValue;

	QString terminamtor;
};

const CalibratorParam CalibratorParams[CALIBRATOR_TYPE_COUNT] =
{
	{ CALIBRATOR_TYPE_TRXII,	QSerialPort::Baud9600,		":SAMPLE",	":PT ",		"\n\r"	},
	{ CALIBRATOR_TYPE_CALYS75,	QSerialPort::Baud115200,	"DISP?",	"SOUR ",	"\r\n"	},
	{ CALIBRATOR_TYPE_KTHL6221,	QSerialPort::Baud9600,		"CURR?",	"CURR ",	"\n\r"	},
};

// ==============================================================================================

struct CalibratorLimit
{
	bool isValid() const;

	int type = CALIBRATOR_TYPE_UNDEFINED;
	int mode = CALIBRATOR_MODE_UNDEFINED;

	double lowLimit = 0;
	double highLimit = 0;
	int unit = CALIBRATOR_UNIT_UNDEFINED;
	int precesion = DEFAULT_ECLECTRIC_UNIT_PRECESION;

	double ac0 = 0;
	double ac1 = 0;

	QString cmdSetUnit;
};

const CalibratorLimit CalibratorLimits[] =
{
	// TRX-II
	//
		// Measure
		//
	{ CALIBRATOR_TYPE_TRXII,	CALIBRATOR_MODE_MEASURE,	 0,		100,	CALIBRATOR_UNIT_MV,			3,		0.02,	0.01	,":MEASURE:MV"},
	{ CALIBRATOR_TYPE_TRXII,	CALIBRATOR_MODE_MEASURE,	 0,		 60,	CALIBRATOR_UNIT_V,			3,		0.05,	0.005	,":MEASURE:VOLT"},
	{ CALIBRATOR_TYPE_TRXII,	CALIBRATOR_MODE_MEASURE,	 0,		 52,	CALIBRATOR_UNIT_MA,			3,		0.01,	0.01	,":MEASURE:MA"},
	{ CALIBRATOR_TYPE_TRXII,	CALIBRATOR_MODE_MEASURE,	 0,		400,	CALIBRATOR_UNIT_LOW_OHM,	2,		0.05,	0.02	,":MEASURE:OHM"},
	{ CALIBRATOR_TYPE_TRXII,	CALIBRATOR_MODE_MEASURE,	 0,	   2000,	CALIBRATOR_UNIT_HIGH_OHM,	1,		0.02,	0.015	,":MEASURE:OHM"},

		// Source
		//
	{ CALIBRATOR_TYPE_TRXII,	CALIBRATOR_MODE_SOURCE,		-10,	100,	CALIBRATOR_UNIT_MV,			3,		0.01,	0.005	,":SOURCE:MV:DIRECT"},
	{ CALIBRATOR_TYPE_TRXII,	CALIBRATOR_MODE_SOURCE,		  0,	 12,	CALIBRATOR_UNIT_V,			3,		0.01,	0.005	,":SOURCE:VOLT:DIRECT"},
	{ CALIBRATOR_TYPE_TRXII,	CALIBRATOR_MODE_SOURCE,		  0,	 24,	CALIBRATOR_UNIT_MA,			3,		0.01,	0.02	,":SOURCE:MA:DIRECT"},
	{ CALIBRATOR_TYPE_TRXII,	CALIBRATOR_MODE_SOURCE,		  0,	400,	CALIBRATOR_UNIT_LOW_OHM,	2,		0.005,	0.02	,":SOURCE:OHM LO:DIRECT"},
	{ CALIBRATOR_TYPE_TRXII,	CALIBRATOR_MODE_SOURCE,		  0,   2000,	CALIBRATOR_UNIT_HIGH_OHM,	1,		0.02,	0.015	,":SOURCE:OHM HI:DIRECT"},


	// Calys 75
	//
		// Measure
		//
	{ CALIBRATOR_TYPE_CALYS75,	CALIBRATOR_MODE_MEASURE,	 0,		100,	CALIBRATOR_UNIT_MV,			3,		0.013,	0.003	,"SENS:FUNC VOLT\r\nSENS:VOLT:RANG 100mV"},			// 0.013% +- 3 microV - i.e.  3 microV * 100% / 100 mV = 0.003%
	{ CALIBRATOR_TYPE_CALYS75,	CALIBRATOR_MODE_MEASURE,	 0,		 50,	CALIBRATOR_UNIT_V,			3,		0.015,	0.004	,"SENS:FUNC VOLT\r\nSENS:VOLT:RANG 50V"},			// 0.015% +- 2 mV - i.e.  2 mV * 100% / 50 V = 0.004%
	{ CALIBRATOR_TYPE_CALYS75,	CALIBRATOR_MODE_MEASURE,	 0,		 50,	CALIBRATOR_UNIT_MA,			3,		0.018,	0.004	,"SENS:FUNC CURR\r\nSENS:CURR:RANG 50mA"},			// 0.018% +- 2 microA - i.e.  2 microA * 100% / 50 mA = 0.004%
	{ CALIBRATOR_TYPE_CALYS75,	CALIBRATOR_MODE_MEASURE,	 0,		400,	CALIBRATOR_UNIT_LOW_OHM,	2,		0.012,	0.0025	,"SENS:FUNC RES\r\nSENS:RES:RANG 400"},				// 0.012% +- 10 mOhm - i.e.  10 mOhm * 100% / 400 Ohm = 0.0025%
	{ CALIBRATOR_TYPE_CALYS75,	CALIBRATOR_MODE_MEASURE,	 0,	   4000,	CALIBRATOR_UNIT_HIGH_OHM,	1,		0.012,	0.0025	,"SENS:FUNC RES\r\nSENS:RES:RANG 4000"},			// 0.012% +- 100 mOhm - i.e.  100 mOhm * 100% / 4000 Ohm = 0.0025%

		// Source
		//
	{ CALIBRATOR_TYPE_CALYS75,	CALIBRATOR_MODE_SOURCE,		  0,	100,	CALIBRATOR_UNIT_MV,			3,		0.013,	0.003	,"SOUR:FUNC VOLT\r\nSOUR:VOLT:RANG 100mV"},			// 0.013% +- 3 microV - i.e.  3 microV * 100% / 100 mV = 0.003%
	{ CALIBRATOR_TYPE_CALYS75,	CALIBRATOR_MODE_SOURCE,		  0,	 50,	CALIBRATOR_UNIT_V,			3,		0.015,	0.004	,"SOUR:FUNC VOLT\r\nSOUR:VOLT:RANG 50V"},			// 0.015% +- 2 mV - i.e.  2 mV * 100% / 50 V = 0.004%
	{ CALIBRATOR_TYPE_CALYS75,	CALIBRATOR_MODE_SOURCE,		  0,	 24,	CALIBRATOR_UNIT_MA,			3,		0.018,	0.0083	,"SOUR:FUNC CURR\r\nSOUR:CURR:RANG 24mA"},			// 0.018% +- 2 microA - i.e.  2 microA * 100% / 24 mA = 0.008%
	{ CALIBRATOR_TYPE_CALYS75,	CALIBRATOR_MODE_SOURCE,		  0,	400,	CALIBRATOR_UNIT_LOW_OHM,	2,		0.014,	0.0075	,"SOUR:FUNC RES\r\nSOUR:RES:RANG 400,1mA"},			// 0.014% +- 30 mOhm - i.e.  30 mOhm * 100% / 400 Ohm = 0.0075%
	{ CALIBRATOR_TYPE_CALYS75,	CALIBRATOR_MODE_SOURCE,		  0,   4000,	CALIBRATOR_UNIT_HIGH_OHM,	1,		0.014,	0.0075	,"SOUR:FUNC RES\r\nSOUR:RES:RANG 4000,1mA"},		// 0.014% +- 300 mOhm - i.e.  300 mOhm * 100% / 4000 Ohm = 0.0075%

	// KEITHLEY-6221
	//
		// Source
		//
	{ CALIBRATOR_TYPE_KTHL6221,	CALIBRATOR_MODE_SOURCE,		0,		 20,	CALIBRATOR_UNIT_MA,			3,		0.050,	0.0500	,"CURR:RANG:AUTO ON\n\rCURR 1e-3"},					// 0.05% +- 10 microA - i.e.  10 microA * 100% / 20 mА = 0.05%
	{ CALIBRATOR_TYPE_KTHL6221,	CALIBRATOR_MODE_SOURCE,		0,		 20,	CALIBRATOR_UNIT_McrA,		3,		0.050,	0.0500	,"CURR:RANG:AUTO ON\n\rCURR 1e-6"},					// 0.05% +- 10 nanoA - i.e.  10 nanoA * 100% / 20 microА = 0.05%
	{ CALIBRATOR_TYPE_KTHL6221,	CALIBRATOR_MODE_SOURCE,		0,		 20,	CALIBRATOR_UNIT_NA,			3,		0.300,	0.0500	,"CURR:RANG:AUTO ON\n\rCURR 1e-9"},					// 0.05% +- 10 picoA - i.e.  10 picoA * 100% / 20 nanoА = 0.05%
};

const int CalibratorLimitCount = sizeof(CalibratorLimits) / sizeof(CalibratorLimits[0]);

// ==============================================================================================

class Calibrator : public QObject
{
	Q_OBJECT

public:

	explicit Calibrator(int channel = INVALID_CALIBRATOR_CHANNEL, QObject *parent = nullptr);
	virtual ~Calibrator();

private:

	int m_channel = INVALID_CALIBRATOR_CHANNEL;											// index calibrator in a common base calibrators CalibratorBase

	QSerialPort m_port;																	// object serial port for management of the calibrator
	QString m_portName;																	// string containing the name of the serial port

	bool m_connected = false;

	int m_type = CALIBRATOR_TYPE_UNDEFINED;												// calibrator type: 0 - CALIBRATOR_TYPE_TRXII or 1 - CALIBRATOR_TYPE_CALYS75
	QString m_caption;																	// name of calibrator
	QString m_serialNo;																	// serial number of calibrator

	int m_timeout = 0;																	// time counter waits for a response from the calibrator

	int m_mode = CALIBRATOR_MODE_UNDEFINED;												// calibrator mode: 0 - CALIBRATOR_MODE_MEASURE or 1 - CALIBRATOR_MODE_SOURCE
	int m_measureUnit = 0;																// measure unit: mA, mV and etc.
	int m_sourceUnit = 0;																// source unit: mA, mV and etc.

	double m_measureValue = 0;															// contains measured electrical value of the calibrator
	double m_sourceValue = 0;															// contains installed electrical value of the calibrator

	QString m_lastResponse;																// string containing the last response data from the calibrator
	QString m_lastError;																// in the case of an error of the calibrator, this string contains the description

	bool m_enableWaitResponse = false;													// enbale wait response from calibrator after open port

	bool m_busy = false;

	void clear();																		// erases all information on the calibrator: SerialNo, Name and etc.

	void setConnected(bool connect);													// function changes status calibrator: connected or disconnected

	bool openPort();																	// open the serial port to manage the calibrator
	bool getIDN();																		// identify the calibrator, get: SerialNo, Type and etc.

	CalibratorParam getParam(int type);													// get claibtator param by type

	bool send(QString cmd);																// sending commands to the calibrator
	bool recv();																		// receiving a response from the calibrator

	void parseResponse();																// extracts from the string of the last response from the calibrator current electrical values

public:

	CalibratorLimit getLimit(int mode, int unit);										// get claibtator limit by mode and unit
	CalibratorLimit currentMeasureLimit();												// get claibtator limit
	CalibratorLimit currentSourceLimit();												// get claibtator limit

	bool isConnected() const { return m_connected; }

	int channel() const{ return m_channel; }

	bool portIsOpen() const { return m_port.isOpen(); }

	QString portName() const { return m_portName; }
	void setPortName(const QString& portName) { m_portName = portName; }

	int type() const { return m_type; }
	QString typeStr() const;
	void setType(int type) { m_type = type; }

	QString caption() const { return m_caption; }
	QString serialNo() const { return m_serialNo; }

	int timeout() const { return m_timeout;	}

	int mode() const { return m_mode; }
	int measureUnit() const { return m_measureUnit; }
	int sourceUnit() const { return m_sourceUnit; }

	double measureValue() const { return m_measureValue; }
	double sourceValue() const { return m_sourceValue; }

	QString lastError() const { return m_lastError; }

	void setWaitResponse(bool enable) { m_enableWaitResponse = enable; }

	bool isBusy() const { return m_busy; }
	void setBusy(bool busy);

signals:

	void connected();
	void disconnected();

	void unitIsChanged();
	void commandIsSent(QString);
	void responseIsReceived(QString);
	void valueIsRequested();
	void valueIsReceived();

	void error(QString);

public slots:

	bool open();																	// initialization of the calibrator

	bool setUnit(int mode, int unit);												// select mode: measure - 0 (CALIBRATOR_MODE_MEASURE) or source - 1 (CALIBRATOR_MODE_SOURCE)
																					// select unit: mA, mV and etc.
	bool setValue(double value);													// set value
	bool stepDown();																// decrease the value on the calibrator
	bool stepUp();																	// increasing the value on the calibrator

	double getValue();																// get electrical values ​​with calibrator

	bool beep();																	// beep
	bool reset();																	// reset

	bool setRemoteControl(bool enable);												// allow remote control of the calibrator

	void close();																	// the end of the session with a calibrator
};

// ==============================================================================================

#endif // CALIBRATOR_H
