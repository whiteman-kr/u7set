#ifndef MEASURETHREAD_H
#define MEASURETHREAD_H

#include <QThread>
#include <QMessageBox>

#include "MeasureBase.h"
#include "SignalBase.h"
#include "TuningSignalBase.h"

// ==============================================================================================

const int				   MEASURE_THREAD_TIMEOUT_STEP = 100; // 100 milliseconds

// ==============================================================================================

class MeasureThread : public QThread
{
	Q_OBJECT

public:

	explicit MeasureThread(QObject *parent = 0);
	virtual ~MeasureThread();

private:

	QWidget*				m_parent = nullptr;

	int						m_measureType = MEASURE_TYPE_UNKNOWN;
	bool					m_cmdStopMeasure = true;

	MeasureMultiParam		m_activeSignalParam[Metrology::ChannelCount];

	double					m_tunSignalState[Metrology::ChannelCount];

	void					waitMeasureTimeout();

	// calibrators
	//
	bool					calibratorIsValid(CalibratorManager* pCalibratorManager);
	bool					hasConnectedCalibrators();
	bool					setCalibratorUnit();
	bool					prepareCalibrator(CalibratorManager* pCalibratorManager, int calibratorMode, E::ElectricUnit signalUnit, double electricHighLimit);

	// function of measure
	//
	void					measureLinearity();
	void					measureComprators();

	// function for tunning signal
	//
	void					saveStateTunSignals();
	void					restoreStateTunSignals();

public:

	void					init(QWidget* parent = 0);
	void					setMeasureType(int measureType) { m_measureType = measureType; }
	bool					enableMesureIsSignal();
	bool					signalIsMeasured(QString& signalID);
	bool					setActiveSignalParam();

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

private slots:

	void					updateSignalParam(const Hash& signalHash);

	void					stopMeasure();
};

// ==============================================================================================

#endif // MEASURETHREAD_H
