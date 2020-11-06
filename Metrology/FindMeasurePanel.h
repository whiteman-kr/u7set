#ifndef FINDMEASUREPANEL_H
#define FINDMEASUREPANEL_H

#include <QMainWindow>
#include <QDockWidget>
#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QTableView>

#include "MeasureBase.h"
#include "MeasureView.h"
#include "ProcessData.h"

// ==============================================================================================

class FindItem
{
public:

	FindItem();
	FindItem(int row, int column, const QString& text, int beginPos, int endPos);
	virtual ~FindItem() {}

private:

	int					m_row = -1;
	int					m_column = -1;

	QString				m_text;

	int					m_beginPos = -1;
	int					m_endPos = -1;

public:

	int					row() const { return m_row; }
	void				setRow(int row) { m_row = row; }

	int					column() const { return m_column; }
	void				setColumn(int column) { m_column = column; }

	void				setCoordinates(int row, int column) { m_row = row; m_column = column; }

	QString				text() const { return m_text; }
	void				setText(const QString& text) { m_text = text; }

	int					beginPos() const { return m_beginPos; }
	void				setBeginPos(int pos) { m_beginPos = pos; }

	int					endPos() const { return m_endPos; }
	void				setEndPos(int pos) { m_endPos = pos; }

	void				setPos(int beginPos, int endPos) { m_beginPos = beginPos; m_endPos = endPos; }

	FindItem&			operator=(const FindItem& from);
};

// ==============================================================================================

Q_DECLARE_METATYPE(FindItem)	// for type QVariant

// ==============================================================================================

const char* const		FindMeasureColumn[] =
{
						QT_TRANSLATE_NOOP("FindMeasurePanel.h", "Row"),
						QT_TRANSLATE_NOOP("FindMeasurePanel.h", "Text"),
};

const int				FIND_MEASURE_COLUMN_COUNT		= sizeof(FindMeasureColumn)/sizeof(FindMeasureColumn[0]);

const int				FIND_MEASURE_COLUMN_ROW			= 0,
						FIND_MEASURE_COLUMN_TEXT		= 1;

// ==============================================================================================

const int				FIND_MEASURE_COLUMN_ROW_WIDTH	= 50;

// ==============================================================================================

class FindMeasureTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit FindMeasureTable(QObject* parent = nullptr);
	virtual ~FindMeasureTable();

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

#define					FIND_MEASURE_OPTIONS_KEY		"Options/Find/FindMeasure/"

// ==============================================================================================

class FindMeasurePanel : public QDockWidget
{
	Q_OBJECT

public:

	explicit FindMeasurePanel(QWidget* parent = nullptr);
	virtual ~FindMeasurePanel();

private:

	MeasureView*		m_pMeasureView = nullptr;

	QMenu*				m_pContextMenu = nullptr;
	QAction*			m_pCopyAction = nullptr;
	QAction*			m_pSelectAllAction = nullptr;

	QString				m_findText;

	QMainWindow*		m_pFindWindow = nullptr;

	CompleterData		m_findCompleter;
	QLineEdit*			m_findTextEdit  = nullptr;

	QTableView*			m_pView = nullptr;
	FindMeasureTable	m_table;

	QLabel*				m_statusLabel = nullptr;

	void				clear();

	void				createInterface();
	void				createContextMenu();

	void				loadSettings();
	void				saveSettings();

protected:

	bool				event(QEvent* e);
	bool				eventFilter(QObject* object, QEvent* e);

public:

	void				setViewFont(const QFont& font);

	void				setFindText(const QString& findText);

public slots:

	void				measureViewChanged(MeasureView* pView);

	void				find();


private slots:

	void				selectItemInMeasureView();

	void				onContextMenu(QPoint);

	void				copy();
	void				selectAll() { m_pView->selectAll(); }
};

// ==============================================================================================

#endif // FINDMEASUREPANEL_H
