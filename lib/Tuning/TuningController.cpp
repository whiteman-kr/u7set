#include "TuningController.h"

#include <QMessageBox>

void TuningController::lock()
{

}

void TuningController::unlock()
{

}

bool TuningController::exists(QString appSignalID)
{

	bool result = false;
	bool ok = true;

	emit signal_exists(appSignalID, &result, &ok);

	if (ok == false)
	{
		return false;
	}

	return result;
}

QVariant TuningController::valid(QString appSignalID)
{
	bool result = false;
	bool ok = true;

	emit signal_valid(appSignalID, &result, &ok);

	if (ok == false)
	{
		return QVariant();
	}

	return result;
}

QVariant TuningController::analog(QString appSignalID)
{
	bool result = false;
	bool ok = true;

	emit signal_analog(appSignalID, &result, &ok);

	if (ok == false)
	{
		return QVariant();
	}

	return result;
}

QVariant TuningController::highLimit(QString appSignalID)
{
	float result = false;
	bool ok = true;

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
	bool ok = true;

	emit signal_lowLimit(appSignalID, &result, &ok);

	if (ok == false)
	{
		return QVariant();
	}

	return result;
}

QVariant TuningController::decimalPlaces(QString appSignalID)
{
	float result = false;
	bool ok = true;

	emit signal_decimalPlaces(appSignalID, &result, &ok);

	if (ok == false)
	{
		return QVariant();
	}

	return result;
}

QVariant TuningController::value(QString appSignalID)
{
	float result = false;
	bool ok = true;

	emit signal_value(appSignalID, &result, &ok);

	if (ok == false)
	{
		return QVariant();
	}

	return result;
}

bool TuningController::setValue(QString appSignalID, float value)
{
	bool ok = true;

	emit signal_setValue(appSignalID, value, &ok);

	if (ok == false)
	{
		return false;
	}

	return true;
}

int TuningController::showMessageBox(QString title, QString text, int icon, int buttons)
{
	QMessageBox mb((QMessageBox::Icon)icon, title, text, (QMessageBox::StandardButton)buttons);
	return mb.exec();
}

int TuningController::showWarningMessageBox(QString text)
{
	return showMessageBox(tr("Warning"), text, QMessageBox::Warning, QMessageBox::Ok);
}

int TuningController::showErrorMessageBox(QString text)
{
	return showMessageBox(tr("Error"), text, QMessageBox::Critical, QMessageBox::Ok);
}

int TuningController::showInfoMessageBox(QString text)
{
	return showMessageBox(tr("Info"), text, QMessageBox::Information, QMessageBox::Ok);
}

AppSignalParam TuningController::signalParam(const QString& appSignalID, bool* ok)
{
	bool result = true;

	AppSignalParam signal;

	emit signal_getParam(appSignalID, signal, &result);

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

	emit signal_getState(appSignalID, state, &result);

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
