#pragma once

#include <QWidget>

#include "../TuningService/TuningService.h"


class QLineEdit;

class AnalogSignalSetter : public QWidget
{
	Q_OBJECT
public:
	explicit AnalogSignalSetter(QString signalId, double lowLimit, double highLimit, Tuning::TuningService* service, QWidget *parent = 0);

public slots:
	void updateValue();
	void setCurrentValue(QString appSignalID, double value, double lowLimit, double highLimit, bool validity);
	void applyNewValue();
	void changeNewValue(double newValue);	//In case if scrollbar used

private:
	QString m_signalId;
	double m_lowLimit;
	double m_highLimit;
	double m_lastSentValue;
	bool m_validity = false;
	Tuning::TuningService* m_service;
	QLineEdit* m_input;
	QLineEdit* m_currentValue;
};

