#include "DiscreteSignalSetter.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDoubleValidator>
#include <QMessageBox>


DiscreteSignalSetter::DiscreteSignalSetter(QString signalId, QString label, Tuning::TuningService* service, QWidget *parent) :
	QWidget(parent),
	m_signalId(signalId),
	m_service(service),
	m_button(new QPushButton(label, this))
{
	QHBoxLayout* hl = new QHBoxLayout;

	hl->addWidget(m_button);

	m_button->setCheckable(true);
	m_button->setStyleSheet("QPushButton:checked {color: black; background-color: red;}");
	connect(m_button, &QPushButton::clicked, this, &DiscreteSignalSetter::applyNewValue);

	setLayout(hl);
}

void DiscreteSignalSetter::updateValue()
{
	m_service->getSignalState(m_signalId);
}

void DiscreteSignalSetter::setCurrentValue(QString appSignalID, double value, double, double, bool valid)
{
	if (appSignalID == m_signalId)
	{
		m_button->setChecked(value != 0 && valid != 0);
		if (valid)
		{
			emit valueChanged(value != 0);
		}
	}
}

void DiscreteSignalSetter::applyNewValue(bool enabled)
{
	m_button->setChecked(!enabled);

	auto reply = QMessageBox::question(this, "Confirmation", QString("Are you sure you want change <b>" + m_signalId + "</b> signal value to <b>%1</b>?")
									   .arg(enabled ? "Yes" : "No"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

	if (reply == QMessageBox::No)
	{
		return;
	}

	m_service->setSignalState(m_signalId, enabled ? 1 : 0);
}

