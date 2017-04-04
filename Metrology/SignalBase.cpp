#include "SignalBase.h"

#include "CalibratorBase.h"
#include "MeasureBase.h"
#include "Database.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MetrologyMultiSignal::MetrologyMultiSignal()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

MetrologyMultiSignal::MetrologyMultiSignal(const MetrologyMultiSignal& from)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

bool MetrologyMultiSignal::isEmpty() const
{
	bool empty = true;

	m_mutex.lock();

		for(int c = 0; c < Metrology::ChannelCount; c++)
		{
			if (m_pSignal[c] != nullptr)
			{
				empty = false;

				break;
			}
		}

	 m_mutex.unlock();

	return empty;
}

// -------------------------------------------------------------------------------------------------------------------

void MetrologyMultiSignal::clear()
{
	m_mutex.lock();

		for(int c = 0; c < Metrology::ChannelCount; c++)
		{
			m_pSignal[c] = nullptr;
		}

		m_location.clear();

		m_strID.clear();

	m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::Signal* MetrologyMultiSignal::metrologySignal(int channel) const
{
	if (channel < 0 || channel >= Metrology::ChannelCount)
	{
		assert(0);
		return nullptr;
	}

	Metrology::Signal* pSignal = nullptr;

	m_mutex.lock();

		pSignal = m_pSignal[channel];

	m_mutex.unlock();

	return pSignal;
}

// -------------------------------------------------------------------------------------------------------------------

bool MetrologyMultiSignal::setMetrologySignal(int measureKind, int channel, Metrology::Signal* pSignal)
{
	if (channel < 0 || channel >= Metrology::ChannelCount)
	{
		assert(0);
		return false;
	}

	if (measureKind < 0 || measureKind >= MEASURE_KIND_COUNT)
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

		m_pSignal[channel] = pSignal;

		m_location.setRack(param.location().rack());
		m_location.setChassis(param.location().chassis());
		m_location.setModule(param.location().module());
		m_location.setPlace(param.location().place());
		m_location.setContact(param.location().contact());

		switch(measureKind)
		{
			case MEASURE_KIND_ONE:		m_strID = param.customAppSignalID();																						break;
			case MEASURE_KIND_MULTI:	m_strID.sprintf("CH %02d _ MD %02d _ IN %02d", m_location.chassis() + 1, m_location.module() + 1, m_location.place() + 1);	break;
			default:					assert(false);
		}

	m_mutex.unlock();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

MetrologyMultiSignal& MetrologyMultiSignal::operator=(const MetrologyMultiSignal& from)
{
	m_mutex.lock();

		for(int c = 0; c < Metrology::ChannelCount; c++)
		{
			m_pSignal[c] = from.m_pSignal[c];
		}

		m_location = from.m_location;
		m_strID = from.m_strID;

	m_mutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureMultiParam::MeasureMultiParam()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

MeasureMultiParam::MeasureMultiParam(const MeasureMultiParam& from)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureMultiParam::isValid() const
{
	bool valid = true;

	m_mutex.lock();

		if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
		{
			valid = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT].isValid();
		}
		else
		{
			for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
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

void MeasureMultiParam::clear()
{
	m_mutex.lock();

		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
		{
			m_param[type].setAppSignalID(QString());
		}

		m_outputSignalType = OUTPUT_SIGNAL_TYPE_UNUSED;

		m_pCalibratorManager = nullptr;

	m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::SignalParam MeasureMultiParam::param(int type) const
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

bool MeasureMultiParam::setParam(int type, const Metrology::SignalParam& param)
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

QString MeasureMultiParam::rackCaption() const
{
	QString result;

	m_mutex.lock();

		if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
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
				result = inParam.location().rack().caption() + MultiTextDivider;
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

QString MeasureMultiParam::signalID(bool showCustomID) const
{
	QString result;

	m_mutex.lock();

		if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (param.isValid() == true)
			{
				result = showCustomID == true ? param.customAppSignalID() : param.appSignalID();
			}
		}
		else
		{
			const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (inParam.isValid() == true)
			{
				result = (showCustomID == true ? inParam.customAppSignalID() : inParam.appSignalID()) + MultiTextDivider;
			}

			const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
			if (outParam.isValid() == true)
			{
				result += (showCustomID == true ? outParam.customAppSignalID() : outParam.appSignalID());
			}
		}

	m_mutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureMultiParam::equipmentID() const
{
	QString result;

	m_mutex.lock();

		if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
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
				result = inParam.location().equipmentID() + MultiTextDivider;
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

QString MeasureMultiParam::chassisStr() const
{
	QString result;

	m_mutex.lock();

		if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
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
				result = inParam.location().chassisStr() + MultiTextDivider;
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

QString MeasureMultiParam::moduleStr() const
{
	QString result;

	m_mutex.lock();

		if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
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
				result = inParam.location().moduleStr() + MultiTextDivider;
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

QString MeasureMultiParam::placeStr() const
{
	QString result;

	m_mutex.lock();

		if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
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
				result = inParam.location().placeStr() + MultiTextDivider;
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

QString MeasureMultiParam::caption() const
{
	QString result;

	m_mutex.lock();

		if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
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
				result = inParam.caption() + MultiTextDivider;
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

QString MeasureMultiParam::physicalRangeStr() const
{
	QString result;

	m_mutex.lock();

		if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (param.isValid() == true)
			{
				result = param.inputPhysicalRangeStr();
			}
		}
		else
		{
			const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (inParam.isValid() == true)
			{
				result = inParam.inputPhysicalRangeStr() + MultiTextDivider;
			}

			const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
			if (outParam.isValid() == true)
			{
				if (inParam.inputPhysicalRangeStr() != outParam.inputPhysicalRangeStr())
				{
					result += outParam.inputPhysicalRangeStr();
				}
				else
				{
					result = outParam.inputPhysicalRangeStr();
				}
			}
		}

	m_mutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureMultiParam::electricRangeStr() const
{
	QString result;

	m_mutex.lock();

		if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (param.isValid() == true)
			{
				result = param.inputElectricRangeStr();
			}
		}
		else
		{
			const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (inParam.isValid() == true)
			{
				result = inParam.inputElectricRangeStr() + MultiTextDivider;
			}

			const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
			if (outParam.isValid() == true)
			{
				if (inParam.inputElectricRangeStr() != outParam.inputElectricRangeStr())
				{
					result += outParam.inputElectricRangeStr();
				}
				else
				{
					result = outParam.inputElectricRangeStr();
				}
			}
		}

	m_mutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureMultiParam::electricSensorStr() const
{
	QString result;

	m_mutex.lock();

		if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (param.isValid() == true)
			{
				result = param.inputElectricSensor();
			}
		}
		else
		{
			const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (inParam.isValid() == true)
			{
				result = inParam.inputElectricSensor() + MultiTextDivider;
			}

			const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
			if (outParam.isValid() == true)
			{
				if (inParam.inputElectricSensor() != outParam.inputElectricSensor())
				{
					result += outParam.inputElectricSensor();
				}
				else
				{
					result = outParam.inputElectricSensor();
				}
			}
		}

	m_mutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureMultiParam::calibratorStr() const
{
	if (m_pCalibratorManager == nullptr || m_pCalibratorManager->calibratorIsConnected() == false)
	{
		return QString("Not connected");
	}

	return QString("Calibrator %1 (%2)").arg(m_pCalibratorManager->calibratorChannel() + 1).arg(m_pCalibratorManager->calibratorPort());
}


// -------------------------------------------------------------------------------------------------------------------

MeasureMultiParam& MeasureMultiParam::operator=(const MeasureMultiParam& from)
{
	m_mutex.lock();

		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
		{
			m_param[type] = from.m_param[type];
		}

		m_outputSignalType = from.m_outputSignalType;

		m_pCalibratorManager = from.m_pCalibratorManager;

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

		m_outputSignalType = OUTPUT_SIGNAL_TYPE_UNUSED;

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

MetrologyMultiSignal MeasureSignal::signal(int type) const
{
	if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return MetrologyMultiSignal();
	}

	MetrologyMultiSignal signal;

	m_mutex.lock();

		signal = m_signal[type];

	m_mutex.unlock();

	return signal;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureSignal::setSignal(int type, const MetrologyMultiSignal& signal)
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
		return 0;
	}

	if (channel < 0 || channel >= Metrology::ChannelCount)
	{
		return 0;
	}

	Metrology::Signal* pSignal = nullptr;

	m_mutex.lock();

		pSignal = m_signal[type].metrologySignal(channel);

	m_mutex.unlock();

	return pSignal;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureSignal::setMetrologySignal(int measureKind, int outputSignalType, int channel, Metrology::Signal* pSignal)
{
	if (measureKind < 0 || measureKind >= MEASURE_KIND_COUNT)
	{
		assert(0);
		return false;
	}

	if (outputSignalType < 0 || outputSignalType >= OUTPUT_SIGNAL_TYPE_COUNT)
	{
		assert(0);
		return false;
	}

	if (channel < 0 || channel >= Metrology::ChannelCount)
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

	switch (outputSignalType)
	{
		case OUTPUT_SIGNAL_TYPE_UNUSED:
			{
				m_mutex.lock();

					m_outputSignalType = OUTPUT_SIGNAL_TYPE_UNUSED;

					result = m_signal[MEASURE_IO_SIGNAL_TYPE_INPUT].setMetrologySignal(measureKind, channel, pSignal);

				m_mutex.unlock();
			}
			break;

		case OUTPUT_SIGNAL_TYPE_FROM_INPUT:
		case OUTPUT_SIGNAL_TYPE_FROM_TUNING:
			{
				// find index of output signal in the base by input signal
				//
				int index = theSignalBase.outputSignals().findIndex(outputSignalType, MEASURE_IO_SIGNAL_TYPE_INPUT, pSignal);
				if (index == -1)
				{
					result = false;
					break;
				}

				// take output signal in the base by index
				//
				OutputSignal outputSignal = theSignalBase.outputSignals().signal(index);
				if (outputSignal.isValid() == false)
				{
					result = false;
					break;
				}

				m_mutex.lock();

					m_outputSignalType = outputSignalType;

					for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type ++)
					{
						Metrology::Signal* pSignalFromOutputSignal = outputSignal.metrologySignal(type);
						if (pSignalFromOutputSignal == nullptr || pSignalFromOutputSignal->param().isValid() == false)
						{
							result = false;
							break;
						}

						if (m_signal[type].setMetrologySignal(measureKind, channel, pSignalFromOutputSignal) == false)
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

	for(int c = 0; c < Metrology::ChannelCount; c++)
	{
		for(int t = 0; t < MEASURE_IO_SIGNAL_TYPE_COUNT; t++)
		{
			if (m_signal[t].metrologySignal(c) == pSignal)
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

		m_outputSignalType = from.m_outputSignalType;

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
	m_tuningBase.clear();

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

		m_unitList.clear();
		m_outputSignalBase.empty();			// set all output signals vlue nullptr
		m_tuningBase.Signals().clear();		// remove all tuning signals

		m_signalHashMap.clear();
		m_signalList.clear();

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::appendSignal(const Metrology::SignalParam& param)
{
	if (param.appSignalID().isEmpty() == true || param.hash() == 0)
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
	if (hash == 0)
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
	if (hash == 0)
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
	if (hash == 0)
	{
		assert(hash != 0);
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
	if (hash == 0)
	{
		assert(hash != 0);
		return;
	}

	m_signalMutex.lock();

		if (m_signalHashMap.contains(hash) == true)
		{
			int index = m_signalHashMap[hash];

			if (index >= 0 && index < m_signalList.count())
			{
				m_signalList[index].setParam(param);

				emit updatedSignalParam(param.hash());
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

			emit updatedSignalParam(param.hash());
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
	if (hash == 0)
	{
		assert(hash != 0);
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
	if (hash == 0)
	{
		assert(hash != 0);
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
	Hash hash;

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

int SignalBase::createRackListForMeasure(int outputSignalType)
{
	if (outputSignalType < 0 || outputSignalType >= OUTPUT_SIGNAL_TYPE_COUNT)
	{
		assert(false);
		return 0;
	}

	QMap<Hash, int> rackHashMap;

	// find all type of racks for selected outputSignalType and create rackTypeList for ToolBar
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

			switch (outputSignalType)
			{
				case OUTPUT_SIGNAL_TYPE_UNUSED:
				case OUTPUT_SIGNAL_TYPE_FROM_INPUT:

					if (param.isInput() == false)
					{
						continue;
					}

					if (param.location().place() == -1)
					{
						continue;
					}

					break;

				case OUTPUT_SIGNAL_TYPE_FROM_TUNING:

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
			if (rackHash == 0)
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

	m_signalMutex.lock();

		int count = m_signalList.count();

		for(int i = 0; i < count; i ++)
		{
			Metrology::SignalParam& param = m_signalList[i].param();
			if (param.isValid() == false)
			{
				continue;
			}

			// units
			//
			param.setInputElectricUnit(m_unitList.value(param.inputElectricUnitID()));
			param.setInputPhysicalUnit(m_unitList.value(param.inputPhysicalUnitID()));
			param.setOutputElectricUnit(m_unitList.value(param.outputElectricUnitID()));
			param.setOutputPhysicalUnit(m_unitList.value(param.outputPhysicalUnitID()));

			// sensors
			//
			int sensorType = param.inputElectricSensorType();
			if (sensorType >= 0 && sensorType < SENSOR_TYPE_COUNT)
			{
				param.setInputElectricSensor(SensorTypeStr[ sensorType ]);
			}

			sensorType = param.outputElectricSensorType();
			if (sensorType >= 0 && sensorType < SENSOR_TYPE_COUNT)
			{
				param.setOutputElectricSensor(SensorTypeStr[ sensorType ]);
			}

			// places for tuning signals
			//
			if (param.enableTuning() == true)
			{
				switch (param.signalType())
				{
					case E::SignalType::Analog:		param.setPlace(analogTuningSignalCount++);		break;
					case E::SignalType::Discrete:	param.setPlace(discreteTuningSignalCount++);	break;
					default:						assert(0);
				}
			}
		}

	m_signalMutex.unlock();

	updateRackParam();

	m_outputSignalBase.init();

	m_tuningBase.Signals().createSignalList();
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

int SignalBase::createSignalListForMeasure(int measureKind, int outputSignalType, int rackIndex)
{
	if (measureKind < 0 || measureKind >= MEASURE_KIND_COUNT)
	{
		assert(false);
		return 0;
	}

	if (outputSignalType < 0 || outputSignalType >= OUTPUT_SIGNAL_TYPE_COUNT)
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
	MeasureSignal	measureSignal;
	QMap<Hash, int>	mesaureSignalMap;

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

			switch (outputSignalType)
			{
				case OUTPUT_SIGNAL_TYPE_UNUSED:

					if (param.isInput() == false)
					{
						continue;
					}

					if (param.location().place() == -1)
					{
						continue;
					}

					break;


				case OUTPUT_SIGNAL_TYPE_FROM_INPUT:

					if (param.isInput() == false)
					{
						continue;
					}

					if (param.location().place() == -1)
					{
						continue;
					}

					if (m_outputSignalBase.findIndex(outputSignalType, MEASURE_IO_SIGNAL_TYPE_INPUT, &m_signalList[i]) == -1)
					{
						continue;
					}

					break;

				case OUTPUT_SIGNAL_TYPE_FROM_TUNING:

					if (param.isInternal() == false)
					{
						continue;
					}

					if (m_outputSignalBase.findIndex(outputSignalType, MEASURE_IO_SIGNAL_TYPE_INPUT, &m_signalList[i]) == -1)
					{
						continue;
					}

					break;

				default:
					assert(0);
					continue;
			}

			switch(measureKind)
			{
				case MEASURE_KIND_ONE:
					{
						if (param.location().rack().index() != rackIndex)
						{
							continue;
						}

						if (measureSignal.setMetrologySignal(measureKind, outputSignalType, Metrology::Channel_0, &m_signalList[i]) == false)
						{
							continue;
						}
					}
					break;

				case MEASURE_KIND_MULTI:
					{
						if (param.location().rack().groupIndex() != rackIndex)
						{
							continue;
						}

						QString id;
						id.sprintf("%d - %d - %d - %d - ",
									param.location().rack().groupIndex(),
									param.location().chassis() + 1,
									param.location().module() + 1,
									param.location().place() + 1);
									id.append(param.location().contact());

						Hash hashid = calcHash(id);

						if (mesaureSignalMap.contains(hashid) == true)
						{
							int index = mesaureSignalMap[hashid];
							if (index >= 0 && index < m_signalMeasureList.count())
							{
								int channel = param.location().rack().channel();
								if (channel >= 0 && channel < Metrology::ChannelCount)
								{
									if (m_signalMeasureList[index].setMetrologySignal(measureKind, outputSignalType, channel, &m_signalList[i]) == false)
									{
										continue;
									}
								}
							}

							continue;
						}

						mesaureSignalMap.insert(hashid, signalIndex);

						int channel = param.location().rack().channel();
						if (channel >= 0 && channel < Metrology::ChannelCount)
						{
							if (measureSignal.setMetrologySignal(measureKind, outputSignalType, channel, &m_signalList[i]) == false)
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

			for(int channel = 0; channel < Metrology::ChannelCount; channel++)
			{
				Metrology::Signal* pSignal = m_activeSignal.signal(MEASURE_IO_SIGNAL_TYPE_INPUT).metrologySignal(channel);
				if (pSignal == nullptr || pSignal->param().isValid() == false)
				{
					continue;
				}

				m_requestStateList.append(pSignal->param().hash());

				if (m_activeSignal.outputSignalType() == OUTPUT_SIGNAL_TYPE_UNUSED)
				{
					continue;
				}

				pSignal = m_activeSignal.signal(MEASURE_IO_SIGNAL_TYPE_OUTPUT).metrologySignal(channel);
				if (pSignal == nullptr || pSignal->param().isValid() == false)
				{
					continue;
				}

				m_requestStateList.append(pSignal->param().hash());
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
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
