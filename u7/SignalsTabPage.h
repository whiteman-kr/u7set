#pragma once

#include "MainTabPage.h"
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include "GlobalMessanger.h"
#include "../lib/Signal.h"

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


const int	ST_ANALOG = TO_INT(E::SignalType::Analog),
			ST_DISCRETE = TO_INT(E::SignalType::Discrete),
			ST_BUS = TO_INT(E::SignalType::Bus),
			ST_ANY = 0xff;


const int	FI_ANY = 0,
			FI_APP_SIGNAL_ID = 1,
			FI_CUSTOM_APP_SIGNAL_ID = 2,
			FI_EQUIPMENT_ID = 3,
			FI_CAPTION = 4;


class SignalsDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
	explicit SignalsDelegate(SignalSet& signalSet, SignalsModel* model, SignalsProxyModel* signalsProxyModel, QObject *parent = 0);

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
	SignalsModel(DbController* dbController, SignalsTabPage* parent = 0);
	virtual ~SignalsModel();

	virtual int rowCount(const QModelIndex& parentIndex = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex & index) const;

	SignalsDelegate* createDelegate(SignalsProxyModel* signalsProxyModel) { return new SignalsDelegate(m_signalSet, this, signalsProxyModel, parent()); }

	void clearSignals();

	Signal getSignalByID(int signalID) { return m_signalSet.value(signalID); }			// for debug purposes
	Signal* getSignalByStrID(const QString signalStrID);
	QVector<int> getChannelSignalsID(int signalGroupID) { return m_signalSet.getChannelSignalsID(signalGroupID); }
	int key(int row) const { return m_signalSet.key(row); }
	int keyIndex(int key) { return m_signalSet.keyIndex(key); }
	const Signal& signal(int row) const { return m_signalSet[row]; }
	QVector<int> getSameChannelSignals(int row);
	bool isEditableSignal(int row);

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
	QList<int> cloneSignals(const QSet<int>& signalIDs);
	void deleteSignalGroups(const QSet<int>& signalGroupIDs);
	void deleteSignals(const QSet<int>& signalIDs);
	void deleteSignal(int signalID);

signals:
	void setCheckedoutSignalActionsVisibility(bool state);
	void aboutToClearSignals();
	void signalsRestored(int focusedSignalId = -1);

public slots:
	void loadSignals();
	void loadSignalSet(QVector<int> keys, bool updateView = true);
	void loadSignal(int signalId, bool updateView = true);
	void addSignal();
	void showError(QString message);

private:
	// Data
	//
	SignalSet m_signalSet;
	QMap<int, QString> m_usernameMap;

	SignalsTabPage* m_parentWindow;
	DbController* m_dbController;
	static SignalsModel* m_instance;

	QString getSensorStr(int sensorID) const;
	QString getOutputModeStr(int outputMode) const;
	QString getUserStr(int userID) const;

	const QPixmap lock = QPixmap(":/Images/Images/lock.png");
	const QPixmap plus = QPixmap(":/Images/Images/plus.png");
	const QPixmap pencil = QPixmap(":/Images/Images/pencil.png");
	const QPixmap cross = QPixmap(":/Images/Images/cross.png");

	void changeCheckedoutSignalActionsVisibility();
};


class SignalsProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	SignalsProxyModel(SignalsModel* sourceModel, QObject* parent = 0);

	bool filterAcceptsRow(int source_row, const QModelIndex&) const override;
	void setSignalTypeFilter(int signalType);
	void setSignalIdFilter(QStringList strIds);
	void setIdFilterField(int field);

private:
	SignalsModel* m_sourceModel;
	int m_signalType = ST_ANY;
	int m_idFilterField = FI_EQUIPMENT_ID;
	QStringList m_strIdMasks;
};


class CheckedoutSignalsModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	CheckedoutSignalsModel(SignalsModel* sourceModel, QTableView* view, QObject* parent = 0);

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
	CheckinSignalsDialog(QString title, SignalsModel* sourceModel, QModelIndexList selection, bool showUndoButton = true, QWidget *parent = 0);

public slots:
	void checkinSelected();
	void undoSelected();
	void cancel();
	void openUndoDialog();

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
	UndoSignalsDialog(SignalsModel* sourceModel, QWidget *parent = 0);

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


class SignalsTabPage : public MainTabPage
{
	Q_OBJECT

public:
	SignalsTabPage(DbController* dbcontroller, QWidget* parent);
	virtual ~SignalsTabPage();

	static QStringList createSignal(DbController* dbController, const QStringList& lmIdList, int schemaCounter, const QString& schemaId, const QString& schemaCaption, const QString& appSignalId, QWidget* parent);

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

	void editSignal();
	void cloneSignal();
	void deleteSignal();

	void undoSignalChanges();
	void showPendingChanges();
	void checkIn();

	void editColumnsVisibilityAndOrder();
	void changeSignalActionsVisibility();

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
	SignalsModel* m_signalsModel = nullptr;
	SignalsProxyModel* m_signalsProxyModel = nullptr;
	QTableView* m_signalsView = nullptr;
	QComboBox* m_signalTypeFilterCombo = nullptr;
	QComboBox* m_signalIdFieldCombo = nullptr;
	QLineEdit* m_filterEdit = nullptr;
	QCompleter* m_completer = nullptr;
	QStringList m_filterHistory;
	int m_lastVerticalScrollPosition = -1;
	int m_lastHorizontalScrollPosition = -1;
	bool m_changingSelectionManualy = false;

	QList<int> m_selectedRowsSignalID;
	int m_focusedCellSignalID = -1;
	int m_focusedCellColumn = -1;

	void saveColumnVisibility(int index, bool visible);
	void saveColumnPosition(int index, int position);

private slots:
	void saveColumnWidth(int index);
};


