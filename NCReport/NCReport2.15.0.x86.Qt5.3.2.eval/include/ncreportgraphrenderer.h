/****************************************************************************
*
*  Copyright (C) 2002-2008 Helta Kft. / NociSoft Software Solutions
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
#ifndef NCREPORTGRAPHRENDERER_H
#define NCREPORTGRAPHRENDERER_H

#include <QRectF>
#include <QString>

#include "ncreport_global.h"

QT_BEGIN_NAMESPACE
class QPainter;
QT_END_NAMESPACE

class NCReportOutput;
class NCReportDataSource;
class NCReportGraphItem;

/*!
  * \brief Abstract class for rendering custom contents in NCReport.
  *
  * Ideal for creating graphs, drawings, painting widget contents, etc.
*/

class NCREPORTSHARED_EXPORT NCReportGraphRenderer
{
public:
    NCReportGraphRenderer();
    virtual ~NCReportGraphRenderer();

    void setID( const QString& id );
    void setId( const QString& id );
    void setId( int id );
    const QString& id() const;
    int intId() const;
    /*!
     Abstract method for painting custom item content into the specified report output
     @param painter active QPainter object
     @param output current NCReportOutput
     @param rect Rectangle of the item. Positions and size come from report definition
     @param itemdata Specification string, come from report definition
     */
    virtual void paintItem( QPainter* painter, NCReportOutput* output, const QRectF& rect, const QString& itemdata ) =0;
    virtual void paintEditorItem(QPainter* painter, const QRectF& rect , const QString &text);
    /*!
     Virtual method to calculate heigh of item in millimeters when automatic height is set.
      */
    virtual qreal calculateHeightMM( NCReportOutput* output ) const;
    /*!
     Virtual method to calculate size of item in millimeters when automatic height is set.
      */
    virtual QSizeF calculateSizeMM( NCReportOutput* output, const QSizeF& requested ) const;
    virtual QSizeF calculateSizeMM( NCReportOutput* output, const QSizeF& requested, const QString& itemdata ) const;

    /*!
     Virtual method for accessing a named data source by ID. dataSource() function returns
      */
    virtual QString dataSourceID() const;

    /*!
     Sets the data source by dataSourceID(). The function is used by report engine, users don't need it.
      */
    void setDataSource( NCReportDataSource* ds );
    void setItem( NCReportGraphItem* item );

protected:
    /*!
     returns the data source by dataSourceID()
      */
    NCReportDataSource* dataSource() const;
    NCReportGraphItem *item() const;

private:
    NCReportDataSource *m_dataSource;
    NCReportGraphItem *m_item;
    QString classid;

};

#endif
