#include "MeasureThread.h"

#include <assert.h>

#include <QTime>

#include "MainWindow.h"
#include "Options.h"
#include "Conversion.h"
#include "CalibratorBase.h"

// -------------------------------------------------------------------------------------------------------------------

MeasureThread::MeasureThread(QObject *parent) :
	QThread(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

MeasureThread::~MeasureThread()
{
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::init(QWidget* parent)
{
	m_parent = parent;

	connect(&theSignalBase, &SignalBase::updatedSignalParam, this, &MeasureThread::updateSignalParam, Qt::QueuedConnection);

	connect(this, &MeasureThread::finished, this, &MeasureThread::stopMeasure);
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureThread::enableMesureIsSignal()
{
	MeasureSignal activeSignal = theSignalBase.activeSignal();
	if (activeSignal.isEmpty() == true)
	{
		return false;
	}

	// check if this signal has been measured before
	//
	if (theOptions.module().warningIfMeasured() == true)
	{
		QString measuredSignals;

		if (signalIsMeasured(activeSignal, measuredSignals) == true)
		{
			int result = QMessageBox::NoButton;
			emit msgBox(QMessageBox::Question, tr("Following signals were measured:\n\n%1\nDo you want to measure them again?").arg(measuredSignals), &result);
			if (result == QMessageBox::No)
			{
				return false;
			}
		}
	}

	// set param of active signal for measure
	//
	if (setActiveSignalParam(activeSignal) == false)
	{
		return false;
	}

	// if we check in single module mode
	// all module inputs must be the same
	//
	if (theOptions.toolBar().measureKind() == MEASURE_KIND_ONE_MODULE)
	{
		if (inputsOfmoduleIsSame() == false)
		{
			emit msgBox(QMessageBox::Information, tr("Unable to start the measurement process!\nAll electrical ranges of the inputs (or outputs) of the module must be the same."));
			return false;
		}
	}

	// set unit and mode on calibrators
	//
	if (setCalibratorUnit() == false)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureThread::signalIsMeasured(const MeasureSignal& activeSignal, QString& signalID)
{
	MultiChannelSignal signal;

	switch (theOptions.toolBar().signalConnectionType())
	{
		case SIGNAL_CONNECTION_TYPE_UNUSED:			signal = activeSignal.multiSignal(MEASURE_IO_SIGNAL_TYPE_INPUT);		break;
		case SIGNAL_CONNECTION_TYPE_FROM_INPUT:
		case SIGNAL_CONNECTION_TYPE_FROM_TUNING:	signal = activeSignal.multiSignal(MEASURE_IO_SIGNAL_TYPE_OUTPUT);	break;
		default:								assert(0);
	}

	if (signal.isEmpty() == true)
	{
		return false;
	}

	// temporary solution
	// find signal in list of statistics
	//
	MainWindow* pMainWindow = dynamic_cast<MainWindow*> (m_parent);
	if (pMainWindow == nullptr)
	{
		return false;
	}

	MeasureView* pMeasureView = pMainWindow->measureView(m_measureType);
	if (pMeasureView == nullptr)
	{
		return false;
	}

	bool isMeasured = false;

	for(int c = 0; c < activeSignal.channelCount(); c++)
	{
		Metrology::Signal* pMetrologySignal = signal.metrologySignal(c);
		if (pMetrologySignal == nullptr)
		{
			continue;
		}

		if (pMetrologySignal->param().isValid() == false)
		{
			continue;
		}

		Metrology::SignalStatistic ss = pMeasureView->table().m_measureBase.getSignalStatistic(pMetrologySignal->param().hash());
		if (ss.measureCount() != 0)
		{
			signalID.append(pMetrologySignal->param().customAppSignalID() + "\n");

			isMeasured = true;
		}
	}
	//
	// temporary solution

	return isMeasured;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureThread::setActiveSignalParam(const MeasureSignal& activeSignal)
{
	m_activeIoParamList.clear();

	// create param list for measure
	//
	for(int c = 0; c < activeSignal.channelCount(); c ++)
	{
		IoSignalParam ioParam;

		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type ++)
		{
			Metrology::Signal* pSignal = activeSignal.multiSignal(type).metrologySignal(c);
			if (pSignal == nullptr)
			{
				continue;
			}

			Metrology::SignalParam& param = pSignal->param();
			if (param.isValid() == false)
			{
				continue;
			}

			if (param.isAnalog() == false)
			{
				continue;
			}

//			if (param.physicalRangeIsValid() == false || param.engeneeringRangeIsValid() == false || param.electricRangeIsValid() == false)
//			{
//				continue;
//			}

			CalibratorManager* pCalibratorManager = theCalibratorBase.calibratorForMeasure(c);
			if (calibratorIsValid(pCalibratorManager) == false)
			{
				continue;
			}

			ioParam.setParam(type, param);
			ioParam.setSignalConnectionType(activeSignal.signalConnectionType());
			ioParam.setCalibratorManager(pCalibratorManager);
		}

		m_activeIoParamList.append(ioParam);
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureThread::inputsOfmoduleIsSame()
{
	bool				eltalonIsFound = false;

	double				electricLowLimit = 0;
	double				electricHighLimit = 0;
	E::ElectricUnit		electricUnitID = E::ElectricUnit::NoUnit;
	E::SensorType		electricSensorType = E::SensorType::NoSensor;

	Metrology::SignalParam signalParam;

	int channelCount = m_activeIoParamList.count();
	for(int c = 0; c < channelCount; c ++)
	{
		if (m_activeIoParamList[c].isValid() == false)
		{
			continue;
		}

		switch (theOptions.toolBar().signalConnectionType())
		{
			case SIGNAL_CONNECTION_TYPE_UNUSED:			signalParam = m_activeIoParamList[c].param(MEASURE_IO_SIGNAL_TYPE_INPUT);	break;
			case SIGNAL_CONNECTION_TYPE_FROM_INPUT:
			case SIGNAL_CONNECTION_TYPE_FROM_TUNING:	signalParam = m_activeIoParamList[c].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);	break;
			default:								assert(0);
		}

		if (eltalonIsFound == false)
		{
			eltalonIsFound = true;

			electricLowLimit = signalParam.electricLowLimit();
			electricHighLimit = signalParam.electricHighLimit();
			electricUnitID = signalParam.electricUnitID();
			electricSensorType = signalParam.electricSensorType();
		}
		else
		{
			if (electricLowLimit != signalParam.electricLowLimit() ||
				electricHighLimit != signalParam.electricHighLimit() ||
				electricUnitID != signalParam.electricUnitID() ||
				electricSensorType != signalParam.electricSensorType())
			{
				return false;
			}

		}
	}

	if (eltalonIsFound == false)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::waitMeasureTimeout()
{
	// Timeout for Measure
	//
	int measureTimeout = theOptions.toolBar().measureTimeout();

	for(int t = 0; t <= measureTimeout; t += MEASURE_THREAD_TIMEOUT_STEP)
	{
		if (m_cmdStopMeasure == true)
		{
			break;
		}

		QThread::msleep(MEASURE_THREAD_TIMEOUT_STEP);

		emit measureInfo(MEASURE_THREAD_TIMEOUT_STEP + t);
	}

	emit measureInfo(0);
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureThread::calibratorIsValid(CalibratorManager* pCalibratorManager)
{
	if (pCalibratorManager == nullptr)
	{
		return false;
	}

	if (pCalibratorManager->calibratorIsConnected() == false)
	{
		return false;
	}

	if (m_cmdStopMeasure == true)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureThread::hasConnectedCalibrators()
{
	if (m_cmdStopMeasure == true)
	{
		return false;
	}

	int connectedCalibratorCount = 0;

	int channelCount = m_activeIoParamList.count();
	for(int c = 0; c < channelCount; c ++)
	{
		CalibratorManager* pCalibratorManager = m_activeIoParamList[c].calibratorManager();
		if (calibratorIsValid(pCalibratorManager) == false)
		{
			continue;
		}

		connectedCalibratorCount++;
	}

	if (connectedCalibratorCount == 0)
	{
		emit msgBox(QMessageBox::Information, tr("No connected calibrators for measure"));
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureThread::setCalibratorUnit()
{
	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return false;
	}

	if (hasConnectedCalibrators() == false)
	{
		return false;
	}

	// select calibrator mode and calibrator unit
	// depend from analog signal
	//
	switch (m_measureType)
	{
		case MEASURE_TYPE_LINEARITY:
		case MEASURE_TYPE_COMPARATOR:
			{
				int channelCount = m_activeIoParamList.count();

				if (theOptions.toolBar().measureKind() == MEASURE_KIND_ONE_MODULE)
				{
					channelCount = 1;
				}

				for(int c = 0; c < channelCount; c ++)
				{
					if (m_activeIoParamList[c].isValid() == false)
					{
						continue;
					}

					CalibratorManager* pCalibratorManager = m_activeIoParamList[c].calibratorManager();
					if (calibratorIsValid(pCalibratorManager) == false)
					{
						continue;
					}

					switch (m_activeIoParamList[c].signalConnectionType())
					{
						case SIGNAL_CONNECTION_TYPE_UNUSED:
						case SIGNAL_CONNECTION_TYPE_FROM_INPUT:
							{
								Metrology::SignalParam inParam = m_activeIoParamList[c].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
								if (inParam.isValid() == false)
								{
									continue;
								}

								if (prepareCalibrator(pCalibratorManager, CALIBRATOR_MODE_SOURCE, inParam.electricUnitID(), inParam.electricHighLimit()) == false)
								{
									emit msgBox(QMessageBox::Information, QString("Calibrator: %1 - can not set source mode.").arg(pCalibratorManager->calibratorPort()));
								}

								Metrology::SignalParam outParam = m_activeIoParamList[c].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
								if (outParam.isValid() == false)
								{
									continue;
								}

								if (outParam.isOutput() == false)
								{
									continue;
								}

								if (prepareCalibrator(pCalibratorManager, CALIBRATOR_MODE_MEASURE, outParam.electricUnitID(), outParam.electricHighLimit()) == false)
								{
									emit msgBox(QMessageBox::Information, QString("Calibrator: %1 - can not set measure mode.").arg(pCalibratorManager->calibratorPort()));
								}

							}
							break;

						case SIGNAL_CONNECTION_TYPE_FROM_TUNING:
							{

								Metrology::SignalParam outParam = m_activeIoParamList[c].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
								if (outParam.isValid() == false)
								{
									continue;
								}

								if (outParam.isOutput() == false)
								{
									continue;
								}

								if (prepareCalibrator(pCalibratorManager, CALIBRATOR_MODE_MEASURE, outParam.electricUnitID(), outParam.electricHighLimit()) == false)
								{
									emit msgBox(QMessageBox::Information, QString("Calibrator: %1 - can not set measure mode.").arg(pCalibratorManager->calibratorPort()));
								}
							}

							break;

						default:
							assert(0);
					}
				}
			}
			break;

		default:
			assert(false);
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureThread::prepareCalibrator(CalibratorManager* pCalibratorManager, int calibratorMode, E::ElectricUnit signalUnit, double electricHighLimit)
{
	if (calibratorIsValid(pCalibratorManager) == false)
	{
		return false;
	}

	if (calibratorMode < 0 || calibratorMode >= CALIBRATOR_MODE_COUNT)
	{
		assert(0);
		return false;
	}

	int calibratorUnit = CALIBRATOR_UNIT_UNKNOWN;

	switch(signalUnit)
	{
		case E::ElectricUnit::mA:	calibratorUnit = CALIBRATOR_UNIT_MA;	break;
		case E::ElectricUnit::mV:	calibratorUnit = CALIBRATOR_UNIT_MV;	break;
		case E::ElectricUnit::V:	calibratorUnit = CALIBRATOR_UNIT_V;		break;
		case E::ElectricUnit::Ohm:
			{
				// Minimal range for calibrators TRX-II and Calys75 this is 400 Ohm
				//
				if (electricHighLimit <= 400)
				{
					calibratorUnit = CALIBRATOR_UNIT_LOW_OHM;
				}
				else
				{
					calibratorUnit = CALIBRATOR_UNIT_HIGH_OHM;
				}
			}

			break;

		default:
			assert(0);
	}

	if (calibratorUnit < 0 || calibratorUnit >= CALIBRATOR_UNIT_COUNT)
	{
		assert(0);
		return false;
	}

	emit measureInfo(QString("Prepare calibrator: %1 ").arg(pCalibratorManager->calibratorPort()));

	bool result =  pCalibratorManager->setUnit(calibratorMode, calibratorUnit);;

	switch (pCalibratorManager->calibrator()->type())
	{
		case CALIBRATOR_TYPE_TRXII:		QThread::msleep(1000);	break;
		case CALIBRATOR_TYPE_CALYS75:	QThread::msleep(500);	break;
		default: assert(0); break;
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::polarityTest(double electricVal, IoSignalParam& ioParam)
{
	if (calibratorIsValid(ioParam.calibratorManager()) == false)
	{
		return;
	}

	Calibrator* pCalibrator = ioParam.calibratorManager()->calibrator();
	if (pCalibrator == nullptr)
	{
		return;
	}

	double negativeLimit = 0;

	if (pCalibrator->mode() == CALIBRATOR_MODE_SOURCE && pCalibrator->sourceUnit() == CALIBRATOR_UNIT_MV)
	{
		negativeLimit = -10;
	}

	if (electricVal < negativeLimit && ioParam.isNegativeRange() == false)
	{
		ioParam.setNegativeRange(true);
		emit msgBox(QMessageBox::Information, tr("Please, switch polarity for calibrator %1\nYou have used the negative (-) part of the electrical range.").arg(ioParam.calibratorManager()->calibratorChannel() + 1));
	}

	if (electricVal >= negativeLimit && ioParam.isNegativeRange() == true)
	{
		ioParam.setNegativeRange(false);
		emit msgBox(QMessageBox::Information, tr("Please, switch polarity for calibrator %1\nYou have used the positive (+) part of the electrical range.").arg(ioParam.calibratorManager()->calibratorChannel() + 1));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::run()
{
	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	switch (m_measureType)
	{
		case MEASURE_TYPE_LINEARITY:				// test - have programm measure points

			if (theOptions.linearity().points().count() == 0)
			{
				emit msgBox(QMessageBox::Information, tr("No points for measure"));
				return;
			}

			break;

		case MEASURE_TYPE_COMPARATOR:	break;		// test - have signals of comporators
		default:						return;
	}

	// set command for exit (stop measure) in state = FALSE
	//
	m_cmdStopMeasure = false;

	bool signalIsSelected = true;

	// start of cycle for measuring all signals in selected module
	//
	while(signalIsSelected == true)
	{
		if (theCalibratorBase.connectedCalibratorsCount() == 0)
		{
			emit msgBox(QMessageBox::Information, tr("No connected calibrators for measure"));
			break;
		}

		if (enableMesureIsSignal() == true)
		{
			// start measure function
			//
			switch (m_measureType)
			{
				case MEASURE_TYPE_LINEARITY:	measureLinearity();		break;
				case MEASURE_TYPE_COMPARATOR:	measureComprators();	break;
				default:
					assert(0);
					m_cmdStopMeasure = true;
					break;
			}

			// if we decided to finish measuring, exit
			//
			if (m_cmdStopMeasure == true)
			{
				break;
			}
		}

		if (theOptions.module().measureEntireModule() == true)
		{
			// if we want to measure all signals of module
			// suspend MeasureThread
			// select next active analog signal
			//
			emit setNextMeasureSignal(signalIsSelected); // call signal how - Qt::BlockingQueuedConnection
			//
			// resume MeasureThread
		}
		else
		{
			// if we want to measure only one selected analog signal of module
			// exit from cycle
			//
			break;
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureLinearity()
{
	saveStateTunSignals();

	int pointCount = theOptions.linearity().points().count();
	for(int p = 0; p < pointCount; p++)
	{
		if (hasConnectedCalibrators() == false)
		{
			break;
		}

		LinearityPoint point = theOptions.linearity().points().at(p);

		emit measureInfo(tr("Set point %1 / %2 ").arg(p + 1).arg(pointCount));

		// set electric value on calibrators, depend from point value
		//
		int channelCount = m_activeIoParamList.count();

		if (theOptions.toolBar().measureKind() == MEASURE_KIND_ONE_MODULE)
		{
			channelCount = 1;
		}

		for(int c = 0; c < channelCount; c ++)
		{
			if (m_activeIoParamList[c].isValid() == false)
			{
				continue;
			}

			CalibratorManager* pCalibratorManager = m_activeIoParamList[c].calibratorManager();
			if (calibratorIsValid(pCalibratorManager) == false)
			{
				continue;
			}

			Metrology::SignalParam param = m_activeIoParamList[c].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
			if (param.isValid() == false)
			{
				continue;
			}

			m_activeIoParamList[c].setPercent(point.percent());

			// set electric value
			//
			switch (m_activeIoParamList[c].signalConnectionType())
			{
				case SIGNAL_CONNECTION_TYPE_UNUSED:
				case SIGNAL_CONNECTION_TYPE_FROM_INPUT:
					{
						// at the beginning we need get engeneering value because if range is not Linear (for instance Ohm or mV)
						// then by engeneering value we may get electric value
						//
						double engeneeringVal = (point.percent() * (param.highEngeneeringUnits() - param.lowEngeneeringUnits()) / 100) + param.lowEngeneeringUnits();
						double electricVal = conversion(engeneeringVal, CT_ENGENEER_TO_ELECTRIC, param);

						polarityTest(electricVal, m_activeIoParamList[c]);	// polarity test

						pCalibratorManager->setValue(m_activeIoParamList[c].isNegativeRange() ? -electricVal : electricVal);
					}
					break;
				case SIGNAL_CONNECTION_TYPE_FROM_TUNING:
					{
						double tuningVal = (point.percent() * (param.tuningHighBound().toDouble() - param.tuningLowBound().toDouble()) / 100) + param.tuningLowBound().toDouble();

						theSignalBase.tuning().appendCmdFowWrite(param.hash(), param.tuningValueType(), tuningVal);
					}
					break;
				default:
					assert(0);
			}
		}

		// wait ready all calibrators,
		// wait until all calibrators will has fixed electric value
		//
		for(int c = 0; c < channelCount; c ++)
		{
			CalibratorManager* pCalibratorManager = m_activeIoParamList[c].calibratorManager();
			if (calibratorIsValid(pCalibratorManager) == false)
			{
				continue;
			}

			while(pCalibratorManager->isReadyForManage() != true)
			{
				if (m_cmdStopMeasure == true)
				{
					break;
				}

				msleep(1);
			}
		}

		// wait timeout for measure
		//
		emit measureInfo(tr("Wait timeout %1 / %2 ").arg(p + 1).arg(pointCount));
		waitMeasureTimeout();

		// save measurement
		//
		emit measureInfo(tr("Save measurement "));

		channelCount = m_activeIoParamList.count();
		for(int c = 0; c < channelCount; c ++)
		{
			if (m_activeIoParamList[c].isValid() == false)
			{
				continue;
			}

			CalibratorManager* pCalibratorManager = m_activeIoParamList[c].calibratorManager();
			if (calibratorIsValid(pCalibratorManager) == false)
			{
				continue;
			}

			LinearityMeasurement* pMeasurement = new LinearityMeasurement(m_activeIoParamList[c]);
			if (pMeasurement == nullptr)
			{
				continue;
			}

			emit measureComplite(pMeasurement);
		}

		emit measureInfo(tr(""));
	}

	restoreStateTunSignals();
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureComprators()
{
	emit measureInfo(tr("Comprators"));

	waitMeasureTimeout();
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::signalSocketDisconnected()
{
	stopMeasure();
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::tuningSocketDisconnected()
{
	if (theOptions.toolBar().signalConnectionType() != SIGNAL_CONNECTION_TYPE_FROM_TUNING)
	{
		return;
	}

	stopMeasure();
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::saveStateTunSignals()
{
	if (theOptions.toolBar().signalConnectionType() != SIGNAL_CONNECTION_TYPE_FROM_TUNING)
	{
		return;
	}

	int channelCount = m_activeIoParamList.count();
	for(int c = 0; c < channelCount; c ++)
	{
		if (m_activeIoParamList[c].isValid() == false)
		{
			continue;
		}

		Metrology::SignalParam tunParam = m_activeIoParamList[c].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
		if (tunParam.isValid() == false)
		{
			continue;
		}

		m_activeIoParamList[c].setTunSignalState(theSignalBase.signalState(tunParam.hash()).value());
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::restoreStateTunSignals()
{
	if (theOptions.toolBar().signalConnectionType() != SIGNAL_CONNECTION_TYPE_FROM_TUNING)
	{
		return;
	}

	int channelCount = m_activeIoParamList.count();
	for(int c = 0; c < channelCount; c ++)
	{
		if (m_activeIoParamList[c].isValid() == false)
		{
			continue;
		}

		Metrology::SignalParam tunParam = m_activeIoParamList[c].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
		if (tunParam.isValid() == false)
		{
			continue;
		}

		theSignalBase.tuning().appendCmdFowWrite(tunParam.hash(), tunParam.tuningValueType(), m_activeIoParamList[c].tunSignalState());
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::updateSignalParam(const QString& appSignalID)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(0);
		return;
	}

	int channelCount = m_activeIoParamList.count();
	for(int c = 0; c < channelCount; c ++)
	{
		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type ++)
		{
			if (m_activeIoParamList[c].param(type).appSignalID() == appSignalID)
			{
				m_activeIoParamList[c].setParam(type, theSignalBase.signalParam(appSignalID));
			}
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::stopMeasure()
{
	setMeasureType(MEASURE_TYPE_UNKNOWN);
	m_cmdStopMeasure = true;
}

// -------------------------------------------------------------------------------------------------------------------
