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
#ifndef NCREPORTSTRINGLISTDATASOURCE_H
#define NCREPORTSTRINGLISTDATASOURCE_H

#include "ncreportdatasource.h"

#include <QStringList>

/*!
Datasource for QStringList based texts. Each QStringList member represents a record and the data columns are separated by
a delimiter character.
Inherits: NCReportDataSource
 */
class NCReportStringListDataSource : public NCReportDataSource
{
	Q_OBJECT
public:
	NCReportStringListDataSource( QObject *parent=0 );
	~NCReportStringListDataSource();
	
	enum Delimiter { Tab=0, Comma, SemiColon, Space, VBar, Custom };
	
	bool open();
	bool close();
	bool first();
	bool last();
	bool next();
	bool previous();
	bool seek( int index );
	int size() const;
	QVariant value( const QString&, bool* ok=0, int nQTDataRole = -1 ) const;
	QVariant value( int, bool* ok=0,int nQTDataRole = -1 ) const;
	bool read( NCReportXMLReader* );
	bool write( NCReportXMLWriter* );
	
    void setStringList( const QStringList& list);
	
	void setColumnDelimiter( const Delimiter );
	Delimiter columnDelimiter() const;
	void setCustomDelimiterCharacter( const char );
	char delimiterCharacter() const;
	
	void setListID( const QString& );
	QString listID() const;
	bool hasNext();
	int rowCount() const;
	int columnCount() const;
	bool isValid() const;
	int columnIndexByName(const QString& columnname) const;

private:
    char m_delimiterChar;
    Delimiter m_delimiter;
    QStringList m_list;
    QString m_listId;
};

#endif
