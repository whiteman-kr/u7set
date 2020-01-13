#ifndef MEASUREVIEWHEADER_H
#define MEASUREVIEWHEADER_H

#include <QColor>

#include "MeasureBase.h"

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
	virtual ~MeasureViewColumn();

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

	explicit MeasureViewHeader(QObject *parent = nullptr);
	virtual ~MeasureViewHeader();

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
// MEASURE_TYPE_LINEARITY
//
const int				MVC_CMN_L_INDEX				= 0,
						MVC_CMN_L_MODULE_SN			= 1,
						MVC_CMN_L_APP_ID			= 2,
						MVC_CMN_L_CUSTOM_ID			= 3,
						MVC_CMN_L_EQUIPMENT_ID		= 4,
						MVC_CMN_L_NAME				= 5,
						MVC_CMN_L_RACK				= 6,
						MVC_CMN_L_CHASSIS			= 7,
						MVC_CMN_L_MODULE			= 8,
						MVC_CMN_L_PLACE				= 9,
						MVC_CMN_L_EL_RANGE			= 10,
						MVC_CMN_L_EN_RANGE			= 11,
						MVC_CMN_L_EL_NOMINAL		= 12,
						MVC_CMN_L_EN_NOMINAL		= 13,
						MVC_CMN_L_PERCENT			= 14,
						MVC_CMN_L_EL_MEASURE		= 15,
						MVC_CMN_L_EN_MEASURE		= 16,
						MVC_CMN_L_SYSTEM_ERROR		= 17,
						MVC_CMN_L_SD				= 18,
						MVC_CMN_L_BORDER			= 19,
						MVC_CMN_L_VALUE_COUNT		= 20,
						MVC_CMN_L_VALUE_0			= 21,
						MVC_CMN_L_VALUE_1			= 22,
						MVC_CMN_L_VALUE_2			= 23,
						MVC_CMN_L_VALUE_3			= 24,
						MVC_CMN_L_VALUE_4			= 25,
						MVC_CMN_L_VALUE_5			= 26,
						MVC_CMN_L_VALUE_6			= 27,
						MVC_CMN_L_VALUE_7			= 28,
						MVC_CMN_L_VALUE_8			= 29,
						MVC_CMN_L_VALUE_9			= 30,
						MVC_CMN_L_VALUE_10			= 31,
						MVC_CMN_L_VALUE_11			= 32,
						MVC_CMN_L_VALUE_12			= 33,
						MVC_CMN_L_VALUE_13			= 34,
						MVC_CMN_L_VALUE_14			= 35,
						MVC_CMN_L_VALUE_15			= 36,
						MVC_CMN_L_VALUE_16			= 37,
						MVC_CMN_L_VALUE_17			= 38,
						MVC_CMN_L_VALUE_18			= 39,
						MVC_CMN_L_VALUE_19			= 40,
						MVC_CMN_L_ERROR				= 41,
						MVC_CMN_L_ERROR_LIMIT		= 42,
						MVC_CMN_L_ERROR_RESULT		= 43,
						MVC_CMN_L_MEASUREMENT_TIME	= 44;

// ==============================================================================================
// MEASURE_TYPE_COMPARATOR
//
const int				MVC_CMN_C_INDEX				= 0,
						MVC_CMN_C_MODULE_SN			= 1,
						MVC_CMN_C_APP_ID			= 2,
						MVC_CMN_C_CUSTOM_ID			= 3,
						MVC_CMN_C_EQUIPMENT_ID		= 4,
						MVC_CMN_C_NAME				= 5,
						MVC_CMN_C_RACK				= 6,
						MVC_CMN_C_CHASSIS			= 7,
						MVC_CMN_C_MODULE			= 8,
						MVC_CMN_C_PLACE				= 9,
						MVC_CMN_C_EL_RANGE			= 10,
						MVC_CMN_C_EN_RANGE			= 11,
						MVC_CMN_C_CMP_TYPE			= 12,
						MVC_CMN_C_EL_NOMINAL		= 13,
						MVC_CMN_C_EN_NOMINAL		= 14,
						MVC_CMN_C_EL_MEASURE		= 15,
						MVC_CMN_C_EN_MEASURE		= 16,
						MVC_CMN_C_ERROR				= 17,
						MVC_CMN_C_ERROR_LIMIT		= 18,
						MVC_CMN_C_ERROR_RESULT		= 19,
						MVC_CMN_C_MEASUREMENT_TIME	= 20;

// ==============================================================================================

#endif // MEASUREVIEWHEADER_H
