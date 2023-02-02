#include "ArchiveData.h"


bool ArchiveData::addChunk(ArchiveRequestResult&& chunk, E::TimeType timeType)
{
	try
	{
		// Calc required size
		//
		size_t sizeRequired = 0;
		for (const auto& stateVector : chunk.states)
		{
			sizeRequired += stateVector.size();
		}

		// Preallocate space in the archive
		//
		if (m_archive.capacity() < m_archive.size() + sizeRequired)
		{
			m_archive.reserve(m_archive.size() + sizeRequired);
		}

		// Copy states to the archive
		//
		for (const auto& stateVector : chunk.states)
		{
			m_archive.insert(m_archive.end(),
							 std::make_move_iterator(stateVector.begin()),
							 std::make_move_iterator(stateVector.end()));
		}
	}
	catch (const std::exception&)
	{
		return false;
	}

	//	Stable sort is applied, so the actual order for same times (from different sources) is kept
	//
	auto functor = [timeType](const AppSignalState& lhs, const AppSignalState& rhs) -> bool
	{
		switch (timeType)
		{
		case E::TimeType::Local:
			return lhs.m_time.local < rhs.m_time.local;
		case E::TimeType::Plant:
			return lhs.m_time.plant< rhs.m_time.plant;
		case E::TimeType::System:
			return lhs.m_time.system< rhs.m_time.system;
		default:
			Q_ASSERT(FALSE);
			return lhs.m_time.system< rhs.m_time.system;
		}
	};

	std::stable_sort(m_archive.begin(), m_archive.end(), functor);

	// Limit the size of archive.
	// We need to add new itesm, sort them, and only them can resize vector, as these items can be from
	// different archive services.
	//
	if (m_archive.size() > MaxArchiveStates)
	{
		m_archive.resize(MaxArchiveStates);
	}

	return true;
}

int ArchiveData::size() const
{
	return static_cast<int>(m_archive.size());
}

void ArchiveData::clear()
{
	m_archive.clear();
}

const AppSignalState& ArchiveData::state(int index) const
{
	if (index < 0 || index >= size())
	{
		Q_ASSERT(false);
		return NullState;
	}

	return m_archive[index];
}
