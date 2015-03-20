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
#ifndef NCREPORTSOURCE_H
#define NCREPORTSOURCE_H

#include <QString>
#include <QPrinter>
#include <QHash>
#include "ncreport_global.h"

QT_BEGIN_NAMESPACE
class QIODevice;
QT_END_NAMESPACE
class NCReportDef;
class NCReportXMLDefReader;
class NCReportXMLDefWriter;

/*!
Report XML definition resource handler. Manages the definition source from the different locations or storage units.\n
Report definitions are ready to fetch from database, file or via internet (http/ftp)
 */
class NCREPORTSHARED_EXPORT NCReportSource
{
public:
    NCReportSource();
    virtual ~NCReportSource();

    enum ReportSourceType { File=0, Database, Http, Ftp, Custom, String };	// report definition source

    void setReportDef( NCReportDef* );
    NCReportDef* reportDef();

    void setReportDefXML( const QString& );
    QString reportDefXML() const;

    void setSourceType( NCReportSource::ReportSourceType );
    ReportSourceType sourceType() const;

    /*
    void setLoadQuery( const QString& );
    QString loadQuery() const;

    void setSaveQuery( const QString& );
    QString saveQuery() const;
    */

    void setConnectionID( const QString& );
    QString connectionID() const;

    void setFileName( const QString& );
    QString fileName() const;

    void setIntegerReportID( int );	// set pk value in report table -if we use report def. file from Database
    int integerReportID() const;

    void setStringReportID( const QString& );	// set string id value in report table -if we use report def. file from Database
    QString stringReportID() const;

    void setColumnName( const QString& column ) { m_columnName = column; }
    QString columnName() const { return m_columnName; }

    void setTableName( const QString& table ) { m_tableName = table; }
    QString tableName() const { return m_tableName; }

    void setKeyColumnName( const QString& column ) { m_keyColumn = column; }
    QString keyColumnName() const { return m_keyColumn; }

    void setKeyValue( const QString& value ) { m_keyValue = value; }
    QString keyValue() const { return m_keyValue; }

    void addParameter( const QString& id, const QString& value );

public:
    virtual bool load( NCReportXMLDefReader& reader);
    virtual bool save( NCReportXMLDefWriter& writer);

protected:
    virtual bool loadFromFile( NCReportXMLDefReader& );
    virtual bool loadFromDatabase( NCReportXMLDefReader&);
    virtual bool loadFromHttp( NCReportXMLDefReader& );
    virtual bool loadFromFtp( NCReportXMLDefReader& );
    virtual bool loadFromCustomSource( NCReportXMLDefReader& );
    virtual bool loadFromString( NCReportXMLDefReader& );

    virtual bool saveToFile( NCReportXMLDefWriter& );
    virtual bool saveToDatabase( NCReportXMLDefWriter&);
    virtual bool saveToHttp( NCReportXMLDefWriter& );
    virtual bool saveToFtp( NCReportXMLDefWriter& );
    virtual bool saveToCustomSource( NCReportXMLDefWriter& );
    virtual bool saveToString( NCReportXMLDefReader& );

    virtual QString loadQuery() const;
    virtual QString saveQuery() const;
    /*!
      * Evaluates the report definition XML. Replaces external parameters in {{parameterID}} formula with their values.
      */
    virtual void evaluateParameters();

    void setParseError( NCReportXMLDefReader& );

private:
    ReportSourceType m_type;
    NCReportDef *rdef;
    int m_id;

private:
    QString m_description;
    QString m_fileName;
    QString m_reportDefXML;	// the xml definition;
    QString m_idS;
    QString m_connectionID;
    QString m_columnName;
    QString m_tableName;
    QString m_keyColumn;
    QString m_keyValue;

    QHash<QString,QString> m_parameters;
};


#endif
