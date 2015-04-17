/****************************************************************************
*
*  Copyright (C) 2002-2014 Helta Ltd. - NociSoft Software Solutions
*  All rights reserved.
*  Author: Norbert Szabo
*  E-mail: office@nocisoft.com
*  Web: www.nocisoft.com
*
*  This file is part of the NCReport reporting software
*  Created: 2014.03.24. (nocisoft)
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
#ifndef NCREPORTMARKUPLANGUAGEOUTPUT_H
#define NCREPORTMARKUPLANGUAGEOUTPUT_H

#include "ncreportoutput.h"

#include <QTextStream>
#include <QMultiMap>
#include <QHash>

QT_BEGIN_NAMESPACE
class QFile;
QT_END_NAMESPACE

class NCReportItem;
class NCReportLabelItem;
class NCReportLineItem;
class NCReportRectItem;
class NCReportImageItem;
class NCReportTextItem;
class NCReportBarcodeItem;
class NCReportChartItem;

/*!
 * \brief The NCReportMarkupLanguageOutput class
 *
 * This class provides a base for text based markup language outputs such as Html, Xml etc.
 * The basic logic is integrated in the class, only needed to care about the output code
 */

class NCREPORTSHARED_EXPORT NCReportMarkupLanguageOutput : public NCReportOutput
{
    Q_OBJECT
public:
    NCReportMarkupLanguageOutput( QObject* parent=0 );
    ~NCReportMarkupLanguageOutput();

    enum SpreadStrategy { SectionOneTable=0, SectionMultiTables };

    virtual bool setup();
    virtual void cleanup();
    virtual void newPage();

    void renderItem( NCReportItem* item, const QRectF& rect );

    void setStrategy( SpreadStrategy strategy );
    SpreadStrategy strategy() const;
    void setStyleSheetFile( const QString& cssFile );
    QString styleSheetFile() const;
    const QString crlf() const;
    const QString styleSheetMarker() const;
    int rowCounter() const;

protected:
    virtual void writeLabel(NCReportLabelItem *item , QTextStream &cellStyle);
    virtual void writeLine( NCReportLineItem *item );
    virtual void writeRectangle( NCReportRectItem * item );
    virtual void writeImage( NCReportImageItem * item );
    virtual void writeText(NCReportTextItem * item );
    virtual void writeBarcode( NCReportBarcodeItem* item);
    virtual void writeChart( NCReportChartItem* item );


//protected:
//    virtual void writeStyleAlignLeft( QTextStream &style );
//    virtual void writeStyleAlignRight( QTextStream &style );
//    virtual void writeStyleAlignHCenter( QTextStream &style );
//    virtual void writeStyleAlignVCenter( QTextStream &style );
//    virtual void writeStyleAlignJustify( QTextStream &style );
//    virtual void writeStyleAlignTop( QTextStream &style );
//    virtual void writeStyleAlignBottom( QTextStream &style );

protected:
    // VIRTUAL CLASSES
    virtual void writeHeader() =0;
    virtual void writeFooter() =0;
    virtual void writeSectionStart( NCReportSection *section ) =0;
    virtual void writeSectionEnd( NCReportSection *section ) =0;
    virtual void writeRowStart( NCReportSection *section ) =0;
    virtual void writeRowEnd( NCReportSection *section ) =0;

    virtual void cellStarts(NCReportItem* item, QTextStream &cellStyle) =0;
    virtual void cellEnds(NCReportItem* item) =0;

    virtual void writeSpace(NCReportSection *section, int row, int width );
    virtual void writeDocumentFormats();

    virtual QString fixedID( const QString& id ) const;
    virtual QString penStyleName( int style ) const;
    virtual QString itemClassId( NCReportItem *item ) const;
    virtual QString styleSheetReference(const QString &css ) const;

protected:
    // FOR INTERNAL USE ONLY. NOT RECOMMENDED TO REIMPLEMENT
    void begin();
    void end();
    void sectionStarts(NCReportSection *section, NCReportEvaluator *evaluator);
    void sectionEnds(NCReportSection *section, NCReportEvaluator *evaluator);

    QTextStream& contentStream();
    QTextStream& styleStream();

    void writeItem(NCReportItem* item);
    void initStyleStream( QTextStream& styleStream, const QString& classID );
    void addCSSItem( const QString& classID, const QString& value );
    int& fileCounter();

private:
    QFile* m_file;
    QByteArray m_crlf;
    QString m_styleSheetMarker;
    QString m_sectionID;
    bool m_initalState;
    int m_rowCounter;
    int m_fileCounter;
    SpreadStrategy m_strategy;
    QString m_cssFile;

    QMultiMap<int,NCReportItem*> m_sectionItems;
    QHash<QString,QString> m_styleClasses;

    QString m_content;
    QString m_style;

    QTextStream m_tsContent;
    QTextStream m_tsStyle;
};


#endif // NCREPORTMARKUPLANGUAGEOUTPUT_H
