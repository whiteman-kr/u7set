#pragma once

#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>

#include "../DbLib/DbStruct.h"
#include "MainTabPage.h"
#include "GlobalMessanger.h"

class DbController;
class QTableView;
class QMenu;
class SignalsModel;
class QToolBar;
class QPlainTextEdit;
class QSplitter;
class SignalsProxyModel;
class QComboBox;
class SignalsTabPage;
class QTimer;
class QCheckBox;
class QLineEdit;
class QCompleter;
class QActionGroup;
class QStandardItemModel;
class TableDataVisibilityController;
class AppSignalSetProvider;
class FindSignalDialog;
class DialogMetrologyConnection;

namespace Hardware
{
	class DeviceAppSignal;
}

struct CreatingSignalOptions
{
	QStringList lmEquipmentIdList;
	QStringList selectedEquipmentIdList;
	QStringList appSignalIdList;
	QStringList customSignalIdList;
	int defaultSignalTypeIndex = -1;
	QString defaultBusTypeId;
	QRect settingsWindowPositionRect;
};

class SignalsDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
	explicit SignalsDelegate(AppSignalSetProvider* signalSetProvider,
							 SignalsModel* model,
							 SignalsProxyModel* signalsProxyModel,
							 QObject* parent = nullptr);
	~SignalsDelegate();

	QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const;

	void setEditorData(QWidget* editor, const QModelIndex& index) const;
	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const;

	void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const;

signals:
	void itemDoubleClicked();

public slots:
	void onCloseEditorEvent(QWidget* editor, EndEditHint hint);

protected:
	bool editorEvent(QEvent* event, QAbstractItemModel* model,
					 const QStyleOptionViewItem& option, const QModelIndex& index);

private:
	AppSignalSetProvider* m_signalSetProvider;
	SignalsModel* m_model;
	SignalsProxyModel* m_proxyModel;
	mutable int signalIdForUndoOnCancelEditing = -1;
};


class SignalsModel;
class SignalsProxyModel;

class SignalsTabPage : public MainTabPage
{
	Q_OBJECT

public:
	static const int FILTER_ST_ANALOG = TO_INT(E::SignalType::Analog);
	static const int FILTER_ST_DISCRETE = TO_INT(E::SignalType::Discrete);
	static const int FILTER_ST_BUS = TO_INT(E::SignalType::Bus);
	static const int FILTER_ST_ANY = 0xff;

	static const int FILTER_STR_ANY = 0;
	static const int FILTER_STR_APP_SIGNAL_ID = 1;
	static const int FILTER_STR_CUSTOM_APP_SIGNAL_ID = 2;
	static const int FILTER_STR_EQUIPMENT_ID = 3;
	static const int FILTER_STR_CAPTION = 4;
	static const int FILTER_STR_TAGS = 5;

public:
	SignalsTabPage(AppSignalSetProvider* signalSetProvider, DbController* dbController, QWidget* parent);
	virtual ~SignalsTabPage() override;

	static bool updateSignalsSpecProps(DbController* dbc,
									   const std::vector<const Hardware::DeviceAppSignal*>& deviceSignalsToUpdate,
									   const QStringList& forceUpdateProperties);
	int getMiddleVisibleRow();

	bool editSignals(const std::vector<int> &ids);

protected:
	void CreateActions(QToolBar* toolBar);

	// Events
	//
protected:
	virtual void closeEvent(QCloseEvent*) override;
	virtual void keyPressEvent(QKeyEvent *e) override;

public slots:
	void projectOpened();
	void projectClosed();

	void onTabPageChanged();

	void loadSignals();
	void addSignal();
	void editSignal();
	void cloneSignal();
	void deleteSignal();
	void findAndReplaceSignal();

	void undoSignalChanges();
	void checkIn();
	void viewSignalHistory();

	DialogMetrologyConnection* createMetrologyDialog();
	void deleteMetrologyDialog();
	void openMetrologyConnections();
	void addMetrologyConnection();
	void metrologyDialogClosed();

	void changeSignalsLoadingSequence();

	void setSelection(const std::vector<int>& selectedRowsSignalID, int focusedCellSignalID = -1);
	void saveSelection();
	void restoreSelection(int focusedSignalId = -1);
	void onSignalSelectionChanged();

	void changeSignalTypeFilter(int selectedType);
	void changeSignalIdFilter(QStringList strIds, bool refreshSignalList);
	void applySignalIdFilter();
	void resetSignalIdFilter();

	void showError(QString message);

	void compareObject(DbChangesetObject object, CompareData compareData);

	// Data
	//
private:
	DbController* m_db = nullptr;
	static SignalsTabPage* m_instance;

	SignalsModel* m_signalsModel = nullptr;
	SignalsProxyModel* m_signalsProxyModel = nullptr;

	AppSignalSetProvider* m_signalSetProvider = nullptr;

	QTableView* m_signalsView = nullptr;
	TableDataVisibilityController* m_signalsColumnVisibilityController = nullptr;
	QComboBox* m_signalTypeFilterCombo = nullptr;
	QComboBox* m_signalIdFieldCombo = nullptr;
	QLineEdit* m_filterEdit = nullptr;
	QCompleter* m_completer = nullptr;
	QStringList m_filterHistory;
	int m_lastVerticalScrollPosition = -1;
	int m_lastHorizontalScrollPosition = -1;
	FindSignalDialog* m_findSignalDialog = nullptr;
	DialogMetrologyConnection* m_metrologyDialog = nullptr;
	QAction* m_addMetrologyConnectionAction = nullptr;

	std::vector<int> m_selectedRowsSignalID;
	int m_focusedCellSignalID = -1;
	int m_focusedCellColumn = -1;
};

class SignalsModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	SignalsModel(AppSignalSetProvider* signalSetProvider, SignalsTabPage* parent = nullptr);
	virtual ~SignalsModel() override;

	virtual int rowCount(const QModelIndex& parentIndex = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parentIndex = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex& index) const override;

	SignalsDelegate* createDelegate(SignalsProxyModel* signalsProxyModel);

	SignalsTabPage* parentWindow();

	void prepareForReset();
	void finishReset();

public slots:
	void slot_signalsUpdated(const std::vector<int>& indexes);
	void slot_signalsCountChanged();
	void beginIncreaseColumnCount(int newColumnCount);
	void beginDecreaseColumnCount(int newColumnCount);
	void endIncreaseColumnCount();
	void endDecreaseColumnCount();

private:
	AppSignalSetProvider* m_signalSetProvider = nullptr;
	SignalsTabPage* m_parentWindow;

	int m_rowCount = 0;
	int m_columnCount = 0;
};

class SignalsProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	SignalsProxyModel(SignalsModel* sourceModel, QObject* parent = nullptr);

	bool filterAcceptsRow(int sourceRow, const QModelIndex&) const override;
	bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;

	void setSignalTypeFilter(int signalType);
	void setSignalIdFilter(QStringList strIds);
	void setIdFilterField(int field);

signals:
	void aboutToSort();	// Before sorting or filtering signals should be fully loaded
	void aboutToFilter();

protected:
	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

private:
	void applyNewFilter();

	SignalsModel* m_sourceModel;
	AppSignalSetProvider* m_signalSetProvider;
	int m_signalType = SignalsTabPage::FILTER_ST_ANY;
	int m_idFilterField = SignalsTabPage::FILTER_STR_APP_SIGNAL_ID;
	QStringList m_strIdMasks;
};

class CheckedoutSignalsModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	CheckedoutSignalsModel(SignalsModel* sourceModel, QTableView* view, QObject* parent = nullptr);

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex & index) const override;

	bool filterAcceptsRow(int source_row, const QModelIndex&) const override;

	void initCheckStates(const QModelIndexList& list, bool fromSourceModel = true);
	void setAllCheckStates(bool state);
	void setCheckState(int row, Qt::CheckState state);

private:
	SignalsModel* m_sourceModel;
	QTableView* m_view;
	QVector<Qt::CheckState> states;
};



