#pragma once

#include "MainTabPage.h"
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include "../include/Signal.h"

class DbController;
class QTableView;
class QMenu;


class SignalsDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SignalsDelegate(DataFormatList& dataFormatInfo, UnitList& unitInfo, SignalSet& signalSet, QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

signals:

public slots:

private:
	DataFormatList& m_dataFormatInfo;
	UnitList& m_unitInfo;
	SignalSet& m_signalSet;
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

	void loadSignals();

	Signal getSignalByID(int signalID) { return m_signalSet.value(signalID); }			// for debug purposes

signals:
	void cellsSizeChanged();

public slots:
	void addSignal();

protected:
	DbController* dbController();

private:
	// Data
	//
	SignalSet m_signalSet;
	DataFormatList m_dataFormatInfo;
	UnitList m_unitInfo;

	QWidget* m_parentWindow;
	DbController* m_dbController;

	QString getUnitStr(int unitID) const;
};


class SignalsTabPage : public MainTabPage
{
	Q_OBJECT

public:
	SignalsTabPage(DbController* dbcontroller, QWidget* parent);
	virtual ~SignalsTabPage();

protected:
	void CreateActions();

	// Events
	//
protected:
	virtual void closeEvent(QCloseEvent*) override;

public slots:
	void projectOpened();
	void projectClosed();
	void contextMenuRequested(QPoint);

	// Data
	//
private:
	//QAction* m_addSystemAction = nullptr;
	//QAction* m_addCaseAction = nullptr;

	//QTextEdit* m_propertyView = nullptr;
	//QSplitter* m_splitter = nullptr;
	SignalsModel* m_signalsModel = nullptr;
	QTableView* m_signalsView = nullptr;
	QMenu* m_signalsMenu = nullptr;
};


