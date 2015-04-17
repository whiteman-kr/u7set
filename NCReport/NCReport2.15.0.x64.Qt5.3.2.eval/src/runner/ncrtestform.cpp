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
#include "ncrtestform.h"
#include "ncreport.h"
#include "ncreportprinteroutput.h"
#include "ncreportpdfoutput.h"
#include "ncreportpreviewoutput.h"
#include "info.h"
#include "ncreportpreviewwindow.h"
#include "ncreportutils.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>
#include <QFileInfo>
#include <QApplication>
#include <QSqlError>
#include <QCloseEvent>
#include <QProgressDialog>

#define NCRTESTER_SETTINGS_ORG	"NociSoft"
#define NCRTESTER_SETTINGS_APP	"NCReportTester"

NCRTestForm::NCRTestForm(QWidget* parent)
: QWidget( parent ), Ui::NCRTestFormUI()
{
    setupUi(this);
    tableParams->setAlternatingRowColors( true );
    tableParams->verticalHeader()->setDefaultSectionSize( 21 );
    setWindowTitle( tr( "NCReport Tester v%1" ).arg( NCREPORTAPP_VERSION ) );

    report = 0;

    connect( cbUseDBConnect, SIGNAL(toggled(bool)), this, SLOT(connectToggled(bool)) );
    connect( buttonBox->button(QDialogButtonBox::Ok), SIGNAL( clicked() ), this, SLOT( run() ) );
    connect( buttonBox->button(QDialogButtonBox::Close), SIGNAL( clicked() ), this, SLOT( close() ) );
    connect( btnAdd, SIGNAL( clicked() ), this, SLOT( addParam() ) );
    connect( btnRemove, SIGNAL( clicked() ), this, SLOT( removeParam() ) );
    connect( btnSelFile, SIGNAL( clicked() ), this, SLOT( selectFiles() ) );

    loadDefaults();
    connectToggled( false );
/*#ifdef Q_WS_WIN
    resize( QSize(600, sizeHint().height() ) );
#else
    resize( QSize(650, sizeHint().height() ) );
#endif*/

}

NCRTestForm::~NCRTestForm()
{
}

/*$SPECIALIZATION$*/
void NCRTestForm::setReport(NCReport *r )
{
    report = r;
    connect( report, SIGNAL(reportStarts()), this, SLOT(showReportProgressDialog()) );
    connect( report, SIGNAL(reportEnds()), this, SLOT(hideReportProgressDialog()) );
}

void NCRTestForm::connectToggled( bool on )
{
    comboDriver->setEnabled(on);
    leHost->setEnabled(on);
    leDB->setEnabled(on);
    leUser->setEnabled(on);
    lePassw->setEnabled(on);
    leConnID->setEnabled(on);
}

void NCRTestForm::selectFiles()
{
    QFileInfo fi( leFile->text() );
    filelist = QFileDialog::getOpenFileNames( this, tr("Select report file(s)"), fi.absoluteFilePath(), tr("Reports (*.xml *.txt)") );
    if ( filelist.isEmpty() )
        return;

    QString filenames;
    for (int i=0; i<filelist.size(); ++i ) {
        filenames += filelist.at(i) +";";
    }
    filenames.truncate( filenames.length()-1 );
    leFile->setText( filenames );
}

void NCRTestForm::addParam()
{
    addParam("","");
    int rows = tableParams->rowCount();
    if ( rows>0 )
        tableParams->setCurrentCell( rows-1, 0 );
    tableParams->setFocus();
}


void NCRTestForm::addParam( const QString& name, const QString& value )
{
    int rows = tableParams->rowCount();
    tableParams->setRowCount( rows+1 );

    QTableWidgetItem *newItem;

    newItem = new QTableWidgetItem(name);
    tableParams->setItem(rows, 0, newItem);

    newItem = new QTableWidgetItem(value);
    tableParams->setItem(rows, 1, newItem);
}

void NCRTestForm::removeParam()
{
    int i = tableParams->currentRow();

    QSettings settings(NCRTESTER_SETTINGS_ORG, NCRTESTER_SETTINGS_APP);
    settings.remove( "ncreporttest/parameters/"+tableParams->item(i,0)->text() );
    tableParams->removeRow(i);
}

void NCRTestForm::saveDefaults( )
{
    QSettings settings(NCRTESTER_SETTINGS_ORG, NCRTESTER_SETTINGS_APP);
    //settings.setPath( "NCReport", "TestForm" );

    settings.setValue( "ncreporttest/connid", leConnID->text() );
    settings.setValue( "ncreporttest/driver", comboDriver->currentIndex() );
    settings.setValue( "ncreporttest/host", leHost->text() );
    settings.setValue( "ncreporttest/DB", leDB->text() );
    settings.setValue( "ncreporttest/user", leUser->text() );
    settings.setValue( "ncreporttest/passw", lePassw->text() );

    settings.setValue( "ncreporttest/reportfile", leFile->text() );
    // paramters
    for (int i=0; i<tableParams->rowCount(); ++i )
        settings.setValue( "ncreporttest/parameters/"+tableParams->item(i,0)->text(), tableParams->item(i,1)->text() );
}

void NCRTestForm::loadDefaults( )
{
    QSettings settings(NCRTESTER_SETTINGS_ORG, NCRTESTER_SETTINGS_APP);

    leConnID->setText(settings.value( "ncreporttest/connid", "" ).toString());
    comboDriver->setCurrentIndex(settings.value( "ncreporttest/driver", "0" ).toInt());
    leHost->setText(settings.value( "ncreporttest/host", "localhost" ).toString());
    leDB->setText(settings.value( "ncreporttest/db", "test" ).toString());
    leUser->setText(settings.value( "ncreporttest/user", "root" ).toString());
    lePassw->setText(settings.value( "ncreporttest/passw", "" ).toString());
    leFile->setText(settings.value( "ncreporttest/reportfile", "" ).toString());
    settings.beginGroup("ncreporttest/parameters");
    QStringList keys = settings.childKeys();

    QStringList::const_iterator it;
    for (it = keys.constBegin(); it != keys.constEnd(); ++it) {
        QString name = *it;
        QString value = settings.value(*it).toString();
        addParam( name, value );
    }

    if ( tableParams->rowCount() == 0 ) {
        addParam( "documentPK", "10367" );
        addParam( "prodFilt", "%" );
    }

    settings.endGroup();
}


void NCRTestForm::closeEvent( QCloseEvent* ce )
{
    saveDefaults();
    ce->accept();
}


bool NCRTestForm::connectToDatabase()
{
    // SQL/DATABASE CONNECTION
    QSqlDatabase defaultDB = QSqlDatabase::addDatabase(comboDriver->currentText(), leConnID->text() );
    if ( !defaultDB.isValid() ) {
            //fprintf( stderr, "Error: Could not load database driver. \n" );
        QApplication::restoreOverrideCursor();
        QMessageBox::warning( this, QObject::tr("NCReport error"), QObject::tr("Could not load database driver.") );
            //delete report;
        return false;
    }
    defaultDB.setHostName( leHost->text() );
    defaultDB.setDatabaseName( leDB->text() );
    defaultDB.setUserName( leUser->text() );
    defaultDB.setPassword( lePassw->text() );

    if ( !defaultDB.open() ) {
            //fprintf( stderr, "Error: Cannot open database. %s\n", qPrintable( defaultDB.lastError().databaseText() ) );
        QApplication::restoreOverrideCursor();
        QMessageBox::warning( this, QObject::tr("NCReport error"), QObject::tr("Cannot open database: ")+defaultDB.lastError().databaseText() );
            //delete report;
        return false;
    }

    return true;
}

void NCRTestForm::run()
{
    //-----------------------------
    // CHECK REPORT OBJECT
    //-----------------------------
    if ( !report )
        return;

    report->reset();

    //NCReport *report = new NCReport( this );

    //-----------------------------
    // INTERNAL DATABASE CONNECTION
    //-----------------------------
    if ( cbUseDBConnect->isChecked() ) {
        if ( !connectToDatabase() )
            return;
    }

    //-----------------------------
    // SET THE REPORT SOURCE
    //-----------------------------

    if ( rbSingle->isChecked() ) {
        // normal (single) mode
        report->clearBatch();
        report->setReportSource( NCReportSource::File );
        report->setReportFile( leFile->text() );
    } else {
        // batch mode
        report->clearBatch();
        QStringList reportfiles = leFile->text().split(";", QString::SkipEmptyParts);
        for ( int i=0; i<reportfiles.count(); ++i ) {
            QString reportXML;
            if ( NCReportUtils::fileToString( reportfiles.at(i), reportXML ) )
                report->addReportToBatch( reportXML );
        }
    }

    //-----------------------------
    // ADD PARAMETERS
    //-----------------------------
    for (int i=0; i<tableParams->rowCount(); ++i )
        report->addParameter( tableParams->item(i,0)->text(), tableParams->item(i,1)->text() );

    //-----------------------------
    // CREATE REPORT OUTPUT
    //-----------------------------

    if ( rbPreview->isChecked() )
        report->runReportToPreview();
    else if ( rbPrinter->isChecked() )
        report->runReportToPrinter();
    else if ( rbQtPreview->isChecked() )
        report->runReportToQtPreview();
    else if ( rbPdf->isChecked() ) {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save PDF File"),
                "report.pdf", tr("Pdf files (*.pdf)"));
        if ( fileName.isEmpty() )
            return;
        report->runReportToPDF( fileName );
    }
    else if ( rbSvg->isChecked() ) {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save multiple SVG files"),
                "svg_output.svg", tr("Svg files (*.svg)"));
        if ( fileName.isEmpty() )
            return;
        report->runReportToSVG( fileName );
    }

    bool error = report->hasError();
    QString err = report->lastErrorMsg();

    //-----------------------------
    // ERROR HANDLING
    //-----------------------------
    if ( error )
        QMessageBox::information( 0, tr("Report error"), err );
    else {
        if ( rbPreview->isChecked() ) {
            //-----------------------------
            // PRINT PREVIEW
            //-----------------------------
            NCReportPreviewWindow *pv = new NCReportPreviewWindow();
            pv->setOutput( (NCReportPreviewOutput*)(report->output()) );
            pv->setReport( report );
            pv->setWindowModality(Qt::ApplicationModal );
            pv->setAttribute( Qt::WA_DeleteOnClose );
            pv->show();
        }
    }

    //delete report;

}

void NCRTestForm::run_old()
{
    //-----------------------------
    // CHECK REPORT OBJECT
    //-----------------------------
    if ( !report )
        return;

    report->reset();

    //NCReport *report = new NCReport( this );

    //-----------------------------
    // INTERNAL DATABASE CONNECTION
    //-----------------------------
    if ( cbUseDBConnect->isChecked() ) {
        if ( !connectToDatabase() )
            return;
    }

    //-----------------------------
    // SET THE REPORT SOURCE
    //-----------------------------

    /*
    report->setReportSource( NCReportSource::File );
    report->reportSource()->setFileName( leFile->text() );
    */

    //TEST
    QStringList reports;

    report->setReportBatch( reports );

    //-----------------------------
    // ADD PARAMETERS
    //-----------------------------
    for (int i=0; i<tableParams->rowCount(); ++i )
        report->addParameter( tableParams->item(i,0)->text(), tableParams->item(i,1)->text() );

    //-----------------------------
    // CREATE REPORT OUTPUT
    //-----------------------------
    NCReportOutput *output=0;

    if ( rbPreview->isChecked() ) {
        output = new NCReportPreviewOutput();
        output->setAutoDelete( false );
        report->setOutput( output );

    } else if ( rbPrinter->isChecked() ) {
        output = new NCReportPrinterOutput();
        output->setCopies(1);
        output->setShowPrintDialog(true);
        output->setParentWidget( this );
        report->setOutput( output );
    } else if ( rbPdf->isChecked() ) {

        QString fileName = QFileDialog::getSaveFileName(this, tr("Save PDF File"),
                "report.pdf", tr("Pdf files (*.pdf)"));
        if ( fileName.isEmpty() ) {
            return;
        } else {
            output = new NCReportPdfOutput();
            output->setFileName( fileName );
            //output->setFileName( "testreport.pdf" );
            report->setOutput( output );
        }
    }

    //-----------------------------
    // RUN REPORT
    //-----------------------------
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    report->runReport();
    bool error = report->hasError();
    QString err = report->lastErrorMsg();

    QApplication::restoreOverrideCursor();

    //-----------------------------
    // ERROR HANDLING
    //-----------------------------
    if ( error )
        QMessageBox::information( 0, tr("Report error"), err );
    else {
        if ( rbPreview->isChecked() ) {
            //-----------------------------
            // PRINT PREVIEW
            //-----------------------------
            NCReportPreviewWindow *pv = new NCReportPreviewWindow();
            pv->setOutput( (NCReportPreviewOutput*)(report->output()) );
            pv->setWindowModality(Qt::ApplicationModal );
            pv->setAttribute( Qt::WA_DeleteOnClose );
            pv->show();
        }
    }

    //delete report;

}

void NCRTestForm::setReportFile(const QString & fname)
{
    leFile->setText( fname );
}

void NCRTestForm::showReportProgressDialog()
{
    progress = new QProgressDialog( this );
    progress->setLabelText( tr("Running report...") );
    progress->setWindowModality(Qt::WindowModal);
    connect( report, SIGNAL(dataRowCount(int)), progress, SLOT(setMaximum(int)) );
    connect( report, SIGNAL(dataRowProgress(int)), progress, SLOT(setValue(int)) );
    connect( report, SIGNAL(pageProgress(QString)), progress, SLOT(setLabelText(QString)) );
    connect( progress, SIGNAL(canceled()), report, SLOT(cancel()) );
}

void NCRTestForm::hideReportProgressDialog()
{
    delete progress;
}

