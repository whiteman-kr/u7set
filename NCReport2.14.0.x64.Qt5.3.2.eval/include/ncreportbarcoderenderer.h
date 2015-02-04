/****************************************************************************
* 
*  Copyright (C) 2007-2010 Helta Kft. / NociSoft Software Solutions
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
#ifndef NCREPORTBARCODERENDERER_H
#define NCREPORTBARCODERENDERER_H

#include <QByteArray>
#include <QXmlStreamReader>
#include <QSize>
#include <QRectF>
#include <QColor>

QT_BEGIN_NAMESPACE
class QPainter;
QT_END_NAMESPACE

class NCReportBarcodeRenderer : public QXmlStreamReader
{
public:
	NCReportBarcodeRenderer( QPainter* painter, const QByteArray & data, const QRectF & rect );
	~NCReportBarcodeRenderer();

	bool render( bool showText );

private:
	QPainter* m_painter;
	QRectF m_paintRect;
	qreal m_rate;
	bool m_showText;
	bool m_error;
	//QXmlStreamReader xml;
	QSize m_svgSize;
	QString m_baseTag;
	QString m_errorMsg;

	void renderContents();
	qreal rc( const char* name ) const;
	void paintElement( QRectF& elementRect, const QColor& fillColor );
	void paintText( const QString& text, QPointF& elementPoint, const QString& fontFamily, qreal fontSize, const QColor& color );
	void paintError();

};

#endif // NCREPORTBARCODERENDERER_H
