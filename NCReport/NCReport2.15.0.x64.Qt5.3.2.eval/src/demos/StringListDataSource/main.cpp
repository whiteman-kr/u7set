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

#include <QApplication>

int main(int argc, char *argv[] )
{
	QApplication app(argc, argv);
	
	NCReport report;

	//-------------------------------------------
	// TEST STRINGLIST DATASOURCE
	//-------------------------------------------
	QStringList list;
	list << "24\tRenate Moulding\tDesert Hot Springs,CA\t1\t2008-01-01";
	list << "78\tAlfred Muller\tMiami Beach, FL\t1\t2008-01-03";
	list << "140\tAngela Merkel\tMunchen, Germany\t1\t2008-01-07";
	list << "139\tBob Larson\tDallas, TX\t0\t2008-01-20";
	
	report.addStringList( list, "sl0" );

	QStringList list2;
	list2 << "0000000014|Abkrzung: 005|Ein -->>|Zeitgruppe|<<-- Karenzzeit|";
	list2 << "0000000015|Abkrzung: 010|Zwei -->>|Zeitgruppe|2254112|";
	list2 << "0000000016|Abkrzung: 015|Drei -->>|Zeitgruppe|6998714|";
	
	report.addStringList( list2, "sl1" );


	NCRTestForm window;
	window.setReport( &report );
	window.setReportFile( "../reports/stringlist_demo.xml");

    window.show();

    return app.exec();
}
