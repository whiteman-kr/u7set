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
#ifndef NCREPORTPREVIEWPAGECONTAINER_H
#define NCREPORTPREVIEWPAGECONTAINER_H

#include <QWidget>
#include <QPoint>

#include "ncreport_global.h"

//#define MAX_PAGES	4

class NCReportPreviewPage;
class NCReportPreviewOutput;

/*!
Container widget class for storing the current report preview page(s) in a QScrollWidget
*/
class NCREPORTSHARED_EXPORT NCReportPreviewPageContainer : public QWidget
{
	Q_OBJECT
public:
    NCReportPreviewPageContainer( QWidget * parent );
	~NCReportPreviewPageContainer();
	
	enum ShowType { Single=0, Double, Continous };
	
	void setOutput(NCReportPreviewOutput*);
	void setShowType( const ShowType );
	void setMargins( int left, int right, int top, int bottom );
	void setAllPageSize( const QSizeF& mmsize );
	void loadPage( int pageno );
	void scale(qreal factor);
	qreal currentScaleFactor() const;
	ShowType showType() const;
	NCReportPreviewPage* page(int) const;
	int pagePosition( int pageno ) const;
	int numPagesOnScreen() const;

protected:
    //void paintEvent( QPaintEvent* );

private:
	int m_leftMargin, m_rightMargin, m_topMargin, m_bottomMargin;
	int m_gap;
	ShowType m_showType;
	//int m_numPagesOnScreen;
	qreal m_currentscalefactor;
	QList<NCReportPreviewPage*> m_pages;
	NCReportPreviewOutput *m_output;
	//NCReportPreviewPage* pages[MAX_PAGES];

	void updateSizeAndPos();
	void updatePages();
	void createPages( bool doUpdateSize = true);
    qreal maxPageWidth() const;
};

#endif
