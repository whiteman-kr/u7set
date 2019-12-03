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
		case SIGNAL_CONNECTION_TYPE_UNUSED:			signal = activeSignal.multiChannelSignal(MEASURE_IO_SIGNAL_TYPE_INPUT);		break;
		case SIGNAL_CONNECTION_TYPE_FROM_INPUT:
		case SIGNAL_CONNECTION_TYPE_FROM_TUNING:	signal = activeSignal.multiChannelSignal(MEASURE_IO_SIGNAL_TYPE_OUTPUT);	break;
		default:									assert(0);
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

	for(int ch = 0; ch < activeSignal.channelCount(); ch++)
	{
		Metrology::Signal* pMetrologySignal = signal.metrologySignal(ch);
		if (pMetrologySignal == nullptr)
		{
			continue;
		}

		if (pMetrologySignal->param().isValid() == false)
		{
			continue;
		}

		Metrology::SignalStatistic ss = pMeasureView->table().m_measureBase.getSignalStatistic(pMetrologySignal->param().hash());
		if (ss.isMeasured() == true)
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
	int channelCount = activeSignal.channelCount();
	for(int ch = 0; ch < channelCount; ch ++)
	{
		IoSignalParam ioParam;

		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type ++)
		{
			Metrology::Signal* pSignal = activeSignal.multiChannelSignal(type).metrologySignal(ch);
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

//			if (param.physicalRangeIsValid() == false || param.engineeringRangeIsValid() == false || param.electricRangeIsValid() == false)
//			{
//				continue;
//			}

			CalibratorManager* pCalibratorManager = theCalibratorBase.calibratorForMeasure(ch);
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
	for(int ch = 0; ch < channelCount; ch ++)
	{
		if (m_activeIoParamList[ch].isValid() == false)
		{
			continue;
		}

		switch (theOptions.toolBar().signalConnectionType())
		{
			case SIGNAL_CONNECTION_TYPE_UNUSED:			signalParam = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);	break;
			case SIGNAL_CONNECTION_TYPE_FROM_INPUT:
			case SIGNAL_CONNECTION_TYPE_FROM_TUNING:	signalParam = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);	break;
			default:									assert(0);
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
			if (compareFloat(electricLowLimit, signalParam.electricLowLimit()) == false ||
				compareFloat(electricHighLimit, signalParam.electricHighLimit()) == false ||
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

int MeasureThread::getConnectedCalibrators()
{
	if (m_cmdStopMeasure == true)
	{
		return 0;
	}

	int connectedCalibratorCount = 0;

	int channelCount = m_activeIoParamList.count();
	for(int ch = 0; ch < channelCount; ch ++)
	{
		CalibratorManager* pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
		if (calibratorIsValid(pCalibratorManager) == false)
		{
			continue;
		}

		connectedCalibratorCount++;
	}

	if (connectedCalibratorCount == 0)
	{
		emit msgBox(QMessageBox::Information, tr("No connected calibrators for measure"));
	}

	return connectedCalibratorCount;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureThread::setCalibratorUnit()
{
	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return false;
	}

	if (getConnectedCalibrators() == 0)
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

				for(int ch = 0; ch < channelCount; ch ++)
				{
					if (m_activeIoParamList[ch].isValid() == false)
					{
						continue;
					}

					CalibratorManager* pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
					if (calibratorIsValid(pCalibratorManager) == false)
					{
						continue;
					}

					switch (m_activeIoParamList[ch].signalConnectionType())
					{
						case SIGNAL_CONNECTION_TYPE_UNUSED:
						case SIGNAL_CONNECTION_TYPE_FROM_INPUT:
							{
								Metrology::SignalParam inParam = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
								if (inParam.isValid() == false)
								{
									continue;
								}

								if (prepareCalibrator(pCalibratorManager, CALIBRATOR_MODE_SOURCE, inParam.electricUnitID(), inParam.electricHighLimit()) == false)
								{
									emit msgBox(QMessageBox::Information, QString("Calibrator: %1 - can not set source mode.").arg(pCalibratorManager->calibratorPort()));
								}

								Metrology::SignalParam outParam = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
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

								Metrology::SignalParam outParam = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
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
	// test - have programm measure points
	//
	if (theOptions.linearity().points().count() == 0)
	{
		emit msgBox(QMessageBox::Information, tr("No points for measure"));
		return;
	}

	saveStateTunSignals();

	int pointCount = theOptions.linearity().points().count();
	for(int pt = 0; pt < pointCount; pt++)
	{
		LinearityPoint point = theOptions.linearity().points().at(pt);

		emit measureInfo(tr("Set point %1 / %2 ").arg(pt + 1).arg(pointCount));

		// set electric value on calibrators, depend from point value
		//
		int channelCount = m_activeIoParamList.count();

		if (theOptions.toolBar().measureKind() == MEASURE_KIND_ONE_MODULE)
		{
			channelCount = 1;
		}

		for(int ch = 0; ch < channelCount; ch ++)
		{
			CalibratorManager* pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
			if (calibratorIsValid(pCalibratorManager) == false)
			{
				continue;
			}

			if (m_activeIoParamList[ch].isValid() == false)
			{
				continue;
			}

			const Metrology::SignalParam& inParam = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
			if (inParam.isValid() == false)
			{
				continue;
			}

			// set electric value
			//
			switch (m_activeIoParamList[ch].signalConnectionType())
			{
				case SIGNAL_CONNECTION_TYPE_UNUSED:
				case SIGNAL_CONNECTION_TYPE_FROM_INPUT:
					{
						// at the beginning we need get engineering value because if range is not Linear (for instance Ohm or mV)
						// then by engineering value we may get electric value
						//
						double engineeringVal = (point.percent() * (inParam.highEngineeringUnits() - inParam.lowEngineeringUnits()) / 100) + inParam.lowEngineeringUnits();
						double electricVal = conversion(engineeringVal, CT_ENGINEER_TO_ELECTRIC, inParam);

						polarityTest(electricVal, m_activeIoParamList[ch]);	// polarity test

						pCalibratorManager->setValue(m_activeIoParamList[ch].isNegativeRange() ? -electricVal : electricVal);
					}
					break;
				case SIGNAL_CONNECTION_TYPE_FROM_TUNING:
					{
						double tuningVal = (point.percent() * (inParam.tuningHighBound().toDouble() - inParam.tuningLowBound().toDouble()) / 100) + inParam.tuningLowBound().toDouble();

						theSignalBase.tuning().appendCmdFowWrite(inParam.hash(), inParam.tuningValueType(), tuningVal);
					}
					break;
				default:
					assert(0);
			}
		}

		// wait ready all calibrators,
		// wait until all calibrators will has fixed electric value
		//
		for(int ch = 0; ch < channelCount; ch ++)
		{
			CalibratorManager* pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
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
		emit measureInfo(tr("Wait timeout for point %1 / %2 ").arg(pt + 1).arg(pointCount));
		waitMeasureTimeout();

		// phase saving of results started
		//
		emit measureInfo(tr("Save measurement "));

		channelCount = m_activeIoParamList.count();
		for(int ch = 0; ch < channelCount; ch ++)
		{
			CalibratorManager* pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
			if (calibratorIsValid(pCalibratorManager) == false)
			{
				continue;
			}

			if (m_activeIoParamList[ch].isValid() == false)
			{
				continue;
			}

			m_activeIoParamList[ch].setPercent(point.percent());

			LinearityMeasurement* pMeasurement = new LinearityMeasurement(m_activeIoParamList[ch]);
			if (pMeasurement == nullptr)
			{
				continue;
			}

			emit measureComplite(pMeasurement);
		}
		//
		// phase saving of results is over

		emit measureInfo(tr(""));
	}

	restoreStateTunSignals();
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureComprators()
{
	quint64 COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_0 = 0;
	quint64 COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_1 = 0;
	quint64 currentStateComparatorsInAllChannels = COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_0;

	// get max amount of comparators
	// get state for all comparators in state of logical "1"
	//
	int maxComparatorCount = 0;

	int channelCount = m_activeIoParamList.count();
	for(int ch = 0; ch < channelCount; ch++)
	{
		COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_1 |= (0x1ULL << ch);

		const Metrology::SignalParam& inParam = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
		if (inParam.isValid() == false)
		{
			continue;
		}

		if (maxComparatorCount < inParam.comparatorCount())
		{
			maxComparatorCount = inParam.comparatorCount();
		}
	}

	if (maxComparatorCount == 0)
	{
		emit msgBox(QMessageBox::Information, tr("Selected signal has no comparators"));
		return;
	}

	// starting from startComparatorIndex
	//
	int startComparatorIndex = theOptions.comparator().startComparatorIndex();

	// iterate over all the comparators from one to maxComparatorCount
	//
	for (int cmp = startComparatorIndex; cmp < maxComparatorCount; cmp++)
	{
		// phase of preparation started
		// switching the all comparators to logical 0
		//
		do
		{
			// for every comparator
			// Goal of preparations for switching the comparator to logical 0
			// 1 - go below return zone to switch comparator to logical 0 state
			// 2 - set the starting value, which will be as close as possible to the state of logical 1, but not reach it in a few steps
			//
			for (int pr = 0; pr < MEASURE_THREAD_CMP_PREPARE_COUNT; pr++)
			{
				emit measureInfo(tr("Comparator %1, Prepare %2").arg(cmp + 1).arg(pr + 1));

				// set electric value on calibrators, depend from comparator value
				//
				for(int ch = 0; ch < channelCount; ch ++)
				{
					CalibratorManager* pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
					if (calibratorIsValid(pCalibratorManager) == false)
					{
						continue;
					}

					const Metrology::SignalParam& inParam = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
					if (inParam.isValid() == false)
					{
						continue;
					}

					if (inParam.comparatorCount() == 0)
					{
						continue;
					}

					std::shared_ptr<Comparator> pComparator = inParam.comparator(cmp);
					if (pComparator == nullptr)
					{
						continue;
					}

					// get compare value
					//
					double compareVal = 0;

					if (pComparator->compare().isConst() == true)
					{
						compareVal = pComparator->compare().constValue();
					}
					else
					{
						if (pComparator->compare().appSignalID().isEmpty() == false)
						{
							Metrology::Signal* pComareSignal = theSignalBase.signalPtr(pComparator->compare().appSignalID());
							if (pComareSignal == nullptr)
							{
								continue;
							}
							else
							{
								compareVal = pComareSignal->state().value();
							}
						}
						else
						{
							continue;
						}
					}

					// get hysteresis value
					//
					double hysteresisVal = 0;

					if (pComparator->hysteresis().isConst() == true)
					{
						hysteresisVal = pComparator->hysteresis().constValue();
					}
					else
					{
						if (pComparator->hysteresis().appSignalID().isEmpty() == false)
						{
							Metrology::Signal* pHysteresisSignal = theSignalBase.signalPtr(pComparator->hysteresis().appSignalID());
							if (pHysteresisSignal == nullptr)
							{
								continue;
							}
							else
							{
								hysteresisVal = pHysteresisSignal->state().value();
							}
						}
						else
						{
							continue;
						}
					}

					// calc start value for comaprator
					//
					double startValueForComapre = ((inParam.highEngineeringUnits() - inParam.lowEngineeringUnits()) * theOptions.comparator().startValueForCompare()) / 100.0 ;

					//
					//
					double deltaVal = 0;

					switch (pr)
					{
						case MEASURE_THREAD_CMP_PREAPRE_1:	deltaVal = hysteresisVal * 2;		break;	// 1 - go below return zone to switch comparator to logical 0 state
						case MEASURE_THREAD_CMP_PREAPRE_2:	deltaVal = startValueForComapre;	break;	// 2 - set the starting value, which will be as close as possible to the state of logical 1, but not reach it in a few steps
						default:							continue;
					}

					double engineeringVal = 0;

					switch (pComparator->cmpType())
					{
						case E::CmpType::Equal:		engineeringVal = compareVal - deltaVal;		break;
						case E::CmpType::Greate:	engineeringVal = compareVal - deltaVal;		break;
						case E::CmpType::Less:		engineeringVal = compareVal + deltaVal;		break;
						case E::CmpType::NotEqual:	engineeringVal = compareVal;				break;
						default:					continue;
					}

					double electricVal = conversion(engineeringVal, CT_ENGINEER_TO_ELECTRIC, inParam);

					if (electricVal < inParam.electricLowLimit())
					{
						electricVal = inParam.electricLowLimit();
					}
					if (electricVal > inParam.electricHighLimit())
					{
						electricVal = inParam.electricHighLimit();
					}

					polarityTest(electricVal, m_activeIoParamList[ch]);	// polarity test

					pCalibratorManager->setValue(m_activeIoParamList[ch].isNegativeRange() ? -electricVal : electricVal);
				}

				// wait ready all calibrators,
				// wait until all calibrators will has fixed electric value
				//
				for(int ch = 0; ch < channelCount; ch ++)
				{
					CalibratorManager* pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
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

				// wait timeout for preparation
				//
				waitMeasureTimeout();
			}

			// if state of all comparstors != logical 0, thеn finish phase of preparations and go to measure
			// looking for comparators that did not switch to the logical state 1
			//
			currentStateComparatorsInAllChannels = COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_1;
			//
			//
			QString strInvalidComaprators = tr("Comparstor %1, for following signals, is already in state of logical \"1\":\n\n").arg(cmp + 1);

			for(int ch = 0; ch < channelCount; ch ++)
			{
				CalibratorManager* pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
				if (calibratorIsValid(pCalibratorManager) == false)
				{
					currentStateComparatorsInAllChannels &= ~(0x1ULL << ch);
					continue;
				}

				const Metrology::SignalParam& inParam = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
				if (inParam.isValid() == false)
				{
					currentStateComparatorsInAllChannels &= ~(0x1ULL << ch);
					continue;
				}

				if (inParam.comparatorCount() == 0)
				{
					currentStateComparatorsInAllChannels &= ~(0x1ULL << ch);
					continue;
				}

				std::shared_ptr<Comparator> pComparator = inParam.comparator(cmp);
				if (pComparator == nullptr)
				{
					currentStateComparatorsInAllChannels &= ~(0x1ULL << ch);
					continue;
				}

				if (pComparator->output().appSignalID().isEmpty() == true)
				{
					currentStateComparatorsInAllChannels &= ~(0x1ULL << ch);
					continue;
				}

				Metrology::Signal* pOutputSignal = pOutputSignal = theSignalBase.signalPtr(pComparator->output().appSignalID());
				if (pOutputSignal == nullptr)
				{
					currentStateComparatorsInAllChannels &= ~(0x1ULL << ch);
					continue;
				}

				// if  state of comparator = logical 0, then you do not need to wait
				//
				if (pOutputSignal->state().value() == 0.0)
				{
					currentStateComparatorsInAllChannels &= ~(0x1ULL << ch);
				}
				else
				{
					strInvalidComaprators.append(tr("%1\n").arg(inParam.customAppSignalID()));
				}
			}

			// if state of all comparstors = logical 0, go to measure
			// else repeat preparation
			//
			if (currentStateComparatorsInAllChannels == COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_0)
			{
				break;
			}

			strInvalidComaprators.append(tr("\nDo you want to repeat the preparation process in order to switch the comparator to state of logical \"0\"?"));

			int result = QMessageBox::NoButton;
			emit msgBox(QMessageBox::Question, strInvalidComaprators, &result);
			if (result == QMessageBox::No)
			{
				currentStateComparatorsInAllChannels = COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_0;
				break;
			}

		} while(m_cmdStopMeasure == false);
		//
		// phase of preparation is over


		// phase of measuring started
		// Okey - go
		//
		int step = 0;

		while (currentStateComparatorsInAllChannels != COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_1)
		{
			emit measureInfo(tr("Comparator %1, Step %2").arg(cmp + 1).arg(step + 1));

			currentStateComparatorsInAllChannels = COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_0;

			for(int ch = 0; ch < channelCount; ch ++)
			{
				CalibratorManager* pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
				if (calibratorIsValid(pCalibratorManager) == false)
				{
					currentStateComparatorsInAllChannels |= (0x1ULL << ch);
					continue;
				}

				const Metrology::SignalParam& inParam = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
				if (inParam.isValid() == false)
				{
					currentStateComparatorsInAllChannels |= (0x1ULL << ch);
					continue;
				}

				if (inParam.comparatorCount() == 0)
				{
					currentStateComparatorsInAllChannels |= (0x1ULL << ch);
					continue;
				}

				std::shared_ptr<Comparator> pComparator = inParam.comparator(cmp);
				if (pComparator == nullptr)
				{
					currentStateComparatorsInAllChannels |= (0x1ULL << ch);
					continue;
				}

				if (pComparator->output().appSignalID().isEmpty() == true)
				{
					currentStateComparatorsInAllChannels |= (0x1ULL << ch);
					continue;
				}

				Metrology::Signal* pOutputSignal = theSignalBase.signalPtr(pComparator->output().appSignalID());
				if (pOutputSignal == nullptr)
				{
					currentStateComparatorsInAllChannels |= (0x1ULL << ch);
					continue;
				}

				// if state of comparator = logical 1, then skip it
				// if state of comparator = logical 0, take a step
				//
				if (pOutputSignal->state().value() == 0.0)
				{
					switch (pComparator->cmpType())
					{
						case E::CmpType::Equal:		m_activeIoParamList[ch].isNegativeRange() == false ? pCalibratorManager->stepUp()	:	pCalibratorManager->stepDown(); break;
						case E::CmpType::Greate:	m_activeIoParamList[ch].isNegativeRange() == false ? pCalibratorManager->stepUp()	:	pCalibratorManager->stepDown(); break;
						case E::CmpType::Less:		m_activeIoParamList[ch].isNegativeRange() == false ? pCalibratorManager->stepDown() :	pCalibratorManager->stepUp(); 	break;
						case E::CmpType::NotEqual:	m_activeIoParamList[ch].isNegativeRange() == false ? pCalibratorManager->stepDown()	:	pCalibratorManager->stepUp();	break;
						default:					continue;
					}
				}
				else
				{
					currentStateComparatorsInAllChannels |= (0x1ULL << ch);
					continue;
				}
			}

			step++;

			// wait ready all calibrators,
			// wait until all calibrators will has fixed electric value
			//
			for(int ch = 0; ch < channelCount; ch ++)
			{
				CalibratorManager* pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
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
			waitMeasureTimeout();
		}
		//
		// phase of measuring is over


		// phase saving of results started
		//
		emit measureInfo(tr("Save measurement "));

		channelCount = m_activeIoParamList.count();
		for(int ch = 0; ch < channelCount; ch ++)
		{
			CalibratorManager* pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
			if (calibratorIsValid(pCalibratorManager) == false)
			{
				continue;
			}

			const Metrology::SignalParam& inParam = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
			if (inParam.isValid() == false)
			{
				continue;
			}

			if (inParam.comparatorCount() == 0)
			{
				continue;
			}

			std::shared_ptr<Comparator> pComparator = inParam.comparator(cmp);
			if (pComparator == nullptr)
			{
				continue;
			}

			m_activeIoParamList[ch].setComparatorIndex(cmp);

			ComparatorMeasurement* pMeasurement = new ComparatorMeasurement(m_activeIoParamList[ch]);
			if (pMeasurement == nullptr)
			{
				continue;
			}

			emit measureComplite(pMeasurement);
		}
		//
		// phase saving of results is over

		emit measureInfo(tr(""));
	}
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
	for(int ch = 0; ch < channelCount; ch ++)
	{
		if (m_activeIoParamList[ch].isValid() == false)
		{
			continue;
		}

		Metrology::SignalParam tunParam = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
		if (tunParam.isValid() == false)
		{
			continue;
		}

		m_activeIoParamList[ch].setTunSignalState(theSignalBase.signalState(tunParam.hash()).value());
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
	for(int ch = 0; ch < channelCount; ch ++)
	{
		if (m_activeIoParamList[ch].isValid() == false)
		{
			continue;
		}

		Metrology::SignalParam tunParam = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
		if (tunParam.isValid() == false)
		{
			continue;
		}

		theSignalBase.tuning().appendCmdFowWrite(tunParam.hash(), tunParam.tuningValueType(), m_activeIoParamList[ch].tunSignalState());
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
	for(int ch = 0; ch < channelCount; ch ++)
	{
		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type ++)
		{
			if (m_activeIoParamList[ch].param(type).appSignalID() == appSignalID)
			{
				m_activeIoParamList[ch].setParam(type, theSignalBase.signalParam(appSignalID));
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
