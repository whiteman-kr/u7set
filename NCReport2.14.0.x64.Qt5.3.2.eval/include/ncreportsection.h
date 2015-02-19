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
#ifndef NCREPORTSECTION_H
#define NCREPORTSECTION_H

#include <QGraphicsScene>

class NCReportGroup;
class NCReportItem;
class NCReportDef;
class NCReportDataSource;
class NCReportDataSourceRelation;

QT_BEGIN_NAMESPACE
class QGraphicsSceneContextMenuEvent;
QT_END_NAMESPACE

/*!
Report section scene. This is the basic section scene in which the report sections based on.
*/
class NCReportSection : public QGraphicsScene
{
    Q_OBJECT
public:
    NCReportSection( QObject* parent );
    virtual ~NCReportSection();

    enum SectionType { PageHeader=0, PageFooter, GroupHeader, GroupFooter, Detail,
        ReportHeader, ReportFooter, Custom };
    enum SpaceCheckRole { SectionOnly=0, SectionPlusDetail, SectionPlusOrphanedTolerance };

    //NCReportSectionData* data();

    inline bool gridVisible() const   { return m_gridVisible; }
    void setGridVisible(bool visible) { m_gridVisible = visible; }

    inline bool snapX() const     { return m_snapX; }
    void setSnapX(bool snap)      { m_snapX = snap; }

    inline bool snapY() const     { return m_snapY; }
    void setSnapY(bool snap)      { m_snapY = snap; }

    inline int deltaX() const     { return m_deltaX; }
    void setDeltaX(int dx)        { m_deltaX = dx; }

    inline int deltaY() const     { return m_deltaY; }
    void setDeltaY(int dy)        { m_deltaY = dy; }

    QPointF snapToPoint(const QPointF &p) const;
    qreal snapToX(const qreal) const;
    qreal snapToY(const qreal) const;
    int nextZValue();
    void resetZValue();
    /*! Adds report item to scene and sets the specified Z-order. Virtual function for differeces between engine and designer */
    virtual void addItemZ( NCReportItem* );
    /*! Adds report item to scene for report engine only.*/
    virtual void addItemForEngine( NCReportItem* );

    /*! Virtual function for emiting changes when used in editor*/
    virtual void emitContentChanged();
    virtual void emitItemChanged( NCReportItem* );
    //void setVirtualMargin( qreal left, qreal right, qreal top, qreal bottom );
    //QPointF mapToMargin( const QPointF & point ) const;
    QList<NCReportItem*>& reportItems() { return m_itemList; }

    inline bool isFinished() const { return m_finish; }
    void setFinished( bool set ) { m_finish = set; }

    inline qreal metricHeight() const { return m_heightMM; }
    void setMetricHeight( qreal mm ) { m_heightMM = mm; }

    inline SectionType type() const { return m_type; }
    void setType( SectionType value ) { m_type = value; }

    inline SpaceCheckRole checkRole() const { return m_checkRole; }
    void setCheckRole( SpaceCheckRole value ) { m_checkRole = value; }

    inline bool isShown() const { return m_show; }
    void setShow( bool value ) { m_show = value; }

    inline bool isAutoHeight() const { return m_autoHeight; }
    void setAutoHeight( bool value ) { m_autoHeight = value; }

    inline bool hideIfBlank() const { return m_hideIfBlank; }
    void setHideIfBlank( bool value ) { m_hideIfBlank = value; }

    inline QString id() const { return m_id; }
    void setID( const QString& value ) { m_id = value; }

    QString caption() const;

    inline NCReportGroup* relatedGroup() const { return m_group; }
    void setRelatedGroup( NCReportGroup* g ) { m_group = g; }

    //detail properties
    inline QString tagname() const { return m_tagname; }
    void setTagname( const QString& value ) { m_tagname = value; }

    inline QString dataSourceId() const { return m_datasourceID; }
    void setDataSourceId( const QString& value ) { m_datasourceID = value; }

    inline QString secondaryDatasourceID() const { return m_datasource2ID; }
    void setSecondaryDatasourceID( const QString& value ) { m_datasource2ID = value; }

    inline QList<NCReportGroup*>& groups() { return m_groups; }

    inline int layoutIndex() const { return m_layoutIndex; } // index for internal purposes for designer
    void setLayoutIndex( int value ) { m_layoutIndex = value; }

    inline QString pageBreakExp() const { return m_pageBreakExp; }
    void setPageBreakExp( const QString& value ) { m_pageBreakExp = value; }

    inline bool startsOnNewPage() const { return m_startsOnNewPage; }
    void setStartsOnNewPage( bool  value ) { m_startsOnNewPage = value; }

    inline QString printWhenExp() const { return m_printWhenExp; }
    void setPrintWhenExp( const QString& value ) { m_printWhenExp = value; }

    inline qreal orphanedTolaranceMM() const { return m_orphanedTolaranceMM; }
    void setOrphanedTolaranceMM( qreal value ) { m_orphanedTolaranceMM = value; }

    inline bool isPrintLock() const { return m_printLock; }
    void setPrintLock(bool set) { m_printLock = set; }

    inline bool isAnchorToPageFooter() const { return m_anchorToFooter; }
    void setAnchorToPageFooter(bool set) { m_anchorToFooter = set; }

    inline bool isNotAlone() const { return m_notAlone; }
    void setNotAlone(bool set) { m_notAlone = set; }

    inline QString htmlOptions() const { return m_htmlOptions; }
    void setHtmlOptions( const QString& options ) { m_htmlOptions = options; }

    NCReportDataSource* dataSource() { return m_dataSource; }
    void setDataSource( NCReportDataSource *ds ) { m_dataSource = ds; }

private:
    //qreal leftMargin, rightMargin, topMargin, bottomMargin;
    bool m_gridVisible;
    bool m_snapX;
    bool m_snapY;
    int m_deltaX;
    int m_deltaY;
    int m_zvalue;

    SectionType m_type;
    SpaceCheckRole m_checkRole;
    qreal m_heightMM;
    qreal m_orphanedTolaranceMM;
    bool m_show;
    bool m_finish;
    bool m_autoHeight;
    bool m_hideIfBlank;
    bool m_printLock;
    QString m_id;
    //QString m_caption;

    //detail properties
    QString m_tagname;
    QString m_datasourceID;
    QString m_datasource2ID;

    int m_layoutIndex;	// index for internal purpuses in designer
    QString m_pageBreakExp;
    bool m_startsOnNewPage; // if multiple details >1
    QString m_printWhenExp;
    NCReportGroup* m_group; // group is assigned to section. Valid only for group header and footer.
    bool m_anchorToFooter;
    bool m_notAlone;  // must not be printed alone at the bottom of the page.

    QString m_htmlOptions;
    NCReportDataSource *m_dataSource;

private:
    QList<NCReportItem*> m_itemList;
    QList<NCReportGroup*> m_groups;

private:
    int snapValue(int value, int grid) const;
};

#endif
