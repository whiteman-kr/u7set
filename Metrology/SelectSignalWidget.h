#ifndef SELECTSIGNALWIDGET_H
#define SELECTSIGNALWIDGET_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QKeyEvent>
#include <QAbstractTableModel>
#include <QHeaderView>
#include <QTableView>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QString>

#include "SignalBase.h"

// ==============================================================================================

class SelectSignalItem
{
public:

	explicit SelectSignalItem();
	explicit SelectSignalItem(const SelectSignalItem& signal);
	explicit SelectSignalItem(int index, Metrology::ConnectionType connectionType, const MeasureSignal& measureSignal);
	virtual ~SelectSignalItem() {}

public:

	void clear();
	bool isValid() const;

	bool set(int index, Metrology::ConnectionType connectionType, const MeasureSignal& measureSignal);

	int index() const { return m_index; }
	void setIndex(int index) { m_index = index; }

	Metrology::ConnectionType connectionType() const { return m_connectionType; }
	void setConnectionType(Metrology::ConnectionType type) { m_connectionType = type; }

	QString signalId(int ioType) const;
	void setSignalId(int ioType, const QString& signalId);

	QString caption(int ioType) const;
	void setCaption(int ioType, const QString& caption);

private:

	int m_index = -1;													// index MeasureSignal in the  SignalBase in the array SignalListForMeasure
	Metrology::ConnectionType m_connectionType = Metrology::ConnectionType::NoConnectionType;

	QString m_signalId[Metrology::ConnectionIoTypeCount];
	QString m_caption[Metrology::ConnectionIoTypeCount];
};

// ==============================================================================================

class SelectSignalTable;

//SelectSignalWidget -- widget to add to ToolBar
//
class SelectSignalWidget : public QWidget
{
	Q_OBJECT

public:

	SelectSignalWidget(QWidget* parent = nullptr);
	virtual ~SelectSignalWidget() {}

public:

	void clear();
	int count() { return  static_cast<int>(m_signalList.size()); }
	bool isEmpty() { return m_signalList.size() == 0; }

	bool setCurrentSignalIndex(const QString& signalId);
	int currentSignalIndex() const;

	bool addSignal(const SelectSignalItem& signal);
	int setSignalList(const std::vector<SelectSignalItem>& signalList, const QString& defaultSignalId);

	void updateActiveOutputSignal(const MeasureSignal& activeSignal);

private:

	int m_currentSignalIndex = -1;
	std::vector<SelectSignalItem> m_signalList;

	QPushButton* m_button = nullptr;

	int m_lastPopupHeight = 0;
	int m_lastPopupWidth = 0;

signals:

	void selectionChanged(int signalIndex);

protected slots:

	void slot_buttonClicked();

public slots:

	void activeSignalChanged(const MeasureSignal& activeSignal);
};

// ==============================================================================================
//SelectSignalPopup -- popup dialog with signal list and filter edit box
//
class SelectSignalPopup : public QDialog
{
	Q_OBJECT

public:

	SelectSignalPopup(int defaultSignalIndex, const std::vector<SelectSignalItem>& signalList, QWidget* parent);

public:

	int selectedSignalIndex() const;

private:

	std::vector<SelectSignalItem> m_signalList;

	QLineEdit* m_edit = nullptr;
	SelectSignalTable* m_tableWidget = nullptr;

	int m_selectedSignalIndex = -1;

	static int m_lastTimeHeigh;

protected:

	virtual void showEvent(QShowEvent* event) override;
	virtual	void keyPressEvent(QKeyEvent* event) override;
	void fillList(int selectSignalIndex);

private slots:

	void filterTextChanged();
	void listCellClicked(const QModelIndex& index);
};

// ==============================================================================================
//SelectSignalModel -- Simple model forSelectSignalTable
//
class SelectSignalModel : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit SelectSignalModel(const std::vector<SelectSignalItem>& signalList, QObject* parent = nullptr);

public:

	virtual int rowCount(const QModelIndex &parent) const override;
	virtual int columnCount(const QModelIndex &parent) const override;
	virtual QVariant data(const QModelIndex &modelIndex, int role) const override;

	int applyFilter(QString filterText, int defaultSignalIndex);

private:

	const std::vector<SelectSignalItem>& m_signalList;
	std::vector<int> m_filteredItems;
};

// ==============================================================================================
//SelectSignalTable -- Tbale widget for selection signal
//
class SelectSignalTable : public QTableView
{
	Q_OBJECT

public:

	SelectSignalTable(const std::vector<SelectSignalItem>& signalList, int defaultSignalIndex, QWidget* parent);

public:

	void applyFilter(QString filter, int defaultSignalIndex);

private:

	const std::vector<SelectSignalItem>& m_signalList;
	SelectSignalModel* m_model = nullptr;

protected slots:

	void mouseOverItem(const QModelIndex& index);
	virtual void currentChanged(const QModelIndex& current, const QModelIndex& previous) override;
};

// ==============================================================================================

#endif // SELECTSIGNALWIDGET_H
