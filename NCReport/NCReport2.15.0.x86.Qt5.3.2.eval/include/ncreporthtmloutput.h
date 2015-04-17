/***************************************************************************
 *   Copyright (C) 2010 by NociSoft Software Solutions
 *   support@nocisoft.com
 ***************************************************************************/
#ifndef NCREPORTHTMLOUTPUT_H
#define NCREPORTHTMLOUTPUT_H

#include "ncreportmarkuplanguageoutput.h"

/*!
Html Output class. Result pages are stored in one .html file
*/
class NCREPORTSHARED_EXPORT NCReportHtmlOutput : public NCReportMarkupLanguageOutput
{
    Q_OBJECT
public:
    NCReportHtmlOutput( QObject* parent=0 );
    ~NCReportHtmlOutput();

    enum ImageMode { Base64, File };
    void setImageMode( ImageMode mode );
    ImageMode imageMode() const;

#ifdef USE_QIMAGE_INSTEAD_OF_QPIXMAP
    QByteArray imageToHtml(const QImage& image, const QSize &size, ImageMode mode );
#else
    QByteArray imageToHtml(const QPixmap& image, const QSize &size, ImageMode mode );
#endif

protected:
    virtual void writeLabel(NCReportLabelItem *item , QTextStream &cellStyle);
    virtual void writeLine( NCReportLineItem *item );
    virtual void writeRectangle( NCReportRectItem * item );
    virtual void writeImage( NCReportImageItem * item );
    virtual void writeText(NCReportTextItem * item );
    virtual void writeBarcode( NCReportBarcodeItem* item);
    virtual void writeChart( NCReportChartItem* item );

    virtual void writeHeader();
    virtual void writeFooter();

    virtual void writeSpace(NCReportSection *section, int row, int width );
    virtual void writeSectionStart( NCReportSection *section );
    virtual void writeSectionEnd( NCReportSection *section );
    virtual void writeRowStart( NCReportSection *section );
    virtual void writeRowEnd(NCReportSection *section);

    virtual QString styleSheetReference(const QString &css ) const;
    virtual void cellStarts(NCReportItem *item, QTextStream& cellStyle);
    virtual void cellEnds(NCReportItem *item);

    virtual void cleanHtml( QString& html );

private:
    ImageMode m_imageMode;
};

#endif // NCREPORTHTMLOUTPUT_H
