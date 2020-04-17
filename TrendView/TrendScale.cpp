#include "TrendScale.h"

namespace TrendLib
{
	const double TrendScale::periodScaleInfinity = 999; // Infinity value for period scale

	double TrendScale::timeToScaledPixel(const TimeStamp& time, const QRectF& rect, const TimeStamp& startTime, qint64 duration)
	{
		if (duration == 0)
		{
			Q_ASSERT(duration != 0);
			duration = 1;
		}

		return rect.left() + (rect.width() / duration) * (time.timeStamp - startTime.timeStamp);
	}

	double TrendScale::valueToScaledPixel(double value, const QRectF& rect, double lowLimit, double highLimit)
	{
		double delta = std::fabs(highLimit - lowLimit);

		if (delta <= DBL_MIN)
		{
			Q_ASSERT(std::fabs(highLimit - lowLimit) > DBL_MIN);
			return 0;
		}

		return rect.bottom() - (rect.height() / delta) * (value - lowLimit);
	}

	double TrendScale::scaledPixelToValue(double pixel, const QRectF& rect, double lowLimit, double highLimit)
	{
		double delta = std::fabs(highLimit - lowLimit);
		if (delta <= DBL_MIN)
		{
			Q_ASSERT(std::fabs(highLimit - lowLimit) > DBL_MIN);
			return 0;
		}

		if (rect.height() <= DBL_MIN)
		{
			Q_ASSERT(rect.height() > DBL_MIN);
			return 0;
		}

		return lowLimit - (pixel - rect.bottom()) / (rect.height() / delta);
	}

	double TrendScale::limitToScaleValue(double value, TrendScaleType scaleType, bool* ok)
	{
		return pointToScaleValue(value, scaleType, ok);
	}

	double TrendScale::valueToScaleValue(double value, TrendScaleType scaleType, bool* ok)
	{
		if (scaleType == TrendScaleType::Period)
		{
			if (std::fabs(value) < DBL_MIN)
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

	double TrendScale::limitFromScaleValue(double scaleValue, TrendScaleType scaleType, bool* ok)
	{
		return pointFromScaleValue(scaleValue, scaleType, ok);
	}

	double TrendScale::valueFromScaleValue(double scaleValue, TrendScaleType scaleType, bool* ok)
	{
		if (ok != nullptr)
		{
			*ok = true;
		}

		double result = pointFromScaleValue(scaleValue, scaleType, ok);

		if (scaleType == TrendScaleType::Period)
		{
			if (std::fabs(result) < DBL_MIN)
			{
				// Divide by 0 is possible
				//
				if (ok != nullptr)
				{
					*ok = false;
				}

				return 0;
			}

			result = TrendScale::periodScaleInfinity / result;
		}

		return result;
	}

	// Build scale points for a trend
	//
	std::optional<std::vector<std::pair<double, double>>> TrendScale::scaleValues(TrendScaleType scaleType, double lowLimit, double highLimit, const QRectF& signalRect, double minInchInterval)
	{
		switch (scaleType)
		{
		case TrendScaleType::Linear:
		case TrendScaleType::Log10:
			{
				return scaleValuesGeneric(scaleType, lowLimit, highLimit, signalRect, minInchInterval);
			}
		case TrendScaleType::Period:
			{
				return scaleValuesPeriod(scaleType, lowLimit, highLimit);
			}
		default:
			Q_ASSERT(false);
		}
		return {};
	}

	QString TrendScale::scaleValueText(double value, TrendScaleType scaleType, const TrendSignalParam& signalParam)
	{
		if (scaleType == TrendScaleType::Period)
		{
			if (std::fabs(round(value)) >= periodScaleInfinity)
			{
				return QString(QChar(0x221E));	// Infinity sign
			}
		}

		E::AnalogFormat format = signalParam.analogFormat();

		if (format == E::AnalogFormat::G_9_or_9E || format == E::AnalogFormat::g_9_or_9e)
		{
			return QString::number(value, static_cast<char>(signalParam.analogFormat())); // Let Qt choose best format and precision
		}
		else
		{
			return QString::number(value, static_cast<char>(signalParam.analogFormat()), signalParam.precision());
		}
	}

	double TrendScale::pointToScaleValue(double value, TrendScaleType scaleType, bool* ok)
	{
		if (ok != nullptr)
		{
			*ok = true;
		}

		switch (scaleType)
		{
		case TrendScaleType::Linear:
			{
				return value;
			}
		case TrendScaleType::Log10:
			{
				if (value <= 0)
				{
					if (ok != nullptr)
					{
						*ok = false;
					}

					return 0;
				}
				else
				{
					return std::log10(value);
				}
			}
		case TrendScaleType::Period:
			{
				value = qBound(-periodScaleInfinity, value, periodScaleInfinity);

				if (std::fabs(value) <= 1.0)
				{
					return value > 0 ? DBL_MIN : -DBL_MIN;
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
						return value > 0 ? DBL_MIN : -DBL_MIN;
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

	double TrendScale::pointFromScaleValue(double scaleValue, TrendScaleType scaleType, bool* ok)
	{
		if (ok != nullptr)
		{
			*ok = true;
		}

		switch (scaleType)
		{
		case TrendScaleType::Linear:
			{
				return scaleValue;
			}
		case TrendScaleType::Log10:
			{
				return std::pow(10, scaleValue);
			}
		case TrendScaleType::Period:
			{
				if (scaleValue < 0)
				{
					scaleValue = -std::exp(-scaleValue);
				}
				else
				{
					scaleValue = std::exp(scaleValue);
				}

				scaleValue = qBound(-periodScaleInfinity, scaleValue, periodScaleInfinity);

				return scaleValue;
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

	// Build scale points for generic or logarithmic trend
	//
	std::optional<std::vector<std::pair<double, double>>> TrendScale::scaleValuesGeneric(TrendScaleType scaleType, double lowLimit, double highLimit, const QRectF& signalRect, double minInchInterval)
	{
		if (scaleType != TrendScaleType::Linear && scaleType != TrendScaleType::Log10)
		{
			Q_ASSERT(scaleType == TrendScaleType::Linear || scaleType == TrendScaleType::Log10);
			return {};
		}

		double delta = highLimit - lowLimit;
		if (delta <= DBL_MIN)
		{
			// Divide by 0 possible
			//
			return {};
		}

		// Calc vert grid
		//
		static const std::array<double, 4> possibleGridIntervals = {0.1, 0.2, 0.25, 0.5};

		double gridValue = 1.0;

		double pow = 1e-100;
		for (int mult = 0; mult <= 200; mult++, pow *= 10.0)
		{
			for (size_t i = 0; i < possibleGridIntervals.size(); i++)
			{
				gridValue = possibleGridIntervals[i] * pow;

				double y = valueToScaledPixel(lowLimit + gridValue, signalRect, lowLimit, highLimit);

				if (signalRect.bottom() - y >= minInchInterval)
				{
					// gridValue contains found suitable value for grid
					//
					mult = 1000000;		// To break outer loop
					break;
				}
			}
		}

		// Align gridValue
		//
		double lowGriddedValue = floor(lowLimit / gridValue) * gridValue;
		int gridCount = static_cast<int>(delta / gridValue) + 2;

		if (gridCount < 0)
		{
			Q_ASSERT(false);
			gridCount = 0;
		}

		if (gridCount > 100)
		{
			// Something wrong
			//
			gridCount = 100;
			return {};
		}

		//
		std::vector<std::pair<double, double>> result;

		for (int i = 0; i < gridCount; i++)
		{

			double value = lowGriddedValue + i * gridValue;

			bool ok = false;

			double scaleValue = limitFromScaleValue(value, scaleType, &ok);

			if (ok == false)
			{
				scaleValue = std::numeric_limits<double>::quiet_NaN();
			}

			result.emplace_back(value, scaleValue);
		}

		return result;
	}

	// Build scale points for periodic trend
	//
	std::optional<std::vector<std::pair<double, double>>> TrendScale::scaleValuesPeriod(TrendScaleType scaleType, double lowLimit, double highLimit)
	{
		if (scaleType != TrendScaleType::Period)
		{
			Q_ASSERT(false);
			return {};
		}

		double delta = highLimit - lowLimit;
		if (delta <= DBL_MIN)
		{
			// Divide by 0 possible
			//
			return {};
		}

		// Calc vert grid
		//

		static const std::array<double, 19> possibleGridPoints = {2, 5, 10, 20, 40, 80, 160, 320, 640,
																  TrendScale::periodScaleInfinity,
																  -640, -320, -160, -80, -40, -20, -10, -5, -2};

		std::vector<std::pair<double, double>> result;

		for (double p : possibleGridPoints)
		{
			bool ok = false;

			double value = valueToScaleValue(p, scaleType, &ok);

			if (ok == false)
			{
				Q_ASSERT(false);
				return result;
			}

			if (value < lowLimit || value > highLimit)
			{
				continue;
			}

			result.emplace_back(value, p);
		}

		return result;
	}
}
