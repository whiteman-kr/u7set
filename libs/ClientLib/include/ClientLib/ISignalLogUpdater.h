#pragma once
#include <optional>


namespace ClientLib
{
	class ISignalLogUpdater
	{
	public:
		using TimeStampType = std::int64_t;
		using RecordIterator = google::protobuf::RepeatedPtrField<::Network::DiscretesLogRecord>::const_iterator;

		virtual ~ISignalLogUpdater() = default;

		virtual void clear() = 0;
		virtual bool enabled() const = 0;

		virtual void add(const std::string& adsId, RecordIterator begin, RecordIterator end) = 0;
		virtual void deleteUpTo(const std::string& adsId, int64_t recordId) = 0;

		virtual std::optional<TimeStampType> getNextAckUpTo() = 0;
	};
} // namespace ClientLib