/****************************************************************************
*
*  Copyright (C) 2002-2012 Helta Kft. / NociSoft Software Solutions
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
#ifndef NCREPORTCHARTITEM_H
#define NCREPORTCHARTITEM_H

#ifdef HIGHCHARTS_INTEGRATION

#include "ncreportitem.h"

class NCReportDataSource;
class NCReportDirector;
class NCReportHighChartsManager;

class NCReportChartItemData : public NCReportItemData
{
public:
    NCReportChartItemData()
    {}
};

class NCReportChartItem : public NCReportItem
{
public:
    NCReportChartItem( NCReportDef* rdef, QGraphicsItem * parent=0 );
    ~NCReportChartItem();

    bool read( NCReportXMLReader* );
    bool write( NCReportXMLWriter* );
    void setDefaultForEditor();
    void paint( NCReportOutput*, const QPointF& );
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void dummyPaint( NCReportOutput* output, const QPointF& point);
    bool isEmpty() const;
    void updateValue(NCReportOutput*, NCReportEvaluator *);
    void updateForEditor();

    QString templateFile() const;
    void setTemplateFile( const QString& fileName );
    QString resultFile() const;
    void setResultFile( const QString& fileName );
    bool updateChart(const QString &chartScript, NCReportHighChartsManager* hcm);

    QRectF boundingRect() const;
    NCReportHighChartsManager *highChartsManager();

    QPixmap pixmap() const;

private:
    NCReportHighChartsManager* manager;


    inline NCReportChartItemData* data()
    { return (NCReportChartItemData*)d; }


    void adjustSize(NCReportOutput* o);
    void adjustHeight(NCReportOutput* o);

    QRectF outputRect(NCReportOutput* o) const;
    void render( QPainter* painter, const QRectF& rect, bool directMode=true );
};

#endif // HIGHCHARTS_INTEGRATION
#endif // NCREPORTCHARTITEM_H
