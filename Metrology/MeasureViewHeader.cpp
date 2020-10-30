#include "MeasureViewHeader.h"

#include <assert.h>
#include <QSettings>

#include "Options.h"

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureViewColumn MeasureViewHeader::m_column[MEASURE_TYPE_COUNT][MEASURE_VIEW_COLUMN_COUNT] =
{
	// Measurements of linearity
	{
		MeasureViewColumn("MVC_CMN_L_INDEX", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Index"), 100, MVC_CMN_HIDE, Qt::AlignLeft, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_MODULE_SN", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Module SN"), 100, MVC_CMN_HIDE, Qt::AlignHCenter, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_CONNECT_APP_ID", QT_TRANSLATE_NOOP("MeasureViewHeader",	"ConnectAppSignalID"), 150, MVC_CMN_HIDE, Qt::AlignLeft, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_CONNECT_TYPE", QT_TRANSLATE_NOOP("MeasureViewHeader",	"ConnectType"), 100, MVC_CMN_HIDE, Qt::AlignLeft, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_APP_ID", QT_TRANSLATE_NOOP("MeasureViewHeader",	"AppSignalID"), 150, MVC_CMN_SHOW, Qt::AlignLeft, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_CUSTOM_ID", QT_TRANSLATE_NOOP("MeasureViewHeader",	"CustomAppSignalID"), 150, MVC_CMN_HIDE, Qt::AlignLeft, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_EQUIPMENT_ID", QT_TRANSLATE_NOOP("MeasureViewHeader",	"EquipmentID"), 150, MVC_CMN_HIDE, Qt::AlignLeft, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_NAME", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Caption"), 200, MVC_CMN_SHOW, Qt::AlignLeft, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_RACK", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Rack"), 100, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_CHASSIS", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Chassis"), 60, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_MODULE", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Module"), 60, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_PLACE", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Place"), 60, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_EL_RANGE", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Electric range"), 150, MVC_CMN_HIDE, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_EN_RANGE", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Engineering range"), 150, MVC_CMN_HIDE, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_EL_NOMINAL", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Electric nominal"), 130, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_EN_NOMINAL", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Engineering nominal"), 130, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_PERCENT", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value to %"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_EL_MEASURE", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Electric measure"), 130, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_EN_MEASURE", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Engineering measure"), 130, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_SYSTEM_ERROR", QT_TRANSLATE_NOOP("MeasureViewHeader",	"System error"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_SD", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Standard deviation"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_BORDER", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Borders"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_UNCERTAINTY", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Uncertainty"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_VALUE_COUNT", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Amount measuremets"), 80, MVC_CMN_HIDE, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_VALUE_0", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value 1"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_VALUE_1", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value 2"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_VALUE_2", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value 3"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_VALUE_3", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value 4"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_VALUE_4", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value 5"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_VALUE_5", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value 6"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_VALUE_6", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value 7"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_VALUE_7", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value 8"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_VALUE_8", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value 9"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_VALUE_9", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value 10"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_VALUE_10", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value 11"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_VALUE_11", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value 12"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_VALUE_12", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value 13"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_VALUE_13", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value 14"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_VALUE_14", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value 15"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_VALUE_15", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value 16"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_VALUE_16", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value 17"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_VALUE_17", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value 18"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_VALUE_18", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value 19"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_VALUE_19", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value 20"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_ERROR", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Error"), 100, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_ERROR_LIMIT", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Limit of error"), 100, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_ERROR_RESULT", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Result"), 100, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_MEASUREMENT_TIME", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Measurement time"), 150, MVC_CMN_HIDE, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_L_CALIBRATOR", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Calibrator"), 150, MVC_CMN_HIDE, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
	},

	// Measurements of comparators

	{
		MeasureViewColumn("MVC_CMN_C_INDEX", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Index"), 100, MVC_CMN_HIDE, Qt::AlignLeft, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_MODULE_SN", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Module SN"), 100, MVC_CMN_HIDE, Qt::AlignHCenter, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_CONNECT_APP_ID", QT_TRANSLATE_NOOP("MeasureViewHeader",	"ConnectAppSignalID"), 150, MVC_CMN_HIDE, Qt::AlignLeft, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_CONNECT_TYPE", QT_TRANSLATE_NOOP("MeasureViewHeader",	"ConnectType"), 100, MVC_CMN_HIDE, Qt::AlignLeft, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_APP_ID", QT_TRANSLATE_NOOP("MeasureViewHeader",	"AppSignalID"), 150, MVC_CMN_SHOW, Qt::AlignLeft, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_CUSTOM_ID", QT_TRANSLATE_NOOP("MeasureViewHeader",	"CustomAppSignalID"), 150, MVC_CMN_HIDE, Qt::AlignLeft, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_EQUIPMENT_ID", QT_TRANSLATE_NOOP("MeasureViewHeader",	"EquipmentID"), 150, MVC_CMN_HIDE, Qt::AlignLeft, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_NAME", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Caption"), 200, MVC_CMN_SHOW, Qt::AlignLeft, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_RACK", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Rack"), 100, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_CHASSIS", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Chassis"), 60, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_MODULE", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Module"), 60, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_PLACE", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Place"), 60, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_EL_RANGE", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Electric range"), 150, MVC_CMN_HIDE, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_EN_RANGE", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Engineering range"), 150, MVC_CMN_HIDE, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_SP_TYPE", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Value type"), 100, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_CMP_TYPE", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Compare type"), 30, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_EL_NOMINAL", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Electric nominal"), 130, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_EN_NOMINAL", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Engineering nominal"), 130, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_EL_MEASURE", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Electric measure"), 130, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_EN_MEASURE", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Engineering measure"), 130, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_CMP_ID", QT_TRANSLATE_NOOP("MeasureViewHeader",	"CompareAppSignalID"), 150, MVC_CMN_HIDE, Qt::AlignLeft, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_OUT_ID", QT_TRANSLATE_NOOP("MeasureViewHeader",	"OutputAppSignalID"), 150, MVC_CMN_SHOW, Qt::AlignLeft, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_ERROR", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Error"), 100, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_ERROR_LIMIT", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Limit of error"), 100, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_ERROR_RESULT", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Result"), 100, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_MEASUREMENT_TIME", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Measurement time"), 150, MVC_CMN_HIDE, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn("MVC_CMN_C_CALIBRATOR", QT_TRANSLATE_NOOP("MeasureViewHeader",	"Calibrator"), 150, MVC_CMN_HIDE, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
		MeasureViewColumn(),
	},
};

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureViewColumn::MeasureViewColumn()
{
}

// -------------------------------------------------------------------------------------------------------------------

MeasureViewColumn::MeasureViewColumn(const MeasureViewColumn& from)
{
	*this = from;
}

// -------------------------------------------------------------------------------------------------------------------

MeasureViewColumn::MeasureViewColumn(const QString& uniqueTitle, const QString& title, int width, bool visible, int alignment, bool duplicate) :
	m_uniqueTitle(uniqueTitle),
	m_title(title) ,
	m_width(width) ,
	m_enableVisible(visible) ,
	m_alignment(alignment) ,
	m_enableDuplicate(duplicate)
{
}

// -------------------------------------------------------------------------------------------------------------------

MeasureViewColumn::~MeasureViewColumn()
{
}

// -------------------------------------------------------------------------------------------------------------------

QString MeasureViewColumn::title() const
{
	 return qApp->translate("MeasureViewHeader", m_title.toUtf8());
}

// -------------------------------------------------------------------------------------------------------------------

MeasureViewColumn& MeasureViewColumn::operator=(const MeasureViewColumn& from)
{
	m_index = from.m_index;
	m_uniqueTitle = from.m_uniqueTitle;

	m_title = from.m_title;
	m_enableVisible = from.m_enableVisible;
	m_width = from.m_width;

	m_alignment = from.m_alignment;

	m_enableDuplicate = from.m_enableDuplicate;

	return *this;
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------

MeasureViewHeader::MeasureViewHeader(QObject *parent) :
	QObject(parent)
{
	for(int type = 0; type < MEASURE_TYPE_COUNT; type++)
	{
		m_columnCount[type] = 0;
	}
}

// -------------------------------------------------------------------------------------------------------------------

MeasureViewHeader::~MeasureViewHeader()
{
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureViewHeader::setMeasureType(int measureType)
{
	if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	m_measureType = measureType;

	for(int column = 0; column < MEASURE_VIEW_COLUMN_COUNT; column++)
	{
		if (m_column[measureType][column].title().isEmpty() == false)
		{
			continue;
		}

		m_columnCount[measureType] = column;
		break;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureViewHeader::init(int measureType)
{
	if (measureType < 0 || measureType >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	int languageType = theOptions.language().languageType();
	if (languageType < 0 || languageType >= LANGUAGE_TYPE_COUNT)
	{
		return;
	}

	setMeasureType(measureType);

	for(int column = 0; column < MEASURE_VIEW_COLUMN_COUNT; column++)
	{
		MeasureViewColumn& c = m_column[measureType][column];

		if (c.title().isEmpty() == true)
		{
			continue;
		}

		c = theOptions.measureView().m_column[measureType][languageType][column];
		c.setIndex(column);
	}

}

// -------------------------------------------------------------------------------------------------------------------

int MeasureViewHeader::count() const
{
	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return 0;
	}

	return m_columnCount[m_measureType];
}

// -------------------------------------------------------------------------------------------------------------------

MeasureViewColumn* MeasureViewHeader::column(int index) const
{
	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return nullptr;
	}

	if (index < 0 || index >= count())
	{
		return nullptr;
	}

	return &m_column[m_measureType][index];
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureViewHeader::updateColumnState()
{
	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	switch (m_measureType)
	{
		case MEASURE_TYPE_LINEARITY:
			{
				// list type
				//
				switch(theOptions.linearity().viewType())
				{
					case LO_VIEW_TYPE_SIMPLE:

						setColumnVisible(MVC_CMN_L_PERCENT, false);
						setColumnVisible(MVC_CMN_L_SYSTEM_ERROR, false);
						setColumnVisible(MVC_CMN_L_SD, false);
						setColumnVisible(MVC_CMN_L_BORDER, false);
						setColumnVisible(MVC_CMN_L_UNCERTAINTY, false);

						for (int m = 0; m < MAX_MEASUREMENT_IN_POINT; m ++)
						{
							setColumnVisible(m + MVC_CMN_L_VALUE_0, false);
						}


						setColumnVisible(MVC_CMN_L_ERROR, true);
						setColumnVisible(MVC_CMN_L_ERROR_LIMIT, true);
						setColumnVisible(MVC_CMN_L_ERROR_RESULT, true);

						break;

					case LO_VIEW_TYPE_EXTENDED:

						setColumnVisible(MVC_CMN_L_PERCENT, true);
						setColumnVisible(MVC_CMN_L_SYSTEM_ERROR, true);
						setColumnVisible(MVC_CMN_L_SD, true);
						setColumnVisible(MVC_CMN_L_BORDER, true);
						setColumnVisible(MVC_CMN_L_UNCERTAINTY, true);

						for (int m = 0; m < MAX_MEASUREMENT_IN_POINT; m ++)
						{
							setColumnVisible(m + MVC_CMN_L_VALUE_0, false);
						}


						setColumnVisible(MVC_CMN_L_ERROR, true);
						setColumnVisible(MVC_CMN_L_ERROR_LIMIT, true);
						setColumnVisible(MVC_CMN_L_ERROR_RESULT, true);

						break;

					case LO_VIEW_TYPE_DETAIL_ELRCTRIC:
					case LO_VIEW_TYPE_DETAIL_ENGINEERING:

						setColumnVisible(MVC_CMN_L_PERCENT, false);
						setColumnVisible(MVC_CMN_L_SYSTEM_ERROR, false);
						setColumnVisible(MVC_CMN_L_SD, false);
						setColumnVisible(MVC_CMN_L_BORDER, false);
						setColumnVisible(MVC_CMN_L_UNCERTAINTY, false);

						for (int m = 0; m < MAX_MEASUREMENT_IN_POINT; m ++)
						{
							setColumnVisible(m + MVC_CMN_L_VALUE_0, true);
						}

						setColumnVisible(MVC_CMN_L_ERROR, false);
						setColumnVisible(MVC_CMN_L_ERROR_LIMIT, false);
						setColumnVisible(MVC_CMN_L_ERROR_RESULT, false);

						break;

					default:
						assert(0);
						break;
				}
			}
			break;

		case MEASURE_TYPE_COMPARATOR:
			{
				// show  or hide columns
				//
			}
			break;

		default:
			assert(0);
			break;
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureViewHeader::setColumnTitle(int column, const QString& title)
{
	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	if (column < 0 || column >= MEASURE_VIEW_COLUMN_COUNT)
	{
		return;
	}

	m_column[m_measureType][column].setTitle(title);
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureViewHeader::setColumnVisible(int column, bool visible)
{
	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	if (column < 0 || column >= MEASURE_VIEW_COLUMN_COUNT)
	{
		return;
	}

	m_column[m_measureType][column].setVisible(visible);
}

// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------------------------
