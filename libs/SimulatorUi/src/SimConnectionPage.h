#pragma once
#include "SimBasePage.h"

#include <HardwareLib/ConnectionsInfo.h>


class QLabel;
class QPushButton;

namespace SimUi
{
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

		void showTxBuffer(int portIndex);
		void showRxBuffer(int portIndex);
		void showXxBuffer(int portNo,
						  QString portId,
						  QString lmEquipmentId,
						  E::LogicModuleRamAccess access,
						  quint32 offsetW,
						  quint32 sizeW,
						  QString trx);

	public:
		const QString& connectionId() const;

	private:
		QString m_connectionId;
		ConnectionInfo m_connectionInfo;

		QLabel* m_connectionIdLabel = nullptr;
		QPushButton* m_disableButton = nullptr;
		QLabel* m_stateLabel = nullptr;

		QLabel* m_connectionType = nullptr;

		QLabel* m_port1Label = nullptr;
		QPushButton* m_port1TxSignals = nullptr;
		QPushButton* m_port1RxSignals = nullptr;
		QPushButton* m_port1TxBuffer = nullptr;
		QPushButton* m_port1RxBuffer = nullptr;

		QLabel* m_port2Label = nullptr;
		QPushButton* m_port2TxSignals = nullptr;
		QPushButton* m_port2RxSignals = nullptr;
		QPushButton* m_port2TxBuffer = nullptr;
		QPushButton* m_port2RxBuffer = nullptr;
	};
} // namespace SimUi