#ifndef TUNINGMAINWINDOW_H
#define TUNINGMAINWINDOW_H

#include <QMainWindow>
#include "../include/ServiceSettings.h"
#include "../include/DataSource.h"
#include "../include/Service.h"
#include "../TuningService/TuningService.h"
#include "../TuningService/TuningDataSource.h"

class QTabWidget;

class TuningMainWindow : public QMainWindow
{
	Q_OBJECT
private:
	QString m_cfgPath;
	QTabWidget* m_setOfSignalsScram;

	TuningService* m_service = nullptr;
	QVector<TuningDataSourceInfo> m_info;

	bool loadConfigurationFromFile(const QString& fileName);
	bool readTuningDataSources(XmlReadHelper& xml);

public:
	explicit TuningMainWindow(QString cfgPath, QWidget *parent = 0);
	~TuningMainWindow();

	void onTuningServiceReady();
};

#endif // TUNINGMAINWINDOW_H
