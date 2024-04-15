#pragma once

#include "AppConfigSettings.h"
#include <ClientLib/ClientTranslator.h>

namespace Ui {
class TestSuiteDialogSettings;
}

class TestSuiteDialogSettings : public QDialog
{
	Q_OBJECT

public:
	explicit TestSuiteDialogSettings(const ClientLib::ClientTranslator& translator, QWidget *parent = nullptr);
	~TestSuiteDialogSettings();

	const AppConfigSettings::Data& settings() const;
	void setSettings(const AppConfigSettings::Data& settings);

protected:
	virtual void showEvent(QShowEvent* event) override;
	virtual void accept() override;

private:
	void createLanguagesList(const ClientLib::ClientTranslator& translator);
	std::optional<AppConfigSettings::Data> parseData();

private slots:
	void on_loadSciptsPathBrowse_clicked();
	void on_loadSciptsPathCheck_stateChanged(int arg1);
	
	void on_saveAsButton_clicked();

private:
	Ui::TestSuiteDialogSettings *ui;
	AppConfigSettings::Data m_settings;
};

