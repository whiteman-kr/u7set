#ifndef SOURCEBASE_H
#define SOURCEBASE_H

#include <QAbstractTableModel>
#include <QColor>

#include "Options.h"

#include "SourceWorker.h"

// ==============================================================================================

struct SourceInfo
{
public:

	SourceInfo::SourceInfo()
	{
		clear();
	}

	QString		m_dataType;
	QString		m_equipmentID;
	int			m_moduleType = 0;
	QString		m_subSystem;
	QString		m_caption;
	QString		m_ip;
	int			m_port = 0;
	int			m_frameCount = 0;

	void clear()
	{
		m_dataType.clear();
		m_equipmentID.clear();
		m_moduleType = 0;
		m_subSystem.clear();
		m_caption.clear();
		m_ip.clear();
		m_port = 0;
		m_frameCount = 0;
	}
};

// ==============================================================================================

class SourceItem : public QObject
{

public:

	SourceItem();
	SourceItem(const SourceItem& from);
	SourceItem(const SourceInfo& si);
	virtual ~SourceItem();

private:

	mutable QMutex	m_sourceMutex;

	SourceWorker*	m_pWorker = nullptr;

	SourceInfo		m_si;

public:

	void			clear();

	QString			dataType() const { return m_si.m_dataType; }
	void			setDataType(const QString& type) { m_si.m_dataType = type; }

	QString			equipmentID() const { return m_si.m_equipmentID; }
	void			setEquipmentID(const QString& equipmentID) { m_si.m_equipmentID = equipmentID; }

	int				moduleType() const { return m_si.m_moduleType; }
	void			setModuleType(int type) { m_si.m_moduleType= type; }

	QString			subSystem() const { return m_si.m_subSystem; }
	void			setSubSystem(const QString& subSystem) { m_si.m_subSystem = subSystem; }

	QString			caption() const { return m_si.m_caption; }
	void			setCaption(const QString& caption) { m_si.m_caption = caption; }

	QString			ip() const { return m_si.m_ip; }
	void			setIP(const QString& ip) { m_si.m_ip = ip; }

	int				port() const { return m_si.m_port; }
	void			setPort(int port) { m_si.m_port = port; }

	int				frameCount() const { return m_si.m_frameCount; }
	void			setFrameCount(int count) { m_si.m_frameCount = count; }

	bool			run();
	bool			stop();

	SourceItem&		operator=(const SourceItem& from);

signals:

public slots:

};

// ==============================================================================================

class SourceBase : public QObject
{
	Q_OBJECT

public:

	explicit SourceBase(QObject *parent = 0);
	virtual ~SourceBase() {}

private:

	mutable QMutex			m_sourceMutex;
	QVector<SourceItem>		m_sourceList;

	bool					m_sourcesIsRunning = false;

public:

	void					clear();
	int						count() const;

	int						readFromXml();

	int						append(const SourceItem &source);
	void					remove(int index);

	SourceItem				source(int index) const;
	SourceItem*				sourcePtr(int index);
	void					setSource(int index, const SourceItem& source);

	SourceBase&				operator=(const SourceBase& from);

	// run stop send udp trhread
	//
	void					runSourece(int index);
	void					stopSourece(int index);

	void					runAllSoureces();
	void					stopAllSoureces();

	bool					sourcesIsRunning() { return m_sourcesIsRunning; }

signals:

public slots:

};


// ----------------------------------------------------------------------------------------------

extern SourceBase theSourceBase;

// ==============================================================================================

const int					LIST_COLUMN_WITDH					= 200;

// ----------------------------------------------------------------------------------------------

const char* const			SourceListColumn[] =
{
							QT_TRANSLATE_NOOP("SourceList.h", "Data type"),
							QT_TRANSLATE_NOOP("SourceList.h", "Equipment ID"),
							QT_TRANSLATE_NOOP("SourceList.h", "Module type"),
							QT_TRANSLATE_NOOP("SourceList.h", "SubSystem"),
							QT_TRANSLATE_NOOP("SourceList.h", "Caption"),
							QT_TRANSLATE_NOOP("SourceList.h", "IP"),
							QT_TRANSLATE_NOOP("SourceList.h", "Frame count"),
};

const int					SOURCE_LIST_COLUMN_COUNT			= sizeof(SourceListColumn)/sizeof(SourceListColumn[0]);

const int					SOURCE_LIST_COLUMN_DATA_TYPE		= 0,
							SOURCE_LIST_COLUMN_EQUIPMENT_ID		= 1,
							SOURCE_LIST_COLUMN_MODULE_TYPE		= 2,
							SOURCE_LIST_COLUMN_SUB_SYSTEM		= 3,
							SOURCE_LIST_COLUMN_CAPTION			= 4,
							SOURCE_LIST_COLUMN_IP				= 5,
							SOURCE_LIST_COLUMN_FRAME_COUNT		= 6;

// ==============================================================================================

const int					UPDATE_SOURCE_TIMEOUT		= 250; // 250 ms

// ==============================================================================================

class SourceTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit SourceTable(QObject* parent = 0);
	virtual ~SourceTable();

private:

	mutable QMutex			m_sourceMutex;
	QVector<SourceItem*>	m_sourceList;

	int						columnCount(const QModelIndex &parent) const;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant				data(const QModelIndex &index, int role) const;

public:

	int						sourceCount() const;
	SourceItem*				sourceAt(int index) const;
	void					set(const QVector<SourceItem *> list_add);
	void					clear();

	QString					text(int row, int column, SourceItem *pSource) const;

	void					updateColumn(int column);
};

// ==============================================================================================

#endif // SOURCELIST_H
