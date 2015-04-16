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
#ifndef NCREPORTSQLDATASOURCE_H
#define NCREPORTSQLDATASOURCE_H

#include "ncreportdatasource.h"
#include <QSqlDatabase>
#include <QSqlRecord>

QT_BEGIN_NAMESPACE
class QSqlQuery;
QT_END_NAMESPACE

/*!
Datasource for SQL queries. Uses Qt's SQL module.\n
Inherits: NCReportDataSource
*/
class NCReportSQLDataSource : public NCReportDataSource
{
    Q_OBJECT

    friend class NCReportXMLDefReader;
public:
    NCReportSQLDataSource( QObject *parent=0 );
    ~NCReportSQLDataSource();

    bool open();
    bool close();
    bool first();
    bool last();
    bool next();
    bool previous();
    bool seek( int index );
    int size() const;
    bool update();
    bool hasNext();

    QVariant value( const QString&, bool* ok=0, int nQTDataRole = -1) const;
    QVariant value( int, bool* ok=0, int nQTDataRole = -1) const;

    bool read(NCReportXMLReader* reader);
    bool write(NCReportXMLWriter* writer);
    void evaluate(NCReportEvaluator* evaluator);

    void setSqlQuery( const QString& );
    //void setDatabase( const QSqlDatabase & );
    QString sqlQuery() const;

    void setHost( const QString& );
    void setDB( const QString& );
    void setUser( const QString& );
    void setPassword( const QString& );
    void setPort( const QString& );
    void setDriver( const QString& );
    void setConnectOptions( const QString& );
    QString connectOptions() const;

    QString dbHost() const;
    QString dbDatabase() const;
    QString dbUser() const;
    QString dbPassword() const;
    QString dbPort() const;
    QString dbDriver() const;
    bool connectDatabase(QSqlDatabase &database);
    bool disconnectDatabase();

    int rowCount() const;
    int columnCount() const;
    bool queryIsValid( const QString& );
    QStringList keywordList();
    QStringList columnNames();
    QString columnName(int column) const;
    bool isValid() const;
    bool isForwardOnly() const;
    void setForwardOnly(bool);
    QSqlQuery *queryObject();
    bool hasColumn(int column) const;
    bool hasColumn(const QString &columnname) const;
    int columnIndexByName(const QString& columnname) const;

private:
    enum RecordStatus { BeforeFirst, Normal, Buffer };

    QSqlQuery *sql;
    RecordStatus m_recordStatus;
    bool m_forwardOnly;

private:
    QString driver;
    QString host, dbname, user, password, port;
    QString query;
    QString connOpt;
    QSqlRecord m_buffer;

private:

    void saveBuffer();
    QString privateConnectionID() const;
    bool executeQuery();
    //QVariant bufferValue(int) const;
};

#endif
