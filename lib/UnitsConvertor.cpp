#include "UnitsConvertor.h"


// UnitsConvertResult

UnitsConvertResult::UnitsConvertResult()
{
}

UnitsConvertResult::UnitsConvertResult(double result)
{
	m_ok = true;
	m_result = result;
}

UnitsConvertResult::UnitsConvertResult(UnitsConvertResultError errorCode, const QString& errorMessage)
{
	if (errorCode != UnitsConvertResultError::Generic)
	{
		assert(false);
		errorCode = UnitsConvertResultError::Generic;
	}

	m_ok = false;

	m_errorCode = errorCode;

	m_errorMessage = errorMessage;
}

UnitsConvertResult::UnitsConvertResult(UnitsConvertResultError errorCode, double expectedLowValidRange, double expectedHighValidRange)
{
	if (errorCode != UnitsConvertResultError::LowLimitOutOfRange &&
			errorCode != UnitsConvertResultError::HighLimitOutOfRange)
	{
		assert(false);
		errorCode = UnitsConvertResultError::Generic;
		m_errorMessage = QString("Internal error, wrong error code");
	}

	m_ok = false;

	m_errorCode = errorCode;

	m_expectedLowValidRange = expectedLowValidRange;
	m_expectedHighValidRange = expectedHighValidRange;

}

bool UnitsConvertResult::ok() const
{
	return m_ok;
}

int UnitsConvertResult::errorCode() const
{
	return static_cast<int>(m_errorCode);
}

double UnitsConvertResult::toDouble() const
{
	return m_result;
}

QString UnitsConvertResult::errorMessage() const
{
	return m_errorMessage;
}

double UnitsConvertResult::expectedLowValidRange() const
{
	return m_expectedLowValidRange;
}

double UnitsConvertResult::expectedHighValidRange() const
{
	return m_expectedHighValidRange;
}

// UnitsConvertor

UnitsConvertor::UnitsConvertor(QObject *parent)
	: QObject(parent)
{
	static bool UnitsConvertResultRegistered = false;
	if (UnitsConvertResultRegistered == false)
	{
		UnitsConvertResultRegistered = true;
		qRegisterMetaType<UnitsConvertResult>();
	}
}

UnitsConvertor::~UnitsConvertor()
{
}

QVariant UnitsConvertor::physicalToElectric(double phVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType)
{
	QVariant elVal;

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

UnitsConvertResult UnitsConvertor::electricToPhysical(double elVal, double electricLowLimit, double electricHighLimit, int unitID, int sensorType)
{
	if (elVal < electricLowLimit || elVal > electricHighLimit)
	{
		return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Function argument is out of range"));
	}

	switch(unitID)
	{
		case E::ElectricUnit::V:

			switch (sensorType)
			{
				case E::SensorType::V_0_5:

					if (electricLowLimit < V_0_5_LOW_LIMIT || electricLowLimit > V_0_5_HIGH_LIMIT)
					{
						return UnitsConvertResult(UnitsConvertResultError::LowLimitOutOfRange, V_0_5_LOW_LIMIT, V_0_5_HIGH_LIMIT);
					}
					if (electricHighLimit < V_0_5_LOW_LIMIT || electricHighLimit > V_0_5_HIGH_LIMIT)
					{
						return UnitsConvertResult(UnitsConvertResultError::HighLimitOutOfRange, V_0_5_LOW_LIMIT, V_0_5_HIGH_LIMIT);
					}

					return UnitsConvertResult(elVal);

				case E::SensorType::V_m10_p10:

					if (electricLowLimit < V_m10_p10_LOW_LIMIT || electricLowLimit > V_m10_p10_HIGH_LIMIT)
					{
						return UnitsConvertResult(UnitsConvertResultError::LowLimitOutOfRange, V_m10_p10_LOW_LIMIT, V_m10_p10_HIGH_LIMIT);
					}
					if (electricHighLimit < V_m10_p10_LOW_LIMIT || electricHighLimit > V_m10_p10_HIGH_LIMIT)
					{
						return UnitsConvertResult(UnitsConvertResultError::HighLimitOutOfRange, V_m10_p10_LOW_LIMIT, V_m10_p10_HIGH_LIMIT);
					}

					return UnitsConvertResult(elVal);

			default:
				assert(false);
				return UnitsConvertResult(UnitsConvertResultError::Generic, tr("Unknown SensorType for V"));
			}

			break;

		case E::ElectricUnit::mA:

			switch (sensorType)
			{
				case E::SensorType::V_0_5:

					return  UnitsConvertResult(elVal * RESISTOR_V_0_5);

			default:
				assert(false);
				return  UnitsConvertResult(UnitsConvertResultError::Generic, tr("Unknown SensorType for mA"));
			}

		default:
			assert(false);
	}

	return  UnitsConvertResult(UnitsConvertResultError::Generic, tr("Unknown unitID"));
}

