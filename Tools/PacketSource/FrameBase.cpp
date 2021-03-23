#include "FrameBase.h"

#include <assert.h>

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

PS::FrameData::FrameData()
{
	clear();
}

// -------------------------------------------------------------------------------------------------------------------

PS::FrameData::FrameData(const FrameData& from)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

PS::FrameData::~FrameData()
{
}

// -------------------------------------------------------------------------------------------------------------------

void PS::FrameData::clear()
{
	QMutexLocker l(&m_frameMutex);

	memset(&m_data[0], 0, Rup::FRAME_DATA_SIZE);
}

// -------------------------------------------------------------------------------------------------------------------

PS::FrameData& PS::FrameData::operator=(const PS::FrameData& from)
{
	QMutexLocker l(&m_frameMutex);

	memcpy(m_data, from.m_data, Rup::FRAME_DATA_SIZE);

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

FrameBase::FrameBase(QObject *parent) :
	QObject(parent)
{
}


// -------------------------------------------------------------------------------------------------------------------

FrameBase::~FrameBase()
{
}

// -------------------------------------------------------------------------------------------------------------------

void FrameBase::clear()
{
	QMutexLocker l(&m_frameMutex);

	m_frameList.clear();
}

// -------------------------------------------------------------------------------------------------------------------

int FrameBase::count() const
{
	QMutexLocker l(&m_frameMutex);

	return TO_INT(m_frameList.size());
}

// -------------------------------------------------------------------------------------------------------------------

bool FrameBase::setFrameCount(int count)
{
	bool result = false;

	if (count == 0)
	{
		count = 1;
	}

	QMutexLocker l(&m_frameMutex);

	m_frameList.clear();

	m_frameList.resize(static_cast<quint64>(count));

	result = TO_INT(m_frameList.size()) == count;
	return result;
}

// -------------------------------------------------------------------------------------------------------------------

PS::FrameData FrameBase::frameData(int index) const
{
	QMutexLocker l(&m_frameMutex);

	if (index < 0 || index >= TO_INT(m_frameList.size()))
	{
		return PS::FrameData();
	}

	return m_frameList[static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

PS::FrameData* FrameBase::frameDataPtr(int index)
{
	QMutexLocker l(&m_frameMutex);

	if (index < 0 || index >= TO_INT(m_frameList.size()))
	{
		return nullptr;
	}

	return &m_frameList[static_cast<quint64>(index)];
}

// -------------------------------------------------------------------------------------------------------------------

void FrameBase::setFrameData(int index, const PS::FrameData &frame)
{
	QMutexLocker l(&m_frameMutex);

	if (index < 0 || index >= TO_INT(m_frameList.size()))
	{
		return;
	}

	m_frameList[static_cast<quint64>(index)] = frame;
}

// -------------------------------------------------------------------------------------------------------------------

FrameBase& FrameBase::operator=(const FrameBase& from)
{
	QMutexLocker l(&m_frameMutex);

	m_frameList = from.m_frameList;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
