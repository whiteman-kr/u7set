#ifndef FRAMEDATAPANEL_H
#define FRAMEDATAPANEL_H

#include <QDockWidget>
#include <QDialog>
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
	virtual ~FrameDataTable() override;

public:

	int	dataSize() const;
	PS::FrameData* frame(int index) const;
	void set(const std::vector<PS::FrameData*>& list_add);
	void clear();

	quint8 byte(int index) const;
	void setByte(int index, quint8 byte);

	QString text(int row, int column, PS::FrameData *pFrameData) const;

	void updateColumn(int column);

private:

	mutable QMutex m_frameMutex;
	std::vector<PS::FrameData*>	m_frameList;

	int columnCount(const QModelIndex &parent) const override;
	int rowCount(const QModelIndex &parent=QModelIndex()) const override;

	QVariant headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const override;
	QVariant data(const QModelIndex &index, int role) const override;
};

// ==============================================================================================

class FrameDataStateDialog : public QDialog
{
	Q_OBJECT

public:

	explicit FrameDataStateDialog(quint8 byte, QWidget *parent = nullptr);
	virtual ~FrameDataStateDialog() override;

public:

	quint8 byte() const { return m_byte; }
	void setByte(quint8 byte) { m_byte = byte; }

private:

	QLineEdit* m_stateEdit = nullptr;

	quint8 m_byte = 0;

	void createInterface();

private slots:

	void onOk();
};

// ==============================================================================================

class FrameDataPanel : public QDockWidget
{
	Q_OBJECT

public:

	explicit FrameDataPanel(QWidget* parent = nullptr);
	virtual ~FrameDataPanel()  override;

public:

	void clear();
	void set(const std::vector<PS::FrameData*>& list_add);

	void setState();

private:

	QMainWindow* m_pMainWindow;

	QMainWindow* m_pFrameWindow = nullptr;
	QTableView* m_pView = nullptr;
	FrameDataTable m_frameDataTable;

	QMenu* m_pContextMenu = nullptr;
	QAction* m_pSetStateAction = nullptr;

	void createInterface();
	void createContextMenu();

protected:

	bool event(QEvent* e) override;

private slots:

	void onContextMenu(QPoint);

	void onSetStateAction();
	void onListDoubleClicked(const QModelIndex& index);
};
// ==============================================================================================

#endif // FRAMEDATAPANEL_H
