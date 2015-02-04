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
#include "testdatasource.h"
#include "ncreportdef.h"

#include <QVariant>

TestDataSource::TestDataSource(QObject * parent) : NCReportDataSource( parent )
{
    setDataSourceType(Custom);
    setLocationType(Static);
    recno() =0;
}

TestDataSource::~ TestDataSource()
{
}

bool TestDataSource::open()
{
    if ( list.isEmpty() ) {
        error()->setError( tr("No data in TestDataSource datasource") );
        return false;
    }
    recno() =0;
    setOpened(true);
    return true;
}

bool TestDataSource::close()
{
    recno() =0;
    setOpened(false);
    return true;
}

bool TestDataSource::next()
{
    recno()++;

    if ( recno() >= list.count() ) {
        recno()--;
        flagEnd() = true;
        return false;
    }
    flagBegin() = false;
    return true;
}

int TestDataSource::size() const
{
    return list.count();
}

bool TestDataSource::previous()
{
    if ( recno() <= 0 ) {
        recno() = 0;
        flagBegin() = true;
        return false;
    } else {
        recno()--;
    }
    return true;
}

bool TestDataSource::first()
{
    recno()=0;
    return true;
}

bool TestDataSource::last()
{
    recno() = list.count()-1;
    return true;
}

QVariant TestDataSource::value(const QString & column, bool* ok, int i ) const
{
    int col = columnIndexByName(column);
    if (col<0) {
        *ok = false;
        return QVariant();
    }
    return value( col, ok, i );
}

QVariant TestDataSource::value( int column, bool*, int ) const
{
    QVariant v;
    switch (column) {
        case 0: v = list.at(recno()).id; break;
        case 1: v = list.at(recno()).name; break;
        case 2: v = list.at(recno()).address; break;
        case 3: v = list.at(recno()).valid; break;
        case 4: v = list.at(recno()).date; break;
    }
    return v;
}

bool TestDataSource::isValid() const
{
    return true;
}

bool TestDataSource::read(NCReportXMLReader *)
{
    return true;
}

bool TestDataSource::write(NCReportXMLWriter *)
{
    return true;
}

void TestDataSource::evaluate(NCReportEvaluator *)
{
}

void TestDataSource::addData(const TestData & data)
{
    list.append( data );
}

bool TestDataSource::hasNext()
{
    return (recno()<list.count()-1);
}

bool TestDataSource::seek( int index )
{
    bool ok=true;
    if ( index == -1 )
        recno() =0;
    else if ( index < list.count())
        recno() = index;
    else
    {
        recno() = list.count()-1;
        ok=false;
    }

    return ok;
}

int TestDataSource::columnIndexByName(const QString &columnname) const
{
    for (int i=0; i<columnCount(); i++) {
        if (columnname==columnName(i))
            return i;
    }

    return -1;
}

int TestDataSource::columnCount() const
{
    return 5;
}

QString TestDataSource::columnName(int column) const
{
    switch(column) {
    case 0: return "id";
    case 1: return "name";
    case 2: return "address";
    case 3: return "valid";
    case 4: return "date";
    }

    return QString();
}


