#ifndef ANALOGSIGNALSETTER_H
#define ANALOGSIGNALSETTER_H

#include <QWidget>

class TuningService;
class QLineEdit;

class AnalogSignalSetter : public QWidget
{
	Q_OBJECT
public:
	explicit AnalogSignalSetter(QString signalId, double highLimit, TuningService* service, QWidget *parent = 0);

signals:

public slots:
	void updateValue();
	void setCurrentValue(QString appSignalID, double value);
	void setNewValue();

private:
	QString m_signalId;
	double m_highLimit;
	TuningService* m_service;
	QLineEdit* m_input;
	QLineEdit* m_currentValue;
};

#endif // ANALOGSIGNALSETTER_H
