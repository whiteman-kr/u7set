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

class PathOptionDialog : public QDialog
{
	Q_OBJECT

public:

	explicit PathOptionDialog(QWidget *parent = nullptr);
	virtual ~PathOptionDialog();

private:

	PathOption				m_pathOption;

	QLineEdit*				m_signalPathEdit = nullptr;
	QPushButton*			m_selectSignalPathBtn = nullptr;

	QLineEdit*				m_sourcePathEdit = nullptr;
	QPushButton*			m_selectSourcePathBtn = nullptr;

	QLineEdit*				m_localIPEdit = nullptr;

	QDialogButtonBox*		m_buttonBox = nullptr;

	bool					createInterface();

public:

	PathOption&				option() { return m_pathOption; }

signals:

private slots:

	// slots of buttons
	//
	void					onSelectSignalPath();
	void					onSelectSourcePath();
	void					onOk();
};

// ==============================================================================================

#endif // SOURCEOPTIONS_H
