#ifndef SOURCELIST_H
#define SOURCELIST_H

#include <QAbstractTableModel>
#include <QColor>

#include "SourceBase.h"

// ==============================================================================================

const char* const			SourceListColumn[] =
{
							QT_TRANSLATE_NOOP("SourceList.h", "IP (LM)"),
							QT_TRANSLATE_NOOP("SourceList.h", "IP (AppDataSrv)"),
							QT_TRANSLATE_NOOP("SourceList.h", "Caption"),
							QT_TRANSLATE_NOOP("SourceList.h", "Equipment ID"),
							QT_TRANSLATE_NOOP("SourceList.h", "Module type"),
							QT_TRANSLATE_NOOP("SourceList.h", "SubSystem"),
							QT_TRANSLATE_NOOP("SourceList.h", "Frame count"),
							QT_TRANSLATE_NOOP("SourceList.h", "State"),
							QT_TRANSLATE_NOOP("SourceList.h", "Signal count"),
};

const int					SOURCE_LIST_COLUMN_COUNT			= sizeof(SourceListColumn)/sizeof(SourceListColumn[0]);

const int					SOURCE_LIST_COLUMN_LM_IP			= 0,
							SOURCE_LIST_COLUMN_SERVER_IP		= 1,
							SOURCE_LIST_COLUMN_CAPTION			= 2,
							SOURCE_LIST_COLUMN_EQUIPMENT_ID		= 3,
							SOURCE_LIST_COLUMN_MODULE_TYPE		= 4,
							SOURCE_LIST_COLUMN_SUB_SYSTEM		= 5,
							SOURCE_LIST_COLUMN_FRAME_COUNT		= 6,
							SOURCE_LIST_COLUMN_STATE			= 7,
							SOURCE_LIST_COLUMN_SIGNAL_COUNT		= 8;

const int					SourceListColumnWidth[SOURCE_LIST_COLUMN_COUNT] =
{
							150, // SOURCE_LIST_COLUMN_LM_IP
							150, // SOURCE_LIST_COLUMN_SERVER_IP
							100, // SOURCE_LIST_COLUMN_CAPTION
							150, // SOURCE_LIST_COLUMN_EQUIPMENT_ID
							100, // SOURCE_LIST_COLUMN_MODULE_TYPE
							100, // SOURCE_LIST_COLUMN_SUB_SYSTEM
							100, // SOURCE_LIST_COLUMN_FRAME_COUNT
							100, // SOURCE_LIST_COLUMN_STATE
							100, // SOURCE_LIST_COLUMN_SIGNAL_COUNT
};

// ==============================================================================================

const int					UPDATE_SOURCE_STATE_TIMEOUT		= 250; // 250 ms

// ==============================================================================================

class SourceTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit SourceTable(QObject* parent = nullptr);
	virtual ~SourceTable();

private:

	mutable QMutex			m_sourceMutex;
	QVector<PS::Source*>	m_sourceList;

	int						columnCount(const QModelIndex &parent) const;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant				data(const QModelIndex &index, int role) const;

	QTimer*					m_updateSourceListTimer = nullptr;
	void					startUpdateSourceListTimer();
	void					stopUpdateSourceListTimer();

public:

	int						sourceCount() const;
	PS::Source*				sourceAt(int index) const;
	void					set(const QVector<PS::Source*> list_add);
	void					clear();

	QString					text(int row, int column, PS::Source *pSource) const;

	void					updateColumn(int column);

private slots:

	void					updateSourceState();
};

// ==============================================================================================

#endif // SOURCELIST_H
