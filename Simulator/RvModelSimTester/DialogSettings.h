#pragma once

#include <QDialog>

class QLabel;
class QLineEdit;

class DialogSettings : public QDialog
{
	Q_OBJECT
public:
	explicit DialogSettings(QDialog* parent = nullptr);

private:
	void loadSettings();
	void saveSettings();

private slots:
	void accept() override;

private:
	QLineEdit* ipEdit = nullptr;
	QLineEdit* portRemoteEdit = nullptr;
	QLineEdit* portLocalEdit = nullptr;
	QLabel* statusLabel = nullptr;

	const QString settingsFile = "settings.ini";
};
