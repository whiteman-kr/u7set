#ifndef SETTINGSFORM_H
#define SETTINGSFORM_H

#include <QWidget>
#include "Settings.h"

class SettingsForm : public QDialog
{
	Q_OBJECT

public:
	SettingsForm(const Settings& settings, QWidget *parent);
	~SettingsForm();

	const Settings& settings() const;

private slots:
	void currentSerialPortChanged(const QString & text);
	void showDebugInfoChanged(int state);
	void verifyChanged(int state);
	void expertModeChanged(int state);

	virtual void accept() override;

private:
	Settings m_settings;

	QComboBox* m_pSerialPort = nullptr;
	QLabel* m_pSerialPortLabel = nullptr;

	QCheckBox* m_pShowDebugInfo = nullptr;
	QCheckBox* m_pVerify = nullptr;
	QCheckBox* m_pExpertMode = nullptr;

	QPushButton* m_pOkButton = nullptr;
	QPushButton* m_pCancelButton = nullptr;

	QLineEdit* m_pServer = nullptr;
	QLabel* m_pServerLabel = nullptr;

	QLineEdit* m_pServerUsername = nullptr;
	QLabel* m_pServerUsernameLabel = nullptr;

	QLineEdit* m_pServerPassword = nullptr;
	QLabel* m_pServerPasswordLabel = nullptr;
};

#endif // SETTINGSFORM_H
