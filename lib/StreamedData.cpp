#include "../include/StreamedData.h"

namespace Proto
{

	CStreamedData::CStreamedData()
	{
	}

	CStreamedData::CStreamedData(const char* data, size_t size) :
		m_data(data, static_cast<int>(size))
	{
	}

	CStreamedData::CStreamedData(const QByteArray& src) :
		m_data(src)
	{
	}

	CStreamedData::CStreamedData(size_t capacity)
	{
		m_data.reserve(static_cast<int>(capacity));
	}

	CStreamedData::~CStreamedData()
	{
	}

	const char* CStreamedData::data() const
	{
		return m_data.data();
	}

	QByteArray& CStreamedData::mutable_data()
	{
		return m_data;
	}

	size_t CStreamedData::length() const
	{
		return m_data.size() * sizeof(m_data[0]);
	}

	void CStreamedData::clear()
	{
		m_data.clear();
	}

}
