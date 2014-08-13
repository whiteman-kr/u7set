#ifndef APPLICATIONTABPAGE_H
#define APPLICATIONTABPAGE_H

#include <QWidget>
#include "ui_applicationtabpage.h"
#include "../include/configdata.h"

class ApplicationTabPage : public QWidget
{
	Q_OBJECT

public:
	ApplicationTabPage(QWidget* parent = 0);
	~ApplicationTabPage();

	bool isFileLoaded() const;
	const ConfigDataReader& configuration() const;

protected slots:
	void openFileClicked();

private:
	Ui::ApplicationTabPage ui;
	ConfigDataReader m_reader;
};

#endif // APPLICATIONTABPAGE_H
