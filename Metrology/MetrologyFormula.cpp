#include "MetrologyFormula.h"
#include "UnitsConverterTable.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

double calcMetrologyError(Measure::MT::ErrorType errorType, double nominal, double measure, double lowLimit, double highLimit)
{
	double errorValue = 0;

	switch (errorType)
	{
		case Measure::MT::ErrorType::Absolute:

			errorValue = std::abs(nominal - measure);

			break;

		case Measure::MT::ErrorType::Reduce:

			errorValue = std::abs(((nominal - measure) / (highLimit - lowLimit)) * 100.0);

			break;

		case Measure::MT::ErrorType::Relative:

			errorValue = std::abs(((nominal-measure) / nominal) * 100.0);

			break;

		default:
			assert(0);
	}

	return errorValue;
}

// -------------------------------------------------------------------------------------------------------------------

double calcMetrologyErrorLimit(Measure::MT::ErrorType errorType, double limitValue, double lowLimit, double highLimit)
{
	double errorLimitValue = 0;

	switch (errorType)
	{
		case Measure::MT::ErrorType::Absolute:

			errorLimitValue = std::abs((highLimit - lowLimit) * limitValue / 100.0);

			break;

		case Measure::MT::ErrorType::Reduce:
		case Measure::MT::ErrorType::Relative:

			errorLimitValue = limitValue;

			break;

		default:
			assert(0);
	}

	return errorLimitValue;
}

// -------------------------------------------------------------------------------------------------------------------

double calcMaxDeviation(double measureAvg, const std::vector<double>& measureArray)
{
	int measureCount = static_cast<int>(measureArray.size());
	if (measureCount == 0)
	{
		assert(0);
		return 0;
	}

	double maxDeviation = 0;
	int maxDeviationIndex = 0;

	for(int index = 0; index < measureCount; index++)
	{
		if (maxDeviation < std::abs(measureAvg - measureArray[index]))
		{
			maxDeviation = std::abs(measureAvg - measureArray[index]);
			maxDeviationIndex = index;
		}
	}

	return measureArray[maxDeviationIndex];
}

// -------------------------------------------------------------------------------------------------------------------

double calcSystemDeviation(double measure, double nominal)
{
	// according to GOST 8.508-84 paragraph 3.4.1 formula 42
	//
	double systemDeviation = measure - nominal;

	return systemDeviation;
}

// -------------------------------------------------------------------------------------------------------------------

double calcSCO(double measureAvg, const std::vector<double>& measureArray)
{
	int measureCount = static_cast<int>(measureArray.size());
	if (measureCount == 0)
	{
		assert(0);
		return 0;
	}

	// according to GOST 8.736-2011 paragraph 5.3 formula 3
	//
	double sumDeviation = 0;

	for(int index = 0; index < measureCount; index++)
	{
		sumDeviation += pow(measureAvg - measureArray[index], 2);	// 1. sum of deviations
	}

	sumDeviation /= static_cast<double>(measureCount - 1);			// 2. divide on (count of measurements - 1)

	double sco = sqrt(sumDeviation);								// 3. sqrt

	return sco;
}

// -------------------------------------------------------------------------------------------------------------------

double calcLowBorder(double systemDeviation, double sco, int measureCount)
{
	if (measureCount == 0)
	{
		return 0;
	}

	// Student's rate according to GOST 27.202 on P = 0.95

		// or GOST 8.207-76 application 2 (last page)
		//
	double k_student = studentK(measureCount, CT_PROPABILITY_95);

		// according to RD 34.11.206-88
		//
	double lowBorder = systemDeviation - k_student * sco;

	return lowBorder;
}

// -------------------------------------------------------------------------------------------------------------------

double calcHighBorder(double systemDeviation, double sco, int measureCount)
{
	if (measureCount == 0)
	{
		return 0;
	}

	// Student's rate according to GOST 27.202 on P = 0.95

		// or GOST 8.207-76 application 2 (last page)
		//
	double k_student = studentK(measureCount, CT_PROPABILITY_95);

		// according to RD 34.11.206-88
		//
	double highBorder = systemDeviation + k_student * sco;

	return highBorder;
}

// -------------------------------------------------------------------------------------------------------------------

double calcUcertainty1(double Kox, double sco, double Kxj, double dEj, double MPx)
{
	double uncertainty = 0;

	// Simulate:	Calibrator Input Electric
	// Receive:		Metrology receive Engineering and save
	//
	uncertainty = Kox * sqrt( pow(sco, 2) + (pow(Kxj,2) * pow(dEj,2) / 3) + (pow(MPx,2) / 3) );

	return uncertainty;
}

// -------------------------------------------------------------------------------------------------------------------

double calcUcertainty2(double Kox, double sco, double dEj, double MPe)
{
	double uncertainty = 0;

	// Simulate:	Calibrator Input Electric
	// Receive:		Metrology receive Engineering conver to Electric and save
	//
	uncertainty = Kox * sqrt( pow(sco, 2) + (pow(dEj,2) / 3) + (pow(MPe,2) / 3) );

	return uncertainty;
}

// -------------------------------------------------------------------------------------------------------------------

double calcUcertainty3(double Kox, double sco, double dIj, double MPi)
{
	double uncertainty = 0;

	// Simulate:	Tuning
	// Receive:		Calibrator Output Electric
	//
	uncertainty = Kox * sqrt( pow(sco, 2) + (pow(dIj,2) / 3) + (pow(MPi,2) / 12) );

	return uncertainty;
}

// -------------------------------------------------------------------------------------------------------------------

double calcUcertainty4(double Kox, double sco, double Kij, double dEj, double dIj, double MPi)
{
	double uncertainty = 0;

	// Simulate:	Calibrator Input Electric
	// Receive:		Calibrator Output Electric
	//
	uncertainty = Kox * sqrt( pow(sco, 2) + (pow(Kij,2) * pow(dEj,2) / 3) + (pow(dIj,2) / 3) + (pow(MPi,2) / 12) );

	return uncertainty;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

namespace Measure
{
	bool MT::ERR_MEASURE_CALC_ERROR_RANGE(CalcErrorRange byRange)
	{
		if (static_cast<int>(byRange) < 0 || static_cast<int>(byRange) >= Measure::MT::CALC_ERROR_RANGE_COUNT)
		{
			return true;
		}

		return false;
	}

	QString MT::CalcErrorRangeCaption(CalcErrorRange byRange)
	{
		QString caption;

		switch (byRange)
		{
			case By_Electric_Range:		caption = QT_TRANSLATE_NOOP("MetrologyFormula", "Electric range");				break;
			case By_Engineering_Range:	caption = QT_TRANSLATE_NOOP("MetrologyFormula", "Engineering range");			break;
			case By_Signal_Type:		caption = QT_TRANSLATE_NOOP("MetrologyFormula", "Depended from signal type");	break;

			default:
				Q_ASSERT(0);
				caption = QT_TRANSLATE_NOOP("MetrologyFormula", "Unknown");
		}

		return QCoreApplication::instance()->translate("MetrologyFormula", caption.toUtf8());
	};

	QString MT::CalcErrorRangeCaptionTr(CalcErrorRange byRange)
	{
		QString caption;

		switch (byRange)
		{
			case By_Electric_Range:		caption = QObject::tr("By_Electric_Range");		break;
			case By_Engineering_Range:	caption = QObject::tr("By_Engineering_Range");	break;
			case By_Signal_Type:		caption = QObject::tr("By_Signal_Type");		break;

			default:
				Q_ASSERT(0);
				caption = QObject::tr("Unknown");
		}

		return caption;
	};

	bool MT::ERR_MEASURE_ERROR_TYPE(ErrorType errorType)
	{
		if (static_cast<int>(errorType) < 0 || static_cast<int>(errorType) >= Measure::MT::ERROR_TYPE_COUNT)
		{
			return true;
		}

		return false;
	}

	QString MT::ErrorTypeCaption(ErrorType errorType)
	{
		QString caption;

		switch (errorType)
		{
			case Absolute:	caption = QT_TRANSLATE_NOOP("MetrologyFormula", "Absolute");	break;
			case Reduce:	caption = QT_TRANSLATE_NOOP("MetrologyFormula", "Reduce");		break;
			case Relative:	caption = QT_TRANSLATE_NOOP("MetrologyFormula", "Relative");	break;

			default:
				Q_ASSERT(0);
				caption = QT_TRANSLATE_NOOP("MetrologyFormula", "Unknown");
		}

		return qApp->translate("MetrologyFormula", caption.toUtf8());
	};

	QString MT::ErrorTypeCaptionTr(ErrorType errorType)
	{
		QString caption;

		switch (errorType)
		{
			case Absolute:	caption = QObject::tr("Absolute");	break;
			case Reduce:	caption = QObject::tr("Reduce");	break;
			case Relative:	caption = QObject::tr("Relative");	break;

			default:
				Q_ASSERT(0);
				caption = QObject::tr("Unknown");
		}

		return caption;
	};
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
