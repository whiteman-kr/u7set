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


struct ArchiveChunk
{
	std::vector<AppSignalState> states;
};

Q_DECLARE_METATYPE(ArchiveChunk)


class ArchiveData
{
public:
	ArchiveData();

public:
	void addChunk(const std::shared_ptr<ArchiveChunk>& chunk);

	[[nodiscard]] int size() const;
	void clear();

	[[nodiscard]] AppSignalState state(int index) const;

private:
	std::vector<std::shared_ptr<ArchiveChunk>> m_chunks;
	int m_cachedSize = 0;
};

