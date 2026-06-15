#ifndef TRENDSCALE_H
#define TRENDSCALE_H

#include <TrendView/TrendParam.h>
#include <TrendView/TrendSignal.h>
#include <optional>


namespace TrendLib
{
	class TrendScale
	{
	public:
		TrendScale() = delete;

		// Scale-specific functions
		//
	public:
		// Value-To-Pixel and vice versa functions
		//
		static double timeToScaledPixel(const TimeStamp& time, const QRectF& rect, const TimeStamp& startTime, qint64 duration)
		{
			if (duration == 0)
			{
				Q_ASSERT(duration != 0);
				duration = 1;
			}

			return rect.left() + (rect.width() / duration) * (time.timeStamp - startTime.timeStamp);
		}

		static double valueToScaledPixel(double value, const QRectF& rect, double lowLimit, double highLimit)
		{
			double delta = std::abs(highLimit - lowLimit);

			if (delta <= std::numeric_limits<double>::min())
			{
				Q_ASSERT(std::abs(highLimit - lowLimit) > std::numeric_limits<double>::min());
				return 0;
			}

			return rect.bottom() - (rect.height() / delta) * (value - lowLimit);
		}

		static double scaledPixelToValue(double pixel, const QRectF& rect, double lowLimit, double highLimit)
		{
			double delta = std::abs(highLimit - lowLimit);
			if (delta <= std::numeric_limits<double>::min())
			{
				Q_ASSERT(std::abs(highLimit - lowLimit) > std::numeric_limits<double>::min());
				return 0;
			}

			if (rect.height() <= std::numeric_limits<double>::min())
			{
				Q_ASSERT(rect.height() > std::numeric_limits<double>::min());
				return 0;
			}

			return lowLimit - (pixel - rect.bottom()) / (rect.height() / delta);
		}

		// Scale convertion functions
		//
		static double scaleLowLimit(const TrendSignalParam& trendSignal, E::TrendScaleType scaleType, bool* ok);
		static double scaleHighLimit(const TrendSignalParam& trendSignal, E::TrendScaleType scaleType, bool* ok);

		static double valueToScaleValue(double value, E::TrendScaleType scaleType, bool* ok) // Reverses period value from infinity point
		{
			if (scaleType == E::TrendScaleType::Period)
			{
				if (std::abs(value) < std::numeric_limits<double>::min())
				{
					// Divide by 0 is possible
					//
					if (ok != nullptr)
					{
						*ok = false;
					}

					return 0;
				}

				value = periodScaleInfinity / value;
			}

			return pointToScaleValue(value, scaleType, ok);
		}

		static double limitFromScaleValue(double scaleValue, E::TrendScaleType scaleType, bool* ok);
		static double valueFromScaleValue(double scaleValue,
										  E::TrendScaleType scaleType,
										  bool* ok); // Reverses period value from infinity point

													 // Scale building functions
		//
		static std::optional<std::vector<std::pair<double, double>>> scaleValues(E::TrendScaleType scaleType,
																				 double lowLimit,
																				 double highLimit,
																				 const QRectF& signalRect,
																				 double minInchInterval);

		// Text formatting functions
		//
		static QString scaleValueText(double value, E::TrendScaleType scaleType, const TrendSignalParam& signalParam);

	private:
		static double trendLog10(double value);
		static double trendPow10(double value);

		static double pointToScaleValue(double value, E::TrendScaleType scaleType, bool* ok)
		{
			if (ok != nullptr)
			{
				*ok = true;
			}

			switch (scaleType)
			{
			case E::TrendScaleType::Linear:
				{
					return value;
				}
			case E::TrendScaleType::Log10:
				{
					return trendLog10(value);
				}
			case E::TrendScaleType::Period:
				{
					value = qBound(-periodScaleInfinity, value, periodScaleInfinity);

					if (std::abs(value) <= 1.0)
					{
						return value > 0 ? std::numeric_limits<double>::min() : -std::numeric_limits<double>::min();
					}

					if (value < 0)
					{
						value = -std::log(-value);
					}
					else
					{
						if (value > 0)
						{
							value = std::log(value);
						}
						else
						{
							Q_ASSERT(false);
							return value > 0 ? std::numeric_limits<double>::min() : -std::numeric_limits<double>::min();
						}
					}

					return value;
				}
			default:
				Q_ASSERT(false);
				if (ok != nullptr)
				{
					*ok = false;
				}
			}

			return 0;
		}

		static double pointFromScaleValue(double scaleValue, E::TrendScaleType scaleType, bool* ok);

		static std::optional<std::vector<std::pair<double, double>>> scaleValuesGeneric(E::TrendScaleType scaleType,
																						double lowLimit,
																						double highLimit,
																						const QRectF& signalRect,
																						double minInchInterval);
		static std::optional<std::vector<std::pair<double, double>>> scaleValuesPeriod(E::TrendScaleType scaleType,
																					   double lowLimit,
																					   double highLimit);

	public:
		const static double periodScaleInfinity; // = 999;	// Infinity value for exponential scale
	};
} // namespace TrendLib

#endif // TRENDSCALE_H
