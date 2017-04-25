#ifndef TUNINGCONTROLLER_H
#define TUNINGCONTROLLER_H

#include <QObject>
#include <QMessageBox>

#include "../lib/Signal.h"
#include "../lib/Tuning/TuningSignalState.h"

class TuningController : public QObject
{
	Q_OBJECT
public:

	void lock();

	void unlock();

	Q_INVOKABLE bool exists(QString appSignalID);
	Q_INVOKABLE QVariant valid(QString appSignalID);
	Q_INVOKABLE QVariant analog(QString appSignalID);

	Q_INVOKABLE QVariant highLimit(QString appSignalID);
	Q_INVOKABLE QVariant lowLimit(QString appSignalID);

	Q_INVOKABLE QVariant decimalPlaces(QString appSignalID);

	Q_INVOKABLE QVariant value(QString appSignalID);
	Q_INVOKABLE bool setValue(QString appSignalID, float value);

	Q_INVOKABLE int showMessageBox(QString title, QString text, int icon, int buttons);

	Q_INVOKABLE int showWarningMessageBox(QString text);
	Q_INVOKABLE int showErrorMessageBox(QString text);
	Q_INVOKABLE int showInfoMessageBox(QString text);

	Signal signalParam(const QString &appSignalID, bool *ok);
	TuningSignalState signalState(const QString &appSignalID, bool *ok);

signals:

	void signal_exists(QString appSignalID, bool* result, bool* ok);
	void signal_valid(QString appSignalID, bool* result, bool* ok);
	void signal_analog(QString appSignalID, bool* result, bool* ok);

	void signal_highLimit(QString appSignalID, float* result, bool* ok);
	void signal_lowLimit(QString appSignalID, float* result, bool* ok);

	void signal_decimalPlaces(QString appSignalID, float* result, bool* ok);

	void signal_value(QString appSignalID, float* result, bool* ok);
	void signal_setValue(QString appSignalID, float value, bool* ok);

	void signal_getParam(QString appSignalID, Signal& result, bool* ok);
	void signal_getState(QString appSignalID, TuningSignalState& result, bool* ok);
};


#endif
