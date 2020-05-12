#include "SimConnectionPage.h"

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

	layout->addWidget(m_connectionIdLabel, 0, 0, 1, 5);
	layout->addWidget(m_connectionType, 1, 0, 1, 5);

	layout->addWidget(m_port1Label, 2, 0, 1, 5);

	layout->addWidget(m_port1TxSignals, 3, 0);
	layout->addWidget(m_port1RxSignals, 3, 1);
	layout->addWidget(m_port1TxBuffer, 3, 2);
	layout->addWidget(m_port1RxBuffer, 3, 3);

	layout->addWidget(m_port2Label, 4, 0, 1, 5);

	layout->addWidget(m_port2TxSignals, 5, 0);
	layout->addWidget(m_port2RxSignals, 5, 1);
	layout->addWidget(m_port2TxBuffer, 5, 2);
	layout->addWidget(m_port2RxBuffer, 5, 3);

	layout->addWidget(spacer, 6, 0, 1, 5);

	setLayout(layout);

	// --
	//
	connect(m_simulator, &Sim::Simulator::projectUpdated, this, &SimConnectionPage::updateData);

	// Fill data
	//
	updateData();

	return;
}

SimConnectionPage::~SimConnectionPage()
{
}

void SimConnectionPage::updateData()
{
	Sim::ConnectionPtr c = m_simulator->connections().connection(m_connectionId);

	if (c != nullptr)
	{
		m_connectionInfo = c->connectionInfo();
	}

	m_connectionIdLabel->setText(tr("ConnectionID: <b>%1</b>")
								 .arg(m_connectionId));

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

			m_port1TxBuffer->setText(tr("Port%1 TX Buffer").arg(ports[0].portNo));
			m_port1RxBuffer->setText(tr("Port%1 RX Buffer").arg(ports[0].portNo));
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

			m_port2TxBuffer->setText(tr("Port%1 TX Buffer").arg(ports[1].portNo));
			m_port2RxBuffer->setText(tr("Port%1 RX Buffer").arg(ports[1].portNo));
		}
		else
		{
			m_port2Label->setText({});
		}
	}

	return;
}

const QString& SimConnectionPage::connectionId() const
{
	return m_connectionId;
}
