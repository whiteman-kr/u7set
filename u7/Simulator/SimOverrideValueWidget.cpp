#include "SimOverrideValueWidget.h"

std::map<QString, SimOverrideValueWidget*> SimOverrideValueWidget::m_openedDialogs;

SimOverrideValueWidget::SimOverrideValueWidget(const Sim::OverrideSignalParam& signal, Sim::Simulator* simulator, QWidget* parent) :
	QDialog(parent),
	m_signal(signal),
	m_simulator(simulator)
{
	assert(m_simulator);

	// --
	//
	setWindowTitle(tr("Override %1").arg(m_signal.m_appSignalId));

	setWindowFlag(Qt::WindowContextHelpButtonHint, false);
	setWindowFlag(Qt::WindowMinimizeButtonHint, false);
	setWindowFlag(Qt::WindowMaximizeButtonHint, false);


	// CustomSignalID/AppSignalID
	//
	QLabel* customSiganIdLabel = new QLabel(m_signal.m_customSignalId, this);
	customSiganIdLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

	QLabel* appSiganIdLabel = new QLabel(m_signal.m_appSignalId, this);
	appSiganIdLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

	// Caption
	//
	QLabel* captionLabel = new QLabel(m_signal.m_caption, this);
	captionLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

	// LM ID
	//
//	QLabel* lmIdLabel = new QLabel(m_signal.m_equipmentId, this);
//	lmIdLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

	// Type/Format
	//
	QLabel* typeLabel = new QLabel(this);

	QString text;

	if (m_signal.m_signalType == E::SignalType::Discrete)
	{
		text = E::valueToString<E::SignalType>(m_signal.m_signalType);
	}

	if (m_signal.m_signalType == E::SignalType::Analog)
	{
		text = QString("%1 (%2)")
				.arg(E::valueToString<E::SignalType>(m_signal.m_signalType))
				.arg(E::valueToString<E::AnalogAppSignalFormat>(m_signal.m_dataFormat));
	}

	if (m_signal.m_signalType == E::SignalType::Bus)
	{
		text = QString("%1")
			   .arg(E::valueToString<E::SignalType>(m_signal.m_signalType));
	}

	typeLabel->setText(text);


	// --
	//
	QGridLayout* layout = new QGridLayout;
	setLayout(layout);

	layout->addWidget(new QLabel("CustomSignalID:"), 0, 0);
	layout->addWidget(customSiganIdLabel, 0, 1);

	layout->addWidget(new QLabel("AppSignalID:"), 1, 0);
	layout->addWidget(appSiganIdLabel, 1, 1);

	layout->addWidget(new QLabel("Caption:"), 2, 0);
	layout->addWidget(captionLabel, 2, 1);

	//layout->addWidget(new QLabel("LogicModule:"), 3, 0);
	//layout->addWidget(lmIdLabel, 3, 1);

	layout->addWidget(new QLabel("Type:"), 3, 0);
	layout->addWidget(typeLabel, 3, 1);

	//layout->addWidget(new QLabel("Value:"), 5, 0);
	//layout->addWidget(edit, 5, 1);

	QWidget* stretch = new QWidget;
	stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	layout->addWidget(stretch, 6, 0, 1, 2);

	//layout->addWidget(buttonBox, 6, 0, 1, 2);

	// --
	//
	connect(m_simulator, &Sim::Simulator::projectUpdated, this, &SimOverrideValueWidget::projectUpdated);
	connect(&m_simulator->overrideSignals(), &Sim::OverrideSignals::signalsChanged, this, &SimOverrideValueWidget::overrideSignalsChaged);

	m_openedDialogs[m_signal.m_appSignalId] = this;

	return;
}

SimOverrideValueWidget::~SimOverrideValueWidget()
{
	m_openedDialogs.erase(m_signal.m_appSignalId);
}


bool SimOverrideValueWidget::showDialog(const Sim::OverrideSignalParam& signal, Sim::Simulator* simulator, QWidget* parent)
{
	SimOverrideValueWidget* w = nullptr;

	auto it = m_openedDialogs.find(signal.m_appSignalId);
	if (it == m_openedDialogs.end())
	{
		w = new SimOverrideValueWidget(signal, simulator, parent);
		w->show();	// show as modalless dialog
		w->layout()->update();
	}
	else
	{
		w = it->second;
	}

	assert(w);
	if (w->isHidden() == true)
	{
		w->setVisible(true);
	}

	if (w->isActiveWindow() == false)
	{
		w->activateWindow();
	}

	w->raise();

	return true;
}

void SimOverrideValueWidget::resizeEvent(QResizeEvent* event)
{
	if (isVisible() == true)
	{
		m_prevVisibleSize = event->size();
	}
	else
	{
		if (m_prevVisibleSize.isValid() == true)
		{
			resize(m_prevVisibleSize);
		}
	}

	return;
}

void SimOverrideValueWidget::projectUpdated()
{
	if (m_simulator->isLoaded() == true)
	{
		std::optional<Sim::OverrideSignalParam> s = m_simulator->overrideSignals().overrideSignal(appSignalId());

		if (s.has_value() == true)
		{
			m_signal = s.value();
		}
		else
		{
			// Signal has been removed?
			//
			setAttribute(Qt::WA_DeleteOnClose, true);
			close();
		}
	}

	return;
}

void SimOverrideValueWidget::overrideSignalsChaged(QStringList /*appSignalIds*/)	// Added or deleted signal
{
	if (m_simulator->isLoaded() == true)
	{
		std::optional<Sim::OverrideSignalParam> s = m_simulator->overrideSignals().overrideSignal(appSignalId());

		if (s.has_value() == false)
		{
			// Signal has been removed
			//
			setAttribute(Qt::WA_DeleteOnClose, true);
			close();
		}
	}

	return;
}


QString SimOverrideValueWidget::appSignalId() const
{
	return m_signal.m_appSignalId;
}

const Sim::OverrideSignalParam& SimOverrideValueWidget::signal() const
{
	return m_signal;
}

Sim::Simulator* SimOverrideValueWidget::simulator()
{
	return m_simulator;
}

const Sim::Simulator* SimOverrideValueWidget::simulator() const
{
	return m_simulator;
}

Sim::OverrideSignals& SimOverrideValueWidget::overrideSignals()
{
	return m_simulator->overrideSignals();
}

const Sim::OverrideSignals& SimOverrideValueWidget::overrideSignals() const
{
	return m_simulator->overrideSignals();
}
