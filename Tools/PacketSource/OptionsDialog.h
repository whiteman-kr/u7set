#ifndef SOURCEOPTIONS_H
#define SOURCEOPTIONS_H

#include <QDialog>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QMessageBox>

#include "Options.h"

// ==============================================================================================

class OptionsDialog : public QDialog
{
	Q_OBJECT

public:

	explicit OptionsDialog(const BuildInfo& buildInfo, QWidget *parent = nullptr);
	virtual ~OptionsDialog();

private:

	BuildInfo				m_buildInfo;

	QLineEdit*				m_buildDirPathEdit = nullptr;
	QPushButton*			m_selectBuildPathBtn = nullptr;
	QLineEdit*				m_signalsFileEdit = nullptr;
	QLineEdit*				m_sourceCfgFileEdit = nullptr;
	QLineEdit*				m_sourcesFileEdit = nullptr;

	QCheckBox*				m_enableReloadCheck = nullptr;
	QLineEdit*				m_timeoutReloadEdit = nullptr;

	QLineEdit*				m_appDataSrvIPEdit = nullptr;
	QLineEdit*				m_ualTesterIPEdit = nullptr;

	QDialogButtonBox*		m_buttonBox = nullptr;

	bool					createInterface();

	bool					loadBuildDirPath(const QString& buildDirPath);

	void					saveWindowState();
	void					restoreWindowState();

public:

	BuildInfo&				buildInfo() { return m_buildInfo; }

protected:

	void					closeEvent(QCloseEvent* e);

signals:

private slots:

	// slots of buttons
	//
	void					onSelectBuildDirPath();
	void					onEnableReload();
	void					onTimeoutReload(const QString &sec);
	void					onAppDataSrvIP(const QString &ip);
	void					onUalTesterIP(const QString &ip);

	void					onOk();
	void					onCancel();
};

// ==============================================================================================

#endif // SOURCEOPTIONS_H
