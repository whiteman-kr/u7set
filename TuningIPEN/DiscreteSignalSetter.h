#pragma once

#include <QWidget>

#include "../TuningService/TuningService.h"


class QPushButton;

class DiscreteSignalSetter : public QWidget
{
	Q_OBJECT
public:
	explicit DiscreteSignalSetter(QString signalId, QString label, Tuning::TuningService* service, QWidget *parent = 0);

public slots:
	void updateValue();
	void setCurrentValue(QString appSignalID, double value, double lowLimit, double highLimit, bool valid);
	void applyNewValue(bool enabled);

signals:
	void valueChanged(bool newValue);

private:
	QString m_signalId;
	Tuning::TuningService* m_service;
	QPushButton* m_button;
};

