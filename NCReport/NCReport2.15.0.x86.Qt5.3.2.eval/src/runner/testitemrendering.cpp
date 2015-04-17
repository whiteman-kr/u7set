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
#include "testitemrendering.h"
#include "ncreportoutput.h"

#include <QPainter>
#include <QColor>

TestItemRendering::TestItemRendering()
{
}

TestItemRendering::~ TestItemRendering()
{
}

void TestItemRendering::paintItem(QPainter * painter, NCReportOutput * output, const QRectF & rect, const QString & itemdata)
{
	
	switch ( output->output() ) {
		case NCReportOutput::Printer:
		case NCReportOutput::Pdf:
		case NCReportOutput::Preview:
			break;
		default:
			return;
	}
	
	const int numcols = 10;
	const int cw = qRound(rect.width()/numcols);
	painter->setPen( Qt::NoPen );
	int ch=0;
	QColor color;
	color.setAlpha( 128 );
	for ( int i=0; i<numcols; ++i ) {
		if ( i%3 == 0 ) {
			color.setRgb(0xAAAAFF);
			ch = qRound(rect.height()*0.8);
		} else if ( i%2 == 0 ) {
			color.setRgb(0xAAFFAA);
			ch = qRound(rect.height()*0.4);
		} else {
			color.setRgb(0xFFAAAA);
			ch = qRound(rect.height()*0.6);
		}
		painter->setBrush( QBrush(color) );
		painter->drawRect( rect.x()+i*cw, rect.y()+qRound(rect.height())-ch , cw, ch );
	}
	
	painter->setPen( QPen(Qt::black) );
	painter->setBrush( Qt::NoBrush );
	
	painter->drawRect( rect );
	
	painter->setFont( QFont("Arial",8) );
	painter->drawText( rect, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap, QString("GRAPH EXAMPLE: %1").arg(itemdata) );
	
}




