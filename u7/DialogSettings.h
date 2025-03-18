#pragma once

#include "AppSettings.h"

namespace Ui
{
	class DialogSettings;
}

class DialogSettings : public QDialog
{
	Q_OBJECT

public:
	explicit DialogSettings(QWidget* parent);
	virtual ~DialogSettings();

	const AppSettings& settings() const;
	void setSettings(const AppSettings& value);

protected:
	virtual void showEvent(QShowEvent* event) override;

private slots:
	void on_ok_clicked();
	void on_cancel_clicked();
	void on_browseOutputPath_clicked();

	void browsePgDump();
	void detectPgDump();
	void browsePsql();
	void detectPsql();

private:
	Ui::DialogSettings* ui;
	AppSettings m_settings;
};
