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
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Index"), 100, MVC_CMN_HIDE, Qt::AlignLeft, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Rack"), 100, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "AppSignalID"), 150, MVC_CMN_SHOW, Qt::AlignLeft, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "CustomAppSignalID"), 150, MVC_CMN_HIDE, Qt::AlignLeft, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "EquipmentID"), 150, MVC_CMN_HIDE, Qt::AlignLeft, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Caption"), 200, MVC_CMN_SHOW, Qt::AlignLeft, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Chassis"), 60, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Module"), 60, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Place"), 60, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_DISABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "El. range"), 150, MVC_CMN_HIDE, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Ph. range"), 150, MVC_CMN_HIDE, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Out. El. range"), 150, MVC_CMN_HIDE, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "El. nominal value"), 130, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Ph. nominal value"), 130, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Out. El. nominal value"), 130, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Value to %"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "El. measure value"), 130, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Ph. measure value"), 130, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Out. El. measure value"), 130, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "System error"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "MSE"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Borders"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Amount measuremets"), 80, MVC_CMN_HIDE, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Value 1"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Value 2"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Value 3"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Value 4"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Value 5"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Value 6"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Value 7"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Value 8"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Value 9"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Value 10"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Value 11"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Value 12"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Value 13"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Value 14"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Value 15"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Value 16"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Value 17"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Value 18"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Value 19"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Value 20"), 80, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Error of input"), 100, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Limit of error (in)"), 100, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Error of output"), 100, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Limit of error (out)"), 100, MVC_CMN_SHOW, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
		MeasureViewColumn(QT_TRANSLATE_NOOP("MeasureViewHeader", "Measurement time"), 150, MVC_CMN_HIDE, Qt::AlignHCenter, MVC_CMN_ENABLE_DUPLICATE),
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

	// Measurements of comparators

	{
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

MeasureViewColumn::MeasureViewColumn(const QString& title, int width, bool visible, int alignment, bool duplicate) :
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

MeasureViewColumn& MeasureViewColumn::operator=(const MeasureViewColumn& from)
{
	m_index = from.m_index;

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
//	if (m_measureType < 0 || m_measureType >= MEASURE_TYPE_COUNT)
//	{
//		return;
//	}

//	for(int column = 0; column < MEASURE_VIEW_COLUMN_COUNT; column++)
//	{
//		theOptions.measureView().m_column[m_measureType][column] = m_column[m_measureType][column];
//	}

//	theOptions.measureView().save();
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
		if (m_column[measureType][column].title().isEmpty() == true)
		{
			m_columnCount[measureType] = column;
			break;
		}
	}
}

// -------------------------------------------------------------------------------------------------------------------

void MeasureViewHeader::init(int type)
{
	if (type < 0 || type >= MEASURE_TYPE_COUNT)
	{
		return;
	}

	setMeasureType(type);

	for(int column = 0; column < MEASURE_VIEW_COLUMN_COUNT; column++)
	{
		if (m_column[type][column].title().isEmpty() == false)
		{
			m_column[type][column] = theOptions.measureView().m_column[type][column];
			m_column[type][column].setIndex(column);
		}
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

				// input and output range
				//
				bool visibleInput = theOptions.linearity().showInputRangeColumn();
				bool visibleOutput = theOptions.linearity().showOutputRangeColumn();

				setColumnVisible(MVC_CMN_L_IN_EL_NOMINAL, visibleInput);
				setColumnVisible(MVC_CMN_L_IN_EL_MEASURE, visibleInput);
				setColumnVisible(MVC_CMN_L_IN_ERROR, visibleInput);
				setColumnVisible(MVC_CMN_L_IN_ERROR_LIMIT, visibleInput);

				setColumnVisible(MVC_CMN_L_OUT_NOMINAL, visibleOutput);
				setColumnVisible(MVC_CMN_L_OUT_MEASURE, visibleOutput);
				setColumnVisible(MVC_CMN_L_OUT_ERROR, visibleOutput);
				setColumnVisible(MVC_CMN_L_OUT_ERROR_LIMIT, visibleOutput);

				if (theOptions.linearity().errorType() == MEASURE_ERROR_TYPE_ABSOLUTE)
				{
					setColumnTitle(MVC_CMN_L_IN_ERROR, visibleOutput == false ? "Error" : "Error of input");
					setColumnTitle(MVC_CMN_L_IN_ERROR_LIMIT, visibleOutput == false ? "Limit of error" : "Limit of error (in)");
					setColumnTitle(MVC_CMN_L_OUT_ERROR, visibleInput == false ? "Error" : "Error of output");
					setColumnTitle(MVC_CMN_L_OUT_ERROR_LIMIT, visibleInput == false ? "Limit of error" : "Limit of error (out)");
				}
				else
				{
					setColumnTitle(MVC_CMN_L_IN_ERROR, visibleOutput == false ? "Error" : "Error of input");
					setColumnTitle(MVC_CMN_L_IN_ERROR_LIMIT, "Limit of error");
					setColumnTitle(MVC_CMN_L_OUT_ERROR, visibleInput == false ? "Error" : "Error of output");
					setColumnTitle(MVC_CMN_L_OUT_ERROR_LIMIT, "Limit of error");

					if (visibleOutput == true)
					{
						setColumnVisible(MVC_CMN_L_IN_ERROR_LIMIT, false);
					}
				}

				// list type
				//
				switch(theOptions.linearity().viewType())
				{
					case LO_VIEW_TYPE_SIMPLE:

						setColumnVisible(MVC_CMN_L_PERCENT, false);
						setColumnVisible(MVC_CMN_L_SYSTEM_ERROR, false);
						setColumnVisible(MVC_CMN_L_MSE, false);
						setColumnVisible(MVC_CMN_L_BORDER, false);

						for (int m = 0; m < MAX_MEASUREMENT_IN_POINT; m ++)
						{
							setColumnVisible(m + MVC_CMN_L_VALUE_0, false);
						}

						break;

					case LO_VIEW_TYPE_EXTENDED:

						setColumnVisible(MVC_CMN_L_PERCENT, true);
						setColumnVisible(MVC_CMN_L_SYSTEM_ERROR, true);
						setColumnVisible(MVC_CMN_L_MSE, true);
						setColumnVisible(MVC_CMN_L_BORDER, true);

						for (int m = 0; m < MAX_MEASUREMENT_IN_POINT; m ++)
						{
							setColumnVisible(m + MVC_CMN_L_VALUE_0, false);
						}

						break;

					case LO_VIEW_TYPE_DETAIL_ELRCTRIC:
					case LO_VIEW_TYPE_DETAIL_PHYSICAL:

						setColumnVisible(MVC_CMN_L_PERCENT, false);
						setColumnVisible(MVC_CMN_L_SYSTEM_ERROR, false);
						setColumnVisible(MVC_CMN_L_MSE, false);
						setColumnVisible(MVC_CMN_L_BORDER, false);

						for (int m = 0; m < MAX_MEASUREMENT_IN_POINT; m ++)
						{
							setColumnVisible(m + MVC_CMN_L_VALUE_0, true);
						}

						setColumnVisible(MVC_CMN_L_IN_ERROR, false);
						setColumnVisible(MVC_CMN_L_IN_ERROR_LIMIT, false);
						setColumnVisible(MVC_CMN_L_OUT_ERROR, false);
						setColumnVisible(MVC_CMN_L_OUT_ERROR_LIMIT, false);

						break;

					default:
						assert(0);
						break;
				}
			}
			break;

		case MEASURE_TYPE_COMPARATOR:
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
