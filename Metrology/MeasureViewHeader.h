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
	MeasureViewColumn(const QString& uniqueTitle, const QString& title, int width, bool visible, int alignment, bool duplicate);
	virtual ~MeasureViewColumn();

private:

	int					m_index = -1;

	QString				m_uniqueTitle;
	QString				m_title;
	int					m_width = 100;
	bool				m_enableVisible = MVC_CMN_SHOW;

	int					m_alignment = Qt::AlignLeft;

	bool				m_enableDuplicate = MVC_CMN_ENABLE_DUPLICATE;

public:

	int					index() const { return m_index; }
	void				setIndex(int index) { m_index = index; }

	QString				uniqueTitle() const { return m_uniqueTitle; }
	void				setUniqueTitle(const QString& title) { m_uniqueTitle = title; }

	QString				title() const;
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

class MeasureViewHeader : public QObject
{
	Q_OBJECT

public:

	explicit MeasureViewHeader(QObject *parent = nullptr);
	virtual ~MeasureViewHeader();

private:

	static				MeasureViewColumn m_column[MEASURE_TYPE_COUNT][MEASURE_VIEW_COLUMN_COUNT];

	int					m_measureType = MEASURE_TYPE_UNDEFINED;

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
						MVC_CMN_L_CONNECT_APP_ID	= 2,
						MVC_CMN_L_CONNECT_TYPE		= 3,
						MVC_CMN_L_APP_ID			= 4,
						MVC_CMN_L_CUSTOM_ID			= 5,
						MVC_CMN_L_EQUIPMENT_ID		= 6,
						MVC_CMN_L_NAME				= 7,
						MVC_CMN_L_RACK				= 8,
						MVC_CMN_L_CHASSIS			= 9,
						MVC_CMN_L_MODULE			= 10,
						MVC_CMN_L_PLACE				= 11,
						MVC_CMN_L_EL_RANGE			= 12,
						MVC_CMN_L_EN_RANGE			= 13,
						MVC_CMN_L_EL_NOMINAL		= 14,
						MVC_CMN_L_EN_NOMINAL		= 15,
						MVC_CMN_L_PERCENT			= 16,
						MVC_CMN_L_EL_MEASURE		= 17,
						MVC_CMN_L_EN_MEASURE		= 18,
						MVC_CMN_L_SYSTEM_DEVIATION	= 19,
						MVC_CMN_L_SD				= 20,
						MVC_CMN_L_BORDER			= 21,
						MVC_CMN_L_UNCERTAINTY		= 22,
						MVC_CMN_L_VALUE_COUNT		= 23,
						MVC_CMN_L_VALUE_0			= 24,
						MVC_CMN_L_VALUE_1			= 25,
						MVC_CMN_L_VALUE_2			= 26,
						MVC_CMN_L_VALUE_3			= 27,
						MVC_CMN_L_VALUE_4			= 28,
						MVC_CMN_L_VALUE_5			= 29,
						MVC_CMN_L_VALUE_6			= 30,
						MVC_CMN_L_VALUE_7			= 31,
						MVC_CMN_L_VALUE_8			= 32,
						MVC_CMN_L_VALUE_9			= 33,
						MVC_CMN_L_VALUE_10			= 34,
						MVC_CMN_L_VALUE_11			= 35,
						MVC_CMN_L_VALUE_12			= 36,
						MVC_CMN_L_VALUE_13			= 37,
						MVC_CMN_L_VALUE_14			= 38,
						MVC_CMN_L_VALUE_15			= 39,
						MVC_CMN_L_VALUE_16			= 40,
						MVC_CMN_L_VALUE_17			= 41,
						MVC_CMN_L_VALUE_18			= 42,
						MVC_CMN_L_VALUE_19			= 43,
						MVC_CMN_L_ERROR				= 44,
						MVC_CMN_L_ERROR_LIMIT		= 45,
						MVC_CMN_L_ERROR_RESULT		= 46,
						MVC_CMN_L_MEASUREMENT_TIME	= 47,
						MVC_CMN_L_CALIBRATOR		= 48;

// ==============================================================================================
// MEASURE_TYPE_COMPARATOR
//
const int				MVC_CMN_C_INDEX				= 0,
						MVC_CMN_C_MODULE_SN			= 1,
						MVC_CMN_C_CONNECT_APP_ID	= 2,
						MVC_CMN_C_CONNECT_TYPE		= 3,
						MVC_CMN_C_APP_ID			= 4,
						MVC_CMN_C_CUSTOM_ID			= 5,
						MVC_CMN_C_EQUIPMENT_ID		= 6,
						MVC_CMN_C_NAME				= 7,
						MVC_CMN_C_RACK				= 8,
						MVC_CMN_C_CHASSIS			= 9,
						MVC_CMN_C_MODULE			= 10,
						MVC_CMN_C_PLACE				= 11,
						MVC_CMN_C_EL_RANGE			= 12,
						MVC_CMN_C_EN_RANGE			= 13,
						MVC_CMN_C_SP_TYPE			= 14,
						MVC_CMN_C_CMP_TYPE			= 15,
						MVC_CMN_C_EL_NOMINAL		= 16,
						MVC_CMN_C_EN_NOMINAL		= 17,
						MVC_CMN_C_EL_MEASURE		= 18,
						MVC_CMN_C_EN_MEASURE		= 19,
						MVC_CMN_C_CMP_ID			= 20,
						MVC_CMN_C_OUT_ID			= 21,
						MVC_CMN_C_ERROR				= 22,
						MVC_CMN_C_ERROR_LIMIT		= 23,
						MVC_CMN_C_ERROR_RESULT		= 24,
						MVC_CMN_C_MEASUREMENT_TIME	= 25,
						MVC_CMN_C_CALIBRATOR		= 26;

// ==============================================================================================

const int				MVG_TYPE_UNDEFINED			= -1,
						MVG_TYPE_LIN_EL				= 0,
						MVG_TYPE_LIN_EN				= 1,
						MVG_TYPE_20VAL_EL			= 2,
						MVG_TYPE_20VAL_EN			= 3;

const int				MVG_TYPE_COUNT				= 4;

// ==============================================================================================

#endif // MEASUREVIEWHEADER_H
