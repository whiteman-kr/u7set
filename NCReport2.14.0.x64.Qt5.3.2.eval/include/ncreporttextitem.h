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
#ifndef NCREPORTTEXTITEM_H
#define NCREPORTTEXTITEM_H

#include "ncreportitem.h"

#include <QString>
#include <QColor>
#include <QFont>

#define NCRD_DEFAULT_TEXT 	"Html text"
QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE
/*!
Html/Rich text item's data class
 */
class NCReportTextData : public NCReportItemData
{
public:
    NCReportTextData() : document(0), requiredHeightMM(0), useFont(true), defaultPosAndSize(false), sizeFix(false), plainText(false), markdown(false),
        forecolor(Qt::black)
    {}

    QTextDocument *document;
    qreal requiredHeightMM;		// required printable height in MM
    bool useFont;
    bool defaultPosAndSize;	// default pos and size for if text document print mode
    bool sizeFix;	// fix the document's height
    bool plainText;		// non html plain text
    bool markdown;  // markDown feature
    QColor forecolor;
    QString filename;
    QString documentTemplate;	// document template for "mixed" source
    QFont font;
};

/*!
Html/Rich text item class
 */
class NCReportTextItem : public NCReportItem
{
public:
    NCReportTextItem( NCReportDef* rdef, QGraphicsItem * parent=0 );
    ~NCReportTextItem();

    bool read( NCReportXMLReader* );
    bool write( NCReportXMLWriter* );
    void setHtml( const QString& );
    QString toHtml() const;
    void setDefaultForEditor();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void paint( NCReportOutput*, const QPointF& );
    void dummyPaint(NCReportOutput *, const QPointF &);
    void adjustSize();
    void adjustSize(NCReportOutput* =0);
    void resize( const QSizeF& );
    bool isEmpty() const;
    void updateValue(NCReportOutput* output, NCReportEvaluator* evaluator);

    inline QTextDocument * document()
    { return ((NCReportTextData*)d)->document; }

    inline bool isDefaultPosAndSize() const
    { return ((NCReportTextData*)d)->defaultPosAndSize; }

    inline void setDefaultPosAndSize( bool set )
    { ((NCReportTextData*)d)->defaultPosAndSize = set; }

    inline QColor foreColor() const
    { return ((NCReportTextData*)d)->forecolor; }

    inline void setForeColor( const QColor& color )
    { ((NCReportTextData*)d)->forecolor = color; }

    inline QFont font() const
    { return ((NCReportTextData*)d)->font; }

    inline void setFont( const QFont& font )
    { ((NCReportTextData*)d)->font = font; }

    inline QString fileName() const
    { return ((NCReportTextData*)d)->filename; }

    inline void setFileName( const QString& fname )
    { ((NCReportTextData*)d)->filename = fname; }

    inline bool isSizeFix() const
    { return ((NCReportTextData*)d)->sizeFix; }

    inline void setSizeFix( bool set )
    { ((NCReportTextData*)d)->sizeFix = set; }

    inline bool isUsedDefaultFont() const
    { return ((NCReportTextData*)d)->useFont; }

    inline void setPlainText( bool set )
    { ((NCReportTextData*)d)->plainText = set; }
    inline bool isPlainText() const
    { return ((NCReportTextData*)d)->plainText; }

    inline void setUseDefaultFont( bool set )
    { ((NCReportTextData*)d)->useFont = set; }

    inline QString documentTemplate() const
    { return ((NCReportTextData*)d)->documentTemplate; }

    inline void setDocumentTemplate( const QString& dt )
    { ((NCReportTextData*)d)->documentTemplate = dt; }

    inline bool isMarkdown() const
    { return ((NCReportTextData*)d)->markdown; }

    inline void setMarkdown( bool set )
    { ((NCReportTextData*)d)->markdown = set; }

    qreal breakTo( qreal heightMM, NCReportOutput* );
    bool isBroken() const;
    QString convertPlainTextToHtml( const QString& text) const;
    QList<int> availableSourceTypes() const;
    /*!
     * \brief htmlContent
     * \return
     * Raw HTML text content
     */
    QString htmlContent() const;

protected:
    void readProps( NCReportXMLReader* );
    void writeProps( NCReportXMLWriter* );
    /*! Font adjustment for MAC users */
    virtual void adjustFont( NCReportOutput* =0 );

private:

    struct TextPart {
        TextPart() : blockNo(-1), lineNo(-1) {}
        int blockNo;
        int lineNo;
    };

    TextPart partFrom, partTo;
    void partialPaint(NCReportOutput *o, QPointF &pos, bool dummyPaint );
};

#endif
