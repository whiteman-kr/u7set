#pragma once

#include <QDialog>

class QLineEdit;
class QComboBox;
class QUdpSocket;
class PacketSourceModel;

class SendTuningFrameWidget : public QDialog
{
	Q_OBJECT

public:
	SendTuningFrameWidget(PacketSourceModel* packetSourceModel, QWidget *parent = 0);
	~SendTuningFrameWidget();

public slots:
	void sendPacket();
	bool checkSocket();

private:
	QComboBox* m_endiansCombo = nullptr;
	QComboBox* m_sourceAddressCombo = nullptr;
	QLineEdit* m_sourcePortEdit = nullptr;
	QLineEdit* m_destinationAddressEdit = nullptr;
	QLineEdit* m_destinationPortEdit = nullptr;

	QLineEdit* m_moduleIdEdit = nullptr;

	QLineEdit* m_uniqueId = nullptr;

	QLineEdit* m_channelNumber = nullptr;
	QLineEdit* m_subsystemCode = nullptr;

	QComboBox* m_operationCode = nullptr;
	QLineEdit* m_flags = nullptr;
	QLineEdit* m_startAddress = nullptr;
	QLineEdit* m_fotipFrameSize = nullptr;
	QLineEdit* m_romSize = nullptr;
	QLineEdit* m_romFrameSize = nullptr;
	QComboBox* m_dataType = nullptr;

	QComboBox* m_fillFotipDataMethod = nullptr;

	std::shared_ptr<QUdpSocket> m_socket;
	PacketSourceModel* m_packetSourceModel = nullptr;

	quint16 m_numerator = 0;

	QRandomGenerator m_randGen;
};
