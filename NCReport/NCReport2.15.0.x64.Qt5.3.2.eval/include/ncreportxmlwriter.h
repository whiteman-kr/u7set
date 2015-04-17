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
#ifndef NCREPORTXMLWRITER_H
#define NCREPORTXMLWRITER_H

#include "ncreport_global.h"

#include <QXmlStreamWriter>
#include <QFont>
#include <QPen>
#include <QBrush>
#include <QPointF>
#include <QSizeF>

class NCReportSection;
class NCReportDataSource;
class NCReportVariable;

/*!
This class represents the base report XML definition stream writer.\n
Responsible for generating report XML definition
 */
class NCREPORTSHARED_EXPORT NCReportXMLWriter : public QXmlStreamWriter
{
public:
    NCReportXMLWriter();
    NCReportXMLWriter( QIODevice *device );
    NCReportXMLWriter( QString * string );
    virtual ~NCReportXMLWriter();
    bool write(QIODevice *device);


    void writeFont( const QFont& );
    void writePen( const QPen& );
    void writeBrush( const QBrush& );

    void writePos(const QPointF& );
    void writePosFrom( const QPointF& );
    void writePosTo(const QPointF& );
    void writeSize( const QSizeF& );
    void writeAlignment( const Qt::Alignment );
    //void writeAlignmentH( const Qt::Alignment );
    //void writeAlignmentV( const Qt::Alignment );
    void writeWidth( const qreal );
    void writeHeight( const qreal );
    void writeLineWidth( const qreal );
    void writeBool( const QString& property, bool );
    void writeReal( const QString& property, qreal );
    void writeInt( const QString& property, int );
    void writeParameters( const QHash<QString, QVariant>& parameters );

    virtual QString reportName() const;
    virtual QString reportType() const;

protected:
    virtual void writeReport();

    virtual void writeItems( NCReportSection* );
    virtual void writeSection( NCReportSection* );

    virtual void writeOptions();
    virtual void writeDataSources();
    virtual void writeVariables();
    virtual void writeScripts();
    virtual void writePageHeader();
    virtual void writePageFooter();
    virtual void writeReportHeader();
    virtual void writeReportFooter();
    virtual void writeDetails();
    //virtual void writeField( NCReportFieldItem* );

    virtual void writeDataSource(  NCReportDataSource* );
    virtual void writeVariable( NCReportVariable* );
    virtual void writeParameter(const QString &tagName, const QString& id, const QVariant& value );
    virtual void writeScript(const QString& id, const QString& script );

    QString qrealToString( const qreal ) const;

private:
    //void writeItem(QTreeWidgetItem *item);

    QString basetag;

};

#endif
