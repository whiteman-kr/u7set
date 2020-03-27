#ifndef TRENDSCALE_H
#define TRENDSCALE_H

#include "TrendParam.h"
#include <optional>


namespace TrendLib
{
	class TrendScale
	{
	public:
		TrendScale();

		// Scale-specific functions
		//
	public:
		static double timeToScaledPixel(const TimeStamp& time, const QRectF& rect, const TimeStamp& startTime, qint64 duration);
		static double valueToScaledPixel(double value, const QRectF& rect, double lowLimit, double highLimit);

		static double limitToScalePoint(double value, TrendScaleType scaleType, bool* ok);
		static double valueToScalePoint(double value, TrendScaleType scaleType, bool* ok);

		static double valueFromScalePoint(double scaleValue, TrendScaleType scaleType, bool* ok);

		static std::optional<std::vector<std::pair<double, double>>> scaleValues(TrendScaleType scaleType, double lowLimit, double highLimit, const QRectF& signalRect);

		static QString scalePointText(double value, const TrendParam& drawParam, int precision);

	private:
		static double pointToScalePoint(double value, TrendScaleType scaleType, bool* ok);

		static std::optional<std::vector<std::pair<double, double>>> scaleValuesGeneric(TrendScaleType scaleType, double lowLimit, double highLimit, const QRectF& signalRect);
		static std::optional<std::vector<std::pair<double, double>>> scaleValuesPeriod(TrendScaleType scaleType, double lowLimit, double highLimit);

	private:
		const static double expScaleInfinityValue; // = 999;	// Infinity value for exponential scale
	};
}

#endif // TRENDSCALE_H
