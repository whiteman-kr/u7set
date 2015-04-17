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
#ifndef NCREPORT_H
#define NCREPORT_H

#include "ncreportparameter.h"
#include "ncreportsource.h"
#include "ncreport_global.h"

#include <QObject>
#include <QPointer>
#include <QStringList>
#include <QColor>

class NCReportDef;
class NCReportOutput;
class NCReportDirector;
class NCReportDataSource;
class NCReportGraphRenderer;
class NCReportPreviewWindow;
class NCReportDataSourceFactory;
class NCReportUserFunction;
class NCReportEvaluator;
class LMailSender;

QT_BEGIN_NAMESPACE
class QAbstractItemModel;
class QTextDocument;
class QWidget;
class QTableView;
QT_END_NAMESPACE

/*!
 * \brief The main report engine class.
 *
 * NCReport report generator engine class. For running reports you need to use this class. Provides the most important methods for API
 */
class NCREPORTSHARED_EXPORT NCReport : public QObject
{
    Q_OBJECT
public:
    NCReport( QObject *parent=0 );
    ~NCReport();

    enum ReportType { Report=0, TextDocumentPrint };

    static const QString version();
    static const QString info();
    void reset( bool all=false );
    void reset(bool clearRenderers, bool clearParameters, bool clearUserFunctions, bool clearStringLists, bool clearBatchReports, bool keepCustomDataSources = true );
    /*! Specifies the report source property by type */
    void setReportSource( NCReportSource::ReportSourceType );
    /*! Specifies the report source object */
    void setReportSource( const NCReportSource & reportSource);
    /*! Specifies the report output */
    void setOutput( NCReportOutput*, bool deleteLast=true );
    /*! Specifies the file name of the report output object that uses files*/
    void setReportFile( const QString& );
    QString reportFile() const;

    /*! Obsolete. Use setReportDefXml() instead. */
    void setReportDef( const QString& xml );
    /*! With this method you can add report definition XML string directly to report */
    void setReportDefXml( const QString& xml );
    QString reportDefXml() const;

    void setForceCopies( int );
    /*! Enables os disables using wait cursor while running process */
    void setUseWaitCursor( bool );
    /*! Sets a custom, external text document for printing in NCReport::TextDocumentPrint mode */
    void setTextDocument( QTextDocument* );
    void setPageProgressText( const QString& );
    QString pageProgressText() const;
    /*! sets a batch of report*/
    void setReportBatch( const QStringList& );
    /*! adds a report to report batch */
    void addReportToBatch( const QString& reportXML );
    void addReportToBatch( const QString& reportXML, const QHash<QString,QVariant>& inAssociatedParameters );
    void addReportToBatch(NCReport *report );
    /*! Clears the list of report batch */
    void clearBatch();
    bool isBatchMode() const;

    void setCurrentLanguage( const QString& langcode );

    /*! Adds a parameter to the report by id and value*/
    void addParameter( const QString& id, const QVariant& value );

    /*! Adds a custom datasource to the report*/
    void addCustomDataSource( NCReportDataSource* );

    /*!
     * Adds a datasource to the report. This is equvivalent to addCustomDataSource() method.
     */
    void addDataSource(NCReportDataSource *ds);

    /*! Sets a factory for creating the custom datasources in the report*/
    void setCustomDataSourceFactory( NCReportDataSourceFactory* );

    /*! Adds a graph item rendering object to the report. This is obsolete. Use addGraphItemRenderer instead.*/
    void addItemRenderingClass(NCReportGraphRenderer* renderer);
    /*! Adds a graph item rendering object to the report.*/
    void addGraphItemRenderer(NCReportGraphRenderer* renderer);

    void addStringList( const QStringList& list, const QString& id );
    void addStringList(  const QString& id, const QStringList& list );

    void addItemModel( QAbstractItemModel* model, const QString& id );
    void addItemModel( const QString& id, QAbstractItemModel* model );

    void setCustomData(const QString& data);
    QString customData() const;

#ifndef NCREPORT_NO_TABLE_VIEW
    /*!
      Adds a table view to the report with the specified id. The view is used by table item to render the table view contents
      */
    void addTableView( QTableView* tableView, const QString& id );
    void addTableView( const QString& id, QTableView* tableView );
#endif

    void addUserFunction( const QString &id, NCReportUserFunction *function );

    /*! Returns true if an error occured; otherwise false.*/
    bool hasError();
    /*! Returns the last error message if error occured*/
    QString lastErrorMsg();
    /*! Returns the current output for the report*/
    NCReportOutput *output();
    /*! Returns the current report source reference*/
    NCReportSource& reportSourceRef();
    /*! Returns the current report source*/
    NCReportSource reportSource() const;
    /*! Returns the current NCReportDataSource for the report*/
    NCReportDataSource *dataSource( const QString& );
    /*! Returns the report's definition object*/
    NCReportDef *reportDef() {return m_reportDef;}
    /*! Returns the type of the report*/
    ReportType reportType() const { return m_reportType; }
    /*! Returns the text document assigned to the report*/
    QTextDocument *document() { return m_document; }
    /*! Returns last elapsed report runtime */
    int runTimeElapsed() const;

    QPrinter::PrinterMode printQuality() const;
    int printResolution() const;
    void setPrintQuality( QPrinter::PrinterMode );
    void setPrintResolution(int);
#ifndef NCREPORT_NO_PREVIEW_WINDOW
    NCReportPreviewWindow *createPreviewWindow( QWidget* parent = 0);
    QWidget* runReportToShowPreview();
    bool runReportToShowPreviewDonePrint();
    QWidget* execPreview( NCReportOutput* output );
    QVariant getParameter( const QString& id, const QVariant& value, QWidget* parent, const QString & title, const QString & label, bool* ok );
#endif
    int pageCount() const;
    /*! Returns true if report running has succesfully finished*/
    bool reportDone() const;

    void setShowWaitCursor( bool set );
    bool showWaitCursor() const;

    void setRootDir( const QString& rootDir );
    NCReportOutput* createOutput( int outputType, QObject* parent = 0);

//    /*!
//     * \brief clone
//     * \param parent
//     * \return new NCReport object
//     *  Creates a new NCReport object with copying data sources and parameters from the original object
//     */
//    NCReport* clone( QObject* parent = 0 );

    /*!
     * \brief setProcessEvents
     * \param enable
     * Enables or disables the mode when QApplication::processEvents() is applied in the report director.
     * This option should be enabled if we want to be able to cancel the report running process from a user interface.
     */
    void setProcessEvents( bool enable );
    bool isProcessEvents() const;
    /*!
     * \brief setConnectionID
     * \param connectionID connection name/id
     * Sets an application level connection id for SQL database connections.
     * If this property is set then the report engine uses the named connection besides the report level defined connection ID.
     * If not any connection ID is set for data sources in the report, this connection is considered.
     */
    void setConnectionID( const QString& connectionID );
    QString connectionID() const;

    void setEvaluator( NCReportEvaluator* evaluator);
    NCReportEvaluator *evaluator();

    QString zintPath() const;
    void setZintPath( const QString& dir );

    void setRotationAngle( int angle );
    int rotationAngle() const;

    void setPageRanges( const QString& ranges );
    QString pageRanges() const;

    void setAlternateRowBackgroundColor( const QColor& color );
    QColor alternateRowBackgroundColor() const;

signals:
    /*! This signal is emitted when the report start */
    void reportStarts();
    /*! This signal is emitted when the report ends */
    void reportEnds();
    /*! This signal is emitted when a datasource opens. This is when SQL query stars in SQL datasource. */
    void dataSourceOpen( const QString& dsID );
    /*! This signal is emitted after the datasource opened. totalRows is the number of rows in the datasource (if available). */
    void dataSourceOpened( const QString& dsID, int totalRows);
    /*! This signal is emitted when a datasource record / row is pending. */
    void dataRowProgress( const QString& dsID, int row );
    /*! This signal is emitted when the current datasource record / row is pending. */
    void dataRowProgress( int row );
    /*! This signal is emitted before the current datasource is progressed. */
    void dataRowCount( int numRows );
    /*! This signal is emitted before a report section is rendered */
    void sectionProgress( const QString& sectionID );
    /*! This signal is emitted when a (new) page started. */
    void pageProgress( int page );
    /*! This signal is emitted when a (new) page started. */
    void pageProgress( const QString& );
    /*! This signal is emitted before the QPrintPreview dialog appears when QtPreview mode. */
    void showQtPreviewDialog();
    /*! This signal is emitted when a datasource emits an update request signal.
     * The data Parameter contains the evaluated data source specification
     */

    /*!
     * \brief dataSourceUpdateRequestFKey
     * \param dataSourceID
     * \param foreignKeyValue
     * This signal is emitted when a datasource emits an update request signal.
     * The signal passes the current foreign key value
     */
    void dataSourceUpdateRequestFKey(const QString& dataSourceID, const QString& foreignKeyValue);
    /*!
     * \brief dataSourceUpdateRequest
     * \param dataSourceID
     * \param data
     * This signal is emitted when a datasource emits an update request signal.
     * The data Parameter contains the evaluated data source specification
     */
    void dataSourceUpdateRequest(const QString& dataSourceID, const QString& data);
    /*! Signal to make possible to manipulate report definition object */
    void reportDefChange( NCReportDef* );

public slots:
    /*! This function runs the report and returns true if the report process succeeded, otherwise false.*/
    bool runReport();
    void runReportToPrinter( int copies=1, bool showdialog=true, QWidget* parent=0, const QString &printerName = QString() );
    void runReportToPrinterNoDialog( int copies, const QString& printerName );
#ifndef NCREPORT_NO_PREVIEW_WINDOW
    void runReportToPreview();
    void runReportToQtPreview();
#endif
    void runReportToPDF(const QString& filename );
    void runReportToPdf(const QString& filename );
#ifndef NCREPORT_NO_EMAIL
    void runReportToPDFSendMail( const QString& filename, LMailSender* mailSender );
#endif
    void runReportToPostScript( const QString& filename );
#ifndef NCREPORT_NO_SVG
    void runReportToSVG( const QString& filename );
#endif
    void runReportToImage( const QString& filename );
    void runReportToHTML(const QString& filename, int strategy=0, const QString& cssFile = QString() );
    void runReportToHtml(const QString& filename, int strategy=0, const QString& cssFile = QString() );
    void runReportToExcelHtml(const QString& filename );
    void runReportToText( const QString& filename, const QString &templateFileName );
    void cancel();
    void setLastPagePositionMM( const QPointF& position );
    QPointF lastPagePositionMM() const;

    void setParentWidget(QWidget* parent);
    QWidget *parentWidget() const;
    bool getParameters();

private slots:
    void slotDataSourceOpen( const QString& dsID );
    void slotDataSourceOpened( const QString& dsID, int totalRows );
    void slotDataRowProgress( const QString& dsID, int row );
    void slotDataRowProgress( int row );
    void slotDataRowCount( int numRows );
    void slotSectionProgress( const QString& sectionID );
    void slotPageProgress( int page );

protected:
    virtual void customizeReportDef( NCReportDef* );

private:
    NCReportDef *m_reportDef;
    NCReportDirector *m_director;
    QPointer<NCReportOutput> m_output;
    int m_forcedCopy;
    bool m_useWaitCursor;
    ReportType m_reportType;
    QTextDocument *m_document;
    bool m_cancelled;
    int m_elapsed;
    QString m_pageProgressText;
    int m_dpi;
    int m_pageCount;
    int m_rotation;
    bool m_reportDone;
    bool m_showWaitCursor;
    bool m_pEvents;
    QPrinter::PrinterMode m_quality;
    NCReportEvaluator *m_evaluator;
    QWidget *m_parentWidget;
    QColor m_altBackColor;

private:
    QString m_customData;
    NCReportSource m_source;
    QStringList m_reportStringBatch;
    QList<NCReport*> m_reportObjectBatch;
    QList< QHash< QString, QVariant > > m_reportBatchParameters;
    QPointF m_lastPagePositionMM;
    QString m_pageRanges;
private:
    void loadConfig();
    bool reportProcess();
    bool runReport( bool batchMode, bool first, bool last );
    bool runReportBatchV1();
    bool runReportBatch();
    void connectDataSourceSignals();
};


#endif //NCREPORT_H

