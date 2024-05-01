#ifndef TREND_H
#define TREND_H

#include <memory>

#include "TrendParam.h"


namespace TrendLib
{
	class TrendImpl;
	class TrendSignalSet;

	class Trend
	{
	public:
		Trend();
		~Trend();

		// Forbid copying
		//
		Trend(const Trend&) = delete;
		Trend& operator=(const Trend&) = delete;

		// But allow moving
		//
		Trend(Trend&&) noexcept = default;
		Trend& operator=(Trend&&) noexcept = default;

		
		// Methods
		//
	public:

		// Draw methods
		//
		void draw(QImage* image, const TrendParam& drawParam) const;

		// Properties
		//
	public:
		QUuid uuid() const;
		void setUuid(QUuid value);

		TrendLib::TrendSignalSet& signalSet();
		const TrendLib::TrendSignalSet& signalSet() const;

		TrendLib::TrendImpl& impl();
		const TrendLib::TrendImpl& impl() const;

	private:
		std::unique_ptr<TrendLib::TrendImpl> m_impl;
	};

}
#endif // TREND_H
