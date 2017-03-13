#ifndef MEASUREVIEWHEADER_H
#define MEASUREVIEWHEADER_H

#include <QColor>

#include "Measure.h"

// ==============================================================================================

#define					MVC_CMN_HIDE				false
#define					MVC_CMN_SHOW				true

#define					MVC_CMN_DISABLE_DUPLICATE	false
#define					MVC_CMN_ENABLE_DUPLICATE	true

// ==============================================================================================

#define					MVC_CMN_COLOR_LIGHT_BLUE	QColor(0xE0, 0xFF, 0xE0)

// ==============================================================================================

class MeasureViewColumn
{
public:

	MeasureViewColumn();
	MeasureViewColumn(const MeasureViewColumn& from);
	MeasureViewColumn(const QString& title, int width, bool visible, int alignment, bool duplicate);
	~MeasureViewColumn();

private:

	int					m_index = -1;

	QString				m_title;
	int					m_width = 100;
	bool				m_enableVisible = MVC_CMN_SHOW;

	int					m_alignment = Qt::AlignLeft;

	bool				m_enableDuplicate = MVC_CMN_ENABLE_DUPLICATE;

public:

	int					index() const { return m_index; }
	void				setIndex(int index) { m_index = index; }

	QString				title() const { return m_title; }
	void				setTitle(const QString& title) { m_title = title; }

	int					width() const { return m_width; }
	void				setWidth(int width) { m_width = width; }

	bool				enableVisible() const { return m_enableVisible; }
	void				setVisible(bool enable) { m_enableVisible = enable; }

	int					alignment() const { return m_alignment; }

	bool				enableDuplicate() const { return m_enableDuplicate; }

	MeasureViewColumn&	operator=(const MeasureViewColumn& from);
};

// ==============================================================================================

const int				MEASURE_VIEW_COLUMN_COUNT	= 64;

// ==============================================================================================

const int				MEASURE_VIEW_COLUMN_INDEX	= 0,
						MEASURE_VIEW_COLUMN_RACK	= 1,
						MEASURE_VIEW_COLUMN_ID		= 2;

// ==============================================================================================

class MeasureViewHeader : public QObject
{
	Q_OBJECT

public:

	explicit MeasureViewHeader(QObject *parent = 0);
	~MeasureViewHeader();

private:

	static MeasureViewColumn m_column[MEASURE_TYPE_COUNT][MEASURE_VIEW_COLUMN_COUNT];

	int					m_measureType = MEASURE_TYPE_UNKNOWN;

	int					m_columnCount[MEASURE_TYPE_COUNT];

public:

	void				setMeasureType(int measureType);

	void				init(int type);

	int					count() const;
	MeasureViewColumn*	column(int index) const;

	void				updateColumnState();

	void				setColumnTitle(int column, const QString& title);
	void				setColumnVisible(int column, bool visible);

};

// ==============================================================================================

const int				MVC_CMN_L_INDEX				= 0;
const int				MVC_CMN_L_RACK				= 1;
const int				MVC_CMN_L_ID				= 2;
const int				MVC_CMN_L_NAME				= 3;
const int				MVC_CMN_L_CHASSIS			= 4;
const int				MVC_CMN_L_MODULE			= 5;
const int				MVC_CMN_L_PLACE				= 6;
const int				MVC_CMN_L_IN_EL_RANGE		= 7;
const int				MVC_CMN_L_PH_RANGE			= 8;
const int				MVC_CMN_L_OUT_RANGE			= 9;
const int				MVC_CMN_L_IN_EL_NOMINAL		= 10;
const int				MVC_CMN_L_PH_NOMINAL		= 11;
const int				MVC_CMN_L_OUT_NOMINAL		= 12;
const int				MVC_CMN_L_PERCENT			= 13;
const int				MVC_CMN_L_IN_EL_MEASURE		= 14;
const int				MVC_CMN_L_PH_MEASURE		= 15;
const int				MVC_CMN_L_OUT_MEASURE		= 16;
const int				MVC_CMN_L_SYSTEM_ERROR		= 17;
const int				MVC_CMN_L_MSE				= 18;
const int				MVC_CMN_L_BORDER			= 19;
const int				MVC_CMN_L_VALUE_COUNT		= 20;
const int				MVC_CMN_L_VALUE_0			= 21;
const int				MVC_CMN_L_VALUE_1			= 22;
const int				MVC_CMN_L_VALUE_2			= 23;
const int				MVC_CMN_L_VALUE_3			= 24;
const int				MVC_CMN_L_VALUE_4			= 25;
const int				MVC_CMN_L_VALUE_5			= 26;
const int				MVC_CMN_L_VALUE_6			= 27;
const int				MVC_CMN_L_VALUE_7			= 28;
const int				MVC_CMN_L_VALUE_8			= 29;
const int				MVC_CMN_L_VALUE_9			= 30;
const int				MVC_CMN_L_VALUE_10			= 31;
const int				MVC_CMN_L_VALUE_11			= 32;
const int				MVC_CMN_L_VALUE_12			= 33;
const int				MVC_CMN_L_VALUE_13			= 34;
const int				MVC_CMN_L_VALUE_14			= 35;
const int				MVC_CMN_L_VALUE_15			= 36;
const int				MVC_CMN_L_VALUE_16			= 37;
const int				MVC_CMN_L_VALUE_17			= 38;
const int				MVC_CMN_L_VALUE_18			= 39;
const int				MVC_CMN_L_VALUE_19			= 40;
const int				MVC_CMN_L_IN_ERROR			= 41;
const int				MVC_CMN_L_IN_ERROR_LIMIT	= 42;
const int				MVC_CMN_L_OUT_ERROR			= 43;
const int				MVC_CMN_L_OUT_ERROR_LIMIT	= 44;
const int				MVC_CMN_L_MEASUREMENT_TIME	= 45;

// ==============================================================================================

#endif // MEASUREVIEWHEADER_H
