#ifndef TUNINGCONTROLLER_H
#define TUNINGCONTROLLER_H

#include <QObject>

class TuningController : public QObject
{
	Q_OBJECT
public:

	void lock();

	void unlock();

	Q_INVOKABLE QVariant exists(QString appSignalID);

	Q_INVOKABLE QVariant valid(QString appSignalID);

	Q_INVOKABLE QVariant value(QString appSignalID);
	Q_INVOKABLE bool setValue(QString appSignalID, float value);

	Q_INVOKABLE QVariant highLimit(QString appSignalID);
	Q_INVOKABLE QVariant lowLimit(QString appSignalID);

signals:

	void signal_exists(QString appSignalID, bool* result, bool* ok);
	void signal_valid(QString appSignalID, bool* result, bool* ok);

	void signal_value(QString appSignalID, float* result, bool* ok);
	void signal_setValue(QString appSignalID, float value, bool* ok);

	void signal_highLimit(QString appSignalID, float* result, bool* ok);
	void signal_lowLimit(QString appSignalID, float* result, bool* ok);

};


#endif
