#ifndef APPLICATIONTABPAGE_H
#define APPLICATIONTABPAGE_H

#include <QWidget>
#include "ui_applicationtabpage.h"
#include "../include/ModuleConfiguration.h"

using namespace Hardware;

class ApplicationTabPage : public QWidget
{
	Q_OBJECT

public:
	ApplicationTabPage(QWidget* parent = 0);
	~ApplicationTabPage();

	bool isFileLoaded() const;
	Hardware::ModuleFirmware* configuration();

protected slots:
	void openFileClicked();

private:
	Ui::ApplicationTabPage ui;
    //ConfigDataReader m_reader;
	ModuleFirmware m_confFirmware;
};

#endif // APPLICATIONTABPAGE_H
