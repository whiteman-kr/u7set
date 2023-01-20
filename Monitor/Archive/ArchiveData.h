#pragma once

#include "../AppSignalLib/AppSignalParam.h"
#include "../CommonLib/Times.h"


struct ArchiveSignal
{
	AppSignalParam signalParam;
	QString archiveServiceId;
	QString archiveServiceShortenId;
};


struct ArchiveSource
{
	std::vector<ArchiveSignal> acceptedSignals;

	E::TimeType timeType = E::TimeType::Local;
	TimeStamp requestStartTime;
	TimeStamp requestEndTime;
	bool removePeriodicRecords;
};
Q_DECLARE_METATYPE(ArchiveSource)

// The request result of one or several signals from one ArchiveServiceService
//
struct ArchiveRequestResult
{
	QString archiveServiceId;
	std::deque<AppSignalState> states;
};
Q_DECLARE_METATYPE(std::shared_ptr<ArchiveRequestResult>)


class ArchiveData
{
public:
	ArchiveData() = default;
	~ArchiveData() = default;

public:
	bool addChunk(ArchiveRequestResult&& ArchiveRequestResult, E::TimeType timeType);

	[[nodiscard]] int size() const;
	void clear();

	[[nodiscard]] const AppSignalState& state(int index) const;

private:
	std::deque<AppSignalState> m_archive;

	static constexpr int MaxArchiveStates = 100'000;
	static constexpr AppSignalState NullState;
};

