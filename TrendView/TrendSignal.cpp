#include "TrendSignal.h"
#include <type_traits>
#include "../Proto/trends.pb.h"

namespace TrendLib
{

	bool TrendStateRecord::save(Proto::TrendStateRecord* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		bool ok = true;

		// Saving TrendStateItem_v1
		//
		static_assert(std::is_same<std::vector<TrendStateItem>::value_type, TrendStateItem_v1>::value, "Expepcted TrendStateItem_v1");
		message->set_states_raw_buffer_v1(reinterpret_cast<const char*>(states.data()), states.size() * sizeof(TrendStateItem_v1));

		return ok;
	}

	bool TrendStateRecord::load(const Proto::TrendStateRecord& message)
	{
		bool ok = true;

		// Loading TrendStateItem_v1
		//
		Q_ASSERT(message.states_raw_buffer_v1().size() % sizeof(TrendStateItem_v1) == 0);
		size_t stateCount = message.states_raw_buffer_v1().size() / sizeof(TrendStateItem_v1);

		states.clear();
		states.resize(stateCount);

		memcpy(states.data(), message.states_raw_buffer_v1().data(), message.states_raw_buffer_v1().size());

		return ok;
	}

	bool OneHourData::save(const TimeStamp& timeStamp, Proto::TrendArchiveHour* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		bool ok = true;

		message->set_time_stamp(timeStamp.timeStamp);
		message->set_state(static_cast<int>(state));

		message->mutable_records()->Reserve(static_cast<int>(data.size()));
		for (const TrendStateRecord& record : data)
		{
			ok &= record.save(message->add_records());
		}

		return ok;
	}

	bool OneHourData::load(const Proto::TrendArchiveHour& message)
	{
		bool ok = true;

		// message.time_stamp() -- is not read jere, it is required on one level lower, in TrendArchive as a key to map
		state = static_cast<OneHourData::State>(message.state());

		data.clear();
		data.reserve(message.records_size());

		for (int i = 0; i < message.records_size(); i++)
		{
			data.emplace_back();
			ok &= data.back().load(message.records(i));
		}

		return ok;
	}

	bool TrendArchive::save(QString mapAppSignalId, Proto::TrendArchive* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		bool ok = true;

		//message->set_app_signal_id(mapAppSignalId.toStdString());
		message->set_app_signal_id(mapAppSignalId.toStdString());

		for (std::pair<TimeStamp, std::shared_ptr<OneHourData>> p : m_hours)
		{
			ok &= p.second->save(p.first, message->add_hours());
		}

		return ok;
	}

	bool TrendArchive::load(const Proto::TrendArchive& message)
	{
		bool ok = true;

		appSignalId = QString::fromStdString(message.app_signal_id());

		m_hours.clear();
		for (int i = 0; i < message.hours_size(); i++)
		{
			const ::Proto::TrendArchiveHour& messageHour = message.hours(i);

			TimeStamp ts(messageHour.time_stamp());

			auto hodit = m_hours.emplace(ts, std::make_shared<OneHourData>());

			std::shared_ptr<OneHourData>& hourData = hodit.first->second;
			ok &= hourData->load(messageHour);
		}

		return ok;
	}

	TrendSignalParam::TrendSignalParam()
	{
	}

	TrendSignalParam::TrendSignalParam(const AppSignalParam& appSignal) :
		m_signalId(appSignal.customSignalId()),
		m_appSignalId(appSignal.appSignalId()),
		m_caption(appSignal.caption()),
		m_equipmentId(appSignal.equipmentId()),
		m_type(appSignal.type()),
		m_unit(appSignal.unit()),
		m_highLimit(appSignal.highEngineeringUnits()),
		m_lowLimit(appSignal.lowEngineeringUnits()),
		m_viewHighLimit(appSignal.highEngineeringUnits()),
		m_viewLowLimit(appSignal.lowEngineeringUnits())
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
		result.setUnit(m_unit);
		result.setHighEngineeringUnits(m_highLimit);
		result.setLowEngineeringUnits(m_lowLimit);

		return result;
	}

	bool TrendSignalParam::save(::Proto::TrendSignalParam* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		message->set_signal_id(m_signalId.toStdString());
		message->set_app_signal_id(m_appSignalId.toStdString());
		message->set_caption(m_caption.toStdString());
		message->set_equipment_id(m_equipmentId.toStdString());

		message->set_type(static_cast<int>(m_type));
		message->set_unit(m_unit.toStdString());

		message->set_high_limit(m_highLimit);
		message->set_low_limit(m_lowLimit);
		message->set_view_high_limit(m_viewHighLimit);
		message->set_view_low_limit(m_viewLowLimit);

		message->set_color(m_color.rgb());

		return true;
	}

	bool TrendSignalParam::load(const ::Proto::TrendSignalParam& message)
	{
		m_signalId = QString::fromStdString(message.signal_id());
		m_appSignalId = QString::fromStdString(message.app_signal_id());
		m_caption = QString::fromStdString(message.caption());
		m_equipmentId = QString::fromStdString(message.equipment_id());

		m_type = static_cast<E::SignalType>(message.type());
		m_unit = QString::fromStdString(message.unit());

		m_highLimit = message.high_limit();
		m_lowLimit = message.low_limit();
		m_viewHighLimit = message.view_high_limit();
		m_viewLowLimit = message.view_low_limit();

		m_color = QColor::fromRgb(message.color());

		return true;
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

	Hash TrendSignalParam::appSignalHash() const
	{
		return ::calcHash(m_appSignalId);
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

	QString TrendSignalParam::unit() const
	{
		return m_unit;
	}

	void TrendSignalParam::setUnit(const QString& value)
	{
		m_unit = value;
	}

	double TrendSignalParam::highLimit() const
	{
		return m_highLimit;
	}

	void TrendSignalParam::setHighLimit(double value)
	{
		m_highLimit = qBound(-1e+100, value, 1e+100);
	}

	double TrendSignalParam::lowLimit() const
	{
		return m_lowLimit;
	}

	void TrendSignalParam::setLowLimit(double value)
	{
		m_lowLimit = qBound(-1e+100, value, 1e+100);
	}

	double TrendSignalParam::viewHighLimit() const
	{
		return m_viewHighLimit;
	}

	void TrendSignalParam::setViewHighLimit(double value)
	{
		m_viewHighLimit = qBound(-1e+100, value, 1e+100);
	}

	double TrendSignalParam::viewLowLimit() const
	{
		return m_viewLowLimit;
	}

	void TrendSignalParam::setViewLowLimit(double value)
	{
		m_viewLowLimit = qBound(-1e+100, value, 1e+100);
	}

	QColor TrendSignalParam::color() const
	{
		return m_color;
	}

	void TrendSignalParam::setColor(const QColor& value)
	{
		m_color = value;
	}

	int TrendSignalParam::tempSignalIndex() const
	{
		return 	m_tempSignalIndex;
	}

	void TrendSignalParam::setTempSignalIndex(int value)
	{
		m_tempSignalIndex = value;
	}

	QRectF TrendSignalParam::tempDrawRect() const
	{
		return m_tempDrawRect;
	}

	void TrendSignalParam::setTempDrawRect(const QRectF& value)
	{
		m_tempDrawRect = value;
	}

	//
	//
	//			TrendSignalSet
	//
	//
	TrendSignalSet::TrendSignalSet()
	{
		static_assert(std::is_trivial<TrendStateItem>::value, "TrendStateItem must be trivial as it is stored in bytearray");
	}

	bool TrendSignalSet::save(::Proto::TrendSignalSet* message) const
	{
		if (message == nullptr)
		{
			Q_ASSERT(message);
			return false;
		}

		bool ok = true;

		// Scope for locking m_paramMutex
		//
		{
			QMutexLocker l(&m_paramMutex);
			for (const TrendSignalParam& p : m_signalParams)
			{
				ok &= p.save(message->add_signal_params());
			}
		}

		// Scope for locking m_archiveMutex
		//
		{
			QMutexLocker l(&m_archiveMutex);

			for (const std::pair<Hash, TrendArchive>& p : m_archiveLocalTime)
			{
				::Proto::TrendArchive* messageTrenaArchive = message->add_archive_local_time();
				ok &= p.second.save(p.second.appSignalId, messageTrenaArchive);
			}

			for (const std::pair<Hash, TrendArchive>& p : m_archiveSystemTime)
			{
				::Proto::TrendArchive* messageTrenaArchive = message->add_archive_system_time();
				ok &= p.second.save(p.second.appSignalId, messageTrenaArchive);
			}

			for (const std::pair<Hash, TrendArchive>& p : m_archivePlantTime)
			{
				::Proto::TrendArchive* messageTrenaArchive = message->add_archive_plant_time();
				ok &= p.second.save(p.second.appSignalId, messageTrenaArchive);
			}
		}

		return ok;
	}

	bool TrendSignalSet::load(const ::Proto::TrendSignalSet& message)
	{
		bool ok = true;

		// Scope for locking m_paramMutex
		//
		{
			QMutexLocker l(&m_paramMutex);

			m_signalParams.clear();
			for (int i = 0; i < message.signal_params_size(); i++)
			{
				TrendSignalParam tsp;

				bool loadTspOk = tsp.load(message.signal_params(i));
				ok &= loadTspOk;

				if (loadTspOk == true)
				{
					m_signalParams.push_back(tsp);
				}
			}
		}

		// Scope for locking m_archiveMutex
		//
		{
			QMutexLocker l(&m_archiveMutex);

			m_archiveLocalTime.clear();
			m_archiveSystemTime.clear();
			m_archivePlantTime.clear();

			for (int i = 0; i < message.archive_local_time_size(); i++)
			{
				const ::Proto::TrendArchive& messageArchive = message.archive_local_time(i);

				QString appSingalId = QString::fromStdString(messageArchive.app_signal_id());

				auto itp = m_archiveLocalTime.emplace(::calcHash(appSingalId), TrendArchive());

				TrendArchive& ta = itp.first->second;
				ok &= ta.load(messageArchive);
			}

			for (int i = 0; i < message.archive_system_time_size(); i++)
			{
				const ::Proto::TrendArchive& messageArchive = message.archive_system_time(i);

				QString appSingalId = QString::fromStdString(messageArchive.app_signal_id());

				auto itp = m_archiveSystemTime.emplace(::calcHash(appSingalId), TrendArchive());

				TrendArchive& ta = itp.first->second;
				ok &= ta.load(messageArchive);
			}

			for (int i = 0; i < message.archive_plant_time_size(); i++)
			{
				const ::Proto::TrendArchive& messageArchive = message.archive_plant_time(i);

				QString appSingalId = QString::fromStdString(messageArchive.app_signal_id());

				auto itp = m_archivePlantTime.emplace(::calcHash(appSingalId), TrendArchive());

				TrendArchive& ta = itp.first->second;
				ok &= ta.load(messageArchive);
			}
		}

		return ok;
	}

	bool TrendSignalSet::addSignal(const TrendSignalParam& signal)
	{
		QMutexLocker locker(&m_paramMutex);

		if (m_signalParams.size() >= 16)
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

		int index = 0;
		for (const TrendSignalParam& s : m_signalParams)
		{
			result.push_back(s);
			result.back().setTempSignalIndex(index);
			index++;
		}

		return result;
	}

	std::vector<TrendLib::TrendSignalParam> TrendSignalSet::analogSignals() const
	{
		QMutexLocker locker(&m_paramMutex);

		std::vector<TrendSignalParam> result;
		result.reserve(m_signalParams.size());

		int index = 0;
		for (const TrendSignalParam& s : m_signalParams)
		{
			if (s.isAnalog() == true)
			{
				result.push_back(s);
				result.back().setTempSignalIndex(index);
				index++;
			}
		}

		return result;
	}

	std::vector<TrendLib::TrendSignalParam> TrendSignalSet::discreteSignals() const
	{
		QMutexLocker locker(&m_paramMutex);

		std::vector<TrendSignalParam> result;
		result.reserve(m_signalParams.size());

		int index = 0;
		for (const TrendSignalParam& s : m_signalParams)
		{
			if (s.isDiscrete() == true)
			{
				result.push_back(s);
				result.back().setTempSignalIndex(index);
				index++;
			}
		}

		return result;
	}

	int TrendSignalSet::discretesSignalsCount() const
	{
		QMutexLocker locker(&m_paramMutex);

		int count = 0;
		for (const TrendSignalParam& s : m_signalParams)
		{
			if (s.isDiscrete() == true)
			{
				count ++;
			}
		}

		return count;
	}

	int TrendSignalSet::analogSignalsCount() const
	{
		QMutexLocker locker(&m_paramMutex);

		int count = 0;
		for (const TrendSignalParam& s : m_signalParams)
		{
			if (s.isAnalog() == true)
			{
				count ++;
			}
		}

		return count;
	}

	bool TrendSignalSet::getExistingTrendData(QString appSignalId, QDateTime from, QDateTime to, E::TimeType timeType, std::list<std::shared_ptr<OneHourData>>* outData) const
	{
		// Get already reqquested and received (o read form file) data
		// Don't request any data if it is not present
		//
		if (outData == nullptr ||
			from > to)
		{
			Q_ASSERT(outData);
			Q_ASSERT(from <= to);
			return false;
		}

		// Find Signal
		//
		QMutexLocker locker(&m_archiveMutex);

		std::map<Hash, TrendArchive>* m_archive = nullptr;
		switch (timeType)
		{
		case E::TimeType::Local:	m_archive = &m_archiveLocalTime;	break;
		case E::TimeType::System:	m_archive = &m_archiveSystemTime;	break;
		case E::TimeType::Plant:	m_archive = &m_archivePlantTime;	break;
		default:
			Q_ASSERT(false);
			return false;
		}

		auto archiveIt = m_archive->find(::calcHash(appSignalId));
		if (archiveIt == m_archive->end())
		{
			return false;
		}

		TrendArchive& archive = archiveIt->second;		// archive is MUTABLE

		// Round from/to to 1hour
		//
		TimeStamp fromTimeStamp((from.toMSecsSinceEpoch() / 1_hour) * 1_hour);
		TimeStamp toTimeStamp((to.toMSecsSinceEpoch() / 1_hour) * 1_hour + (to.toMSecsSinceEpoch() % 1_hour == 0 ? 0 : 1_hour));

//		qDebug() << "getExistingTrendData for appSignalID: " << appSignalId;
//		qDebug() << "\tAsk data from " << from << ", rounded to " << fromTimeStamp.toDateTime();
//		qDebug() << "\tAsk data to  " << to << ", rounded to " << toTimeStamp.toDateTime();

		// --
		//
		for (TimeStamp archHour = fromTimeStamp; archHour < toTimeStamp; archHour.timeStamp += 1_hour)
		{
			if (archHour.toDateTime().time().minute() != 0 ||
				archHour.toDateTime().time().second() != 0 ||
				archHour.toDateTime().time().msec() != 0)
			{
				Q_ASSERT(archHour.toDateTime().time().minute() == 0);
				Q_ASSERT(archHour.toDateTime().time().second() == 0);
				Q_ASSERT(archHour.toDateTime().time().msec() == 0);
				return false;
			}

			auto archHourIt = archive.m_hours.find(archHour);

			if (archHourIt == archive.m_hours.end())
			{
				continue;
			}

			std::shared_ptr<OneHourData> hourData = archHourIt->second;
			if (hourData == nullptr)
			{
				Q_ASSERT(hourData);
				continue;
			}

			// Make a copy of OneHourData, as it can be appended in Realtime trends
			//
			auto copiedHourData = std::make_shared<TrendLib::OneHourData>(hourData.operator*());

			outData->push_back(copiedHourData);				// Request state does not matter
		}

		return true;
	}

	bool TrendSignalSet::getTrendData(QString appSignalId, QDateTime from, QDateTime to, E::TimeType timeType, std::list<std::shared_ptr<OneHourData>>* outData) const
	{
		if (outData == nullptr ||
			from > to)
		{
			Q_ASSERT(outData);
			Q_ASSERT(from <= to);
			return false;
		}

		// Find Signal
		//
		{
			QMutexLocker locker(&m_archiveMutex);

			std::map<Hash, TrendArchive>* m_archive = nullptr;
			switch (timeType)
			{
			case E::TimeType::Local:	m_archive = &m_archiveLocalTime;	break;
			case E::TimeType::System:	m_archive = &m_archiveSystemTime;	break;
			case E::TimeType::Plant:	m_archive = &m_archivePlantTime;	break;
			default:
				Q_ASSERT(false);
				return false;
			}

			auto archiveIt = m_archive->find(::calcHash(appSignalId));
			if (archiveIt == m_archive->end())
			{
				auto emplaceResult = m_archive->emplace(::calcHash(appSignalId), TrendArchive());
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
			for (TimeStamp archHour = fromTimeStamp; archHour < toTimeStamp; archHour.timeStamp += 1_hour)
			{
				if (archHour.toDateTime().time().minute() != 0 ||
					archHour.toDateTime().time().second() != 0 ||
					archHour.toDateTime().time().msec() != 0)
				{
					Q_ASSERT(archHour.toDateTime().time().minute() == 0);
					Q_ASSERT(archHour.toDateTime().time().second() == 0);
					Q_ASSERT(archHour.toDateTime().time().msec() == 0);
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
					{
						// Data requested and received, pass it to the result
						// MAKE A COPY of hourData
						//
						auto copiedHourData = std::make_shared<TrendLib::OneHourData>(hourData.operator*());

						outData->push_back(copiedHourData);
					}
					break;
				default:
					Q_ASSERT(false);
					return false;
				}
			}

		}	// QMutexLocker locker(&m_archiveMutex);

		return true;
	}

	void TrendSignalSet::clear(E::TimeType timeType)
	{
		QMutexLocker locker(&m_archiveMutex);

		switch (timeType)
		{
		case E::TimeType::Local:
			m_archiveLocalTime.clear();
			break;
		case E::TimeType::System:
			m_archiveSystemTime.clear();
			break;
		case E::TimeType::Plant:
			m_archivePlantTime.clear();
			break;
		default:
			Q_ASSERT(false);
			break;
		}

		return;
	}

	void TrendSignalSet::addNonValidPoint()
	{
		// Add non valid points to all signals, useful in switching mode Archive/RealTime
		//
		addNonValidPoint(E::TimeType::Local);
		addNonValidPoint(E::TimeType::System);
		addNonValidPoint(E::TimeType::Plant);

		return;
	}

	void TrendSignalSet::addNonValidPoint(E::TimeType timeType)
	{
		// Add non valid points to all signals, useful in switching mode Archive/RealTime
		//
		QMutexLocker locker(&m_archiveMutex);

		std::map<Hash, TrendArchive>* archive = nullptr;
		switch (timeType)
		{
		case E::TimeType::Local:	archive = &m_archiveLocalTime;	break;
		case E::TimeType::System:	archive = &m_archiveSystemTime;	break;
		case E::TimeType::Plant:	archive = &m_archivePlantTime;	break;
		default:
			Q_ASSERT(false);
			return;
		}

		// --
		//
		for (auto& [hash, trendArchive] : *archive)
		{
			if (trendArchive.m_hours.empty() == true)
			{
				// Do not add non valid point if signal archive is empty
				//
			}
			else
			{
				// Find the last hour with points and add non valid state to it
				//
				for (auto rhit = trendArchive.m_hours.rbegin(); rhit != trendArchive.m_hours.rend(); ++rhit)
				{
					std::shared_ptr<OneHourData> hour = rhit->second;

					if (hour->data.empty() == true)
					{
						// It can be just request for ah hour
						// Process the next hour
						//
						continue;
					}
					else
					{
						// Assume that record has some states
						//
						TrendStateRecord& record = hour->data.back();

						if (record.states.empty() == false)
						{
							// Just duplicate last state with invalid flag
							//
							TrendStateItem tsi = record.states.back();
							tsi.flags = 0;

							record.states.push_back(tsi);
						}
						else
						{
							Q_ASSERT(record.states.empty() == false);
						}

						break;
					}
				}
			}
		}


		return;
	}

	void TrendSignalSet::slot_archiveDataReceived(QString appSignalId, TimeStamp requestedHour, E::TimeType timeType, std::shared_ptr<TrendLib::OneHourData> data)
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

		std::map<Hash, TrendArchive>* m_archive = nullptr;
		switch (timeType)
		{
		case E::TimeType::Local:	m_archive = &m_archiveLocalTime;	break;
		case E::TimeType::System:	m_archive = &m_archiveSystemTime;	break;
		case E::TimeType::Plant:	m_archive = &m_archivePlantTime;	break;
		default:
			Q_ASSERT(false);
			return;
		}

		auto archiveIt = m_archive->find(::calcHash(appSignalId));
		if (archiveIt == m_archive->end())
		{
			auto emplaceResult = m_archive->emplace(::calcHash(appSignalId), TrendArchive());
			archiveIt = emplaceResult.first;
		}

		TrendArchive& archive = archiveIt->second;		// archive is MUTABLE

		// --
		//
		if (requestedHour.toDateTime().time().minute() != 0 ||
			requestedHour.toDateTime().time().second() != 0 ||
			requestedHour.toDateTime().time().msec() != 0)
		{
			Q_ASSERT(requestedHour.toDateTime().time().minute() == 0);
			Q_ASSERT(requestedHour.toDateTime().time().second() == 0);
			Q_ASSERT(requestedHour.toDateTime().time().msec() == 0);
			return;
		}

		archive.m_hours[requestedHour] = data;

		return;
	}

	void TrendSignalSet::slot_archiveRequestError(QString appSignalId, TimeStamp requestedHour, E::TimeType timeType)
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

		std::map<Hash, TrendArchive>* m_archive = nullptr;
		switch (timeType)
		{
		case E::TimeType::Local:	m_archive = &m_archiveLocalTime;	break;
		case E::TimeType::System:	m_archive = &m_archiveSystemTime;	break;
		case E::TimeType::Plant:	m_archive = &m_archivePlantTime;	break;
		default:
			Q_ASSERT(false);
			return;
		}

		auto archiveIt = m_archive->find(::calcHash(appSignalId));
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
			Q_ASSERT(requestedHour.toDateTime().time().minute() == 0);
			Q_ASSERT(requestedHour.toDateTime().time().second() == 0);
			Q_ASSERT(requestedHour.toDateTime().time().msec() == 0);
			return;
		}

		archive.m_hours.erase(requestedHour);

		return;
	}

	void TrendSignalSet::slot_realtimeDataReceived(std::shared_ptr<TrendLib::RealtimeData> data, TrendLib::TrendStateItem /*minState*/, TrendLib::TrendStateItem /*maxState*/)
	{
		for (const TrendLib::RealtimeDataChunk& chunk : data->signalData)
		{
			const Hash signalHash = chunk.appSignalHash;
			const std::vector<TrendStateItem>& states = chunk.states;

			// For now add all three times, maybe later it will be changed to add just for one time
			// I just don't know whick kind of time is used now
			//
			appendRealtimeDataToArchive(E::TimeType::Local, signalHash, states);
			appendRealtimeDataToArchive(E::TimeType::System, signalHash, states);
			appendRealtimeDataToArchive(E::TimeType::Plant, signalHash, states);
		}

		return;
	}

	void TrendSignalSet::slot_realtimeRequestError(QString errorText)
	{
	}

	void TrendSignalSet::appendRealtimeDataToArchive(E::TimeType timeType, Hash signalhash, const std::vector<TrendStateItem>& states)
	{
		QMutexLocker locker(&m_archiveMutex);

		std::map<Hash, TrendArchive>* m_archive = nullptr;

		switch (timeType)
		{
		case E::TimeType::Local:	m_archive = &m_archiveLocalTime;	break;
		case E::TimeType::System:	m_archive = &m_archiveSystemTime;	break;
		case E::TimeType::Plant:	m_archive = &m_archivePlantTime;	break;
		default:
			Q_ASSERT(false);
			return;
		}

		auto archiveIt = m_archive->find(signalhash);
		if (archiveIt == m_archive->end())
		{
			return;
		}

		TrendArchive& archive = archiveIt->second;		// archive is MUTABLE

		TimeStamp lastHourTime = 0;
		std::shared_ptr<OneHourData> hourData;

		for (const TrendStateItem& state : states)
		{
			TimeStamp ts = state.getTime(timeType).roundedToHour();
			if (ts == 0)
			{
				qDebug() << "TrendSignalSet: Received wront timestamp: " << ts.timeStamp << ", " << timeType;
				continue;
			}

			if (lastHourTime == ts)
			{
				Q_ASSERT(hourData);
			}
			else
			{
				hourData = archive.m_hours[ts];

				if (hourData.get() == nullptr)	// Just created
				{
					hourData = std::make_shared<TrendLib::OneHourData>();
				}

				lastHourTime = ts;
			}

			hourData->state = TrendLib::OneHourData::State::Received;

			if (hourData->data.empty() == true)
			{
				TrendLib::TrendStateRecord& record = hourData->data.emplace_back();
				record.states.reserve(TrendLib::TrendStateRecord::RecomendedSize);
			}
			else
			{
				TrendLib::TrendStateRecord& lastRecord = hourData->data.back();

				if (lastRecord.states.size() >= TrendLib::TrendStateRecord::RecomendedSize)
				{
					TrendLib::TrendStateRecord& record = hourData->data.emplace_back();
					record.states.reserve(TrendLib::TrendStateRecord::RecomendedSize);
				}
			}

			// Add state
			//
			TrendLib::TrendStateRecord& recordToAddState = hourData->data.back();
			recordToAddState.states.push_back(state);
		}

		return;
	}

}
