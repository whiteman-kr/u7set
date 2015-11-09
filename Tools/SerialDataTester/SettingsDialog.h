#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
	class SettingsDialog;
}

class SettingsDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SettingsDialog(QWidget *parent = 0);
	~SettingsDialog();

private slots:
	void settingsConfirmed();
	void settingsCanceled();
	void browseSignalsXmlFile();

signals:
	void sendSettingsCreated();

private:
	Ui::SettingsDialog *ui;

	QString m_pathToSignalsXml;
};

#endif // SETTINGSDIALOG_H
