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
#ifndef NCREPORTGROUP_H
#define NCREPORTGROUP_H

#include "ncreport_global.h"

#include <QString>

class NCReportSection;

/*!
This class represents the a group handler of the report engine. Managed by NCReportDirector
 */
class NCREPORTSHARED_EXPORT NCReportGroup
{
    friend class NCReportXMLDefReader;
    friend class NCReportDef;
public:
    NCReportGroup();
    ~NCReportGroup();

    enum GroupState { Closed=0, OnProcess };
    enum HeaderState { On=0, Repeated, Off };

    struct GroupMetaInfo {
        GroupMetaInfo() : firstRow(-1), lastRow(-1)
        {}
        int firstRow;
        int lastRow;
        QString firstValue;
        QString lastValue;
    };

    void setId( const QString& id );
    void setValue( const QString& value);
    void setGroupExp( const QString& exp);
    void setResetVarList( const QString& varList);
    void setReprintHeaderOnNextPage( bool set );
    void setStartsOnNewPage( bool set);
    void setPageBreakExp( const QString& expression );
    /*! Flag for saving first page/initial print */
    void setInitialPrintDone( bool set );
    /*! Sets the group status */
    void setGroupChanged( bool set);
    bool isGroupChanged();
    /*! flag is useful for avoid duplicated prints*/
    void setPrintLock(bool set);
    bool printLock();
    void setNestedLevel( short level );
    short nestedLevel() const;

    NCReportSection *header();
    NCReportSection *footer();
    NCReportSection *detail();

    const QString& resetVarList() const;
    const QString& expression() const;
    const QString& pageBreakExp() const;
    bool repeatHeaderOnNextPage() const;
    const QString& groupId() const;
    bool startsOnNewPage() const;
    bool initialPrintDone() const;
    const QString& value() const;
    void reset();
    HeaderState headerState() const;
    void setHeaderState( HeaderState state );

    const QString& currentValue() const;

    const GroupMetaInfo& currentMetaInfo() const;
    const GroupMetaInfo& lastMetaInfo() const;

    int counter() const;

private:
    NCReportSection *m_header;
    NCReportSection *m_footer;
    NCReportSection *m_detail;
    GroupState m_state;
    HeaderState m_headerState;
    bool m_enabled;	// is grouping enabled:
    bool m_groupchanged;
    bool m_startsOnNewPage;
    bool m_repeatHeader;
    bool m_initialPrintDone;
    bool m_printLock; // flag for avoid duplicated prints
    short m_nestedLevel;	// nested level = order index
    int m_counter;

    QString m_id;
    QString m_groupExp;	// data source column or expression
    QString m_currentValue;
    QString m_resetVariables;		//list of variables which have to be reset at the end of group. (delimited with "," comma)
    QString m_pageBreakExp;
    GroupMetaInfo m_currentMetaInfo;
    GroupMetaInfo m_lastMetaInfo;
};

#endif
