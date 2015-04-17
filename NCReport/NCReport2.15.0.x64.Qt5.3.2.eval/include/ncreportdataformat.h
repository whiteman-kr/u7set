/****************************************************************************
*
*  Copyright (C) 2002-2015 Helta Kft. - NociSoft Software Solutions
*  All rights reserved.
*  Author: Norbert Szabo
*  E-mail: norbert@nocisoft.com, office@nocisoft.com
*  Web: www.nocisoft.com
*
*  Created: 2015. 01. 07. (nszabo)
*  This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
*  WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*
****************************************************************************/

#ifndef NCREPORTDATAFORMAT_H
#define NCREPORTDATAFORMAT_H

#include "ncreportdata.h"

#include <QChar>
#include <QVariant>

class NCReportFieldItem;

class NCReportDataFormat
{
public:
    NCReportDataFormat( NCReportData::DataType dt, const QVariant& value );
    ~NCReportDataFormat();

    void setNumberIsFormatted(bool set) { m_formatNum = set; }
    void setLocalized(bool set) { m_localized = set; }
    void setFieldWidth(int width) { m_fieldwidth = width; }
    void setNumericFormat(const char format) { m_format = format; }
    void setPrecision(int precision) { m_precision = precision; }
    void setFillChar( const QChar& fillChar) { m_fillchar = fillChar; }
    void setBlankIfZero(bool set) { m_blankIfZero = set; }
    void setDateFormat( const QString& format ) { m_dateFormat = format; }
    void setArg(const QString& arg) { m_arg = arg; }
    void setNumChars(int width) { m_numChars = width; }
    void setAlignment( Qt::Alignment alignment ) { m_alignment = alignment; }

    QString formattedData();

    void formatData(QString &result);
    static void formatField( NCReportFieldItem *field );

//    static void formatData( NCReportData::DataType dt, const QVariant& value, QString& result,
//                                bool formatNum, bool localized, int fieldwidth, char format, int precision, const QChar &fillchar, bool blankIfZero,
//                                const QString &dateFormat, const QString& arg, int numChars, Qt::Alignment alignment );


private:
    NCReportData::DataType m_type;
    QVariant m_value;

    bool m_formatNum;
    bool m_localized;
    int m_fieldwidth;
    char m_format;
    int m_precision;
    QChar m_fillchar;
    bool m_blankIfZero;
    int m_numChars;

    QString m_dateFormat;
    QString m_arg;
    Qt::Alignment m_alignment;

private:
    void formatString(QString& result);
    void formatNumber(QString& result);
    void formatDate(QString& result);
    void formatDateTime(QString& result);
    void formatBool(QString& result);


};

#endif // NCREPORTDATAFORMAT_H
