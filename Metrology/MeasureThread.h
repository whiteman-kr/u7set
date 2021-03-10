#ifndef MEASURETHREAD_H
#define MEASURETHREAD_H

#include <QThread>
#include <QMessageBox>

#include "CalibratorBase.h"
#include "MeasureBase.h"
#include "SignalBase.h"
#include "TuningSignalBase.h"
#include "Options.h"

// ==============================================================================================

const int				MEASURE_THREAD_TIMEOUT_STEP			= 100; // 100 milliseconds

// ==============================================================================================

const int				MEASURE_THREAD_CMP_PREAPRE_1		= 0,
						MEASURE_THREAD_CMP_PREAPRE_2		= 1;

const int				MEASURE_THREAD_CMP_PREPARE_COUNT	= 2;

// ==============================================================================================

class MeasureThreadInfo
{
public:

		MeasureThreadInfo();

		enum msgType
		{
			String,
			StringError,
			Timeout,
		};

		enum ExitCode
		{
			Program = 0,
			Manual = 1,
		};

public:

		void clear();

		msgType type() const { return m_type; }

		QString message() const { return m_message; }
		void setMessage(const QString& message, const msgType& type = msgType::String);

		int timeout() const { return m_timeout; }
		void setTimeout(int timeout);

		bool cmdStopMeasure() const { return m_cmdStopMeasure; }
		void setCmdStopMeasure(bool stop) { m_cmdStopMeasure = stop; }

		ExitCode exitCode() const { return m_exitCode; }
		void setExitCode(ExitCode exitCode) { m_exitCode = exitCode; }

private:

		msgType m_type = msgType::String;
		QString m_message;
		int m_timeout = 0;

		bool m_cmdStopMeasure = true;
		ExitCode m_exitCode = ExitCode::Manual;
};

Q_DECLARE_METATYPE(MeasureThreadInfo)

// ==============================================================================================

class MeasureThread : public QThread
{
	Q_OBJECT

public:

	explicit MeasureThread(QObject* parent = nullptr);
	virtual ~MeasureThread() override;

public:

	MeasureThreadInfo		info() const { return m_info; }

	bool					setActiveSignalParam(const MeasureSignal& activeSignal, const CalibratorBase& calibratorBase);

	void					setLinearityOption(const LinearityOption& option) { m_linearityOption = option; }
	void					setComparatorOption(const ComparatorOption& option) { m_comparatorOption = option; }

private:

	Measure::Type			m_measureType = Measure::Type::NoMeasureType;
	Measure::Kind			m_measureKind = Measure::Kind::NoMeasureKind;
	Metrology::ConnectionType m_connectionType = Metrology::ConnectionType::NoConnectionType;

	MeasureThreadInfo		m_info;

	QVector<IoSignalParam>	m_activeIoParamList;

	int						m_measureTimeout = 0;
	void					waitMeasureTimeout();

	// calibrators
	//
	bool					calibratorIsValid(CalibratorManager* pCalibratorManager);
	int						getConnectedCalibrators();
	bool					setCalibratorUnit();
	bool					prepareCalibrator(CalibratorManager* pCalibratorManager, CalibratorMode calibratorMode, E::ElectricUnit signalUnit, double electricHighLimit);
	void					polarityTest(double electricVal, IoSignalParam& ioParam);

	// Options
	//
	LinearityOption			m_linearityOption;
	ComparatorOption		m_comparatorOption;

	// function of measure
	//
	void					measureLinearity();
	void					measureCompratorsInSeries();
	void					measureCompratorsInParallel();

	// function for tunning signal
	//
	void					saveStateTunSignals();
	void					restoreStateTunSignals();

protected:

	void					run() override;

signals:

	void					sendMeasureInfo(const MeasureThreadInfo& info);
	void					msgBox(int type, QString text, int* result = nullptr);

	void					measureComplite(Measure::Item*);

public slots:

	void					signalSocketDisconnected();
	void					tuningSocketDisconnected();

	void					measureTimeoutChanged(int timeout);
	void					measureTypeChanged(int measureType);
	void					measureKindChanged(int measureKind);
	void					connectionTypeChanged(int connectionType);

	void					signalParamChanged(const QString& appSignalID);

	void					stopMeasure(MeasureThreadInfo::ExitCode exitCode);
};

// ==============================================================================================

#endif // MEASURETHREAD_H
