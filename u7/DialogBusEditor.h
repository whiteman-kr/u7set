#ifndef DIALOGBUSEDITOR_H
#define DIALOGBUSEDITOR_H

#include "../lib/PropertyEditor.h"
#include "../lib/PropertyEditorDialog.h"
#include "../lib/PropertyEditorDialog.h"

#include "BusStorage.h"

#include <QMessageBox>
#include <QItemDelegate>

class DialogBusEditorDelegate: public QItemDelegate
{
	Q_OBJECT

public:
	DialogBusEditorDelegate(QObject *parent);
	QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
	void setEditorData(QWidget *editor, const QModelIndex &index) const override;

};


class DialogBusEditor : public QDialog
{
	Q_OBJECT
public:
	DialogBusEditor(DbController *pDbController, QWidget* parent);
	~DialogBusEditor();

private slots:

	void onAdd();
	void onRemove();
	void onCheckOut();
	void onCheckIn();
	void onUndo();
	void onRefresh();

	void onBusItemChanged(QTreeWidgetItem *item, int column);
	void onBusItemSelectionChanged();
	void onBusCustomContextMenuRequested(const QPoint& pos);
	void onBusSortIndicatorChanged(int column, Qt::SortOrder order);


protected:
	virtual void closeEvent(QCloseEvent* e);
	virtual void reject();

private:

	void fillBusList();

	void fillBusSignals();

	bool addBus(VFrame30::Bus bus);

	void updateButtonsEnableState();

	void updateBusTreeItemText(QTreeWidgetItem* item);

	void updateSignalsTreeItemText(QTreeWidgetItem* item, const VFrame30::BusSignal& signal);

private:

	BusStorage* m_busses = nullptr;

	DbController* m_db = nullptr;

	QSplitter* m_splitter = nullptr;

	QTreeWidget* m_busTree = nullptr;
	QTreeWidget* m_signalsTree = nullptr;

	QPushButton* m_btnAdd = nullptr;
	QPushButton* m_btnRemove = nullptr;
	QPushButton* m_btnCheckOut = nullptr;
	QPushButton* m_btnCheckIn = nullptr;
	QPushButton* m_btnUndo = nullptr;
	QPushButton* m_btnRefresh = nullptr;
	QPushButton* m_btnClose = nullptr;

	QMenu* m_popupMenu = nullptr;
	QAction* m_addAction = nullptr;
	QAction* m_removeAction = nullptr;
	QAction* m_checkOutAction = nullptr;
	QAction* m_checkInAction = nullptr;
	QAction* m_undoAction = nullptr;
	QAction* m_refreshAction = nullptr;
};

extern DialogBusEditor* theDialogBusEditor;


#endif // DIALOGBUSEDITOR_H
