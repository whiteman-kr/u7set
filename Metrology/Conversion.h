#ifndef CONVERSION_H
#define CONVERSION_H

#include <QtGlobal>
#include <assert.h>

#include "SignalBase.h"

#include "../lib/UnitsConvertor.h"

double conversionByConnection(double val, const IoSignalParam& ioParam, ConversionDirection directType);

#endif // CONVERSION_H
