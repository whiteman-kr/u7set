#ifndef TUNINGMAINWINDOW_H
#define TUNINGMAINWINDOW_H

#include <QMainWindow>
#include "../include/ServiceSettings.h"
#include "../include/DataSource.h"
#include "../include/Service.h"
#include "../TuningService/TuningService.h"
#include "../TuningService/TuningDataSource.h"

class QTabWidget;
class QFormLayout;
class QPushButton;
class QScrollBar;
class AnalogSignalSetter;

class TuningMainWindow : public QMainWindow
{
	Q_OBJECT
private:
	QString m_cfgPath;
	QTabWidget* m_setOfSignalsScram;
	QWidget* m_automaticPowerRegulatorWidget;

	TuningService* m_service = nullptr;
	QVector<TuningDataSourceInfo> m_info;

	QPushButton* m_automaticMode;
	QScrollBar* m_scrollBar;
	QTimer* m_updateTimer;

	void addAnalogSetter(QFormLayout* fl, QVector<TuningDataSourceInfo>& sourceInfoVector, QString label, QString id, double highLimit);
	bool loadConfigurationFromFile(const QString& fileName);
	bool readTuningDataSources(XmlReadHelper& xml);

public slots:
	void updateSignalStates();
	void updateSignalState(QString appSignalID, double value);
	void applyNewScrollBarValue();
	void applyNewAutomaticMode(bool enabled);

public:
	explicit TuningMainWindow(QString cfgPath, QWidget *parent = 0);
	~TuningMainWindow();

	void onTuningServiceReady();
};

#endif // TUNINGMAINWINDOW_H
