#pragma once

#include <QMainWindow>
#include "../lib/ServiceSettings.h"
#include "../lib/DataSource.h"
#include "../lib/Service.h"
#include "../TuningService/TuningDataSource.h"
#include "../TuningService/TuningService.h"
#include "TuningIPENService.h"

class QTabWidget;
class QTableView;
class QFormLayout;
class QPushButton;
class QScrollBar;
class QLabel;
class AnalogSignalSetter;
class DiscreteSignalSetter;

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
	QTableView* m_beamDoorsWidget;
	QWidget* m_reactivityWidget;
	QWidget* m_automaticPowerRegulatorWidget;

	TuningIPEN::TuningIPENService* m_service = nullptr;
	QVector<Tuning::TuningDataSourceInfo> m_info;
	QMap<QString, QLabel*> m_statusLabelMap;

	QScrollBar* m_scrollBar;
	QTimer* m_updateTimer;
	QThread* m_logThread;
	LogWriter* m_logWriter;

	AnalogSignalSetter* addAnalogSetter(QFormLayout* fl, QVector<Tuning::TuningDataSourceInfo>& sourceInfoVector, QString label, QString id, double highLimit);
	DiscreteSignalSetter* addDiscreteSetter(QFormLayout* fl, QVector<Tuning::TuningDataSourceInfo>& sourceInfoVector, QString label, QString id);

	bool loadConfigurationFromFile(const QString& fileName);
	bool readTuningDataSources(XmlReadHelper& xml);

public slots:
	void updateSignalStates();

	void updateSignalState(QString appSignalID, double currentValue, double lowLimit, double highLimit, bool valid);
	void updateDataSourceStatus(Tuning::TuningDataSourceState state);

	void applyNewScrollBarValue();

signals:
	void scrollBarMoved(double newValue);
	void automaticModeChanged(bool newValue);

public:
	explicit TuningMainWindow(QString buildPath, QWidget *parent = 0);
	~TuningMainWindow();

	void onTuningServiceReady();
};

