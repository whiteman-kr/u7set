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
#include "ncreportdemo.h"
#include "ncreport.h"
#include "ncreportoutput.h"
#include "ncreportpreviewoutput.h"
#include "ncreportpreviewwindow.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QMessageBox>

#define DB_DRIVER		"QMYSQL"
#define DB_DATABASE		"northwind"
#define DB_USER			"northuser"
#define DB_PASSWORD		"northwind"


NCReportDemo::NCReportDemo()
{
	mReport = new NCReport();	// create report obj
}

NCReportDemo::~NCReportDemo()
{
	delete mReport;	// delete report obj
}

bool NCReportDemo::runReport()
{
   if ( !connectDB("northwind") )	// connect SQL database using connection id: northwind
		return false;

	mReport->reset();  // reset report 
    mReport->setReportSource( NCReportSource::File ); // set report source type
	mReport->setReportFile("../reports/simple_productlist_demo.xml"); //set the report filename fullpath or relative to dir
    mReport->runReportToPreview(); // run to preview output

    if ( mReport->hasError()) {
        QMessageBox::information( 0, "Report error", mReport->lastErrorMsg());
		return false;
    } else {

        NCReportPreviewWindow *pv = new NCReportPreviewWindow();	// create preview window
        pv->setOutput( (NCReportPreviewOutput*)mReport->output() );  // add output to the window
        pv->setWindowModality(Qt::ApplicationModal );	// set modality
        pv->setAttribute( Qt::WA_DeleteOnClose );	// set attrib
        pv->show();	// show
    }

	return true;
}


bool NCReportDemo::connectDB( const QString& id )
{
    QSqlDatabase db = QSqlDatabase::addDatabase(DB_DRIVER, id );
    if ( !db.isValid() ) {
        QMessageBox::warning( 0, "Database error", QObject::tr("Could not load database driver.") );
		return false;
    }
    db.setDatabaseName(DB_DATABASE);
    db.setUserName(DB_USER);
    db.setPassword(DB_PASSWORD);
   
    if ( !db.open() ) {
        QMessageBox::warning( 0, "NCReport error", QObject::tr("Cannot open database: ")+db.lastError().databaseText() );
		return false;
    }
   
	return true;
}

