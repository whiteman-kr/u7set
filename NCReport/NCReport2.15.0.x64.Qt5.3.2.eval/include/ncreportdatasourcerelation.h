/****************************************************************************
*
*  Copyright (C) 2002-2014 Helta Ltd. - NociSoft Software Solutions
*  All rights reserved.
*  Author: Norbert Szabo
*  E-mail: office@nocisoft.com
*  Web: www.nocisoft.com
*
*  This file is part of the NCReport reporting software
*  Created: 2014.07.25. (nocisoft)
*
*  Licensees holding a valid NCReport License Agreement may use this
*  file in accordance with the rights, responsibilities, and obligations
*  contained therein. Please consult your licensing agreement or contact
*  office@nocisoft.com if any conditions of this licensing are not clear
*  to you.
*
*  This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
*  WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*
****************************************************************************/

#ifndef NCREPORTDATASOURCERELATION_H
#define NCREPORTDATASOURCERELATION_H

#include <QList>
#include <QString>
#include <QHash>

class NCReportDataSource;
class NCReportEvaluator;
class NCReportDirector;

class NCReportDataSourceRelation
{
public:
    //explicit NCReportDataSourceRelation(NCReportDataSourceRelation *parent = 0);
    explicit NCReportDataSourceRelation(NCReportDirector* director, NCReportDataSource *dataSource, NCReportDataSourceRelation *parent = 0);
    ~NCReportDataSourceRelation();

    bool insertDataSources( const QHash<QString, NCReportDataSource*>& dataSources );
    bool insertDataSource( NCReportDataSource* dataSource );
    void appendChild(NCReportDataSourceRelation *child);
    NCReportDataSourceRelation* find(NCReportDataSource* ds );

    NCReportDataSourceRelation *child(int row);
    int childCount() const;
    int row() const;
    NCReportDataSourceRelation *parent();

    //void setDataSource( NCReportDataSource* dataSource );
    NCReportDataSource *dataSource();

    bool open();
    bool nextRecord();
    bool nextRecord( NCReportDataSource* ds);
    bool previousRecord();
    bool previousRecord(NCReportDataSource* ds);
    bool firstRecord();
    bool firstRecord(NCReportDataSource* ds);
    bool lastRecord();
    bool lastRecord(NCReportDataSource* ds);

//    bool previousRecord(NCReportDirector* director, NCReportDataSource* ds = 0);
//    bool firstRecord(NCReportDirector* director, NCReportDataSource* ds = 0);
//    bool lastRecord(NCReportDirector* director, NCReportDataSource* ds = 0);

private:
    NCReportDataSourceRelation *m_parentItem;
    NCReportDataSource *m_dataSource;
    NCReportDirector* m_director;
    int m_currentChildIndex;
    NCReportDataSourceRelation* m_lastFoundItem;

    QList<NCReportDataSourceRelation*> m_childItems;

private:
    enum RecordAction { Next=0, Previous, First, Last };

    bool recordPositionChange(RecordAction action);
    bool recordPositionChange(RecordAction action, NCReportDataSource* ds);
    bool updateDataSource(RecordAction action, NCReportDataSourceRelation* rel);
};

#endif // NCREPORTDATASOURCERELATION_H
