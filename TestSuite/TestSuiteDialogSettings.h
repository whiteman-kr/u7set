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

private:
	Ui::TestSuiteDialogSettings *ui;

	Settings m_settings;
};

