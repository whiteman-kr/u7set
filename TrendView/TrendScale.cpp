#include "TrendScale.h"

namespace TrendLib
{
	const double TrendScale::expScaleInfinityValue = 999; // Infinity value for exponential scale

	TrendScale::TrendScale()
	{

	}

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
		double delta = fabs(highLimit - lowLimit);

		if (delta <= DBL_MIN)
		{
			Q_ASSERT(fabs(highLimit - lowLimit) > DBL_MIN);
			return 0;
		}

		return rect.bottom() - (rect.height() / delta) * (value - lowLimit);
	}

	double TrendScale::limitToScalePoint(double value, TrendScaleType scaleType, bool* ok)
	{
		return pointToScalePoint(value, scaleType, ok);
	}

	double TrendScale::valueToScalePoint(double value, TrendScaleType scaleType, bool* ok)
	{
		double result = pointToScalePoint(value, scaleType, ok);

		if (ok == nullptr || *ok == true)
		{
			// Period trend is drawn from infinity point

			if (scaleType == TrendScaleType::Period)
			{
				double infinityValueLog = log(expScaleInfinityValue);

				result = qBound(-infinityValueLog, result, infinityValueLog);

				if (result < 0)
				{
					result = -infinityValueLog - result;
				}
				else
				{
					result = infinityValueLog - result;
				}
			}
		}

		return result;
	}

	double TrendScale::valueFromScalePoint(double scaleValue, TrendScaleType scaleType, bool* ok)
	{
		if (ok != nullptr)
		{
			*ok = true;
		}

		switch (scaleType)
		{
		case TrendScaleType::Generic:
		{
			return scaleValue;
		}
		case TrendScaleType::Logarithmic:
		{
			return std::pow(10, scaleValue);
		}
		case TrendScaleType::Period:
		{
			if (scaleValue < 0)
			{
				scaleValue = -exp(-scaleValue);
			}
			else
			{
				scaleValue = exp(scaleValue);
			}

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

	std::optional<std::vector<std::pair<double, double>>> TrendScale::scaleValues(TrendScaleType scaleType, double lowLimit, double highLimit, const QRectF& signalRect)
	{
		switch (scaleType)
		{
		case TrendScaleType::Generic:
		case TrendScaleType::Logarithmic:
		{
			return scaleValuesGeneric(scaleType, lowLimit, highLimit, signalRect);
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

	QString TrendScale::scalePointText(double value, const TrendParam& drawParam, int precision)
	{
		switch (drawParam.scaleType())
		{
		case TrendScaleType::Generic:
		{
			return QString(" %1 ").arg(QString::number(value, 'f', precision));
		}
		case TrendScaleType::Logarithmic:
		{
			return QString(" %1 ").arg(QString::number(value, 'e', precision));
		}
		case TrendScaleType::Period:
		{
			if (fabs(value) >= expScaleInfinityValue)
			{
				return QString(QChar(0x221E));
			}
			return QString(" %1 ").arg(QString::number(value, 'f', precision));
		}
		default:
			Q_ASSERT(false);
			return QString();
		}
	}

	double TrendScale::pointToScalePoint(double value, TrendScaleType scaleType, bool* ok)
	{
		if (ok != nullptr)
		{
			*ok = true;
		}

		switch (scaleType)
		{
		case TrendScaleType::Generic:
		{
			return value;
		}
		case TrendScaleType::Logarithmic:
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
				return log10(value);
			}
		}
		case TrendScaleType::Period:
		{
			value = qBound(-expScaleInfinityValue, value, expScaleInfinityValue);

			if (fabs(value) <= 1.0)
			{
				return value > 0 ? DBL_MIN : -DBL_MIN;
			}

			if (value < 0)
			{
				value = -log(-value);
			}
			else
			{
				if (value > 0)
				{
					value = log(value);
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

	std::optional<std::vector<std::pair<double, double>>> TrendScale::scaleValuesGeneric(TrendScaleType scaleType, double lowLimit, double highLimit, const QRectF& signalRect)
	{
		if (scaleType != TrendScaleType::Generic && scaleType != TrendScaleType::Logarithmic)
		{
			Q_ASSERT(scaleType == TrendScaleType::Generic || scaleType == TrendScaleType::Logarithmic);
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

		double minInchInterval = 1.0/4.0;	// 1/4 in -- minimum inches interval
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

			double scaleValue = valueFromScalePoint(value, scaleType, &ok);

			if (ok == false)
			{
				scaleValue = std::numeric_limits<double>::quiet_NaN();
			}

			result.emplace_back(value, scaleValue);
		}

		return result;
	}

	std::optional<std::vector<std::pair<double, double>>> TrendScale::scaleValuesPeriod(TrendScaleType scaleType, double lowLimit, double highLimit)
	{
		if (scaleType != TrendScaleType::Period)
		{
			Q_ASSERT(scaleType == TrendScaleType::Period);
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
																  TrendScale::expScaleInfinityValue,
																  -640, -320, -160, -80, -40, -20, -10, -5, -2};

		std::vector<std::pair<double, double>> result;

		for (double p : possibleGridPoints)
		{
			bool ok = false;

			double value = valueToScalePoint(p, scaleType, &ok);

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
