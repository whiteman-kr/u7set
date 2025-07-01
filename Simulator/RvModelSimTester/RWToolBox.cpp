#include "RWToolBox.h"

#include <QCompleter>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QString>
#include <QStringListModel>
#include <QVBoxLayout>

RWToolBox::RWToolBox(QWidget* parent) :
	QWidget(parent)
{
	QGroupBox* box = new QGroupBox("", this);

	m_signalIdEdit = new QLineEdit(this);


	m_readButton = new QPushButton(tr("Read"), this);
	m_writeValueEdit = new QLineEdit(this);
	m_writeButton = new QPushButton(tr("Write"), this);

	QFormLayout* formLayout = new QFormLayout;
	formLayout->addRow(tr("Signal ID:"), m_signalIdEdit);


	QString lastSignalId = "#SYSTEMID_RACKID_CH01_MD02_CTRLIN_IN01A";
	m_signalIdModel = new QStringListModel(this);
	m_signalIdModel->setStringList({lastSignalId});

	QSettings settings;
	QString lastSaved = settings.value("lastSignalId").toString();
	if (!lastSaved.isEmpty())
	{
		m_signalIdModel->setStringList({lastSaved});
	}

	m_signalIdCompleter = new QCompleter(m_signalIdModel, this);
	m_signalIdCompleter->setCompletionMode(QCompleter::InlineCompletion);
	m_signalIdCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	m_signalIdEdit->setCompleter(m_signalIdCompleter);


	QHBoxLayout* readLayout = new QHBoxLayout;
	readLayout->addStretch();
	readLayout->addWidget(m_readButton);
	formLayout->addRow("", readLayout);


	QHBoxLayout* writeLayout = new QHBoxLayout;
	writeLayout->addWidget(m_writeValueEdit);
	writeLayout->addWidget(m_writeButton);
	formLayout->addRow(tr("Value:"), writeLayout);

	box->setLayout(formLayout);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(box);
	mainLayout->addStretch();
	setLayout(mainLayout);

	connect(m_signalIdEdit,
			&QLineEdit::editingFinished,
			[this]()
			{
				QString currentText = m_signalIdEdit->text();
				if (currentText.isEmpty() == false)
				{
					QSettings settings;
					settings.setValue("lastSignalId", currentText);

					m_signalIdModel->setStringList({currentText});
				}
			});

	connect(m_readButton, &QPushButton::clicked, this, &RWToolBox::onReadClicked);
	connect(m_writeButton, &QPushButton::clicked, this, &RWToolBox::onWriteClicked);
}

void RWToolBox::onReadClicked()
{
	QString signalId = m_signalIdEdit->text();
	requestRead(signalId);
}

void RWToolBox::onWriteClicked()
{
	QString signalId = m_signalIdEdit->text();
	QString value = m_writeValueEdit->text();
	requestWrite(signalId, value);
}
