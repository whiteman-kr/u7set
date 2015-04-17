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
#include "testdatasource.h"
#include "ncreport.h"
#include "ncrtestform.h"

#include <QApplication>

int main(int argc, char *argv[] )
{
	QApplication app(argc, argv);
	
	NCReport report;
	
	//-------------------------------------------
	// TEST CUSTOM DATASOURCE
	//-------------------------------------------
	TestDataSource *ds = new TestDataSource();
	ds->setID("cds0");
	
	TestData d1;
	d1.id = 123;
	d1.name = "Alexander Henry";
	d1.address = "HOT SPRINGS VILLAGE, AR";
	d1.valid = true;
	d1.date = QDate(2008,01,10);
	ds->addData( d1 );
	
	TestData d2;
	d2.id = 157;
	d2.name = "Julius Coleman";
	d2.address = "Coronado, CA";
	d2.valid = false;
	d2.date = QDate(2008,01,12);
	ds->addData( d2 );

	TestData d3;
	d3.id = 157;
	d3.name = "Peter Moulding";
	d3.address = "San francisco, CA";
	d3.valid = true;
	d3.date = QDate(2008,01,07);
	ds->addData( d3 );
	
	report.addCustomDataSource( ds );
	
	NCRTestForm window;
	window.setReport( &report );
	window.setReportFile( "../reports/customdatasource_demo.xml");
    window.show();
    return app.exec();
}
