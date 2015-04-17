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
#ifndef NCREPORTOUTPUT_H
#define NCREPORTOUTPUT_H

#include "ncreportscale.h"
#include "ncreportpageoption.h"
#include "ncreport_global.h"

#include <QString>
#include <QObject>

QT_BEGIN_NAMESPACE
class QPainter;
class QWidget;
class QPaintDevice;
QT_END_NAMESPACE

class NCReport;
class NCReportItem;
class NCReportSection;
class NCReportEvaluator;

/*!
Report output base class. This virtual class is used for defining output for NCReport.\n
To create your own report output class just implement an NCReportOutput subclass.\n
The standard subclasses are: NCReportPrinterOutput, NCReportPreviewOutput, NCReportPdfOutput.
 */
class NCREPORTSHARED_EXPORT NCReportOutput : public QObject
{
    Q_OBJECT
public:
    enum OutputType { Preview=0, QtPreview, Printer, Pdf, PostScript, Svg, Image, Text, Xml, Html, Odf, ExcelHtml, ExcelXml, PlainText, Null, Custom };
    enum GeneratingMode { DirectPainter=0, DocumentFile };

    NCReportOutput( QObject* parent=0 );
    //NCReportOutput( OutputType );
    virtual ~NCReportOutput();

    void setPageOption( const NCReportPageOption& );
    NCReportPageOption& option();
    void setAutoDelete( bool );
    bool isAutoDelete() const;
    QString fileName() const;
    OutputType output() const;
    void setShowPrintDialog( bool );
    bool showPrintDialog();
    int copies() const;
    /*!The painter of the specified output. This is defined in begin() method.*/
    QPainter* painter();
    NCReportScale& scale();
    void setParentWidget( QWidget* );
    QWidget *parentWidget();
    void setPageRange( int from, int to );
    void setPageRanges( const QString& pageRanges );
    void setPageRangeList( const QList<int>& pageList );
    void addToPageRangeList( int page );
    QList<int> pageRangeList() const;
    int pageRangeFrom() const;
    int pageRangeTo() const;
    bool isPageInRange(int page) const;
    void setBatchMode( bool );
    bool batchMode();
    void setFlagFirst( bool );
    void setFlagLast( bool );
    void setFlagPrintStarted( bool );
    bool flagFirst() const;
    bool flagLast() const;
    bool flagPrintStarted() const;
    void setReportNum( int );
    int reportNum() const;

    virtual void setCopies( int num );
    virtual void setFileName( const QString& );

    /*!
    Setup method for prepating the output, for example run printer setup dialog.\n
    If returns false, the report process won't be started
    */
    virtual bool setup() =0;
    /*!
    Cleanup method for finalizing the output.
    */
    virtual void cleanup() =0;
    virtual void begin() =0;
    virtual void end() =0;
    virtual void newPage() =0;
    /*!
    Virtual function for custom rendering methods if subclassing this class
    */
    virtual void renderItem( NCReportItem* item, const QRectF& rect );

    virtual int resolution() const;
    virtual void setResolution( int dpi );
    virtual QPaintDevice* device() const;

    //void setCurrentSection(NCReportSection *section);
    //NCReportSection* currentSection() const;
    bool isDocumentTemplateBased() const;
    virtual bool renderSection(NCReportSection*, NCReportEvaluator*);
    /*!
     * \brief sectionStarts
     * \param section report section
     * \param evaluator
     * Virtual method for custom actions when the section starts
     */
    virtual void sectionStarts(NCReportSection* section, NCReportEvaluator* evaluator);

    /*!
     * \brief sectionEnds
     * \param section
     * \param evaluator
     * Virtual method for custom actions when the section ends
     */
    virtual void sectionEnds(NCReportSection* section, NCReportEvaluator* evaluator);

    /*!
    Sets if output is successive and will not break to pages.
      */
    void setSuccessive(bool set);
    /*!
    Returns true if output is successive and will not break to pages.
      */
    bool isSuccessive() const;

    void setFinalPosMM( const QPointF& point );
    QPointF finalPosMM() const;
    GeneratingMode generatingMode() const;

    void setRotationAngle( int angle );
    int rotationAngle() const;

protected:
    virtual void setOutput( OutputType );
    virtual void setPainter( QPainter* );
    virtual void deletePainter();
    virtual void rotatePainter( QPainter* painter );
    virtual void customizePainter( QPainter* painter );

    virtual bool hasValidRotationAngle();

    void setDocumentTemplateBased(bool set);
    void setGeneratingMode(GeneratingMode mode);

private:
    OutputType m_outputType;
    GeneratingMode m_genMode;
    QPainter *m_painter;
    QWidget *m_parentWidget;

    int m_numCopies;
    int m_fromPage;
    int m_toPage;
    int m_reportno;
    int m_rotation;

    bool m_showPrintDialog;
    bool m_autoDelete;
    bool m_isDocumentTemplateBased;
    bool m_isEndlessMedia;
    bool m_batchMode;
    bool m_flagFirst;
    bool m_flagLast;
    bool m_flagPrinting;

private:
    QString m_fileName;
    QPointF m_finalPosMM;
    NCReportPageOption m_pageOption;
    NCReportScale m_scale;
    QList<int> m_pageList;
};


/*!
Null output class for double pass reports
*/
class NCReportNullOutput : public NCReportOutput
{
    Q_OBJECT
public:
    NCReportNullOutput( NCReportOutput * realOutput, QObject* parent=0);
    virtual ~NCReportNullOutput();

    bool setup();
    void cleanup();
    void begin();
    void end();
    void newPage();
    QPaintDevice* device() const;
private:
    NCReportOutput *m_realOutput;
};
#endif
