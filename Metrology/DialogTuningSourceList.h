#ifndef DIALOGTUNINGSOURCELIST_H
#define DIALOGTUNINGSOURCELIST_H

#include "DialogList.h"
#include "TuningSignalBase.h"

// ==============================================================================================

const char* const			TuningSourceColumn[] =
{
							QT_TRANSLATE_NOOP("DialogTuningSourceList", "EquipmentID"),
							QT_TRANSLATE_NOOP("DialogTuningSourceList", "Caption"),
							QT_TRANSLATE_NOOP("DialogTuningSourceList", "IP"),
							QT_TRANSLATE_NOOP("DialogTuningSourceList", "Channel"),
							QT_TRANSLATE_NOOP("DialogTuningSourceList", "Subsytem"),
							QT_TRANSLATE_NOOP("DialogTuningSourceList", "LM number"),
							QT_TRANSLATE_NOOP("DialogTuningSourceList", "isReply"),
							QT_TRANSLATE_NOOP("DialogTuningSourceList", "Request count"),
							QT_TRANSLATE_NOOP("DialogTuningSourceList", "Reply count"),
							QT_TRANSLATE_NOOP("DialogTuningSourceList", "Cmd queue size"),
};

const int					TUN_SOURCE_LIST_COLUMN_COUNT		= sizeof(TuningSourceColumn)/sizeof(TuningSourceColumn[0]);

const int					TUN_SOURCE_LIST_COLUMN_EQUIPMENT_ID	= 0,
							TUN_SOURCE_LIST_COLUMN_CAPTION		= 1,
							TUN_SOURCE_LIST_COLUMN_IP			= 2,
							TUN_SOURCE_LIST_COLUMN_CHANNEL		= 3,
							TUN_SOURCE_LIST_COLUMN_SUBSYSTEM	= 4,
							TUN_SOURCE_LIST_COLUMN_LM_NUMBER	= 5,
							TUN_SOURCE_LIST_COLUMN_IS_REPLY		= 6,
							TUN_SOURCE_LIST_COLUMN_REQUESTS		= 7,
							TUN_SOURCE_LIST_COLUMN_REPLIES		= 8,
							TUN_SOURCE_LIST_COLUMN_COMMANDS		= 9;

const int					TuningSourceColumnWidth[TUN_SOURCE_LIST_COLUMN_COUNT] =
{
							250,	 // TUN_SOURCE_LIST_COLUMN_EQUIPMENT_ID
							150,	 // TUN_SOURCE_LIST_COLUMN_CAPTION
							150,	 // TUN_SOURCE_LIST_COLUMN_IP
							100,	 // TUN_SOURCE_LIST_COLUMN_CHANNEL
							100,	 // TUN_SOURCE_LIST_COLUMN_SUBSYSTEM
							100,	 // TUN_SOURCE_LIST_COLUMN_LM_NUMBER
							100,	 // TUN_SOURCE_LIST_COLUMN_IS_REPLY
							100,	 // TUN_SOURCE_LIST_COLUMN_REQUESTS
							100,	 // TUN_SOURCE_LIST_COLUMN_REPLIES
							100,	 // TUN_SOURCE_LIST_COLUMN_COMMANDS
};

// ==============================================================================================

class TuningSourceTable : public ListTable<TuningSource>
{
	Q_OBJECT

public:

	explicit TuningSourceTable(QObject* parent = nullptr) { Q_UNUSED(parent) }
	virtual ~TuningSourceTable() override {}

public:

	QString					text(int row, int column, const TuningSource& source, const TuningSourceState& state) const;

private:

	QVariant				data(const QModelIndex &index, int role) const override;
};

// ==============================================================================================

class DialogTuningSourceList : public DialogList
{
	Q_OBJECT

public:

	explicit DialogTuningSourceList(QWidget* parent = nullptr);
	virtual ~DialogTuningSourceList() override;

private:

	QMenu*					m_pSourceMenu = nullptr;
	QMenu*					m_pEditMenu = nullptr;

	TuningSourceTable		m_sourceTable;

	void					createInterface();
	void					createContextMenu();

	QTimer*					m_updateSourceStateTimer = nullptr;
	void					startSourceStateTimer();
	void					stopSourceStateTimer();

public slots:

	// slots for updating source signal list
	//
	void					updateList() override;

private slots:

	// slot informs that signal for measure has updated his state
	//
	void					updateState();
};

// ==============================================================================================

#endif // DIALOGTUNINGSOURCELIST_H
