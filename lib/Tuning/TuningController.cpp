#include "TuningController.h"


void TuningController::lock()
{

}

void TuningController::unlock()
{

}

QVariant TuningController::exists(QString appSignalID)
{
	bool result = false;
	bool ok = false;

	emit signal_exists(appSignalID, &result, &ok);

	if (ok == false)
	{
		return QVariant();
	}

	return result;
}

QVariant TuningController::valid(QString appSignalID)
{
	bool result = false;
	bool ok = false;

	emit signal_valid(appSignalID, &result, &ok);

	if (ok == false)
	{
		return QVariant();
	}

	return result;
}

QVariant TuningController::value(QString appSignalID)
{
	float result = false;
	bool ok = false;

	emit signal_value(appSignalID, &result, &ok);

	if (ok == false)
	{
		return QVariant();
	}

	return result;
}

bool TuningController::setValue(QString appSignalID, float value)
{
	bool ok = false;

	emit signal_setValue(appSignalID, value, &ok);

	if (ok == false)
	{
		return false;
	}

	return true;
}

QVariant TuningController::highLimit(QString appSignalID)
{
	float result = false;
	bool ok = false;

	emit signal_highLimit(appSignalID, &result, &ok);

	if (ok == false)
	{
		return QVariant();
	}

	return result;
}

QVariant TuningController::lowLimit(QString appSignalID)
{
	float result = false;
	bool ok = false;

	emit signal_lowLimit(appSignalID, &result, &ok);

	if (ok == false)
	{
		return QVariant();
	}

	return result;
}
