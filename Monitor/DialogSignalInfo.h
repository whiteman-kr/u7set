#ifndef DIALOGSIGNALINFO_H
#define DIALOGSIGNALINFO_H

#include "../lib/AppSignalManager.h"
#include "MonitorConfigController.h"
#include "TcpSignalClient.h"

namespace Ui {
	class DialogSignalInfo;
}


class MonitorCentralWidget;

class DialogSetpointDetails : public QDialog
{
	Q_OBJECT

public:
	DialogSetpointDetails(QWidget* parent, std::shared_ptr<Comparator> comparator);

private:
	std::shared_ptr<Comparator> m_comparator;

};


class SignalFlagsWidget : public QWidget
{
	Q_OBJECT

public:
	enum class SignalFlagsFields
	{
		Valid = 0,
		StateAvailable,
		Simulated,
		Blocked,
		Mismatch,
		AboveHighLimit,
		BelowLowLimit,
		Count
	};


public:
	SignalFlagsWidget(QWidget* parent = nullptr);

	void updateControl(AppSignalStateFlags flags);

protected:
	void paintEvent(QPaintEvent *event);

private:
	AppSignalStateFlags m_flags;
};


class DialogSignalInfo : public QDialog
{
	Q_OBJECT

public:
	static bool showDialog(QString appSignalId, MonitorConfigController* configController, TcpSignalClient* tcpSignalClient, MonitorCentralWidget* centralWidget);

public slots:
	void onSignalParamAndUnitsArrived();

private:
	DialogSignalInfo(const AppSignalParam& signal, MonitorConfigController* configController, MonitorCentralWidget* centralWidget);
	~DialogSignalInfo();

private:
	enum class SchemasColumns
	{
		SchemaId
	};

	enum class SetpointsColumns
	{
		Index,
		Type,
		CompareTo,
		CompareToValue,
		Output,
		OutputValue,
		SchemaId
	};

private slots:
	void preparePropertiesContextMenu(const QPoint& pos);
	void prepareSchemaContextMenu(const QPoint& pos);
	void prepareSetpointsContextMenu(const QPoint& pos);

	void on_treeSchemas_itemDoubleClicked(QTreeWidgetItem *item, int column);
	void on_treeSetpoints_itemDoubleClicked(QTreeWidgetItem *item, int column);

	void on_pushButtonSetZero_clicked();
	void on_pushButtonSetOne_clicked();
	void on_pushButtonSetValue_clicked();

	void switchToSchema();
	void showSetpointDetails();


protected:
	virtual void timerEvent(QTimerEvent* event) override;
	void mousePressEvent(QMouseEvent* event);

private:
	void fillSignalInfo();
	void fillProperties();
	void fillSetpoints();
	void fillSchemas();

	void updateData();

	void updateState();
	void updateSetpoints();

	void stateContextMenu(QPoint pos);

	QString signalStateText(const AppSignalParam& param, const AppSignalState& state, E::ValueViewType viewType, int precision);



private:
	static std::map<QString, DialogSignalInfo*> m_dialogSignalInfoMap;

	Ui::DialogSignalInfo *ui;

	MonitorConfigController* m_configController = nullptr;
	MonitorCentralWidget* m_centralWidget = nullptr;

	AppSignalParam m_signal;

	QVector<std::shared_ptr<Comparator>> m_comparators;

	int m_updateStateTimerId = -1;
	int m_currentPrecision = 0;
	E::ValueViewType m_viewType = E::ValueViewType::Dec;
	int m_currentFontSize = 20;

	QString m_contextMenuSchemaId;
	int m_contextMenuSetpointIndex = -1;
};


class QLabelAppSignalDragAndDrop : public QLabel
{
public:
	explicit QLabelAppSignalDragAndDrop(QWidget* parent = nullptr);

public:
	void setAppSignal(const AppSignalParam& signal);

protected:
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;

private:
	AppSignalParam m_appSignalParam;
	QPoint m_dragStartPosition;
};

#endif // DIALOGSIGNALINFO_H
