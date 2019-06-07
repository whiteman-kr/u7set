#ifndef FRAMEBASE_H
#define FRAMEBASE_H

#include <QAbstractTableModel>
#include <QColor>
#include <QMutex>
#include <QIcon>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QIntValidator>

#include "../../lib/DataProtocols.h"

// ==============================================================================================

namespace PS
{
	class FrameData : public QObject
	{
		Q_OBJECT

	public:

		FrameData();
		FrameData(const FrameData& from);
		virtual ~FrameData();

	private:

		mutable QMutex		m_frameMutex;

		Rup::Data			m_data;

	public:

		void				clear();

		Rup::Data&			data() { return m_data; }

		FrameData&			operator=(const FrameData& from);

	signals:

	public slots:

	};
}

// ==============================================================================================

class FrameBase : public QObject
{
	Q_OBJECT

public:

	explicit FrameBase(QObject *parent = nullptr);
	virtual ~FrameBase();

private:

	mutable QMutex			m_frameMutex;
	QVector<PS::FrameData>	m_frameList;

public:

	void					clear();
	int						count() const;

	bool					setFrameCount(int count);

	PS::FrameData			frameData(int index) const;
	PS::FrameData*			frameDataPtr(int index);
	void					setFrameData(int index, const PS::FrameData& frameData);

	FrameBase&				operator=(const FrameBase& from);

signals:

public slots:

};

// ==============================================================================================

const char* const			FrameDataListColumn[] =
{
							QT_TRANSLATE_NOOP("DataList.h", "Dec"),
							QT_TRANSLATE_NOOP("DataList.h", "Hex"),
};

const int					FRAME_LIST_COLUMN_COUNT		= sizeof(FrameDataListColumn)/sizeof(FrameDataListColumn[0]);

const int					FRAME_LIST_COLUMN_DEC		= 0,
							FRAME_LIST_COLUMN_HEX		= 1;


const int					FrameListColumnWidth[FRAME_LIST_COLUMN_COUNT] =
{
							50, // FRAME_LIST_COLUMN_DEC
							50, // FRAME_LIST_COLUMN_HEX
};

// ==============================================================================================

const int					UPDATE_FRAME_STATE_TIMEOUT		= 250; // 250 ms

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
	void					set(const QVector<PS::FrameData*> list_add);
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

#endif // FRAMEBASE_H
