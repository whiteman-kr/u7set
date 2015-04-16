/****************************************************************************
*
*  Copyright (C) 2002-2012 Helta Kft. / NociSoft Software Solutions
*  All rights reserved.
*  Author: Norbert Szabo
*  E-mail: norbert@nocisoft.com, office@nocisoft.com
*  Web: www.nocisoft.com
*
*  This file is part of the NCReport Report Generator System
*
*  Licensees holding a valid NCReport License Agreement may use this
*  file in accordance with the rights, responsibilities, and obligations
*  contained therein. Please consult your licensing agreement or contact
*  norbert@nocisoft.com if any conditions of this licensing are not clear
*  to you.
*
*  This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
*  WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*
****************************************************************************/
#ifndef NCREPORTIMAGEOUTPUT_H
#define NCREPORTIMAGEOUTPUT_H

#include "ncreportoutput.h"

#include <QImage>
#include <QList>
#include <QByteArray>

QT_BEGIN_NAMESPACE
class QImage;
class QBuffer;
QT_END_NAMESPACE

/*!
SVG Output class. Result pages are stored in separated .svg files
*/
class NCREPORTSHARED_EXPORT NCReportImageOutput : public NCReportOutput
{
	Q_OBJECT
public:
	explicit NCReportImageOutput( QObject* parent=0 );
	~NCReportImageOutput();

	void setImageFormat( const char* format );
	const QImage& page( int ) const;
	void setMaximumHeight(int height);
	int maximumHeight() const;
	void setFormat( QImage::Format format );
	QImage::Format format() const;
	const QImage& successiveResult() const;

	bool setup();
	void cleanup();
	void begin();
	void end();
	void newPage();

private:
	QString m_imageFormat;
	QImage::Format m_format;
	int m_maxHeight;
	QSize imageSize;
	QList<QImage> m_pages;
	QImage m_result;
};


#endif // NCREPORTIMAGEOUTPUT_H
