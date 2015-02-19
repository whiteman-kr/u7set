/****************************************************************************
*
*  Copyright (C) 2002-2013 Helta Kft. / NociSoft Software Solutions
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
#ifndef NCREPORTTABLEITEM_H
#define NCREPORTTABLEITEM_H

#include "ncreportitem.h"

class NCReportDataSource;
class NCReportDirector;

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
class QTableView;
QT_END_NAMESPACE

class NCReportTableItemData : public NCReportItemData
{
public:
    NCReportTableItemData() : tableSpacingMM(0), cellPaddingMM(0), cellSpacing(0),
        leftMarginMM(0), rightMarginMM(0), topMarginMM(0), bottomMarginMM(0),
        dataSource(0), director(0), model(0), view(0), horizontalHeader(true), verticalHeader(true), elidedMode(false), showBorderAndBG(true)
    {}

    qreal tableSpacingMM;
    qreal cellPaddingMM;
    qreal cellSpacing;
    qreal leftMarginMM;
    qreal rightMarginMM;
    qreal topMarginMM;
    qreal bottomMarginMM;

    NCReportDataSource *dataSource;
    NCReportDirector *director;
    QAbstractItemModel *model;
    QTableView *view;
    bool horizontalHeader;
    bool verticalHeader;
    bool elidedMode;
    bool showBorderAndBG;
    QString modelID;
    QString viewID;
    QList<int> hiddenRows;
    QList<int> hiddenCols;
};

class NCReportTableItem : public NCReportItem
{
public:
    NCReportTableItem( NCReportDef* rdef, QGraphicsItem * parent=0 );
    ~NCReportTableItem();

    bool read( NCReportXMLReader* reader );
    bool write( NCReportXMLWriter* writer);
    void setDefaultForEditor();
    void paint( NCReportOutput* output, const QPointF& painterPosMM );
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void dummyPaint( NCReportOutput* output, const QPointF& point);

    void setDataSource( NCReportDataSource* );
    void setDataSourceByID( const QString& id );
    NCReportDataSource* dataSource() const;
    QRectF boundingRect() const;

    qreal tableSpacingMM() const { return m_d->tableSpacingMM; }
    void setTableSpacingMM( qreal value ) { m_d->tableSpacingMM=value; }
    qreal cellPaddingMM() const { return m_d->cellPaddingMM; }
    void setCellPaddingMM( qreal value ) { m_d->cellPaddingMM=value; }

    qreal leftMarginMM() const { return m_d->leftMarginMM; }
    void setLeftMarginMM( qreal value ) { m_d->leftMarginMM=value; }
    qreal rightMarginMM() const { return m_d->rightMarginMM; }
    void setRightMarginMM( qreal value ) { m_d->rightMarginMM=value; }
    qreal topMarginMM() const { return m_d->topMarginMM; }
    void setTopMarginMM( qreal value ) { m_d->topMarginMM=value; }
    qreal bottomMarginMM() const { return m_d->bottomMarginMM; }
    void setBottomMarginMM( qreal value ) { m_d->bottomMarginMM=value; }

    qreal cellSpacing() const { return m_d->cellSpacing; }
    void setCellSpacing( qreal value ) { m_d->cellSpacing=value; }

    bool isElidedMode() const { return m_d->elidedMode; }
    void setElidedMode( bool set ) { m_d->elidedMode=set; }

    bool isHorizontalHeaderVisible() const { return m_d->horizontalHeader; }
    void setHorizontalHeaderVisible( bool set ) { m_d->horizontalHeader = set; }

    bool isVerticalHeaderVisible() const { return m_d->verticalHeader; }
    void setVerticalHeaderVisible( bool set ) { m_d->verticalHeader = set; }

    bool showBorderAndBG() const { return m_d->showBorderAndBG; }
    void setshowBorderAndBG( bool set ) { m_d->showBorderAndBG = set; }

    void setModelID( const QString& modelID ) { m_d->modelID = modelID; }
    QString modelID() const { return m_d->modelID; }

    void setTableViewID( const QString& viewID ) { m_d->viewID = viewID; }
    QString tableViewID() const { return m_d->viewID; }

    void setHiddenRows( const QList<int>& rows );
    void setHiddenRows( const QString& list );
    QString hiddenRowsList() const;
    void setHiddenColumns( const QList<int>& columns );
    void setHiddenColumns( const QString& list );
    QString hiddenColumnsList() const;

    int columnCount() const;
    int rowCount() const;

//    qreal columnWidthMM( int column ) const;
//    qreal rowHeightMM( int row ) const;
    qreal columnWidthPixel( int column ) const;
    qreal rowHeightPixel( int row ) const;
    int calculateSourceTableWidth() const;

private:
    enum CellType { Corner=0x0, HorizontalHeader, VerticalHeader, Data };
    enum ImageType { None=0x0, Pixmap, Icon };

    NCReportTableItemData* m_d;

    qreal m_rate;
    qreal m_deviceRate;
    qreal m_tableWidth;
    qreal m_calculatedTableHeight;
    qreal m_tableSpacing;
    qreal m_cellSpacing;
    QSizeF m_defaultIconSize;
    QRectF m_itemRect;

private:
    void adjustSize(NCReportOutput* o);
    void adjustHeight(NCReportOutput* o);

    void paintRow( NCReportOutput* output, QPointF &topLeft, int row );
    void paintCell( NCReportOutput* output, const QRectF &cellRect, int row, int col );
    ImageType paintCellImage( NCReportOutput* output, const QRectF &cellRect, const QVariant& imageData ) const;
    void paintIndentationImage( NCReportOutput* output, const QRectF &cellRect, int dimension );
    void paintElitedText( QPainter *painter, const QString& text, const QRectF& cellRect,  Qt::Alignment alignment );

    qreal spaceToBottom( NCReportOutput* output, const QRectF& rect) const;
    void indentation( int role, int& dimension, bool& hasChildren );
};

#endif // NCREPORTTABLEITEM_H
