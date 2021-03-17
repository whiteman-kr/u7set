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

template <typename TYPE>
class ListTable : public QAbstractTableModel
{
public:

	explicit ListTable(QObject* parent = nullptr);
	virtual ~ListTable() override;

public:

	void setColumnCaption(const QString& className, int columnCount, const char* const* pColumnCaption);

	void clear();
	void set(const QVector<TYPE>& list_add);

	int count() const;
	TYPE at(int index) const;

	void updateColumn(int column);

protected:

	int m_rowCount = 0;
	int m_columnCount = 0;

	mutable QMutex m_mutex;
	QVector<TYPE> m_list;

private:

	QString m_className;
	const char* const* m_pColumnCaption = nullptr;

	int columnCount(const QModelIndex &parent) const override;
	int rowCount(const QModelIndex &parent=QModelIndex()) const override;

	QVariant headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const override;
};

// ----------------------------------------------------------------------------------------------

template <typename TYPE>
ListTable<TYPE>::ListTable(QObject* parent)
{
	Q_UNUSED(parent)
}

// ----------------------------------------------------------------------------------------------

template <typename TYPE>
ListTable<TYPE>::~ListTable()
{
	QMutexLocker l(&m_mutex);

	m_rowCount = 0;
	m_list.clear();
}

// ----------------------------------------------------------------------------------------------

template <typename TYPE>
void ListTable<TYPE>::setColumnCaption(const QString& className, int columnCount, const char* const* pColumnCaption)
{
	if(pColumnCaption == nullptr)
	{
		m_columnCount = 0;
		return;
	}

	m_className = className;

	m_columnCount = columnCount;
	m_pColumnCaption = pColumnCaption;
}

// ----------------------------------------------------------------------------------------------

template <typename TYPE>
void ListTable<TYPE>::clear()
{
	int itemCount = count();
	if (itemCount == 0)
	{
		return;
	}

	beginRemoveRows(QModelIndex(), 0, itemCount - 1);

		m_mutex.lock();

			m_rowCount = 0;
			m_list.clear();

		m_mutex.unlock();

	endRemoveRows();
}

// ----------------------------------------------------------------------------------------------

template <typename TYPE>
void ListTable<TYPE>::set(const QVector<TYPE>& list_add)
{
	int itemCount = list_add.count();
	if (itemCount == 0)
	{
		return;
	}

	beginInsertRows(QModelIndex(), 0, itemCount - 1);

		m_mutex.lock();

			m_rowCount = itemCount;
			m_list = list_add;

		m_mutex.unlock();

	endInsertRows();
}

// ----------------------------------------------------------------------------------------------

template <typename TYPE>
int ListTable<TYPE>::count() const
{
	return m_rowCount;
}

// ----------------------------------------------------------------------------------------------

template <typename TYPE>
TYPE ListTable<TYPE>::at(int index) const
{
	QMutexLocker l(&m_mutex);

	if (index < 0 || index >= m_list.count())
	{
		return TYPE();
	}

	return m_list[index];
}

// ----------------------------------------------------------------------------------------------

template <typename TYPE>
void ListTable<TYPE>::updateColumn(int column)
{
	if (column < 0 || column >= m_columnCount)
	{
		return;
	}

	for (int row = 0; row < m_columnCount; row ++)
	{
		QModelIndex cellIndex = index(row, column);

		emit dataChanged(cellIndex, cellIndex, QVector<int>() << Qt::DisplayRole);
	}
}

// ----------------------------------------------------------------------------------------------

template <typename TYPE>
int ListTable<TYPE>::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)

	return m_columnCount;
}

// ----------------------------------------------------------------------------------------------

template <typename TYPE>
int ListTable<TYPE>::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)

	return m_rowCount;
}

// ----------------------------------------------------------------------------------------------

template <typename TYPE>
QVariant ListTable<TYPE>::headerData(int section,Qt::Orientation orientation, int role) const
{
	if (role != Qt::DisplayRole)
	{
		return QVariant();
	}

	QVariant result = QVariant();

	if (orientation == Qt::Horizontal)
	{
		if (section >= 0 && section < m_columnCount)
		{
			result = qApp->translate(m_className.toUtf8(), m_pColumnCaption[section]);
		}
	}

	if (orientation == Qt::Vertical)
	{
		result = QString("%1").arg(section + 1);
	}

	return result;
}

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
