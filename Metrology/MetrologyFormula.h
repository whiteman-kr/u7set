#ifndef METROLOGY_FORMULA_H
#define METROLOGY_FORMULA_H
#include <QObject>

// ==============================================================================================

namespace Measure
{
	namespace MT
	{
		Q_NAMESPACE

		// =======================================================================

		enum CalcErrorRange
		{
			By_Electric_Range = 0,
			By_Engineering_Range = 1,
			By_Signal_Type = 2,
		};
		Q_ENUM_NS(CalcErrorRange)

		const int CALC_ERROR_RANGE_COUNT = 3;

		bool ERR_MEASURE_CALC_ERROR_RANGE(CalcErrorRange byRange);

		QString CalcErrorRangeCaption(CalcErrorRange byRange);
		QString CalcErrorRangeCaptionTr(CalcErrorRange byRange);

		// =======================================================================

		enum ErrorType
		{
			Absolute = 0,
			Reduce = 1,
			Relative = 2,
		};
		Q_ENUM_NS(ErrorType)

		const int ERROR_TYPE_COUNT = 3;

		bool ERR_MEASURE_ERROR_TYPE(ErrorType errorType);

		QString ErrorTypeCaption(ErrorType errorType);
		QString ErrorTypeCaptionTr(ErrorType errorType);

		// =======================================================================
	}
}

// ==============================================================================================

double calcMetrologyError(Measure::MT::ErrorType errorType, double nominal, double measure, double lowLimit, double highLimit);
double calcMetrologyErrorLimit(Measure::MT::ErrorType errorType, double limitValue, double lowLimit, double highLimit);

// ==============================================================================================

double calcMaxDeviation(double measureAvg, const std::vector<double>& measureArray);
double calcSystemDeviation(double measure, double nominal);
double calcSCO(double measureAvg, const std::vector<double>& measureArray);
double calcLowBorder(double systemDeviation, double sco, int measureCount);
double calcHighBorder(double systemDeviation, double sco, int measureCount);

// ==============================================================================================
// Uncertainty of measurement to Document: EA-04/02 M:2013
// Instruction of Radiy: 460009.034-I19
//
double calcUcertainty1(double Kox, double sco, double Kxj, double dEj, double MPx);
double calcUcertainty2(double Kox, double sco, double dEj, double MPe);
double calcUcertainty3(double Kox, double sco, double dIj, double MPi);
double calcUcertainty4(double Kox, double sco, double Kij, double dEj, double dIj, double MPi);

// ==============================================================================================

#endif // METROLOGY_FORMULA_H
