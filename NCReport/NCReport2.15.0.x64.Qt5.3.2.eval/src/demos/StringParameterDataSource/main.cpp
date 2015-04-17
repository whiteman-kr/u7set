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
	// TEST STRING PARAMETER DATASOURCE
	//-------------------------------------------
	// 0. column: id
	// 1. column: product name
	// 2. column: price
	// 3. column: group1 value
	// 4. column: value
	// 5. column: group0 value
	
	
	QString data;
	
	data += "1 \tChai                            \t16.0000\t1\t1540\t0\n";
	data += "2 \tChang                           \t17.0000\t1\t 874\t0\n";   
	data += "3 \tAniseed Syrup                   \t 9.0000\t1\t1687\t0\n";   
	data += "4 \tChef Anton's Cajun Seasoning    \t20.0000\t1\t1230\t0\n";   
	data += "5 \tChef Anton's Gumbo Mixj         \t19.0000\t2\t1900\t0\n";   
	data += "6 \tGrandma's Boysenberry Spread    \t21.0000\t2\t 520\t0\n";   
	data += "7 \tUncle Bob's Organic Dried Pears \t25.0000\t3\t 540\t0\n";   
	data += "8 \tNorthwoods Cranberry Sauce      \t34.0000\t3\t 120\t0\n";   
	data += "9 \tMishi Kobe Niku                 \t72.0000\t3\t 130\t0\n";   
	data += "10 \tIkura                          \t26.0000\t3\t2247\t0\n";   
	data += "11 \tQueso Cabrales                 \t19.0000\t4\t 741\t0\n";   
	data += "12 \tQueso Manchego La Pastora      \t32.0000\t4\t 512\t0\n";   
	data += "13 \tKonbu                          \t 5.0000\t4\t1470\t0\n";   
	data += "14 \tTofu                           \t21.0000\t4\t 978\t0\n";   
	data += "15 \tGenen Shouyu                   \t14.0000\t4\t1005\t0\n";   
			
	
	report.addParameter( "data1", data );
	

	NCRTestForm window;
	window.setReport( &report );
	window.setReportFile( "../reports/stringparameter_demo.xml");

    window.show();

    return app.exec();
}

