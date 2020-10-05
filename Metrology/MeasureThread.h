#ifndef MEASURETHREAD_H
#define MEASURETHREAD_H

#include <QThread>
#include <QMessageBox>

#include "MeasureBase.h"
#include "SignalBase.h"
#include "TuningSignalBase.h"

// ==============================================================================================

const int				MEASURE_THREAD_TIMEOUT_STEP			= 100; // 100 milliseconds

// ==============================================================================================

const int				MEASURE_THREAD_CMP_PREAPRE_1		= 0,
						MEASURE_THREAD_CMP_PREAPRE_2		= 1;

const int				MEASURE_THREAD_CMP_PREPARE_COUNT	= 2;

// ==============================================================================================

class MeasureThread : public QThread
{
	Q_OBJECT

public:

	explicit MeasureThread(QObject *parent = nullptr);
	virtual ~MeasureThread();

private:

	int						m_measureType = MEASURE_TYPE_UNKNOWN;
	bool					m_cmdStopMeasure = true;

	QVector<IoSignalParam>	m_activeIoParamList;

	void					waitMeasureTimeout();

	// calibrators
	//
	bool					calibratorIsValid(CalibratorManager* pCalibratorManager);
	int						getConnectedCalibrators();
	bool					setCalibratorUnit();
	bool					prepareCalibrator(CalibratorManager* pCalibratorManager, int calibratorMode, E::ElectricUnit signalUnit, double electricHighLimit);
	void					polarityTest(double electricVal, IoSignalParam& ioParam);

	// function of measure
	//
	void					measureLinearity();
	void					measureComprators();

	// function for tunning signal
	//
	void					saveStateTunSignals();
	void					restoreStateTunSignals();

public:

	int						measureType() const { return m_measureType; }
	void					setMeasureType(int measureType) { m_measureType = measureType; }

	bool					enableMesureIsSignal();
	bool					signalIsMeasured(const MeasureSignal& activeSignal, QString& signalID);
	bool					setActiveSignalParam(const MeasureSignal& activeSignal);
	bool					inputsOfmoduleIsSame();														// only for mode "Simaple module"

	void					stop() { m_cmdStopMeasure = true; }

protected:

	void					run();

signals:

	void					msgBox(int type, QString text, int* result = nullptr);

	void					measureInfo(QString);
	void					measureInfo(int);

	void					measureComplite(Measurement*);

	void					setNextMeasureSignal(bool& signalIsSelected);

public slots:

	void					signalSocketDisconnected();
	void					tuningSocketDisconnected();

	void					updateSignalParam(const QString& appSignalID);

	void					stopMeasure();
};

// ==============================================================================================

#endif // MEASURETHREAD_H
