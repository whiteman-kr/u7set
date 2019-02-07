#include "UnitsConvertor.h"

UnitsConvertor::UnitsConvertor(QObject *parent)
	: QObject(parent)
{
}

UnitsConvertor::~UnitsConvertor()
{
}

QVariant UnitsConvertor::physicalToElectric(double val, double electricLowLimit, double electricHighLimit, int unitID, int sensorType)
{
	QVariant retVal;
	retVal.clear();

	switch(unitID)
	{
		case E::ElectricUnit::V:

			retVal = val;

			break;

		case E::ElectricUnit::mA:

			switch (sensorType)
			{
				case E::SensorType::V_0_5:
					retVal = val / RESISTOR_V_0_5;
					break;

				default:
					assert(0);
					break;
			}
			break;

		default:
			assert(0);
			break;
	}

	return retVal;
}

QVariant UnitsConvertor::electricToPhysical(double val, double electricLowLimit, double electricHighLimit, int unitID, int sensorType)
{
	QVariant retVal;
	retVal.clear();

	switch(unitID)
	{
		case E::ElectricUnit::V:

			retVal = val;

			break;

		case E::ElectricUnit::mA:

			switch (sensorType)
			{
				case E::SensorType::V_0_5:
					retVal = val * RESISTOR_V_0_5;
					break;

				default:
					assert(0);
					break;
			}
			break;

		default:
			assert(0);
			break;
	}

	return retVal;
}

