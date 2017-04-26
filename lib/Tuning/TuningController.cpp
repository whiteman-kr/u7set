#include "TuningController.h"

#include <QMessageBox>

Q_DECLARE_METATYPE(TuningSignalState)

AppSignalParam TuningController::signalParam(const QString& appSignalID, bool* ok)
{
	bool result = true;

	AppSignalParam signal;

	emit signal_getParam(appSignalID, &signal, &result);

	if (ok != nullptr)
	{
		*ok = result;
	}

	if (result == false)
	{
		return AppSignalParam();
	}

	return signal;
}

TuningSignalState TuningController::signalState(const QString& appSignalID, bool* ok)
{
	bool result = true;

	TuningSignalState state;

	emit signal_getState(appSignalID, &state, &result);

	if (ok != nullptr)
	{
		*ok = result;
	}

	if (result == false)
	{
		return TuningSignalState();
	}

	return state;
}

QVariant TuningController::param(const QString &appSignalID)
{
	bool ok = true;

	QVariant result = QVariant::fromValue(signalParam(appSignalID, &ok));

	if (ok == false)
	{
		return QVariant();
	}

	return result;

}

QVariant TuningController::state(const QString &appSignalID)
{

	bool ok = true;

	QVariant result = QVariant::fromValue(signalState(appSignalID, &ok));

	if (ok == false)
	{
		return QVariant();
	}

	return result;
}

bool TuningController::writeValue(QString appSignalID, float value)
{
	bool ok = true;

	emit signal_writeValue(appSignalID, value, &ok);

	if (ok == false)
	{
		return false;
	}

	return true;
}
