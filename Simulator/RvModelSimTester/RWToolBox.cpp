#include "RWToolBox.h"
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QString>

RWToolBox::RWToolBox(QWidget* parent) : QWidget(parent)
{
    QGroupBox* box = new QGroupBox("R/W Read and Write Tool", this);

    signalIdEdit = new QLineEdit(this);
    readButton = new QPushButton("Read", this);


    writeValueEdit = new QLineEdit(this);
    writeButton = new QPushButton("Write", this);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow("Signal ID:", signalIdEdit);

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

    setFixedSize(350, 160);

    connect(readButton, &QPushButton::clicked, this, &RWToolBox::onReadClicked);
    connect(writeButton, &QPushButton::clicked, this, &RWToolBox::onWriteClicked);
}

void RWToolBox::onReadClicked()
{
	QString signalId = signalIdEdit->text();
	QString value = "(some value)"; // Module that can read value from existanse signal
	emit readAction("Read sygnal " + signalId + ": " + value);
}

void RWToolBox::onWriteClicked()
{
	QString signalId = signalIdEdit->text();
	QString value = writeValueEdit->text(); // Module that can be wtire value to existanse signal
	emit writeAction("New value of " + signalId + " is " + value);
}
