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

int TuningController::showMessageBox(QString title, QString text, QMessageBox::Icon icon, QMessageBox::StandardButton buttons)
{
	QMessageBox mb(icon, title, text, buttons);
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
