#ifndef TUNINGMAINWINDOW_H
#define TUNINGMAINWINDOW_H

#include <QMainWindow>
#include "../include/ServiceSettings.h"
#include "../include/DataSource.h"
#include "../include/Service.h"
#include "../TuningService/TuningService.h"
#include "../TuningService/TuningDataSource.h"


namespace Ui {
class TuningMainWindow;
}

class TuningMainWindow : public QMainWindow
{
	Q_OBJECT
private:
	QString m_cfgPath;

	Service* m_service = nullptr;

	bool loadConfigurationFromFile(const QString& fileName);
	bool readTuningDataSources(XmlReadHelper& xml);

public:
	explicit TuningMainWindow(QString cfgPath, QWidget *parent = 0);
	~TuningMainWindow();

private:
	Ui::TuningMainWindow *ui;
};

#endif // TUNINGMAINWINDOW_H
