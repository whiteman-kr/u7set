#include "ArchiveData.h"

ArchiveData::ArchiveData()
{
	m_chunks.reserve(128);
	m_cachedSize = 0;
}


void ArchiveData::addChunk(std::shared_ptr<ArchiveChunk> chunk)
{
	if (chunk == nullptr ||
		chunk->states.empty() == true)
	{
		assert(chunk);
		return;
	}

	m_chunks.push_back(chunk);
	m_cachedSize += static_cast<int>(chunk->states.size());

	return;
}

int ArchiveData::size() const
{
	return m_cachedSize;
//	int result = 0;

//	for (const std::shared_ptr<ArchiveChunk>& c : m_chunks)
//	{
//		result += static_cast<int>(c->states.size());
//	}

//	return result;
}

void ArchiveData::clear()
{
	m_chunks.clear();
	m_cachedSize = 0;
}

AppSignalState ArchiveData::state(int index) const
{
	if (index < 0)
	{
		assert(index >= 0);
		return AppSignalState();
	}

	if (index >= 20000)
	{
		int uiiu = 0;
		uiiu ++;
	}

	int currentIndex = 0;
	for (const std::shared_ptr<ArchiveChunk>& c : m_chunks)
	{
		int chunkSize = static_cast<int>(c->states.size());

		if (index >= currentIndex && index < currentIndex + chunkSize)
		{
			return c->states[index - currentIndex];
		}

		currentIndex += chunkSize;
	}

	assert(index >= size());
	return AppSignalState();
}
