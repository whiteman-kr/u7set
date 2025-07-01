#include "SimControlModule.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "PacketsMessages.h"

SimControlModule::SimControlModule(QWidget* parent) :
	QWidget(parent)
{
	QGroupBox* box = new QGroupBox("", this);
	m_startButton = new QPushButton(tr("Start Sim"), this);
	m_stopButton = new QPushButton(tr("Stop Sim"), this);
	m_pauseButton = new QPushButton(tr("Pause"), this);
	m_resumeButton = new QPushButton(tr("Resume"), this);
	m_statusBar = new QLabel(tr("Mode: ???"), this);
	m_statusBar->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	m_statusBar->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);


	QHBoxLayout* layout = new QHBoxLayout;
	layout->addWidget(m_startButton);
	layout->addWidget(m_stopButton);
	layout->addWidget(m_pauseButton);
	layout->addWidget(m_resumeButton);
	layout->addStretch();
	layout->addWidget(m_statusBar);
	layout->addStretch();

	box->setLayout(layout);

	QVBoxLayout* mainLayout = new QVBoxLayout;
	mainLayout->addWidget(box);
	mainLayout->addStretch();

	setLayout(mainLayout);

	connect(m_startButton, &QPushButton::clicked, this, &SimControlModule::onStartClicked);
	connect(m_stopButton, &QPushButton::clicked, this, &SimControlModule::onStopClicked);
	connect(m_pauseButton, &QPushButton::clicked, this, &SimControlModule::onPauseClicked);
	connect(m_resumeButton, &QPushButton::clicked, this, &SimControlModule::onResumeClicked);
}

void SimControlModule::onSimStateReady(int errorCode, int stateCode)
{
	if (errorCode == ErrorCode::Success)
	{
		m_statusBar->setText(QString(tr("Mode: %1")).arg(simStateToString(static_cast<SimulatorStateCode>(stateCode))));
	}
	else
	{
		m_statusBar->setText(QString(tr("Error: %1")).arg(errorCodeToString(static_cast<ErrorCode>(errorCode))));
	}
}

void SimControlModule::onStartClicked()
{
	simControlMode("Start");
	emit simAction(tr("Start sim button pressed"));
}

void SimControlModule::onStopClicked()
{
	simControlMode("Stop");
	emit simAction(tr("Stop sim button pressed"));
}

void SimControlModule::onPauseClicked()
{
	simControlMode("Pause");
	emit simAction(tr("Pause button pressed "));
}

void SimControlModule::onResumeClicked()
{
	simControlMode("Resume");
	emit simAction(tr("Resume button pressed"));
}