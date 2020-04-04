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
        // Value-To-Pixel and vice versa functions
        //
		static double timeToScaledPixel(const TimeStamp& time, const QRectF& rect, const TimeStamp& startTime, qint64 duration);

        static double valueToScaledPixel(double value, const QRectF& rect, double lowLimit, double highLimit);
        static double scaledPixelToValue(double pixel, const QRectF& rect, double lowLimit, double highLimit);

        // Scale convertion functions
        //
        static double limitToScaleValue(double value, TrendScaleType scaleType, bool* ok);
        static double valueToScaleValue(double value, TrendScaleType scaleType, bool* ok);  // Reverses period value from infinity point

        static double limitFromScaleValue(double scaleValue, TrendScaleType scaleType, bool* ok);
        static double valueFromScaleValue(double scaleValue, TrendScaleType scaleType, bool* ok);  // Reverses period value from infinity point

        // Scale building functions
        //
        static std::optional<std::vector<std::pair<double, double>>> scaleValues(TrendScaleType scaleType, double lowLimit, double highLimit, const QRectF& signalRect, double minInchInterval);

        // Text formatting functions
        //
        static QString scaleValueText(double value, const TrendParam& drawParam, int precision);

	private:
        static double pointToScaleValue(double value, TrendScaleType scaleType, bool* ok);
        static double pointFromScaleValue(double scaleValue, TrendScaleType scaleType, bool* ok);

        static std::optional<std::vector<std::pair<double, double>>> scaleValuesGeneric(TrendScaleType scaleType, double lowLimit, double highLimit, const QRectF& signalRect, double minInchInterval);
		static std::optional<std::vector<std::pair<double, double>>> scaleValuesPeriod(TrendScaleType scaleType, double lowLimit, double highLimit);

	public:
		const static double periodScaleInfinity; // = 999;	// Infinity value for exponential scale
	};
}

#endif // TRENDSCALE_H
