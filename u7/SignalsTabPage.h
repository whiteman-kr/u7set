#pragma once

#include "MainTabPage.h"
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>
#include "../include/Signal.h"

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


const int	ST_ANALOG = TO_INT(SignalType::Analog),
			ST_DISCRETE = TO_INT(SignalType::Discrete),
			ST_ANY = 0xff;


class SignalsDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SignalsDelegate(DataFormatList& dataFormatInfo, UnitList& unitInfo, SignalSet& signalSet, SignalsModel* model, SignalsProxyModel* signalsProxyModel, QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

signals:
	void itemDoubleClicked(int row);

public slots:

protected:
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

private:
	const DataFormatList& m_dataFormatInfo;
	const UnitList& m_unitInfo;
	SignalSet& m_signalSet;
	SignalsModel* m_model;
	SignalsProxyModel* m_proxyModel;
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

	SignalsDelegate* createDelegate(SignalsProxyModel* signalsProxyModel) { return new SignalsDelegate(m_dataFormatInfo, m_unitInfo, m_signalSet, this, signalsProxyModel, parent()); }

	void clearSignals();

	Signal getSignalByID(int signalID) { return m_signalSet.value(signalID); }			// for debug purposes
	int key(int row) const { return m_signalSet.key(row); }
	int keyIndex(int key) { return m_signalSet.keyIndex(key); }
	const Signal& signal(int row) const { return m_signalSet[row]; }
	QVector<int> getSameChannelSignals(int row);
	bool isEditableSignal(int row);

	DbController* dbController();
	const DbController* dbController() const;
	SignalsTabPage* parrentWindow() { return m_parentWindow; }
	QString errorMessage(const ObjectState& state) const;
	void showError(const ObjectState& state) const;
	void showErrors(const QVector<ObjectState>& states) const;
	bool checkoutSignal(int index);
	bool checkoutSignal(int index, QString& message);
	bool editSignal(int row);
	bool editSignals(QVector<int> ids);
	void saveSignal(Signal& signal);
	void deleteSignalGroups(const QSet<int>& signalGroupIDs);
	void deleteSignal(int signalID);

signals:
	void setCheckedoutSignalActionsVisibility(bool state);
	void aboutToClearSignals();
	void signalsRestored();
	void cellsSizeChanged();
	void signalActivated(int row);

public slots:
	void loadSignals();
	void loadSignalSet(QVector<int> keys, bool updateView = true);
	void loadSignal(int row, bool updateView = true);
	void addSignal();
	void showError(QString message);

private:
	// Data
	//
	SignalSet m_signalSet;
	DataFormatList m_dataFormatInfo;
	UnitList m_unitInfo;
	QMap<int, QString> m_usernameMap;

	SignalsTabPage* m_parentWindow;
	DbController* m_dbController;

	QString getUnitStr(int unitID) const;
	QString getSensorStr(int sensorID) const;
	QString getOutputRangeModeStr(int outputRangeMode) const;
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

private:
	SignalsModel* m_sourceModel;
	int m_signalType = ST_ANY;
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
    CheckinSignalsDialog(SignalsModel* sourceModel, QModelIndexList selection, QWidget *parent = 0);

public slots:
	void checkinSelected();
	void undoSelected();
	void cancel();
	void openUndoDialog();

private:
	SignalsModel *m_sourceModel;
	CheckedoutSignalsModel* m_proxyModel;
	QTableView* m_signalsView = nullptr;
	QPlainTextEdit* m_commentEdit;
	QSplitter* m_splitter;

	void saveGeometry();
};


class UndoSignalsDialog : public QDialog
{
	Q_OBJECT
public:
	UndoSignalsDialog(SignalsModel* sourceModel, QWidget *parent = 0);

    void setCheckStates(QModelIndexList selection, bool fromSourceModel);

public slots:
	void undoSelected();

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

protected:
	void CreateActions(QToolBar* toolBar);

	// Events
	//
protected:
	virtual void closeEvent(QCloseEvent*) override;

signals:
	void setSignalActionsVisibility(bool state);

public slots:
	void projectOpened();
	void projectClosed();

	void editSignal();
	void deleteSignal();

	void undoSignalChanges();
	void showPendingChanges();

	void updateCellsSize();
	void saveScrollPosition();
	void checkScrollPosition();

	void changeSignalActionsVisibility();

	void saveSelection();
	void restoreSelection();

	void changeSignalTypeFilter(int selectedType);

	void showError(QString message);

	// Data
	//
private:
	SignalsModel* m_signalsModel = nullptr;
	SignalsProxyModel* m_signalsProxyModel = nullptr;
	QTableView* m_signalsView = nullptr;
	QComboBox* m_signalTypeFilterCombo = nullptr;
	QCheckBox* m_autoResizeToContents = nullptr;
	int m_lastVerticalScrollPosition = -1;
	int m_lastHorizontalScrollPosition = -1;
	QTimer* m_scrollTimer = nullptr;

	QList<int> m_selectedRowsSignalID;
	int m_focusedCellSignalID = -1;
	int m_focusedCellColumn = -1;
};


