#include "SimConnectionPage.h"
#include "../SimulatorTabPage.h"

SimConnectionPage::SimConnectionPage(SimIdeSimulator* simulator, QString connectionId, QWidget* parent) :
	SimBasePage(simulator, parent),
	m_connectionId(connectionId)
{
	assert(m_simulator);
	assert(m_connectionId.isEmpty() == false);

	// UI
	//
#if defined(Q_OS_WIN)
		QFont font = QFont("Consolas");
#else
		QFont font = QFont("Courier");
#endif

	m_connectionIdLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	m_connectionIdLabel->setFont(font);

	m_disableButton->setCheckable(true);
	m_disableButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

	m_connectionType->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	m_connectionType->setFont(font);

	m_port1Label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	m_port1Label->setFont(font);

	m_port1TxSignals->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	m_port1RxSignals->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	m_port1TxBuffer->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	m_port1RxBuffer->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

	m_port2Label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	m_port2Label->setFont(font);

	m_port2TxSignals->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	m_port2RxSignals->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	m_port2TxBuffer->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	m_port2RxBuffer->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);


	QWidget* spacer = new QWidget;
	spacer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	// --
	//
	QGridLayout* layout = new QGridLayout{this};

	int row = 0;

	layout->addWidget(m_connectionIdLabel, row++, 0, 1, 5);
	layout->addWidget(m_connectionType, row++, 0, 1, 5);

	layout->addWidget(m_disableButton, row, 0, 1, 1);
	layout->addWidget(m_stateLabel, row++, 1, 1, 1);

	layout->addWidget(m_port1Label, row++, 0, 1, 5);

	layout->addWidget(m_port1TxSignals, row, 0);
	layout->addWidget(m_port1RxSignals, row, 1);
	layout->addWidget(m_port1TxBuffer, row, 2);
	layout->addWidget(m_port1RxBuffer, row, 3);
	row++;

	layout->addWidget(m_port2Label, row++, 0, 1, 5);

	layout->addWidget(m_port2TxSignals, row, 0);
	layout->addWidget(m_port2RxSignals, row, 1);
	layout->addWidget(m_port2TxBuffer, row, 2);
	layout->addWidget(m_port2RxBuffer, row, 3);
	row++;

	layout->addWidget(spacer, row++, 0, 1, 5);

	setLayout(layout);

	// --
	//
	connect(m_simulator, &Sim::Simulator::projectUpdated, this, &SimConnectionPage::updateData);
	connect(&(m_simulator->control()), &Sim::Control::stateChanged, this, &SimConnectionPage::updateData);
	connect(&(m_simulator->connections()), &Sim::Connections::connectionStateChanged, this, &SimConnectionPage::updateConnectionState);

	connect(m_disableButton, &QPushButton::toggled, this, &SimConnectionPage::disableConnection);

	connect(m_port1TxSignals, &QPushButton::clicked, [this](){	this->showTxSignals(0); });
	connect(m_port1RxSignals, &QPushButton::clicked, [this](){	this->showRxSignals(0); });
	connect(m_port2TxSignals, &QPushButton::clicked, [this](){	this->showTxSignals(1); });
	connect(m_port2RxSignals, &QPushButton::clicked, [this](){	this->showRxSignals(1); });

	connect(m_port1TxBuffer, &QPushButton::clicked, [this](){	this->showTxBuffer(0);	});
	connect(m_port1RxBuffer, &QPushButton::clicked, [this](){	this->showRxBuffer(0);	});
	connect(m_port2TxBuffer, &QPushButton::clicked, [this](){	this->showTxBuffer(1);	});
	connect(m_port2RxBuffer, &QPushButton::clicked, [this](){	this->showRxBuffer(1);	});

	// Fill data
	//
	updateData();

	return;
}

SimConnectionPage::~SimConnectionPage()
{
}

void SimConnectionPage::disableConnection(bool disable)
{
	bool enable = !disable;
	Sim::ConnectionPtr c = m_simulator->connections().connection(m_connectionId);

	if (c == nullptr || enable == c->enabled())
	{
		return;
	}

	m_simulator->connections().enableConnection(m_connectionId, enable);

	return;
}

void SimConnectionPage::updateConnectionState(QString connectionId, bool enabled)
{
	if (connectionId != m_connectionId)
	{
		return;
	}

	bool disabled = !enabled;

	if (m_disableButton->isChecked() != disabled)
	{
		// Update push button state
		//
		m_disableButton->setChecked(disabled);
	}

	// State
	//
	QString stateText;

	if (m_simulator->isLoaded() == true)
	{
		if (m_simulator->isStopped() == true)
		{
			stateText = tr("State: Stopped");
		}
		else
		{
			stateText = tr("State: %1").arg(enabled ? tr("ok") : "Disabled");
		}
	}

	m_stateLabel->setText(stateText);

	return;
}

void SimConnectionPage::updateData()
{
	Sim::ConnectionPtr c = m_simulator->connections().connection(m_connectionId);

	if (c != nullptr)
	{
		m_connectionInfo = c->connectionInfo();
		updateConnectionState(m_connectionId, c->enabled());
	}
	else
	{
		m_connectionInfo = {};
		updateConnectionState(m_connectionId, false);
	}

	m_disableButton->setEnabled(m_simulator->isLoaded());

	m_port1TxSignals->setEnabled(m_simulator->isLoaded());
	m_port1RxSignals->setEnabled(m_simulator->isLoaded());
	m_port1TxBuffer->setEnabled(m_simulator->isLoaded());
	m_port1RxBuffer->setEnabled(m_simulator->isLoaded());
	m_port2TxSignals->setEnabled(m_simulator->isLoaded());
	m_port2RxSignals->setEnabled(m_simulator->isLoaded());
	m_port2TxBuffer->setEnabled(m_simulator->isLoaded());
	m_port2RxBuffer->setEnabled(m_simulator->isLoaded());

	m_connectionIdLabel->setText(tr("ConnectionID: <b>%1</b>")
								 .arg(m_connectionId));

	// --
	//
	if (m_connectionInfo.ID.isEmpty() == true)
	{
		m_connectionType->setText(tr("Unknown"));
		m_port1Label->setText(tr("Unknown"));
		m_port2Label->setText(tr("Unknown"));
	}
	else
	{
		m_connectionType->setText(tr("\tType:\t\t\t%1\n"
									 "\tLinkID:\t\t0x%2(%3)\n"
									 "\tManual:\t\t%4\n"
									 "\tControlDataID:\t%5\n")
									.arg(m_connectionInfo.type)
									.arg(m_connectionInfo.linkID, 4, 16, QChar('0')).arg(m_connectionInfo.linkID)
									.arg(m_connectionInfo.enableManualSettings ? "true" : "false")
									.arg(m_connectionInfo.disableDataIDControl ? "false" : "true")
								  );

		const auto& ports = m_connectionInfo.ports;

		auto formPortInfo = [](const ::ConnectionPortInfo& portInfo) -> QString
			{
				QString result = QString("\nPort%1:\t%2\n\n"
										 "\tLogicModule:\t%3\n\n"
										 "\tTxBufferAddress = 0x%4 (%5)\t TxDataSizeW = 0x%6 (%7)\t TxDataID = 0x%8\n"
										 "\tRxBufferAddress = 0x%9 (%10)\t RxDataSizeW = 0x%11 (%12)\t RxDataID = 0x%13\n"
										 )
								 .arg(portInfo.portNo)
								 .arg(portInfo.equipmentID)
								 .arg(portInfo.lmID)
								 .arg(portInfo.txBufferAbsAddr, 4, 16, QChar('0')).arg(portInfo.txBufferAbsAddr)
								 .arg(portInfo.txDataSizeW, 4, 16, QChar('0')).arg(portInfo.txDataSizeW)
								 .arg(portInfo.txDataID, 8, 16, QChar('0'))
								 .arg(portInfo.rxBufferAbsAddr, 4, 16, QChar('0')).arg(portInfo.rxBufferAbsAddr)
								 .arg(portInfo.rxDataSizeW, 4, 16, QChar('0')).arg(portInfo.rxDataSizeW)
								 .arg(portInfo.rxDataID, 8, 16, QChar('0'));

				return result;
			};

		m_port1TxSignals->setVisible(ports.size() >= 1);
		m_port1RxSignals->setVisible(ports.size() >= 1);
		m_port1TxBuffer->setVisible(ports.size() >= 1);
		m_port1RxBuffer->setVisible(ports.size() >= 1);

		if (ports.size() >= 1)
		{
			m_port1Label->setText(formPortInfo(ports[0]));

			m_port1TxSignals->setText(tr("Port%1 TX Signals (%2)")
										.arg(ports[0].portNo)
										.arg(ports[0].txSignals.size()));

			m_port1RxSignals->setText(tr("Port%1 RX Signals (%2)")
										.arg(ports[0].portNo)
										.arg(ports[0].rxSignals.size()));

			m_port1TxBuffer->setText(tr("Port%1 TX Buffer Snapshot").arg(ports[0].portNo));
			m_port1RxBuffer->setText(tr("Port%1 RX Buffer Snapshot").arg(ports[0].portNo));
		}
		else
		{
			m_port1Label->setText({});
		}


		m_port2TxSignals->setVisible(ports.size() >= 2);
		m_port2RxSignals->setVisible(ports.size() >= 2);
		m_port2TxBuffer->setVisible(ports.size() >= 2);
		m_port2RxBuffer->setVisible(ports.size() >= 2);

		if (ports.size() >= 2)
		{
			m_port2Label->setText(formPortInfo(ports[1]));

			m_port2TxSignals->setText(tr("Port%1 TX Signals (%2)")
										.arg(ports[1].portNo)
										.arg(ports[1].txSignals.size()));

			m_port2RxSignals->setText(tr("Port%1 RX Signals (%2)")
										.arg(ports[1].portNo)
										.arg(ports[1].rxSignals.size()));

			m_port2TxBuffer->setText(tr("Port%1 TX Buffer Snapshot").arg(ports[1].portNo));
			m_port2RxBuffer->setText(tr("Port%1 RX Buffer Snapshot").arg(ports[1].portNo));
		}
		else
		{
			m_port2Label->setText({});
		}
	}

	return;
}

void SimConnectionPage::showTxSignals(int portIndex)
{
	const auto& ports = m_connectionInfo.ports;

	if (ports.size() < static_cast<size_t>(portIndex + 1))
	{
		return;
	}

	showXxSignals(ports[portIndex].portNo, ports[portIndex].equipmentID, "TX", ports[portIndex].txSignals);
	return;
}

void SimConnectionPage::showRxSignals(int portIndex)
{
	const auto& ports = m_connectionInfo.ports;

	if (ports.size() < static_cast<size_t>(portIndex + 1))
	{
		return;
	}

	showXxSignals(ports[portIndex].portNo, ports[portIndex].equipmentID, "RX", ports[portIndex].rxSignals);
	return;
}

void SimConnectionPage::showXxSignals(int portNo, QString portId, QString trx, const std::vector<ConnectionTxRxSignal>& ss)
{
	QString text;
	text.reserve(128000);

	for (const ConnectionTxRxSignal& s : ss)
	{
		switch (s.type)
		{
		case E::SignalType::Analog:
			text += QString("%1 %2 Offset=0x%3 (%4)\n")
					.arg(s.ID)
					.arg("A")
					.arg(s.addrInBuf.offset(), 4, 16, QChar('0'))
					.arg(s.addrInBuf.offset());
			break;
		case E::SignalType::Discrete:
			text += QString("%1 %2 Offset=0x%3[%5] (%4[%5])\n")
					.arg(s.ID)
					.arg("A")
					.arg(s.addrInBuf.offset(), 4, 16, QChar('0'))
					.arg(s.addrInBuf.offset())
					.arg(s.addrInBuf.bit());
			break;
		case E::SignalType::Bus:
			text += QString("%1 %2 Offset=0x%3 (%4)\n")
					.arg(s.ID)
					.arg("B")
					.arg(s.addrInBuf.offset(), 4, 16, QChar('0'))
					.arg(s.addrInBuf.offset());
			break;
		default:
			assert(false);
		}
	}

	QInputDialog d(this->parentWidget());

	d.setWindowFlag(Qt::WindowContextHelpButtonHint, false);
	d.setWindowFlag(Qt::WindowSystemMenuHint, false);
	d.setLabelText(tr("Port%1 %2 Signals\n%3").arg(portNo).arg(trx).arg(portId));
	d.setInputMode(QInputDialog::InputMode::TextInput);
	d.setOption(QInputDialog::UsePlainTextEditForTextInput);

	d.setTextValue(text);

	d.exec();

	return;
}

void SimConnectionPage::showTxBuffer(int portIndex)
{
	const auto& ports = m_connectionInfo.ports;
	if (ports.size() < static_cast<size_t>(portIndex + 1))
	{
		return;
	}

	showXxBuffer(ports[portIndex].portNo,
				 ports[portIndex].equipmentID,
				 ports[portIndex].lmID,
				 E::LogicModuleRamAccess::Write,
				 ports[portIndex].txBufferAbsAddr,
				 ports[portIndex].txDataSizeW,
				 "TX");
	return;
}

void SimConnectionPage::showRxBuffer(int portIndex)
{
	const auto& ports = m_connectionInfo.ports;
	if (ports.size() < static_cast<size_t>(portIndex + 1))
	{
		return;
	}

	showXxBuffer(ports[portIndex].portNo,
				 ports[portIndex].equipmentID,
				 ports[portIndex].lmID,
				 E::LogicModuleRamAccess::Read,
				 ports[portIndex].rxBufferAbsAddr,
				 ports[portIndex].rxDataSizeW,
				 "RX");
}

void SimConnectionPage::showXxBuffer(int portNo,
									 QString portId,
									 QString lmEquipmentId,
									 E::LogicModuleRamAccess access,
									 quint32 offsetW,
									 quint32 sizeW,
									 QString trx)
{
	// Parent will be SimulatorTabPage*
	//
	QWidget* parent = this->parentWidget();
	while (parent != nullptr)
	{
		if (dynamic_cast<SimulatorTabPage*>(parent) != nullptr)
		{
			break;
		}

		parent = parent->parentWidget();
	}
	Q_ASSERT(parent);

	// Get current buffer
	//
	Sim::Ram ram;

	bool ok = m_simulator->appSignalManager().getUpdateForRam(lmEquipmentId, &ram);
	if (ok == false)
	{
		QMessageBox::critical(this, qAppName(), tr("Memory for module %1 is not found.").arg(lmEquipmentId));
		return;
	}

	QByteArray data;
	ram.readToBuffer(offsetW, access, sizeW, &data, true);

	// Show memory dialog
	//
	QString text;
	text.reserve(65534);

	int offsetInBuffer = 0;

	while (offsetInBuffer < data.size())
	{
		if (offsetInBuffer % 16 == 0)
		{
			if (text.isEmpty() == false)
			{
				text += "\n";
			}

			// New line
			//
			text += QString("%1 | ").arg(offsetInBuffer, 4, 16, QChar('0'));
		}

		text += QString("%1 ").arg(static_cast<quint8>(data[offsetInBuffer]), 2, 16, QChar('0'));

		offsetInBuffer++;
	}

	QInputDialog d(this->parentWidget());

	d.setWindowFlag(Qt::WindowContextHelpButtonHint, false);
	d.setWindowFlag(Qt::WindowSystemMenuHint, false);

	d.setLabelText(tr("Port%1 %2 Buffer\n%3").arg(portNo).arg(trx).arg(portId));
	d.setInputMode(QInputDialog::InputMode::TextInput);
	d.setOption(QInputDialog::UsePlainTextEditForTextInput);

	d.setTextValue(text);

#if defined(Q_OS_WIN)
		QFont font = QFont("Consolas");
#else
		QFont font = QFont("Courier");
#endif
	d.setFont(font);

	d.exec();

	return;
}

const QString& SimConnectionPage::connectionId() const
{
	return m_connectionId;
}
