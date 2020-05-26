#ifndef DIALOGSIGNALINFO_H
#define DIALOGSIGNALINFO_H

#include <optional>
#include "../VFrame30/TuningController.h"
#include "../lib/AppSignalManager.h"
#include "../lib/Signal.h"
#include "../lib/Ui/DragDropHelper.h"

namespace Ui {
	class DialogSignalInfo;
}

class DialogSetpointDetails : public QDialog
{
	Q_OBJECT

public:
	DialogSetpointDetails(QWidget* parent, IAppSignalManager* appSignalManager, std::shared_ptr<Comparator> comparator);

private:
	std::shared_ptr<Comparator> m_comparator;

	IAppSignalManager* m_appSignalManager = nullptr;

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
	void mouseMoveEvent(QMouseEvent *event);

private:
	QRect flag2Rect(int flagNo);
	int point2Flag(const QPoint& pt);

private:
	AppSignalStateFlags m_flags;

	const int m_colCount = 5;
	const int m_rowCount = 2;

	int m_lastFlagAbove = -1;

};


class DialogSignalInfo : public QDialog
{
	Q_OBJECT

protected:
	enum class DialogType
	{
		Monitor,
		Simulator
	};

	DialogSignalInfo(const AppSignalParam& signal,
					 IAppSignalManager* appSignalManager,
					 VFrame30::TuningController* tuningController,
					 bool tuningEnabled,
					 DialogType dialogType,
					 QWidget* parent);
	virtual ~DialogSignalInfo();

	// Register functions
	static DialogSignalInfo* dialogRegistered(const QString& appSignalId);
	static void registerDialog(const QString& appSignalId, DialogSignalInfo* dialog);
	static void unregisterDialog(const QString& appSignalId);

	// Signal and parameters
	bool tuningEnabled() const;
	void setTuningEnabled(bool enabled);

	AppSignalParam signal() const;
	void updateSignal(const AppSignalParam& signal);

protected:
	virtual std::optional<Signal> getSignalExt(const AppSignalParam& appSignalParam) = 0;

private:
	enum class SchemasColumns
	{
		SchemaId
	};

	enum class SetpointsColumns
	{
		Type,			// Contains user data index
		CompareTo,		// Contains user data paramCompareTo(AppSignalParam)
		CompareToValue,
		Output,			// Contains user data paramOutput(AppSignalParam)
		OutputValue,
		SchemaId
	};

private slots:
	void preparePropertiesContextMenu(const QPoint& pos);
	void preparePropertiesExtContextMenu(const QPoint& pos);
	void prepareSchemaContextMenu(const QPoint& pos);
	void prepareSetpointsContextMenu(const QPoint& pos);

	void on_treeSchemas_itemDoubleClicked(QTreeWidgetItem *item, int column);
	void on_treeSetpoints_itemDoubleClicked(QTreeWidgetItem *item, int column);

	void on_pushButtonSetZero_clicked();
	void on_pushButtonSetOne_clicked();
	void on_pushButtonSetValue_clicked();

	void switchToSchema();
	void showSetpointDetails();

private:
	virtual void showEvent(QShowEvent* e) override;
	virtual void timerEvent(QTimerEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void dragEnterEvent(QDragEnterEvent* event) override;
	virtual void dropEvent(QDropEvent* event) override;

	virtual QStringList schemasByAppSignalId(const QString& appSignalId) = 0;
	virtual void setSchema(QString schemaId, QStringList highlightIds) = 0;

private:
	// Tab widget helper functions
	//
	int tabPageExists(const QString& tabName);
	QWidget* tabPageWidget(const QString& tabName);
	void addTabPage(const QString& tabName, QWidget* widget);
	void removeTabPage(const QString& tabName);

	//
	void updateSingnalData();

	void fillSignalInfo();
	void fillProperties();
	void fillExtProperties();
	void fillSetpoints();
	void fillSchemas();
	void updateTuningTab();

	//
	void updateDynamicData();

	void updateState();
	void updateSetpoints();

	//
	void stateContextMenu(QPoint pos);

	QString signalStateText(const AppSignalParam& param, const AppSignalState& state, E::ValueViewType viewType, int precision);

private:
	Ui::DialogSignalInfo *ui;

	static std::map<QString, DialogSignalInfo*> m_dialogSignalInfoMap;

	AppSignalParam m_signal;

	IAppSignalManager* m_appSignalManager = nullptr;

	VFrame30::TuningController* m_tuningController = nullptr;	// Can be null if tuning is not enabled
	bool m_tuningEnabled = false;
	QWidget* m_tuningTabWidget = nullptr;

	std::vector<std::shared_ptr<Comparator>> m_setpoints;

	int m_updateStateTimerId = -1;
	int m_currentPrecision = 0;
	E::ValueViewType m_viewType = E::ValueViewType::Dec;
	int m_currentFontSize = 20;

	QString m_contextMenuSchemaId;
	int m_contextMenuSetpointIndex = -1;

	bool m_firstShow = true;

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
	DragDropHelper m_dragDrop;
};

#endif // DIALOGSIGNALINFO_H
