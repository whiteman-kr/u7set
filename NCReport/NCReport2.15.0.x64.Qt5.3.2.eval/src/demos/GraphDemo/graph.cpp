/****************************************************************************
* 
*  Copyright (C) 2002-2011 Helta Kft. / NociSoft Software Solutions
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
#include "graph.h"
#include "ncreportoutput.h"

#include <QPainter>
#include <QColor>

Graph::Graph()
{
}

Graph::~ Graph()
{
}

void Graph::paintItem(QPainter * painter, NCReportOutput * output, const QRectF & rect, const QString & itemdata)
{
	Q_UNUSED(output)
	QStringList values = itemdata.split(",");

	const int numcols = values.count();
	const int cw = qRound(rect.width()/numcols);
	painter->setPen( Qt::NoPen );
	int ch=0;
	QColor color;
	for ( int i=0; i<numcols; ++i ) {
		if ( i%3 == 0 ) 
			color.setRgb(0xEC8125);
		else if ( i%2 == 0 )
			color.setRgb(0x2573EC);
		else
			color.setRgb(0x576273);
		
		ch = qRound(rect.height()*values.at(i).toInt()/100);
		painter->setBrush( QBrush(color) );
		painter->drawRect( rect.x()+i*cw, rect.y()+rect.height()-ch , cw, ch );
	}
	
	painter->setPen( QPen(Qt::black) );
	painter->setBrush( Qt::NoBrush );
	
	//painter->drawRect( rect );
	
	painter->setFont( QFont("Arial",8) );
	painter->drawText( rect, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextWordWrap, QString("GRAPH EXAMPLE: %1").arg(itemdata) );
	
}




