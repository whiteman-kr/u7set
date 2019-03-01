#ifndef OPTIONDIALOG_H
#define OPTIONDIALOG_H

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

class OptionDialog : public QDialog
{
	Q_OBJECT

public:

	explicit OptionDialog(QWidget *parent = nullptr);
	virtual ~OptionDialog();

private:

	QLabel*					m_pFileNameLabel = nullptr;
	QLineEdit*				m_pFileNameEdit = nullptr;

	QLabel*					m_pPercentLabel = nullptr;
	QLineEdit*				m_pPercentEdit = nullptr;

	QLabel*					m_pPacketCountLabel = nullptr;
	QLineEdit*				m_pPacketCountEdit = nullptr;

	QDialogButtonBox*		m_buttonBox = nullptr;

	bool					createInterface();

public:

signals:

private slots:

	// slots of buttons
	//
	void					onOk();
};

// ==============================================================================================

#endif // OPTIONDIALOG_H
