#ifndef CONVERSION_H
#define CONVERSION_H

#include <QtGlobal>

#include "../lib/Signal.h"
#include "../lib/Types.h"

// ==============================================================================================

const int	CT_PHYSICAL_TO_ELECTRIC	= 0,
            CT_ELECTRIC_TO_PHYSICAL	= 1;

const int	CT_COUNT                = 2;

// ==============================================================================================

double conversion(double val, const int& type, const Signal& param);
double conversion(double val, const int& type, const E::InputUnit& unitID, const E::SensorType& sensorType);

// ==============================================================================================

extern double K_STUDENT[];
extern const int K_STUDENT_COUNT;

// ==============================================================================================


#endif // CONVERSION_H
