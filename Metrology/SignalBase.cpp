#include "SignalBase.h"

#include "Measure.h"
#include "CalibratorBase.h"
#include "MeasurementBase.h"
#include "Database.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

StatisticItem::StatisticItem(const Hash& signalHash):
	m_signalHash (signalHash)
{
}

// -------------------------------------------------------------------------------------------------------------------

QString StatisticItem::measureCountStr() const
{
	if (m_measureCount == 0)
	{
		return QString();
	}

	return QString::number(m_measureCount);
}

// -------------------------------------------------------------------------------------------------------------------

QString StatisticItem::stateStr() const
{
	if (m_measureCount == 0)
	{
		return QString("Not measured");
	}

	if (m_state < 0 || m_state >= STATISTIC_STATE_COUNT)
	{
		return QString();
	}

	return StatisticStateStr[m_state];
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MetrologySignal::MetrologySignal(const Metrology::SignalParam& param)
{
	setParam(param);

	// temporary solution
	// because u7 can not set electric range
	//
	m_param.setInputElectricLowLimit(0);
	m_param.setInputElectricHighLimit(5);
	m_param.setInputElectricUnitID(E::InputUnit::V);
	//
	// temporary solution
}

// -------------------------------------------------------------------------------------------------------------------

MetrologySignal& MetrologySignal::operator=(const MetrologySignal& from)
{
	m_param = from.m_param;
	m_state = from.m_state;

	return *this;
}

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

void MetrologyMultiSignal::clear()
{
	m_mutex.lock();

		for(int c = 0; c < MAX_CHANNEL_COUNT; c++)
		{
			m_signalHash[c] = 0;
		}

		m_location.clear();

		m_strID = QString();

	m_mutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

bool MetrologyMultiSignal::isEmpty() const
{
	bool empty = true;

	m_mutex.lock();

		for(int c = 0; c < MAX_CHANNEL_COUNT; c++)
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

Hash MetrologyMultiSignal::hash(int channel) const
{
	if (channel < 0 || channel >= MAX_CHANNEL_COUNT)
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
	if (channel < 0 || channel >= MAX_CHANNEL_COUNT)
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

		for(int c = 0; c < MAX_CHANNEL_COUNT; c++)
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

MeasureParam::MeasureParam()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

MeasureParam::MeasureParam(const MeasureParam& from)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureParam::clear()
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

bool MeasureParam::isValid() const
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

Metrology::SignalParam MeasureParam::param(int type) const
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

bool MeasureParam::setParam(int type, const Metrology::SignalParam& param)
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

bool MeasureParam::testPhysicalRange()
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

QString MeasureParam::rackCaption() const
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

QString MeasureParam::signalID(bool showCustomID, const QString& divider) const
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

QString MeasureParam::chassisStr() const
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

QString MeasureParam::moduleStr() const
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

QString MeasureParam::placeStr() const
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

QString MeasureParam::caption(const QString& divider) const
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

QString MeasureParam::physicalRangeStr(const QString& divider) const
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

QString MeasureParam::electricRangeStr(const QString& divider) const
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

QString MeasureParam::electricSensorStr(const QString& divider) const
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

QString MeasureParam::calibratorStr() const
{
	if (m_pCalibratorManager == nullptr || m_pCalibratorManager->calibratorIsConnected() == false)
	{
		return QString("Not connected");
	}

	return QString("Calibrator %1 (%2)").arg(m_pCalibratorManager->calibratorChannel() + 1).arg(m_pCalibratorManager->calibratorPort());
}


// -------------------------------------------------------------------------------------------------------------------

MeasureParam& MeasureParam::operator=(const MeasureParam& from)
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

	if (channel < 0 || channel >= MAX_CHANNEL_COUNT)
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
	if (channel < 0 || channel >= MAX_CHANNEL_COUNT)
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
				int index = theOutputSignalBase.find(MEASURE_IO_SIGNAL_TYPE_INPUT, param.hash(), outputSignalType);
				if (index == -1)
				{
					result = false;
					break;
				}

				OutputSignal outputSignal = theOutputSignalBase.signal(index);
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

	clearRackList();

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
					MetrologySignal signal	= m_signalList[ i ];
					m_signalList[ i ]		= m_signalList[ j ];
					m_signalList[ j ]		= signal;
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
			MetrologySignal metrologySignal(param);

			m_signalList.append(metrologySignal);
			index = m_signalList.size() - 1;

			m_signalHashMap.insert(param.hash(), index);
		}

	 m_signalMutex.unlock();

	 return index;
}

// -------------------------------------------------------------------------------------------------------------------

MetrologySignal SignalBase::signal(const QString& appSignalID)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(false);
		return MetrologySignal();
	}

	return signal(calcHash(appSignalID));
}

// -------------------------------------------------------------------------------------------------------------------

MetrologySignal SignalBase::signal(const Hash& hash)
{
	if (hash == 0)
	{
		assert(hash != 0);
		return MetrologySignal();
	}

	MetrologySignal signal;

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

MetrologySignal SignalBase::signal(int index)
{
	MetrologySignal signal;

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

AppSignalState SignalBase::signalState(const QString& appSignalID)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(false);
		return AppSignalState();
	}

	return signalState(calcHash(appSignalID));
}

// -------------------------------------------------------------------------------------------------------------------

AppSignalState SignalBase::signalState(const Hash& hash)
{
	if (hash == 0)
	{
		assert(hash != 0);
		return AppSignalState();
	}

	AppSignalState state;

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

AppSignalState SignalBase::signalState(int index)
{
	AppSignalState state;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.size())
		{
			state = m_signalList[index].state();
		}

	m_signalMutex.unlock();

	return state;
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignalState(const QString& appSignalID, const AppSignalState &state)
{
	if (appSignalID.isEmpty() == true)
	{
		assert(false);
		return;
	}

	setSignalState(calcHash(appSignalID), state);
}

// -------------------------------------------------------------------------------------------------------------------

void SignalBase::setSignalState(const Hash& hash, const AppSignalState &state)
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

void SignalBase::setSignalState(int index, const AppSignalState &state)
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

int SignalBase::rackCount() const
{
	int count = 0;

	m_rackMutex.lock();

		count = m_rackList.size();

	m_rackMutex.unlock();

	return count;
}


// -------------------------------------------------------------------------------------------------------------------

Metrology::RackParam SignalBase::rack(int index)
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

int SignalBase::createRackList(int outputSignalType)
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

void SignalBase::clearRackList()
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

			// fistly we are now only rackID, so we must set RackParam from RackBase for every signal
			//
			Metrology::RackParam rack = theRackBase.rack(param.location().rack().hash());
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

						measureSignal.setSignal(CHANNEL_0, measureKind, outputSignalType, param);
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
								if (channel >= 0 && channel < MAX_CHANNEL_COUNT)
								{
									m_signalMesaureList[index].setSignal(channel, measureKind, outputSignalType, param);
								}
							}

							continue;
						}

						mesaureSignalMap.insert(hashid, signalIndex);

						int channel = param.location().rack().channel();
						if (channel >= 0 && channel < MAX_CHANNEL_COUNT)
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

			for(int i = 0; i < MAX_CHANNEL_COUNT; i++)
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

RackGroup::RackGroup(const QString& caption)
{
	setCaption(caption);
}

// -------------------------------------------------------------------------------------------------------------------

bool RackGroup::isValid() const
{
	if (m_caption.isEmpty() == true || m_hash == 0)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

void RackGroup::clear()
{
	m_hash = 0;
	m_caption.clear();
}

// -------------------------------------------------------------------------------------------------------------------

void RackGroup::setCaption(const QString& caption)
{
	m_caption = caption;

	if (m_caption.isEmpty() == true)
	{
		m_hash = 0;
		return;
	}

	m_hash = calcHash(m_caption);
}

// -------------------------------------------------------------------------------------------------------------------

QString RackGroup::rackID(int channel) const
{
	if (channel < 0 || channel >= MAX_CHANNEL_COUNT)
	{
		return QString();
	}

	return m_rackID[channel];
}

// -------------------------------------------------------------------------------------------------------------------

void RackGroup::setRackID(int channel, const QString& rackID)
{
	if (channel < 0 || channel >= MAX_CHANNEL_COUNT)
	{
		return;
	}

	m_rackID[channel] = rackID;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

RackGroupBase::RackGroupBase(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

void RackGroupBase::clear()
{
	m_groupMutex.lock();

		m_groupList.clear();

	m_groupMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int RackGroupBase::count() const
{
	int count = 0;

	m_groupMutex.lock();

		count = m_groupList.size();

	m_groupMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

int RackGroupBase::append(const RackGroup& group)
{
	if (group.isValid() == false)
	{
		assert(false);
		return -1;
	}

	int index = -1;

	m_groupMutex.lock();

		m_groupList.append(group);
		index = m_groupList.size() - 1;

	m_groupMutex.unlock();

	return index;
}

// -------------------------------------------------------------------------------------------------------------------

RackGroup RackGroupBase::group(int index) const
{
	RackGroup group;

	m_groupMutex.lock();

		if (index >= 0 && index < m_groupList.size())
		{
			group = m_groupList[index];
		}

	m_groupMutex.unlock();

	return group;
}

// -------------------------------------------------------------------------------------------------------------------

bool RackGroupBase::setGroup(int index, const RackGroup& group)
{
	bool result = false;

	m_groupMutex.lock();

		if (index >= 0 && index < m_groupList.size())
		{
			m_groupList[index] = group;

			result = true;
		}

	m_groupMutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

bool RackGroupBase::remove(int index)
{
	bool result = false;

	m_groupMutex.lock();

		if (index >= 0 && index < m_groupList.size())
		{
			m_groupList.remove(index);

			result = true;
		}

	m_groupMutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

int RackGroupBase::load()
{
	if (thePtrDB == nullptr)
	{
		return 0;
	}

	QTime responseTime;
	responseTime.start();

	SqlTable* table = thePtrDB->openTable(SQL_TABLE_RACK_GROUP);
	if (table == nullptr)
	{
		return false;
	}

	int readedRecordCount = 0;

	m_groupMutex.lock();

		m_groupList.resize(table->recordCount());

		readedRecordCount = table->read(m_groupList.data());

	m_groupMutex.unlock();

	table->close();

	qDebug() << "RackBase::loadGroup() - Loaded rack groups: " << readedRecordCount << ", Time for load: " << responseTime.elapsed() << " ms";

	return readedRecordCount;
}

// -------------------------------------------------------------------------------------------------------------------

bool RackGroupBase::save()
{
	if (thePtrDB == nullptr)
	{
		return false;
	}

	SqlTable* table = thePtrDB->openTable(SQL_TABLE_RACK_GROUP);
	if (table == nullptr)
	{
		return false;
	}

	if (table->clear() == false)
	{
		table->close();
		return false;
	}

	int writtenRecordCount = 0;

	m_groupMutex.lock();

		writtenRecordCount = table->write(m_groupList.data(), m_groupList.count());

	m_groupMutex.unlock();

	table->close();

	if (writtenRecordCount != count())
	{
		return false;
	}

	qDebug() << "RackBase::saveGroup() - Written rack groups: " << writtenRecordCount;

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

RackGroupBase& RackGroupBase::operator=(const RackGroupBase& from)
{
	m_groupMutex.lock();

		m_groupList = from.m_groupList;

	m_groupMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

RackBase theRackBase;

// -------------------------------------------------------------------------------------------------------------------

RackBase::RackBase(QObject *parent) :
	QObject(parent)
{
}

 // -------------------------------------------------------------------------------------------------------------------

void RackBase::clear()
{
	m_rackMutex.lock();

		m_rackHashMap.clear();
		m_rackList.clear();

	m_rackMutex.unlock();
}


// -------------------------------------------------------------------------------------------------------------------

int RackBase::count() const
{
	int count = 0;

	m_rackMutex.lock();

		count = m_rackList.size();

	m_rackMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

int RackBase::append(const Metrology::RackParam& rack)
{
	if (rack.isValid() == false)
	{
		assert(false);
		return -1;
	}

	int index = -1;

	m_rackMutex.lock();

		if (m_rackHashMap.contains(rack.hash()) == false)
		{
			m_rackList.append(rack);
			index = m_rackList.size() - 1;

			m_rackHashMap.insert(rack.hash(), index);
		}

	 m_rackMutex.unlock();

	 return index;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::RackParam RackBase::rack(const QString& rackID)
{
	if (rackID.isEmpty() == true)
	{
		assert(false);
		return Metrology::RackParam();
	}

	return rack(calcHash(rackID));
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::RackParam RackBase::rack(const Hash& hash)
{
	if (hash == 0)
	{
		assert(hash != 0);
		return Metrology::RackParam();
	}

	Metrology::RackParam rack;

	m_rackMutex.lock();

		if (m_rackHashMap.contains(hash) == true)
		{
			int index = m_rackHashMap[hash];

			if (index >= 0 && index < m_rackList.size())
			{
				rack = m_rackList[index];
			}
		}

	m_rackMutex.unlock();

	return rack;
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::RackParam RackBase::rack(int index)
{
	Metrology::RackParam rack;

	m_rackMutex.lock();

		if (index >= 0 && index < m_rackList.size())
		{
			rack = m_rackList[index];
		}

	m_rackMutex.unlock();

	return rack;
}

// -------------------------------------------------------------------------------------------------------------------

void RackBase::setRack(const QString& rackID, const Metrology::RackParam& rack)
{
	if (rackID.isEmpty() == true)
	{
		assert(false);
		return;
	}

	if (rack.isValid() == false)
	{
		return;
	}

	setRack(calcHash(rackID), rack);
}

// -------------------------------------------------------------------------------------------------------------------

void RackBase::setRack(const Hash& hash, const Metrology::RackParam& rack)
{
	if (hash == 0)
	{
		assert(hash != 0);
		return;
	}

	if (rack.isValid() == false)
	{
		return;
	}

	m_rackMutex.lock();

		if (m_rackHashMap.contains(rack.hash()) == true)
		{
			int index = m_rackHashMap[rack.hash()];

			if (index >= 0 && index < m_rackList.size())
			{
				m_rackList[index] = rack;
			}
		}

	m_rackMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void RackBase::setRack(int index, const Metrology::RackParam& rack)
{
	if (rack.isValid() == false)
	{
		return;
	}

	m_rackMutex.lock();

		if (index >= 0 && index < m_rackList.size())
		{
			m_rackList[index] = rack;
		}

	m_rackMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void RackBase::updateParamFromGroups()
{
	int count = groups().count();
	for(int i = 0; i < count; i++)
	{
		RackGroup group = groups().group(i);
		if (group.isValid() == false)
		{
			continue;
		}

		for(int channel = 0; channel < MAX_CHANNEL_COUNT; channel++)
		{
			QString rackID = group.rackID(channel);
			if (rackID.isEmpty() == true)
			{
				continue;
			}

			Metrology::RackParam r = rack(rackID);
			if (r.isValid() == false)
			{
				continue;
			}

			r.setGroupIndex(group.Index());
			r.setChannel(channel);

			setRack(r.hash(), r);
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

RackBase& RackBase::operator=(const RackBase& from)
{
	m_rackMutex.lock();

		m_rackHashMap = from.m_rackHashMap;
		m_rackList = from.m_rackList;

		m_groupBase = from.m_groupBase;

	m_rackMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

OutputSignal::OutputSignal()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignal::OutputSignal(const OutputSignal& from)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignal::clear()
{
	m_hash = 0;

	m_type = OUTPUT_SIGNAL_TYPE_UNUSED;

	for(int k = 0; k < MEASURE_IO_SIGNAL_TYPE_COUNT; k++)
	{
		m_param[k].setAppSignalID(QString());
	}
}

// -------------------------------------------------------------------------------------------------------------------

bool OutputSignal::isValid() const
{
	if (m_hash == 0)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

bool OutputSignal::setHash()
{
	QString strID;

	for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
	{
		strID.append(m_appSignalID[type]);
	}

	if (strID.isEmpty() == true)
	{
		return false;
	}

	m_hash = calcHash(strID);
	if (m_hash == 0)
	{
		return false;
	}

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

QString OutputSignal::typeStr() const
{
	if (m_type < 0 || m_type >= OUTPUT_SIGNAL_TYPE_COUNT)
	{
		return QString();
	}

	return OutputSignalType[m_type];
}

// -------------------------------------------------------------------------------------------------------------------

QString OutputSignal::appSignalID(int type) const
{
	if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return QString();
	}

	QString appSignalID;

	m_signalMutex.lock();

		appSignalID = m_appSignalID[type];

	m_signalMutex.unlock();

	return appSignalID;
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignal::setAppSignalID(int type, const QString& appSignalID)
{
	if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return;
	}

	m_signalMutex.lock();

		m_appSignalID[type] = appSignalID;

		setHash();

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

Metrology::SignalParam OutputSignal::param(int type) const
{
	if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return Metrology::SignalParam();
	}

	Metrology::SignalParam param;

	m_signalMutex.lock();

		param = m_param[type];

	m_signalMutex.unlock();

	return param;
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignal::setParam(int type, const Metrology::SignalParam& param)
{
	if (type < 0 || type >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		return;
	}

	if (param.isValid() == false)\
	{
		return;
	}

	m_signalMutex.lock();

		m_appSignalID[type] = param.appSignalID();

		setHash();

		m_param[type] = param;

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignal::updateParam()
{
	m_signalMutex.lock();

		for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
		{
			if (m_appSignalID[type].isEmpty() == true)
			{
				continue;
			}

			Hash signalHash = calcHash(m_appSignalID[type]);
			if (signalHash == 0)
			{
				continue;
			}

			m_param[type] = theSignalBase.signalParam(signalHash);
		}

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignal& OutputSignal::operator=(const OutputSignal& from)
{
	m_signalID = from.m_signalID;
	m_hash = from.m_signalID;

	m_type = from.m_type;

	for(int type = 0; type < MEASURE_IO_SIGNAL_TYPE_COUNT; type++)
	{
		m_appSignalID[type] = from.m_appSignalID[type];

		m_param[type] = from.m_param[type];
	}

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

OutputSignalBase theOutputSignalBase;

// -------------------------------------------------------------------------------------------------------------------

OutputSignalBase::OutputSignalBase(QObject *parent) :
	QObject(parent)
{
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalBase::clear()
{
	m_signalMutex.lock();

		m_signalList.clear();

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int OutputSignalBase::signalCount() const
{
	int count = 0;

	m_signalMutex.lock();

		count = m_signalList.size();

	m_signalMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalBase::sort()
{

}

// -------------------------------------------------------------------------------------------------------------------

int OutputSignalBase::load()
{
	if (thePtrDB == nullptr)
	{
		return 0;
	}

	QTime responseTime;
	responseTime.start();

	SqlTable* table = thePtrDB->openTable(SQL_TABLE_OUTPUT_SIGNAL);
	if (table == nullptr)
	{
		return false;
	}

	int readedRecordCount = 0;

	m_signalMutex.lock();

		m_signalList.resize(table->recordCount());

		readedRecordCount = table->read(m_signalList.data());

	m_signalMutex.unlock();

	table->close();

	qDebug() << "OutputSignalBase::load() - Loaded output signals: " << readedRecordCount << ", Time for load: " << responseTime.elapsed() << " ms";

	return readedRecordCount;
}

// -------------------------------------------------------------------------------------------------------------------

bool OutputSignalBase::save()
{
	if (thePtrDB == nullptr)
	{
		return false;
	}

	SqlTable* table = thePtrDB->openTable(SQL_TABLE_OUTPUT_SIGNAL);
	if (table == nullptr)
	{
		return false;
	}

	if (table->clear() == false)
	{
		table->close();
		return false;
	}

	int writtenRecordCount = 0;

	m_signalMutex.lock();

		writtenRecordCount = table->write(m_signalList.data(), m_signalList.count());

	m_signalMutex.unlock();

	table->close();

	if (writtenRecordCount != signalCount())
	{
		return false;
	}

	qDebug() << "OutputSignalBase::save() - Written output signals: " << writtenRecordCount;

	return true;
}

// -------------------------------------------------------------------------------------------------------------------

int OutputSignalBase::appendSignal(const OutputSignal& signal)
{
	int index = -1;

	m_signalMutex.lock();

			m_signalList.append(signal);
			index = m_signalList.size() - 1;

	m_signalMutex.unlock();

	return index;
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignal OutputSignalBase::signal(int index) const
{
	OutputSignal signal;

	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.size())
		{
			signal = m_signalList[index];
		}

	m_signalMutex.unlock();

	return signal;
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalBase::setSignal(int index, const OutputSignal& signal)
{
	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.size())
		{
			m_signalList[index] = signal;
		}

	m_signalMutex.unlock();
}


// -------------------------------------------------------------------------------------------------------------------

void OutputSignalBase::remove(const OutputSignal& signal)
{
	int index = find(signal);

	return remove(index);
}

// -------------------------------------------------------------------------------------------------------------------

void OutputSignalBase::remove(int index)
{
	m_signalMutex.lock();

		if (index >= 0 && index < m_signalList.size())
		{
			m_signalList.remove(index);
		}

	m_signalMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int OutputSignalBase::find(int measureIoType, const Hash& hash, int outputSignalType)
{
	if (measureIoType < 0 || measureIoType >= MEASURE_IO_SIGNAL_TYPE_COUNT)
	{
		assert(0);
		return -1;
	}

	if (hash == 0)
	{
		assert(hash != 0);
		return -1;
	}

	if (outputSignalType < 0 || outputSignalType >= OUTPUT_SIGNAL_TYPE_COUNT)
	{
		assert(0);
		return -1;
	}

	int foundIndex = -1;

	m_signalMutex.lock();

		int count = m_signalList.size();

		for(int i = 0; i < count; i ++)
		{
			const OutputSignal& signal = m_signalList[i];

			if (calcHash(m_signalList[i].appSignalID(measureIoType)) != hash)
			{
				continue;
			}

			if (signal.type() != outputSignalType)
			{
				continue;
			}

			foundIndex = i;
			break;
		}

	m_signalMutex.unlock();

	return foundIndex;

}

// -------------------------------------------------------------------------------------------------------------------

int OutputSignalBase::find(const OutputSignal& signal)
{
	int foundIndex = -1;

		m_signalMutex.lock();

		int count = m_signalList.size();

		for(int i = 0; i < count; i ++)
		{
			if (m_signalList[i].hash() == signal.hash())
			{
				foundIndex = i;

				break;
			}
		 }

	m_signalMutex.unlock();

	return foundIndex;
}

// -------------------------------------------------------------------------------------------------------------------

OutputSignalBase& OutputSignalBase::operator=(const OutputSignalBase& from)
{
	m_signalMutex.lock();

		m_signalList = from.m_signalList;

	m_signalMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
