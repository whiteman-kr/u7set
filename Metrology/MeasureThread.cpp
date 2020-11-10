#include "MeasureThread.h"

#include <assert.h>
#include <QTime>

#include "../lib/UnitsConvertor.h"

#include "CalibratorBase.h"
#include "Conversion.h"
#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureThreadInfo::MeasureThreadInfo()
{
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThreadInfo::init()
{
	m_message.clear();
	m_timeout = 0;

	m_cmdStopMeasure = false;
	m_exitCode = ExitCode::Usual;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThreadInfo::setMessage(const QString& message, const msgType& type)
{
	m_type = type;
	m_message = message;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThreadInfo::setTimeout(int timeout)
{
	m_type = msgType::Timeout;
	m_timeout = timeout;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
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

//			if (param.physicalRangeIsValid() == false ||
//				param.engineeringRangeIsValid() == false ||
//				param.electricRangeIsValid() == false)
//			{
//				continue;
//			}

			CalibratorManager* pCalibratorManager = theCalibratorBase.calibratorForMeasure(ch);
			if (pCalibratorManager == nullptr || pCalibratorManager->calibratorIsConnected() == false)
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

void MeasureThread::waitMeasureTimeout()
{
	if (m_signalConnectionType != SIGNAL_CONNECTION_TYPE_TUNING_OUTPUT)
	{
		if (getConnectedCalibrators() == 0)
		{
			return;
		}
	}

	// Timeout for Measure
	//
	for(int t = 0; t <= m_measureTimeout; t += MEASURE_THREAD_TIMEOUT_STEP)
	{
		if (m_info.cmdStopMeasure() == true)
		{
			break;
		}

		QThread::msleep(MEASURE_THREAD_TIMEOUT_STEP);

		m_info.setTimeout(MEASURE_THREAD_TIMEOUT_STEP + t);
		emit sendMeasureInfo(m_info);
	}

	m_info.setTimeout(0);
	emit sendMeasureInfo(m_info);
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

	if (m_info.cmdStopMeasure() == true)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

int MeasureThread::getConnectedCalibrators()
{
	if (m_info.cmdStopMeasure()== true)
	{
		return 0;
	}

	int connectedCalibratorCount = 0;

	int channelCount = m_activeIoParamList.count();
	for(int ch = 0; ch < channelCount; ch ++)
	{
		CalibratorManager* pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
		if (pCalibratorManager == nullptr || pCalibratorManager->calibratorIsConnected() == false)
		{
			continue;
		}

		connectedCalibratorCount++;
	}

	if (connectedCalibratorCount == 0)
	{
		emit msgBox(QMessageBox::Information, tr("No connected calibrators for measuring!"));
		stopMeasure(MeasureThreadInfo::ExitCode::Usual);
	}

	return connectedCalibratorCount;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureThread::setCalibratorUnit()
{
	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		assert(0);
		return false;
	}

	if (getConnectedCalibrators() == 0)
	{
		return false;
	}

	// select calibrator mode and calibrator unit
	// depend from analog signal
	//

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
		if (pCalibratorManager == nullptr)
		{
			continue;
		}

		Calibrator*	pCalibrator = pCalibratorManager->calibrator();
		if (pCalibrator == nullptr || pCalibrator->isConnected() == false)
		{
			return false;
		}

		if (m_activeIoParamList[ch].signalConnectionType() != SIGNAL_CONNECTION_TYPE_TUNING_OUTPUT)
		{
			const Metrology::SignalParam& inParam = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
			if (inParam.isValid() == false)
			{
				continue;
			}

			if (prepareCalibrator(pCalibratorManager,
								  CALIBRATOR_MODE_SOURCE,
								  inParam.electricUnitID(),
								  inParam.electricHighLimit()) == false)
			{
				emit msgBox(QMessageBox::Information,
							tr("Calibrator: %1 - %2 can not set source mode.").
							arg(pCalibrator->typeStr()).
							arg(pCalibrator->portName()));
				m_activeIoParamList[ch].setCalibratorManager(nullptr);
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

			if (prepareCalibrator(pCalibratorManager,
								  CALIBRATOR_MODE_MEASURE,
								  outParam.electricUnitID(),
								  outParam.electricHighLimit()) == false)
			{
				emit msgBox(QMessageBox::Information,
							tr("Calibrator: %1 - %2 can not set measure mode.").
							arg(pCalibrator->typeStr()).
							arg(pCalibrator->portName()));

				m_activeIoParamList[ch].setCalibratorManager(nullptr);
			}
		}
		else
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

			if (prepareCalibrator(pCalibratorManager,
								  CALIBRATOR_MODE_MEASURE,
								  outParam.electricUnitID(),
								  outParam.electricHighLimit()) == false)
			{
				emit msgBox(QMessageBox::Information,
							tr("Calibrator: %1 - %2 can not set measure mode.").
							arg(pCalibrator->typeStr()).
							arg(pCalibrator->portName()));
				m_activeIoParamList[ch].setCalibratorManager(nullptr);
			}
		}
	}



	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureThread::prepareCalibrator(CalibratorManager* pCalibratorManager, int calibratorMode, E::ElectricUnit signalUnit, double electricHighLimit)
{
	if (pCalibratorManager == nullptr)
	{
		return false;
	}

	Calibrator*	pCalibrator = pCalibratorManager->calibrator();
	if (pCalibrator == nullptr || pCalibrator->isConnected() == false)
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

	CalibratorLimit limit = pCalibrator->getLimit(calibratorMode, calibratorUnit);
	if (limit.isValid() == false)
	{
		emit msgBox(QMessageBox::Critical,	tr("Calibrator: %1 - %2 can not set unit \"%3\".").
											arg(pCalibrator->typeStr()).
											arg(pCalibrator->portName()).
											arg(qApp->translate("Calibrator.h", CalibratorUnit[calibratorUnit])));
		return false;
	}


	m_info.setMessage(tr("Prepare calibrator: %1 %2").arg(pCalibrator->typeStr()).arg(pCalibrator->portName()));
	emit sendMeasureInfo(m_info);

	bool result = pCalibratorManager->setUnit(calibratorMode, calibratorUnit);

	switch (pCalibrator->type())
	{
		case CALIBRATOR_TYPE_TRXII:		QThread::msleep(1000);	break;
		case CALIBRATOR_TYPE_CALYS75:	QThread::msleep(500);	break;
		case CALIBRATOR_TYPE_KTHL6221:	QThread::msleep(500);	break;
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
		emit msgBox(QMessageBox::Information,
					tr("Please, switch polarity for calibrator %1\n"
					   "You have used the negative (-) part of the electrical range.").
					arg(ioParam.calibratorManager()->calibratorChannel() + 1));
	}

	if (electricVal >= negativeLimit && ioParam.isNegativeRange() == true)
	{
		ioParam.setNegativeRange(false);
		emit msgBox(QMessageBox::Information,
					tr("Please, switch polarity for calibrator %1\n"
					   "You have used the positive (+) part of the electrical range.").
					arg(ioParam.calibratorManager()->calibratorChannel() + 1));
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::run()
{
	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		assert(0);
		return;
	}

	m_info.init();

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
		default:						assert(0);				break;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureLinearity()
{
	// test - have programm measure points
	//
	int pointCount = m_linearityOption.points().count();
	if (m_linearityOption.points().count() == 0)
	{
		emit msgBox(QMessageBox::Information, tr("No points for measure"));
		return;
	}

	UnitsConvertor uc;

	saveStateTunSignals();

	for(int pt = 0; pt < pointCount; pt++)
	{
		MeasurePoint point = m_linearityOption.points().point(pt);

		m_info.setMessage(tr("Point %1 / %2 ").arg(pt + 1).arg(pointCount));
		emit sendMeasureInfo(m_info);

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
			if (m_activeIoParamList[ch].signalConnectionType() != SIGNAL_CONNECTION_TYPE_TUNING_OUTPUT)
			{
				// at the beginning we need get engineering value because if range is not Linear (for instance Ohm or mV)
				// then by engineering value we may get electric value
				//
				double engineeringVal = (point.percent() *
										 (inParam.highEngineeringUnits() - inParam.lowEngineeringUnits()) / 100) +
										 inParam.lowEngineeringUnits();

				double electricVal = uc.conversion(engineeringVal, UnitsConvertType::PhysicalToElectric, inParam);

				polarityTest(electricVal, m_activeIoParamList[ch]);	// polarity test

				pCalibratorManager->setValue(m_activeIoParamList[ch].isNegativeRange() ? -electricVal : electricVal);
			}
			else
			{
				double tuningVal = (point.percent() *
									(inParam.tuningHighBound().toDouble() - inParam.tuningLowBound().toDouble()) / 100) +
									inParam.tuningLowBound().toDouble();

				theSignalBase.tuning().appendCmdFowWrite(inParam.hash(), inParam.tuningValueType(), tuningVal);
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
				if (calibratorIsValid(pCalibratorManager) == false)
				{
					break;
				}

				msleep(1);
			}
		}

		// wait timeout for measure
		//
		m_info.setMessage(tr("Wait timeout for point %1 / %2 ").arg(pt + 1).arg(pointCount));
		emit sendMeasureInfo(m_info);

		waitMeasureTimeout();

		// phase saving of results started
		//
		m_info.setMessage(tr("Save measurement "));
		emit sendMeasureInfo(m_info);

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

		m_info.setMessage(QString());
		emit sendMeasureInfo(m_info);
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
			case SIGNAL_CONNECTION_TYPE_UNUSED:
				param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
				break;
			default:
				param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
				break;
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

	UnitsConvertor uc;

	// starting from startComparatorIndex
	//
	int startComparatorIndex = m_comparatorOption.startComparatorIndex();

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
			if (m_comparatorOption.enableMeasureHysteresis() == false && cmpValueType == Metrology::CmpValueTypeHysteresis)
			{
				break;  // go to next comparator
			}

			// select active output state
			//
			bool activeDiscreteOutputState = false;	// output state of comparator, denend from pass

			switch (cmpValueType)
			{
				case Metrology::CmpValueTypeSetPoint:	activeDiscreteOutputState = false;	break;
				case Metrology::CmpValueTypeHysteresis:	activeDiscreteOutputState = true;	break;
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
						case Metrology::CmpValueTypeSetPoint:
							m_info.setMessage(tr("Comparator %1, Prepare %2").arg(cmp + 1).arg(pr + 1));
							break;
						case Metrology::CmpValueTypeHysteresis:
							m_info.setMessage(tr("Hysteresis of comparator %1, Prepare %2").arg(cmp + 1).arg(pr + 1));
							break;
					}

					emit sendMeasureInfo(m_info);

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
							case SIGNAL_CONNECTION_TYPE_UNUSED:
								param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
								break;
							default:
								param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
								break;
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
						double startValueForComapre = ((param.highEngineeringUnits() - param.lowEngineeringUnits()) *
													   m_comparatorOption.startValueForCompare()) / 100.0;

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


						double engineeringCalcVal = conversionCalcVal(engineeringVal,
																	  ConversionCalcType::Inversion,
																	  m_activeIoParamList[ch].signalConnectionType(),
																	  m_activeIoParamList[ch]);

						double electricVal = uc.conversion(engineeringCalcVal, UnitsConvertType::PhysicalToElectric, inParam);


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
							if (calibratorIsValid(pCalibratorManager) == false)
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
							case Metrology::CmpValueTypeSetPoint:
								m_info.setMessage(tr("Comparator %1, additional delay").arg(cmp + 1));
								break;
							case Metrology::CmpValueTypeHysteresis:
								m_info.setMessage(tr("Hysteresis of comparator %1, additional delay").arg(cmp + 1));
								break;
						}

						emit sendMeasureInfo(m_info);

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
									case SIGNAL_CONNECTION_TYPE_UNUSED:
										param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
										break;
									default:
										param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
										break;
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
								if (comparatorEx->outputState() == activeDiscreteOutputState)
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

							m_info.setTimeout((t+1) * timeoutStep);
							emit sendMeasureInfo(m_info);
						}

						m_info.setTimeout(0);
						emit sendMeasureInfo(m_info);
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
													.arg(static_cast<int>(!activeDiscreteOutputState));

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
						case SIGNAL_CONNECTION_TYPE_UNUSED:
							param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
							break;
						default:
							param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
							break;
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
					if (comparatorEx->outputState() == activeDiscreteOutputState)
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

				strInvalidComaprators.append(tr(	"\nDo you want to repeat the preparation process "
													"in order to switch the comparator "
													"to state of logical \"%1\", click \"Yes\". "
													"Go to next comparator, click \"No\"")
													.arg(static_cast<int>(activeDiscreteOutputState)));

				int result = QMessageBox::NoButton;
				emit msgBox(QMessageBox::Question, strInvalidComaprators, &result);
				if (result == QMessageBox::No)
				{
					currentStateComparatorsInAllChannels = COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_0;
					goToNextComparator = true;
					break;
				}

			} while(m_info.cmdStopMeasure() == false);
			//
			// phase of preparation is over

			if (goToNextComparator == true)
			{
				break;
			}

			MeasureThreadInfo::msgType msgType = MeasureThreadInfo::msgType::String;

			// phase of measuring started
			// Okey - go
			//
			int step = 0;

			while (currentStateComparatorsInAllChannels != COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_1)
			{
				switch (cmpValueType)
				{
					case Metrology::CmpValueTypeSetPoint:
						m_info.setMessage(tr("Comparator %1, Step %2").arg(cmp + 1).arg(step + 1), msgType);
						break;
					case Metrology::CmpValueTypeHysteresis:
						m_info.setMessage(tr("Hysteresis of comparator %1, Step %2").arg(cmp + 1).arg(step + 1), msgType);
						break;
				}

				emit sendMeasureInfo(m_info);

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
						case SIGNAL_CONNECTION_TYPE_UNUSED:
							param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
							break;
						default:
							param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
							break;
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
					if (comparatorEx->outputState() == activeDiscreteOutputState)
					{
						switch (cmpValueType)
						{
							case Metrology::CmpValueTypeSetPoint:

								switch (comparatorEx->cmpType())
								{
									case E::CmpType::Greate:
										m_activeIoParamList[ch].isNegativeRange() == false ? pCalibratorManager->stepUp()	:
																							 pCalibratorManager->stepDown();
										break;

									case E::CmpType::Less:
										m_activeIoParamList[ch].isNegativeRange() == false ? pCalibratorManager->stepDown() :
																							 pCalibratorManager->stepUp();
										break;

									default:
										continue;
								}

								break;

							case Metrology::CmpValueTypeHysteresis:

								switch (comparatorEx->cmpType())
								{
									case E::CmpType::Greate:
										m_activeIoParamList[ch].isNegativeRange() == false ? pCalibratorManager->stepDown()	:
																							 pCalibratorManager->stepUp();
										break;

									case E::CmpType::Less:
										m_activeIoParamList[ch].isNegativeRange() == false ? pCalibratorManager->stepUp() :
																							 pCalibratorManager->stepDown();
										break;

									default:
										continue;
								}

								break;
						}

						// red string in the StatusBar reduce error >= limit of error
						//
						const Metrology::SignalParam& inParam = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
						if (inParam.isValid() == true)
						{
							double comporatorEtalonVal = comparatorEx->compareOnlineValue(cmpValueType);		// get compare or hyst value

							double engineeringEtalonCalcVal = conversionCalcVal(comporatorEtalonVal,
																				ConversionCalcType::Inversion,
																				m_activeIoParamList[ch].signalConnectionType(),
																				m_activeIoParamList[ch]);

							double electricEtalonVal = uc.conversion(engineeringEtalonCalcVal, UnitsConvertType::PhysicalToElectric, inParam);

							double reduceError = (std::abs(pCalibratorManager->calibrator()->sourceValue() - electricEtalonVal) /
												  inParam.electricHighLimit() - inParam.electricLowLimit()) * 100;

							if (reduceError >= m_comparatorOption.errorLimit())
							{
								msgType = MeasureThreadInfo::msgType::StringError;
							}
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
						if (calibratorIsValid(pCalibratorManager) == false)
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
			m_info.setMessage(tr("Save measurement "));
			emit sendMeasureInfo(m_info);

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
					case SIGNAL_CONNECTION_TYPE_UNUSED:
						param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_INPUT);
						break;
					default:
						param = m_activeIoParamList[ch].param(MEASURE_IO_SIGNAL_TYPE_OUTPUT);
						break;
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

			m_info.setMessage(QString());
			emit sendMeasureInfo(m_info);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::signalSocketDisconnected()
{
	stopMeasure(MeasureThreadInfo::ExitCode::Usual);
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::tuningSocketDisconnected()
{
	if (m_signalConnectionType == SIGNAL_CONNECTION_TYPE_TUNING_OUTPUT)
	{
		stopMeasure(MeasureThreadInfo::ExitCode::Usual);
	}
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

		if (tunParam.enableTuning() == false)
		{
			continue;
		}

		m_activeIoParamList[ch].setTunStateForRestore(theSignalBase.signalState(tunParam.hash()).value());
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

		if (tunParam.enableTuning() == false)
		{
			continue;
		}

		theSignalBase.tuning().appendCmdFowWrite(tunParam.hash(),
												 tunParam.tuningValueType(),
												 m_activeIoParamList[ch].tunStateForRestore());
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureTimeoutChanged(int timeout)
{
	m_measureTimeout = timeout;
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

void MeasureThread::activeSignalChanged(const MeasureSignal& activeSignal)
{
	if (activeSignal.isEmpty() == true)
	{
		return;
	}

	setActiveSignalParam(activeSignal);
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::signalParamChanged(const QString& appSignalID)
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

void MeasureThread::stopMeasure(MeasureThreadInfo::ExitCode exitCode)
{
	m_info.setCmdStopMeasure(true);
	m_info.setExitCode(exitCode);
}

// -------------------------------------------------------------------------------------------------------------------
