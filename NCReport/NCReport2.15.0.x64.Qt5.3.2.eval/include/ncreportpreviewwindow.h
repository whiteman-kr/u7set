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
#ifndef NCREPORTPREVIEWWINDOW_H
#define NCREPORTPREVIEWWINDOW_H

#include "ncreportpageoption.h"
#include "ncreportpreviewpagecontainer.h"
#include "ncreport_global.h"

#include <QMainWindow>
#include <QCloseEvent>
#include <QScrollArea>

class NCReportPreviewPageContainer;
class NCReportPreviewOutput;
class NCReportPreviewScrollArea;
class NCReportPreviewWindowPrivate;

QT_BEGIN_NAMESPACE
class QWidget;
class QAction;
class QActionGroup;
class QComboBox;
class QMenu;
class QTextEdit;
class QSpinBox;
class QProgressDialog;
class QSettings;
QT_END_NAMESPACE

/*!
Main window class for Print preview. Represents the contents of an NCReportPreviewOutput object
 */
class NCREPORTSHARED_EXPORT NCReportPreviewWindow : public QMainWindow
{
      Q_OBJECT
public:
    NCReportPreviewWindow(QWidget *parent = 0, Qt::WindowFlags flags=0, const QString &orgName = QString(), const QString &appName = QString() );
    ~NCReportPreviewWindow();

    enum PagePosition { Top, Bottom, Off };

    void setOutput( NCReportPreviewOutput* );
    void setReport( NCReport* );
    NCReport* report() const;
    void setDefaultOutputDir( const QString& dir );
    QString defaultOutputDir() const;

    void setPrinterName( const QString& printer );
    QString printerName() const;
    void setUsePrintDialog( bool set );
    bool usePrintDialog() const;

    //bool setPageIMFData(QIODevice *device, const QString& imfdata);
    int pageCount() const;
    int reportCount() const;
    NCReportPreviewPageContainer::ShowType showType() const;
    void setCurrentPage( int pageno, NCReportPreviewPageContainer::ShowType type = NCReportPreviewPageContainer::Single );

    QAction* actionPrint() const { return m_actionPrint; }
    QAction* actionPdf() const { return m_actionPdf; }
    QAction* actionSvg() const { return m_actionSvg; }

    QAction* actionClose() const { return m_actionExit; }

    QAction* actionFirstPage() const { return m_actionFrst; }
    QAction* actionNextPage() const { return m_actionNext; }
    QAction* actionPreviousPage() const { return m_actionPrev; }
    QAction* actionLastPage() const { return m_actionLast; }
    QAction* actionGoToPage() const { return m_actionGo; }
    QAction* actionZoomIn() const { return m_actionZP; }
    QAction* actionZoomOut() const { return m_actionZM; }
    QAction* actionZoomOff() const { return m_actionZ1; }
    QAction* actionNextReport() const { return m_actNextReport; }
    QAction* actionPreviousReport() const { return m_actPrevReport; }
    QAction* actionViewSingleMode() const { return m_actViewSingle; }
    QAction* actionViewDoubleMode() const { return m_actViewDouble; }
    QAction* actionViewContinousMode() const { return m_actViewContinous; }

    bool donePrint() const { return m_donePrint; }
    bool donePDF() const { return m_donePDF; }
    bool doneSVG() const { return m_doneSVG; }

    QToolBar *toolBarFile();
    QToolBar *toolBarView();
    QToolBar *toolBarNavigation();

    void exec();

    void setDefaultOutputFileName( const QString& fileName );
    QString defaultOutputFileName() const;

    void setSettingsData( const QString& orgName, const QString& appName );

signals:
    void runPrinterDone();
    void runPdfDone();
    void runSvgDone();

public slots:
    virtual void nextPage( const PagePosition = Top );
    virtual void prevPage( const PagePosition = Top );
    virtual void firstPage( const PagePosition = Top );
    virtual void lastPage( const PagePosition = Top );

    virtual void open();
    virtual bool save();
    virtual bool saveAs();
    virtual void about();
    virtual void documentWasModified();
    virtual void activatePrint();
    virtual void activatePdf();
    virtual void activateSvg();

    virtual void zoomChanged( const QString & s);
    virtual void zoomChanged();
    virtual void zoomChanged( int scaleFactor );
    virtual void zoomPlus();
    virtual void zoomMinus();
    virtual void zoom1();

    virtual void gotoPage();
    virtual void gotoPage( int pageno );

    virtual void nextReport();
    virtual void prevReport();
    virtual void sliderMoved(int value);

protected slots:
    virtual void loadPage( int pageno );
    virtual void setViewSingle();
    virtual void setViewDouble();
    virtual void setViewContinous();
    virtual void viewChange(QAction*);

    //TESTS
    virtual void testPrint1();
    virtual void testPrint2();
    virtual void addTestItems();
    virtual void clearPage();

protected:
    virtual void closeEvent(QCloseEvent *event);
    virtual void loadPage( int pageno, const PagePosition );

private:
    Q_DECLARE_PRIVATE(NCReportPreviewWindow);
    QScopedPointer<NCReportPreviewWindowPrivate> const d_ptr;

    NCReportPreviewScrollArea *m_scrollArea;
    NCReportPreviewPageContainer *m_pageContainer;
    NCReportPreviewOutput *m_output;
    NCReport *m_report;

    QMenu *mnFile;
    QMenu *mnView;
    QMenu *mnNav;
    QMenu *mnAbout;

    QToolBar *tbFile;
    QToolBar *tbView;
    QToolBar *tbNav;

    QActionGroup *m_actGrpView;
    QAction *m_actionOpen;
    QAction *m_actionPrint, *m_actionPdf, *m_actionSvg, *m_actionFrst, *m_actionNext, *m_actionPrev, *m_actionLast;
    QAction *m_actionGo, *m_actionExit, *m_actionZP, *m_actionZM, *m_actionZ1;
    QAction *m_actNextReport, *m_actPrevReport;
    QAction *m_actViewSingle, *m_actViewDouble, *m_actViewContinous;
    QAction *m_actionAbout;
    QAction *m_actionAboutQt;
    QComboBox *m_cbZoom;
    QSpinBox *m_spZoom;
    QSpinBox *m_spPage;

    int m_currentPage;
    int m_currentReport;

    qreal m_initialZoomLevel;
    bool m_usePrintDialog;
    bool m_donePrint, m_donePDF, m_doneSVG;

    QString m_orgName;
    QString m_appName;
    QString m_lastSelectedDir;
    QString m_defaultOutputFileName;
    QString m_printerName;
    QString m_winTitle;
    QString m_curFile;
    QStringList pagesData;

private:

    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();

    void readSettings();
    void readSettings( QSettings* settings );
    void writeSettings();
    void writeSettings( QSettings* settings );
    bool maybeSave();
    void loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void refreshActions();
    void updateZoomInfo();
    void showEvent(QShowEvent *event);
    void initPrinter( QPrinter* );
    void initProgress( QProgressDialog* );
    void printPicturePages( QPrinter* );
    void zoom( qreal );

    void setCurrentFile(const QString &fileName);
    QString strippedName(const QString &fullFileName);
    QString genOutputFileName(const QString &extension) const;

};


class NCReportPreviewScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    NCReportPreviewScrollArea( NCReportPreviewWindow * parent = 0 );
    ~NCReportPreviewScrollArea();

protected:
    void keyPressEvent( QKeyEvent* );

private:
    NCReportPreviewWindow* pWindow;
};


#endif
