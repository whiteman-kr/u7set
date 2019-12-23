#ifndef FINDSIGNALPANEL_H
#define FINDSIGNALPANEL_H

#include <QDockWidget>
#include <QMenu>
#include <QAction>
#include <QKeyEvent>
#include <QList>
#include <QLabel>
#include <QLineEdit>
#include <QTableView>
#include <QTableWidget>
#include <QStyledItemDelegate>
#include <QPainter>

// ==============================================================================================

class FindItem
{
public:

	FindItem();
	FindItem(int row, int column, const QString& text);
	virtual ~FindItem() {}

private:

	int					m_row = -1;
	int					m_column = -1;

	QString				m_text;

public:

	int					row() const { return m_row; }
	void				setRow(int row) { m_row = row; }

	int					column() const { return m_column; }
	void				setColumn(int column) { m_column = column; }

	void				setCoordinates(int row, int column) { m_row = row; m_column = column; }

	QString				text() const { return m_text; }
	void				setText(const QString& text) { m_text = text; }

	FindItem&			operator=(const FindItem& from);
};

// ==============================================================================================

Q_DECLARE_METATYPE(FindItem)	// for type QVariant

// ==============================================================================================

const char* const		FindSignalTextColumn[] =
{
						QT_TRANSLATE_NOOP("FindSignalTextPanel.h", "Row"),
						QT_TRANSLATE_NOOP("FindSignalTextPanel.h", "Text"),
};

const int				FIND_SIGNAL_COLUMN_COUNT		= sizeof(FindSignalTextColumn)/sizeof(FindSignalTextColumn[0]);

const int				FIND_SIGNAL_COLUMN_ROW			= 0,
						FIND_SIGNAL_COLUMN_TEXT			= 1;

// ==============================================================================================

const int				FIND_SIGNAL_COLUMN_ROW_WIDTH	= 50;

// ==============================================================================================

class FindSignalTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit FindSignalTable(QObject* parent = nullptr);
	virtual ~FindSignalTable();

private:

	QVector<FindItem>	m_findItemList;

	int					columnCount(const QModelIndex &parent) const;
	int					rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant			headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant			data(const QModelIndex &index, int role) const;

public:

	int					count() const { return m_findItemList.count(); }
	FindItem			at(int index) const;
	void				set(const QVector<FindItem>& list_add);
	void				clear();

	QString				text(int row, int column) const;

};

// ==============================================================================================

#define				 FIND_SIGNAL_OPTIONS_KEY		"Options/FindSignalText/"

// ==============================================================================================

class FindSignalPanel : public QDockWidget
{
	Q_OBJECT

public:

	explicit FindSignalPanel(QWidget* parent = nullptr);
	virtual ~FindSignalPanel();

private:

	QMainWindow*		m_pMainWindow;

	QString				m_findText;

	QMainWindow*		m_pFindWindow = nullptr;
	QLineEdit*			m_findTextEdit  = nullptr;
	QTableView*			m_pView = nullptr;
	QLabel*				m_statusLabel = nullptr;
	FindSignalTable	m_table;

	QMenu*				m_pContextMenu = nullptr;
	QAction*			m_pCopyAction = nullptr;
	QAction*			m_pSelectAllAction = nullptr;

	void				createInterface();
	void				createContextMenu();

	void				loadSettings();
	void				saveSettings();

public:

	void				clear() { m_table.clear(); m_statusLabel->setText(QString()); }

protected:

	bool				event(QEvent* e);
	bool				eventFilter(QObject* object, QEvent* e);

private slots:

	void				find();

	void				selectItemInSignalView();

	void				onContextMenu(QPoint);

	void				copy();
	void				selectAll() { m_pView->selectAll(); }
};

// ==============================================================================================

#endif // FINDSIGNALPANEL_H
