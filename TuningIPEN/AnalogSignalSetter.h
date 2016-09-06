#pragma once

#include <QWidget>

#include "TuningIPENService.h"


class QLineEdit;

class AnalogSignalSetter : public QWidget
{
	Q_OBJECT
public:
	explicit AnalogSignalSetter(QString signalId, double lowLimit, double highLimit, TuningIPEN::TuningIPENService* service, QWidget *parent = 0);

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
	TuningIPEN::TuningIPENService* m_service = nullptr;
	QLineEdit* m_input;
	QLineEdit* m_currentValue;
};

