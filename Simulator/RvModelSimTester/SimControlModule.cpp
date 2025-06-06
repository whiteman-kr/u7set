#include "SimControlModule.h"
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QLineEdit>
#include <QFormLayout>
#include <QVBoxLayout>


SimControlModule::SimControlModule(QWidget* parent) :
	QWidget(parent)
{
	QGroupBox* box = new QGroupBox("Simulator action", this);
	startButton = new QPushButton("Start sim", this);
	stopButton = new QPushButton("Stop sim", this);
	pauseButton = new QPushButton("Pause", this);
	statusBar = new QLabel("Mode: Run", this);
	statusBar->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	statusBar->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);


	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(startButton);
	layout->addWidget(stopButton);
	layout->addWidget(pauseButton);
	layout->addStretch();
	layout->addWidget(statusBar);
	layout->addStretch();

	box->setLayout(layout);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(box);
	mainLayout->addStretch();

	setLayout(mainLayout);
	

	connect(startButton, &QPushButton::clicked, this, &SimControlModule::onStartClicked);
	connect(stopButton, &QPushButton::clicked, this, &SimControlModule::onStopClicked);
	connect(pauseButton, &QPushButton::clicked, this, &SimControlModule::onPauseClicked);
}

void SimControlModule::onStartClicked()
{
	statusBar->setText("Mode: Run");
	emit simAction("Start sim button pressed (Run)");
}

void SimControlModule::onStopClicked()
{
	statusBar->setText("Mode: Stop");
	emit simAction("Stop sim button pressed (Stop)");
}

void SimControlModule::onPauseClicked()
{
	statusBar->setText("Mode: Pause");
	emit simAction("Pause button pressed (Pause)");
}