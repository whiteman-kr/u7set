#pragma once

#include <QWidget>

class QLineEdit;
class QComboBox;
class QUdpSocket;

class SendTuningFrameWidget : public QWidget
{
	Q_OBJECT

public:
	SendTuningFrameWidget(QWidget *parent = 0);
	~SendTuningFrameWidget();

public slots:
	void sendPacket();
	void rebindSocket();

private:
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

	QUdpSocket* m_socket;

	int numerator;
};
