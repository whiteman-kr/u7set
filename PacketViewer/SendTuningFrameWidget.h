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
	QComboBox* m_endiansCombo;
	QComboBox* m_sourceAddressCombo;
	QLineEdit* m_sourcePortEdit;
	QLineEdit* m_destinationAddressEdit;
	QLineEdit* m_destinationPortEdit;

	QLineEdit* m_moduleIdEdit;

	QLineEdit* m_uniqueId;

	QLineEdit* m_channelNumber;
	QLineEdit* m_subsystemCode;

	QComboBox* m_operationCode;
	QLineEdit* m_flags;
	QLineEdit* m_startAddress;
	QLineEdit* m_fotipFrameSize;
	QLineEdit* m_romSize;
	QLineEdit* m_romFrameSize;
	QComboBox* m_dataType;

	QComboBox* m_fillFotipDataMethod;

	std::shared_ptr<QUdpSocket> m_socket;
	PacketSourceModel* m_packetSourceModel;

	int numerator;
};
