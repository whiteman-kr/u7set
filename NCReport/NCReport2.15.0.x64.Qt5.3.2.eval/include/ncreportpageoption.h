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
#ifndef NCREPORTPAGEOPTION_H
#define NCREPORTPAGEOPTION_H

#include "ncreport.h"

#include <QPrinter>
#include <QFont>
#include <QColor>
#include <QVector>

class NCReportXMLReader;
class NCReportXMLWriter;

/*!
This class represents the page properties of report
*/
class NCREPORTSHARED_EXPORT NCReportPageOption
{
public:
    NCReportPageOption();

    enum MultiLanguageRole { UseDefaultLanguage = 0, LeaveEmpty };

    class PageData
    {
        public:
        PageData() : name( QString() ), paperSize( QPrinter::A4 ), width( -1 ), height( -1 )
        {}
        PageData( const QString& nm, const QPrinter::PaperSize ps, const qreal w, const qreal h )
            : name(nm), paperSize( ps ), width(w), height(h)
        {}
        QString name;
        QPrinter::PaperSize paperSize;
        qreal width;
        qreal height;
    };
    void setPaperSize( const QPrinter::PaperSize );
    void setPaperSize( const QPrinter::PaperSize size, const QPrinter::Orientation orientation );
    void setPaperSize( const QString& paperSizeName,  const QSizeF& customSize = QSizeF() );
    //void setCustomPageSize( const QSizeF& );
    void setOrientation( const QPrinter::Orientation orientation );
    static void setPaperSizeByName( const QString psname, const QPrinter::Orientation orient, QPrinter::PaperSize& psid, QSizeF& size );
    static QString paperSizeToName( const QPrinter::PaperSize );
    static void loadPageDataVector( QVector<NCReportPageOption::PageData>& );

    QString reportTypeName() const;
    void setReportType( const QString& name );
    void setReportType( NCReport::ReportType );

    void adjustPageSize();
    void swapOrientation();
    bool read( NCReportXMLReader* r );
    bool write( NCReportXMLWriter* r );

    QSizeF purePageSize() const;

    QPrinter::PaperSize paperSize() const;
    QPrinter::Orientation pageOrientation() const;
    NCReport::ReportType reportType() const;

    QString reportName() const;
    QSizeF pageSizeMM() const;
    qreal pageWidthMM() const;
    qreal pageHeightMM() const;
    qreal topMarginMM() const;
    qreal bottomMarginMM() const;
    qreal leftMarginMM() const;
    qreal rightMarginMM() const;
    int columnCount() const;
    qreal columnWidth() const;
    qreal columnSpacing() const;
    QFont defaultFont() const;
    QString encoding() const;
    QColor backColor() const;
    qreal zoneSpacing() const;
    qreal sectionSpacing() const;
    bool isDoublePassMode() const;
    QString languages() const;
    QString currentLanguage() const;
    QString defaultLanguage() const;
    MultiLanguageRole languageRole() const;
    bool isMultiLanguage() const;
    bool zonesAreBreakable() const;
    bool subReportOnNewPage() const;
    QString htmlBodyStyle() const;
    QString htmlHead() const;
    QString defaultFontStyle() const;


    void setReportName(const QString&) ;
    void setPageWidthMM(qreal width) ;
    void setPageHeightMM(qreal height);
    void setTopMarginMM(qreal margin);
    void setBottomMarginMM(qreal margin);
    void setLeftMarginMM(qreal margin);
    void setRightMarginMM(qreal margin);
    void setColumnCount(int);
    void setColumnWidth(qreal width);
    void setColumnSpacing(qreal spacing);
    void setDefaultFont(const QFont& );
    void setDefaultFontFamily(const QString& );
    void setDefaultFontSize(int pointsize);
    void setDefaultFontStyle( const QString& styleExpr );
    void setEncoding(const QString& enc);
    void setBackColor(const QColor& color);
    void setZoneSpacing(qreal spacing);
    void setSectionSpacing(qreal spacing);
    void setDoublePassMode(bool);
    void setLanguages(const QString&);
    void setCurrentLanguage(const QString&);
    void setLanguageRole( MultiLanguageRole );
    void setZonesAreBreakable(bool);
    void setSubReportOnNewPage(bool set);
    void setHtmlBodyStyle(const QString& options);
    void setHtmlHead(const QString& options);

    void setCustomData(const QString& data);
    QString customData() const;

    bool isDataTrimming() const;
    void setDataTrimming( bool trimming );

    bool updateOnlyCurrentDataSourceVariables() const;
    void setupdateOnlyCurrentDataSourceVariables( bool update );

    //void setPrintQuality( QPrinter::PrinterMode );
    //void setPrintResolution(int);
    //QPrinter::PrinterMode printQuality() const;
    //int printResolution() const;

private:
    //-------------------------------------------
    QPrinter::PaperSize m_paperSize;
    QPrinter::Orientation m_pageOrientation;
    //QPrinter::PrinterMode m_quality;
    NCReport::ReportType m_reportType;

    QString m_reportName;
    QSizeF m_pageMetricSize;
    qreal m_topMargin, m_bottomMargin, m_leftMargin, m_rightMargin;
    int m_columnCount;
    qreal m_columnWidth, m_columnSpacing;
    QFont m_defaultFont;
    QString m_encoding;
    QColor m_backcolor;
    qreal m_zoneSpacing;
    qreal m_sectionSpacing;
    bool m_doublePassMode;
    QString m_languages;
    QString m_currentLanguage;
    MultiLanguageRole m_langRole;
    bool m_zonesAreBreakable;
    bool m_subreportOnNewPage;
    QString m_htmlBodyStyle, m_htmlHead;
    QString m_customData;
    bool m_dataTrimming;
    bool m_updateOnlyCurrentDsVars;
    //int m_dpi;
};

#endif
