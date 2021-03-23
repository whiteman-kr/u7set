#include "SignalBase.h"

#include "CalibratorBase.h"
#include "Database.h"
#include "MeasureBase.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

IoSignalParam::IoSignalParam()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

IoSignalParam::IoSignalParam(const IoSignalParam& from)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

bool IoSignalParam::isValid() const
{
	QMutexLocker l(&m_mutex);

	bool valid = true;

	if (m_connectionType == Metrology::ConnectionType::Unused)
	{
		valid = m_param[Metrology::ConnectionIoType::Source].isValid();					// only input
	}
	else
	{
		for(int ioType = 0; ioType < Metrology::ConnectionIoTypeCount; ioType++)		// input and output
		{
			if (m_param[ioType].isValid() == false)
			{
				valid = false;

				break;
			}
		}
	}

	return valid;
}

// -------------------------------------------------------------------------------------------------------------------

void IoSignalParam::clear()
{
	QMutexLocker l(&m_mutex);

	m_connectionType = Metrology::ConnectionType::NoConnectionType;

	for(int ioType = 0; ioType < Metrology::ConnectionIoTypeCount; ioType++)
	{
		m_param[ioType].setAppSignalID(QString());
	}

	m_pCalibratorManager = nullptr;

	m_percent = 0;
	m_comparatorIndex = -1;
	m_comparatorValueType = Metrology::CmpValueType::NoCmpValueType;

	m_negativeRange = false;
	m_tunStateForRestore = 0;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::SignalParam IoSignalParam::param(int ioType) const
{
	QMutexLocker l(&m_mutex);

	if (ioType < 0 || ioType >= Metrology::ConnectionIoTypeCount)
	{
		return Metrology::SignalParam();
	}

	return m_param[ioType];
}

// -------------------------------------------------------------------------------------------------------------------

bool IoSignalParam::setParam(int ioType, const Metrology::SignalParam& param)
{
	if (ioType < 0 || ioType >= Metrology::ConnectionIoTypeCount)
	{
		return false;
	}

	if (param.isValid() == false)
	{
		return false;
	}

	QMutexLocker l(&m_mutex);

	m_param[ioType] = param;

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::appSignalID() const
{
	QMutexLocker l(&m_mutex);

	QString result;

	if (m_connectionType == Metrology::ConnectionType::Unused)
	{
		const Metrology::SignalParam& param = m_param[Metrology::ConnectionIoType::Source];
		if (param.isValid() == true)
		{
			result = param.appSignalID();
		}
	}
	else
	{
		const Metrology::SignalParam& inParam = m_param[Metrology::ConnectionIoType::Source];
		if (inParam.isValid() == true)
		{
			result = inParam.appSignalID() + MULTI_TEXT_DEVIDER;
		}

		const Metrology::SignalParam& outParam = m_param[Metrology::ConnectionIoType::Destination];
		if (outParam.isValid() == true)
		{
			result += outParam.appSignalID();
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::customSignalID() const
{
	QMutexLocker l(&m_mutex);

	QString result;

	if (m_connectionType == Metrology::ConnectionType::Unused)
	{
		const Metrology::SignalParam& param = m_param[Metrology::ConnectionIoType::Source];
		if (param.isValid() == true)
		{
			result = param.customAppSignalID();
		}
	}
	else
	{
		const Metrology::SignalParam& inParam = m_param[Metrology::ConnectionIoType::Source];
		if (inParam.isValid() == true)
		{
			result = inParam.customAppSignalID() + MULTI_TEXT_DEVIDER;
		}

		const Metrology::SignalParam& outParam = m_param[Metrology::ConnectionIoType::Destination];
		if (outParam.isValid() == true)
		{
			result += outParam.customAppSignalID();
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::equipmentID() const
{
	QMutexLocker l(&m_mutex);

	QString result;

	if (m_connectionType == Metrology::ConnectionType::Unused)
	{
		const Metrology::SignalParam& param = m_param[Metrology::ConnectionIoType::Source];
		if (param.isValid() == true)
		{
			result = param.equipmentID();
		}
	}
	else
	{
		const Metrology::SignalParam& inParam = m_param[Metrology::ConnectionIoType::Source];
		if (inParam.isValid() == true)
		{
			result = inParam.equipmentID() + MULTI_TEXT_DEVIDER;
		}

		const Metrology::SignalParam& outParam = m_param[Metrology::ConnectionIoType::Destination];
		if (outParam.isValid() == true)
		{
			if (inParam.equipmentID() != outParam.equipmentID())
			{
				result += outParam.equipmentID();
			}
			else
			{
				result = outParam.equipmentID();
			}
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::rackCaption() const
{
	QMutexLocker l(&m_mutex);

	QString result;

	if (m_connectionType == Metrology::ConnectionType::Unused)
	{
		const Metrology::SignalParam& param = m_param[Metrology::ConnectionIoType::Source];
		if (param.isValid() == true)
		{
			result = param.location().rack().caption();
		}
	}
	else
	{
		const Metrology::SignalParam& inParam = m_param[Metrology::ConnectionIoType::Source];
		if (inParam.isValid() == true)
		{
			result = inParam.location().rack().caption() + MULTI_TEXT_DEVIDER;
		}

		const Metrology::SignalParam& outParam = m_param[Metrology::ConnectionIoType::Destination];
		if (outParam.isValid() == true)
		{
			if (inParam.location().rack().caption() != outParam.location().rack().caption())
			{
				result += outParam.location().rack().caption();
			}
			else
			{
				result = outParam.location().rack().caption();
			}
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::chassisStr() const
{
	QMutexLocker l(&m_mutex);

	QString result;

	if (m_connectionType == Metrology::ConnectionType::Unused)
	{
		const Metrology::SignalParam& param = m_param[Metrology::ConnectionIoType::Source];
		if (param.isValid() == true)
		{
			result = param.location().chassisStr();
		}
	}
	else
	{
		const Metrology::SignalParam& inParam = m_param[Metrology::ConnectionIoType::Source];
		if (inParam.isValid() == true)
		{
			result = inParam.location().chassisStr() + MULTI_TEXT_DEVIDER;
		}

		const Metrology::SignalParam& outParam = m_param[Metrology::ConnectionIoType::Destination];
		if (outParam.isValid() == true)
		{
			if (inParam.location().chassisStr() != outParam.location().chassisStr())
			{
				result += outParam.location().chassisStr();
			}
			else
			{
				result = outParam.location().chassisStr();
			}
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::moduleStr() const
{
	QMutexLocker l(&m_mutex);

	QString result;

	if (m_connectionType == Metrology::ConnectionType::Unused)
	{
		const Metrology::SignalParam& param = m_param[Metrology::ConnectionIoType::Source];
		if (param.isValid() == true)
		{
			result = param.location().moduleStr();
		}
	}
	else
	{
		const Metrology::SignalParam& inParam = m_param[Metrology::ConnectionIoType::Source];
		if (inParam.isValid() == true)
		{
			result = inParam.location().moduleStr() + MULTI_TEXT_DEVIDER;
		}

		const Metrology::SignalParam& outParam = m_param[Metrology::ConnectionIoType::Destination];
		if (outParam.isValid() == true)
		{
			if (inParam.location().moduleStr() != outParam.location().moduleStr())
			{
				result += outParam.location().moduleStr();
			}
			else
			{
				result = outParam.location().moduleStr();
			}
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::placeStr() const
{
	QMutexLocker l(&m_mutex);

	QString result;

	if (m_connectionType == Metrology::ConnectionType::Unused)
	{
		const Metrology::SignalParam& param = m_param[Metrology::ConnectionIoType::Source];
		if (param.isValid() == true)
		{
			result = param.location().placeStr();
		}
	}
	else
	{
		const Metrology::SignalParam& inParam = m_param[Metrology::ConnectionIoType::Source];
		if (inParam.isValid() == true)
		{
			result = inParam.location().placeStr() + MULTI_TEXT_DEVIDER;
		}

		const Metrology::SignalParam& outParam = m_param[Metrology::ConnectionIoType::Destination];
		if (outParam.isValid() == true)
		{
			if (inParam.location().placeStr() != outParam.location().placeStr())
			{
				result += outParam.location().placeStr();
			}
			else
			{
				result = outParam.location().placeStr();
			}
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::caption() const
{
	QMutexLocker l(&m_mutex);

	QString result;

	if (m_connectionType == Metrology::ConnectionType::Unused)
	{
		const Metrology::SignalParam& param = m_param[Metrology::ConnectionIoType::Source];
		if (param.isValid() == true)
		{
			result = param.caption();
		}
	}
	else
	{
		const Metrology::SignalParam& inParam = m_param[Metrology::ConnectionIoType::Source];
		if (inParam.isValid() == true)
		{
			result = inParam.caption() + MULTI_TEXT_DEVIDER;
		}

		const Metrology::SignalParam& outParam = m_param[Metrology::ConnectionIoType::Destination];
		if (outParam.isValid() == true)
		{
			if (inParam.caption() != outParam.caption())
			{
				result += outParam.caption();
			}
			else
			{
				result = outParam.caption();
			}
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::electricRangeStr() const
{
	QMutexLocker l(&m_mutex);

	QString result;

	switch(m_connectionType)
	{
		case Metrology::ConnectionType::Unused:
		case Metrology::ConnectionType::Input_Internal:
		case Metrology::ConnectionType::Input_DP_Internal_F:
		case Metrology::ConnectionType::Input_C_Internal_F:
			{
				const Metrology::SignalParam& param = m_param[Metrology::ConnectionIoType::Source];
				if (param.isValid() == true)
				{
					result = param.electricRangeStr();
				}
			}

			break;

		case Metrology::ConnectionType::Input_Output:
		case Metrology::ConnectionType::Input_DP_Output_F:
		case Metrology::ConnectionType::Input_C_Output_F:
			{
				const Metrology::SignalParam& inParam = m_param[Metrology::ConnectionIoType::Source];
				if (inParam.isValid() == true)
				{
					result = inParam.electricRangeStr() + MULTI_TEXT_DEVIDER;
				}

				const Metrology::SignalParam& outParam = m_param[Metrology::ConnectionIoType::Destination];
				if (outParam.isValid() == true)
				{
					if (inParam.electricRangeStr() != outParam.electricRangeStr())
					{
						result += outParam.electricRangeStr();
					}
					else
					{
						result = outParam.electricRangeStr();
					}
				}
			}

			break;

		case Metrology::ConnectionType::Tuning_Output:
			{
				const Metrology::SignalParam& param = m_param[Metrology::ConnectionIoType::Destination];
				if (param.isValid() == true)
				{
					result = param.electricRangeStr();
				}
			}

			break;

		default:
			assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::electricSensorStr() const
{
	QMutexLocker l(&m_mutex);

	QString result;

	switch(m_connectionType)
	{
		case Metrology::ConnectionType::Unused:
		case Metrology::ConnectionType::Input_Internal:
		case Metrology::ConnectionType::Input_DP_Internal_F:
		case Metrology::ConnectionType::Input_C_Internal_F:
			{
				const Metrology::SignalParam& param = m_param[Metrology::ConnectionIoType::Source];
				if (param.isValid() == true)
				{
					result = param.electricSensorTypeStr();
				}
			}

			break;

		case Metrology::ConnectionType::Input_Output:
		case Metrology::ConnectionType::Input_DP_Output_F:
		case Metrology::ConnectionType::Input_C_Output_F:
			{
				const Metrology::SignalParam& inParam = m_param[Metrology::ConnectionIoType::Source];
				if (inParam.isValid() == true)
				{
					result = inParam.electricSensorTypeStr() + MULTI_TEXT_DEVIDER;
				}

				const Metrology::SignalParam& outParam = m_param[Metrology::ConnectionIoType::Destination];
				if (outParam.isValid() == true)
				{
					if (inParam.electricSensorTypeStr() != outParam.electricSensorTypeStr())
					{
						result += outParam.electricSensorTypeStr();
					}
					else
					{
						result = outParam.electricSensorTypeStr();
					}
				}
			}

			break;

		case Metrology::ConnectionType::Tuning_Output:
			{
				const Metrology::SignalParam& param = m_param[Metrology::ConnectionIoType::Destination];
				if (param.isValid() == true)
				{
					result = param.electricSensorTypeStr();
				}
			}

			break;

		default: assert(0);
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::physicalRangeStr() const
{
	QMutexLocker l(&m_mutex);

	QString result;

	if (m_connectionType == Metrology::ConnectionType::Unused)
	{
		const Metrology::SignalParam& param = m_param[Metrology::ConnectionIoType::Source];
		if (param.isValid() == true)
		{
			result = param.physicalRangeStr();
		}
	}
	else
	{
		const Metrology::SignalParam& inParam = m_param[Metrology::ConnectionIoType::Source];
		if (inParam.isValid() == true)
		{
			result = inParam.physicalRangeStr() + MULTI_TEXT_DEVIDER;
		}

		const Metrology::SignalParam& outParam = m_param[Metrology::ConnectionIoType::Destination];
		if (outParam.isValid() == true)
		{
			if (inParam.physicalRangeStr() != outParam.physicalRangeStr())
			{
				result += outParam.physicalRangeStr();
			}
			else
			{
				result = outParam.physicalRangeStr();
			}
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::engineeringRangeStr() const
{
	QMutexLocker l(&m_mutex);

	QString result;

	switch(m_connectionType)
	{
		case Metrology::ConnectionType::Unused:
			{
				const Metrology::SignalParam& param = m_param[Metrology::ConnectionIoType::Source];
				if (param.isValid() == true)
				{
					result = param.engineeringRangeStr();
				}
			}
			break;

		case Metrology::ConnectionType::Input_Internal:
		case Metrology::ConnectionType::Input_Output:
		case Metrology::ConnectionType::Input_DP_Internal_F:
		case Metrology::ConnectionType::Input_DP_Output_F:
		case Metrology::ConnectionType::Input_C_Internal_F:
		case Metrology::ConnectionType::Input_C_Output_F:
			{
				const Metrology::SignalParam& inParam = m_param[Metrology::ConnectionIoType::Source];
				if (inParam.isValid() == true)
				{
					result = inParam.engineeringRangeStr() + MULTI_TEXT_DEVIDER;
				}

				const Metrology::SignalParam& outParam = m_param[Metrology::ConnectionIoType::Destination];
				if (outParam.isValid() == true)
				{
					if (inParam.engineeringRangeStr() != outParam.engineeringRangeStr())
					{
						result += outParam.engineeringRangeStr();
					}
					else
					{
						result = outParam.engineeringRangeStr();
					}
				}
			}
			break;

		case Metrology::ConnectionType::Tuning_Output:
		{
			const Metrology::SignalParam& inParam = m_param[Metrology::ConnectionIoType::Source];
			if (inParam.isValid() == true)
			{
				result = inParam.tuningRangeStr() + MULTI_TEXT_DEVIDER;
			}

			const Metrology::SignalParam& outParam = m_param[Metrology::ConnectionIoType::Destination];

			if (outParam.isValid() == true)
			{
				if (compareDouble(inParam.tuningLowBound().toDouble(), outParam.lowEngineeringUnits()) == false ||
					compareDouble(inParam.tuningHighBound().toDouble(), outParam.highEngineeringUnits()) == false)
				{
					result += outParam.engineeringRangeStr();
				}
				else
				{
					result = outParam.engineeringRangeStr();
				}
			}
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::calibratorStr() const
{
	if (m_pCalibratorManager == nullptr || m_pCalibratorManager->calibratorIsConnected() == false)
	{
		return qApp->translate("CalibratorManager.h", CalibratorNotConnected);
	}

	return QString("%1 %2").
			arg(qApp->translate("CalibratorManager.h", CalibratorStr)).
			arg(m_pCalibratorManager->calibratorChannel() + 1);
}

// -------------------------------------------------------------------------------------------------------------------

IoSignalParam& IoSignalParam::operator=(const IoSignalParam& from)
{
	QMutexLocker l(&m_mutex);

	m_connectionType = from.m_connectionType;

	for(int ioType = 0; ioType < Metrology::ConnectionIoTypeCount; ioType++)
	{
		m_param[ioType] = from.m_param[ioType];
	}

	m_pCalibratorManager = from.m_pCalibratorManager;

	m_percent = from.m_percent;
	m_comparatorIndex = from.m_comparatorIndex;
	m_comparatorValueType = from.m_comparatorValueType;

	m_negativeRange  = from.m_negativeRange;
	m_tunStateForRestore = from.m_tunStateForRestore;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MultiChannelSignal::MultiChannelSignal()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

MultiChannelSignal::MultiChannelSignal(const MultiChannelSignal& from)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

void MultiChannelSignal::clear()
{
	QMutexLocker l(&m_mutex);

	int channelCount = m_signalList.count();
	for(int ch = 0; ch < channelCount; ch++)
	{
		m_signalList[ch] = nullptr;
	}

	m_location.clear();
	m_signalID.clear();
}

// -------------------------------------------------------------------------------------------------------------------

bool MultiChannelSignal::isEmpty() const
{
	QMutexLocker l(&m_mutex);

	bool empty = true;

	int channelCount = m_signalList.count();
	for(int ch = 0; ch < channelCount; ch++)
	{
		if (m_signalList[ch] != nullptr)
		{
			empty = false;

			break;
		}
	}

	return empty;
}

// -------------------------------------------------------------------------------------------------------------------


void MultiChannelSignal::setSignalCount(int count)
{
	QMutexLocker l(&m_mutex);

	m_signalList.fill(nullptr, count);
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal* MultiChannelSignal::metrologySignal(int channel) const
{
	QMutexLocker l(&m_mutex);

	if (channel < 0 || channel >= m_signalList.count())
	{
		assert(0);
		return nullptr;
	}

	return m_signalList[channel];
}

// -------------------------------------------------------------------------------------------------------------------

bool MultiChannelSignal::setMetrologySignal(int measureKind, int channel, Metrology::Signal* pSignal)
{
	QMutexLocker l(&m_mutex);

	if (ERR_MEASURE_KIND(measureKind) == true)
	{
		assert(0);
		return false;
	}

	if (channel < 0 || channel >= m_signalList.count())
	{
		assert(0);
		return false;
	}

	if (pSignal == nullptr)
	{
		assert(0);
		return false;
	}

	Metrology::SignalParam& param = pSignal->param();
	if (param.isValid() == false)
	{
		assert(false);
		return false;
	}

	m_signalList[channel] = pSignal;

	m_location.setRack(param.location().rack());
	m_location.setChassis(param.location().chassis());
	m_location.setModule(param.location().module());
	m_location.setPlace(param.location().place());
	m_location.setContact(param.location().contact());

	m_caption.clear();

	switch(measureKind)
	{
		case Measure::Kind::OneRack:
			m_signalID = param.appSignalID();
			m_caption = param.caption();
			break;

		case Measure::Kind::OneModule:
			m_signalID = param.location().moduleID();
			break;

		case Measure::Kind::MultiRack:

			switch (param.inOutType())
			{
				case E::SignalInOutType::Input:
				case E::SignalInOutType::Output:

					m_signalID = QString::asprintf("CH %02d _ MD %02d _ IN %02d",
												   m_location.chassis(),
												   m_location.module(),
												   m_location.place());

					break;

				case E::SignalInOutType::Internal:

					if (param.enableTuning() == true)
					{
						m_signalID = QString::asprintf("CH %02d _ MD %02d _ IN %02d",
													   m_location.chassis(),
													   m_location.module(),
													   m_location.place());
					}
					else
					{
						m_signalID = QString::asprintf("CH %02d _ MD %02d",
													   m_location.chassis(),
													   m_location.module());
					}

					break;

				default:
					assert(0);
					break;
			}
			break;

		default:
			assert(false);
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal* MultiChannelSignal::firstMetrologySignal() const
{
	QMutexLocker l(&m_mutex);

	Metrology::Signal* pSignal = nullptr;

	int channelCount = m_signalList.count();
	for(int ch = 0; ch < channelCount; ch++ )
	{
		if (m_signalList[ch] != nullptr)
		{
			pSignal = m_signalList[ch];
			break;
		}
	}

	return pSignal;
}

// -------------------------------------------------------------------------------------------------------------------

MultiChannelSignal& MultiChannelSignal::operator=(const MultiChannelSignal& from)
{
	QMutexLocker l(&m_mutex);

	m_signalList = from.m_signalList;
	m_location = from.m_location;
	m_signalID = from.m_signalID;
	m_caption = from.m_caption;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureSignal::MeasureSignal()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal::MeasureSignal(const MeasureSignal& from)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureSignal::clear()
{
	QMutexLocker l(&m_mutex);

	m_connectionType = Metrology::ConnectionType::NoConnectionType;

	for(int ioType = 0; ioType < Metrology::ConnectionIoTypeCount; ioType++)
	{
		m_signal[ioType].clear();
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureSignal::isEmpty() const
{
	QMutexLocker l(&m_mutex);

	bool empty = true;

	for(int ioType = 0; ioType < Metrology::ConnectionIoTypeCount; ioType++)
	{
		if (m_signal[ioType].isEmpty() == false)
		{
			empty = false;

			break;
		}
	}

	return empty;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureSignal::setChannelCount(int count)
{
	QMutexLocker l(&m_mutex);

	m_channelCount = count;

	for(int ioType = 0; ioType < Metrology::ConnectionIoTypeCount; ioType++)
	{
		m_signal[ioType].setSignalCount(count);
	}
}

// -------------------------------------------------------------------------------------------------------------------

MultiChannelSignal MeasureSignal::multiChannelSignal(int ioType) const
{
	if (ioType < 0 || ioType >= Metrology::ConnectionIoTypeCount)
	{
		return MultiChannelSignal();
	}

	QMutexLocker l(&m_mutex);

	return m_signal[ioType];
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureSignal::setMultiSignal(int ioType, const MultiChannelSignal& signal)
{
	if (ioType < 0 || ioType >= Metrology::ConnectionIoTypeCount)
	{
		return false;
	}

	QMutexLocker l(&m_mutex);

	m_signal[ioType] = signal;

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal* MeasureSignal::metrologySignal(int ioType, int channel) const
{
	if (ioType < 0 || ioType >= Metrology::ConnectionIoTypeCount)
	{
		return nullptr;
	}

	if (channel < 0 || channel >= m_channelCount)
	{
		return nullptr;
	}

	QMutexLocker l(&m_mutex);

	return m_signal[ioType].metrologySignal(channel);
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureSignal::setMetrologySignal(int measureKind,
									   const Metrology::ConnectionBase& connectionBase,
									   Metrology::ConnectionType connectionType,
									   int channel,
									   Metrology::Signal* pSignal)
{
	if (ERR_MEASURE_KIND(measureKind) == true)
	{
		assert(0);
		return false;
	}

	if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
	{
		assert(0);
		return false;
	}

	if (channel < 0 || channel >= m_channelCount)
	{
		assert(0);
		return false;
	}

	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		assert(0);
		return false;
	}

	// set input signal
	//

	bool result = true;

	m_mutex.lock();

		m_connectionType = connectionType;

		result = m_signal[Metrology::ConnectionIoType::Source].setMetrologySignal(measureKind, channel, pSignal);

	m_mutex.unlock();

	if (result == false)
	{
		clear();
		return false;
	}

	// set output signal
	//
	if (connectionType != Metrology::ConnectionType::Unused)
	{
		Metrology::Signal* pDestSignal = nullptr;

		switch (measureKind)
		{
			case Measure::Kind::OneRack:
				{
					// take destination signals by source signal
					//
					std::vector<Metrology::Signal*> destSignals = connectionBase.destinationSignals(pSignal->param().appSignalID(), connectionType);
					if (channel < 0 || channel >= TO_INT(destSignals.size()))
					{
						break;
					}

					pDestSignal = destSignals[static_cast<quint64>(channel)];
				}
				break;

			case Measure::Kind::OneModule:
			case Measure::Kind::MultiRack:
				{
					// find index of metrology connection in the base by source signal
					//
				    int connectionIndex = connectionBase.findConnectionIndex(Metrology::ConnectionIoType::Source, connectionType, pSignal);
					if (connectionIndex == -1)
					{
						break;
					}

					// take metrology connection in the base by index
					//
					const Metrology::Connection& connection = connectionBase.connection(connectionIndex);
					if (connection.isValid() == false)
					{
						break;
					}

					pDestSignal = connection.metrologySignal(Metrology::ConnectionIoType::Destination);
				}
				break;

			default:
				{
					assert(0);
					break;
				}
		}

		if (pDestSignal == nullptr || pDestSignal->param().isValid() == false)
		{
			result = false;
		}
		else
		{
			m_mutex.lock();

				result &= m_signal[Metrology::ConnectionIoType::Destination].setMetrologySignal(measureKind, channel, pDestSignal);

			m_mutex.unlock();
		}
	}

	if (result == false)
	{
		clear();
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureSignal::contains(Metrology::Signal* pSignal) const
{
	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		return false;
	}

	bool result = false;

	for(int ch = 0; ch < m_channelCount; ch++)
	{
		for(int ioType = 0; ioType < Metrology::ConnectionIoTypeCount; ioType++)
		{
			if (m_signal[ioType].metrologySignal(ch) == pSignal)
			{
				result = true;

				break;
			}
		}

		if (result == true)
		{
			break;
		}
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal& MeasureSignal::operator=(const MeasureSignal& from)
{
	QMutexLocker l(&m_mutex);

	m_connectionType = from.m_connectionType;

	m_channelCount = from.m_channelCount;

	for(int ioType = 0; ioType < Metrology::ConnectionIoTypeCount; ioType++)
	{
		m_signal[ioType] = from.m_signal[ioType];
	}

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalBase theSignalBase;

// -------------------------------------------------------------------------------------------------------------------

SignalBase::SignalBase(QObject* parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::clear()
{
	clearActiveSignal();

	clearSignalListForMeasure();

	clearRackListForMeasure();

	clearSignalList();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::signalCount() const
{
	QMutexLocker l(&m_signalMutex);

	return TO_INT(m_signalList.size());
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::clearSignalList()
{
	QMutexLocker l(&m_signalMutex);

	m_rackBase.clear();
	m_tuningBase.clear();
	m_connectionBase.clear();
	m_statisticsBase.clear();

	m_signalHashList.clear();
	m_signalList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::appendSignal(const Metrology::SignalParam& param)
{
	if (param.appSignalID().isEmpty() == true || param.hash() == UNDEFINED_HASH)
	{
		assert(false);
		return -1;
	}

	QMutexLocker l(&m_signalMutex);

	if (m_signalHashList.contains(param.hash()) == true)
	{
		return -1;
	}

	int index = -1;

	Metrology::Signal metrologySignal(param);

	m_signalList.push_back(metrologySignal);
	index = TO_INT(m_signalList.size() - 1);

	m_signalHashList.insert(param.hash(), index);

	return index;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal* SignalBase::signalPtr(const QString& appSignalID)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(false);
		return nullptr;
	}

	return signalPtr(calcHash(appSignalID));
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal* SignalBase::signalPtr(const Hash& hash)
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash);
		return nullptr;
	}

	QMutexLocker l(&m_signalMutex);

	if (m_signalHashList.contains(hash) == false)
	{
		return nullptr;
	}

	int index = m_signalHashList[hash];
	if (index < 0 || index >= TO_INT(m_signalList.size()))
	{
		return nullptr;
	}

	return &m_signalList[static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal* SignalBase::signalPtr(int index)
{
	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= TO_INT(m_signalList.size()))
	{
		return nullptr;
	}

	return &m_signalList[static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal SignalBase::signal(const QString& appSignalID) const
{
	if (appSignalID.isEmpty() == true)
	{
		assert(false);
		return Metrology::Signal();
	}

	return signal(calcHash(appSignalID));
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal SignalBase::signal(const Hash& hash) const
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != 0);
		return Metrology::Signal();
	}

	QMutexLocker l(&m_signalMutex);

	if (m_signalHashList.contains(hash) == false)
	{
		return Metrology::Signal();
	}

	int index = m_signalHashList[hash];

	if (index < 0 || index >= TO_INT(m_signalList.size()))
	{
		return Metrology::Signal();
	}

	return m_signalList[static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal SignalBase::signal(int index) const
{
	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= TO_INT(m_signalList.size()))
	{
		return Metrology::Signal();
	}

	return m_signalList[static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::SignalParam SignalBase::signalParam(const QString& appSignalID) const
{
	if (appSignalID.isEmpty() == true)
	{
		assert(false);
		return Metrology::SignalParam();
	}

	return signalParam(calcHash(appSignalID));
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::SignalParam SignalBase::signalParam(const Hash& hash) const
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != UNDEFINED_HASH);
		return Metrology::SignalParam();
	}

	QMutexLocker l(&m_signalMutex);

	if (m_signalHashList.contains(hash) == false)
	{
		return Metrology::SignalParam();
	}

	int index = m_signalHashList[hash];

	if (index < 0 || index >= TO_INT(m_signalList.size()))
	{
		return Metrology::SignalParam();
	}

	return m_signalList[static_cast<quint64>(index)].param();
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::SignalParam SignalBase::signalParam(int index) const
{
	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= TO_INT(m_signalList.size()))
	{
		return Metrology::SignalParam();
	}

	return m_signalList[static_cast<quint64>(index)].param();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignalParam(const Hash& hash, const Metrology::SignalParam& param)
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != UNDEFINED_HASH);
		return;
	}

	QMutexLocker l(&m_signalMutex);

	if (m_signalHashList.contains(hash) == false)
	{
		return;
	}

	int index = m_signalHashList[hash];

	if (index < 0 || index >= TO_INT(m_signalList.size()))
	{
		return;
	}

	m_signalList[static_cast<quint64>(index)].setParam(param);

	emit signalParamChanged(param.appSignalID());
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignalParam(int index, const Metrology::SignalParam& param)
{
	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= TO_INT(m_signalList.size()))
	{
		return;
	}

	m_signalList[static_cast<quint64>(index)].setParam(param);

	emit signalParamChanged(param.appSignalID());
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::SignalState SignalBase::signalState(const QString& appSignalID) const
{
	if (appSignalID.isEmpty() == true)
	{
		assert(false);
		return Metrology::SignalState();
	}

	return signalState(calcHash(appSignalID));
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::SignalState SignalBase::signalState(const Hash& hash) const
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != UNDEFINED_HASH);
		return Metrology::SignalState();
	}

	QMutexLocker l(&m_signalMutex);

	if (m_signalHashList.contains(hash) == false)
	{
		return Metrology::SignalState();
	}

	int index = m_signalHashList[hash];

	if (index < 0 || index >= TO_INT(m_signalList.size()))
	{
		return Metrology::SignalState();
	}

	return m_signalList[static_cast<quint64>(index)].state();
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::SignalState SignalBase::signalState(int index) const
{
	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= TO_INT(m_signalList.size()))
	{
		return Metrology::SignalState();
	}

	return m_signalList[static_cast<quint64>(index)].state();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignalState(const QString& appSignalID, const Metrology::SignalState& state)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(false);
		return;
	}

	setSignalState(calcHash(appSignalID), state);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignalState(const Hash& hash, const Metrology::SignalState &state)
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != UNDEFINED_HASH);
		return;
	}

	QMutexLocker l(&m_signalMutex);

	if (m_signalHashList.contains(hash) == false)
	{
		return;
	}

	int index = m_signalHashList[hash];

	if (index < 0 || index >= TO_INT(m_signalList.size()))
	{
		return;
	}

	m_signalList[static_cast<quint64>(index)].setState(state);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignalState(int index, const Metrology::SignalState& state)
{
	QMutexLocker l(&m_signalMutex);

	if (index < 0 || index >= TO_INT(m_signalList.size()))
	{
		return;
	}

	m_signalList[static_cast<quint64>(index)].setState(state);
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::enableForMeasure(Metrology::ConnectionType connectionType, Metrology::Signal* pSignal)
{
	if (pSignal == nullptr)
	{
		return false;
	}

	const Metrology::SignalParam& param = pSignal->param();
	if (param.isValid() == false)
	{
		return false;
	}

	if (param.isAnalog() == false)
	{
		return false;
	}

	if (param.location().shownOnSchemas() == false)
	{
		return false;
	}

	switch (connectionType)
	{
		case Metrology::ConnectionType::Unused:
			{
				if (param.isInput() == false)
				{
					return false;
				}

				if (param.location().chassis() == -1 || param.location().module() == -1 || param.location().place() == -1)
				{
					return false;
				}

				if (param.electricRangeIsValid() == false)
				{
					return false;
				}
			}
			break;

		case Metrology::ConnectionType::Input_Internal:
		case Metrology::ConnectionType::Input_DP_Internal_F:
		case Metrology::ConnectionType::Input_C_Internal_F:
			{
				if (param.isInput() == false)
				{
					return false;
				}

				if (param.electricRangeIsValid() == false)
				{
					return false;
				}

				if (param.location().chassis() == -1 || param.location().module() == -1 || param.location().place() == -1)
				{
					return false;
				}

				int connectionIndex = m_connectionBase.findConnectionIndex(Metrology::ConnectionIoType::Source, connectionType, pSignal);
				if (connectionIndex == -1)
				{
					return false;
				}

				const Metrology::Connection& connection = m_connectionBase.connection(connectionIndex);
				if (connection.isValid() == false)
				{
					return false;
				}

				Metrology::Signal* pDestSignal = connection.metrologySignal(Metrology::ConnectionIoType::Destination);
				if (pDestSignal == nullptr || pDestSignal->param().isValid() == false)
				{
					return false;
				}

				if (pDestSignal->param().isInternal() == false)
				{
					return false;
				}
			}
			break;

		case Metrology::ConnectionType::Input_Output:
		case Metrology::ConnectionType::Input_DP_Output_F:
		case Metrology::ConnectionType::Input_C_Output_F:
			{
				if (param.isInput() == false)
				{
					return false;
				}

				if (param.location().chassis() == -1 || param.location().module() == -1 || param.location().place() == -1)
				{
					return false;
				}

				if (param.electricRangeIsValid() == false)
				{
					return false;
				}

				int connectionIndex = m_connectionBase.findConnectionIndex(Metrology::ConnectionIoType::Source, connectionType, pSignal);
				if (connectionIndex == -1)
				{
					return false;
				}

				const Metrology::Connection& connection = m_connectionBase.connection(connectionIndex);
				if (connection.isValid() == false)
				{
					return false;
				}

				Metrology::Signal* pDestSignal = connection.metrologySignal(Metrology::ConnectionIoType::Destination);
				if (pDestSignal == nullptr || pDestSignal->param().isValid() == false)
				{
					return false;
				}

				if (pDestSignal->param().isOutput() == false)
				{
					return false;
				}

				if (pDestSignal->param().electricRangeIsValid() == false)
				{
					return false;
				}
			}
			break;

		case Metrology::ConnectionType::Tuning_Output:
			{
				if (param.isInternal() == false)
				{
					return false;
				}

				if (param.location().chassis() == -1 || param.location().module() == -1)
				{
					return false;
				}

				if (param.enableTuning() == false)
				{
					return false;
				}

				int connectionIndex = m_connectionBase.findConnectionIndex(Metrology::ConnectionIoType::Source, connectionType, pSignal);
				if (connectionIndex == -1)
				{
					return false;
				}

				const Metrology::Connection& connection = m_connectionBase.connection(connectionIndex);
				if (connection.isValid() == false)
				{
					return false;
				}

				Metrology::Signal* pDestSignal = connection.metrologySignal(Metrology::ConnectionIoType::Destination);
				if (pDestSignal == nullptr || pDestSignal->param().isValid() == false)
				{
					return false;
				}

				if (pDestSignal->param().isOutput() == false)
				{
					return false;
				}

				if (pDestSignal->param().electricRangeIsValid() == false)
				{
					return false;
				}
			}
			break;

		default:
			assert(0);
			return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::hashForRequestStateCount() const
{
	QMutexLocker l(&m_stateMutex);

	return TO_INT(m_requestStateList.size());
}

// -------------------------------------------------------------------------------------------------------------------

Hash SignalBase::hashForRequestState(int index)
{
	QMutexLocker l(&m_stateMutex);

	if (index < 0 || index >= TO_INT(m_requestStateList.size()))
	{
		return UNDEFINED_HASH;
	}

	return m_requestStateList[static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::rackForMeasureCount() const
{
	QMutexLocker l(&m_rackMutex);

	return TO_INT(m_rackList.size());
}


// -------------------------------------------------------------------------------------------------------------------

Metrology::RackParam SignalBase::rackForMeasure(int index) const
{
	QMutexLocker l(&m_rackMutex);

	if (index < 0 || index >= TO_INT(m_rackList.size()))
	{
		return Metrology::RackParam();
	}

	return m_rackList[static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalBase::findAppSignalIDforSerialNo(const QString& moduleID)
{
	QString appSignalID;

	quint64 signalCount = m_signalList.size();
	for(quint64 i = 0; i < signalCount; i++)
	{
		Metrology::Signal& signal = m_signalList[i];

		if (signal.param().isInput() == false)
		{
			continue;
		}

		if (signal.param().isAnalog() == false)
		{
			continue;
		}

		if (signal.param().electricUnitID() != E::ElectricUnit::NoUnit)
		{
			continue;
		}

		if (signal.param().electricSensorType() != E::SensorType::NoSensor)
		{
			continue;
		}

		if (signal.param().location().moduleID() != moduleID)
		{
			continue;
		}

		if (signal.param().location().contact() != theOptions.module().suffixSN())
		{
			continue;
		}

		appSignalID = signal.param().appSignalID();

		break;
	}

	return appSignalID;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::createRackListForMeasure(int measureKind, Metrology::ConnectionType connectionType)
{
	if (ERR_MEASURE_KIND(measureKind) == true)
	{
		assert(false);
		return 0;
	}

	if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
	{
		assert(false);
		return 0;
	}

	// find all type of racks for selected connectionType and create rackTypeList for ToolBar
	//
	QMutexLocker l(&m_rackMutex);

	m_rackList.clear();

	switch (measureKind)
	{
		case Measure::Kind::OneRack:
		case Measure::Kind::OneModule:
			{
				// select racks that has signals, other racks ignore
				//
				QMap<Hash, int> rackHashMap;

				quint64 signalCount = m_signalList.size();
				for(quint64 i = 0; i < signalCount; i ++)
				{
					if (enableForMeasure(connectionType, &m_signalList[i]) == false)
					{
						continue;
					}

					const Metrology::SignalParam& param = m_signalList[i].param();
					if (param.isValid() == false)
					{
						continue;
					}

					Hash rackHash = param.location().rack().hash();
					if (rackHash == UNDEFINED_HASH)
					{
						continue;
					}

					if (rackHashMap.contains(rackHash) == false)
					{
						rackHashMap.insert(rackHash, param.location().rack().index());

						m_rackList.push_back(param.location().rack());
					}
				}

				// sort by index
				//
				std::sort(m_rackList.begin(), m_rackList.end(),
						[](const Metrology::RackParam& rack1, const Metrology::RackParam& rack2) -> bool
						{
							return rack1.index() < rack2.index();
						});
			}
			break;

		case Measure::Kind::MultiRack:
			{
				// select racks from tables
				//
				int rackGroupCount = racks().groups().count();

				for(int g = 0; g < rackGroupCount; g++)
				{
					RackGroup group = racks().groups().group(g);
					if (group.isValid() == false)
					{
						continue;
					}

					QString caption = group.caption();
					if (caption.isEmpty() == true)
					{
						continue;
					}

					Metrology::RackParam rack(g, QString("GROUP_%1").arg(g), group.caption());

					m_rackList.push_back(rack);
				}
			}
			break;

		default:
			assert(0);
	}


	return TO_INT(m_rackList.size());
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::clearRackListForMeasure()
{
	QMutexLocker l(&m_rackMutex);

	m_rackList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::initSignals()
{
	QElapsedTimer responseTime;
	responseTime.start();

	int analogTuningSignalCount = 0;
	int discreteTuningSignalCount = 0;
	int busTuningSignalCount = 0;

	m_signalMutex.lock();

		quint64 count = m_signalList.size();
		for(quint64 i = 0; i < count; i ++)
		{
			Metrology::SignalParam& param = m_signalList[i].param();
			if (param.isValid() == false)
			{
				continue;
			}

			// places for tuning signals
			//
			if (param.enableTuning() == true)
			{
				switch (param.signalType())
				{
					case E::SignalType::Analog:		param.setPlace(analogTuningSignalCount++);		break;
					case E::SignalType::Discrete:	param.setPlace(discreteTuningSignalCount++);	break;
					case E::SignalType::Bus:		param.setPlace(busTuningSignalCount++);			break;
					default:						assert(0);
				}
			}
		}

	m_signalMutex.unlock();

	initRackParam();
	initConnectionSignals();
	m_tuningBase.signalBase().createSignalList();
	m_statisticsBase.createSignalList();

	qDebug() << __FUNCTION__ << "Signals have been initialized" << " Time for init: " << responseTime.elapsed() << " ms";
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::initRackParam()
{
	QMutexLocker l(&m_signalMutex);

	quint64 count = m_signalList.size();
	for(quint64 i = 0; i < count; i ++)
	{
		Metrology::SignalParam& param = m_signalList[i].param();
		if (param.isValid() == false)
		{
			continue;
		}

		if (param.location().rack().hash() == UNDEFINED_HASH)
		{
			continue;
		}

		// fistly we are now only rackID, so we must set RackParam from RackBase for every signal
		//
		Metrology::RackParam rack = m_rackBase.rack(param.location().rack().hash());
		if (rack.isValid() == true)
		{
			param.setRack(rack);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::initConnectionSignals()
{
	// init signals
	//
	int connectionCount = m_connectionBase.count();
	for(int i = 0; i < connectionCount; i++)
	{
		Metrology::Connection* pConnection = m_connectionBase.connectionPtr(i);
		if (pConnection == nullptr)
		{
			continue;
		}

		for(int ioType = 0; ioType < Metrology::ConnectionIoTypeCount; ioType++)
		{
			if (pConnection->appSignalID(ioType).isEmpty() == true)
			{
				continue;
			}

			Metrology::Signal* pSignal = signalPtr(pConnection->appSignalID(ioType));
			if (pSignal == nullptr || pSignal->param().isValid() == false)
			{
				continue;
			}

			pConnection->setSignal(ioType, pSignal);
		}
	}

	// sort
	//
	m_connectionBase.sort();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::signalForMeasureCount() const
{
	QMutexLocker l(&m_signalMesaureMutex);

	return TO_INT(m_signalMeasureList.size());
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal SignalBase::signalForMeasure(int index) const
{
	QMutexLocker l(&m_signalMesaureMutex);

	if (index < 0 || index >= TO_INT(m_signalMeasureList.size()))
	{
		return MeasureSignal();
	}

	return m_signalMeasureList[static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::setSignalForMeasure(int index, const MeasureSignal& signal)
{
	QMutexLocker l(&m_signalMesaureMutex);

	if (index < 0 || index >= TO_INT(m_signalMeasureList.size()))
	{
		return false;
	}

	m_signalMeasureList[static_cast<quint64>(index)] = signal;

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::createSignalListForMeasure(int measureKind, Metrology::ConnectionType connectionType, int rackIndex)
{
	if (ERR_MEASURE_KIND(measureKind) == true)
	{
		assert(false);
		return 0;
	}

	if (ERR_METROLOGY_CONNECTION_TYPE(connectionType) == true)
	{
		assert(false);
		return 0;
	}

	if (rackIndex == -1)
	{
		assert(false);
		return 0;
	}

	quint64 signalIndex = 0;
	MeasureSignal measureSignal;
	QMap<Hash, quint64>	mesaureSignalMap;

	// find all signals for selected rack or group and create Measure Signal List map for ToolBar
	//
	QMutexLocker l(&m_signalMesaureMutex);

	m_signalMeasureList.clear();

	quint64 signalCount = m_signalList.size();
	for(quint64 i = 0; i < signalCount; i ++)
	{
		if (enableForMeasure(connectionType, &m_signalList[i]) == false)
		{
			continue;
		}

		Metrology::SignalParam& param = m_signalList[i].param();
		if (param.isValid() == false)
		{
			continue;
		}

		// determine the number of channels for a multi-channel signal
		//
		int channelCount = 0;

		switch(measureKind)
		{
			case Measure::Kind::OneRack:

				if (connectionType == Metrology::ConnectionType::Unused)
				{
					channelCount = 1;
				}
				else
				{
					channelCount = TO_INT(m_connectionBase.destinationSignals(param.appSignalID(), connectionType).size());
				}
				break;

			case Measure::Kind::OneModule:

				channelCount = theOptions.module().maxInputCount();
				break;

			case Measure::Kind::MultiRack:

				channelCount = Metrology::ChannelCount;
				break;

			default:
				assert(0);
				continue;
		}

		measureSignal.clear();
		measureSignal.setChannelCount(channelCount);

		// switch for Measure kind
		//
		switch(measureKind)
		{
			case Measure::Kind::OneRack:
				{
					if (param.location().rack().index() != rackIndex)
					{
						continue;
					}

					for(int channel = 0; channel < measureSignal.channelCount(); channel++)
					{
						if (measureSignal.setMetrologySignal(measureKind,
															 m_connectionBase,
															 connectionType,
															 channel,
															 &m_signalList[i]) == false)
						{
							continue;
						}
					}
				}
				break;

			case Measure::Kind::OneModule:
				{
					if (param.location().rack().index() != rackIndex)
					{
						continue;
					}

					QString id;
					id = QString::asprintf("%d - %d - %d",
										   param.location().rack().index(),
										   param.location().chassis(),
										   param.location().module());

					Hash hashid = calcHash(id);

					if (mesaureSignalMap.contains(hashid) == true)
					{
						quint64 index = mesaureSignalMap[hashid];
						if (index < m_signalMeasureList.size())
						{
							int channel = param.location().place() - 1;
							if (channel >= 0 && channel < measureSignal.channelCount())
							{
								if (m_signalMeasureList[index].metrologySignal(connectionType, channel) != nullptr)
								{
									continue;
								}

								if (m_signalMeasureList[index].setMetrologySignal(measureKind,
																				  m_connectionBase,
																				  connectionType,
																				  channel,
																				  &m_signalList[i]) == false)
								{
									continue;
								}
							}
						}

						continue;
					}

					mesaureSignalMap.insert(hashid, signalIndex);

					int channel = param.location().place() - 1;
					if (channel >= 0 && channel < measureSignal.channelCount())
					{
						if (measureSignal.setMetrologySignal(measureKind,
															 m_connectionBase,
															 connectionType,
															 channel,
															 &m_signalList[i]) == false)
						{
							mesaureSignalMap.remove(hashid);
							continue;
						}
					}
				}
				break;

			case Measure::Kind::MultiRack:
				{
					if (param.location().rack().groupIndex() != rackIndex)
					{
						continue;
					}

					QString id;
					id = QString::asprintf("%d - %d - %d - %d - ",
										   param.location().rack().groupIndex(),
										   param.location().chassis(),
										   param.location().module(),
										   param.location().place());
					id.append(param.location().contact());

					Hash hashid = calcHash(id);

					if (mesaureSignalMap.contains(hashid) == true)
					{
						quint64 index = mesaureSignalMap[hashid];
						if (index < m_signalMeasureList.size())
						{
							int channel = param.location().rack().channel();
							if (channel >= 0 && channel < measureSignal.channelCount())
							{
								if (m_signalMeasureList[index].setMetrologySignal(measureKind,
																				  m_connectionBase,
																				  connectionType,
																				  channel,
																				  &m_signalList[i]) == false)
								{
									continue;
								}
							}
						}

						continue;
					}

					mesaureSignalMap.insert(hashid, signalIndex);

					int channel = param.location().rack().channel();
					if (channel >= 0 && channel < measureSignal.channelCount())
					{
						if (measureSignal.setMetrologySignal(measureKind,
															 m_connectionBase,
															 connectionType,
															 channel,
															 &m_signalList[i]) == false)
						{
							mesaureSignalMap.remove(hashid);
							continue;
						}
					}
				}
				break;

			default:
				assert(false);
				continue;
		}

		if (measureSignal.isEmpty() == true)
		{
			continue;
		}

		m_signalMeasureList.push_back(measureSignal);

		signalIndex++;
	}

	return TO_INT(signalIndex);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::clearSignalListForMeasure()
{
	QMutexLocker l(&m_signalMesaureMutex);

	m_signalMeasureList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal SignalBase::activeSignal() const
{
	QMutexLocker l(&m_activeSignalMutex);

	return m_activeSignal;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setActiveSignal(const MeasureSignal& signal)
{
	QMutexLocker la(&m_activeSignalMutex);

	m_activeSignal = signal;

	QMutexLocker ls(&m_stateMutex);

	m_requestStateList.clear();

	for(int channel = 0; channel < signal.channelCount(); channel++)
	{
		// append hash of input signal
		//
		Metrology::Signal* pSignal = m_activeSignal.multiChannelSignal(Metrology::ConnectionIoType::Source).metrologySignal(channel);
		if (pSignal == nullptr || pSignal->param().isValid() == false)
		{
			continue;
		}

		m_requestStateList.push_back(pSignal->param().hash());

		// append hash of signal that contains ID of module for input signal - Serial Number
		//
		QString appSignalID_of_SerialNo = findAppSignalIDforSerialNo(pSignal->param().location().moduleID());
		if (appSignalID_of_SerialNo.isEmpty() == false)
		{
			pSignal->param().location().setModuleSerialNoID(appSignalID_of_SerialNo);
			m_requestStateList.push_back(calcHash(appSignalID_of_SerialNo));
		}

		// append hash of comparators input signal
		//
		int comparatorCount = pSignal->param().comparatorCount();
		for (int c = 0; c < comparatorCount; c++)
		{
			std::shared_ptr<Metrology::ComparatorEx> comparatorEx = pSignal->param().comparator(c);
			if (comparatorEx == nullptr)
			{
				continue;
			}

			if (comparatorEx->signalsIsValid() == false)
			{
				continue;
			}

			if (comparatorEx->compare().isConst() == false)
			{
				m_requestStateList.push_back(comparatorEx->compareSignal()->param().hash());
			}

			if (comparatorEx->hysteresis().isConst() == false)
			{
				m_requestStateList.push_back(comparatorEx->hysteresisSignal()->param().hash());
			}

			m_requestStateList.push_back(comparatorEx->outputSignal()->param().hash());
		}

		// if input has not output signals got to next channel
		//
		if (m_activeSignal.connectionType() == Metrology::ConnectionType::Unused)
		{
			continue;
		}

		// append hash of output signal
		//
		pSignal = m_activeSignal.multiChannelSignal(Metrology::ConnectionIoType::Destination).metrologySignal(channel);
		if (pSignal == nullptr || pSignal->param().isValid() == false)
		{
			continue;
		}

		m_requestStateList.push_back(pSignal->param().hash());

		// append hash of signal that contains ID of module for output signal
		//
		appSignalID_of_SerialNo = findAppSignalIDforSerialNo(pSignal->param().location().moduleID());
		if (appSignalID_of_SerialNo.isEmpty() == false)
		{
			pSignal->param().location().setModuleSerialNoID(appSignalID_of_SerialNo);
			m_requestStateList.push_back(calcHash(appSignalID_of_SerialNo));
		}

		// append hash of comparators output signal
		//
		comparatorCount = pSignal->param().comparatorCount();
		for (int c = 0; c < comparatorCount; c++)
		{
			std::shared_ptr<Metrology::ComparatorEx> comparatorEx = pSignal->param().comparator(c);
			if (comparatorEx == nullptr)
			{
				continue;
			}

			if (comparatorEx->signalsIsValid() == false)
			{
				continue;
			}

			if (comparatorEx->compare().isConst() == false)
			{
				m_requestStateList.push_back(comparatorEx->compareSignal()->param().hash());
			}

			if (comparatorEx->hysteresis().isConst() == false)
			{
				m_requestStateList.push_back(comparatorEx->hysteresisSignal()->param().hash());
			}

			m_requestStateList.push_back(comparatorEx->outputSignal()->param().hash());
		}
	}

	emit activeSignalChanged(m_activeSignal);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::clearActiveSignal()
{
	QMutexLocker la(&m_activeSignalMutex);

	m_activeSignal.clear();

	QMutexLocker ls(&m_stateMutex);

	m_requestStateList.clear();

	emit activeSignalChanged(m_activeSignal);
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::loadComparatorsInSignal(const ComparatorSet& comparatorSet)
{
	QStringList appSignalIDList = comparatorSet.inputSignalIDs();

	for(const QString& appSignalID : appSignalIDList)
	{
		if (appSignalID.isEmpty() == true)
		{
			continue;
		}

		Metrology::Signal* pInputSignal = signalPtr(appSignalID);
		if (pInputSignal == nullptr || pInputSignal->param().isValid() == false)
		{
			continue;
		}

		std::vector<std::shared_ptr<Metrology::ComparatorEx>> metrologyComparatorList;

		QVector<std::shared_ptr<Comparator>> comparatorList = comparatorSet.getByInputSignalID(appSignalID);
		for(std::shared_ptr<Comparator> comparator : comparatorList)
		{
			std::shared_ptr<Metrology::ComparatorEx> comparatorEx;

			switch (comparator->cmpType())
			{
				case E::CmpType::Equal:
					{
						//
						//
						comparatorEx = std::make_shared<Metrology::ComparatorEx>(comparator.get());

						comparatorEx->setIndex(TO_INT(metrologyComparatorList.size()));
						initComparatorSignals(comparatorEx.get());

						comparatorEx->setCmpType(E::CmpType::Less);
						comparatorEx->setDeviation(Metrology::ComparatorEx::DeviationType::Up);

						metrologyComparatorList.push_back(comparatorEx);

						//
						//
						comparatorEx = std::make_shared<Metrology::ComparatorEx>(comparator.get());

						comparatorEx->setIndex(TO_INT(metrologyComparatorList.size()));
						initComparatorSignals(comparatorEx.get());

						comparatorEx->setCmpType(E::CmpType::Greate);
						comparatorEx->setDeviation(Metrology::ComparatorEx::DeviationType::Down);

						metrologyComparatorList.push_back(comparatorEx);
					}

					break;

				case E::CmpType::NotEqual:
					{
						//
						//
						comparatorEx = std::make_shared<Metrology::ComparatorEx>(comparator.get());

						comparatorEx->setIndex(TO_INT(metrologyComparatorList.size()));
						initComparatorSignals(comparatorEx.get());

						comparatorEx->setCmpType(E::CmpType::Greate);
						comparatorEx->setDeviation(Metrology::ComparatorEx::DeviationType::Up);

						metrologyComparatorList.push_back(comparatorEx);

						//
						//
						comparatorEx = std::make_shared<Metrology::ComparatorEx>(comparator.get());

						comparatorEx->setIndex(TO_INT(metrologyComparatorList.size()));
						initComparatorSignals(comparatorEx.get());

						comparatorEx->setCmpType(E::CmpType::Less);
						comparatorEx->setDeviation(Metrology::ComparatorEx::DeviationType::Down);

						metrologyComparatorList.push_back(comparatorEx);
					}

					break;

				default:
					{
						comparatorEx = std::make_shared<Metrology::ComparatorEx>(comparator.get());

						comparatorEx->setIndex(TO_INT(metrologyComparatorList.size()));
						initComparatorSignals(comparatorEx.get());
						metrologyComparatorList.push_back(comparatorEx);
					}

					break;
			}
		}

		pInputSignal->param().setComparatorList(metrologyComparatorList);
	}

	m_statisticsBase.createComparatorList();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool SignalBase::initComparatorSignals(Metrology::ComparatorEx* pComparatorEx)
{
	if (pComparatorEx == nullptr)
	{
		return false;
	}

	if (pComparatorEx->input().appSignalID().isEmpty() == false)
	{
		pComparatorEx->setInputSignal(signalPtr(pComparatorEx->input().appSignalID()));
	}

	if (pComparatorEx->compare().isConst() == false && pComparatorEx->compare().appSignalID().isEmpty() == false)
	{
		pComparatorEx->setCompareSignal(signalPtr(pComparatorEx->compare().appSignalID()));
	}

	if (pComparatorEx->hysteresis().isConst() == false && pComparatorEx->hysteresis().appSignalID().isEmpty() == false)
	{
		pComparatorEx->setHysteresisSignal(signalPtr(pComparatorEx->hysteresis().appSignalID()));
	}

	if (pComparatorEx->output().appSignalID().isEmpty() == false)
	{
		pComparatorEx->setOutputSignal(signalPtr(pComparatorEx->output().appSignalID()));
	}

	return pComparatorEx->signalsIsValid();
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
