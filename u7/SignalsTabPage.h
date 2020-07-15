#pragma once

#include "MainTabPage.h"
#include "GlobalMessanger.h"
#include "../lib/SignalProperties.h"
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include <QDialog>
#include <QHash>

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


#define SIGNAL_TYPE_COUNT (QMetaEnum::fromType<E::SignalType>().keyCount())
#define IN_OUT_TYPE_COUNT (QMetaEnum::fromType<E::SignalInOutType>().keyCount())
#define TOTAL_SIGNAL_TYPE_COUNT (SIGNAL_TYPE_COUNT * IN_OUT_TYPE_COUNT)


const int	ST_ANALOG = TO_INT(E::SignalType::Analog),
			ST_DISCRETE = TO_INT(E::SignalType::Discrete),
			ST_BUS = TO_INT(E::SignalType::Bus),
			ST_ANY = 0xff;


const int	FI_ANY = 0,
			FI_APP_SIGNAL_ID = 1,
			FI_CUSTOM_APP_SIGNAL_ID = 2,
			FI_EQUIPMENT_ID = 3,
			FI_CAPTION = 4;


class SignalPropertyManager : public QObject
{
	Q_OBJECT
public:
	struct PropertyBehaviourDescription
	{
		QString name;
		bool dependsOnPrecision = false;
		std::vector<E::PropertyBehaviourType> behaviourType = std::vector<E::PropertyBehaviourType>(
					static_cast<size_t>(TOTAL_SIGNAL_TYPE_COUNT),
					E::PropertyBehaviourType::Write);
	};

	static const E::PropertyBehaviourType defaultBehaviour = E::PropertyBehaviourType::Write;

signals:
	void propertyCountChanged();

public:
	SignalPropertyManager(DbController* dbController, QWidget* parentWidget);

	static SignalPropertyManager* getInstance();

	// Data for models
	//

	int count() const;

	int index(const QString& name);
	QString caption(int propertyIndex) const;
	QString name(int propertyIndex);

	QVariant value(const Signal* signal, int propertyIndex) const;
	const std::vector<std::pair<int, QString>> values(int propertyIndex) const;

	void setValue(Signal* signal, int propertyIndex, const QVariant& value);

	QVariant::Type type(const int propertyIndex) const;
	E::PropertyBehaviourType getBehaviour(const Signal& signal, const int propertyIndex) const;
	E::PropertyBehaviourType getBehaviour(E::SignalType type, E::SignalInOutType directionType, const int propertyIndex) const;
	bool dependsOnPrecision(const int propertyIndex) const;
	bool isHiddenFor(E::SignalType type, const int propertyIndex) const;
	bool isHidden(E::PropertyBehaviourType behaviour) const;
	bool isReadOnly(E::PropertyBehaviourType behaviour) const;

	void loadNotSpecificProperties();
	void reloadPropertyBehaviour();

public slots:
	void detectNewProperties(const Signal& signal);

private:
	bool isNotCorrect(int propertyIndex) const;
	QString typeName(E::SignalType type, E::SignalInOutType inOutType);
	QString typeName(int typeIndex, int inOutTypeIndex);

	static TuningValue variant2TuningValue(const QVariant& variant, TuningValueType type);

	void addNewProperty(const SignalPropertyDescription& newProperty);

	static void trimm(QStringList& stringList);

	std::vector<PropertyBehaviourDescription> m_propertyBehaviorDescription;
	QHash<QString, int> m_propertyName2IndexMap;
	QHash<int, int> m_propertyIndex2BehaviourIndexMap;

	DbController* m_dbController;
	QWidget* m_parentWidget;
	static SignalPropertyManager* m_instance;

	// is initialized by non specific properties
	//
	std::vector<SignalPropertyDescription> m_propertyDescription = {
		{ SignalProperties::appSignalIDCaption,
		  SignalProperties::appSignalIDCaption,
		  QVariant::String, {},
		  [](const Signal* s){ return s->appSignalID(); },
		  [](Signal* s, QVariant v){ s->setAppSignalID(v.toString()); }, },

		{ SignalProperties::customSignalIDCaption,
		  SignalProperties::customSignalIDCaption,
		  QVariant::String, {},
		  [](const Signal* s){ return s->customAppSignalID(); },
		  [](Signal* s, QVariant v){ s->setCustomAppSignalID(v.toString()); }, },

		{ SignalProperties::equipmentIDCaption,
		  SignalProperties::equipmentIDCaption,
		  QVariant::String, {},
		  [](const Signal* s){ return s->equipmentID(); },
		  [](Signal* s, QVariant v){ s->setEquipmentID(v.toString()); }, },

		{ SignalProperties::busTypeIDCaption,
		  SignalProperties::busTypeIDCaption,
		  QVariant::String, {},
		  [](const Signal* s){ return s->busTypeID(); },
		  [](Signal* s, QVariant v){ s->setBusTypeID(v.toString()); }, },

		{ SignalProperties::typeCaption,
		  "A/D/B",
		  QVariant::String, {},
		  [](const Signal* s){ return E::valueToString<E::SignalType>(s->signalType()).left(1); },
		  nullptr },

		{ SignalProperties::inOutTypeCaption,
		  "Input-output type",
		  QVariant::Int, E::enumValues<E::SignalInOutType>(),
		  [](const Signal* s){ return TO_INT(s->inOutType()); },
		  [](Signal* s, QVariant v){ s->setInOutType(IntToEnum<E::SignalInOutType>(v.toInt())); }, },
	};
};


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
	SignalsModel(SignalSetProvider* signalSetProvider, DbController* dbController, SignalsTabPage* parent = nullptr);
	virtual ~SignalsModel() override;

	virtual int rowCount(const QModelIndex& parentIndex = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parentIndex = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex & index) const override;

	SignalsDelegate* createDelegate(SignalsProxyModel* signalsProxyModel) { return new SignalsDelegate(m_signalSetProvider, this, signalsProxyModel, parent()); }

	SignalPropertyManager& signalPropertyManager() { return m_propertyManager; }

	SignalsTabPage* parentWindow() { return m_parentWindow; }

signals:
	void aboutToClearSignals();
	void signalsRestored(int focusedSignalId = -1);
	void signalsLoadingFinished();
	void updateColumnList();

public slots:
	void updateSignalsPropertyBehaviour();
	void updateSignal(int signalIndex);
	void changeRowCount();
	void changeColumnCount();

private:
	// Data
	//
	SignalPropertyManager m_propertyManager;
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
	void aboutToSort();
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
	SignalSetProvider* m_signalSetProvider;
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
	SignalSetProvider* m_signalSetProvider;
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
	SignalSetProvider* m_signalSetProvider;
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

	bool reopen() { return m_reopen; }
	void setReopen() { m_reopen = true; }

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
	bool m_reopen = true;
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

signals:
	void setSignalActionsVisibility(bool state);
	void setCheckedoutSignalActionsVisibility(bool state);

public slots:
	void projectOpened();
	void projectClosed();

	void onTabPageChanged();
	void stopLoadingSignals();

	void addSignal();
	void editSignal();
	void cloneSignal();
	void deleteSignal();
	void findAndReplaceSignal();
	void updateFindOrReplaceDialog();

	void undoSignalChanges();
	void checkIn();
	void viewSignalHistory();

	void changeSignalActionsVisibility();
	void changeCheckedoutSignalActionsVisibility();
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
	QTabWidget* m_tabWidget = nullptr;
	QTimer* m_loadSignalsTimer = nullptr;
	QTableView* m_signalsView = nullptr;
	TableDataVisibilityController* m_signalsColumnVisibilityController = nullptr;
	QComboBox* m_signalTypeFilterCombo = nullptr;
	QComboBox* m_signalIdFieldCombo = nullptr;
	QLineEdit* m_filterEdit = nullptr;
	QCompleter* m_completer = nullptr;
	QStringList m_filterHistory;
	int m_lastVerticalScrollPosition = -1;
	int m_lastHorizontalScrollPosition = -1;
	bool m_changingSelectionManualy = false;
	FindSignalDialog* m_findSignalDialog = nullptr;

	QVector<int> m_selectedRowsSignalID;
	int m_focusedCellSignalID = -1;
	int m_focusedCellColumn = -1;
};


