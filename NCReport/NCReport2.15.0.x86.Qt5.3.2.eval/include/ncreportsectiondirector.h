/****************************************************************************
*
*  Copyright (C) 2002-2014 Helta Kft. / NociSoft Software Solutions
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
#ifndef NCREPORTSECTIONDIRECTOR_H
#define NCREPORTSECTIONDIRECTOR_H

#include <QMap>
#include <QSet>
#include <QList>
#include <QPointF>
#include <QSizeF>

class NCReportDirector;
class NCReportSection;
class NCReportItem;

class NCReportSectionDirector
{
public:
    NCReportSectionDirector( NCReportDirector *director );
    ~NCReportSectionDirector();

    enum ZoneMode { Normal=0, Breakable };

    class NCReportZone
    {
        public:
        NCReportZone() : id( -1 ), startY(0), endY(0), offsetY(0), visible(false), hasEnoughSpace(false), totalHeightMM(0) {}

            int id;
            qreal startY, endY, offsetY;
            bool visible;
            bool hasEnoughSpace;
            qreal totalHeightMM;
            QList<NCReportItem*> items;

            qreal heightMM() const { return endY - startY; }
    };

    bool renderSection(NCReportSection *section);
    bool simpleRenderSection(NCReportSection *section);
    void reset();

    QSizeF autoSectionMetricSize() const;
    QSizeF staticSectionMetricSize() const;
    QSizeF zoneMetricSize( const NCReportZone& zone ) const;
    QSizeF zoneMetricSizeWithSpacing( const NCReportZone& zone ) const;

private:
    NCReportDirector *rd;
    NCReportSection *mSection;
    qreal mTotalZonesHeight;	// height of all zones
    ZoneMode zoneMode;
    bool m_pageBreakStop;

    QMap<int,NCReportZone> zones;
    QSet<NCReportItem*> hiddenItems;

    bool loadZones();
    bool checkSpace( const qreal spaceMM );
    bool checkZoneSpace( NCReportZone& );
    bool renderItems();

    bool renderZones();
    bool renderZonesNormal();
    bool renderZonesBreakable();

    bool renderZoneItems( NCReportZone& );
    bool renderZoneItemsNormal( NCReportZone& );
    bool renderZoneItemsBreakable( NCReportZone& );

    void paintItem( NCReportItem *item );
    void paintItem( NCReportItem *item, const QPointF& painterPosMM );
    void paintAlternateRowBackground(const QPointF& painterPosMM , const NCReportZone &zone = NCReportZone() );

    void setItemHidden( NCReportItem *item );
    bool itemIsHidden( NCReportItem *item );

    void increaseTotalZoneHeight(const NCReportZone&, const short factor );

    /*! Updates the height of the actual section depending on dynamic settings. If the height is changed it is written to height parameter*/
    //void updateDynamicSectionHeight( NCReportSection*, NCReportItem*, qreal& height );
    void adjustSize( NCReportItem* );
    //void afterRenderItem( NCReportItem* );
    void setPageBreakStop(bool set);
    bool isPageBreakStop() const;
    void addSectionInfo(const qreal heightMM);
    void aloneSectionProtection();

};

#endif
