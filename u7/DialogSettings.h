#pragma once

#include <QDialog>
#include "Settings.h"

namespace Ui {
	class DialogSettings;
}

class DialogSettings : public QDialog
{
	Q_OBJECT
	
public:
	explicit DialogSettings(QWidget* parent);
	virtual ~DialogSettings();

	const Settings& settings() const;
	void setSettings(const Settings& value);
	
private slots:
	void on_ok_clicked();

	void on_cancel_clicked();

private:
	Ui::DialogSettings *ui;
	Settings m_settings;
};


