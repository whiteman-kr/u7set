#include "AnalogSignalSetter.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDoubleValidator>
#include <QMessageBox>


AnalogSignalSetter::AnalogSignalSetter(QString signalId, double lowLimit, double highLimit, Tuning::TuningService* service, QWidget *parent) :
	QWidget(parent),
	m_signalId(signalId),
	m_lowLimit(lowLimit),
	m_highLimit(highLimit),
	m_lastSentValue(qQNaN()),
	m_service(service),
	m_input(new QLineEdit(this)),
	m_currentValue(new QLineEdit(this))
{
	QHBoxLayout* hl = new QHBoxLayout;

	m_input->setPlaceholderText("Enter new value");
	m_input->setValidator(new QDoubleValidator(this));
	connect(m_input, &QLineEdit::returnPressed, this, &AnalogSignalSetter::setNewValue);
	hl->addWidget(m_input);

	QPushButton* applyButton = new QPushButton("Apply >>", this);
	connect(applyButton, &QPushButton::clicked, this, &AnalogSignalSetter::setNewValue);
	hl->addWidget(applyButton);

	m_currentValue->setReadOnly(true);
	hl->addWidget(m_currentValue);

	setLayout(hl);
}

void AnalogSignalSetter::updateValue()
{
	m_service->getSignalState(m_signalId);
}

void AnalogSignalSetter::setCurrentValue(QString appSignalID, double value, double, double, bool validity)
{
	if (appSignalID == m_signalId)
	{
		m_validity = validity;
		if (m_validity == false)
		{
			m_currentValue->setText("???");
		}
		else
		{
			m_currentValue->setText(QString::number(value));
		}

		if (validity == true && qAbs(value - m_lastSentValue) < std::numeric_limits<float>::epsilon())
		{
			m_input->clear();
			m_lastSentValue = qQNaN();
		}
	}
}

void AnalogSignalSetter::setNewValue()
{
	bool ok = false;
	double newValue = m_input->text().replace(',', '.').toDouble(&ok);
	if (!ok)
	{
		QMessageBox::critical(this, "Not valid input", "Please, enter valid float pointing number");
		return;
	}

	if (newValue < 0 || newValue > m_highLimit)
	{
		QMessageBox::critical(this, "Not valid input", QString("Please, enter number between 0 and %1").arg(m_highLimit));
		return;
	}

	auto reply = QMessageBox::question(this, "Confirmation", QString("Are you sure you want change <b>%1</b> signal value to <b>%2</b>?")
									   .arg(m_signalId).arg(newValue), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

	if (reply == QMessageBox::No)
	{
		return;
	}

	m_service->setSignalState(m_signalId, newValue);
	m_lastSentValue = newValue;
}

void AnalogSignalSetter::changeNewValue(double newValue)
{
	m_input->setText(QString::number(newValue));
}

