#pragma once

#include "MainTabPage.h"
#include <QAbstractTableModel>
#include "../include/Signal.h"

class DbController;
class QTableView;


class SignalsModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	SignalsModel(QObject* parent = 0);
	virtual ~SignalsModel();

	virtual int rowCount(const QModelIndex& parentIndex = QModelIndex()) const override;
	virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) override;
	Qt::ItemFlags flags(const QModelIndex & index) const;

signals:
	void signalsIdRequest();
	void signalDataRequest(int id);
	void signalChanged(Signal signal);
	void signalAdded(Signal signal);

public slots:
	void signalsIdReceived(QVector<int> signalsId);
	void signalDataReceived(Signal signal);

private:
	// Data
	//
	QVector<Signal> m_signals;
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

	// Data
	//
private:
	//QAction* m_addSystemAction = nullptr;
	//QAction* m_addCaseAction = nullptr;

	//QTextEdit* m_propertyView = nullptr;
	//QSplitter* m_splitter = nullptr;
	SignalsModel* m_signalsModel = nullptr;
	QTableView* m_signalsView = nullptr;
};


