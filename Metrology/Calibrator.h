#ifndef CALIBRATOR_H
#define CALIBRATOR_H

#include <QObject>
#include <QtGlobal>
#include <QtSerialPort/QSerialPort>
#include <QStringList>

// ==============================================================================================

enum CalibratorType
{
	NoType	= -1,
	TrxII	= 0,
	Calys75	= 1,
	Ktl6221	= 2,
};

const int CalibratorTypeCount = 3;

#define ERR_CALIBRATOR_TYPE(type) (static_cast<int>(type) < 0 || static_cast<int>(type) >= CalibratorTypeCount)

QString CalibratorTypeCaption(int сalibratorType);
QString CalibratorIdnCaption(int сalibratorType);

// ----------------------------------------------------------------------------------------------

enum CalibratorMode
{
	NoMode	= -1,
	Measure	= 0,
	Source	= 1,
};

const int CalibratorModeCount = 2;

#define ERR_CALIBRATOR_MODE(mode) (static_cast<int>(mode) < 0 || static_cast<int>(mode) >= CalibratorModeCount)

QString CalibratorModeCaption(int сalibratorType);

// ----------------------------------------------------------------------------------------------

enum CalibratorUnit
{
	NoUnit	= -1,
	mV		= 0,
	mA		= 1,
	uA		= 2,
	nA		= 3,
	V		= 4,
	Hz		= 5,
	OhmLow	= 6,
	OhmHigh	= 7,
};

const int CalibratorUnitCount = 8;

#define ERR_CALIBRATOR_UNIT(unit) (static_cast<int>(unit) < 0 || static_cast<int>(unit) >= CalibratorUnitCount)

QString CalibratorUnitCaption(int сalibratorUnit);

// ----------------------------------------------------------------------------------------------
// Minimal range for calibrators TRX-II and Calys75 this is 400 Ohm
//
const double	CalibratorMinimalRangeOhm	= 400;

// ----------------------------------------------------------------------------------------------

const int		DefaultElectricUnitPrecesion = 4;

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

	int type = CalibratorType::NoType;

	int baudRate = 0;

	QString cmdGetValue;
	QString cmdSetValue;

	QString terminamtor;
};

const CalibratorParam CalibratorParams[CalibratorTypeCount] =
{
	{ CalibratorType::TrxII,	QSerialPort::Baud9600,		":SAMPLE",	":PT ",		"\n\r"	},
	{ CalibratorType::Calys75,	QSerialPort::Baud115200,	"DISP?",	"SOUR ",	"\r\n"	},
	{ CalibratorType::Ktl6221,	QSerialPort::Baud9600,		"CURR?",	"CURR ",	"\n\r"	},
};

// ==============================================================================================

struct CalibratorLimit
{
	bool isValid() const;

	int type = CalibratorType::NoType;
	int mode = CalibratorMode::NoMode;

	double lowLimit = 0;
	double highLimit = 0;
	int unit = CalibratorUnit::NoUnit;
	int precesion = DefaultElectricUnitPrecesion;

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
	{ CalibratorType::TrxII,	CalibratorMode::Measure,	0,	 100,	CalibratorUnit::mV,			3,	0.02,	0.01	,":MEASURE:MV"},
	{ CalibratorType::TrxII,	CalibratorMode::Measure,	0,	  60,	CalibratorUnit::V,			4,	0.05,	0.005	,":MEASURE:VOLT"},
	{ CalibratorType::TrxII,	CalibratorMode::Measure,	0,	  52,	CalibratorUnit::mA,			3,	0.01,	0.01	,":MEASURE:MA"},
	{ CalibratorType::TrxII,	CalibratorMode::Measure,	0, 20000,	CalibratorUnit::Hz,			0,	0.00,	0.00	,":MEASURE:FREQ Hz,5.00"},
	{ CalibratorType::TrxII,	CalibratorMode::Measure,	0,	 400,	CalibratorUnit::OhmLow,		2,	0.05,	0.02	,":MEASURE:OHM"},
	{ CalibratorType::TrxII,	CalibratorMode::Measure,	0,  2000,	CalibratorUnit::OhmHigh,	1,	0.02,	0.015	,":MEASURE:OHM"},

		// Source
		//
	{ CalibratorType::TrxII,	CalibratorMode::Source,		0,	 100,	CalibratorUnit::mV,			3,	0.01,	0.005	,":SOURCE:MV:DIRECT"},
	{ CalibratorType::TrxII,	CalibratorMode::Source,		0,	  12,	CalibratorUnit::V,			4,	0.01,	0.005	,":SOURCE:VOLT:DIRECT"},
	{ CalibratorType::TrxII,	CalibratorMode::Source,		0,	  24,	CalibratorUnit::mA,			3,	0.01,	0.02	,":SOURCE:MA:DIRECT"},
	{ CalibratorType::TrxII,	CalibratorMode::Source,		0, 20000,	CalibratorUnit::Hz,			0,	0.00,	0.00	,":SOURCE:FREQ 20kHz,10.0"},
	{ CalibratorType::TrxII,	CalibratorMode::Source,		0,	 400,	CalibratorUnit::OhmLow,		2,	0.005,	0.02	,":SOURCE:OHM LO:DIRECT"},
	{ CalibratorType::TrxII,	CalibratorMode::Source,		0,  2000,	CalibratorUnit::OhmHigh,	1,	0.02,	0.015	,":SOURCE:OHM HI:DIRECT"},


	// Calys 75
	//
		// Measure
		//
	{ CalibratorType::Calys75,	CalibratorMode::Measure,	0,	 100,	CalibratorUnit::mV,			3,	0.013,	0.003	,"SENS:FUNC VOLT\r\nSENS:VOLT:RANG 100mV"},			// 0.013% +- 3 microV - i.e.  3 microV * 100% / 100 mV = 0.003%
	{ CalibratorType::Calys75,	CalibratorMode::Measure,	0,	  50,	CalibratorUnit::V,			3,	0.015,	0.004	,"SENS:FUNC VOLT\r\nSENS:VOLT:RANG 50V"},			// 0.015% +- 2 mV - i.e.  2 mV * 100% / 50 V = 0.004%
	{ CalibratorType::Calys75,	CalibratorMode::Measure,	0,	  50,	CalibratorUnit::mA,			3,	0.018,	0.004	,"SENS:FUNC CURR\r\nSENS:CURR:RANG 50mA"},			// 0.018% +- 2 microA - i.e.  2 microA * 100% / 50 mA = 0.004%
	{ CalibratorType::Calys75,	CalibratorMode::Measure,	0, 20000,	CalibratorUnit::Hz,			0,	0.000,	0.000	,"SENS:FUNC FREQ\r\nSENS:FREQ:RANG 20KHZ"},			//
	{ CalibratorType::Calys75,	CalibratorMode::Measure,	0,	 400,	CalibratorUnit::OhmLow,		2,	0.012,	0.0025	,"SENS:FUNC RES\r\nSENS:RES:RANG 400"},				// 0.012% +- 10 mOhm - i.e.  10 mOhm * 100% / 400 Ohm = 0.0025%
	{ CalibratorType::Calys75,	CalibratorMode::Measure,	0,  4000,	CalibratorUnit::OhmHigh,	1,	0.012,	0.0025	,"SENS:FUNC RES\r\nSENS:RES:RANG 4000"},			// 0.012% +- 100 mOhm - i.e.  100 mOhm * 100% / 4000 Ohm = 0.0025%

		// Source
		//
	{ CalibratorType::Calys75,	CalibratorMode::Source,		0,	 100,	CalibratorUnit::mV,			3,	0.013,	0.003	,"SOUR:FUNC VOLT\r\nSOUR:VOLT:RANG 100mV"},			// 0.013% +- 3 microV - i.e.  3 microV * 100% / 100 mV = 0.003%
	{ CalibratorType::Calys75,	CalibratorMode::Source,		0,	  50,	CalibratorUnit::V,			3,	0.015,	0.004	,"SOUR:FUNC VOLT\r\nSOUR:VOLT:RANG 50V"},			// 0.015% +- 2 mV - i.e.  2 mV * 100% / 50 V = 0.004%
	{ CalibratorType::Calys75,	CalibratorMode::Source,		0,	  24,	CalibratorUnit::mA,			3,	0.018,	0.0083	,"SOUR:FUNC CURR\r\nSOUR:CURR:RANG 24mA"},			// 0.018% +- 2 microA - i.e.  2 microA * 100% / 24 mA = 0.008%
	{ CalibratorType::Calys75,	CalibratorMode::Source,		0, 10000,	CalibratorUnit::Hz,			0,	0.000,	0.000	,"SOUR:FUNC FREQ\r\nSOUR:FREQ:RANG 10KHZ"},			//
	{ CalibratorType::Calys75,	CalibratorMode::Source,		0,	 400,	CalibratorUnit::OhmLow,		2,	0.014,	0.0075	,"SOUR:FUNC RES\r\nSOUR:RES:RANG 400,1mA"},			// 0.014% +- 30 mOhm - i.e.  30 mOhm * 100% / 400 Ohm = 0.0075%
	{ CalibratorType::Calys75,	CalibratorMode::Source,		0,  4000,	CalibratorUnit::OhmHigh,	1,	0.014,	0.0075	,"SOUR:FUNC RES\r\nSOUR:RES:RANG 4000,1mA"},		// 0.014% +- 300 mOhm - i.e.  300 mOhm * 100% / 4000 Ohm = 0.0075%

	// KEITHLEY-6221
	//
		// Source
		//
	{ CalibratorType::Ktl6221,	CalibratorMode::Source,		0,	 20,	CalibratorUnit::mA,			3,	0.050,	0.0500	,"CURR:RANG:AUTO ON\n\rCURR 1e-3"},					// 0.05% +- 10 microA - i.e.  10 microA * 100% / 20 mА = 0.05%
	{ CalibratorType::Ktl6221,	CalibratorMode::Source,		0,	 20,	CalibratorUnit::uA,			3,	0.050,	0.0500	,"CURR:RANG:AUTO ON\n\rCURR 1e-6"},					// 0.05% +- 10 nanoA - i.e.  10 nanoA * 100% / 20 microА = 0.05%
	{ CalibratorType::Ktl6221,	CalibratorMode::Source,		0,	 20,	CalibratorUnit::nA,			3,	0.300,	0.0500	,"CURR:RANG:AUTO ON\n\rCURR 1e-9"},					// 0.05% +- 10 picoA - i.e.  10 picoA * 100% / 20 nanoА = 0.05%
};

const int CalibratorLimitCount = sizeof(CalibratorLimits) / sizeof(CalibratorLimits[0]);

// ==============================================================================================

class Calibrator : public QObject
{
	Q_OBJECT

public:

	explicit Calibrator(int channel = INVALID_CALIBRATOR_CHANNEL, QObject* parent = nullptr);
	virtual ~Calibrator();

public:

	CalibratorLimit getLimit(CalibratorMode mode, CalibratorUnit unit);					// get claibtator limit by mode and unit
	CalibratorLimit currentMeasureLimit();												// get claibtator limit
	CalibratorLimit currentSourceLimit();												// get claibtator limit

	bool isConnected() const { return m_connected; }

	int channel() const{ return m_channel; }

	bool portIsOpen() const { return m_port.isOpen(); }

	QString portName() const { return m_portName; }
	void setPortName(const QString& portName) { m_portName = portName; }

	CalibratorType type() const { return m_type; }
	QString typeStr() const;
	void setType(CalibratorType type) { m_type = type; }
	void setType(int type) { m_type = static_cast<CalibratorType>(type); }

	QString caption() const { return m_caption; }
	QString serialNo() const { return m_serialNo; }

	int timeout() const { return m_timeout;	}

	CalibratorMode mode() const { return m_mode; }
	CalibratorUnit measureUnit() const { return m_measureUnit; }
	CalibratorUnit sourceUnit() const { return m_sourceUnit; }

	double measureValue() const { return m_measureValue; }
	double sourceValue() const { return m_sourceValue; }

	QString lastError() const { return m_lastError; }

	void setWaitResponse(bool enable) { m_enableWaitResponse = enable; }

	bool isBusy() const { return m_busy; }
	void setBusy(bool busy);

private:

	int m_channel = INVALID_CALIBRATOR_CHANNEL;											// index calibrator in a common base calibrators CalibratorBase

	QSerialPort m_port;																	// object serial port for management of the calibrator
	QString m_portName;																	// string containing the name of the serial port

	bool m_connected = false;

	CalibratorType m_type = CalibratorType::NoType;										// calibrator type: 0 - CalibratorType::TrxII or 1 - CalibratorType::Calys75
	QString m_caption;																	// name of calibrator
	QString m_serialNo;																	// serial number of calibrator

	int m_timeout = 0;																	// time counter waits for a response from the calibrator

	CalibratorMode m_mode = CalibratorMode::NoMode;										// calibrator mode: 0 - CalibratorMode::Measure or 1 - CalibratorMode::Source
	CalibratorUnit m_measureUnit = CalibratorUnit::NoUnit;								// measure unit: mA, mV and etc.
	CalibratorUnit m_sourceUnit = CalibratorUnit::NoUnit;								// source unit: mA, mV and etc.

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

	bool setUnit(int mode, int unit);												// select mode: measure - 0 (CalibratorMode::Measure) or source - 1 (CalibratorMode::Source)
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
