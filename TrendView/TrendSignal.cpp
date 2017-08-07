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
		m_highLimit(appSignal.highEngineeringUnits()),
		m_lowLimit(appSignal.lowEngineeringUnits()),
		m_viewHighLimit(appSignal.highEngineeringUnits()),
		m_viewLowLimit(appSignal.lowEngineeringUnits()),
		m_unit(appSignal.unit())
	{
	}

	AppSignalParam TrendSignalParam::toAppSignalParam() const
	{
		AppSignalParam result;

		result.setCustomSignalId(m_signalId);
		result.setAppSignalId(m_appSignalId);
		result.setCaption(m_caption);
		result.setEquipmentId(m_equipmentId);
		result.setType(m_type);
		result.setHighEngineeringUnits(m_highLimit);
		result.setLowEngineeringUnits(m_lowLimit);
		result.setUnit(m_unit);

		return result;
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

	double TrendSignalParam::highLimit() const
	{
		return m_highLimit;
	}

	void TrendSignalParam::setHighLimit(double value)
	{
		m_highLimit = value;
	}

	double TrendSignalParam::lowLimit() const
	{
		return m_lowLimit;
	}

	void TrendSignalParam::setLowLimit(double value)
	{
		m_lowLimit = value;
	}

	double TrendSignalParam::viewHighLimit() const
	{
		return m_viewHighLimit;
	}

	void TrendSignalParam::setViewHighLimit(double value)
	{
		m_viewHighLimit = value;
	}

	double TrendSignalParam::viewLowLimit() const
	{
		return m_viewLowLimit;
	}

	void TrendSignalParam::setViewLowLimit(double value)
	{
		m_viewLowLimit = value;
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

		if (m_signalParams.size() >= 12)
		{
			return false;
		}

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

	TrendLib::TrendSignalParam TrendSignalSet::signalParam(const QString& appSignalId, bool* ok) const
	{
		if (ok != nullptr)
		{
			*ok = false;
		}

		QMutexLocker locker(&m_paramMutex);

		for (const TrendSignalParam& s : m_signalParams)
		{
			if (s.appSignalId() == appSignalId)
			{
				*ok = true;
				return s;
			}
		}

		return TrendLib::TrendSignalParam();
	}

	bool TrendSignalSet::setSignalParam(const TrendLib::TrendSignalParam& signalParam)
	{
		QMutexLocker locker(&m_paramMutex);

		for (TrendSignalParam& s : m_signalParams)
		{
			if (s.appSignalId() == signalParam.appSignalId())
			{
				s = signalParam;
				return true;
			}
		}

		return false;
	}

	std::vector<TrendLib::TrendSignalParam> TrendSignalSet::trendSignals() const
	{
		QMutexLocker locker(&m_paramMutex);

		std::vector<TrendSignalParam> result;
		result.reserve(m_signalParams.size());

		for (const TrendSignalParam& s : m_signalParams)
		{
			result.push_back(s);
		}

		return result;
	}

	std::vector<TrendLib::TrendSignalParam> TrendSignalSet::analogSignals() const
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

	std::vector<TrendLib::TrendSignalParam> TrendSignalSet::discreteSignals() const
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

	bool TrendSignalSet::getTrendData(QString appSignalId, QDateTime from, QDateTime to, TimeType timeType, std::list<std::shared_ptr<OneHourData>>* outData) const
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

			std::map<QString, TrendArchive>* m_archive = nullptr;
			switch (timeType)
			{
			case TimeType::Local:	m_archive = &m_archiveLocalTime;	break;
			case TimeType::System:	m_archive = &m_archiveSystemTime;	break;
			case TimeType::Plant:	m_archive = &m_archivePlantTime;	break;
			default:
				assert(false);
				return false;
			}

			auto archiveIt = m_archive->find(appSignalId);
			if (archiveIt == m_archive->end())
			{
				auto emplaceResult = m_archive->emplace(appSignalId, TrendArchive());
				archiveIt = emplaceResult.first;
			}

			TrendArchive& archive = archiveIt->second;		// archive is MUTABLE

			// Round from/to to 1hour
			//
			TimeStamp fromTimeStamp((from.toMSecsSinceEpoch() / 1_hour) * 1_hour);
			TimeStamp toTimeStamp((to.toMSecsSinceEpoch() / 1_hour) * 1_hour + (to.toMSecsSinceEpoch() % 1_hour == 0 ? 0 : 1_hour));

//			qDebug() << "Requested for trend data, appSignalID: " << appSignalId;
//			qDebug() << "\tRequested from " << from << ", rounded to " << fromTimeStamp.toDateTime();
//			qDebug() << "\tRequested to " << to << ", rounded to " << toTimeStamp.toDateTime();

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

				std::shared_ptr<OneHourData>& hourData = archive.m_hours[archHour];		// Get hour data, insert if there is no such record
																						// hourData is REFERENCE, it's important as shared_ptr can be changed

				if (hourData == nullptr)
				{
					// hourData just was created in call "archive.m_hours[archHour]"
					//
					hourData = std::make_shared<OneHourData>();
				}

				switch (hourData->state)
				{
				case OneHourData::State::NoData:
					// No data, request data from archive
					//
					emit requestData(appSignalId, archHour, timeType);
					hourData->state = OneHourData::State::Requested;
					break;
				case OneHourData::State::Requested:
					// Data already requested, wait for it, just do nothing
					//
					break;
				case OneHourData::State::Received:
					// Data requested and received, pass it to the result
					//
					outData->push_back(hourData);
					// Debug
					//
//					qDebug() << "Found data for hour " << archHour.toDateTime();
//					qDebug() << "\t Has TrendStateRecord(s): " << hourData->data.size();
//					for (const auto& r : hourData->data)
//					{
//						qDebug() << "\t\t Has states: " << r.states.size();
//						if (r.states.size() != 0)
//						{
//							qDebug() << "\t\t\t First state: " << r.states[0].getTime(timeType).toDateTime();
//						}
//					}
					break;
				default:
					assert(false);
					return false;
				}
			}

		}	// QMutexLocker locker(&m_archiveMutex);

		return true;
	}

	void TrendSignalSet::clear(TimeType timeType)
	{
		QMutexLocker locker(&m_archiveMutex);

		switch (timeType)
		{
		case TimeType::Local:
			m_archiveLocalTime.clear();
			break;
		case TimeType::System:
			m_archiveSystemTime.clear();
			break;
		case TimeType::Plant:
			m_archivePlantTime.clear();
			break;
		default:
			assert(false);
			break;
		}

		return;
	}

	void TrendSignalSet::slot_dataReceived(QString appSignalId, TimeStamp requestedHour, TimeType timeType, std::shared_ptr<TrendLib::OneHourData> data)
	{
		// Ignore data if there is no such signal in SignalParams
		// Probably it was requested but later signal was excluded
		//
		{
			QMutexLocker paramLocker(&m_paramMutex);
			auto it = std::find_if(m_signalParams.begin(), m_signalParams.end(),
								   [appSignalId](const TrendSignalParam& p)
								   {
										return p.appSignalId() == appSignalId;
								   });

			if (it == m_signalParams.end())
			{
				return;
			}
		}

		// Find Signal
		//
		QMutexLocker locker(&m_archiveMutex);

		std::map<QString, TrendArchive>* m_archive = nullptr;
		switch (timeType)
		{
		case TimeType::Local:	m_archive = &m_archiveLocalTime;	break;
		case TimeType::System:	m_archive = &m_archiveSystemTime;	break;
		case TimeType::Plant:	m_archive = &m_archivePlantTime;	break;
		default:
			assert(false);
			return;
		}

		auto archiveIt = m_archive->find(appSignalId);
		if (archiveIt == m_archive->end())
		{
			auto emplaceResult = m_archive->emplace(appSignalId, TrendArchive());
			archiveIt = emplaceResult.first;
		}

		TrendArchive& archive = archiveIt->second;		// archive is MUTABLE

		// --
		//
		if (requestedHour.toDateTime().time().minute() != 0 ||
			requestedHour.toDateTime().time().second() != 0 ||
			requestedHour.toDateTime().time().msec() != 0)
		{
			assert(requestedHour.toDateTime().time().minute() == 0);
			assert(requestedHour.toDateTime().time().second() == 0);
			assert(requestedHour.toDateTime().time().msec() == 0);
			return;
		}

		archive.m_hours[requestedHour] = data;

		return;
	}

	void TrendSignalSet::slot_requestError(QString appSignalId, TimeStamp requestedHour, TimeType timeType)
	{
		// Ignore data if there is no such signal in SignalParams
		// Probably it was requested but later signal was excluded
		//
		{
			QMutexLocker paramLocker(&m_paramMutex);
			auto it = std::find_if(m_signalParams.begin(), m_signalParams.end(),
								   [appSignalId](const TrendSignalParam& p)
								   {
										return p.appSignalId() == appSignalId;
								   });

			if (it == m_signalParams.end())
			{
				return;
			}
		}

		// Find Signal
		//
		QMutexLocker locker(&m_archiveMutex);

		std::map<QString, TrendArchive>* m_archive = nullptr;
		switch (timeType)
		{
		case TimeType::Local:	m_archive = &m_archiveLocalTime;	break;
		case TimeType::System:	m_archive = &m_archiveSystemTime;	break;
		case TimeType::Plant:	m_archive = &m_archivePlantTime;	break;
		default:
			assert(false);
			return;
		}

		auto archiveIt = m_archive->find(appSignalId);
		if (archiveIt == m_archive->end())
		{
			return;
		}

		TrendArchive& archive = archiveIt->second;		// archive is MUTABLE

		// --
		//
		if (requestedHour.toDateTime().time().minute() != 0 ||
			requestedHour.toDateTime().time().second() != 0 ||
			requestedHour.toDateTime().time().msec() != 0)
		{
			assert(requestedHour.toDateTime().time().minute() == 0);
			assert(requestedHour.toDateTime().time().second() == 0);
			assert(requestedHour.toDateTime().time().msec() == 0);
			return;
		}

		archive.m_hours.erase(requestedHour);

		return;
	}

}
