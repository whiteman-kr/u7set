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

class OptionsDialog : public QDialog
{
	Q_OBJECT

public:

	explicit OptionsDialog(QWidget *parent = nullptr);
	virtual ~OptionsDialog();

private:

	BuildOption				m_pathOption;

	QLineEdit*				m_buildDirPathEdit = nullptr;
	QPushButton*			m_selectBuildPathBtn = nullptr;
	QLineEdit*				m_signalsFileEdit = nullptr;
	QLineEdit*				m_sourceCfgFileEdit = nullptr;
	QLineEdit*				m_sourcesFileEdit = nullptr;

	QLineEdit*				m_appDataSrvIPEdit = nullptr;
	QLineEdit*				m_ualTesterIPEdit = nullptr;

	QDialogButtonBox*		m_buttonBox = nullptr;

	bool					createInterface();

	void					saveWindowState();
	void					restoreWindowState();

public:

	BuildOption&			option() { return m_pathOption; }

protected:

	void					closeEvent(QCloseEvent* e);

signals:

private slots:

	// slots of buttons
	//
	void					onSelectBuildDirPath();
	void					onOk();
	void					onCancel();
};

// ==============================================================================================

#endif // SOURCEOPTIONS_H
