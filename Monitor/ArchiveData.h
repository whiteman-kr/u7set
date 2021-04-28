#ifndef ARCHIVEDATA_H
#define ARCHIVEDATA_H

#include "../AppSignalLib/AppSignalParam.h"
#include "../CommonLib/Times.h"

struct ArchiveSource
{
	std::vector<AppSignalParam> acceptedSignals;
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
	void addChunk(std::shared_ptr<ArchiveChunk> chunk);

	int size() const;
	void clear();

	AppSignalState state(int index) const;

private:
	std::vector<std::shared_ptr<ArchiveChunk>> m_chunks;
	int m_cachedSize = 0;
};

#endif // ARCHIVEDATA_H
