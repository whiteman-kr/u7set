#ifndef SERIALPORTOPTIONS_H
#define SERIALPORTOPTIONS_H

#include <QDialog>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QMessageBox>

#include "Options.h"

// ==============================================================================================

class SerialPortDialog : public QDialog
{
	Q_OBJECT

public:

	explicit SerialPortDialog(const SerialPortOption& portOption, QWidget *parent = nullptr);
	virtual ~SerialPortDialog();

private:

	SerialPortOption		m_portOption;

	QLabel*					m_pPortLabel = nullptr;
	QComboBox*				m_pPortList = nullptr;

	QLabel*					m_pTypeLabel = nullptr;
	QComboBox*				m_pTypeList = nullptr;

	QLabel*					m_pBaudRateLabel = nullptr;
	QComboBox*				m_pBaudRateList = nullptr;

	QLabel*					m_pDataSizeLabel = nullptr;
	QLineEdit*				m_pDataSizeEdit = nullptr;

	QDialogButtonBox*		m_buttonBox = nullptr;

	bool					createInterface();
	void					updatePortOption();

public:

	SerialPortOption&		option() { return m_portOption; }

signals:

private slots:

	// slots of list custom
	//
	void					customPort(int idx);
	void					customBaudRate(int idx);

	// slots of buttons
	//
	void					onOk();
};

// ==============================================================================================

#endif // SERIALPORTOPTIONS_H
