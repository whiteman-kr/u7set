#ifndef SETTINGSFORM_H
#define SETTINGSFORM_H

#include <QWidget>
#include "settings.h"

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
	void expertModeChanged(int state);

	virtual void accept() override;

private:
	Settings m_settings;

	QComboBox* m_pSerialPort;
	QLabel* m_pSerialPortLabel;

	QCheckBox* m_pShowDebugInfo;
	QCheckBox* m_pExpertMode;

	QPushButton* m_pOkButton;
	QPushButton* m_pCancelButton;

	QLineEdit* m_pServer;
	QLabel* m_pServerLabel;

	QLineEdit* m_pServerUsername;
	QLabel* m_pServerUsernameLabel;

	QLineEdit* m_pServerPassword;
	QLabel* m_pServerPasswordLabel;
};

#endif // SETTINGSFORM_H
