#ifndef DIALOGLIST_H
#define DIALOGLIST_H

#include <QDebug>
#include <QScreen>
#include <QDialog>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QVBoxLayout>
#include <QTableView>
#include <QDialogButtonBox>
#include <QKeyEvent>

#include "DialogObjectProperties.h"

// ==============================================================================================

class DialogList : public QDialog
{
	Q_OBJECT

public:

	explicit DialogList(double width, double height, bool hasButtons, QWidget* parent = nullptr);
	virtual ~DialogList() override;

protected:

	//
	//
	QAction*				m_pExportAction = nullptr;
	QAction*				m_pFindAction = nullptr;
	QAction*				m_pCopyAction = nullptr;
	QAction*				m_pSelectAllAction = nullptr;
	QAction*				m_pPropertyAction = nullptr;

	//
	//
	void					addMenu(QMenu *menu);
	void					addContextMenu(QMenu *menu);
	void					addContextAction(QAction *action);
	void					addContextSeparator();

	//
	//
	QTableView*				view() { return m_pView; }
	void					setModel(QAbstractItemModel *model);
	void					createHeaderContexMenu(int columnCount, const char* const* columnCaption, const int* columnWidth);
	void					hideColumn(int column, bool hide);

	//
	//
	bool					eventFilter(QObject* object, QEvent* event) override;

private:

	QMenuBar*				m_pMenuBar = nullptr;
	QMenu*					m_pContextMenu = nullptr;

	QTableView*				m_pView = nullptr;

	QDialogButtonBox*		m_buttonBox = nullptr;

	QVector<QAction*>		m_pColumnActionList;
	QMenu*					m_headerContextMenu = nullptr;

	void					createInterface(double width, double height, bool hasButtons);
	void					createContextMenu();

public slots:

	// slots for updating
	//
	virtual void			updateVisibleColunm();
	virtual void			updateList();

private slots:

	// slots of menu
	//
	virtual void			onExport();
	virtual void			onFind();
	virtual void			onCopy();
	virtual void			onSelectAll() { m_pView->selectAll(); }
	virtual void			onProperties();

	void					onContextMenu(QPoint);

	// slots for list header, to hide or show columns
	//
	void					onHeaderContextMenu(QPoint);
	void					onColumnAction(QAction* action);

	// slots for list
	//
	void					onListDoubleClicked(const QModelIndex&);

	// slots of buttons
	//
	virtual void			onOk();
};

// ==============================================================================================

#endif // DIALOGLIST_H
