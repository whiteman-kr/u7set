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
#ifndef NCREPORTITEMMODELDATASOURCE_H
#define NCREPORTITEMMODELDATASOURCE_H

#include "ncreportdatasource.h"

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
QT_END_NAMESPACE
/*!
Datasource class for QAbstractItemModel based data
 */
class NCReportItemModelDataSource : public NCReportDataSource
{
    Q_OBJECT
public:
    NCReportItemModelDataSource( QObject *parent=0 );
    ~NCReportItemModelDataSource();

    bool open();
    bool close();
    bool first();
    bool last();
    bool next();
    bool previous();
    int size() const;
    int rowCount() const;
    bool seek( int index );
    bool update();

    QVariant value( const QString&, bool* ok=0, int nQTDataRole = -1) const;
    QVariant value( int, bool* ok=0, int nQTDataRole = -1) const;

    bool read( NCReportXMLReader* r);
    bool write( NCReportXMLWriter* w);
    void evaluate( NCReportEvaluator* ev);

    void setItemModel( QAbstractItemModel* );
    QAbstractItemModel* itemModel() const;
    void setModelID( const QString& );
    QString modelID() const;
    bool hasNext();
    bool isValid() const;
    QString columnName(int column) const;
    QString columnTitle(int column) const;
    QString columnToolTip(int column) const;
    QString rowTitle(int column) const;
    int columnCount() const;
    int columnIndexByName(const QString& columnname) const;

protected:
    virtual bool checkModel();
private:
    QAbstractItemModel *model;
    QString modelid;
    QString relationData;
};



#endif
