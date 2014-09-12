#include "../include/StreamedData.h"

namespace Proto
{

	StreamedData::StreamedData()
	{
	}

	StreamedData::StreamedData(const char* data, size_t size) :
		m_data(data, static_cast<int>(size))
	{
	}

	StreamedData::StreamedData(const QByteArray& src) :
		m_data(src)
	{
	}

	StreamedData::StreamedData(size_t capacity)
	{
		m_data.reserve(static_cast<int>(capacity));
	}

	StreamedData::~StreamedData()
	{
	}

	const char* StreamedData::data() const
	{
		return m_data.data();
	}

	QByteArray& StreamedData::mutable_data()
	{
		return m_data;
	}

	size_t StreamedData::length() const
	{
		return m_data.size() * sizeof(m_data[0]);
	}

	void StreamedData::clear()
	{
		m_data.clear();
	}

}
