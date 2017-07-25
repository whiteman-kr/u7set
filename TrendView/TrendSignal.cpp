#include "TrendSignal.h"

namespace TrendLib
{

	TrendSignalParam::TrendSignalParam()
	{
	}

	TrendSignalParam::TrendSignalParam(const AppSignalParam& appSignal) :
		m_signalId(appSignal.customSignalId()),
		m_appSignalId(appSignal.appSignalId()),
		m_caption(appSignal.caption()),
		m_equipmentId(appSignal.equipmentId()),
		m_type(appSignal.type()),
		m_lowLimit(appSignal.lowEngineeringUnits()),
		m_highLimit(appSignal.highEngineeringUnits()),
		m_unit(appSignal.unit())
	{
	}

	QString TrendSignalParam::signalId() const
	{
		return m_signalId;
	}

	void TrendSignalParam::setSignalId(const QString& value)
	{
		m_signalId = value;
	}

	QString TrendSignalParam::appSignalId() const
	{
		return m_appSignalId;
	}

	void TrendSignalParam::setAppSignalId(const QString& value)
	{
		m_appSignalId = value;
	}

	QString TrendSignalParam::caption() const
	{
		return m_caption;
	}

	void TrendSignalParam::setCaption(const QString& value)
	{
		m_caption = value;
	}

	QString TrendSignalParam::equipmnetId() const
	{
		return m_equipmentId;
	}

	void TrendSignalParam::setEquipmnetId(const QString& value)
	{
		m_equipmentId = value;
	}

	bool TrendSignalParam::isAnalog() const
	{
		return m_type == E::SignalType::Analog;
	}

	bool TrendSignalParam::isDiscrete() const
	{
		return m_type == E::SignalType::Discrete;
	}

	E::SignalType TrendSignalParam::type() const
	{
		return m_type;
	}

	void TrendSignalParam::setType(E::SignalType value)
	{
		m_type = value;
	}

	double TrendSignalParam::lowLimit() const
	{
		return m_lowLimit;
	}

	void TrendSignalParam::setLowLimit(double value)
	{
		m_lowLimit = value;
	}

	double TrendSignalParam::highLimit() const
	{
		return m_highLimit;
	}
	void TrendSignalParam::setHighLimit(double value)
	{
		m_highLimit = value;
	}

	QString TrendSignalParam::unit() const
	{
		return m_unit;
	}

	void TrendSignalParam::setUnit(const QString& value)
	{
		m_unit = value;
	}

	QColor TrendSignalParam::color() const
	{
		return m_color;
	}

	void TrendSignalParam::setColor(const QColor& value)
	{
		m_color = value;
	}

	TrendSignalSet::TrendSignalSet()
	{
	}

	bool TrendSignalSet::addSignal(const TrendSignalParam& signal)
	{
		QMutexLocker locker(&m_paramMutex);

		auto foundIt = std::find_if(m_signalParams.begin(), m_signalParams.end(),
			[&signal](const TrendSignalParam& s)
			{
				return s.signalId() == signal.signalId();
			});

		if (foundIt != m_signalParams.end())
		{
			return false;
		}

		m_signalParams.push_back(signal);

		return true;
	}

	void TrendSignalSet::removeSignal(QString appSignalId)
	{
		QMutexLocker locker(&m_paramMutex);

		m_signalParams.remove_if(
			[&appSignalId](const TrendSignalParam& s)
			{
				return s.appSignalId() == appSignalId;
			});

		return;
	}

	std::vector<TrendSignalParam> TrendSignalSet::analogSignals() const
	{
		QMutexLocker locker(&m_paramMutex);

		std::vector<TrendSignalParam> result;
		result.reserve(m_signalParams.size());

		for (const TrendSignalParam& s : m_signalParams)
		{
			if (s.isAnalog() == true)
			{
				result.push_back(s);
			}
		}

		return result;
	}

	std::vector<TrendSignalParam> TrendSignalSet::discreteSignals() const
	{
		QMutexLocker locker(&m_paramMutex);

		std::vector<TrendSignalParam> result;
		result.reserve(m_signalParams.size());

		for (const TrendSignalParam& s : m_signalParams)
		{
			if (s.isDiscrete() == true)
			{
				result.push_back(s);
			}
		}

		return result;
	}

	bool TrendSignalSet::getTrendData(QString appSignalId, QDateTime from, QDateTime to, std::list<OneHourData>* outData)
	{
		if (outData == nullptr ||
			from > to)
		{
			assert(outData);
			assert(from <= to);
			return false;
		}

		// Find Signal
		//
		{
			QMutexLocker locker(&m_archiveMutex);

			auto archiveIt = m_archive.find(appSignalId);
			if (archiveIt == m_archive.end())
			{
				auto emplaceResult = m_archive.emplace(appSignalId, TrendArchive());
				archiveIt = emplaceResult.first;
			}

			TrendArchive& archive = archiveIt->second;		// archive is MUTABLE

			// Round from/to to 1hour
			//
			TimeStamp fromTimeStamp((from.toMSecsSinceEpoch() / 1_hour) * 1_hour);
			TimeStamp toTimeStamp((to.toMSecsSinceEpoch() / 1_hour) * 1_hour + (to.toMSecsSinceEpoch() % 1_hour == 0 ? 0 : 1_hour));

			//QDateTime roundedToHourFrom = fromTimeStamp.toDateTime();
			//QDateTime roundedToHourTo = toTimeStamp.toDateTime();

			qDebug() << "Requested for trend data, appSignalID: " << appSignalId;
			qDebug() << "\tRequested from " << from << ", rounded to " << fromTimeStamp.toDateTime();
			qDebug() << "\tRequested to " << to << ", rounded to " << toTimeStamp.toDateTime();

			// --
			//
			for (TimeStamp archHour = fromTimeStamp; archHour <= toTimeStamp; archHour.timeStamp += 1_hour)
			{
				if (archHour.toDateTime().time().minute() != 0 ||
					archHour.toDateTime().time().second() != 0 ||
					archHour.toDateTime().time().msec() != 0)
				{
					assert(archHour.toDateTime().time().minute() == 0);
					assert(archHour.toDateTime().time().second() == 0);
					assert(archHour.toDateTime().time().msec() == 0);
					return false;
				}

				OneHourData& hourData = archive.m_hours[archHour];		// Get hour data, insert if there is no such record

				switch (hourData.state)
				{
				case OneHourData::State::NoData:
					// No data, request data from archive
					//
					emit requestData(appSignalId, archHour);
					break;
				case OneHourData::State::Requested:
					// Data already requested, wait for it, so just do nothing
					//
					break;
				case OneHourData::State::Received:
					// Data requested and received, pass it to the result
					//
					outData->push_back(hourData);
					break;
				default:
					assert(false);
					return false;
				}
			}

		}	// QMutexLocker locker(&m_archiveMutex);

		return true;
	}

}
