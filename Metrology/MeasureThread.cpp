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
	connect(this, &MeasureThread::finished, this, &MeasureThread::stopMeasure);
}

// -------------------------------------------------------------------------------------------------------------------

MeasureThread::~MeasureThread()
{
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
	if (m_measureKind == MEASURE_KIND_ONE_MODULE)
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

	switch (m_signalConnectionType)
	{
		case SIGNAL_CONNECTION_TYPE_UNUSED:			signal = activeSignal.multiChannelSignal(MEASURE_IO_SIGNAL_TYPE_INPUT);		break;
		default:									signal = activeSignal.multiChannelSignal(MEASURE_IO_SIGNAL_TYPE_OUTPUT);	break;
	}

	if (signal.isEmpty() == true)
	{
		return false;
	}

	bool isMeasured = false;

	for(int ch = 0; ch < activeSignal.channelCount(); ch++)
	{
		Metrology::Signal* pMetrologySignal = signal.metrologySignal(ch);
		if (pMetrologySignal == nullptr || pMetrologySignal->param().isValid() == false)
		{
			continue;
		}

		StatisticsItem si;

		switch (m_measureType)
		{
			case MEASURE_TYPE_LINEARITY:
				{
					si.setSignal(pMetrologySignal);
				}
				break;

			case MEASURE_TYPE_COMPARATOR:
				{
					si.setSignal(pMetrologySignal);

					int startComparatorIndex = theOptions.comparator().startComparatorIndex();
					if (startComparatorIndex < 0 || startComparatorIndex >= pMetrologySignal->param().comparatorCount())
					{
						continue;
					}

					si.setComparator(pMetrologySignal->param().comparator(startComparatorIndex));
				}

				break;

			default:
				assert(0);
				break;
		}

		theMeasureBase.updateStatistics(m_measureType, si);
		if (si.isMeasured() == true)
		{
			signalID.append(pMetrologySignal->param().appSignalID() + "\n");

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

	Metrology::SignalParam param;

	int channelCount = m_activeIoParamList.count();
	for(int ch = 0; ch < channelCount; ch ++)
	{
		if (m_activeIoParamList[ch].isValid() == false)
		{
			continue;
		}

		switch (m_signalConnectionType)
		{
			case SIGNAL_CONNECTION_TYPE_UNUSED:
			case SIGNAL_CONNECTION_TYPE_INPUT_INTERNAL:
			case SIGNAL_CONNECTION_TYPE_INPUT_DP_TO_INTERNAL_F:
			case SIGNAL_CONNECTION_TYPE_INPUT_C_TO_INTERNAL_F:	param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);	break;
			case SIGNAL_CONNECTION_TYPE_INPUT_OUTPUT:
			case SIGNAL_CONNECTION_TYPE_INPUT_DP_TO_OUTPUT_F:
			case SIGNAL_CONNECTION_TYPE_INPUT_C_TO_OUTPUT_F:
			case SIGNAL_CONNECTION_TYPE_TUNING_OUTPUT:			param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);	break;
			default:											assert(0);
		}

		if (eltalonIsFound == false)
		{
			eltalonIsFound = true;

			electricLowLimit = param.electricLowLimit();
			electricHighLimit = param.electricHighLimit();
			electricUnitID = param.electricUnitID();
			electricSensorType = param.electricSensorType();
		}
		else
		{
			if (compareDouble(electricLowLimit, param.electricLowLimit()) == false ||
				compareDouble(electricHighLimit, param.electricHighLimit()) == false ||
				electricUnitID != param.electricUnitID() ||
				electricSensorType != param.electricSensorType())
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
	if (m_signalConnectionType != SIGNAL_CONNECTION_TYPE_TUNING_OUTPUT)
	{
		if (getConnectedCalibrators() == 0)
		{
			stopMeasure();
		}
	}

	// Timeout for Measure
	//
	for(int t = 0; t <= m_measureTimeout; t += MEASURE_THREAD_TIMEOUT_STEP)
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

				if (m_measureKind == MEASURE_KIND_ONE_MODULE)
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

					if (m_activeIoParamList[ch].signalConnectionType() == SIGNAL_CONNECTION_TYPE_TUNING_OUTPUT)
					{
						const Metrology::SignalParam& outParam = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
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
					else
					{
						const Metrology::SignalParam& inParam = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
						if (inParam.isValid() == false)
						{
							continue;
						}

						if (prepareCalibrator(pCalibratorManager, CALIBRATOR_MODE_SOURCE, inParam.electricUnitID(), inParam.electricHighLimit()) == false)
						{
							emit msgBox(QMessageBox::Information, QString("Calibrator: %1 - can not set source mode.").arg(pCalibratorManager->calibratorPort()));
						}

						const Metrology::SignalParam& outParam = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
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

	int calibratorUnit = CALIBRATOR_UNIT_UNDEFINED;

	switch(signalUnit)
	{
		case E::ElectricUnit::mA:	calibratorUnit = CALIBRATOR_UNIT_MA;	break;
		case E::ElectricUnit::mV:	calibratorUnit = CALIBRATOR_UNIT_MV;	break;
		case E::ElectricUnit::V:	calibratorUnit = CALIBRATOR_UNIT_V;		break;
		case E::ElectricUnit::Ohm:
			{
				// Minimal range for calibrators TRX-II and Calys75 this is 400 Ohm
				//
				if (electricHighLimit <= CALIBRATOR_MINIMAL_RANGE_OHM)
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

	bool result =  pCalibratorManager->setUnit(calibratorMode, calibratorUnit);

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
			emit setNextMeasureSignal(signalIsSelected); // call signal how - Qt::BlockingQueuedConnection and return signalIsSelected, if it == true - enable measure next signal, if it == false - dont measure next signal
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

		emit measureInfo(tr("Point %1 / %2 ").arg(pt + 1).arg(pointCount));

		// set electric value on calibrators, depend from point value
		//
		int channelCount = m_activeIoParamList.count();

		if (m_measureKind == MEASURE_KIND_ONE_MODULE)
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

			// set electric value or tuning value
			//
			if (m_activeIoParamList[ch].signalConnectionType() == SIGNAL_CONNECTION_TYPE_TUNING_OUTPUT)
			{
				double tuningVal = (point.percent() * (inParam.tuningHighBound().toDouble() - inParam.tuningLowBound().toDouble()) / 100) + inParam.tuningLowBound().toDouble();

				theSignalBase.tuning().appendCmdFowWrite(inParam.hash(), inParam.tuningValueType(), tuningVal);
			}
			else
			{
				// at the beginning we need get engineering value because if range is not Linear (for instance Ohm or mV)
				// then by engineering value we may get electric value
				//
				double engineeringVal = (point.percent() * (inParam.highEngineeringUnits() - inParam.lowEngineeringUnits()) / 100) + inParam.lowEngineeringUnits();
				double electricVal = conversion(engineeringVal, CT_ENGINEER_TO_ELECTRIC, inParam);

				polarityTest(electricVal, m_activeIoParamList[ch]);	// polarity test

				pCalibratorManager->setValue(m_activeIoParamList[ch].isNegativeRange() ? -electricVal : electricVal);
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
				if (pCalibratorManager->calibratorIsConnected() == false)
				{
					break;
				}

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

		Metrology::SignalParam param;

		switch (m_activeIoParamList[ch].signalConnectionType())
		{
			case SIGNAL_CONNECTION_TYPE_UNUSED:	param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);	break;
			default:							param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);	break;
		}

		if (param.isValid() == false)
		{
			continue;
		}

		if (maxComparatorCount < param.comparatorCount())
		{
			maxComparatorCount = param.comparatorCount();
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
		bool goToNextComparator = false;

		// we must do two pass
		// first pass to measure set point of comparator
		// second pass to measure set point of hysteresis
		//
		for(int cmpValueType = 0; cmpValueType < Metrology::CmpValueTypeCount; cmpValueType ++)
		{
			//
			//
			if (theOptions.comparator().enableMeasureHysteresis() == false && cmpValueType == Metrology::CmpValueTypeHysteresis)
			{
				break;  // go to next comparator
			}

			// select active output state
			//
			bool activeOutputState = false;	// output state of comparator, denend from pass

			switch (cmpValueType)
			{
				case Metrology::CmpValueTypeSetPoint:	activeOutputState = false;	break;
				case Metrology::CmpValueTypeHysteresis:	activeOutputState = true;	break;
			}

			// phase of preparation started
			// switching the all comparators to logical 0
			//
			do
			{
				// Two preparations
				// The purpose of preparations for switching the comparator to logical 0, and go to starting value
				// 1 - go below return zone to switch comparator to logical 0
				// 2 - set the starting value, which will be as close as possible to the state of logical 1, but not reach it in a few steps
				//
				for (int pr = 0; pr < MEASURE_THREAD_CMP_PREPARE_COUNT; pr++)
				{
					switch (cmpValueType)
					{
						case Metrology::CmpValueTypeSetPoint:	emit measureInfo(tr("Comparator %1, Prepare %2").arg(cmp + 1).arg(pr + 1));					break;
						case Metrology::CmpValueTypeHysteresis:	emit measureInfo(tr("Hysteresis of comparator %1, Prepare %2").arg(cmp + 1).arg(pr + 1));	break;
					}

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

						Metrology::SignalParam param;

						switch (m_activeIoParamList[ch].signalConnectionType())
						{
							case SIGNAL_CONNECTION_TYPE_UNUSED:	param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);	break;
							default:							param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);	break;
						}

						if (param.isValid() == false || param.hasComparators() == false)
						{
							continue;
						}

						std::shared_ptr<Metrology::ComparatorEx> comparatorEx = param.comparator(cmp);
						if (comparatorEx == nullptr || comparatorEx->signalsIsValid() == false)
						{
							continue;
						}

						//
						//
						double compareVal = comparatorEx->compareOnlineValue(cmpValueType);		// get compare value
						double hysteresisVal = comparatorEx->hysteresisOnlineValue();			// get hysteresis value

						// calc start value for comaprator
						//
						double startValueForComapre = ((param.highEngineeringUnits() - param.lowEngineeringUnits()) * theOptions.comparator().startValueForCompare()) / 100.0;

						//
						//
						double deltaVal = 0;

						switch (pr)
						{
							case MEASURE_THREAD_CMP_PREAPRE_1:		// 1 - go below return zone to switch comparator to logical 0 state

								if (comparatorEx->deviation() == Metrology::ComparatorEx::DeviationType::Unused)
								{
									deltaVal = hysteresisVal * 2;	// for comparators Less and Greate
								}
								else
								{
									deltaVal = hysteresisVal / 2;	// for comparators Equal and NotEqual
								}

								break;

							case MEASURE_THREAD_CMP_PREAPRE_2:		// 2 - set the starting value, which will be as close as possible to the state of logical 1, but not reach it in a few steps

								deltaVal = startValueForComapre;

								break;

							default:
								continue;
						}

						double engineeringVal = 0;

						switch (cmpValueType)
						{
							case Metrology::CmpValueTypeSetPoint:

								switch (comparatorEx->cmpType())
								{
									case E::CmpType::Less:		engineeringVal = compareVal + deltaVal;	break;	// becomes higher than the set point (if the set point is Less)
									case E::CmpType::Greate:	engineeringVal = compareVal - deltaVal;	break;	// falls below the set point (if the set point for Greate)
									default:					continue;
								}

								break;

							case Metrology::CmpValueTypeHysteresis:

								switch (comparatorEx->cmpType())
								{
									case E::CmpType::Less:		engineeringVal = compareVal - deltaVal;	break;	// becomes higher than the hysteresis (if the hysteresis is Less)
									case E::CmpType::Greate:	engineeringVal = compareVal + deltaVal;	break;	// falls below the hysteresis (if the hysteresis for Greate)
									default:					continue;
								}

								break;
						}


						double engineeringCalcVal = conversionCalcVal(engineeringVal, CT_CALC_VAL_INVERSION, m_activeIoParamList[ch].signalConnectionType(), m_activeIoParamList[ch]);

						double electricVal = conversion(engineeringCalcVal, CT_ENGINEER_TO_ELECTRIC, inParam);


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
							if (pCalibratorManager->calibratorIsConnected() == false)
							{
								break;
							}

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

					// additional delay = 10 sec
					// before starting the measure test, all comparators must be in logical 0
					// maybe for someone the comparator did not have enough damper time to switch to logical 0
					//
					if (pr == MEASURE_THREAD_CMP_PREAPRE_1)
					{
						switch (cmpValueType)
						{
							case Metrology::CmpValueTypeSetPoint:	emit measureInfo(tr("Comparator %1, additional delay").arg(cmp + 1));				break;
							case Metrology::CmpValueTypeHysteresis:	emit measureInfo(tr("Hysteresis of comparator %1, additional delay").arg(cmp + 1));	break;
						}

						int timeoutStep = m_measureTimeout / 100;

						currentStateComparatorsInAllChannels = COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_1;

						for (int t = 0; t < 100; t++ )
						{
							for(int ch = 0; ch < channelCount; ch ++)
							{
								CalibratorManager* pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
								if (calibratorIsValid(pCalibratorManager) == false)
								{
									currentStateComparatorsInAllChannels &= ~(0x1ULL << ch);
									continue;
								}

								Metrology::SignalParam param;

								switch (m_activeIoParamList[ch].signalConnectionType())
								{
									case SIGNAL_CONNECTION_TYPE_UNUSED:	param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);	break;
									default:							param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);	break;
								}

								if (param.isValid() == false || param.hasComparators() == false)
								{
									currentStateComparatorsInAllChannels &= ~(0x1ULL << ch);
									continue;
								}

								std::shared_ptr<Metrology::ComparatorEx> comparatorEx = param.comparator(cmp);
								if (comparatorEx == nullptr || comparatorEx->signalsIsValid() == false)
								{
									currentStateComparatorsInAllChannels &= ~(0x1ULL << ch);
									continue;
								}

								// if  state of comparator = logical 0, then you do not need to wait
								//
								if (comparatorEx->outputState() == activeOutputState)
								{
									currentStateComparatorsInAllChannels &= ~(0x1ULL << ch);
								}
							}

							// if state of all comparators = logical 0, go to measure
							//
							if (currentStateComparatorsInAllChannels == COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_0)
							{
								break;
							}

							QThread::msleep(MEASURE_THREAD_TIMEOUT_STEP);

							emit measureInfo((t+1) * timeoutStep);
						}

						emit measureInfo(0);
					}
				}

				// before starting the test, all comparators must be in a logical 0
				// if state of all comparators = logical 0, thеn finish phase of preparations and go to measure
				// looking for comparators that did not switch to logical 1
				// if at least one of the comparators did not switch to logical 0, then we issue messages and repeat the preparations
				//
				currentStateComparatorsInAllChannels = COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_1;
				//
				//
				QString strInvalidComaprators = tr(	"Comparator %1, for following signals, is already in state of logical \"%2\":\n\n")
													.arg(cmp + 1)
													.arg(static_cast<int>(!activeOutputState));

				for(int ch = 0; ch < channelCount; ch ++)
				{
					CalibratorManager* pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
					if (calibratorIsValid(pCalibratorManager) == false)
					{
						currentStateComparatorsInAllChannels &= ~(0x1ULL << ch);
						continue;
					}

					Metrology::SignalParam param;

					switch (m_activeIoParamList[ch].signalConnectionType())
					{
						case SIGNAL_CONNECTION_TYPE_UNUSED:	param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);	break;
						default:							param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);	break;
					}

					if (param.isValid() == false || param.hasComparators() == false)
					{
						currentStateComparatorsInAllChannels &= ~(0x1ULL << ch);
						continue;
					}

					std::shared_ptr<Metrology::ComparatorEx> comparatorEx = param.comparator(cmp);
					if (comparatorEx == nullptr || comparatorEx->signalsIsValid() == false)
					{
						currentStateComparatorsInAllChannels &= ~(0x1ULL << ch);
						continue;
					}

					// if  state of comparator = logical 0, then you do not need to wait
					//
					if (comparatorEx->outputState() == activeOutputState)
					{
						currentStateComparatorsInAllChannels &= ~(0x1ULL << ch);
					}
					else
					{
						strInvalidComaprators.append(tr("%1\n").arg(param.appSignalID()));
					}
				}

				// if state of all comparators = logical 0, go to measure
				// else repeat preparation
				//
				if (currentStateComparatorsInAllChannels == COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_0)
				{
					break;
				}

				strInvalidComaprators.append(tr(	"\nDo you want to repeat the preparation process in order to switch the comparator to state of logical \"%1\", click \"Yes\". Go to next comparator, click \"No\"")
													.arg(static_cast<int>(activeOutputState)));

				int result = QMessageBox::NoButton;
				emit msgBox(QMessageBox::Question, strInvalidComaprators, &result);
				if (result == QMessageBox::No)
				{
					currentStateComparatorsInAllChannels = COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_0;
					goToNextComparator = true;
					break;
				}

			} while(m_cmdStopMeasure == false);
			//
			// phase of preparation is over

			if (goToNextComparator == true)
			{
				break;
			}

			// phase of measuring started
			// Okey - go
			//
			int step = 0;

			while (currentStateComparatorsInAllChannels != COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_1)
			{
				switch (cmpValueType)
				{
					case Metrology::CmpValueTypeSetPoint:	emit measureInfo(tr("Comparator %1, Step %2").arg(cmp + 1).arg(step + 1));					break;
					case Metrology::CmpValueTypeHysteresis:	emit measureInfo(tr("Hysteresis of comparator %1, Step %2").arg(cmp + 1).arg(step + 1));	break;
				}

				currentStateComparatorsInAllChannels = COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_0;

				for(int ch = 0; ch < channelCount; ch ++)
				{
					CalibratorManager* pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
					if (calibratorIsValid(pCalibratorManager) == false)
					{
						currentStateComparatorsInAllChannels |= (0x1ULL << ch);
						continue;
					}

					Metrology::SignalParam param;

					switch (m_activeIoParamList[ch].signalConnectionType())
					{
						case SIGNAL_CONNECTION_TYPE_UNUSED:	param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);	break;
						default:							param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);	break;
					}

					if (param.isValid() == false || param.hasComparators() == false)
					{
						currentStateComparatorsInAllChannels |= (0x1ULL << ch);
						continue;
					}

					std::shared_ptr<Metrology::ComparatorEx> comparatorEx = param.comparator(cmp);
					if (comparatorEx == nullptr || comparatorEx->signalsIsValid() == false)
					{
						currentStateComparatorsInAllChannels |= (0x1ULL << ch);
						continue;
					}

					// if state of comparator = logical 1, then skip it
					// if state of comparator = logical 0, take a step
					//
					if (comparatorEx->outputState() == activeOutputState)
					{
						switch (cmpValueType)
						{
							case Metrology::CmpValueTypeSetPoint:

								switch (comparatorEx->cmpType())
								{
									case E::CmpType::Greate:	m_activeIoParamList[ch].isNegativeRange() == false ? pCalibratorManager->stepUp()	:	pCalibratorManager->stepDown(); break;
									case E::CmpType::Less:		m_activeIoParamList[ch].isNegativeRange() == false ? pCalibratorManager->stepDown() :	pCalibratorManager->stepUp(); 	break;
									default:					continue;
								}

								break;

							case Metrology::CmpValueTypeHysteresis:

								switch (comparatorEx->cmpType())
								{
									case E::CmpType::Greate:	m_activeIoParamList[ch].isNegativeRange() == false ? pCalibratorManager->stepDown()	:	pCalibratorManager->stepUp();	break;
									case E::CmpType::Less:		m_activeIoParamList[ch].isNegativeRange() == false ? pCalibratorManager->stepUp() :		pCalibratorManager->stepDown();	break;
									default:					continue;
								}

								break;
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
						if (pCalibratorManager->calibratorIsConnected() == false)
						{
							break;
						}

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

				Metrology::SignalParam param;

				switch (m_activeIoParamList[ch].signalConnectionType())
				{
					case SIGNAL_CONNECTION_TYPE_UNUSED:	param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);	break;
					default:							param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);	break;
				}

				if (param.isValid() == false || param.hasComparators() == false)
				{
					continue;
				}

				std::shared_ptr<Metrology::ComparatorEx> comparatorEx = param.comparator(cmp);
				if (comparatorEx == nullptr || comparatorEx->signalsIsValid() == false)
				{
					continue;
				}

				m_activeIoParamList[ch].setComparatorIndex(cmp);
				m_activeIoParamList[ch].setComparatorValueType(cmpValueType);

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
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::signalSocketDisconnected()
{
	stopMeasure();
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::tuningSocketDisconnected()
{
	if (m_signalConnectionType != SIGNAL_CONNECTION_TYPE_TUNING_OUTPUT)
	{
		return;
	}

	stopMeasure();
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::saveStateTunSignals()
{
	if (m_signalConnectionType != SIGNAL_CONNECTION_TYPE_TUNING_OUTPUT)
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

		const Metrology::SignalParam& tunParam = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
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
	if (m_signalConnectionType != SIGNAL_CONNECTION_TYPE_TUNING_OUTPUT)
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

		const Metrology::SignalParam& tunParam = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
		if (tunParam.isValid() == false)
		{
			continue;
		}

		theSignalBase.tuning().appendCmdFowWrite(tunParam.hash(), tunParam.tuningValueType(), m_activeIoParamList[ch].tunSignalState());
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureTypeChanged(int type)
{
	if (type < 0 || type >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	m_measureType = type;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureKindChanged(int kind)
{
	if (kind < 0 || kind >= MEASURE_KIND_COUNT)
	{
		return;
	}

	m_measureKind = kind;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::signalConnectionTypeChanged(int type)
{
	if (type < 0 || type >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		return;
	}

	m_signalConnectionType = type;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureTimeoutChanged(int timeout)
{
	m_measureTimeout = timeout;
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
	m_cmdStopMeasure = true;
}

// -------------------------------------------------------------------------------------------------------------------
