#ifndef MEASUREVIEWHEADER_H
#define MEASUREVIEWHEADER_H

#include <QColor>
#include "Measure.h"

// ==============================================================================================

#define                 MVC_CMN_HIDE                        false
#define                 MVC_CMN_SHOW                        true

#define                 MVC_CMN_NO_BOLD                     false
#define                 MVC_CMN_BOLD                        true

#define                 MVC_CMN_COLOR_LIGHT_BLUE            QColor(0xE0, 0xFF, 0xE0)
#define                 MVC_CMN_COLOR_WHITE                 Qt::white

#define                 MVC_CMN_DISABLE_DUPLICATE           false
#define                 MVC_CMN_ENABLE_DUPLICATE            true

// ==============================================================================================

class MeasureViewColumn
{
public:

                        MeasureViewColumn();
                        MeasureViewColumn(const MeasureViewColumn& from);
                        MeasureViewColumn(const QString& title, const int width, const bool visible, const int alignment, const bool bold, const QColor& color, const bool duplicate);
                        ~MeasureViewColumn();

private:

    int                 m_index = -1;

    QString             m_title;
    int                 m_width = 100;
    bool                m_enableVisible = MVC_CMN_SHOW;

    int                 m_alignment = Qt::AlignLeft;
    bool                m_boldFont = MVC_CMN_NO_BOLD;
    QColor              m_color = MVC_CMN_COLOR_WHITE;

    bool                m_enableDuplicate = MVC_CMN_ENABLE_DUPLICATE;

public:

    int                 index() const { return m_index; }
    void                setIndex(const int index) { m_index = index; }

    QString             title() const { return m_title; }

    int                 width() const { return m_width; }
    void                setWidth(const int width) { m_width = width; }


    bool                enableVisible() const { return m_enableVisible; }
    void                setVisible(const bool enable) { m_enableVisible = enable; }

    int                 alignment() const { return m_alignment; }

    bool                boldFont() const { return m_boldFont; }
    void                setBoldFont(const bool bold) { m_boldFont = bold; }

    QColor              color() const { return m_color; }
    void                setColor(const QColor& color) { m_color = color; }

    bool                isEnableDuplicate() const { return m_enableDuplicate; }

    MeasureViewColumn&  operator=(const MeasureViewColumn& from);
};

// ==============================================================================================

const int               MEASURE_VIEW_COLUMN_COUNT   = 64;

// ==============================================================================================

const int               MEASURE_VIEW_COLUMN_INDEX	= 0,
                        MEASURE_VIEW_COLUMN_CASE	= 1,
                        MEASURE_VIEW_COLUMN_ID		= 2;

// ==============================================================================================

class MeasureViewHeader : public QObject
{
    Q_OBJECT

public:

    explicit            MeasureViewHeader(QObject *parent = 0);
                        ~MeasureViewHeader();

    void                setMeasureType(const int measureType);

    void                init(const int type);

    int                 count() const;
    MeasureViewColumn*  column(const int index) const;

    void                updateColumnState();

    void                setColumnVisible(const int column, const bool visible);

private:

    static MeasureViewColumn m_column[MEASURE_TYPE_COUNT][MEASURE_VIEW_COLUMN_COUNT];

    int                 m_measureType = MEASURE_TYPE_UNKNOWN;

    int                 m_columnCount[MEASURE_TYPE_COUNT];
};

// ==============================================================================================

const int               MVC_CMN_L_INDEX             = 0;
const int               MVC_CMN_L_CASE              = 1;
const int               MVC_CMN_L_ID                = 2;
const int               MVC_CMN_L_NAME              = 3;
const int               MVC_CMN_L_SUBBLOCK          = 4;
const int               MVC_CMN_L_BLOCK             = 5;
const int               MVC_CMN_L_ENTRY             = 6;
const int               MVC_CMN_L_EL_RANGE          = 7;
const int               MVC_CMN_L_PH_RANGE          = 8;
const int               MVC_CMN_L_OUT_RANGE         = 9;
const int               MVC_CMN_L_EL_NOMINAL        = 10;
const int               MVC_CMN_L_PH_NOMINAL        = 11;
const int               MVC_CMN_L_OUT_NOMINAL       = 12;
const int               MVC_CMN_L_PERCENT           = 13;
const int               MVC_CMN_L_EL_MEASURE        = 14;
const int               MVC_CMN_L_PH_MEASURE        = 15;
const int               MVC_CMN_L_OUT_MEASURE       = 16;
const int               MVC_CMN_L_SYSTEM_ERROR      = 17;
const int               MVC_CMN_L_MSE               = 18;
const int               MVC_CMN_L_LOW_BORDER        = 19;
const int               MVC_CMN_L_HIGH_BORDER       = 20;
const int               MVC_CMN_L_VALUE_COUNT       = 21;
const int               MVC_CMN_L_VALUE_0           = 22;
const int               MVC_CMN_L_VALUE_1           = 23;
const int               MVC_CMN_L_VALUE_2           = 24;
const int               MVC_CMN_L_VALUE_3           = 25;
const int               MVC_CMN_L_VALUE_4           = 26;
const int               MVC_CMN_L_VALUE_5           = 27;
const int               MVC_CMN_L_VALUE_6           = 28;
const int               MVC_CMN_L_VALUE_7           = 29;
const int               MVC_CMN_L_VALUE_8           = 30;
const int               MVC_CMN_L_VALUE_9           = 31;
const int               MVC_CMN_L_VALUE_10          = 32;
const int               MVC_CMN_L_VALUE_11          = 33;
const int               MVC_CMN_L_VALUE_12          = 34;
const int               MVC_CMN_L_VALUE_13          = 35;
const int               MVC_CMN_L_VALUE_14          = 36;
const int               MVC_CMN_L_VALUE_15          = 37;
const int               MVC_CMN_L_VALUE_16          = 38;
const int               MVC_CMN_L_VALUE_17          = 39;
const int               MVC_CMN_L_VALUE_18          = 40;
const int               MVC_CMN_L_VALUE_19          = 41;
const int               MVC_CMN_L_ERROR             = 42;
const int               MVC_CMN_L_OUT_ERROR         = 43;
const int               MVC_CMN_L_LIMIT_ERROR       = 44;
const int               MVC_CMN_L_MEASUREMENT_TIME  = 45;

// ==============================================================================================

#endif // MEASUREVIEWHEADER_H
