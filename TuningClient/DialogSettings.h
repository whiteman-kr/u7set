#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include <QDialog>
#include "Settings.h"
#include <ClientLib/ClientTranslator.h>

namespace Ui {
	class DialogSettings;
}

class DialogSettings : public QDialog
{
	Q_OBJECT

public:
	explicit DialogSettings(const ClientLib::ClientTranslator& translator, QWidget* parent);
	~DialogSettings();

	const TuningClientAppSettings::SystemData& settings() const;
	void setSettings(const TuningClientAppSettings::SystemData& value);

protected:
	virtual void showEvent(QShowEvent* event) override;
	virtual void accept() override;

private:
	void createLanguagesList(const ClientLib::ClientTranslator& translator);
	std::optional<TuningClientAppSettings::SystemData> parseData();

private slots:
	void on_m_useCustomFilters_stateChanged(int arg1);
	void on_m_filtersBrowse_clicked();

	void on_saveAsButton_clicked();

private:
	Ui::DialogSettings* ui;
	TuningClientAppSettings::SystemData m_settings;

};

#endif // DIALOGSETTINGS_H
