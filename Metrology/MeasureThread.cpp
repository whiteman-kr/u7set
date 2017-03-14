#include "MeasureThread.h"

#include <assert.h>

#include <QTime>

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

	connect(this, &MeasureThread::showMsgBox, this, &MeasureThread::msgBox);
	connect(this, &MeasureThread::finished, this, &MeasureThread::stopMeasure);
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
	for(int c = 0; c < MAX_CHANNEL_COUNT; c ++)
	{
		m_activeSignalParam[c].clear();

		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type ++)
		{
			Hash signalHash = activeSignal.signal(type).hash(c);
			if (signalHash == 0)
			{
				continue;
			}

			Metrology::SignalParam param =  theSignalBase.signalParam(signalHash);
			if (param.isValid() == false)
			{
				continue;
			}

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

	for(int c = 0; c < MAX_CHANNEL_COUNT; c ++)
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
		emit showMsgBox(tr("No connected calibrators for measure"));
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
				for(int c = 0; c < MAX_CHANNEL_COUNT; c ++)
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

								if (prepareCalibrator(pCalibratorManager, CALIBRATOR_MODE_SOURCE, inParam.inputElectricUnitID(), inParam.inputElectricHighLimit()) == false)
								{
									emit showMsgBox(QString("Calibrator: %1 - can not set source mode.").arg(pCalibratorManager->calibratorPort()));
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

								if (prepareCalibrator(pCalibratorManager, CALIBRATOR_MODE_MEASURE, outParam.outputElectricUnitID(), outParam.outputElectricHighLimit()) == false)
								{
									emit showMsgBox(QString("Calibrator: %1 - can not set measure mode.").arg(pCalibratorManager->calibratorPort()));
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

								if (prepareCalibrator(pCalibratorManager, CALIBRATOR_MODE_MEASURE, outParam.outputElectricUnitID(), outParam.outputElectricHighLimit()) == false)
								{
									emit showMsgBox(QString("Calibrator: %1 - can not set measure mode.").arg(pCalibratorManager->calibratorPort()));
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

bool MeasureThread::prepareCalibrator(CalibratorManager* pCalibratorManager, int calibratorMode, E::InputUnit signalUnit, double electricHighLimit)
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

	if (theSignalBase.units().contains(signalUnit) == false)
	{
		assert(0);
		return false;
	}

	int calibratorUnit = CALIBRATOR_UNIT_UNKNOWN;

	switch(signalUnit)
	{
		case E::InputUnit::mA:	calibratorUnit = CALIBRATOR_UNIT_MA;	break;
		case E::InputUnit::mV:	calibratorUnit = CALIBRATOR_UNIT_MV;	break;
		case E::InputUnit::V:	calibratorUnit = CALIBRATOR_UNIT_V;		break;
		case E::InputUnit::Ohm:
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

	if (pCalibratorManager->calibrator()->type() == CALIBRATOR_TYPE_TRXII)
	{
		QThread::msleep(1000);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::run()
{
	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	// set command for exit (stop measure) in state = FALSE
	//
	m_cmdStopMeasure = false;

	// set param of active signal for measure
	//
	if (setActiveSignalParam() == false)
	{
		return;
	}

	// set unit and mode on calibrators
	//
	if (setCalibratorUnit() == false)
	{
		return;
	}

	// start measure function
	//
	switch (m_measureType)
	{
		case MEASURE_TYPE_LINEARITY:	measureLinearity();		break;
		case MEASURE_TYPE_COMPARATOR:	measureComprators();	break;
		default:						assert(0);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureLinearity()
{
	int pointCount = theOptions.linearity().points().count();
	if (pointCount == 0)
	{
		emit showMsgBox(tr("No points for measure"));
		return;
	}

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
		for(int c = 0; c < MAX_CHANNEL_COUNT; c ++)
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

			// at the beginning we need get physical value because if range is not Linear (for instance Ohm or mV)
			// then by physical value we may get electric value
			//
			double physicalVal = (point.percent() * (param.inputPhysicalHighLimit() - param.inputPhysicalLowLimit()) / 100) + param.inputPhysicalLowLimit();
			double electricVal = conversion(physicalVal, CT_PHYSICAL_TO_ELECTRIC, param);

			switch (m_activeSignalParam[c].outputSignalType())
			{
				case OUTPUT_SIGNAL_TYPE_UNUSED:
				case OUTPUT_SIGNAL_TYPE_FROM_INPUT:		pCalibratorManager->setValue(electricVal);							break;
				case OUTPUT_SIGNAL_TYPE_FROM_TUNING:	theTuningSignalBase.appendCmdFowWrite(param.hash(), physicalVal);	break;
				default:								assert(0);
			}
		}

		// wait ready all calibrators,
		// wait until all calibrators will has fixed electric value
		//
		for(int c = 0; c < MAX_CHANNEL_COUNT; c ++)
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
			}
		}

		// wait timeout for measure
		//
		emit measureInfo(tr("Wait timeout %1 / %2 ").arg(p + 1).arg(pointCount));
		waitMeasureTimeout();

		// save measurement
		//
		emit measureInfo(tr("Save measurement "));

		for(int c = 0; c < MAX_CHANNEL_COUNT; c ++)
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
	}
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

void MeasureThread::updateSignalParam(const Hash& signalHash)
{
	if (signalHash == 0)
	{
		assert(signalHash != 0);
		return;
	}

	for(int c = 0; c < MAX_CHANNEL_COUNT; c ++)
	{
		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type ++)
		{
			if (m_activeSignalParam[c].param(type).hash() == signalHash)
			{
				m_activeSignalParam[c].setParam(type, theSignalBase.signalParam(signalHash));
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
