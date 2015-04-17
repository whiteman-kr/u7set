/****************************************************************************
*
*  Copyright (C) 2002-2011 Helta Kft. / NociSoft Software Solutions
*  All rights reserved.
*  Author: Norbert Szabo
*  E-mail: nszabo@helta.hu, info@nocisoft.com
*  Web: www.nocisoft.com
*
*  This file is part of the NCReport reporting software
*
*  Licensees holding a valid NCReport License Agreement may use this
*  file in accordance with the rights, responsibilities, and obligations
*  contained therein. Please consult your licensing agreement or contact
*  nszabo@helta.hu if any conditions of this licensing are not clear
*  to you.
*
*  This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
*  WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*
****************************************************************************/
#ifndef NCREPORTCROSSTABITEM_H
#define NCREPORTCROSSTABITEM_H

#include "ncreportitem.h"
#include "ncreportcrosstabitemcell.h"

#define NCREPORTCROSSTABITEMCELL_COUNT	9
#define NCREPORTCROSSTABREGION_COUNT	6
#define NCREPORTCROSSTAB_DEFAULT_COLWIDTH	24.0
#define NCREPORTCROSSTAB_DEFAULT_ROWHEIGHT	7.0

class NCReportDataSource;
class NCReportDirector;
class NCReportItemModelDataSource;
class NCReportPivotTableModel;

class NCReportCrossTabItemData : public NCReportItemData
{
public:
    NCReportCrossTabItemData() : tableSpacingMM(0), cellPaddingMM(0), cellSpacingMM(0),
        leftMarginMM(0), rightMarginMM(0), topMarginMM(0), bottomMarginMM(0),
        colWidthMM(NCREPORTCROSSTAB_DEFAULT_COLWIDTH),rowHeightMM(NCREPORTCROSSTAB_DEFAULT_ROWHEIGHT),
        showColHeader(true),showRowHeader(true),
        showBottomSummary(true),showSideSummary(true),breakTable(true),isPivotTable(false),ptAutoSort(false),ptAggregate(1),ptDataSource(0),ptModel(0),
        dataSource(0),metaDataSource(0),director(0)
    {}

    qreal tableSpacingMM;
    qreal cellPaddingMM;
    qreal cellSpacingMM;
    qreal leftMarginMM;
    qreal rightMarginMM;
    qreal topMarginMM;
    qreal bottomMarginMM;
    /*! Default widths of columns */
    qreal colWidthMM;
    /*! Default heights of rows */
    qreal rowHeightMM;
    bool showColHeader;
    bool showRowHeader;
    bool showBottomSummary;
    bool showSideSummary;
    bool breakTable;
    /*! Create pivot table */
    bool isPivotTable;
    bool ptAutoSort;
    short ptAggregate;
    NCReportItemModelDataSource *ptDataSource;
    NCReportPivotTableModel *ptModel;

    /*! Table data source */
    NCReportDataSource *dataSource;
    /*! Data source of meta data such as columnnames, type, format, etc.*/
    NCReportDataSource *metaDataSource;
    NCReportDirector *director;
    //QString cornerHeaderText, sideSummaryHeaderText, bottomSummaryHeaderText;
    /*! Data source column ID of (dynamic) column title*/
    QString columnTitleSource;
    /*! Row name/reference of pivot table*/
    QString ptRowSource;
    /*! Column name/reference of pivot table*/
    QString ptColumnSource;
    /*! Value name/reference of pivot table*/
    QString ptValueSource;
};

class NCReportCrossTabIndex
{
public:
    NCReportCrossTabIndex() : m_row(-1), m_column(-1) {}
    NCReportCrossTabIndex(int row, int column) : m_row(row), m_column(column) {}

    void setRow( int row ) { m_row = row; }
    void setColumn( int column ) { m_column = column; }
    int row() const { return m_row; }
    int column() const { return m_column; }
    int& rowRef() { return m_row; }
    int& columnRef() { return m_column; }
    void reset() {
        m_row=-1;
        m_column=-1;
    }

private:
    int m_row;
    int m_column;
};

class NCReportCrossTabItem : public NCReportItem
{
public:
    NCReportCrossTabItem( NCReportDef* rdef, QGraphicsItem * parent=0 );
    ~NCReportCrossTabItem();

    enum TableRegion { HeaderColumn=0, DataColumn, TotalColumn, HeaderRow, DataRow, TotalRow };
    //enum TableRows { HeaderRow=0, DataRow, TotalRow };
    //enum TableColumns { HeaderColumn=0, DataColumn, TotalColumn };
    enum CellChangeDirection { None=0, Down, Right, NewColumn, NewRow };

    struct PrintedRange {
        int from;
        int to;
    };

    bool read( NCReportXMLReader* );
    bool write( NCReportXMLWriter* );
    void setDefaultForEditor();
    void paint( NCReportOutput*, const QPointF& );
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void dummyPaint( NCReportOutput* output, const QPointF& point);

    void setDataSource( NCReportDataSource* );
    void setDataSourceByID( const QString& id );
    NCReportDataSource* dataSource() const;
    NCReportDataSource* metaDataSource() const;
    QRectF boundingRect() const;
    //NCReportCrossTabItemCell& cell(const NCReportCrossTabItemCell::CellType type);
    NCReportCrossTabItemCell& cell( int cellType );
    void setCell( const NCReportCrossTabItemCell& cell, int cellType );

    QString hiddenDataSourceColumns() const;
    void setHiddenDataSourceColumns( const QString& columnlist );

    qreal tableSpacingMM() const { return m_d->tableSpacingMM; }
    void setTableSpacingMM( qreal value ) { m_d->tableSpacingMM=value; }
    qreal cellPaddingMM() const { return m_d->cellPaddingMM; }
    void setCellPaddingMM( qreal value ) { m_d->cellPaddingMM=value; }
    qreal cellSpacingMM() const { return m_d->cellSpacingMM; }
    void setCellSpacingMM( qreal value ) { m_d->cellSpacingMM=value; }
    qreal leftMarginMM() const { return m_d->leftMarginMM; }
    void setLeftMarginMM( qreal value ) { m_d->leftMarginMM=value; }
    qreal rightMarginMM() const { return m_d->rightMarginMM; }
    void setRightMarginMM( qreal value ) { m_d->rightMarginMM=value; }
    qreal topMarginMM() const { return m_d->topMarginMM; }
    void setTopMarginMM( qreal value ) { m_d->topMarginMM=value; }
    qreal bottomMarginMM() const { return m_d->bottomMarginMM; }
    void setBottomMarginMM( qreal value ) { m_d->bottomMarginMM=value; }
    qreal columnWidthMM() const { return m_d->colWidthMM; }
    void setColumnWidthMM( qreal value ) { m_d->colWidthMM=value; }
    qreal rowHeightMM() const { return m_d->rowHeightMM; }
    void setRowHeightMM( qreal value ) { m_d->rowHeightMM=value; }

    //bool autoWidth() const { return m_d->autoWidth; }
    //void setAutoWidth( bool value ) { m_d->autoWidth=value; }
    bool showColumnHeader() const { return m_d->showColHeader; }
    void setShowColumnHeader( bool value ) { m_d->showColHeader=value; }
    bool showRowHeader() const { return m_d->showRowHeader; }
    void setShowRowHeader( bool value ) { m_d->showRowHeader=value; }
    bool showBottomSummary() const { return m_d->showBottomSummary; }
    void setShowBottomSummary( bool value ) { m_d->showBottomSummary=value; }
    bool showSideSummary() const { return m_d->showSideSummary; }
    void setShowSideSummary( bool value ) { m_d->showSideSummary=value; }
    bool breakTable() const { return m_d->breakTable; }
    void setBreakTable( bool value ) { m_d->breakTable=value; }
    QString columnTitleSource() const { return m_d->columnTitleSource; }
    void setColumnTitleSource( const QString& value ) { m_d->columnTitleSource=value; }

    bool isPivotTable() const { return m_d->isPivotTable; }
    void setPivotTable(bool set) { m_d->isPivotTable = set; }
    bool isPivotAutoSort() const { return m_d->ptAutoSort; }
    void setPivotAutoSort(bool set) { m_d->ptAutoSort = set; }
    short pivotTableAggregation() const { return m_d->ptAggregate; }
    void setPivotTableAggregation(short value) { m_d->ptAggregate = value; }
    QString pivotTableRowSource() const { return m_d->ptRowSource;  }
    void setPivotTableRowSource( const QString& value ) { m_d->ptRowSource = value; }
    QString pivotTableColumnSource() const { return m_d->ptColumnSource;  }
    void setPivotTableColumnSource( const QString& value ) { m_d->ptColumnSource = value; }
    QString pivotTableValueSource() const { return m_d->ptValueSource; }
    void setPivotTableValueSource( const QString& value ) { m_d->ptValueSource = value; }

    qreal regionSize( int region ) const;
    void setRegionSize( qreal size, int region );
    bool isHiddenDataColumn( int column ) const;

protected:
    //virtual void nextColumn();

    void setFlagLastRecord( bool value ) { m_flagLastRecord = value; }
    void setFlagNoSpaceRight( bool value ) { m_flagNoSpaceRight = value; }
    void setFlagNoSpaceBottom( bool value ) { m_flagNoSpaceBottom = value; }
    void setFlagRowFinished( bool value ) { m_flagRowFinished = value; }

    bool isFlagLastRecord() const { return m_flagLastRecord; }
    bool isFlagNoSpaceRight() const { return m_flagNoSpaceRight; }
    bool isFlagNoSpaceBottom() const { return m_flagNoSpaceBottom; }
    bool isFlagRowFinished() const { return m_flagRowFinished; }

    void prepareDataSource( int& recordPosition, const NCReportCrossTabIndex& index );

private:
    NCReportCrossTabItemData* m_d;

    NCReportCrossTabItemCell m_cells[NCREPORTCROSSTABITEMCELL_COUNT];
    qreal m_regionSizes[NCREPORTCROSSTABREGION_COUNT];
    QList<double> totals_bottom;
    QList<double> totals_side;
    QStringList m_reservedDSColumns;
    NCReportCrossTabIndex m_printedDataCellIndex;
    bool m_flagNoSpaceRight;
    bool m_flagNoSpaceBottom;
    bool m_flagLastRecord;
    bool m_flagRowFinished;
    QRectF m_tableRect;
    bool m_tableRectSet;
    qreal m_calculatedTableHeight;
    qreal m_tableSpacing;
    qreal m_cellSpacing;

    void adjustSize(NCReportOutput* o);
    void adjustHeight(NCReportOutput* o);

    QRectF outputRect(NCReportOutput* o) const;
    bool readCell( NCReportXMLReader* );
    bool writeCell( NCReportXMLWriter* );
    bool readRegionSizes(NCReportXMLReader *r);
    bool writeRegionSizes(NCReportXMLWriter *w);
    void paintScheme( QPainter* );

    void paintTableHeaderRow( NCReportOutput* o, QRectF& cellRect, const QRectF& itemRect, const NCReportCrossTabIndex& index );
    void paintTableDataRow( NCReportOutput* o, QRectF& cellRect, const QRectF& itemRect, const NCReportCrossTabIndex& index );
    void paintTableFooterRow( NCReportOutput* o, QRectF& cellRect, const QRectF& itemRect, const NCReportCrossTabIndex& index );

    bool paintCell( NCReportOutput* o, const NCReportCrossTabItemCell::CellType, const QVariant& value, QRectF& cellRect, const NCReportCrossTabIndex& index);
    /*! Moves paint position to the next cell */
    void nextCell( NCReportOutput* o, QRectF& cellRect, const QRectF& itemRect, const CellChangeDirection ccd);

    qreal spaceToBottom( NCReportOutput* o, const QRectF& rect) const;
    qreal spaceToRight( NCReportOutput* o, const QRectF& rect) const;
    void setCellSize( NCReportOutput* o, const TableRegion horizontalRegion, const TableRegion verticalRegion, QRectF& cellRect );
    int dataSourceRowCount() const;
    int dataSourceColumnCount() const;
    bool createPivotTable(NCReportDataSource *sourceDS);
    bool deletePivotTable();
};

#endif // NCREPORTCROSSTABITEM_H
