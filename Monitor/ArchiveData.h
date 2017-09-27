#ifndef ARCHIVEDATA_H
#define ARCHIVEDATA_H
#include <list>
#include <map>
#include <memory>
#include "../lib/TimeStamp.h"
#include "../lib/AppSignal.h"


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
};

#endif // ARCHIVEDATA_H
