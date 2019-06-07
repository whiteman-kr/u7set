#ifndef DIALOGSIGNALINFO_H
#define DIALOGSIGNALINFO_H

#include "../lib/AppSignalManager.h"
#include "MonitorConfigController.h"

namespace Ui {
	class DialogSignalInfo;
}


class MonitorCentralWidget;


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
	SignalFlagsWidget(QWidget* parent = 0);

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
	static bool showDialog(QString appSignalId, MonitorConfigController* configController, MonitorCentralWidget* centralWidget);

private:
	DialogSignalInfo(const AppSignalParam& signal, MonitorConfigController* configController, MonitorCentralWidget* centralWidget);
	~DialogSignalInfo();


private slots:
	void preparePropertiesContextMenu(const QPoint& pos);
	void prepareSchemasContextMenu(const QPoint& pos);

	void on_treeSchemas_itemDoubleClicked(QTreeWidgetItem *item, int column);
	void on_pushButtonSetZero_clicked();
	void on_pushButtonSetOne_clicked();
	void on_pushButtonSetValue_clicked();

protected:
	virtual void timerEvent(QTimerEvent* event) override;
	void mousePressEvent(QMouseEvent* event);

private:
	void fillProperties();
	void fillSchemas();

	void updateData();
	void contextMenu(QPoint pos);

private:
	static std::map<QString, DialogSignalInfo*> m_dialogSignalInfoMap;

	Ui::DialogSignalInfo *ui;

	MonitorConfigController* m_configController = nullptr;
	MonitorCentralWidget* m_centralWidget = nullptr;

	AppSignalParam m_signal;

	int m_updateStateTimerId = -1;
	int m_currentPrecision = 0;
	E::ValueViewType m_viewType = E::ValueViewType::Dec;
	int m_currentFontSize = 20;
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
