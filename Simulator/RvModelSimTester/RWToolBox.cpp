#include "RWToolBox.h"
#include <QCompleter>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
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

	signalIdEdit = new QLineEdit(this);


	readButton = new QPushButton("Read", this);
	writeValueEdit = new QLineEdit(this);
	writeButton = new QPushButton("Write", this);

	QFormLayout* formLayout = new QFormLayout;
	formLayout->addRow("Signal ID:", signalIdEdit);


	QString lastSignalId = "#SYSTEMID_RACKID_CH01_MD02_CTRLIN_IN01A";
	signalIdModel = new QStringListModel(this);
	signalIdModel->setStringList({lastSignalId});

	QSettings settings;
	QString lastSaved = settings.value("lastSignalId").toString();
	if (!lastSaved.isEmpty())
	{
		signalIdModel->setStringList({lastSaved});
	}

	signalIdCompleter = new QCompleter(signalIdModel, this);
	signalIdCompleter->setCompletionMode(QCompleter::InlineCompletion);
	signalIdCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	signalIdEdit->setCompleter(signalIdCompleter);


	QHBoxLayout* readLayout = new QHBoxLayout;
	readLayout->addStretch();
	readLayout->addWidget(readButton);
	formLayout->addRow("", readLayout);


	QHBoxLayout* writeLayout = new QHBoxLayout;
	writeLayout->addWidget(writeValueEdit);
	writeLayout->addWidget(writeButton);
	formLayout->addRow("Value:", writeLayout);

	box->setLayout(formLayout);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(box);
	mainLayout->addStretch();
	setLayout(mainLayout);

	resize(350, 160);

	connect(signalIdEdit,
			&QLineEdit::editingFinished,
			[this]()
			{
				QString currentText = signalIdEdit->text();
				if (!currentText.isEmpty())
				{
					QSettings settings;
					settings.setValue("lastSignalId", currentText);

					signalIdModel->setStringList({currentText});
				}
			});

	connect(readButton, &QPushButton::clicked, this, &RWToolBox::onReadClicked);
	connect(writeButton, &QPushButton::clicked, this, &RWToolBox::onWriteClicked);
}

void RWToolBox::onReadClicked()
{
	QString signalId = signalIdEdit->text();
	requestRead(signalId);
}

void RWToolBox::onWriteClicked()
{
	QString signalId = signalIdEdit->text();
	QString value = writeValueEdit->text();
	requestWrite(signalId, value);
}
