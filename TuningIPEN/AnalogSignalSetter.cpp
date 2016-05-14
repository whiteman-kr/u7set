#include "AnalogSignalSetter.h"
#include "../TuningService/TuningService.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDoubleValidator>
#include <QMessageBox>

AnalogSignalSetter::AnalogSignalSetter(QString signalId, double highLimit, TuningService* service, QWidget *parent) :
	QWidget(parent),
	m_signalId(signalId),
	m_highLimit(highLimit),
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

void AnalogSignalSetter::setCurrentValue(QString appSignalID, double value, double lowLimit, double highLimit, bool validity)
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

		if (m_currentValue->text() == m_input->text())
		{
			m_input->clear();
		}
	}
}

void AnalogSignalSetter::setNewValue()
{
	bool ok = false;
	double newValue = m_input->text().toDouble(&ok);
	if (!ok)
	{
		QMessageBox::critical(nullptr, "Not valid input", "Please, enter valid float pointing number");
		return;
	}

	if (newValue < 0 || newValue > m_highLimit)
	{
		QMessageBox::critical(nullptr, "Not valid input", QString("Please, enter number between 0 and %1").arg(m_highLimit));
		return;
	}

	auto reply = QMessageBox::question(nullptr, "Confirmation", QString("Are you sure you want change <b>%1</b> signal value to <b>%2</b>?")
									   .arg(m_signalId).arg(newValue), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

	if (reply == QMessageBox::No)
	{
		return;
	}

	m_service->setSignalState(m_signalId, newValue);
}
