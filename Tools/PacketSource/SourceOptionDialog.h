#ifndef SOURCEOPTIONS_H
#define SOURCEOPTIONS_H

#include <QDialog>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>

#include "Options.h"

// ==============================================================================================

class SourceOptionDialog : public QDialog
{
	Q_OBJECT

public:

	explicit SourceOptionDialog(QWidget *parent = 0);
	virtual ~SourceOptionDialog();

private:

	SourceOption			m_sourceOption;

	QLineEdit*				m_pathEdit = nullptr;
	QPushButton*			m_selectPathBtn = nullptr;

	QDialogButtonBox*		m_buttonBox = nullptr;

	bool					createInterface();

public:

	SourceOption&			option() { return m_sourceOption; }

signals:

private slots:

	// slots of buttons
	//
	void					onSelectPath();
	void					onOk();
};

// ==============================================================================================

#endif // SOURCEOPTIONS_H
