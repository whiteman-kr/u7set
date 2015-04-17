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
#ifndef NCREPORTDIRECTOR_H
#define NCREPORTDIRECTOR_H

#include <QObject>
#include <QStack>
#include <QQueue>
#include <QTime>
#include <QPointF>
#include <QTextStream>

#include "ncreportdatasource.h"
#include "ncreportvariable.h"

class NCReport;
class NCReportDef;
class NCReportSection;
class NCReportEvaluator;
//class NCReportDataSource;
class NCReportGroup;
class NCReportItem;
class NCReportSectionDirector;
class QTextDocument;
class NCReportDataSourceRelation;

/*!
This class is the internal engine of NCReport. The NCReportDirector manages the whole report running process.\n
Responsible for running datasources and rendering report.
 */
class NCReportDirector : public QObject
{
    Q_OBJECT
public:
    friend class NCReportSectionDirector;
    enum ProcessState { Initial=0, Finished, PageBegin, PageOnProcess, PageEnd, GroupHeader, GroupFooter,
                        PageHeader, PageFooter, ReportHeader, ReportFooter, Detail, Unknown };
    enum PageStatus { Printable=0, Hidden };
    enum PassState { Off=0, TestPass, RealPass };
    enum VariableResetMode { All=0, CurrentDataSourceOnly };
    enum Flags { FlagFinished=0, FlagRHeader, FlagRFooter, FlagNoSpaceInPage, FlagBeginReport, FlagNoUpdate, FlagLastRecord };

    NCReportDirector( NCReport* parent );
    ~NCReportDirector();

    void setReportDef( NCReportDef* );
    void setForceCopies( int );
    void setCurrentForceCopy( int );
    void setReportNumber( int );
    void setReportCount( int );
    bool openDataSource( NCReportDataSource* ds );

    bool reportProcess();
    bool reportSectionProcess(bool doPageBegin, bool doPageEnd);
    int pageNum() const;
    int globalPageNum() const;
    int pageCount() const;
    int reportNumber() const;
    int reportCount() const;
    int numForceCopies() const;
    int currentForceCopy() const;
    int currentRow() const;
    int numRows() const;
    NCReportEvaluator* evaluator();
    NCReportSection* currentDetail();
    NCReportDataSource *currentDataSource();
    PageStatus pageStatus( int page ) const;
    PageStatus currentPageStatus() const;
    bool flag( Flags ) const;
    bool nextRecord(NCReportDataSource* ds = 0);
    bool previousRecord(NCReportDataSource* ds = 0);
    bool firstRecord(NCReportDataSource* ds = 0);
    bool lastRecord(NCReportDataSource* ds = 0);

    void addSlicedItem( NCReportItem* );
    void setPageBreakStop();
    QPointF paintPosMM() const;
    //QPointF& rPaintPosMM();
    void translatePaintPosY( qreal byMM );
    void setPaintPosY( qreal yMM );
    void setPaintPosX( qreal xMM );
    void anchorBottomPaintPosY(NCReportSection* section);
    /*! Breaks the current page and begins a new one*/
    void pageBreak( bool trimRecord, bool doRepeatGroupHeaders );
    /*! Returns available space to bottom in the current page in mm.*/
    qreal spaceToBottom() const;

    void resetPageNum();
    void resetGlobalPageNum();
    void nextPageNum();

    /*!
     * \brief The SectionInfo struct
     * The descriptor of two pass report process to store section information of first pass.
     * This is neccessary for protection to avoid lonely headers.
     */
    struct SectionInfo {
        /*!
         * \brief number
         * The number of rendered section
         */
        int index;
        /*!
         * \brief sectionHeightMM
         *  The calculated height of the current section.
         */
        qreal sectionHeightMM;
        /*!
         * \brief linked
         * This is a flag shows the section is linked to another, for example a detail to its group header
         * for protection to avoid lonely headers.
         */
        bool linked;
        /*!
         * \brief startedOnNewPage
         * This flag is set true if the section started on a new page when running the first (test) pass
         */
        bool startedOnNewPage;
        /*!
         * \brief lastSectionOfThePage
         *  Logical variable returns true if the section is the last one on the current page.
         * The section cannot be a page footer.
         */
        bool lastSectionOfThePage;
        /*!
         * \brief section
         * The current section pointer
         */
        NCReportSection *section;
    };

    QList<SectionInfo>& sectionInfoList();
    PassState currentPassState() const;
    ProcessState currentProcessState() const;
    int& sectionCounter();

    void setProcessEvents( bool enable );
    bool isProcessEvents() const;
    void finish();
    bool isFinished() const;

signals:
    void dataSourceOpen( const QString& dsID );
    void dataSourceOpened( const QString& dsID, int );
    void dataRowProgress( const QString& dsID, int );
    void dataRowProgress( int row );
    void dataRowCount( int numRows );
    void sectionProgress( const QString& sectionID );
    void pageProgress( int );

public slots:
    /*!Cancels report process*/
    void cancel();

private:
    NCReport *m_report;
    NCReportDef *m_reportDef;
    NCReportEvaluator *m_evaluator;
    ProcessState m_state;
    PageStatus m_pageStatus;
    PassState m_passState;
    /*!
     * \brief mPaintPos
     * painter positions in mm
     */
    QPointF m_paintPosMM;
    int m_pageNo;
    int m_globalPageNo;
    int m_pageCount;
    int m_globalPageCount;
    int m_reportNo;
    int m_reportCount;
    int m_currentDetailId; // current processing detail
    int m_forceCopies;
    int m_currentForceCopy;
    int m_currentRow;
    NCReportSection* m_lastRenderedSection;	// save the last printed section
    QTime m_runtime;
    bool m_pageBreakStop;
    bool m_pageBreakIsOnProcess;
    int m_variableCorrectionCallerID;

    NCReportDataSource* m_subReportDS;
    NCReportDataSourceRelation *m_dataSourceRelation;
    int m_sectionCounter;
    bool m_processEvents;
    bool m_allowNewPage;

    bool m_flags[7];
    QStack<NCReportGroup*> inside;
    QStack<NCReportGroup*> outside;
    QQueue<NCReportItem*> m_slicedItems;

    QList<SectionInfo> m_sectionInfoList;

private:
    void setFlag( Flags, bool );
    bool openDataSources();
    bool openDataSources( NCReportDataSource::OpenRoles role );
    bool closeDataSources( bool all, NCReportDataSource::OpenRoles role = NCReportDataSource::BeginReport );
    //QStringList childDataSources( NCReportDataSource* parent ) const;

    void pageBegin();
    void pageEnd();
    void renderReportHeader();
    void renderReportFooter();
    bool renderDetailsAndGroups();
    bool renderRepeatedGroupHeaders();
    void setGroupHeaderPrintLocks( bool enable );

    /*!Enters to the next group level and handles it*/
    bool groupIn();
    /*!Leaves from the group level and handles it*/
    bool groupOut( bool final = false );
    /*! Renders the content of a section*/
    bool renderSection( NCReportSection* );

    void updateVariables();
    void updateDynamicItems();
    void updateSectionDynamicItems( NCReportSection * );
    void updateGroups( NCReportSection *d );
    void resetGroup( NCReportGroup* );
    void reprintGroupProtection( bool enable, NCReportSection* =0 );
    void initGroupStacks();

    /*!
     Reset all variables by reset mode
    */
    void resetVariables( VariableResetMode mode );
    /*!
     * Resets the variables
     * \brief resetVariablesByScope
     * \param scope resets the variables with this scope only
     */
    void resetVariablesByScope( NCReportVariable::ResetScope scope );

    //bool printTextDocument_obsolete();
    bool documentReportProcess();
    void documentReportProcess_Items( NCReportSection* detail, QTextStream& inputHtml,
                                      qreal& documentLeftMarginMM, QSizeF& documentPageSizeMM, bool& sizeAndPosIsModified,
                                      QTextDocument* document, NCReportDataSource* ds );

    /*! returns the width of the section */
    qreal sectionWidthMM( NCReportSection* ) const;
    /*! returns true if there are more detail(s) after current */
    bool isNextDetail() const;
    bool updateGroupDataSources( const QString& groupID ) const;

    //void restoreFields( NCReportSection* section, uint mode );
    void benchmark( const QString& msg );
    void paintUnregText();
    bool activateNextDetail();
    void fatalError( const QString& msg );
    bool pageBreakIsAllowed( NCReportGroup* );
    void variableCorrections(bool enable, int callerID );
    void registerLastPrintedSection();

    int progressRowCount(NCReportDataSource* ds) const;
    int progressCurrentRow(NCReportDataSource *ds) const;
    bool updateDataSource( NCReportDataSource* ds ) const;

protected:
    virtual void beforePageBreak();
    virtual void afterPageBreak();
};

#endif
