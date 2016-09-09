#pragma once

#include <QWidget>

#include "TuningIPENService.h"


class QPushButton;

class DiscreteSignalSetter : public QWidget
{
	Q_OBJECT
public:
	explicit DiscreteSignalSetter(QString signalId, QString label, TuningIPEN::TuningIPENService* service, QWidget *parent = 0);

public slots:
	void updateValue();
	void setCurrentValue(QString appSignalID, double value, double lowLimit, double highLimit, bool valid);
	void applyNewValue(bool enabled);

signals:
	void valueChanged(bool newValue);

private:
	QString m_signalId;
	TuningIPEN::TuningIPENService* m_service = nullptr;
	QPushButton* m_button = nullptr;
};

