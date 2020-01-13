#include "FrameBase.h"

#include <assert.h>
#include <QMessageBox>

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
	memset(&m_data[0], 0, Rup::FRAME_DATA_SIZE);
}

// -------------------------------------------------------------------------------------------------------------------

PS::FrameData& PS::FrameData::operator=(const PS::FrameData& from)
{
	m_frameMutex.lock();

		memcpy(m_data, from.m_data, Rup::FRAME_DATA_SIZE);

	m_frameMutex.unlock();

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
	m_frameMutex.lock();

		m_frameList.clear();

	m_frameMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

int FrameBase::count() const
{
	int count = 0;

	m_frameMutex.lock();

		count = m_frameList.count();

	m_frameMutex.unlock();

	return count;
}

// -------------------------------------------------------------------------------------------------------------------

bool FrameBase::setFrameCount(int count)
{
	bool result = false;

	m_frameMutex.lock();

		m_frameList.clear();

		m_frameList.resize(count);

		result = m_frameList.count() == count;

	m_frameMutex.unlock();

	return result;
}

// -------------------------------------------------------------------------------------------------------------------

PS::FrameData FrameBase::frameData(int index) const
{
	PS::FrameData frameData;

	m_frameMutex.lock();

		if (index >= 0 && index < m_frameList.count())
		{
			frameData = m_frameList[index];
		}

	m_frameMutex.unlock();

	return frameData;
}

// -------------------------------------------------------------------------------------------------------------------

PS::FrameData* FrameBase::frameDataPtr(int index)
{
	PS::FrameData* pFrameData = nullptr;

	m_frameMutex.lock();

		if (index >= 0 && index < m_frameList.count())
		{
			pFrameData = &m_frameList[index];
		}

	m_frameMutex.unlock();

	return pFrameData;
}

// -------------------------------------------------------------------------------------------------------------------

void FrameBase::setFrameData(int index, const PS::FrameData &frame)
{
	m_frameMutex.lock();

		if (index >= 0 && index < m_frameList.count())
		{
			m_frameList[index] = frame;
		}

	m_frameMutex.unlock();
}

// -------------------------------------------------------------------------------------------------------------------

FrameBase& FrameBase::operator=(const FrameBase& from)
{
	m_frameMutex.lock();

		m_frameList = from.m_frameList;

	m_frameMutex.unlock();

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
