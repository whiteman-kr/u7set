#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include <QDialog>

namespace Ui {
	class DialogSettings;
}

class DialogSettings : public QDialog
{
	Q_OBJECT

public:
	explicit DialogSettings(QWidget* parent = 0);
	~DialogSettings();

private slots:
	void on_DialogSettings_accepted();

	void on_m_useCustomFilters_stateChanged(int arg1);

	void on_m_filtersBrowse_clicked();

private:
	void createLanguagesList();

private:
	Ui::DialogSettings* ui;
};

#endif // DIALOGSETTINGS_H
