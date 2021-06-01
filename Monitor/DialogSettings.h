#pragma once

#include "MonitorAppSettings.h"

namespace Ui {
	class DialogSettings;
}

class DialogSettings : public QDialog
{
	Q_OBJECT
	
public:
	explicit DialogSettings(QWidget* parent);
	virtual ~DialogSettings();

	const MonitorAppSettings::Data& settings() const;
	void setSettings(const MonitorAppSettings::Data& value);

protected:
	virtual void showEvent(QShowEvent* event) override;
	
private slots:
	std::optional<MonitorAppSettings::Data> parseData();

	void ok_clicked();
	void cancel_clicked();
	void saveAs_clicked();

private:
	Ui::DialogSettings *ui;
	MonitorAppSettings::Data m_settings;
};


