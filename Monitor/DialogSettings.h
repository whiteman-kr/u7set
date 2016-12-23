#pragma once

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

protected:
	virtual void showEvent(QShowEvent* event) override;
	
private slots:
	void ok_clicked();
	void cancel_clicked();

private:
	Ui::DialogSettings *ui;
	Settings m_settings;
};


