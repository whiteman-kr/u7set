#ifndef TUNINGCONTROLLER_H
#define TUNINGCONTROLLER_H

#include <QObject>
#include <QMessageBox>

#include "../lib/AppSignal.h"
#include "../lib/Tuning/TuningSignalState.h"

class TuningController : public QObject
{
	Q_OBJECT
public:

	AppSignalParam signalParam(const QString &appSignalID, bool *ok);
	TuningSignalState signalState(const QString &appSignalID, bool *ok);

	Q_INVOKABLE QVariant param(const QString &appSignalID); // If no signal with specified appSignalID found, QVariant is null
	Q_INVOKABLE QVariant state(const QString &appSignalID);	// If no signal with specified appSignalID found, QVariant is null

	Q_INVOKABLE bool writeValue(QString appSignalID, float value);

signals:

	void signal_writeValue(QString appSignalID, float value, bool* ok);
	void signal_getParam(QString appSignalID, AppSignalParam* result, bool* ok);
	void signal_getState(QString appSignalID, TuningSignalState* result, bool* ok);
};


#endif
