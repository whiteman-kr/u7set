/****************************************************************************
*
*  Copyright (C) 2002-2014 Helta Ltd. - NociSoft Software Solutions
*  All rights reserved.
*  Author: Norbert Szabo
*  E-mail: office@nocisoft.com
*  Web: www.nocisoft.com
*
*  This file is part of the NCReport reporting software
*  Created: 2014.03.25. (nocisoft)
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

#ifndef NCREPORTEXCELHTMLOUTPUT_H
#define NCREPORTEXCELHTMLOUTPUT_H

#include "ncreporthtmloutput.h"

/*!
 * \brief Html based Excel output class
 *
 * Excel output based on html markup code. Result pages are stored in one xls.html file
 */
class NCREPORTSHARED_EXPORT NCReportExcelHtmlOutput : public NCReportHtmlOutput
{
    Q_OBJECT
public:
    NCReportExcelHtmlOutput( QObject* parent=0 );
    ~NCReportExcelHtmlOutput();

protected:
    void writeLabel(NCReportLabelItem *item , QTextStream &cellStyle);
    void writeText(NCReportTextItem * item );

    void writeHeader();
    void writeFooter();

    void writeSpace(NCReportSection *section, int row, int width );
    void writeSectionStart( NCReportSection *section );
    void writeSectionEnd( NCReportSection *section );
    void writeRowStart( NCReportSection *section );
    void writeRowEnd(NCReportSection *section);

    void cellStarts(NCReportItem *item, QTextStream& cellStyle);
    void cellEnds(NCReportItem *item);

    QString styleSheetReference(const QString &css ) const;

private:
    QString alignmentName( const Qt::Alignment a ) const;
    void fixNumericStringValue( QString& value );
    void setFontAttributes(NCReportLabelItem *label, QString& value );

    void setFontAttributesStart(NCReportLabelItem *label);
    void setFontAttributesEnd(NCReportLabelItem *label);
};


#endif // NCREPORTEXCELHTMLOUTPUT_H
