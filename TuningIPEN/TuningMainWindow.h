#pragma once

#include <QMainWindow>
#include "../include/ServiceSettings.h"
#include "../include/DataSource.h"
#include "../include/Service.h"
#include "../TuningService/TuningDataSource.h"
#include "../TuningService/TuningService.h"
#include "TuningIPENService.h"

class QTabWidget;
class QFormLayout;
class QPushButton;
class QScrollBar;
class QLabel;
class AnalogSignalSetter;

class LogWriter : public QObject
{
	Q_OBJECT

private:
	void writeFrameToLog(QString caption, FotipFrame& fotipFrame);

public slots:
	void onUserRequest(FotipFrame fotipFrame);
	void onReplyWithNoZeroFlags(FotipFrame fotipFrame);
};

class TuningMainWindow : public QMainWindow
{
	Q_OBJECT
private:
	QString m_cfgPath;
	QTabWidget* m_setOfSignalsScram;
	QWidget* m_automaticPowerRegulatorWidget;

	Tuning::TuningService* m_service = nullptr;
	QVector<Tuning::TuningDataSourceInfo> m_info;
	QMap<QString, QLabel*> m_statusLabelMap;

	QPushButton* m_automaticMode;
	QScrollBar* m_scrollBar;
	QTimer* m_updateTimer;
	QThread* m_logThread;

	AnalogSignalSetter* addAnalogSetter(QFormLayout* fl, QVector<Tuning::TuningDataSourceInfo>& sourceInfoVector, QString label, QString id, double highLimit);
	bool loadConfigurationFromFile(const QString& fileName);
	bool readTuningDataSources(XmlReadHelper& xml);

public slots:
	void updateSignalStates();

	void updateSignalState(QString appSignalID, double currentValue, double lowLimit, double highLimit, bool valid);
	void updateDataSourceStatus(Tuning::TuningDataSourceState state);

	void applyNewScrollBarValue();
	void applyNewAutomaticMode(bool enabled);

signals:
	void scrollBarMoved(double newValue);
	void automaticModeChanged(bool newValue);

public:
	explicit TuningMainWindow(QString cfgPath, QWidget *parent = 0);
	~TuningMainWindow();

	void onTuningServiceReady();
};

