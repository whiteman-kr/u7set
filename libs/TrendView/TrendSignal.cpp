#include <TrendView/TrendSignal.h>
#include <type_traits>


namespace TrendLib
{

	TrendSignalParam::TrendSignalParam()
	{
	}

	TrendSignalParam::TrendSignalParam(const AppSignalParam& appSignal,
									   const TrendLib::ArchiveServer& archiveServer) :
		m_signalId(appSignal.customSignalId()),
		m_appSignalId(appSignal.appSignalId()),
		m_caption(appSignal.caption()),
		m_equipmentId(appSignal.equipmentId()),
		m_archiveServer{archiveServer},
		m_type(appSignal.type()),
		m_unit(appSignal.units()),
		m_tags(appSignal.tags()),
		m_precision(appSignal.precision()),
		m_highLimit(appSignal.highEngineeringUnits()),
		m_lowLimit(appSignal.lowEngineeringUnits())
	{
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

		message->set_archive_service_id(m_archiveServer.equipmentId.toStdString());
		message->set_archive_service_short_id(m_archiveServer.shortEquipmentId.toStdString());
		message->set_data_service_id(m_archiveServer.dataServiceId.toStdString());

		message->set_type(static_cast<int>(m_type));
		message->set_unit(m_unit.toStdString());
		message->set_analog_format(E::valueToString<E::AnalogFormat>(m_analogFormat).toStdString());
		message->set_precision(m_precision);

		message->set_line_weight(m_lineWeight);

		message->set_high_limit(m_highLimit);
		message->set_low_limit(m_lowLimit);

		message->set_color(m_color);

		// Tags
		//
		message->clear_tags();
		for (const QString& tag : m_tags)
		{
			message->add_tags(tag.toStdString());
		}

		// View limits
		//
		for (const auto& it : m_viewLimits)
		{
		  ::Proto::TrendViewLimit* l = message->add_view_limits();
			l->set_type(static_cast<int>(it.first));	// Type

			const TrendViewLimits& limits = it.second;
			l->set_high_limit(limits.highLimit);
			l->set_low_limit(limits.lowLimit);
		}

		return true;
	}

	bool TrendSignalParam::load(const ::Proto::TrendSignalParam& message)
	{
		m_signalId = QString::fromStdString(message.signal_id());
		m_appSignalId = QString::fromStdString(message.app_signal_id());
		m_caption = QString::fromStdString(message.caption());
		m_equipmentId = QString::fromStdString(message.equipment_id());

		m_archiveServer.equipmentId = QString::fromStdString(message.archive_service_id());
		m_archiveServer.shortEquipmentId = QString::fromStdString(message.archive_service_short_id());
		m_archiveServer.dataServiceId = QString::fromStdString(message.archive_service_short_id());

		m_type = static_cast<E::SignalType>(message.type());
		m_unit = QString::fromStdString(message.unit());
		m_precision = message.precision();

		QString analogFormatString = message.has_analog_format() ?
										 QString::fromStdString(message.analog_format()) :
										 QStringLiteral("g_9_or_9e");
		std::pair<E::AnalogFormat, bool> loadedAnalogFormat = E::stringToValue<E::AnalogFormat>(analogFormatString);
		if (loadedAnalogFormat.second == true)
		{
			m_analogFormat = loadedAnalogFormat.first;
		}

		m_lineWeight = message.line_weight();

		m_highLimit = message.high_limit();
		m_lowLimit = message.low_limit();

		m_color = message.color();

		// Tags
		//
		m_tags.clear();
		for (const auto& t : message.tags())
		{
			m_tags.insert(QString::fromStdString(t));
		}

		// View limits
		//
		if (message.has_view_high_limit() == true && message.has_view_low_limit() == true)
		{
			// Legacy trends before 21.04.2020
			//
			std::vector<E::TrendScaleType> scaleTypes = E::values<E::TrendScaleType>();
			for (auto scaleType : scaleTypes)
			{
				setViewHighLimit(scaleType, message.view_high_limit());
				setViewLowLimit(scaleType, message.view_low_limit());
			}
		}
		else
		{
			for (int i = 0; i < message.view_limits_size(); i++)
			{
				const ::Proto::TrendViewLimit& limit = message.view_limits(i);
				setViewHighLimit(static_cast<E::TrendScaleType>(limit.type()), limit.high_limit());
				setViewLowLimit(static_cast<E::TrendScaleType>(limit.type()), limit.low_limit());
			}
		}

		return true;
	}

	TrendSignalPlusServerId TrendSignalParam::signalPlusServerId() const
	{
		return TrendSignalPlusServerId
			{
				.appSignalId = m_appSignalId,
				.archiveServerId = m_archiveServer.equipmentId
			};
	}

	const QString& TrendSignalParam::signalId() const
	{
		return m_signalId;
	}

	void TrendSignalParam::setSignalId(const QString& value)
	{
		m_signalId = value;
	}

	const QString& TrendSignalParam::appSignalId() const
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

	const QString& TrendSignalParam::caption() const
	{
		return m_caption;
	}

	void TrendSignalParam::setCaption(const QString& value)
	{
		m_caption = value;
	}

	const QString& TrendSignalParam::equipmentId() const
	{
		return m_equipmentId;
	}

	void TrendSignalParam::setEquipmentId(const QString& value)
	{
		m_equipmentId = value;
	}

	const QString& TrendSignalParam::archiveServerId() const
	{
		return m_archiveServer.equipmentId;
	}

	void TrendSignalParam::setArchiveServerId(const QString& value)
	{
		m_archiveServer.equipmentId = value;
	}

	const QString& TrendSignalParam::archiveServerShortId() const
	{
		return m_archiveServer.shortEquipmentId;
	}

	void TrendSignalParam::setArchiveServerShortId(const QString& value)
	{
		m_archiveServer.shortEquipmentId = value;
	}

	const ArchiveServer& TrendSignalParam::archiveServer() const
	{
		return m_archiveServer;
	}

	void TrendSignalParam::setArchiveServer(const ArchiveServer& value)
	{
		m_archiveServer = value;
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

	const QString& TrendSignalParam::unit() const
	{
		return m_unit;
	}

	void TrendSignalParam::setUnit(const QString& value)
	{
		m_unit = value;
	}

	const std::set<QString>& TrendSignalParam::tags() const
	{
		return m_tags;
	}

	std::set<QString>& TrendSignalParam::mutableTags()
	{
		return m_tags;
	}

	QStringList TrendSignalParam::tagStringList() const
	{
		QStringList result;
		result.reserve(static_cast<int>(m_tags.size()));

		for (const QString& tag : m_tags)
		{
			result << tag;
		}

		return result;
	}

	void TrendSignalParam::setTags(std::set<QString> tags)
	{
		m_tags = std::move(tags);
	}

	bool TrendSignalParam::hasTag(const QString& tag) const
	{
		return m_tags.contains(tag);
	}

	E::AnalogFormat TrendSignalParam::analogFormat() const
	{
		return m_analogFormat;
	}

	void TrendSignalParam::setAnalogFormat(E::AnalogFormat format)
	{
		m_analogFormat = format;
	}

	int TrendSignalParam::precision() const
	{
		return m_precision;
	}

	void TrendSignalParam::setPrecision(int value)
	{
		m_precision = value;
	}

	double TrendSignalParam::lineWeight() const
	{
		return m_lineWeight;
	}

	void TrendSignalParam::setLineWeight(double value)
	{
		m_lineWeight = qBound<double>(0.0, value, 10.0);
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

	double TrendSignalParam::viewHighLimit(E::TrendScaleType scaleType) const
	{
		const auto& it = m_viewLimits.find(scaleType);
		if (it == m_viewLimits.end())
		{
			return highLimit();
		}

		const TrendViewLimits& limits = it->second;
		return limits.highLimit;
	}

	void TrendSignalParam::setViewHighLimit(E::TrendScaleType scaleType, double value)
	{
		TrendViewLimits& limits = m_viewLimits[scaleType];
		limits.highLimit = qBound(-1e+100, value, 1e+100);
	}

	double TrendSignalParam::viewLowLimit(E::TrendScaleType scaleType) const
	{
		const auto& it = m_viewLimits.find(scaleType);
		if (it == m_viewLimits.end())
		{
			return lowLimit();
		}

		const TrendViewLimits& limits = it->second;
		return limits.lowLimit;
	}

	void TrendSignalParam::setViewLowLimit(E::TrendScaleType scaleType, double value)
	{
		TrendViewLimits& limits = m_viewLimits[scaleType];
		limits.lowLimit = qBound(-1e+100, value, 1e+100);
	}

	TrendColor TrendSignalParam::color() const
	{
		return m_color;
	}

	void TrendSignalParam::setColor(const TrendColor& value)
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

	const QRectF& TrendSignalParam::tempDrawRect() const
	{
		return m_tempDrawRect;
	}

	void TrendSignalParam::setTempDrawRect(const QRectF& value)
	{
		m_tempDrawRect = value;
	}
}
