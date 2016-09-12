#ifndef DIALOGSETTINGSCONFIGURATOR_H
#define DIALOGSETTINGSCONFIGURATOR_H

#include <QDialog>

namespace Ui {
class DialogSettingsConfigurator;
}

class DialogSettingsConfigurator : public QDialog
{
	Q_OBJECT

public:
	explicit DialogSettingsConfigurator(QWidget *parent = 0);
	~DialogSettingsConfigurator();

private slots:
	void on_DialogSettingsConfigurator_accepted();

private:
	Ui::DialogSettingsConfigurator *ui;
};

#endif // DIALOGSETTINGSCONFIGURATOR_H
