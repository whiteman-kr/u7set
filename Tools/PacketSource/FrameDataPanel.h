#ifndef FRAMEDATAPANEL_H
#define FRAMEDATAPANEL_H

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

#include "FrameBase.h"

// ==============================================================================================

const char* const			FrameDataListColumn[] =
{
							QT_TRANSLATE_NOOP("DataList.h", "Dec"),
							QT_TRANSLATE_NOOP("DataList.h", "Hex"),
};

const int					FRAME_LIST_COLUMN_COUNT		= sizeof(FrameDataListColumn)/sizeof(FrameDataListColumn[0]);

const int					FRAME_LIST_COLUMN_DEC		= 0,
							FRAME_LIST_COLUMN_HEX		= 1;

// ==============================================================================================

const int					FRAME_LIST_COLUMN_ROW_WIDTH	= 50;

// ==============================================================================================

const int					UPDATE_FRAME_STATE_TIMEOUT	= 250; // 250 ms

// ==============================================================================================

class FrameDataTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit FrameDataTable(QObject* parent = nullptr);
	virtual ~FrameDataTable();

private:

	mutable QMutex			m_frameMutex;
	QVector<PS::FrameData*>	m_frameList;

	int						columnCount(const QModelIndex &parent) const;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant				data(const QModelIndex &index, int role) const;

public:

	int						dataSize() const;
	PS::FrameData*			frame(int index) const;
	void					set(const QVector<PS::FrameData*>& list_add);
	void					clear();

	quint8					byte(int index) const;
	void					setByte(int index, quint8 byte);

	QString					text(int row, int column, PS::FrameData *pFrameData) const;

	void					updateColumn(int column);
};

// ==============================================================================================

class FrameDataStateDialog : public QDialog
{
	Q_OBJECT

public:

	explicit FrameDataStateDialog(quint8 byte, QWidget *parent = nullptr);
	virtual ~FrameDataStateDialog();

private:

	QLineEdit*				m_stateEdit = nullptr;

	quint8					m_byte = 0;

	void					createInterface();

public:

	quint8					byte() const { return m_byte; }
	void					setByte(quint8 byte) { m_byte = byte; }

protected:

signals:

private slots:

	void					onOk();
};

// ==============================================================================================

class FrameDataPanel : public QDockWidget
{
	Q_OBJECT

public:

	explicit FrameDataPanel(QWidget* parent = nullptr);
	virtual ~FrameDataPanel();

private:

	QMainWindow*		m_pMainWindow;

	QMainWindow*		m_pFrameWindow = nullptr;
	QTableView*			m_pView = nullptr;
	FrameDataTable		m_frameDataTable;

	QMenu*				m_pContextMenu = nullptr;
	QAction*			m_pSetStateAction = nullptr;

	void				createInterface();
	void				createContextMenu();

public:

	void				clear();
	void				set(const QVector<PS::FrameData*>& list_add);

	void				setState();
protected:

	bool				event(QEvent* e);

private slots:

	void				onContextMenu(QPoint);

	void				onSetStateAction();
	void				onListDoubleClicked(const QModelIndex& index);
};
// ==============================================================================================

#endif // FRAMEDATAPANEL_H
