#include "UnitsConvertor.h"

UnitsConvertor::UnitsConvertor(QObject *parent)
	: QObject(parent)
{
}

UnitsConvertor::~UnitsConvertor()
{
}

QVariant UnitsConvertor::physicalToElectric(double phVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType)
{
	QVariant elVal;
	elVal.clear();

	switch(unitID)
	{
		case E::ElectricUnit::V:

			switch (sensorType)
			{
				case E::SensorType::V_0_5:

					if (electricLowLimit < V_0_5_LOW_LIMIT || electricHighLimit > V_0_5_HIGH_LIMIT)
					{
						break;
					}

					elVal = phVal;
					break;

				case E::SensorType::V_m10_p10:

					if (electricLowLimit < V_m10_p10_LOW_LIMIT || electricHighLimit > V_m10_p10_HIGH_LIMIT)
					{
						break;
					}

					elVal = phVal;
					break;
			}

			break;

		case E::ElectricUnit::mA:

			switch (sensorType)
			{
				case E::SensorType::V_0_5:

					elVal = phVal / RESISTOR_V_0_5;
					break;

			}
			break;

		default:
			break;
	}

	if (elVal < electricLowLimit || elVal > electricHighLimit)
	{
		elVal.clear();
		return elVal;
	}

	return elVal;
}

QVariant UnitsConvertor::electricToPhysical(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType)
{
	QVariant phVal;
	phVal.clear();

	if (elVal < electricLowLimit || elVal > electricHighLimit)
	{
		return phVal;
	}

	switch(unitID)
	{
		case E::ElectricUnit::V:

			switch (sensorType)
			{
				case E::SensorType::V_0_5:

					if (electricLowLimit < V_0_5_LOW_LIMIT || electricHighLimit > V_0_5_HIGH_LIMIT)
					{
						break;
					}

					phVal = elVal;
					break;

				case E::SensorType::V_m10_p10:

					if (electricLowLimit < V_m10_p10_LOW_LIMIT || electricHighLimit > V_m10_p10_HIGH_LIMIT)
					{
						break;
					}

					phVal = elVal;
					break;
			}

			break;

		case E::ElectricUnit::mA:

			switch (sensorType)
			{
				case E::SensorType::V_0_5:

					phVal = elVal * RESISTOR_V_0_5;
					break;
			}
			break;

		default:
			break;
	}

	return phVal;
}

