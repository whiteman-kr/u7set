#include "Conversion.h"

// -------------------------------------------------------------------------------------------------------------------------------------------------

double conversionByConnection(double val, const IoSignalParam &ioParam, ConversionDirection directType)
{
	int connectionType = ioParam.connectionType();
	if (connectionType < 0 || connectionType >= Metrology::ConnectionTypeCount)
	{
		return val;
	}

	const Metrology::SignalParam& inParam = ioParam.param(Metrology::ConnectionIoType::Source);
	if (inParam.isValid() == false)
	{
		return val;
	}

	const Metrology::SignalParam& outParam = ioParam.param(Metrology::ConnectionIoType::Destination);
	if (outParam.isValid() == false)
	{
		return val;
	}

	UnitsConvertor uc;

	double retVal = uc.conversionByConnection(val, connectionType, inParam, outParam, directType);
	return retVal;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------


