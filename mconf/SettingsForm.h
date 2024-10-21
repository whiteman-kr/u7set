#pragma once

#include "MconfSettings.h"

#include <QDialog>

class QComboBox;
class QLabel;
class QLineEdit;
class QCheckBox;


class SettingsForm : public QDialog
{
	Q_OBJECT

public:
	SettingsForm(const MconfSettings& settings, QWidget *parent);
	~SettingsForm();

	const MconfSettings& settings() const;

private slots:
	void currentSerialPortChanged(const QString & text);
	void showDebugInfoChanged(int state);
	void verifyChanged(int state);
	void expertModeChanged(int state);

	virtual void accept() override;

private:
	MconfSettings m_settings;

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
