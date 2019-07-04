#pragma once

#include "MainTabPage.h"
#include "GlobalMessanger.h"
#include "../lib/SignalProperties.h"
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include <QDialog>

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


struct SignalPropertyDescription
{
	QString name;
	QString caption;
	QVariant::Type type;
	std::list<std::pair<int, QString>> enumValues;
	std::function<QVariant (const Signal*)> valueGetter;
	std::function<void (Signal*, const QVariant&)> valueSetter;
};


class SignalPropertyManager
{
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

public:
	SignalPropertyManager();

	// Data for models
	//

	int count() const;

	int index(const QString& name);
	QString caption(int propertyIndex) const;
	QString name(int propertyIndex);

	QVariant value(const Signal* signal, int propertyIndex) const;
	const std::list<std::pair<int, QString>> values(int propertyIndex) const;

	void setValue(Signal* signal, int propertyIndex, const QVariant& value);

	QVariant::Type type(const int propertyIndex) const;
	E::PropertyBehaviourType getBehaviour(const Signal& signal, const int propertyIndex) const;
	E::PropertyBehaviourType getBehaviour(E::SignalType type, E::SignalInOutType directionType, const int propertyIndex) const;
	bool dependsOnPrecision(const int propertyIndex) const;
	bool isHiddenFor(E::SignalType type, const int propertyIndex) const;
	bool isHidden(E::PropertyBehaviourType behaviour) const;
	bool isReadOnly(E::PropertyBehaviourType behaviour) const;

	void detectNewProperties(const Signal& signal);
	void reloadPropertyBehaviour(DbController* dbController, QWidget* parent);

private:
	bool isNotCorrect(int propertyIndex) const;
	QString typeName(E::SignalType type, E::SignalInOutType inOutType);
	QString typeName(int typeIndex, int inOutTypeIndex);

	static QString generateCaption(const QString& name);
	static TuningValue variant2TuningValue(const QVariant& variant, TuningValueType type);

	void addNewProperty(const SignalPropertyDescription& newProperty);

	static void trimm(QStringList& stringList);

	std::vector<PropertyBehaviourDescription> m_propertyBehaviorDescription;
	QHash<QString, int> m_propertyName2IndexMap;
	QHash<int, int> m_propertyIndex2BehaviourIndexMap;

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
		  [](Signal* s, QVariant v){ s->setAppSignalID(v.toString()); }, },

		{ SignalProperties::busTypeIDCaption,
		  SignalProperties::busTypeIDCaption,
		  QVariant::String, {},
		  [](const Signal* s){ return s->busTypeID(); },
		  [](Signal* s, QVariant v){ s->setBusTypeID(v.toString()); }, },

		{ SignalProperties::captionCaption,
		  SignalProperties::captionCaption,
		  QVariant::String, {},
		  [](const Signal* s){ return s->caption(); },
		  [](Signal* s, QVariant v){ s->setAppSignalID(v.toString()); }, },

		{ SignalProperties::typeCaption,
		  "A/D/B",
		  QVariant::String, {},
		  [](const Signal* s){ return E::valueToString<E::SignalType>(s->signalType()).left(1); },
		  nullptr },

		{ SignalProperties::analogSignalFormatCaption,
		  generateCaption(SignalProperties::analogSignalFormatCaption),
		  QVariant::Int, E::enumValues<E::AnalogAppSignalFormat>(),
		  [](const Signal* s){ return TO_INT(s->analogSignalFormat()); },
		  [](Signal* s, QVariant v){ s->setAnalogSignalFormat(IntToEnum<E::AnalogAppSignalFormat>(v.toInt())); }, },

		{ SignalProperties::inOutTypeCaption,
		  "Input-output type",
		  QVariant::Int, E::enumValues<E::SignalInOutType>(),
		  [](const Signal* s){ return TO_INT(s->inOutType()); },
		  [](Signal* s, QVariant v){ s->setInOutType(IntToEnum<E::SignalInOutType>(v.toInt())); }, },

		{ SignalProperties::byteOrderCaption,
		  generateCaption(SignalProperties::byteOrderCaption),
		  QVariant::Int, E::enumValues<E::ByteOrder>(),
		  [](const Signal* s){ return TO_INT(s->byteOrder()); },
		  [](Signal* s, QVariant v){ s->setByteOrder(IntToEnum<E::ByteOrder>(v.toInt())); }, },

		{ SignalProperties::dataSizeCaption,
		  generateCaption(SignalProperties::dataSizeCaption),
		  QVariant::Int, {},
		  [](const Signal* s){ return s->dataSize(); },
		  [](Signal* s, QVariant v){ s->setDataSize(v.toInt()); }, },

		{ SignalProperties::unitCaption,
		  SignalProperties::unitCaption,
		  QVariant::String, {},
		  [](const Signal* s){ return s->unit(); },
		  [](Signal* s, QVariant v){ s->setUnit(v.toString()); }, },

		{ SignalProperties::acquireCaption,
		  SignalProperties::acquireCaption,
		  QVariant::Bool, {},
		  [](const Signal* s){ return s->acquire(); },
		  [](Signal* s, QVariant v){ s->setAcquire(v.toBool()); }, },

		{ SignalProperties::archiveCaption,
		  SignalProperties::archiveCaption,
		  QVariant::Bool, {},
		  [](const Signal* s){ return s->archive(); },
		  [](Signal* s, QVariant v){ s->setArchive(v.toBool()); }, },

		{ SignalProperties::decimalPlacesCaption,
		  generateCaption(SignalProperties::decimalPlacesCaption),
		  QVariant::Int, {},
		  [](const Signal* s){ return s->decimalPlaces(); },
		  [](Signal* s, QVariant v){ s->setDecimalPlaces(v.toInt()); }, },

		{ SignalProperties::excludeFromBuildCaption,
		  generateCaption(SignalProperties::excludeFromBuildCaption),
		  QVariant::Bool, {},
		  [](const Signal* s){ return s->excludeFromBuild(); },
		  [](Signal* s, QVariant v){ s->setExcludeFromBuild(v.toBool()); }, },

		{ SignalProperties::adaptiveApertureCaption,
		  generateCaption(SignalProperties::adaptiveApertureCaption),
		  QVariant::Bool, {},
		  [](const Signal* s){ return s->adaptiveAperture(); },
		  [](Signal* s, QVariant v){ s->setAdaptiveAperture(v.toBool()); }, },

		{ SignalProperties::coarseApertureCaption,
		  generateCaption(SignalProperties::coarseApertureCaption),
		  QVariant::Double, {},
		  [](const Signal* s){ return s->coarseAperture(); },
		  [](Signal* s, QVariant v){ s->setCoarseAperture(v.toDouble()); }, },

		{ SignalProperties::fineApertureCaption,
		  generateCaption(SignalProperties::fineApertureCaption),
		  QVariant::Double, {},
		  [](const Signal* s){ return s->fineAperture(); },
		  [](Signal* s, QVariant v){ s->setFineAperture(v.toDouble()); }, },

		{ SignalProperties::specificPropertiesStructCaption,
		  generateCaption(SignalProperties::specificPropertiesStructCaption),
		  QVariant::String, {},
		  [](const Signal* s){ return s->specPropStruct(); },
		  [](Signal* s, QVariant v){ s->setSpecPropStruct(v.toString()); }, },

		{ SignalProperties::enableTuningCaption,
		  generateCaption(SignalProperties::enableTuningCaption),
		  QVariant::Bool, {},
		  [](const Signal* s){ return s->enableTuning(); },
		  [](Signal* s, QVariant v){ s->setEnableTuning(v.toBool()); }, },

		{ SignalProperties::tuningDefaultValueCaption,
		  generateCaption(SignalProperties::tuningDefaultValueCaption),
		  QVariant::String, {},
		  [](const Signal* s){ return s->tuningDefaultValue().toVariant(); },
		  [](Signal* s, QVariant v){ s->setTuningDefaultValue(variant2TuningValue(v, s->tuningDefaultValue().type())); }, },

		{ SignalProperties::tuningLowBoundCaption,
		  generateCaption(SignalProperties::tuningLowBoundCaption),
		  QVariant::String, {},
		  [](const Signal* s){ return s->tuningLowBound().toVariant(); },
		  [](Signal* s, QVariant v){ s->setTuningLowBound(variant2TuningValue(v, s->tuningDefaultValue().type())); }, },

		{ SignalProperties::tuningHighBoundCaption,
		  generateCaption(SignalProperties::tuningHighBoundCaption),
		  QVariant::String, {},
		  [](const Signal* s){ return s->tuningHighBound().toVariant(); },
		  [](Signal* s, QVariant v){ s->setTuningHighBound(variant2TuningValue(v, s->tuningDefaultValue().type())); }, },
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
	explicit SignalsDelegate(SignalSet& signalSet, SignalsModel* model, SignalsProxyModel* signalsProxyModel, QObject *parent = nullptr);

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
	SignalSet& m_signalSet;
	SignalsModel* m_model;
	SignalsProxyModel* m_proxyModel;
	mutable int signalIdForUndoOnCancelEditing = -1;
};


class SignalsModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	SignalsModel(DbController* dbController, SignalsTabPage* parent = nullptr);
	virtual ~SignalsModel() override;

	virtual int rowCount(const QModelIndex& parentIndex = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex & index) const override;

	SignalsDelegate* createDelegate(SignalsProxyModel* signalsProxyModel) { return new SignalsDelegate(m_signalSet, this, signalsProxyModel, parent()); }

	void clearSignals();

	const SignalSet& signalSet() const	{ return m_signalSet;	};

	Signal getSignalByID(int signalID) { return m_signalSet.value(signalID); }			// for debug purposes
	Signal* getSignalByStrID(const QString signalStrID);
	QVector<int> getChannelSignalsID(int signalGroupID) { return m_signalSet.getChannelSignalsID(signalGroupID); }
	int key(int row) const { return m_signalSet.key(row); }
	int keyIndex(int key) { return m_signalSet.keyIndex(key); }
	const Signal& signal(int row) const { return m_signalSet[row]; }
	QVector<int> getSameChannelSignals(int row);
	bool isEditableSignal(int row);

	SignalPropertyManager& signalPropertyManager() { return m_propertyManager; }

	DbController* dbController();
	const DbController* dbController() const;
	SignalsTabPage* parentWindow() { return m_parentWindow; }
	static SignalsModel* instance() { return m_instance; }
	QString errorMessage(const ObjectState& state) const;
	void showError(const ObjectState& state) const;
	void showErrors(const QVector<ObjectState>& states) const;
	bool checkoutSignal(int index);
	bool checkoutSignal(int index, QString& message);
	bool undoSignal(int id);
	bool editSignals(QVector<int> ids);
	static void trimSignalTextFields(Signal& signal);
	void saveSignal(Signal& signal);
	QVector<int> cloneSignals(const QSet<int>& signalIDs);
	void deleteSignalGroups(const QSet<int>& signalGroupIDs);
	void deleteSignals(const QSet<int>& signalIDs);
	void deleteSignal(int signalID);

signals:
	void setCheckedoutSignalActionsVisibility(bool state);
	void aboutToClearSignals();
	void signalsRestored(int focusedSignalId = -1);
	void signalsLoadingFinished();
	void updateColumnList();

public slots:
	void initLazyLoadSignals();
	void finishLoadSignals();
	void loadNextSignalsPortion();
	void loadUsers();
	void loadSignals();
	void loadSignalSet(QVector<int> keys, bool updateView = true);
	void loadSignal(int signalId, bool updateView = true);
	void addSignal();
	void showError(QString message);

private:
	void detectNewProperties(const Signal& signal);
	// Data
	//
	SignalPropertyManager m_propertyManager;
	SignalSet m_signalSet;
	QMap<int, QString> m_usernameMap;
	bool m_partialLoading = true;

	SignalsTabPage* m_parentWindow;
	DbController* m_dbController;
	static SignalsModel* m_instance;

	/*QString getSensorStr(int sensorID) const;
	QString getOutputModeStr(int outputMode) const;*/
	QString getUserStr(int userID) const;

	void changeCheckedoutSignalActionsVisibility();
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


class SignalsTabPage : public MainTabPage
{
	Q_OBJECT

public:
	SignalsTabPage(DbController* dbcontroller, QWidget* parent);
	virtual ~SignalsTabPage() override;

	static bool updateSignalsSpecProps(DbController* dbc, const QVector<Hardware::DeviceSignal*>& deviceSignalsToUpdate, const QStringList& forceUpdateProperties);
	int getMiddleVisibleRow();

protected:
	void CreateActions(QToolBar* toolBar);

	// Events
	//
protected:
	virtual void closeEvent(QCloseEvent*) override;
	virtual void keyPressEvent(QKeyEvent *e) override;

signals:
	void setSignalActionsVisibility(bool state);

public slots:
	void projectOpened();
	void projectClosed();

	void onTabPageChanged();
	void stopLoadingSignals();

	void editSignal();
	void cloneSignal();
	void deleteSignal();

	void undoSignalChanges();
	void checkIn();
	void viewSignalHistory();

	void changeSignalActionsVisibility();

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

	QVector<int> m_selectedRowsSignalID;
	int m_focusedCellSignalID = -1;
	int m_focusedCellColumn = -1;
};


