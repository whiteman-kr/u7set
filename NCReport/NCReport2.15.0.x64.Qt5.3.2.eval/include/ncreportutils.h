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
#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include "ncreport_global.h"

class NCREPORTSHARED_EXPORT NCReportUtils
{
public:
    NCReportUtils();

    static bool fileToString( const QString& filename, QString& target );
    static bool fileToStringAll( const QString& filename, QString& target );
    static void evaluateFileName( QString& filename );
    static QString evaluatedFileName( const QString& filename );
    static qreal fontSizeConvert( qreal pointSize );
    static qreal fontSizeRestore( qreal pointSize );
    static bool stringToFile( const QString& filename, const QString& content, const char* codecName = "UTF-8" );

    static int verticalAlignmentToIndex( const Qt::Alignment a );
    static int horizontalAlignmentToIndex( const Qt::Alignment a );
    static Qt::Alignment indexToVerticalAlignment( int idx );
    static Qt::Alignment indexToHorizontalAlignment( int idx );

    static Qt::Alignment verticalAlignment( const Qt::Alignment alignment );
    static Qt::Alignment horizontalAlignment( const Qt::Alignment alignment );

    static QString randomString(const int len);
    static QString parse( const QString& string, const QString& startTag, const QString& endTag );
    static void saveFilePath(const char *name, const QString& fileName );
    //static qreal roundToDecimals(const double& x, const int& numDecimals);
    static qreal fround(qreal n, int d);
    static double standardDeviation( const QList<double>& numbers );
    static QString intListToString( const QList<int>& list );
    static void stringToIntList(const QString& str, QList<int>& intlist);

    class id2D {
    public:
        id2D( short x, short y);
        id2D( int id );
        short x() const;
        void setX( short x );
        short y() const;
        void setY( short y );
        int id() const;
    public:
        static short x( int id );
        static short y( int id );
        static int id( short x, short y );
        static id2D fromId( int id );
    private:
        short m_x, m_y;
    };

};

#endif // UTILS_H
