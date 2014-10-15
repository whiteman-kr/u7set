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


class SignalsDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SignalsDelegate(DataFormatList& dataFormatInfo, UnitList& unitInfo, SignalSet& signalSet, SignalsModel* model, QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

signals:

public slots:

private:
	const DataFormatList& m_dataFormatInfo;
	const UnitList& m_unitInfo;
	SignalSet& m_signalSet;
	SignalsModel* m_model;
};


class SignalsModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	SignalsModel(DbController* dbController, QWidget* parent = 0);
	virtual ~SignalsModel();

	virtual int rowCount(const QModelIndex& parentIndex = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex & index) const;

	SignalsDelegate* createDelegate() { return new SignalsDelegate(m_dataFormatInfo, m_unitInfo, m_signalSet, this, parent()); }

	void clearSignals();

	Signal getSignalByID(int signalID) { return m_signalSet.value(signalID); }			// for debug purposes
	int key(int row) const { return m_signalSet.key(row); }
	const Signal& signal(int row) const { return m_signalSet[row]; }
	bool isEditableSignal(int row);

	DbController* dbController();
	const DbController* dbController() const;
	QWidget* parrentWindow() { return m_parentWindow; }
	void showError(const ObjectState& state) const;
	bool checkoutSignal(int index);
	bool editSignal(int row);
	void deleteSignal(int row);

signals:
	void cellsSizeChanged();
	void setCheckedoutSignalActionsVisibility(bool state);

public slots:
	void loadSignals();
	void addSignal();

private:
	// Data
	//
	SignalSet m_signalSet;
	DataFormatList m_dataFormatInfo;
	UnitList m_unitInfo;
	QMap<int, QString> m_usernameMap;

	QWidget* m_parentWindow;
	DbController* m_dbController;

	QString getUnitStr(int unitID) const;
	QString getSensorStr(int sensorID) const;
	QString getUserStr(int userID) const;

	void changeCheckedoutSignalActionsVisibility();
};


class CheckedoutSignalsModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	CheckedoutSignalsModel(SignalsModel* sourceModel, QObject* parent = 0);

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex & index) const override;

	bool filterAcceptsRow(int source_row, const QModelIndex&) const override;

	void initCheckStates(const QModelIndexList& list);
	void setAllCheckStates(bool state);

private:
	SignalsModel* m_sourceModel;
	QVector<Qt::CheckState> states;
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

	void changeSignalActionsVisibility();

	// Data
	//
private:
	SignalsModel* m_signalsModel = nullptr;
	QTableView* m_signalsView = nullptr;
	QMenu* m_signalsMenu = nullptr;
};


