#pragma once

#include <QDialog>
#include "Settings.h"

namespace Ui {
class TestSuiteDialogSettings;
}

class TestSuiteDialogSettings : public QDialog
{
	Q_OBJECT

public:
	explicit TestSuiteDialogSettings(QWidget *parent = nullptr);
	~TestSuiteDialogSettings();

	void setSettings(const Settings& settings);
	const Settings& settings() const;

private slots:
	void on_TestSuiteDialogSettings_accepted();

	void on_loadSciptsPathBrowse_clicked();

	void on_loadSciptsPathCheck_stateChanged(int arg1);

private:
	Ui::TestSuiteDialogSettings *ui;

	Settings m_settings;
};

