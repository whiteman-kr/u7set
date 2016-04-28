#pragma once

#include <QWidget>

class QLineEdit;
class QComboBox;
class QUdpSocket;

class Widget : public QWidget
{
	Q_OBJECT

public:
	Widget(QWidget *parent = 0);
	~Widget();

public slots:
	void sendPacket();
	void rebindSocket();

private:
	QComboBox* m_sourceAddressCombo;
	QLineEdit* m_sourcePortEdit;
	QLineEdit* m_destinationAddressEdit;
	QLineEdit* m_destinationPortEdit;

	QUdpSocket* m_socket;

	int numerator;
};
