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
	DialogBusEditor(DbController* db, QWidget* parent);
	~DialogBusEditor();

private slots:

	void onAdd();
	void onRemove();
	void onCheckOut();
	void onCheckIn();
	void onUndo();
	void onRefresh();

	void onSignalAdd();
	void onSignalCreate(E::SignalType type);
	void onSignalEdit();
	void onSignalRemove();
	void onSignalUp();
	void onSignalDown();

	void onSignalItemDoubleClicked(QTreeWidgetItem *item, int column);

	void onBusItemChanged(QTreeWidgetItem *item, int column);
	void onBusItemSelectionChanged();
	void onBusCustomContextMenuRequested(const QPoint& pos);
	void onBusSortIndicatorChanged(int column, Qt::SortOrder order);

	void onSignalItemSelectionChanged();
	void onSignalCustomContextMenuRequested(const QPoint& pos);

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

	VFrame30::Bus* getCurrentBus(QUuid* uuid);

	bool saveBus(const QUuid& busUuid);

private:
	BusStorage m_busses = nullptr;
	DbController* m_db = nullptr;

	QSplitter* m_splitter = nullptr;

	QTreeWidget* m_busTree = nullptr;
	QTreeWidget* m_signalsTree = nullptr;

	QPushButton* m_buttonAdd = nullptr;
	QPushButton* m_buttonRemove = nullptr;
	QPushButton* m_buttonCheckOut = nullptr;
	QPushButton* m_buttonCheckIn = nullptr;
	QPushButton* m_buttonUndo = nullptr;
	QPushButton* m_buttonRefresh = nullptr;
	QPushButton* m_btnClose = nullptr;

	QMenu* m_popupMenu = nullptr;
	QAction* m_addAction = nullptr;
	QAction* m_removeAction = nullptr;
	QAction* m_checkOutAction = nullptr;
	QAction* m_checkInAction = nullptr;
	QAction* m_undoAction = nullptr;
	QAction* m_refreshAction = nullptr;

	QPushButton* m_btnSignalAdd = nullptr;
	QPushButton* m_btnSignalEdit = nullptr;
	QPushButton* m_btnSignalRemove = nullptr;
	QPushButton* m_btnSignalUp = nullptr;
	QPushButton* m_btnSignalDown = nullptr;

	QMenu* m_signalPopupMenu = nullptr;
	QAction* m_signalAddAction = nullptr;
	QAction* m_signalAddSubmenuAction = nullptr;
	QAction* m_signalEditAction = nullptr;
	QAction* m_signalRemoveAction = nullptr;
	QAction* m_signalUpAction = nullptr;
	QAction* m_signalDownAction = nullptr;

	QMenu* m_addSignalMenu = nullptr;

	QAction* m_analogAction = nullptr;
	QAction* m_discreteAction = nullptr;
	QAction* m_busAction = nullptr;

	PropertyEditorDialog* m_propEditorDialog = nullptr;
};

extern DialogBusEditor* theDialogBusEditor;


#endif // DIALOGBUSEDITOR_H
