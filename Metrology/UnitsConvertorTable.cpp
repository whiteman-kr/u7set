#include "UnitsConvertorTable.h"
#include <limits>

double findConversionVal(double val, const double* pArray, int size, bool isDegree)
{
	double retVal = 0;

	for (int i = 0; i < (size*2) - 1; i += 2)
	{
		if (isDegree == true)
		{
			if ((std::nextafter(val, std::numeric_limits<double>::lowest()) <= pArray[i] && std::nextafter(val, std::numeric_limits<double>::max()) >= pArray[i]) == true)
			{
				retVal = pArray[i+1];
				break;
			}

			if ((val > pArray[i]) && (val < pArray[i+2]))
			{
				retVal = ((pArray[i+3] - pArray[i+1])*(val-pArray[i]))/(pArray[i+2]-pArray[i])+pArray[i+1];
				break;
			}
		}
		else
		{
			if ((std::nextafter(val, std::numeric_limits<double>::lowest()) <= pArray[i+1] && std::nextafter(val, std::numeric_limits<double>::max()) >= pArray[i+1]) == true)
			{
				retVal = pArray[i];
				break;
			}

			if ((val > pArray[i+1]) && (val < pArray[i+3]))
			{
				retVal = ((pArray[i+2] - pArray[i])*(val-pArray[i+1]))/(pArray[i+3]-pArray[i+1])+pArray[i];
				break;
			}
		}
	}

	return retVal;
}

double studentK(int measureCount, int p)
{
	if (measureCount < 0 || measureCount >= K_STUDENT_COUNT)
	{
		return 0;
	}

	if (p < 0 || p >= CT_PROPABILITY_COUNT)
	{
		return 0;
	}

	return K_STUDENT[measureCount];
	//return K_STUDENT[measureCount][P];
}

// -------------------------------------------------------------------------------------------------------------------------------------------------
