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
	if (theOptions.module().warningIfMeasured() == true)
	{
		QString measuredSignals;

		if (signalIsMeasured(measuredSignals) == true)
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
	if (setActiveSignalParam() == false)
	{
		return false;
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

bool MeasureThread::signalIsMeasured(QString& signalID)
{
	MainWindow* pMainWindow = dynamic_cast<MainWindow*> (m_parent);
	if (pMainWindow == nullptr)
	{
		return false;
	}

	MeasureSignal measureSignal = theSignalBase.activeSignal();
	if (measureSignal.isEmpty() == true)
	{
		return false;
	}

	MetrologyMultiSignal signal;

	switch (theOptions.toolBar().outputSignalType())
	{
		case OUTPUT_SIGNAL_TYPE_UNUSED:			signal = measureSignal.signal(MEASURE_IO_SIGNAL_TYPE_INPUT);	break;
		case OUTPUT_SIGNAL_TYPE_FROM_INPUT:
		case OUTPUT_SIGNAL_TYPE_FROM_TUNING:	signal = measureSignal.signal(MEASURE_IO_SIGNAL_TYPE_OUTPUT);	break;
		default:								assert(0);
	}

	if (signal.isEmpty() == true)
	{
		return false;
	}

	// temporary solution
	// metrologySignal.setStatistic(theMeasureBase.statisticItem(param.hash()));
	//
	MeasureView* pMeasureView = pMainWindow->measureView(m_measureType);
	if (pMeasureView == nullptr)
	{
		return false;
	}

	bool isMeasured = false;

	for(int c = 0; c < Metrology::ChannelCount; c++)
	{
		Metrology::Signal* pMetrologySignal = signal.metrologySignal(c);
		if (pMetrologySignal == nullptr)
		{
			continue;
		}

		Metrology::SignalParam& param = pMetrologySignal->param();
		if (param.isValid() == false)
		{
			continue;
		}

		pMetrologySignal->setStatistic(pMeasureView->table().m_measureBase.statistic(param.hash()));
		if (pMetrologySignal->statistic().measureCount() != 0)
		{
			signalID.append(param.customAppSignalID() + "\n");

			isMeasured = true;
		}
	}
	//
	// temporary solution

	return isMeasured;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureThread::setActiveSignalParam()
{
	MeasureSignal activeSignal = theSignalBase.activeSignal();
	if (activeSignal.isEmpty() == true)
	{
		return false;
	}

	// create param list for measure
	//
	for(int c = 0; c < Metrology::ChannelCount; c ++)
	{
		m_activeSignalParam[c].clear();

		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type ++)
		{
			Metrology::Signal* pSignal = activeSignal.signal(type).metrologySignal(c);
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

			m_activeSignalParam[c].setParam(type, param);
			m_activeSignalParam[c].setOutputSignalType(activeSignal.outputSignalType());
			m_activeSignalParam[c].setCalibratorManager(pCalibratorManager);
		}
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

	for(int c = 0; c < Metrology::ChannelCount; c ++)
	{
		CalibratorManager* pCalibratorManager = m_activeSignalParam[c].calibratorManager();
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
				for(int c = 0; c < Metrology::ChannelCount; c ++)
				{
					CalibratorManager* pCalibratorManager = m_activeSignalParam[c].calibratorManager();
					if (calibratorIsValid(pCalibratorManager) == false)
					{
						continue;
					}

					switch (m_activeSignalParam[c].outputSignalType())
					{
						case OUTPUT_SIGNAL_TYPE_UNUSED:
						case OUTPUT_SIGNAL_TYPE_FROM_INPUT:
							{
								Metrology::SignalParam inParam = m_activeSignalParam[c].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
								if (inParam.isValid() == false)
								{
									continue;
								}

								if (prepareCalibrator(pCalibratorManager, CALIBRATOR_MODE_SOURCE, inParam.electricUnitID(), inParam.electricHighLimit()) == false)
								{
									emit msgBox(QMessageBox::Information, QString("Calibrator: %1 - can not set source mode.").arg(pCalibratorManager->calibratorPort()));
								}

								Metrology::SignalParam outParam = m_activeSignalParam[c].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
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

						case OUTPUT_SIGNAL_TYPE_FROM_TUNING:
							{

								Metrology::SignalParam outParam = m_activeSignalParam[c].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
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

void MeasureThread::polarityTest(double electricVal, MeasureMultiParam& param)
{
	if (calibratorIsValid(param.calibratorManager()) == false)
	{
		return;
	}

	Calibrator* pCalibrator = param.calibratorManager()->calibrator();
	if (pCalibrator == nullptr)
	{
		return;
	}

	double negativeLimit = 0;

	if (pCalibrator->mode() == CALIBRATOR_MODE_SOURCE && pCalibrator->sourceUnit() == CALIBRATOR_UNIT_MV)
	{
		negativeLimit = -10;
	}

	if (electricVal < negativeLimit && param.isNegativeRange() == false)
	{
		param.setNegativeRange(true);
		emit msgBox(QMessageBox::Information, tr("Please, switch polarity for calibrator %1\nYou have used the negative (-) part of the electrical range.").arg(param.calibratorManager()->calibratorChannel() + 1));
	}

	if (electricVal >= negativeLimit && param.isNegativeRange() == true)
	{
		param.setNegativeRange(false);
		emit msgBox(QMessageBox::Information, tr("Please, switch polarity for calibrator %1\nYou have used the positive (+) part of the electrical range.").arg(param.calibratorManager()->calibratorChannel() + 1));
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

		case MEASURE_TYPE_COMPARATOR:	break;		// test - have signal comporators
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
		for(int c = 0; c < Metrology::ChannelCount; c ++)
		{
			CalibratorManager* pCalibratorManager = m_activeSignalParam[c].calibratorManager();
			if (calibratorIsValid(pCalibratorManager) == false)
			{
				continue;
			}

			Metrology::SignalParam param = m_activeSignalParam[c].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
			if (param.isValid() == false)
			{
				continue;
			}

			m_activeSignalParam[c].setPercent(point.percent());

			// set electric value
			//
			switch (m_activeSignalParam[c].outputSignalType())
			{
				case OUTPUT_SIGNAL_TYPE_UNUSED:
				case OUTPUT_SIGNAL_TYPE_FROM_INPUT:
					{
						// at the beginning we need get engeneering value because if range is not Linear (for instance Ohm or mV)
						// then by engeneering value we may get electric value
						//
						double engeneeringVal = (point.percent() * (param.highEngeneeringUnits() - param.lowEngeneeringUnits()) / 100) + param.lowEngeneeringUnits();
						double electricVal = conversion(engeneeringVal, CT_ENGENEER_TO_ELECTRIC, param);

						polarityTest(electricVal, m_activeSignalParam[c]);	// polarity test

						pCalibratorManager->setValue(m_activeSignalParam[c].isNegativeRange() ? -electricVal : electricVal);
					}
					break;
				case OUTPUT_SIGNAL_TYPE_FROM_TUNING:
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
		for(int c = 0; c < Metrology::ChannelCount; c ++)
		{
			CalibratorManager* pCalibratorManager = m_activeSignalParam[c].calibratorManager();
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

		for(int c = 0; c < Metrology::ChannelCount; c ++)
		{
			CalibratorManager* pCalibratorManager = m_activeSignalParam[c].calibratorManager();
			if (calibratorIsValid(pCalibratorManager) == false)
			{
				continue;
			}

			if (m_activeSignalParam[c].isValid() == false)
			{
				continue;
			}

			LinearityMeasurement* pMeasurement = new LinearityMeasurement(m_activeSignalParam[c]);
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
	emit measureInfo(tr("Comprators "));

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
	if (theOptions.toolBar().outputSignalType() != OUTPUT_SIGNAL_TYPE_FROM_TUNING)
	{
		return;
	}

	stopMeasure();
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::saveStateTunSignals()
{
	if (theOptions.toolBar().outputSignalType() != OUTPUT_SIGNAL_TYPE_FROM_TUNING)
	{
		return;
	}

	for(int c = 0; c < Metrology::ChannelCount; c ++)
	{
		if (m_activeSignalParam[c].isValid() == false)
		{
			continue;
		}

		Metrology::SignalParam tunParam = m_activeSignalParam[c].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
		if (tunParam.isValid() == false)
		{
			continue;
		}

		m_tunSignalState[c] = theSignalBase.signalState(tunParam.hash()).value();

//		QString val_str;
//		val_str.sprintf("Tun save - %.3f", m_tunSignalState[c]);
//		emit msgBox(QMessageBox::Information, val_str);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::restoreStateTunSignals()
{
	if (theOptions.toolBar().outputSignalType() != OUTPUT_SIGNAL_TYPE_FROM_TUNING)
	{
		return;
	}

	for(int c = 0; c < Metrology::ChannelCount; c ++)
	{
		if (m_activeSignalParam[c].isValid() == false)
		{
			continue;
		}

		Metrology::SignalParam tunParam = m_activeSignalParam[c].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
		if (tunParam.isValid() == false)
		{
			continue;
		}

//		QString val_str;
//		val_str.sprintf("Tun restore - %.3f", m_tunSignalState[c]);
//		emit msgBox(QMessageBox::Information, val_str);

		theSignalBase.tuning().appendCmdFowWrite(tunParam.hash(), tunParam.tuningValueType(), m_tunSignalState[c]);
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

	for(int c = 0; c < Metrology::ChannelCount; c ++)
	{
		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type ++)
		{
			if (m_activeSignalParam[c].param(type).appSignalID() == appSignalID)
			{
				m_activeSignalParam[c].setParam(type, theSignalBase.signalParam(appSignalID));
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
