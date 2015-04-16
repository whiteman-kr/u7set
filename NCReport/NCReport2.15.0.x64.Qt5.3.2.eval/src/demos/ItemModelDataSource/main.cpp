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
#include <QStandardItemModel>
#include <QStandardItem>

int main(int argc, char *argv[] )
{
	QApplication app(argc, argv);
	
	NCReport report;

	//-------------------------------------------
	// TEST ITEM MODEL DATASOURCE
	//-------------------------------------------
	QStandardItemModel *model = new QStandardItemModel( 6, 4 );
	QStandardItem *item =0;
	
	// -----------------------------------	
	item = new QStandardItem();
	item->setData( 1, Qt::EditRole );
	model->setItem( 0, 0, item);
	
	item = new QStandardItem();
	item->setData( "Chai", Qt::EditRole );
	model->setItem( 0, 1, item);
	
	item = new QStandardItem();
	item->setData( 16.0, Qt::EditRole );
	model->setItem( 0, 2, item);
	
	item = new QStandardItem();
	item->setData( 1540.0, Qt::EditRole );
	model->setItem( 0, 3, item);

	// -----------------------------------	
	item = new QStandardItem();
	item->setData( 2, Qt::EditRole );
	model->setItem( 1, 0, item);
	
	item = new QStandardItem();
	item->setData( "Chef Anton's Cajun Seasoning", Qt::EditRole );
	model->setItem( 1, 1, item);
	
	item = new QStandardItem();
	item->setData( 20.0, Qt::EditRole );
	model->setItem( 1, 2, item);
	
	item = new QStandardItem();
	item->setData( 1230.0, Qt::EditRole );
	model->setItem( 1, 3, item);
	
	// -----------------------------------	
	item = new QStandardItem();
	item->setData( 3, Qt::EditRole );
	model->setItem( 2, 0, item);
	
	item = new QStandardItem();
	item->setData( "Grandma's Boysenberry Spread", Qt::EditRole );
	model->setItem( 2, 1, item);
	
	item = new QStandardItem();
	item->setData( 21.0, Qt::EditRole );
	model->setItem( 2, 2, item);
	
	item = new QStandardItem();
	item->setData( 520.2, Qt::EditRole );
	model->setItem( 2, 3, item);
	
	// -----------------------------------	
	item = new QStandardItem();
	item->setData( 4, Qt::EditRole );
	model->setItem( 3, 0, item);
	
	item = new QStandardItem();
	item->setData( "Uncle Bob's Organic Dried Pears", Qt::EditRole );
	model->setItem( 3, 1, item);
	
	item = new QStandardItem();
	item->setData( 25.6, Qt::EditRole );
	model->setItem( 3, 2, item);
	
	item = new QStandardItem();
	item->setData( 593.0, Qt::EditRole );
	model->setItem( 3, 3, item);
	
	// -----------------------------------	
	item = new QStandardItem();
	item->setData( 5, Qt::EditRole );
	model->setItem( 4, 0, item);
	
	item = new QStandardItem();
	item->setData( "Mishi Kobe Niku", Qt::EditRole );
	model->setItem( 4, 1, item);
	
	item = new QStandardItem();
	item->setData( 72.0, Qt::EditRole );
	model->setItem( 4, 2, item);
	
	item = new QStandardItem();
	item->setData( 130.0, Qt::EditRole );
	model->setItem( 4, 3, item);
		
	// -----------------------------------	
	item = new QStandardItem();
	item->setData( 6, Qt::EditRole );
	model->setItem( 5, 0, item);
	
	item = new QStandardItem();
	item->setData( "Queso Manchego La Pastora", Qt::EditRole );
	model->setItem( 5, 1, item);
	
	item = new QStandardItem();
	item->setData( 32.0, Qt::EditRole );
	model->setItem( 5, 2, item);
	
	item = new QStandardItem();
	item->setData( 985.5, Qt::EditRole );
	model->setItem( 5, 3, item);
	
	// -----------------------------------	
	item = new QStandardItem();
	item->setData( 7, Qt::EditRole );
	model->setItem( 6, 0, item);
	
	item = new QStandardItem();
	item->setData( "Genen Shouyu", Qt::EditRole );
	model->setItem( 6, 1, item);
	
	item = new QStandardItem();
	item->setData( 14.2, Qt::EditRole );
	model->setItem( 6, 2, item);
	
	item = new QStandardItem();
	item->setData( 1005.0, Qt::EditRole );
	model->setItem( 6, 3, item);
	
	report.addItemModel( model, "model1" );
	
	NCRTestForm window;
	window.setReport( &report );
	window.setReportFile( "../reports/itemmodel_demo.xml");

    window.show();

    return app.exec();
}
