/****************************************************************************
*
*  Copyright (C) 2002-2012 Helta Kft. / NociSoft Software Solutions
*  All rights reserved.
*  Author: Norbert Szabo
*  E-mail: norbert@nocisoft.com, office@nocisoft.com
*  Web: www.nocisoft.com
*
*  This file is part of the NCReport Report Generator System
*
*  Licensees holding a valid NCReport License Agreement may use this
*  file in accordance with the rights, responsibilities, and obligations
*  contained therein. Please consult your licensing agreement or contact
*  norbert@nocisoft.com if any conditions of this licensing are not clear
*  to you.
*
*  This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
*  WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*
****************************************************************************/
#ifndef NCREPORTXMLDATASOURCE_H
#define NCREPORTXMLDATASOURCE_H

#include "ncreportdatasource.h"

#include <QHash>

class NCReportXMLDataSource : public NCReportDataSource
{
	Q_OBJECT
public:
	NCReportXMLDataSource(QObject *parent = 0);
	~NCReportXMLDataSource();

	bool open();
	bool close();
	bool first();
	bool last();
	bool next();
	bool previous();
	int size() const;
	QVariant value( const QString&, bool* ok=0, int nQTDataRole = -1 ) const;
	QVariant value( int, bool* ok=0, int nQTDataRole = -1 ) const;
	bool read( NCReportXMLReader* );
	bool write( NCReportXMLWriter* );
    void evaluate( NCReportEvaluator* evaluator);
	bool seek( int index );

	int rowCount() const;
	int columnCount() const;
	bool hasNext();

	QStringList keywordList();
	QStringList columnNames();
	QString columnName(int column) const;
	bool isValid() const;
	int columnIndexByName(const QString& columnname) const;
	
private:
	enum ReadState { Off=0, ParametersTag, ParameterTag, DataSourcesTag, DataSourceTag, ColumnsTag, ColumnTag, RowTag };

	ReadState m_state;
//	QFile *xmlFile;
//	QXmlStreamReader *reader;

	//QHash<QString,QVariant> m_parameters;
	QList<QString> m_columns;
	QList<QStringList> m_rows;
	QStringList m_currentRow;
};

#endif // NCREPORTXMLDATASOURCE_H
