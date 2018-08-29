#ifndef SOURCEBASE_H
#define SOURCEBASE_H

#include <QAbstractTableModel>
#include <QColor>

#include "Options.h"
#include "SourceWorker.h"

#include "../../lib/HostAddressPort.h"

// ==============================================================================================

namespace PS
{
	//
	//

	const int SUPPORT_VERSION		= 5;		// last version of Rup::VERSION

	const int SIM_FRAME_VERSION		= 1;		// last version of SimFrame

	const int UDP_PORT				= 10000;

	const int SEND_TIMEOUT			= 5;		// 5 ms

	//
	//

	struct SourceInfo
	{
	public:

		SourceInfo();

		void				clear();

		int					index = -1;

		QString				caption;
		QString				equipmentID;

		int					moduleType = 0;
		QString				subSystem;
		int					frameCount = 0;
		quint32				dataID = 0;

		HostAddressPort		lmAddress;
		HostAddressPort		serverAddress;
	};

	//
	//

	class Source : public QObject
	{
		Q_OBJECT

	public:

		Source();
		Source(const Source& from);
		Source(const PS::SourceInfo& si);
		virtual ~Source();

	private:

		mutable QMutex		m_sourceMutex;

		QThread*			m_pThread = nullptr;
		SourceWorker*		m_pWorker = nullptr;

		PS::SourceInfo		m_si;

	public:

		void				clear();

		PS::SourceInfo&		info() { return m_si; }

		bool				run();
		bool				stop();

		bool				isRunning();
		int					sentFrames();

		Source&				operator=(const Source& from);

	signals:

	public slots:

		bool				createWorker();
		void				deleteWorker();

	};
}

// ==============================================================================================

class SourceBase : public QObject
{
	Q_OBJECT

public:

	explicit SourceBase(QObject *parent = 0);
	virtual ~SourceBase();

private:

	mutable QMutex			m_sourceMutex;
	QVector<PS::Source>		m_sourceList;

public:

	void					clear();
	int						count() const;

	int						readFromXml();

	int						append(const PS::Source &source);
	void					remove(int index);

	PS::Source				source(int index) const;
	PS::Source*				sourcePtr(int index);
	void					setSource(int index, const PS::Source& source);

	SourceBase&				operator=(const SourceBase& from);

	// run stop send udp trhread
	//
	void					runSourece(int index);
	void					stopSourece(int index);

	void					runAllSoureces();
	void					stopAllSoureces();

signals:

public slots:

};

// ----------------------------------------------------------------------------------------------

extern SourceBase theSourceBase;

// ==============================================================================================

const char* const			SourceListColumn[] =
{
							QT_TRANSLATE_NOOP("SourceList.h", "Caption"),
							QT_TRANSLATE_NOOP("SourceList.h", "Equipment ID"),
							QT_TRANSLATE_NOOP("SourceList.h", "Module type"),
							QT_TRANSLATE_NOOP("SourceList.h", "SubSystem"),
							QT_TRANSLATE_NOOP("SourceList.h", "Frame count"),
							QT_TRANSLATE_NOOP("SourceList.h", "IP (LM)"),
							QT_TRANSLATE_NOOP("SourceList.h", "IP (AppDataSrv)"),
							QT_TRANSLATE_NOOP("SourceList.h", "State"),
};

const int					SOURCE_LIST_COLUMN_COUNT			= sizeof(SourceListColumn)/sizeof(SourceListColumn[0]);

const int					SOURCE_LIST_COLUMN_CAPTION			= 0,
							SOURCE_LIST_COLUMN_EQUIPMENT_ID		= 1,
							SOURCE_LIST_COLUMN_MODULE_TYPE		= 2,
							SOURCE_LIST_COLUMN_SUB_SYSTEM		= 3,
							SOURCE_LIST_COLUMN_FRAME_COUNT		= 4,
							SOURCE_LIST_COLUMN_LM_IP			= 5,
							SOURCE_LIST_COLUMN_SERVER_IP		= 6,
							SOURCE_LIST_COLUMN_STATE			= 7;

const int					SourceListColumnWidth[SOURCE_LIST_COLUMN_COUNT] =
{
							100, // SOURCE_LIST_COLUMN_CAPTION
							150, // SOURCE_LIST_COLUMN_EQUIPMENT_ID
							100, // SOURCE_LIST_COLUMN_MODULE_TYPE
							100, // SOURCE_LIST_COLUMN_SUB_SYSTEM
							100, // SOURCE_LIST_COLUMN_FRAME_COUNT
							120, // SOURCE_LIST_COLUMN_LM_IP
							120, // SOURCE_LIST_COLUMN_SERVER_IP
							100, // SOURCE_LIST_COLUMN_STATE
};

// ==============================================================================================

const int					UPDATE_SOURCE_STATE_TIMEOUT		= 250; // 250 ms

// ==============================================================================================

class SourceTable : public QAbstractTableModel
{
	Q_OBJECT

public:

	explicit SourceTable(QObject* parent = 0);
	virtual ~SourceTable();

private:

	mutable QMutex			m_sourceMutex;
	QVector<PS::Source*>	m_sourceList;

	int						columnCount(const QModelIndex &parent) const;
	int						rowCount(const QModelIndex &parent=QModelIndex()) const;

	QVariant				headerData(int section,Qt::Orientation orientation, int role=Qt::DisplayRole) const;
	QVariant				data(const QModelIndex &index, int role) const;

public:

	int						sourceCount() const;
	PS::Source*				sourceAt(int index) const;
	void					set(const QVector<PS::Source *> list_add);
	void					clear();

	QString					text(int row, int column, PS::Source *pSource) const;

	void					updateColumn(int column);
};

// ==============================================================================================

#endif // SOURCELIST_H
