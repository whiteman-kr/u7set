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

#endif // FRAMEBASE_H
