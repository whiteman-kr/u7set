#pragma once

#include "MainTabPage.h"
#include "GlobalMessanger.h"
#include "../lib/SignalProperties.h"
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include <QDialog>
#include <QHash>
#include "DialogMetrologyConnection.h"

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
class SignalSetProvider;


const int	ST_ANALOG = TO_INT(E::SignalType::Analog),
			ST_DISCRETE = TO_INT(E::SignalType::Discrete),
			ST_BUS = TO_INT(E::SignalType::Bus),
			ST_ANY = 0xff;


const int	FI_ANY = 0,
			FI_APP_SIGNAL_ID = 1,
			FI_CUSTOM_APP_SIGNAL_ID = 2,
			FI_EQUIPMENT_ID = 3,
			FI_CAPTION = 4;


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
	explicit SignalsDelegate(SignalSetProvider* signalSetProvider, SignalsModel* model, SignalsProxyModel* signalsProxyModel, QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

signals:
	void itemDoubleClicked();

public slots:
	void onCloseEditorEvent(QWidget* editor, EndEditHint hint);

protected:
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

private:
	SignalSetProvider* m_signalSetProvider;
	SignalsModel* m_model;
	SignalsProxyModel* m_proxyModel;
	mutable int signalIdForUndoOnCancelEditing = -1;
};


class SignalsModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	SignalsModel(SignalSetProvider* signalSetProvider, SignalsTabPage* parent = nullptr);
	virtual ~SignalsModel() override;

	virtual int rowCount(const QModelIndex& parentIndex = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parentIndex = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex & index) const override;

	SignalsDelegate* createDelegate(SignalsProxyModel* signalsProxyModel) { return new SignalsDelegate(m_signalSetProvider, this, signalsProxyModel, parent()); }

	SignalsTabPage* parentWindow() { return m_parentWindow; }

	void prepareForReset() { beginResetModel(); }
	void finishReset()
	{
		m_rowCount = 0;
		m_columnCount = 0;
		endResetModel();
	}


public slots:
	void updateSignal(int signalIndex);
	void changeRowCount();
	void changeColumnCount();

private:
	// Data
	//
	SignalSetProvider* m_signalSetProvider;
	int m_rowCount = 0;
	int m_columnCount = 0;

	SignalsTabPage* m_parentWindow;
	QString getUserStr(int userId) const;
};


class SignalsProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	SignalsProxyModel(SignalsModel* sourceModel, QObject* parent = nullptr);

	bool filterAcceptsRow(int source_row, const QModelIndex&) const override;
	bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

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
	SignalSetProvider* m_signalSetProvider;
	int m_signalType = ST_ANY;
	int m_idFilterField = FI_EQUIPMENT_ID;
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


class CheckinSignalsDialog : public QDialog
{
	Q_OBJECT
public:
	CheckinSignalsDialog(SignalsModel* sourceModel, TableDataVisibilityController* columnManager, QModelIndexList selection, QWidget *parent = nullptr);

public slots:
	void checkinSelected();
	void cancel();

protected:
	void closeEvent(QCloseEvent* event);

private:
	SignalsModel *m_sourceModel;
	CheckedoutSignalsModel* m_proxyModel;
	QTableView* m_signalsView = nullptr;
	QPlainTextEdit* m_commentEdit;
	QSplitter* m_splitter;

	void saveDialogGeometry();
};


class UndoSignalsDialog : public QDialog
{
	Q_OBJECT
public:
	UndoSignalsDialog(SignalsModel* sourceModel, TableDataVisibilityController* columnManager, QWidget *parent = nullptr);

	void setCheckStates(QModelIndexList selection, bool fromSourceModel);
	void saveDialogGeometry();

public slots:
	void undoSelected();

protected:
	void closeEvent(QCloseEvent* event);

private:
	SignalsModel *m_sourceModel;
	CheckedoutSignalsModel* m_proxyModel;
};


class SignalHistoryDialog : public QDialog
{
	Q_OBJECT
public:
	SignalHistoryDialog(DbController* dbController, const QString& appSignalId, int signalId, QWidget *parent = nullptr);

protected:
	void closeEvent(QCloseEvent* event);

private:
	DbController* m_dbController = nullptr;
	QStandardItemModel* m_historyModel = nullptr;
	int m_signalId = -1;
};


class FindSignalDialog : public QDialog
{
	Q_OBJECT

	class SearchOptions
	{
	public:
		QString findString;
		int searchedPropertyIndex;
		bool searchInSelected;
		bool caseSensitive;
		bool wholeWords;

		bool operator==(const SearchOptions &other) const {
			return findString == other.findString &&
					searchedPropertyIndex == other.searchedPropertyIndex &&
					searchInSelected == other.searchInSelected &&
					caseSensitive == other.caseSensitive &&
					wholeWords == other.wholeWords;
		}
	};

	static const QString notUniqueMessage;
	static const QString notEditableMessage;
	static const QString notCorrectIdMessage;
	static const QString cannotCheckoutMessage;
	static const QString replaceableMessage;
	static const QString replacedMessage;

public:
	FindSignalDialog(int currentUserId, bool currentUserIsAdmin, QTableView* parent = nullptr);
	void notifyThatSignalSetHasChanged();

	bool shouldReopen() { return m_shouldReopen; }
	void allowReopen() { m_shouldReopen = true; }

signals:
	void signalSelected(int signalId);

protected:
	void closeEvent(QCloseEvent* event);

private:
	void addSignalIfNeeded(const Signal& signal);
	bool match(QString signalProperty, int& start, int& end);
	bool checkForEditableSignal(const Signal& signal);
	bool checkForUniqueSignalId(const QString& original, const QString& replaced);
	bool checkForCorrectSignalId(const QString& replaced);
	SearchOptions getCurrentSearchOptions();
	QString getProperty(const Signal& signal);
	void setProperty(Signal& signal, const QString& value);
	int getSignalId(int row);
	int getSelectedRow();
	void selectRow(int row);
	bool isReplaceable(int row);
	void replace(int row);
	void reloadCurrentIdsMap();
	void markFistInstancesIfItTheyNotUnique();
	void generateListIfNeeded(bool throwWarning = true);

	void updateCounters();

	void saveDialogGeometry();

private slots:
	void generateListIfNeededWithWarning();
	void updateAllReplacement();
	void updateReplacement(int row);
	void updateReplacement(const Signal& signal, int row);
	void replaceAll();
	void replaceAndFindNext();
	void findPrevious();
	void findNext();
	void selectCurrentSignalOnAppSignalsTab();
	void blinkReplaceableSignalQuantity();

private:
	QTableView* m_signalTable = nullptr;
	SignalsProxyModel* m_signalProxyModel = nullptr;
	SignalsModel* m_signalModel = nullptr;

	SignalSetProvider* m_signalSetProvider = nullptr;

	QLineEdit* m_findString = nullptr;
	QLineEdit* m_replaceString = nullptr;

	QComboBox* m_searchInPropertyList = nullptr;

	QCheckBox* m_caseSensitive = nullptr;
	QCheckBox* m_wholeWords = nullptr;
	QCheckBox* m_searchInSelected = nullptr;

	QLabel* m_signalsQuantityLabel = nullptr;
	QLabel* m_canBeReplacedQuantityLabel = nullptr;

	QTableView* m_foundList = nullptr;
	QStandardItemModel* m_foundListModel = nullptr;

	QPushButton* m_replaceAllButton = nullptr;
	QPushButton* m_replaceAndFindNextButton = nullptr;
	QPushButton* m_findPreviousButton = nullptr;
	QPushButton* m_findNextButton = nullptr;

	int m_totalSignalQuantity = 0;
	int m_replaceableSignalQuantity = 0;
	bool m_checkCorrectnessOfId = false;
	QTimer* m_replaceableSignalQuantityBlinkTimer = nullptr;
	bool m_replaceableSignalQuantityBlinkIsOn = false;

	SearchOptions m_searchOptionsUsedLastTime;
	bool m_isMatchToCurrentSignalSet = false;
	QSet<QString> m_signalIds;
	QSet<QString> m_repeatedSignalIds;
	QRegExp m_regExp4Id;
	int m_currentUserId = -1;
	bool m_currentUserIsAdmin = false;
	bool m_shouldReopen = true;
};


class SignalsTabPage : public MainTabPage
{
	Q_OBJECT

public:
	SignalsTabPage(SignalSetProvider* signalSetProvider, DbController* dbController, QWidget* parent);
	virtual ~SignalsTabPage() override;

	static bool updateSignalsSpecProps(DbController* dbc, const QVector<Hardware::DeviceSignal*>& deviceSignalsToUpdate, const QStringList& forceUpdateProperties);
	int getMiddleVisibleRow();
	bool editSignals(QVector<int> ids);

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
	void updateFindOrReplaceDialog();

	void undoSignalChanges();
	void checkIn();
	void viewSignalHistory();

	bool createMetrologyDialog();
	void deleteMetrologyDialog();
	void openMetrologyConnections();
	void addMetrologyConnection();
	void metrologyDialogClosed();

	void changeLazySignalLoadingSequence();

	void setSelection(const QVector<int> &selectedRowsSignalID, int focusedCellSignalID = -1);
	void saveSelection();
	void restoreSelection(int focusedSignalId = -1);

	void changeSignalTypeFilter(int selectedType);
	void changeSignalIdFilter(QStringList strIds, bool refreshSignalList);
	void applySignalIdFilter();
	void resetSignalIdFilter();

	void showError(QString message);

	void compareObject(DbChangesetObject object, CompareData compareData);

	// Data
	//
private:
	static SignalsTabPage* m_instance;
	SignalsModel* m_signalsModel = nullptr;
	SignalSetProvider* m_signalSetProvider = nullptr;
	SignalsProxyModel* m_signalsProxyModel = nullptr;
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

	QVector<int> m_selectedRowsSignalID;
	int m_focusedCellSignalID = -1;
	int m_focusedCellColumn = -1;
};


