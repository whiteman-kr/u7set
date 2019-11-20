#include "SignalBase.h"

#include "CalibratorBase.h"
#include "MeasureBase.h"
#include "Database.h"
#include "Options.h"

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
	bool valid = true;

	m_mutex.lock();


		if (m_signalConnectionType == SIGNAL_CONNECTION_TYPE_UNUSED)
		{
			valid = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT].isValid();			// only input
		}
		else
		{
			for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)		// input and output
			{
				if (m_param[type].isValid() == false)
				{
					valid = false;

					break;
				}
			}
		}

	 m_mutex.unlock();

	return valid;
}

// -------------------------------------------------------------------------------------------------------------------

void IoSignalParam::clear()
{
	m_mutex.lock();

		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
		{
			m_param[type].setAppSignalID(QString());
		}

		m_signalConnectionType = SIGNAL_CONNECTION_TYPE_UNUSED;

		m_pCalibratorManager = nullptr;
		m_percent = 0;
		m_negativeRange = false;
		m_tunSignalState = 0;

	m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::SignalParam IoSignalParam::param(int type) const
{
	if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return Metrology::SignalParam();
	}

	Metrology::SignalParam param;

	m_mutex.lock();

		param = m_param[type];

	m_mutex.unlock();

	return param;
}

// -------------------------------------------------------------------------------------------------------------------

bool IoSignalParam::setParam(int type, const Metrology::SignalParam& param)
{
	if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return false;
	}

	if (param.isValid() == false)
	{
		return false;
	}

	m_mutex.lock();

		m_param[type] = param;

	m_mutex.unlock();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::rackCaption() const
{
	QString result;

	m_mutex.lock();

		if (m_signalConnectionType == SIGNAL_CONNECTION_TYPE_UNUSED)
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (param.isValid() == true)
			{
				result = param.location().rack().caption();
			}
		}
		else
		{
			const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (inParam.isValid() == true)
			{
				result = inParam.location().rack().caption() + MULTI_TEXT_DEVIDER;
			}

			const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
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

	m_mutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::appSignalID() const
{
	QString result;

	m_mutex.lock();

		if (m_signalConnectionType == SIGNAL_CONNECTION_TYPE_UNUSED)
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (param.isValid() == true)
			{
				result = param.appSignalID();
			}
		}
		else
		{
			const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (inParam.isValid() == true)
			{
				result = inParam.appSignalID() + MULTI_TEXT_DEVIDER;
			}

			const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
			if (outParam.isValid() == true)
			{
				result += outParam.appSignalID();
			}
		}

	m_mutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::customSignalID() const
{
	QString result;

	m_mutex.lock();

		if (m_signalConnectionType == SIGNAL_CONNECTION_TYPE_UNUSED)
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (param.isValid() == true)
			{
				result = param.customAppSignalID();
			}
		}
		else
		{
			const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (inParam.isValid() == true)
			{
				result = inParam.customAppSignalID() + MULTI_TEXT_DEVIDER;
			}

			const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
			if (outParam.isValid() == true)
			{
				result += outParam.customAppSignalID();
			}
		}

	m_mutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::equipmentID() const
{
	QString result;

	m_mutex.lock();

		if (m_signalConnectionType == SIGNAL_CONNECTION_TYPE_UNUSED)
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (param.isValid() == true)
			{
				result = param.location().equipmentID();
			}
		}
		else
		{
			const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (inParam.isValid() == true)
			{
				result = inParam.location().equipmentID() + MULTI_TEXT_DEVIDER;
			}

			const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
			if (outParam.isValid() == true)
			{
				if (inParam.location().equipmentID() != outParam.location().equipmentID())
				{
					result += outParam.location().equipmentID();
				}
				else
				{
					result = outParam.location().equipmentID();
				}
			}
		}

	m_mutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::chassisStr() const
{
	QString result;

	m_mutex.lock();

		if (m_signalConnectionType == SIGNAL_CONNECTION_TYPE_UNUSED)
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (param.isValid() == true)
			{
				result = param.location().chassisStr();
			}
		}
		else
		{
			const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (inParam.isValid() == true)
			{
				result = inParam.location().chassisStr() + MULTI_TEXT_DEVIDER;
			}

			const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
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

	m_mutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::moduleStr() const
{
	QString result;

	m_mutex.lock();

		if (m_signalConnectionType == SIGNAL_CONNECTION_TYPE_UNUSED)
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (param.isValid() == true)
			{
				result = param.location().moduleStr();
			}
		}
		else
		{
			const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (inParam.isValid() == true)
			{
				result = inParam.location().moduleStr() + MULTI_TEXT_DEVIDER;
			}

			const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
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

	m_mutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::placeStr() const
{
	QString result;

	m_mutex.lock();

		if (m_signalConnectionType == SIGNAL_CONNECTION_TYPE_UNUSED)
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (param.isValid() == true)
			{
				result = param.location().placeStr();
			}
		}
		else
		{
			const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (inParam.isValid() == true)
			{
				result = inParam.location().placeStr() + MULTI_TEXT_DEVIDER;
			}

			const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
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

	m_mutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::caption() const
{
	QString result;

	m_mutex.lock();

		if (m_signalConnectionType == SIGNAL_CONNECTION_TYPE_UNUSED)
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (param.isValid() == true)
			{
				result = param.caption();
			}
		}
		else
		{
			const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (inParam.isValid() == true)
			{
				result = inParam.caption() + MULTI_TEXT_DEVIDER;
			}

			const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
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

	m_mutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::electricRangeStr() const
{
	QString result;

	m_mutex.lock();

		switch(m_signalConnectionType)
		{
			case SIGNAL_CONNECTION_TYPE_UNUSED:
				{
					const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
					if (param.isValid() == true)
					{
						result = param.electricRangeStr();
					}
				}

				break;

			case SIGNAL_CONNECTION_TYPE_FROM_INPUT:
				{
					const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
					if (inParam.isValid() == true)
					{
						result = inParam.electricRangeStr() + MULTI_TEXT_DEVIDER;
					}

					const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
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

			case SIGNAL_CONNECTION_TYPE_FROM_TUNING:
				{
					const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
					if (param.isValid() == true)
					{
						result = param.electricRangeStr();
					}
				}

				break;

			default: assert(0);
		}

	m_mutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::electricSensorStr() const
{
	QString result;

	m_mutex.lock();

		switch(m_signalConnectionType)
		{
			case SIGNAL_CONNECTION_TYPE_UNUSED:
				{
					const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
					if (param.isValid() == true)
					{
						result = param.electricSensorTypeStr();
					}
				}

				break;

			case SIGNAL_CONNECTION_TYPE_FROM_INPUT:
				{
					const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
					if (inParam.isValid() == true)
					{
						result = inParam.electricSensorTypeStr() + MULTI_TEXT_DEVIDER;
					}

					const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
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

			case SIGNAL_CONNECTION_TYPE_FROM_TUNING:
				{
					const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
					if (param.isValid() == true)
					{
						result = param.electricSensorTypeStr();
					}
				}

				break;

			default: assert(0);
		}

	m_mutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::physicalRangeStr() const
{
	QString result;

	m_mutex.lock();

		if (m_signalConnectionType == SIGNAL_CONNECTION_TYPE_UNUSED)
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (param.isValid() == true)
			{
				result = param.physicalRangeStr();
			}
		}
		else
		{
			const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (inParam.isValid() == true)
			{
				result = inParam.physicalRangeStr() + MULTI_TEXT_DEVIDER;
			}

			const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
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

	m_mutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::engineeringRangeStr() const
{
	QString result;

	m_mutex.lock();

		switch (m_signalConnectionType)
		{
			case SIGNAL_CONNECTION_TYPE_UNUSED:
				{
					const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
					if (param.isValid() == true)
					{
						result = param.engineeringRangeStr();
					}
				}
				break;

			case SIGNAL_CONNECTION_TYPE_FROM_INPUT:
				{
					const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
					if (inParam.isValid() == true)
					{
						result = inParam.engineeringRangeStr() + MULTI_TEXT_DEVIDER;
					}

					const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
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

			case SIGNAL_CONNECTION_TYPE_FROM_TUNING:
				{
					const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
					if (inParam.isValid() == true)
					{
						result = inParam.tuningRangeStr() + MULTI_TEXT_DEVIDER;
					}

					const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
					if (outParam.isValid() == true)
					{
						if (inParam.tuningLowBound().toDouble() != outParam.lowEngineeringUnits() || inParam.tuningHighBound().toDouble() != outParam.highEngineeringUnits())
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


	m_mutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString IoSignalParam::calibratorStr() const
{
	if (m_pCalibratorManager == nullptr || m_pCalibratorManager->calibratorIsConnected() == false)
	{
		return QString("Not connected");
	}

	return QString("Calibrator %1 (%2)").arg(m_pCalibratorManager->calibratorChannel() + 1).arg(m_pCalibratorManager->calibratorPort());
}

// -------------------------------------------------------------------------------------------------------------------

IoSignalParam& IoSignalParam::operator=(const IoSignalParam& from)
{
	m_mutex.lock();

		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
		{
			m_param[type] = from.m_param[type];
		}

		m_signalConnectionType = from.m_signalConnectionType;

		m_pCalibratorManager = from.m_pCalibratorManager;
		m_percent = from.m_percent;
		m_negativeRange  = from.m_negativeRange;
		m_tunSignalState = from.m_tunSignalState;

	m_mutex.unlock();

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
	m_mutex.lock();

		m_pSignalList.fill(nullptr, m_channelCount);
		m_location.clear();
		m_strID.clear();

	m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

bool MultiChannelSignal::isEmpty() const
{
	bool empty = true;

	m_mutex.lock();

		for(int ch = 0; ch < m_channelCount; ch++)
		{
			if (m_pSignalList[ch] != nullptr)
			{
				empty = false;

				break;
			}
		}

	 m_mutex.unlock();

	return empty;
}

// -------------------------------------------------------------------------------------------------------------------


void MultiChannelSignal::setChannelCount(int count)
{
	m_mutex.lock();

		m_channelCount = count;
		m_pSignalList.fill(nullptr, count);

	m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal* MultiChannelSignal::metrologySignal(int channel) const
{
	if (channel < 0 || channel >= m_channelCount)
	{
		assert(0);
		return nullptr;
	}

	Metrology::Signal* pSignal = nullptr;

	m_mutex.lock();

		pSignal = m_pSignalList[channel];

	m_mutex.unlock();

	return pSignal;
}

// -------------------------------------------------------------------------------------------------------------------

bool MultiChannelSignal::setMetrologySignal(int measureKind, int channel, Metrology::Signal* pSignal)
{
	if (measureKind < 0 || measureKind >= MEASURE_KIND_COUNT)
	{
		assert(0);
		return false;
	}

	if (channel < 0 || channel >= m_channelCount)
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

	m_mutex.lock();

		m_pSignalList[channel] = pSignal;

		m_location.setRack(param.location().rack());
		m_location.setChassis(param.location().chassis());
		m_location.setModule(param.location().module());
		m_location.setPlace(param.location().place());
		m_location.setContact(param.location().contact());

		switch(measureKind)
		{
			case MEASURE_KIND_ONE_RACK:		m_strID = param.customAppSignalID();																			break;
			case MEASURE_KIND_ONE_MODULE:	m_strID = param.location().moduleID();																			break;
			case MEASURE_KIND_MULTI_RACK:	m_strID.sprintf("CH %02d _ MD %02d _ IN %02d", m_location.chassis(), m_location.module(), m_location.place());	break;
			default:						assert(false);
		}

	m_mutex.unlock();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal* MultiChannelSignal::firstMetrologySignal() const
{
	Metrology::Signal* pSignal = nullptr;

	m_mutex.lock();

		for(int ch = 0; ch < m_channelCount; ch++ )
		{
			if (m_pSignalList[ch] != nullptr)
			{
				pSignal = m_pSignalList[ch];
				break;
			}
		}

	m_mutex.unlock();

	return pSignal;
}

// -------------------------------------------------------------------------------------------------------------------

MultiChannelSignal& MultiChannelSignal::operator=(const MultiChannelSignal& from)
{
	m_mutex.lock();

		m_channelCount = from.m_channelCount;
		m_pSignalList = from.m_pSignalList;
		m_location = from.m_location;
		m_strID = from.m_strID;

	m_mutex.unlock();

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
	m_mutex.lock();

		m_signalConnectionType = SIGNAL_CONNECTION_TYPE_UNUSED;

		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
		{
			m_signal[type].clear();
		}

	m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureSignal::isEmpty() const
{
	bool empty = true;

	m_mutex.lock();

		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
		{
			if (m_signal[type].isEmpty() == false)
			{
				empty = false;

				break;
			}
		}

	 m_mutex.unlock();

	return empty;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureSignal::setChannelCount(int count)
{
	m_mutex.lock();

		m_channelCount = count;

		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
		{
			m_signal[type].setChannelCount(count);
		}

	m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

MultiChannelSignal MeasureSignal::multiChannelSignal(int type) const
{
	if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return MultiChannelSignal();
	}

	MultiChannelSignal signal;

	m_mutex.lock();

		signal = m_signal[type];

	m_mutex.unlock();

	return signal;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureSignal::setMultiSignal(int type, const MultiChannelSignal& signal)
{
	if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return false;
	}

	m_mutex.lock();

		m_signal[type] = signal;

	m_mutex.unlock();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal* MeasureSignal::metrologySignal(int type, int channel) const
{
	if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return nullptr;
	}

	if (channel < 0 || channel >= m_channelCount)
	{
		return nullptr;
	}

	Metrology::Signal* pSignal = nullptr;

	m_mutex.lock();

		pSignal = m_signal[type].metrologySignal(channel);

	m_mutex.unlock();

	return pSignal;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureSignal::setMetrologySignal(int measureKind, const SignalConnectionBase& signalConnections, int signalConnectionType, int channel, Metrology::Signal* pSignal)
{
	if (measureKind < 0 || measureKind >= MEASURE_KIND_COUNT)
	{
		assert(0);
		return false;
	}

	if (signalConnectionType < 0 || signalConnectionType >= SIGNAL_CONNECTION_TYPE_COUNT)
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

	bool result = true;

	switch (signalConnectionType)
	{
		case SIGNAL_CONNECTION_TYPE_UNUSED:
			{
				m_mutex.lock();

					m_signalConnectionType = SIGNAL_CONNECTION_TYPE_UNUSED;

					result = m_signal[MEASURE_IO_SIGNAL_TYPE_INPUT].setMetrologySignal(measureKind, channel, pSignal);

				m_mutex.unlock();
			}
			break;

		case SIGNAL_CONNECTION_TYPE_FROM_INPUT:
		case SIGNAL_CONNECTION_TYPE_FROM_TUNING:
			{
				// find index of signal connection in the base by input signal
				//
				int index = signalConnections.findIndex(signalConnectionType, MEASURE_IO_SIGNAL_TYPE_INPUT, pSignal);
				if (index == -1)
				{
					result = false;
					break;
				}

				// take signal connection in the base by index
				//
				SignalConnection signalConnection = signalConnections.connection(index);
				if (signalConnection.isValid() == false)
				{
					result = false;
					break;
				}

				m_mutex.lock();

					m_signalConnectionType = signalConnectionType;

					for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type ++)
					{
						Metrology::Signal* pSignalFromConnection = signalConnection.metrologySignal(type);
						if (pSignalFromConnection == nullptr || pSignalFromConnection->param().isValid() == false)
						{
							result = false;
							break;
						}

						if (m_signal[type].setMetrologySignal(measureKind, channel, pSignalFromConnection) == false)
						{
							result = false;
							break;
						}
					}

				m_mutex.unlock();
			}

			break;

		default:
			assert(0);
	}

	if (result == false)
	{
		clear();
	}

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureSignal::contains(Metrology::Signal* pSignal)
{
	if (pSignal == nullptr || pSignal->param().isValid() == false)
	{
		return false;
	}

	bool result = false;

	for(int ch = 0; ch < m_channelCount; ch++)
	{
		for(int t = 0; t < MEASURE_IO_SIGNAL_TYPE_COUNT; t++)
		{
			if (m_signal[t].metrologySignal(ch) == pSignal)
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
	m_mutex.lock();

		m_signalConnectionType = from.m_signalConnectionType;

		m_channelCount = from.m_channelCount;

		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
		{
			m_signal[type] = from.m_signal[type];
		}

	m_mutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

SignalBase theSignalBase;

// -------------------------------------------------------------------------------------------------------------------

SignalBase::SignalBase(QObject *parent) :
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
	int count = 0;

	m_signalMutex.lock();

		count = m_signalList.count();

	m_signalMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::clearSignalList()
{
	m_signalMutex.lock();

	m_signalConnectionBase.empty();		// set all signal connection value nullptr
	m_tuningBase.Signals().clear();		// remove all tuning signals
	m_statisticBase.clear();

	m_signalHashMap.clear();
	m_signalList.clear();

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::appendSignal(const Metrology::SignalParam& param)
{
	if (param.appSignalID().isEmpty() == true || param.hash() == UNDEFINED_HASH)
	{
		assert(false);
		return -1;
	}

	int index = -1;

	m_signalMutex.lock();

		if (m_signalHashMap.contains(param.hash()) == false)
		{
			Metrology::Signal metrologySignal(param);

			m_signalList.append(metrologySignal);
			index = m_signalList.count() - 1;

			m_signalHashMap.insert(param.hash(), index);
		}

	 m_signalMutex.unlock();

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
		assert(hash != 0);
		return nullptr;
	}

	Metrology::Signal* pSignal = nullptr;

	m_signalMutex.lock();

		if (m_signalHashMap.contains(hash) == true)
		{
			int index = m_signalHashMap[hash];

			if (index >= 0 && index < m_signalList.count())
			{
				pSignal = &m_signalList[index];
			}
		}

	m_signalMutex.unlock();

	return pSignal;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal* SignalBase::signalPtr(int index)
{
	Metrology::Signal* pSignal = nullptr;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			pSignal = &m_signalList[index];
		}

	m_signalMutex.unlock();

	return pSignal;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal SignalBase::signal(const QString& appSignalID)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(false);
		return Metrology::Signal();
	}

	return signal(calcHash(appSignalID));
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal SignalBase::signal(const Hash& hash)
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != 0);
		return Metrology::Signal();
	}

	Metrology::Signal signal;

	m_signalMutex.lock();

		if (m_signalHashMap.contains(hash) == true)
		{
			int index = m_signalHashMap[hash];

			if (index >= 0 && index < m_signalList.count())
			{
				signal = m_signalList[index];
			}
		}

	m_signalMutex.unlock();

	return signal;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal SignalBase::signal(int index)
{
	Metrology::Signal signal;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			signal = m_signalList[index];
		}

	m_signalMutex.unlock();

	return signal;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::SignalParam SignalBase::signalParam(const QString& appSignalID)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(false);
		return Metrology::SignalParam();
	}

	return signalParam(calcHash(appSignalID));
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::SignalParam SignalBase::signalParam(const Hash& hash)
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != UNDEFINED_HASH);
		return Metrology::SignalParam();
	}

	Metrology::SignalParam param;

	m_signalMutex.lock();

		if (m_signalHashMap.contains(hash) == true)
		{
			int index = m_signalHashMap[hash];

			if (index >= 0 && index < m_signalList.count())
			{
				param = m_signalList[index].param();
			}
		}

	m_signalMutex.unlock();

	return param;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::SignalParam SignalBase::signalParam(int index)
{
	Metrology::SignalParam param;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			param = m_signalList[index].param();
		}

	m_signalMutex.unlock();

	return param;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignalParam(const Hash& hash, const Metrology::SignalParam& param)
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != UNDEFINED_HASH);
		return;
	}

	m_signalMutex.lock();

		if (m_signalHashMap.contains(hash) == true)
		{
			int index = m_signalHashMap[hash];

			if (index >= 0 && index < m_signalList.count())
			{
				m_signalList[index].setParam(param);

				emit updatedSignalParam(param.appSignalID());
			}
		}

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignalParam(int index, const Metrology::SignalParam& param)
{
	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			m_signalList[index].setParam(param);

			emit updatedSignalParam(param.appSignalID());
		}

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::SignalState SignalBase::signalState(const QString& appSignalID)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(false);
		return Metrology::SignalState();
	}

	return signalState(calcHash(appSignalID));
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::SignalState SignalBase::signalState(const Hash& hash)
{
	if (hash == UNDEFINED_HASH)
	{
		assert(hash != UNDEFINED_HASH);
		return Metrology::SignalState();
	}

	Metrology::SignalState state;

	m_signalMutex.lock();

		if (m_signalHashMap.contains(hash) == true)
		{
			int index = m_signalHashMap[hash];

			if (index >= 0 && index < m_signalList.count())
			{
				state = m_signalList[index].state();
			}
		}

	m_signalMutex.unlock();

	return state;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::SignalState SignalBase::signalState(int index)
{
	Metrology::SignalState state;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			state = m_signalList[index].state();
		}

	m_signalMutex.unlock();

	return state;
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

	int index = -1;

	m_signalMutex.lock();

		if (m_signalHashMap.contains(hash) == true)
		{
			index = m_signalHashMap[hash];

			if (index >= 0 && index < m_signalList.count())
			{
				m_signalList[index].setState(state);
			}
		}

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignalState(int index, const Metrology::SignalState& state)
{
	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.count())
		{
			m_signalList[index].setState(state);
		}

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::hashForRequestStateCount() const
{
	int count = 0;

	m_stateMutex.lock();

		count = m_requestStateList.count();

	m_stateMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

Hash SignalBase::hashForRequestState(int index)
{
	Hash hash = UNDEFINED_HASH;

	m_stateMutex.lock();

		if (index >= 0 && index < m_requestStateList.count())
		{
			hash = m_requestStateList[index];
		}

	m_stateMutex.unlock();

	return hash;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::rackCountForMeasure() const
{
	int count = 0;

	m_rackMutex.lock();

		count = m_rackList.count();

	m_rackMutex.unlock();

	return count;
}


// -------------------------------------------------------------------------------------------------------------------

Metrology::RackParam SignalBase::rackForMeasure(int index)
{
	Metrology::RackParam param;

	m_rackMutex.lock();

		if (index >= 0 && index < m_rackList.count())
		{
			param = m_rackList[index];
		}

	m_rackMutex.unlock();

	return param;
}

// -------------------------------------------------------------------------------------------------------------------

QString SignalBase::findAppSignalIDforSerialNo(const QString& moduleID)
{
	QString appSignalID;

	int signalCount = m_signalList.count();
	for(int i = 0; i < signalCount; i++)
	{
		Metrology::Signal& signal = m_signalList[i];

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

		appSignalID = signal.param().location().appSignalID();

		break;
	}

	return appSignalID;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::createRackListForMeasure(int signalConnectionType)
{
	if (signalConnectionType < 0 || signalConnectionType >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		assert(false);
		return 0;
	}

	QMap<Hash, int> rackHashMap;

	// find all type of racks for selected signalConnectionType and create rackTypeList for ToolBar
	//
	m_rackMutex.lock();

		m_rackList.clear();

		int signalCount = m_signalList.count();

		// select racks that has signals, other racks ignore
		//
		for(int i = 0; i < signalCount; i ++)
		{
			Metrology::SignalParam& param = m_signalList[i].param();
			if (param.isValid() == false)
			{
				continue;
			}

			if (param.isAnalog() == false)
			{
				continue;
			}

			if (param.location().chassis() == -1 || param.location().module() == -1)
			{
				continue;
			}

			switch (signalConnectionType)
			{
				case SIGNAL_CONNECTION_TYPE_UNUSED:
				case SIGNAL_CONNECTION_TYPE_FROM_INPUT:

					if (param.isInput() == false)
					{
						continue;
					}

					if (param.location().place() == -1)
					{
						continue;
					}

					break;

				case SIGNAL_CONNECTION_TYPE_FROM_TUNING:

					if (param.isInternal() == false)
					{
						continue;
					}

					break;

				default:
					assert(0);
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

				m_rackList.append(param.location().rack());
			}
		}

		//
		//
		int rackCount = m_rackList.count();

		for(int i = 0; i < rackCount - 1; i++)
		{
			for(int j = i+1; j < rackCount; j++)
			{
				if (m_rackList[i].index() > m_rackList[j].index())
				{
					Metrology::RackParam rack	= m_rackList[ i ];
					m_rackList[ i ]				= m_rackList[ j ];
					m_rackList[ j ]				= rack;
				}
			}
		}

	m_rackMutex.unlock();

	return rackCount;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::clearRackListForMeasure()
{
	m_rackMutex.lock();

		m_rackList.clear();

	m_rackMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::initSignals()
{
	int analogTuningSignalCount = 0;
	int discreteTuningSignalCount = 0;
	int busTuningSignalCount = 0;

	m_signalMutex.lock();

		int count = m_signalList.count();

		for(int i = 0; i < count; i ++)
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

	updateRackParam();

	m_signalConnectionBase.init();

	m_tuningBase.Signals().createSignalList();

	m_statisticBase.createSignalList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::updateRackParam()
{
	m_signalMutex.lock();

		int count = m_signalList.count();

		for(int i = 0; i < count; i ++)
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

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::signalForMeasureCount() const
{
	int count;

	m_signalMesaureMutex.lock();

		count = m_signalMeasureList.count();

	m_signalMesaureMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal SignalBase::signalForMeasure(int index)
{
	MeasureSignal signal;

	m_signalMesaureMutex.lock();

		if (index >= 0 && index < m_signalMeasureList.count())
		{
			signal = m_signalMeasureList[index];
		}

	m_signalMesaureMutex.unlock();

	return signal;
}


// -------------------------------------------------------------------------------------------------------------------

int SignalBase::createSignalListForMeasure(int measureKind, int signalConnectionType, int rackIndex)
{
	if (measureKind < 0 || measureKind >= MEASURE_KIND_COUNT)
	{
		assert(false);
		return 0;
	}

	if (signalConnectionType < 0 || signalConnectionType >= SIGNAL_CONNECTION_TYPE_COUNT)
	{
		assert(false);
		return 0;
	}

	if (rackIndex == -1)
	{
		assert(false);
		return 0;
	}

	int				signalIndex = 0;
	QMap<Hash, int>	mesaureSignalMap;

	// determine the number of channels for a multi-channel signal
	//
	int channelCount = 0;

	switch(measureKind)
	{
		case MEASURE_KIND_ONE_RACK:		channelCount = 1;						break;
		case MEASURE_KIND_ONE_MODULE:	channelCount = Metrology::InputCount;	break;
		case MEASURE_KIND_MULTI_RACK:	channelCount = Metrology::ChannelCount;	break;
		default:						assert(0);
	}

	MeasureSignal measureSignal;
	measureSignal.setChannelCount(channelCount);

	// find all signals for selected rack or group and create Measure Signal List map for ToolBar
	//
	m_signalMesaureMutex.lock();

		m_signalMeasureList.clear();

		int signalCount = m_signalList.count();

		for(int i = 0; i < signalCount; i ++)
		{
			measureSignal.clear();

			Metrology::SignalParam& param = m_signalList[i].param();
			if (param.isValid() == false)
			{
				continue;
			}

			if (param.isAnalog() == false)
			{
				continue;
			}

			if (param.location().chassis() == -1 || param.location().module() == -1)
			{
				continue;
			}

			// switch for signal connection type
			//
			switch (signalConnectionType)
			{
				case SIGNAL_CONNECTION_TYPE_UNUSED:

					if (param.isInput() == false)
					{
						continue;
					}

					if (param.location().place() == -1)
					{
						continue;
					}

					if (param.electricUnitID() == E::ElectricUnit::NoUnit)
					{
						continue;
					}

					break;

				case SIGNAL_CONNECTION_TYPE_FROM_INPUT:

					if (param.isInput() == false)
					{
						continue;
					}

					if (param.location().place() == -1)
					{
						continue;
					}

					if (param.electricUnitID() == E::ElectricUnit::NoUnit)
					{
						continue;
					}

					if (m_signalConnectionBase.findIndex(signalConnectionType, MEASURE_IO_SIGNAL_TYPE_INPUT, &m_signalList[i]) == -1)
					{
						continue;
					}

					break;

				case SIGNAL_CONNECTION_TYPE_FROM_TUNING:

					if (param.isInternal() == false)
					{
						continue;
					}

					if (m_signalConnectionBase.findIndex(signalConnectionType, MEASURE_IO_SIGNAL_TYPE_INPUT, &m_signalList[i]) == -1)
					{
						continue;
					}

					break;

				default:
					assert(0);
					continue;
			}

			// switch for Measure kind
			//
			switch(measureKind)
			{
				case MEASURE_KIND_ONE_RACK:
					{
						if (param.location().rack().index() != rackIndex)
						{
							continue;
						}

						if (measureSignal.setMetrologySignal(measureKind, m_signalConnectionBase, signalConnectionType, Metrology::Channel_0, &m_signalList[i]) == false)
						{
							continue;
						}
					}
					break;

				case MEASURE_KIND_ONE_MODULE:
					{
						if (param.location().rack().index() != rackIndex)
						{
							continue;
						}

						QString id;
						id.sprintf("%d - %d - %d",
									param.location().rack().index(),
									param.location().chassis(),
									param.location().module());

						Hash hashid = calcHash(id);

						if (mesaureSignalMap.contains(hashid) == true)
						{
							int index = mesaureSignalMap[hashid];
							if (index >= 0 && index < m_signalMeasureList.count())
							{
								int channel = param.location().place() - 1;
								if (channel >= 0 && channel < measureSignal.channelCount())
								{
									if (m_signalMeasureList[index].metrologySignal(signalConnectionType, channel) != nullptr)
									{
										continue;
									}

									if (m_signalMeasureList[index].setMetrologySignal(measureKind, m_signalConnectionBase, signalConnectionType, channel, &m_signalList[i]) == false)
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
							if (measureSignal.setMetrologySignal(measureKind, m_signalConnectionBase, signalConnectionType, channel, &m_signalList[i]) == false)
							{
								mesaureSignalMap.remove(hashid);
								continue;
							}
						}
					}
					break;

				case MEASURE_KIND_MULTI_RACK:
					{
						if (param.location().rack().groupIndex() != rackIndex)
						{
							continue;
						}

						QString id;
						id.sprintf("%d - %d - %d - %d - ",
									param.location().rack().groupIndex(),
									param.location().chassis(),
									param.location().module(),
									param.location().place());
									id.append(param.location().contact());

						Hash hashid = calcHash(id);

						if (mesaureSignalMap.contains(hashid) == true)
						{
							int index = mesaureSignalMap[hashid];
							if (index >= 0 && index < m_signalMeasureList.count())
							{
								int channel = param.location().rack().channel();
								if (channel >= 0 && channel < measureSignal.channelCount())
								{
									if (m_signalMeasureList[index].setMetrologySignal(measureKind, m_signalConnectionBase, signalConnectionType, channel, &m_signalList[i]) == false)
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
							if (measureSignal.setMetrologySignal(measureKind, m_signalConnectionBase, signalConnectionType, channel, &m_signalList[i]) == false)
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
				assert(false);
				continue;
			}

			m_signalMeasureList.append(measureSignal);

			signalIndex++;
		}

	m_signalMesaureMutex.unlock();

	return signalIndex;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::clearSignalListForMeasure()
{
	m_signalMesaureMutex.lock();

		m_signalMeasureList.clear();

	m_signalMesaureMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal SignalBase::activeSignal() const
{
	MeasureSignal signal;

	m_activeSignalMutex.lock();

		signal = m_activeSignal;

	m_activeSignalMutex.unlock();

	return signal;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setActiveSignal(const MeasureSignal& signal)
{
	m_activeSignalMutex.lock();

		m_activeSignal = signal;

		m_stateMutex.lock();

			m_requestStateList.clear();

			for(int channel = 0; channel < signal.channelCount(); channel++)
			{
				// append hash of input signal
				//
				Metrology::Signal* pSignal = m_activeSignal.multiChannelSignal(MEASURE_IO_SIGNAL_TYPE_INPUT).metrologySignal(channel);
				if (pSignal == nullptr || pSignal->param().isValid() == false)
				{
					continue;
				}

				m_requestStateList.append(pSignal->param().hash());

				// append hash of signal that contains ID of module for input signal - Serial Number
				//
				QString appSignalID_of_SerialNo = findAppSignalIDforSerialNo(pSignal->param().location().moduleID());
				pSignal->param().setModuleSerialNoID(appSignalID_of_SerialNo);
				if (pSignal->param().moduleSerialNoID().isEmpty() == false)
				{
					m_requestStateList.append(calcHash(pSignal->param().moduleSerialNoID()));
				}

				// append hash of comparators input signal
				//
				int comparatorCount = pSignal->param().comparatorCount();
				for (int c = 0; c < comparatorCount; c++)
				{
					std::shared_ptr<Comparator> comparator = pSignal->param().comparator(c);

					if (comparator->compare().isConst() == false && comparator->compare().appSignalID().isEmpty() == false)
					{
						m_requestStateList.append(calcHash(comparator->compare().appSignalID()));
					}

					if (comparator->hysteresis().isConst() == false && comparator->hysteresis().appSignalID().isEmpty() == false)
					{
						m_requestStateList.append(calcHash(comparator->hysteresis().appSignalID()));
					}

					if (comparator->output().isAcquired() == true)
					{
						m_requestStateList.append(calcHash(comparator->output().appSignalID()));
					}
				}

				// if input has not output signals got to next channel
				//
				if (m_activeSignal.signalConnectionType() == SIGNAL_CONNECTION_TYPE_UNUSED)
				{
					continue;
				}

				// append hash of output signal
				//
				pSignal = m_activeSignal.multiChannelSignal(MEASURE_IO_SIGNAL_TYPE_OUTPUT).metrologySignal(channel);
				if (pSignal == nullptr || pSignal->param().isValid() == false)
				{
					continue;
				}

				m_requestStateList.append(pSignal->param().hash());

				// append hash of signal that contains ID of module for output signal
				//
				appSignalID_of_SerialNo = findAppSignalIDforSerialNo(pSignal->param().location().moduleID());
				pSignal->param().setModuleSerialNoID(appSignalID_of_SerialNo);
				if (pSignal->param().moduleSerialNoID().isEmpty() == false)
				{
					m_requestStateList.append(calcHash(pSignal->param().moduleSerialNoID()));
				}

				// append hash of comparators output signal
				//
				comparatorCount = pSignal->param().comparatorCount();
				for (int c = 0; c < comparatorCount; c++)
				{
					std::shared_ptr<Comparator> comparator = pSignal->param().comparator(c);

					if (comparator->compare().isConst() == false && comparator->compare().appSignalID().isEmpty() == false)
					{
						m_requestStateList.append(calcHash(comparator->compare().appSignalID()));
					}

					if (comparator->hysteresis().isConst() == false && comparator->hysteresis().appSignalID().isEmpty() == false)
					{
						m_requestStateList.append(calcHash(comparator->hysteresis().appSignalID()));
					}

					if (comparator->output().isAcquired() == true)
					{
						m_requestStateList.append(calcHash(comparator->output().appSignalID()));
					}
				}

			}

		m_stateMutex.unlock();

	m_activeSignalMutex.unlock();

	emit activeSignalChanged(m_activeSignal);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::clearActiveSignal()
{
	m_activeSignalMutex.lock();

		m_activeSignal.clear();

		m_stateMutex.lock();

			m_requestStateList.clear();

		m_stateMutex.unlock();

	m_activeSignalMutex.unlock();

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

		pInputSignal->param().setComparatorList(comparatorSet.getByInputSignalID(appSignalID));
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
