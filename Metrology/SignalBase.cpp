#include "SignalBase.h"

#include "Measure.h"
#include "CalibratorBase.h"
#include "MeasurementBase.h"
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
			if (m_signalHash[c] != 0)
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
			m_signalHash[c] = 0;
		}

		m_location.clear();

		m_strID = QString();

	m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

Hash MetrologyMultiSignal::hash(int channel) const
{
	if (channel < 0 || channel >= Metrology::ChannelCount)
	{
		assert(0);
		return 0;
	}

	Hash hash = 0;

	m_mutex.lock();

		hash = m_signalHash[channel];

	m_mutex.unlock();

	return hash;
}

// -------------------------------------------------------------------------------------------------------------------

bool MetrologyMultiSignal::setSignal(int channel, int measureKind, const Metrology::SignalParam& param)
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

	if (param.isValid() == false)
	{
		assert(false);
		return false;
	}

	m_mutex.lock();

		m_signalHash[channel] = param.hash();

		m_location.setRack(param.location().rack());
		m_location.setChassis(param.location().chassis());
		m_location.setModule(param.location().module());
		m_location.setPlace(param.location().place());

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
			m_signalHash[c] = from.m_signalHash[c];
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

void MeasureMultiParam::clear()
{
	m_mutex.lock();

		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
		{
			m_param[type].setAppSignalID(QString());
		}

		m_outputSignalType = OUTPUT_SIGNAL_TYPE_UNUSED;

		m_equalPhysicalRange = false;

		m_pCalibratorManager = nullptr;

	m_mutex.unlock();
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

		m_equalPhysicalRange = testPhysicalRange();

	m_mutex.unlock();

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool MeasureMultiParam::testPhysicalRange()
{
	if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
	{
		return true;
	}

	const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
	const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];

	if (inParam.isValid() == false || outParam.isValid() == false)
	{
		return false;
	}

	if (inParam.inputPhysicalLowLimit() != outParam.inputPhysicalLowLimit())
	{
		return false;
	}

	if (inParam.inputPhysicalHighLimit() != outParam.inputPhysicalHighLimit())
	{
		return false;
	}

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
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
			if (param.isValid() == true)
			{
				result = param.location().rack().caption();
			}
		}

	m_mutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureMultiParam::signalID(bool showCustomID, const QString& divider) const
{
	QString strResult;

	m_mutex.lock();

		if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (param.isValid() == true)
			{
				strResult = showCustomID == true ? param.customAppSignalID() : param.appSignalID();
			}
		}
		else
		{
			const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (inParam.isValid() == true)
			{
				strResult = (showCustomID == true ? inParam.customAppSignalID() : inParam.appSignalID()) + divider;
			}

			const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
			if (outParam.isValid() == true)
			{
				strResult += (showCustomID == true ? outParam.customAppSignalID() : outParam.appSignalID());
			}
		}

	m_mutex.unlock();

	return strResult;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureMultiParam::chassisStr() const
{
	QString strResult;

	m_mutex.lock();

		if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (param.isValid() == true)
			{
				strResult = param.location().chassisStr();
			}
		}
		else
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
			if (param.isValid() == true)
			{
				strResult = param.location().chassisStr();
			}
		}

	m_mutex.unlock();

	return strResult;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureMultiParam::moduleStr() const
{
	QString strResult;

	m_mutex.lock();

		if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (param.isValid() == true)
			{
				strResult = param.location().moduleStr();
			}
		}
		else
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
			if (param.isValid() == true)
			{
				strResult = param.location().moduleStr();
			}
		}

	m_mutex.unlock();

	return strResult;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureMultiParam::placeStr() const
{
	QString strResult;

	m_mutex.lock();

		if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (param.isValid() == true)
			{
				strResult = param.location().placeStr();
			}
		}
		else
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];
			if (param.isValid() == true)
			{
				strResult = param.location().placeStr();
			}
		}

	m_mutex.unlock();

	return strResult;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureMultiParam::caption(const QString& divider) const
{
	QString strResult;

	m_mutex.lock();

		if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (param.isValid() == true)
			{
				strResult = param.caption();
			}
		}
		else
		{
			const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];

			QString inCaptionStr, outCaptionStr;

			if (inParam.isValid() == true)
			{
				inCaptionStr = inParam.caption();
			}

			if (outParam.isValid() == true)
			{
				outCaptionStr = outParam.caption();
			}

			strResult = inCaptionStr + divider + outCaptionStr;
		}

	m_mutex.unlock();

	return strResult;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureMultiParam::physicalRangeStr(const QString& divider) const
{
	QString strResult;

	m_mutex.lock();

		if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (param.isValid() == true)
			{
				strResult = param.inputPhysicalRangeStr();
			}
		}
		else
		{
			const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];

			QString inRangeStr, outRangeStr;

			if (inParam.isValid() == true)
			{
				inRangeStr = inParam.inputPhysicalRangeStr();
			}

			if (outParam.isValid() == true)
			{
				outRangeStr = outParam.inputPhysicalRangeStr();
			}

			if (m_equalPhysicalRange == true)
			{
				strResult = outRangeStr;
			}
			else
			{
				strResult = inRangeStr + divider + outRangeStr;
			}
		}

	m_mutex.unlock();

	return strResult;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureMultiParam::electricRangeStr(const QString& divider) const
{
	QString strResult;

	m_mutex.lock();

		if (m_outputSignalType == OUTPUT_SIGNAL_TYPE_UNUSED)
		{
			const Metrology::SignalParam& param = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			if (param.isValid() == true)
			{
				strResult = param.inputElectricRangeStr();
			}
		}
		else
		{
			const Metrology::SignalParam& inParam = m_param[MEASURE_IO_SIGNAL_TYPE_INPUT];
			const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];

			QString inRangeStr, outRangeStr;

			if (inParam.isValid() == true && inParam.isInput() == true)
			{
				inRangeStr = inParam.inputElectricRangeStr();
			}

			if (outParam.isValid() == true && outParam.isOutput() == true)
			{
				outRangeStr = outParam.outputElectricRangeStr();
			}

			strResult = inRangeStr + divider + outRangeStr;
		}

	m_mutex.unlock();

	return strResult;
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureMultiParam::electricSensorStr(const QString& divider) const
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
			const Metrology::SignalParam& outParam = m_param[MEASURE_IO_SIGNAL_TYPE_OUTPUT];

			QString inSensor, outSensor;

			if (inParam.isValid() == true && inParam.isInput() == true)
			{
				inSensor = inParam.inputElectricSensor();
			}

			if (outParam.isValid() == true && outParam.isOutput() == true)
			{
				outSensor = outParam.outputElectricSensor();
			}

			result = inSensor + divider + outSensor;
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

		m_equalPhysicalRange = from.m_equalPhysicalRange;

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

Hash MeasureSignal::signalHash(int type, int channel) const
{
	if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return 0;
	}

	if (channel < 0 || channel >= Metrology::ChannelCount)
	{
		return 0;
	}

	Hash hash;

	m_mutex.lock();

		hash = m_signal[type].hash(channel);

	m_mutex.unlock();

	return hash;
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

bool MeasureSignal::setSignal(int channel, int measureKind, int outputSignalType, const Metrology::SignalParam& param)
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

	if (outputSignalType < 0 || outputSignalType >= OUTPUT_SIGNAL_TYPE_COUNT)
	{
		assert(0);
		return false;
	}

	if (param.isValid() == false)
	{
		assert(false);
		return false;
	}

	bool result = true;

	switch (outputSignalType)
	{
		case OUTPUT_SIGNAL_TYPE_UNUSED:
			{
				m_mutex.lock();

					m_outputSignalType = OUTPUT_SIGNAL_TYPE_UNUSED;

					result = m_signal[MEASURE_IO_SIGNAL_TYPE_INPUT].setSignal(channel, measureKind, param);

				m_mutex.unlock();
			}
			break;

		case OUTPUT_SIGNAL_TYPE_FROM_INPUT:
		case OUTPUT_SIGNAL_TYPE_FROM_TUNING:
			{
				int index = theSignalBase.outputSignals().find(MEASURE_IO_SIGNAL_TYPE_INPUT, param.hash(), outputSignalType);
				if (index == -1)
				{
					result = false;
					break;
				}

				OutputSignal outputSignal = theSignalBase.outputSignals().signal(index);
				if (outputSignal.isValid() == false)
				{
					result = false;
					break;
				}

				outputSignal.updateParam();

				m_mutex.lock();

					m_outputSignalType = outputSignalType;

					for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type ++)
					{
						Metrology::SignalParam paramFromOutputSignal = outputSignal.param(type);
						if (paramFromOutputSignal.isValid() == false)
						{
							result = false;
							break;
						}

						if (m_signal[type].setSignal(channel, measureKind, paramFromOutputSignal) == false)
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
	clearActiveSignal();

	clearMeasureSignalList();

	clearMeasureRackList();

	clearSignalList();
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::sortByPosition()
{
	return;

	QTime responseTime;
	responseTime.start();

	m_signalMutex.lock();

		int count = m_signalList.size();

		for(int i = 0; i < count - 1; i++)
		{
			for(int j = i+1; j < count; j++)
			{
				if (m_signalList[i].param().location().equipmentID() > m_signalList[j].param().location().equipmentID())
				{
					Metrology::Signal signal	= m_signalList[ i ];
					m_signalList[ i ]			= m_signalList[ j ];
					m_signalList[ j ]			= signal;
				}
			}
		}

	m_signalMutex.unlock();

	qDebug() << __FUNCTION__ << responseTime.elapsed() << " ms";
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::signalCount() const
{
	int count = 0;

	m_signalMutex.lock();

		count = m_signalList.size();

	m_signalMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::clearSignalList()
{
	m_signalMutex.lock();

		m_unitList.clear();

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
			index = m_signalList.size() - 1;

			m_signalHashMap.insert(param.hash(), index);
		}

	 m_signalMutex.unlock();

	 return index;
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

			if (index >= 0 && index < m_signalList.size())
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

		if (index >= 0 && index < m_signalList.size())
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

			if (index >= 0 && index < m_signalList.size())
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

		if (index >= 0 && index < m_signalList.size())
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

			if (index >= 0 && index < m_signalList.size())
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

		if (index >= 0 && index < m_signalList.size())
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

			if (index >= 0 && index < m_signalList.size())
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

		if (index >= 0 && index < m_signalList.size())
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

			if (index >= 0 && index < m_signalList.size())
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

		if (index >= 0 && index < m_signalList.size())
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

		count = m_requestStateList.size();

	m_stateMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

Hash SignalBase::hashForRequestState(int index)
{
	Hash hash;

	m_stateMutex.lock();

		if (index >= 0 && index < m_requestStateList.size())
		{
			hash = m_requestStateList[index];
		}

	m_stateMutex.unlock();

	return hash;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::measureRackCount() const
{
	int count = 0;

	m_rackMutex.lock();

		count = m_rackList.size();

	m_rackMutex.unlock();

	return count;
}


// -------------------------------------------------------------------------------------------------------------------

Metrology::RackParam SignalBase::measureRack(int index)
{
	Metrology::RackParam param;

	m_rackMutex.lock();

		if (index >= 0 && index < m_rackList.size())
		{
			param = m_rackList[index];
		}

	m_rackMutex.unlock();

	return param;
}

// -------------------------------------------------------------------------------------------------------------------

int SignalBase::createMeasureRackList(int outputSignalType)
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

		int signalCount = m_signalList.size();

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
		int rackCount = m_rackList.size();

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

void SignalBase::clearMeasureRackList()
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

int SignalBase::measureSignalCount() const
{
	int count;

	m_signalMesaureMutex.lock();

		count = m_signalMesaureList.size();

	m_signalMesaureMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

MeasureSignal SignalBase::measureSignal(int index)
{
	MeasureSignal signal;

	m_signalMesaureMutex.lock();

		if (index >= 0 && index < m_signalMesaureList.size())
		{
			signal = m_signalMesaureList[index];
		}

	m_signalMesaureMutex.unlock();

	return signal;
}


// -------------------------------------------------------------------------------------------------------------------

int SignalBase::createMeasureSignalList(int measureKind, int outputSignalType, int rackIndex)
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

		m_signalMesaureList.clear();

		int signalCount = m_signalList.size();

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

			measureSignal.clear();

			switch(measureKind)
			{
				case MEASURE_KIND_ONE:
					{
						if (param.location().rack().index() != rackIndex)
						{
							continue;
						}

						measureSignal.setSignal(Metrology::Channel_0, measureKind, outputSignalType, param);
					}
					break;

				case MEASURE_KIND_MULTI:
					{
						if (param.location().rack().groupIndex() != rackIndex)
						{
							continue;
						}

						QString id;
						id.sprintf("%d - %d - %d - %d",
									param.location().rack().groupIndex(),
									param.location().chassis() + 1,
									param.location().module() + 1,
									param.location().place() + 1);

						Hash hashid = calcHash(id);

						if (mesaureSignalMap.contains(hashid) == true)
						{
							int index = mesaureSignalMap[hashid];
							if (index >= 0 && index < m_signalMesaureList.size())
							{
								int channel = param.location().rack().channel();
								if (channel >= 0 && channel < Metrology::ChannelCount)
								{
									m_signalMesaureList[index].setSignal(channel, measureKind, outputSignalType, param);
								}
							}

							continue;
						}

						mesaureSignalMap.insert(hashid, signalIndex);

						int channel = param.location().rack().channel();
						if (channel >= 0 && channel < Metrology::ChannelCount)
						{
							measureSignal.setSignal(channel, measureKind, outputSignalType, param);
						}
					}
					break;

				default:
					assert(false);
					continue;
			}

			if (measureSignal.isEmpty() == true)
			{
				// assert(false);
				continue;
			}

			m_signalMesaureList.append(measureSignal);

			signalIndex++;
		}

	m_signalMesaureMutex.unlock();

	return signalIndex;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::clearMeasureSignalList()
{
	m_signalMesaureMutex.lock();

		m_signalMesaureList.clear();

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

			for(int i = 0; i < Metrology::ChannelCount; i++)
			{
				Hash hash = m_activeSignal.signal(MEASURE_IO_SIGNAL_TYPE_INPUT).hash(i);
				if (hash == 0)
				{
					continue;
				}

				m_requestStateList.append(hash);

				if (m_activeSignal.outputSignalType() == OUTPUT_SIGNAL_TYPE_UNUSED)
				{
					continue;
				}

				hash = m_activeSignal.signal(MEASURE_IO_SIGNAL_TYPE_OUTPUT).hash(i);
				if (hash == 0)
				{
					continue;
				}

				m_requestStateList.append(hash);
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
