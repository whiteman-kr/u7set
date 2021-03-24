#include "MeasureThread.h"

#include <assert.h>
#include <QTime>

#include "../lib/UnitsConvertor.h"

#include "Conversion.h"
#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureThreadInfo::MeasureThreadInfo()
{
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThreadInfo::clear()
{
	m_message.clear();
	m_timeout = 0;

	m_exitCode = ExitCode::Program;
	m_cmdStopMeasure = false;
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

MeasureThread::MeasureThread(QObject* parent) :
	QThread(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

MeasureThread::~MeasureThread()
{
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureThread::setActiveSignalParam(const MeasureSignal& activeSignal, const CalibratorBase& calibratorBase)
{
	m_activeIoParamList.clear();

	// create param list for measure
	//
	int channelCount = activeSignal.channelCount();
	for(int ch = 0; ch < channelCount; ch ++)
	{
		IoSignalParam ioParam;

		for(int ioType = 0; ioType < Metrology::ConnectionIoTypeCount; ioType ++)
		{
			Metrology::Signal* pSignal = activeSignal.multiChannelSignal(ioType).metrologySignal(ch);
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

			std::shared_ptr<CalibratorManager> pCalibratorManager = calibratorBase.calibratorForMeasure(ch);
			if (pCalibratorManager == nullptr || pCalibratorManager->calibratorIsConnected() == false)
			{
				continue;
			}

			ioParam.setParam(ioType, param);
			ioParam.setConnectionType(activeSignal.connectionType());
			ioParam.setCalibratorManager(pCalibratorManager);
		}

		m_activeIoParamList.push_back(ioParam);
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::waitMeasureTimeout()
{
	if (m_connectionType != Metrology::ConnectionType::Tuning_Output)
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

bool MeasureThread::calibratorIsValid(std::shared_ptr<CalibratorManager> pCalibratorManager)
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

	for(const IoSignalParam& ioParam : m_activeIoParamList)
	{
		std::shared_ptr<CalibratorManager> pCalibratorManager = ioParam.calibratorManager();
		if (pCalibratorManager == nullptr || pCalibratorManager->calibratorIsConnected() == false)
		{
			continue;
		}

		connectedCalibratorCount++;
	}

	if (connectedCalibratorCount == 0)
	{
		emit msgBox(QMessageBox::Information, tr("No connected calibrators for measuring!"));
		stopMeasure(MeasureThreadInfo::ExitCode::Program);
	}

	return connectedCalibratorCount;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureThread::setCalibratorUnit()
{
	if (ERR_MEASURE_TYPE(m_measureType) == true)
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

	quint64 channelCount = m_activeIoParamList.size();

	if (m_measureKind == Measure::Kind::OneRack || m_measureKind == Measure::Kind::OneModule)
	{
		channelCount = 1;
	}

	for(quint64 ch = 0; ch < channelCount; ch ++)
	{
		if (m_activeIoParamList[ch].isValid() == false)
		{
			continue;
		}

		std::shared_ptr<CalibratorManager> pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
		if (pCalibratorManager == nullptr)
		{
			continue;
		}

		std::shared_ptr<Calibrator>	pCalibrator = pCalibratorManager->calibrator();
		if (pCalibrator == nullptr || pCalibrator->isConnected() == false)
		{
			return false;
		}

		if (m_activeIoParamList[ch].connectionType() != Metrology::ConnectionType::Tuning_Output)
		{
			const Metrology::SignalParam& inParam = m_activeIoParamList[ch].param(Metrology::ConnectionIoType::Source);
			if (inParam.isValid() == false)
			{
				continue;
			}

			if (prepareCalibrator(pCalibratorManager,
								  CalibratorMode::SourceMode,
								  inParam.electricUnitID(),
								  inParam.electricHighLimit()) == false)
			{
				emit msgBox(QMessageBox::Information,
							tr("Calibrator: %1 - %2 can not set source mode.").
							arg(pCalibrator->typeStr()).
							arg(pCalibrator->portName()));
				m_activeIoParamList[ch].setCalibratorManager(nullptr);
			}

			const Metrology::SignalParam& outParam = m_activeIoParamList[ch].param(Metrology::ConnectionIoType::Destination);
			if (outParam.isValid() == false)
			{
				continue;
			}

			if (outParam.isOutput() == false)
			{
				continue;
			}

			if (prepareCalibrator(pCalibratorManager,
								  CalibratorMode::MeasureMode,
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
			const Metrology::SignalParam& outParam = m_activeIoParamList[ch].param(Metrology::ConnectionIoType::Destination);
			if (outParam.isValid() == false)
			{
				continue;
			}

			if (outParam.isOutput() == false)
			{
				continue;
			}

			if (prepareCalibrator(pCalibratorManager,
								  CalibratorMode::MeasureMode,
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

bool MeasureThread::prepareCalibrator(std::shared_ptr<CalibratorManager> pCalibratorManager, CalibratorMode calibratorMode, E::ElectricUnit signalUnit, double electricHighLimit)
{
	if (pCalibratorManager == nullptr)
	{
		return false;
	}

	std::shared_ptr<Calibrator>	pCalibrator = pCalibratorManager->calibrator();
	if (pCalibrator == nullptr || pCalibrator->isConnected() == false)
	{
		return false;
	}

	if (ERR_CALIBRATOR_MODE(calibratorMode) == true)
	{
		assert(0);
		return false;
	}

	CalibratorUnit calibratorUnit = CalibratorUnit::NoCalibratorUnit;

	switch(signalUnit)
	{
		case E::ElectricUnit::mA:	calibratorUnit = CalibratorUnit::mA;	break;
		case E::ElectricUnit::uA:	calibratorUnit = CalibratorUnit::uA;	break;
		case E::ElectricUnit::mV:	calibratorUnit = CalibratorUnit::mV;	break;
		case E::ElectricUnit::V:	calibratorUnit = CalibratorUnit::V;		break;
		case E::ElectricUnit::Hz:	calibratorUnit = CalibratorUnit::Hz;	break;
		case E::ElectricUnit::Ohm:
			{
				// Minimal range for calibrators TRX-II and Calys75 this is 400 Ohm
				//
				if (electricHighLimit <= CalibratorMinimalRangeOhm)
				{
					calibratorUnit = CalibratorUnit::OhmLow;
				}
				else
				{
					calibratorUnit = CalibratorUnit::OhmHigh;
				}
			}

			break;

		default:
			assert(0);
	}

	if (ERR_CALIBRATOR_UNIT(calibratorUnit) == true)
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
											arg(qApp->translate("Calibrator", CalibratorUnitCaption(calibratorUnit).toUtf8())));
		return false;
	}


	m_info.setMessage(tr("Prepare calibrator: %1 %2").arg(pCalibrator->typeStr()).arg(pCalibrator->portName()));
	emit sendMeasureInfo(m_info);

	bool result = pCalibratorManager->setUnit(calibratorMode, calibratorUnit);

	switch (pCalibrator->type())
	{
		case CalibratorType::TrxII:		QThread::msleep(1000);	break;
		case CalibratorType::Calys75:	QThread::msleep(500);	break;
		case CalibratorType::Ktl6221:	QThread::msleep(500);	break;
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

	std::shared_ptr<Calibrator> pCalibrator = ioParam.calibratorManager()->calibrator();
	if (pCalibrator == nullptr)
	{
		return;
	}

	if (pCalibrator->type() == CalibratorType::Ktl6221)
	{
		return;
	}

	double negativeLimit = 0;

	if (pCalibrator->mode() == CalibratorMode::SourceMode && pCalibrator->sourceUnit() == CalibratorUnit::mV)
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

	// Since only one calibrator is used for Measure::Kind::OneModule, then changes need to be made in all channels
	//
	if (m_measureType == Measure::Type::Linearity && m_measureKind == Measure::Kind::OneModule)
	{
		for(IoSignalParam& ioSignalParam : m_activeIoParamList)
		{
			ioSignalParam.setNegativeRange(ioParam.isNegativeRange());
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::run()
{
	if (ERR_MEASURE_TYPE(m_measureType) == true)
	{
		assert(0);
		return;
	}

	m_info.clear();

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
		case Measure::Type::Linearity:
			measureLinearity();
			break;

		case Measure::Type::Comparators:

			switch (m_measureKind)
			{
				case Measure::Kind::OneRack:
				case Measure::Kind::OneModule:
					measureCompratorsInSeries();
					break;

				case Measure::Kind::MultiRack:
					measureCompratorsInParallel();
					break;

				default:
					assert(0);
					break;
			}
			break;

		default:
			assert(0);
			break;
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
		Measure::Point point = m_linearityOption.points().point(pt);

		m_info.setMessage(tr("Point %1 / %2 ").arg(pt + 1).arg(pointCount));
		emit sendMeasureInfo(m_info);

		// set electric value on calibrators, depend from point value
		//
		quint64 channelCount = m_activeIoParamList.size();

		if (m_measureKind == Measure::Kind::OneRack || m_measureKind == Measure::Kind::OneModule)
		{
			channelCount = 1;
		}

		for(quint64 ch = 0; ch < channelCount; ch ++)
		{
			std::shared_ptr<CalibratorManager> pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
			if (calibratorIsValid(pCalibratorManager) == false)
			{
				continue;
			}

			if (m_activeIoParamList[ch].isValid() == false)
			{
				continue;
			}

			const Metrology::SignalParam& inParam = m_activeIoParamList[ch].param(Metrology::ConnectionIoType::Source);
			if (inParam.isValid() == false)
			{
				continue;
			}

			// set electric value or tuning value
			//
			if (m_activeIoParamList[ch].connectionType() != Metrology::ConnectionType::Tuning_Output)
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
		for(quint64 ch = 0; ch < channelCount; ch ++)
		{
			std::shared_ptr<CalibratorManager> pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
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

		for(IoSignalParam& ioParam : m_activeIoParamList)
		{
			std::shared_ptr<CalibratorManager> pCalibratorManager = ioParam.calibratorManager();
			if (calibratorIsValid(pCalibratorManager) == false)
			{
				continue;
			}

			if (ioParam.isValid() == false)
			{
				continue;
			}

			ioParam.setPercent(point.percent());

			Measure::LinearityItem* pMeasurement = new Measure::LinearityItem(ioParam);
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

void MeasureThread::measureCompratorsInSeries()
{
	UnitsConvertor uc;

	for(IoSignalParam& ioParam : m_activeIoParamList)
	{
		std::shared_ptr<CalibratorManager> pCalibratorManager = ioParam.calibratorManager();
		if (calibratorIsValid(pCalibratorManager) == false)
		{
			continue;
		}

		// get max amount of comparators
		//
		int comparatorCount = 0;

		Metrology::SignalParam param;

		switch (ioParam.connectionType())
		{
			case Metrology::ConnectionType::Unused:
				param = ioParam.param(Metrology::ConnectionIoType::Source);
				break;
			default:
				param = ioParam.param(Metrology::ConnectionIoType::Destination);
				break;
		}

		if (param.isValid() == false)
		{
			continue;
		}

		comparatorCount = param.comparatorCount();
		if (comparatorCount == 0)
		{
			continue;
		}

		// starting from startComparatorIndex
		//
		int startComparatorIndex = m_comparatorOption.startComparatorIndex();

		// iterate over all the comparators from one to maxComparatorCount
		//
		for (int cmp = startComparatorIndex; cmp < comparatorCount; cmp++)
		{
			std::shared_ptr<Metrology::ComparatorEx> comparatorEx = param.comparator(cmp);
			if (comparatorEx == nullptr || comparatorEx->signalsIsValid() == false)
			{
				break;
			}

			bool goToNextComparator = false;

			// we must do two pass
			// first pass to measure set point of comparator
			// second pass to measure set point of hysteresis
			//
			for(int cmpValueType = 0; cmpValueType < Metrology::CmpValueTypeCount; cmpValueType ++)
			{
				//
				//
				if (cmpValueType == Metrology::CmpValueType::Hysteresis && m_comparatorOption.enableMeasureHysteresis() == false)
				{
					break;  // go to next comparator
				}

				// select active output state
				//
				bool activeDiscreteOutputState = false;	// output state of comparator, depended from pass

				switch (cmpValueType)
				{
					case Metrology::CmpValueType::SetPoint:		activeDiscreteOutputState = false;	break;
					case Metrology::CmpValueType::Hysteresis:	activeDiscreteOutputState = true;	break;
				}

				// phase of preparation started
				// switching comparator to logical 0
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
							case Metrology::CmpValueType::SetPoint:
								m_info.setMessage(tr("Comparator %1, Prepare %2").arg(cmp + 1).arg(pr + 1));
								break;
							case Metrology::CmpValueType::Hysteresis:
								m_info.setMessage(tr("Hysteresis of comparator %1, Prepare %2").arg(cmp + 1).arg(pr + 1));
								break;
						}

						emit sendMeasureInfo(m_info);

						// get compare value and hysteresis value
						//
						double compareVal = comparatorEx->compareOnlineValue(cmpValueType);		// get compare value
						double hysteresisVal = comparatorEx->hysteresisOnlineValue();			// get hysteresis value

						// calc start value for comaprator by engineering range
						//
						double startValueForComapre = ((param.highEngineeringUnits() - param.lowEngineeringUnits()) *
													   m_comparatorOption.startValueForCompare()) / 100.0;

						// calc delta value
						//
						double deltaVal = 0;

						switch (pr)
						{
							// 1 - go below return zone to switch comparator to logical 0 state
							//
							case MEASURE_THREAD_CMP_PREAPRE_1:

								if (comparatorEx->deviation() == Metrology::ComparatorEx::DeviationType::Unused)
								{
									deltaVal = hysteresisVal * 2;	// for comparators Less and Greate
								}
								else
								{
									deltaVal = hysteresisVal / 2;	// for comparators Equal and NotEqual
								}

								break;

							// 2 - set the starting value, which will be as close as possible to the state of logical 1,
							// but not reach it in a few steps
							//
							case MEASURE_THREAD_CMP_PREAPRE_2:

								deltaVal = startValueForComapre;

								break;

							default:
								break;
						}

						double engineeringVal = 0;

						switch (cmpValueType)
						{
							case Metrology::CmpValueType::SetPoint:

								switch (comparatorEx->cmpType())
								{
									case E::CmpType::Less:		engineeringVal = compareVal + deltaVal;	break;	// becomes higher than the set point (if the set point is Less)
									case E::CmpType::Greate:	engineeringVal = compareVal - deltaVal;	break;	// falls below the set point (if the set point for Greate)
									default:					break;
								}

								break;

							case Metrology::CmpValueType::Hysteresis:

								switch (comparatorEx->cmpType())
								{
									case E::CmpType::Less:		engineeringVal = compareVal - deltaVal;	break;	// becomes higher than the hysteresis (if the hysteresis is Less)
									case E::CmpType::Greate:	engineeringVal = compareVal + deltaVal;	break;	// falls below the hysteresis (if the hysteresis for Greate)
									default:					break;
								}

								break;
						}


						double engineeringCalcVal = conversionByConnection(engineeringVal, ioParam, ConversionDirection::Inversion);

						const Metrology::SignalParam& inParam = ioParam.param(Metrology::ConnectionIoType::Source);
						if (inParam.isValid() == false)
						{
							break;
						}

						double electricVal = uc.conversion(engineeringCalcVal, UnitsConvertType::PhysicalToElectric, inParam);

						if (electricVal < inParam.electricLowLimit())
						{
							electricVal = inParam.electricLowLimit();
						}
						if (electricVal > inParam.electricHighLimit())
						{
							electricVal = inParam.electricHighLimit();
						}

						polarityTest(electricVal, ioParam);	// polarity test

						// set electric value on calibrators, depend from comparator value
						//
						if (calibratorIsValid(pCalibratorManager) == false)
						{
							break;
						}

						pCalibratorManager->setValue(ioParam.isNegativeRange() ? -electricVal : electricVal);

						// wait ready of calibrator,
						// wait until calibrator will has fixed electric value
						//
						while(pCalibratorManager->isReadyForManage() != true)
						{
							if (calibratorIsValid(pCalibratorManager) == false)
							{
								break;
							}

							msleep(1);
						}

						// wait timeout for preparation
						//
						waitMeasureTimeout();

						// additional delay = 10 sec
						// before starting the measure test, comparator must be in logical 0
						// maybe for someone the comparator did not have enough time to switch to logical 0
						//
						if (pr == MEASURE_THREAD_CMP_PREAPRE_1)
						{
							switch (cmpValueType)
							{
								case Metrology::CmpValueType::SetPoint:
									m_info.setMessage(tr("Comparator %1, additional delay").arg(cmp + 1));
									break;
								case Metrology::CmpValueType::Hysteresis:
									m_info.setMessage(tr("Hysteresis of comparator %1, additional delay").arg(cmp + 1));
									break;
							}

							emit sendMeasureInfo(m_info);

							int timeoutStep = m_measureTimeout / 100;

							for (int t = 0; t < 100; t++ )
							{
								if (calibratorIsValid(pCalibratorManager) == false)
								{
									break;
								}

								// if  state of comparator = logical 0, then you do not need to wait
								//
								if (comparatorEx->outputState() == activeDiscreteOutputState)
								{
									break;
								}

								QThread::msleep(MEASURE_THREAD_TIMEOUT_STEP);

								m_info.setTimeout((t+1) * timeoutStep);
								emit sendMeasureInfo(m_info);
							}

							m_info.setTimeout(0);
							emit sendMeasureInfo(m_info);
						} // end of additional delay
					} // end one of preparations

					if (calibratorIsValid(pCalibratorManager) == false)
					{
						break;
					}

					// before starting the test, comparator must be in a logical 0
					// if state of comparator = logical 0, thеn finish phase of preparations and go to measure
					// looking for comparator that did not switch to logical 1
					// if comparator did not switch to logical 0, then we issue messages and repeat the preparations
					//
					// if  state of comparator = logical 0, then you do not need to wait
					//
					if (comparatorEx->outputState() == activeDiscreteOutputState)
					{
						break;
					}
					else
					{
						QString strInvalidComaprator = tr(	"Comparator %1, for following signals, is already in state of logical \"%2\":\n\n")
															.arg(cmp + 1)
															.arg(static_cast<int>(!activeDiscreteOutputState));

						strInvalidComaprator.append(tr("%1\n").arg(param.appSignalID()));

						strInvalidComaprator.append(tr(	"\nDo you want to repeat the preparation process "
															"in order to switch the comparator "
															"to state of logical \"%1\", click \"Yes\". "
															"Go to next comparator, click \"No\"")
															.arg(static_cast<int>(activeDiscreteOutputState)));


						// if state comparator = logical 0, go to measure
						// else repeat preparation
						//
						int result = QMessageBox::NoButton;
						emit msgBox(QMessageBox::Question, strInvalidComaprator, &result);
						if (result == QMessageBox::No)
						{
							goToNextComparator = true;
							break;
						}
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

				while (comparatorEx->outputState() == activeDiscreteOutputState)
				{
					switch (cmpValueType)
					{
						case Metrology::CmpValueType::SetPoint:
							m_info.setMessage(tr("Comparator %1, Step %2").arg(cmp + 1).arg(step + 1), msgType);
							break;
						case Metrology::CmpValueType::Hysteresis:
							m_info.setMessage(tr("Hysteresis of comparator %1, Step %2").arg(cmp + 1).arg(step + 1), msgType);
							break;
						default:
							break;
					}

					emit sendMeasureInfo(m_info);

					if (calibratorIsValid(pCalibratorManager) == false)
					{
						break;
					}

					// if state of comparator = logical 1, then skip it
					// if state of comparator = logical 0, take a step
					//
					if (comparatorEx->outputState() == activeDiscreteOutputState)
					{
						switch (cmpValueType)
						{
							case Metrology::CmpValueType::SetPoint:

								switch (comparatorEx->cmpType())
								{
									case E::CmpType::Greate:	ioParam.isNegativeRange() == false ? pCalibratorManager->stepUp()	:
																									 pCalibratorManager->stepDown();
										break;

									case E::CmpType::Less:		ioParam.isNegativeRange() == false ? pCalibratorManager->stepDown() :
																									 pCalibratorManager->stepUp();
										break;

									default:
										break;
								}

								break;

							case Metrology::CmpValueType::Hysteresis:

								switch (comparatorEx->cmpType())
								{
									case E::CmpType::Greate:	ioParam.isNegativeRange() == false ? pCalibratorManager->stepDown()	:
																									 pCalibratorManager->stepUp();
										break;

									case E::CmpType::Less:		ioParam.isNegativeRange() == false ? pCalibratorManager->stepUp() :
																									 pCalibratorManager->stepDown();
										break;

									default:
										break;
								}

								break;

							default:
								break;
						}

						// red string in the StatusBar reduce error >= limit of error
						//
						const Metrology::SignalParam& inParam = ioParam.param(Metrology::ConnectionIoType::Source);
						if (inParam.isValid() == true)
						{
							double comporatorEtalonVal = comparatorEx->compareOnlineValue(cmpValueType);		// get compare or hyst value

							double engineeringEtalonCalcVal = conversionByConnection(comporatorEtalonVal, ioParam, ConversionDirection::Inversion);

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
						continue;
					}

					step++;

					// wait ready of calibrator,
					// wait until calibrator will has fixed electric value
					//
					while(pCalibratorManager->isReadyForManage() != true)
					{
						if (calibratorIsValid(pCalibratorManager) == false)
						{
							break;
						}

						msleep(1);
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

				if (calibratorIsValid(pCalibratorManager) == false)
				{
					break;
				}

				ioParam.setComparatorIndex(cmp);
				ioParam.setComparatorValueType(cmpValueType);

				Measure::ComparatorItem* pMeasurement = new Measure::ComparatorItem(ioParam);
				if (pMeasurement == nullptr)
				{
					continue;
				}

				emit measureComplite(pMeasurement);

				//
				// phase saving of results is over

				m_info.setMessage(QString());
				emit sendMeasureInfo(m_info);
			}
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureCompratorsInParallel()
{
	quint64 COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_0 = 0;
	quint64 COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_1 = 0;
	quint64 currentStateComparatorsInAllChannels = COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_0;

	// get max amount of comparators
	// get state for all comparators in state of logical "1"
	//
	int maxComparatorCount = 0;

	quint64 channelCount = m_activeIoParamList.size();
	for(quint64 ch = 0; ch < channelCount; ch++)
	{
		COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_1 |= (0x1ULL << ch);

		Metrology::SignalParam param;

		switch (m_activeIoParamList[ch].connectionType())
		{
			case Metrology::ConnectionType::Unused:
				param = m_activeIoParamList[ch].param(Metrology::ConnectionIoType::Source);
				break;
			default:
				param = m_activeIoParamList[ch].param(Metrology::ConnectionIoType::Destination);
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
		// emit msgBox(QMessageBox::Information, tr("Selected signal has no comparators"));
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
			if (cmpValueType == Metrology::CmpValueType::Hysteresis && m_comparatorOption.enableMeasureHysteresis() == false)
			{
				break;  // go to next comparator
			}

			// select active output state
			//
			bool activeDiscreteOutputState = false;	// output state of comparator, denend from pass

			switch (cmpValueType)
			{
				case Metrology::CmpValueType::SetPoint:		activeDiscreteOutputState = false;	break;
				case Metrology::CmpValueType::Hysteresis:	activeDiscreteOutputState = true;	break;
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
						case Metrology::CmpValueType::SetPoint:
							m_info.setMessage(tr("Comparator %1, Prepare %2").arg(cmp + 1).arg(pr + 1));
							break;
						case Metrology::CmpValueType::Hysteresis:
							m_info.setMessage(tr("Hysteresis of comparator %1, Prepare %2").arg(cmp + 1).arg(pr + 1));
							break;
					}

					emit sendMeasureInfo(m_info);

					// set electric value on calibrators, depend from comparator value
					//
					for(quint64 ch = 0; ch < channelCount; ch ++)
					{
						std::shared_ptr<CalibratorManager> pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
						if (calibratorIsValid(pCalibratorManager) == false)
						{
							continue;
						}

						const Metrology::SignalParam& inParam = m_activeIoParamList[ch].param(Metrology::ConnectionIoType::Source);
						if (inParam.isValid() == false)
						{
							continue;
						}

						Metrology::SignalParam param;

						switch (m_activeIoParamList[ch].connectionType())
						{
							case Metrology::ConnectionType::Unused:
								param = m_activeIoParamList[ch].param(Metrology::ConnectionIoType::Source);
								break;
							default:
								param = m_activeIoParamList[ch].param(Metrology::ConnectionIoType::Destination);
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
							case Metrology::CmpValueType::SetPoint:

								switch (comparatorEx->cmpType())
								{
									case E::CmpType::Less:		engineeringVal = compareVal + deltaVal;	break;	// becomes higher than the set point (if the set point is Less)
									case E::CmpType::Greate:	engineeringVal = compareVal - deltaVal;	break;	// falls below the set point (if the set point for Greate)
									default:					continue;
								}

								break;

							case Metrology::CmpValueType::Hysteresis:

								switch (comparatorEx->cmpType())
								{
									case E::CmpType::Less:		engineeringVal = compareVal - deltaVal;	break;	// becomes higher than the hysteresis (if the hysteresis is Less)
									case E::CmpType::Greate:	engineeringVal = compareVal + deltaVal;	break;	// falls below the hysteresis (if the hysteresis for Greate)
									default:					continue;
								}

								break;
						}


						double engineeringCalcVal = conversionByConnection(	engineeringVal,
																			m_activeIoParamList[ch],
																			ConversionDirection::Inversion);

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
					for(quint64 ch = 0; ch < channelCount; ch ++)
					{
						std::shared_ptr<CalibratorManager> pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
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
							case Metrology::CmpValueType::SetPoint:
								m_info.setMessage(tr("Comparator %1, additional delay").arg(cmp + 1));
								break;
							case Metrology::CmpValueType::Hysteresis:
								m_info.setMessage(tr("Hysteresis of comparator %1, additional delay").arg(cmp + 1));
								break;
						}

						emit sendMeasureInfo(m_info);

						int timeoutStep = m_measureTimeout / 100;

						currentStateComparatorsInAllChannels = COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_1;

						for (int t = 0; t < 100; t++ )
						{
							for(quint64 ch = 0; ch < channelCount; ch ++)
							{
								std::shared_ptr<CalibratorManager> pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
								if (calibratorIsValid(pCalibratorManager) == false)
								{
									currentStateComparatorsInAllChannels &= ~(0x1ULL << ch);
									continue;
								}

								Metrology::SignalParam param;

								switch (m_activeIoParamList[ch].connectionType())
								{
									case Metrology::ConnectionType::Unused:
										param = m_activeIoParamList[ch].param(Metrology::ConnectionIoType::Source);
										break;
									default:
										param = m_activeIoParamList[ch].param(Metrology::ConnectionIoType::Destination);
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

				for(quint64 ch = 0; ch < channelCount; ch ++)
				{
					std::shared_ptr<CalibratorManager> pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
					if (calibratorIsValid(pCalibratorManager) == false)
					{
						currentStateComparatorsInAllChannels &= ~(0x1ULL << ch);
						continue;
					}

					Metrology::SignalParam param;

					switch (m_activeIoParamList[ch].connectionType())
					{
						case Metrology::ConnectionType::Unused:
							param = m_activeIoParamList[ch].param(Metrology::ConnectionIoType::Source);
							break;
						default:
							param = m_activeIoParamList[ch].param(Metrology::ConnectionIoType::Destination);
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
					case Metrology::CmpValueType::SetPoint:
						m_info.setMessage(tr("Comparator %1, Step %2").arg(cmp + 1).arg(step + 1), msgType);
						break;
					case Metrology::CmpValueType::Hysteresis:
						m_info.setMessage(tr("Hysteresis of comparator %1, Step %2").arg(cmp + 1).arg(step + 1), msgType);
						break;
				}

				emit sendMeasureInfo(m_info);

				currentStateComparatorsInAllChannels = COMPARATORS_IN_ALL_CHANNELS_IN_LOGICAL_0;

				for(quint64 ch = 0; ch < channelCount; ch ++)
				{
					std::shared_ptr<CalibratorManager> pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
					if (calibratorIsValid(pCalibratorManager) == false)
					{
						currentStateComparatorsInAllChannels |= (0x1ULL << ch);
						continue;
					}

					Metrology::SignalParam param;

					switch (m_activeIoParamList[ch].connectionType())
					{
						case Metrology::ConnectionType::Unused:
							param = m_activeIoParamList[ch].param(Metrology::ConnectionIoType::Source);
							break;
						default:
							param = m_activeIoParamList[ch].param(Metrology::ConnectionIoType::Destination);
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
							case Metrology::CmpValueType::SetPoint:

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

							case Metrology::CmpValueType::Hysteresis:

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
						const Metrology::SignalParam& inParam = m_activeIoParamList[ch].param(Metrology::ConnectionIoType::Source);
						if (inParam.isValid() == true)
						{
							double comporatorEtalonVal = comparatorEx->compareOnlineValue(cmpValueType);	// get compare or hyst value

							double engineeringEtalonCalcVal = conversionByConnection(	comporatorEtalonVal,
																						m_activeIoParamList[ch],
																						ConversionDirection::Inversion);

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
				for(quint64 ch = 0; ch < channelCount; ch ++)
				{
					std::shared_ptr<CalibratorManager> pCalibratorManager = m_activeIoParamList[ch].calibratorManager();
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

			for(IoSignalParam& ioParam : m_activeIoParamList)
			{
				std::shared_ptr<CalibratorManager> pCalibratorManager = ioParam.calibratorManager();
				if (calibratorIsValid(pCalibratorManager) == false)
				{
					continue;
				}

				Metrology::SignalParam param;

				switch (ioParam.connectionType())
				{
					case Metrology::ConnectionType::Unused:
						param = ioParam.param(Metrology::ConnectionIoType::Source);
						break;
					default:
						param = ioParam.param(Metrology::ConnectionIoType::Destination);
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

				ioParam.setComparatorIndex(cmp);
				ioParam.setComparatorValueType(cmpValueType);

				Measure::ComparatorItem* pMeasurement = new Measure::ComparatorItem(ioParam);
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
	stopMeasure(MeasureThreadInfo::ExitCode::Program);
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::tuningSocketDisconnected()
{
	if (m_connectionType == Metrology::ConnectionType::Tuning_Output)
	{
		stopMeasure(MeasureThreadInfo::ExitCode::Program);
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::saveStateTunSignals()
{
	if (m_connectionType != Metrology::ConnectionType::Tuning_Output)
	{
		return;
	}

	for(IoSignalParam& ioParam : m_activeIoParamList)
	{
		if (ioParam.isValid() == false)
		{
			continue;
		}

		const Metrology::SignalParam& tunParam = ioParam.param(Metrology::ConnectionIoType::Source);
		if (tunParam.isValid() == false)
		{
			continue;
		}

		if (tunParam.enableTuning() == false)
		{
			continue;
		}

		ioParam.setTunStateForRestore(theSignalBase.signalState(tunParam.hash()).value());
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::restoreStateTunSignals()
{
	if (m_connectionType != Metrology::ConnectionType::Tuning_Output)
	{
		return;
	}

	for(const IoSignalParam& ioParam : m_activeIoParamList)
	{
		if (ioParam.isValid() == false)
		{
			continue;
		}

		const Metrology::SignalParam& tunParam = ioParam.param(Metrology::ConnectionIoType::Source);
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
												 ioParam.tunStateForRestore());
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureTimeoutChanged(int timeout)
{
	m_measureTimeout = timeout;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureTypeChanged(Measure::Type measureType)
{
	if (ERR_MEASURE_TYPE(measureType) == true)
	{
		return;
	}

	m_measureType = measureType;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::measureKindChanged(Measure::Kind measureKind)
{
	if (ERR_MEASURE_KIND(measureKind) == true)
	{
		return;
	}

	m_measureKind = measureKind;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::connectionTypeChanged(Metrology::ConnectionType connectionType)
{
	if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
	{
		return;
	}

	m_connectionType = connectionType;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::signalParamChanged(const QString& appSignalID)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(0);
		return;
	}

	for(IoSignalParam& ioParam : m_activeIoParamList)
	{
		for(int ioType = 0; ioType < Metrology::ConnectionIoTypeCount; ioType ++)
		{
			if (ioParam.param(ioType).appSignalID() == appSignalID)
			{
				ioParam.setParam(ioType, theSignalBase.signalParam(appSignalID));
			}
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureThread::stopMeasure(MeasureThreadInfo::ExitCode exitCode)
{
	m_info.setExitCode(exitCode);
	m_info.setCmdStopMeasure(true);
}

// -------------------------------------------------------------------------------------------------------------------
