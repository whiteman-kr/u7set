#pragma once

#include "../AppSignalLib/AppSignalParam.h"
#include "../AppSignalLib/AppSignalState.h"
#include "../CommonLib/Times.h"


struct ArchiveSignal
{
	AppSignalParam signalParam;
	QString archiveServiceId;
	QString archiveServiceShortenId;
};


struct ArchiveSignalState
{
	AppSignalState appState;
	QString archiveServiceId;
	QString archiveServiceShortenId;
};


struct ArchiveSource
{
	std::vector<ArchiveSignal> acceptedSignals;

	E::TimeType timeType = E::TimeType::Local;
	TimeStamp requestStartTime;
	TimeStamp requestEndTime;
	bool removePeriodicRecords = true;
};
Q_DECLARE_METATYPE(ArchiveSource)

// The request result of one or several signals from one ArchiveServiceService
//
struct ArchiveRequestResult
{
	QString archiveServiceId;
	std::list<std::vector<ArchiveSignalState>> states;
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
	void removeSignal(QString appSignalId, QString archiveServiceId);

	[[nodiscard]] const ArchiveSignalState& state(int index) const;

private:
	std::vector<ArchiveSignalState> m_archive;

	inline static constexpr int MaxArchiveStates = 250'000;
	inline static const ArchiveSignalState NullState{};
};

