#ifndef SOURCEBASE_H
#define SOURCEBASE_H

#include <QAbstractTableModel>
#include <QColor>

#include "Options.h"
#include "SourceWorker.h"

#include "../../lib/HostAddressPort.h"

// ==============================================================================================

struct SourceInfo
{
public:

	SourceInfo::SourceInfo()
	{
		clear();
	}

	QString			caption;
	QString			equipmentID;

	QString			dataType;
	int				moduleType = 0;
	QString			subSystem;
	int				frameCount = 0;
	quint32			dataID = 0;

	HostAddressPort	lmAddress;
	HostAddressPort	serverAddress;

	void clear()
	{

		caption.clear();
		equipmentID.clear();

		dataType.clear();
		moduleType = 0;
		subSystem.clear();
		frameCount = 0;
		dataID = 0;

		lmAddress.clear();
		serverAddress.clear();
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

	QString			caption() const { return m_si.caption; }
	void			setCaption(const QString& caption) { m_si.caption = caption; }

	QString			equipmentID() const { return m_si.equipmentID; }
	void			setEquipmentID(const QString& equipmentID) { m_si.equipmentID = equipmentID; }

	QString			dataType() const { return m_si.dataType; }
	void			setDataType(const QString& type) { m_si.dataType = type; }

	int				moduleType() const { return m_si.moduleType; }
	void			setModuleType(int type) { m_si.moduleType= type; }

	QString			subSystem() const { return m_si.subSystem; }
	void			setSubSystem(const QString& subSystem) { m_si.subSystem = subSystem; }

	int				frameCount() const { return m_si.frameCount; }
	void			setFrameCount(int count) { m_si.frameCount = count; }

	quint32			dataID() const { return m_si.dataID; }
	void			setDataID(quint32 id) { m_si.dataID = id; }

	HostAddressPort	lmAddress() const { return m_si.lmAddress; }
	void			setLmAddress(const HostAddressPort& address) { m_si.lmAddress = address; }

	HostAddressPort	serverAddress() const { return m_si.serverAddress; }
	void			setServerAddress(const HostAddressPort& address) { m_si.serverAddress = address; }

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

	//
	//
	void					setServerAddress(const HostAddressPort& address);

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
							QT_TRANSLATE_NOOP("SourceList.h", "Caption"),
							QT_TRANSLATE_NOOP("SourceList.h", "Equipment ID"),
							QT_TRANSLATE_NOOP("SourceList.h", "Data type"),
							QT_TRANSLATE_NOOP("SourceList.h", "Module type"),
							QT_TRANSLATE_NOOP("SourceList.h", "SubSystem"),
							QT_TRANSLATE_NOOP("SourceList.h", "Frame count"),
							QT_TRANSLATE_NOOP("SourceList.h", "IP (LM)"),
							QT_TRANSLATE_NOOP("SourceList.h", "IP (AppDataSrv)"),
};

const int					SOURCE_LIST_COLUMN_COUNT			= sizeof(SourceListColumn)/sizeof(SourceListColumn[0]);

const int					SOURCE_LIST_COLUMN_CAPTION			= 0,
							SOURCE_LIST_COLUMN_EQUIPMENT_ID		= 1,
							SOURCE_LIST_COLUMN_DATA_TYPE		= 2,
							SOURCE_LIST_COLUMN_MODULE_TYPE		= 3,
							SOURCE_LIST_COLUMN_SUB_SYSTEM		= 4,
							SOURCE_LIST_COLUMN_FRAME_COUNT		= 5,
							SOURCE_LIST_COLUMN_LM_IP			= 6,
							SOURCE_LIST_COLUMN_SERVER_IP		= 7;

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
