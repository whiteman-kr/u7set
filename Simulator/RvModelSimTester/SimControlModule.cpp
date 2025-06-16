#include "SimControlModule.h"

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "PacketsMessages.h"

SimControlModule::SimControlModule(QWidget* parent) :
	QWidget(parent)
{
	QGroupBox* box = new QGroupBox("", this);
	startButton = new QPushButton("Start sim", this);
	stopButton = new QPushButton("Stop sim", this);
	pauseButton = new QPushButton("Pause", this);
	resumeButton = new QPushButton("Resume", this);
	statusBar = new QLabel("Mode: ???", this);
	statusBar->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	statusBar->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);


	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(startButton);
	layout->addWidget(stopButton);
	layout->addWidget(pauseButton);
	layout->addWidget(resumeButton);
	layout->addStretch();
	layout->addWidget(statusBar);
	layout->addStretch();

	box->setLayout(layout);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(box);
	mainLayout->addStretch();

	setLayout(mainLayout);

	resize(350, 240);

	connect(startButton, &QPushButton::clicked, this, &SimControlModule::onStartClicked);
	connect(stopButton, &QPushButton::clicked, this, &SimControlModule::onStopClicked);
	connect(pauseButton, &QPushButton::clicked, this, &SimControlModule::onPauseClicked);
	connect(resumeButton, &QPushButton::clicked, this, &SimControlModule::onResumeClicked);
}

void SimControlModule::onSimStateReady(int errorCode, int stateCode)
{
	if (errorCode == ErrorCode::Success) {
		statusBar->setText(QString("Mode: %1").arg(simStateToString(static_cast<SimulatorStateCode>(stateCode))));
	}
	else {
		statusBar->setText(QString("Error: %1").arg(errorCodeToString(static_cast<ErrorCode>(errorCode))));
	}
	
}

void SimControlModule::onStartClicked()
{
	simControlMode("Start");
	emit simAction("Start sim button pressed");
}

void SimControlModule::onStopClicked()
{
	simControlMode("Stop");
	emit simAction("Stop sim button pressed");
}

void SimControlModule::onPauseClicked()
{
	simControlMode("Pause");
	emit simAction("Pause button pressed ");
}

void SimControlModule::onResumeClicked()
{
	simControlMode("Resume");
	emit simAction("Resume button pressed");
}