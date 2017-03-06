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
	explicit DialogSettings(QWidget *parent = 0);
	~DialogSettings();

    bool filterSettingsChanged();

private slots:
	void on_DialogSettings_accepted();

private:
    void createLanguagesList();

private:
	Ui::DialogSettings *ui;

    bool m_filterSettingsChanged = false;
};

#endif // DIALOGSETTINGS_H
