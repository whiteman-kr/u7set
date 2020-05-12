#pragma once
#include "SimBasePage.h"

class SimConnectionPage : public SimBasePage
{
	Q_OBJECT

public:
	SimConnectionPage(SimIdeSimulator* simulator, QString connectionId, QWidget* parent);
	virtual ~SimConnectionPage();

protected slots:
	void disableConnection(bool disable);

	void updateConnectionState(QString connectionId, bool enabled);
	void updateData();

	void showTxSignals(int portIndex);
	void showRxSignals(int portIndex);
	void showXxSignals(int portNo, QString portId, QString trx, const std::vector<ConnectionTxRxSignal>& ss);

public:
	const QString& connectionId() const;

private:
	QString m_connectionId;
	ConnectionInfo m_connectionInfo;

	QLabel* m_connectionIdLabel = new QLabel{this};
	QPushButton* m_disableButton = new QPushButton{tr("Disable"), this};
	QLabel* m_stateLabel = new QLabel{this};

	QLabel* m_connectionType = new QLabel{this};

	QLabel* m_port1Label = new QLabel{this};
	QPushButton* m_port1TxSignals = new QPushButton{this};
	QPushButton* m_port1RxSignals = new QPushButton{this};
	QPushButton* m_port1TxBuffer = new QPushButton{this};
	QPushButton* m_port1RxBuffer = new QPushButton{this};

	QLabel* m_port2Label = new QLabel{this};
	QPushButton* m_port2TxSignals = new QPushButton{this};
	QPushButton* m_port2RxSignals = new QPushButton{this};
	QPushButton* m_port2TxBuffer = new QPushButton{this};
	QPushButton* m_port2RxBuffer = new QPushButton{this};
};


