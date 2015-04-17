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
#ifndef NCREPORTFIELDITEM_H
#define NCREPORTFIELDITEM_H

#include "ncreportlabelitem.h"

class NCReportDataSource;

/*!
Field item data class
*/
class NCReportFieldData : public NCReportLabelData
{
public:
    NCReportFieldData()
     : fieldtype(0), precision(-1), fieldwidth(0), format('f'), fillchar(QChar(' ')),
       localized(false), numBlankIfZero(false), formatnum(false), numChars(0), dataSource(0), dataColumn(-1), dataRole(-1)
    {}

    uint fieldtype;
    int precision;
    int fieldwidth;
    char format;
    QChar fillchar;
    bool localized;
    bool numBlankIfZero;
    bool formatnum;
    int numChars;
    NCReportDataSource* dataSource;
    int dataColumn;
    int dataRole;

    QString dateFormat;
    QString arg;	// use QString::arg(...) evaluation
};

/*!
Field item class
 */
class NCReportFieldItem : public NCReportLabelItem
{
public:
    NCReportFieldItem( NCReportDef* rdef, QGraphicsItem* parent =0 );
    ~NCReportFieldItem();

    enum SwapMode { OriginalValue=0 ,LastValue };

    inline int precision() const
    { return ((NCReportFieldData*)d)->precision; }

    inline void setPrecision( int prec )
    { ((NCReportFieldData*)d)->precision = prec; }

    inline int fieldWidth() const
    { return ((NCReportFieldData*)d)->fieldwidth; }

    inline void setFieldWidth( int fw )
    { ((NCReportFieldData*)d)->fieldwidth = fw; }

    inline char format() const
    { return ((NCReportFieldData*)d)->format; }

    inline void setFormat( char f )
    { ((NCReportFieldData*)d)->format = f; }

    inline QChar fillChar() const
    { return ((NCReportFieldData*)d)->fillchar; }

    inline void setFillChar( const QChar& c )
    { ((NCReportFieldData*)d)->fillchar = c; }

    inline bool isLocalized() const
    { return ((NCReportFieldData*)d)->localized; }

    inline void setLocalized( bool set )
    { ((NCReportFieldData*)d)->localized = set; }

    inline bool isNumBlankIfZero() const
    { return ((NCReportFieldData*)d)->numBlankIfZero; }

    inline void setNumBlankIfZero( bool set )
    { ((NCReportFieldData*)d)->numBlankIfZero = set; }

    inline bool isNumFormat() const
    { return ((NCReportFieldData*)d)->formatnum; }

    inline void setNumFormat( bool set )
    { ((NCReportFieldData*)d)->formatnum = set; }

    inline int numCharacters() const
    { return ((NCReportFieldData*)d)->numChars; }

    inline void setNumberOfCharacters( int num )
    { ((NCReportFieldData*)d)->numChars = num; }

    inline QString arg() const
    { return ((NCReportFieldData*)d)->arg; }

    inline void setArg( const QString& arg )
    { ((NCReportFieldData*)d)->arg = arg; }

    inline QString dateFormat() const
    { return ((NCReportFieldData*)d)->dateFormat; }

    inline void setDateFormat( const QString& df )
    { ((NCReportFieldData*)d)->dateFormat = df; }

    inline NCReportDataSource* dataSource() const
    { return ((NCReportFieldData*)d)->dataSource; }
    inline void setDataSource( NCReportDataSource* ds )
    { ((NCReportFieldData*)d)->dataSource = ds; }

    inline int dataColumn() const
    { return ((NCReportFieldData*)d)->dataColumn; }
    inline void setDataColumn( int column )
    { ((NCReportFieldData*)d)->dataColumn = column; }

    inline int dataRole() const
    { return ((NCReportFieldData*)d)->dataRole; }
    inline void setDataRole( int role )
    { ((NCReportFieldData*)d)->dataRole = role; }

    bool read( NCReportXMLReader* reader );
    bool write( NCReportXMLWriter* writer);
    void setDefaultForEditor();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void paint( NCReportOutput* o, const QPointF& mPos );
    void adjustSize();
    bool isEmpty() const;

    //void swapLastValue( SwapMode );
    void updateValue(NCReportOutput*, NCReportEvaluator *evaluator);
    QList<int> availableSourceTypes() const;

    //void paint( NCReportOutput* );
protected:
    void readProps( NCReportXMLReader* r);
    void writeProps( NCReportXMLWriter* w);
};


#endif
