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

	double TrendScale::scaleLowLimit(const TrendSignalParam& trendSignal, E::TrendScaleType scaleType, bool* ok)
	{
		double value = qMin(trendSignal.viewHighLimit(scaleType), trendSignal.viewLowLimit(scaleType));

		return pointToScaleValue(value, scaleType, ok);
	}

	double TrendScale::scaleHighLimit(const TrendSignalParam& trendSignal, E::TrendScaleType scaleType, bool* ok)
	{
		double value = qMax(trendSignal.viewHighLimit(scaleType), trendSignal.viewLowLimit(scaleType));

		return pointToScaleValue(value, scaleType, ok);
	}

	double TrendScale::valueToScaleValue(double value, E::TrendScaleType scaleType, bool* ok)
	{
		if (scaleType == E::TrendScaleType::Period)
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

	double TrendScale::limitFromScaleValue(double scaleValue, E::TrendScaleType scaleType, bool* ok)
	{
		return pointFromScaleValue(scaleValue, scaleType, ok);
	}

	double TrendScale::valueFromScaleValue(double scaleValue, E::TrendScaleType scaleType, bool* ok)
	{
		if (ok != nullptr)
		{
			*ok = true;
		}

		double result = pointFromScaleValue(scaleValue, scaleType, ok);

		if (scaleType == E::TrendScaleType::Period)
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
	std::optional<std::vector<std::pair<double, double>>> TrendScale::scaleValues(E::TrendScaleType scaleType, double lowLimit, double highLimit, const QRectF& signalRect, double minInchInterval)
	{
		switch (scaleType)
		{
		case E::TrendScaleType::Linear:
		case E::TrendScaleType::Log10:
			{
				return scaleValuesGeneric(scaleType, lowLimit, highLimit, signalRect, minInchInterval);
			}
		case E::TrendScaleType::Period:
			{
				return scaleValuesPeriod(scaleType, lowLimit, highLimit);
			}
		default:
			Q_ASSERT(false);
		}
		return {};
	}

	QString TrendScale::scaleValueText(double value, E::TrendScaleType scaleType, const TrendSignalParam& signalParam)
	{
		if (scaleType == E::TrendScaleType::Period)
		{
			if (std::fabs(round(value)) >= periodScaleInfinity)
			{
				return QString(QChar(0x221E));	// Infinity sign
			}
		}

		if (scaleType == E::TrendScaleType::Log10)
		{
			if (std::fabs(value) <= DBL_MIN)
			{
				return "0";
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

	double TrendScale::trendLog10(double value)
	{
		// Logarithm calculation.
		// The result is shifted up by DBL_MAX_10_EXP.
		// For negative value, logarithm is taken from absolute value and then shifted and multiplied by -1.
		// This means that we take a "ghost" logarithm from negative value.

		double result = std::fabs(value);

		if (result < DBL_MIN)
		{
			result = DBL_MIN;
		}

		result = std::log10(result);

		result += DBL_MAX_10_EXP;

		if (value < 0)
		{
			result = -result;
		}

		return result;
	}

	double TrendScale::trendPow10(double value)
	{
		// Power calculation, reverse function for trendLog10.
		// Input value is shifted down by DBL_MAX_10_EXP and power is calculated from its absoulte value.
		// The sign of the result depened on input value sign.

		double result = std::fabs(value);

		result -= DBL_MAX_10_EXP;

		result = std::pow(10, result);

		if (value < 0)
		{
			result = -result;
		}

		return result;
	}

	double TrendScale::pointToScaleValue(double value, E::TrendScaleType scaleType, bool* ok)
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

	double TrendScale::pointFromScaleValue(double scaleValue, E::TrendScaleType scaleType, bool* ok)
	{
		if (ok != nullptr)
		{
			*ok = true;
		}

		switch (scaleType)
		{
		case E::TrendScaleType::Linear:
			{
				return scaleValue;
			}
		case E::TrendScaleType::Log10:
			{
				scaleValue = trendPow10(scaleValue);

				return scaleValue;

			}
		case E::TrendScaleType::Period:
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
	std::optional<std::vector<std::pair<double, double>>> TrendScale::scaleValuesGeneric(E::TrendScaleType scaleType, double lowLimit, double highLimit, const QRectF& signalRect, double minInchInterval)
	{
		if (scaleType != E::TrendScaleType::Linear && scaleType != E::TrendScaleType::Log10)
		{
			Q_ASSERT(scaleType == E::TrendScaleType::Linear || scaleType == E::TrendScaleType::Log10);
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
	std::optional<std::vector<std::pair<double, double>>> TrendScale::scaleValuesPeriod(E::TrendScaleType scaleType, double lowLimit, double highLimit)
	{
		if (scaleType != E::TrendScaleType::Period)
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
