#include "./include/TrendView/Trend.h"
#include "TrendImpl.h"

namespace TrendLib
{
	Trend::Trend() :
		m_impl(std::make_unique<TrendLib::TrendImpl>())
	{
	}

	Trend::~Trend() = default;

	void Trend::draw(QImage* image, const TrendParam& drawParam) const
	{
		return m_impl->draw(image, drawParam);
	}

	QUuid Trend::uuid() const
	{
		return m_impl->uuid();
	}

	void Trend::setUuid(QUuid value)
	{
		m_impl->setUuid(value);
	}

	TrendLib::TrendSignalSet& Trend::signalSet()
	{
		return m_impl->signalSet();
	}

	const TrendLib::TrendSignalSet& Trend::signalSet() const
	{
		return m_impl->signalSet();
	}

	TrendLib::TrendImpl& Trend::impl()
	{
		return *m_impl;
	}

	const TrendLib::TrendImpl& Trend::impl() const
	{
		return *m_impl;
	}
} // namespace TrendLib
