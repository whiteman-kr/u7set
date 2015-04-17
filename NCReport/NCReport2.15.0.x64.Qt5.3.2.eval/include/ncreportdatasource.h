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
#ifndef NCREPORTDATASOURCE_H
#define NCREPORTDATASOURCE_H

#include <QObject>
#include <QStringList>
#include <QVector>
#include <QHash>
#include <QVariant>

#include "ncreport_global.h"

class NCReportError;
class NCReportXMLReader;
class NCReportXMLWriter;
class NCReportEvaluator;
class NCReportDataFieldInfo;


#define def_strMarkEditRole ":e"
#define def_strMarkUserRole ":u"
#define def_strMarkDecorationRole ":c"
#define def_strMarkDisplayRole ":d"

#define def_strMarkHeaderRole ":h"
#define def_nMarkHeaderRole -2
#define def_strMarkHeaderDecorationRole ":hc"
#define def_nMarkHeaderDecorationRole -3
#define def_strMarkHeaderUserRole ":hu"
#define def_nMarkHeaderUserRole -4
#define def_strMarkHeaderDisplayRole ":hd"
#define def_nMarkHeaderDisplayRole -5
#define def_strMarkHeaderToolTipRole ":ht"
#define def_nMarkHeaderToolTipRole -6


/*!
This abstract class represents the NCReport's data source.
 */
class NCREPORTSHARED_EXPORT NCReportDataSource : public QObject
{
    Q_OBJECT
public:
    NCReportDataSource( QObject *parent=0 );
    virtual ~NCReportDataSource();

    enum DataSourceType { Sql=0, Text, Xml, StringList, ItemModel, Custom };
    enum LocationType { Static=0, File, Http, Ftp, Parameter };
    enum ConnectionType { Internal=0, External };
    enum OpenRoles { BeginReport=0, ChildDataSource, GroupChange, SubReport };

    void setID( const QString& dsId);
    void setId(const QString& dsId);
    QString id() const;

    //! Assign a brief description for the data source.
    void setBriefDescription( const QString& description);

    //! Returns the brief description.
    QString briefDescription() const;

    //! Assign a long description for the data source.
    void setLongDescription( const QString& description);

    //! Returns the long description.
    QString longDescription() const;

    void setDataSourceType( DataSourceType dsType );
    void setConnectionType( ConnectionType cType );
    void setConnectionID( const QString& id );
    void setLocationType( LocationType ltype );
    void setFileName( const QString& fileName );
    void setEncoding( const QString& encoding );
    QString encoding() const;
    void setRecordFilter( const QString& filter );
    void setSpecText( const QString& txt );
    void setSpecification( const QString& txt );
    void setErrorHandler( NCReportError* );
    /*! Sets the parent datasource ID*/
    void setParentId( const QString& parentId );
    QString parentId() const;
    /*! Sets a parent datasource */
    void setParentDataSource( NCReportDataSource* ds );
    NCReportDataSource* parentDataSource() const;
    DataSourceType dataSourceType() const;
    ConnectionType connectionType() const;
    QString connectionID() const;
    LocationType locationType() const;
    QString specText() const;
    QString specification() const;
    QString fileName() const;
    QString rawFileName() const;
    QString recordFilter() const;
    virtual int rowCount() const;
    virtual int columnCount() const;
    bool allowEmpty();
    void setAllowEmpty( bool );

    virtual bool read( NCReportXMLReader* reader );
    virtual bool write( NCReportXMLWriter* writer );

    /*!Opens the datasource and fetches data from it*/
    virtual bool open() =0;
    /*!Closes the datasource opened by open()*/
    virtual bool close() =0;
    // data record navigation
    virtual bool first() =0;
    virtual bool last() =0;
    virtual bool next() =0;
    virtual bool previous() =0;
    /*! Returns true if it has next data row. */
    virtual bool hasNext() =0;
    /*! Jumps to the data row/record at position index, if available, and positions the query on the retrieved record.
    The first record is at position 0. If index = -1 the record positioning is skipped (keeps current record)*/
    virtual bool seek( int index )=0;

    /*! Returns the size of the datasource (number of rows/records) */
    virtual int size() const =0;
    /*! Reloads the data if applicable */
    virtual bool update();

    virtual QVariant value( const QString& columnname, bool* ok=0, int nQTDataRole = -1 ) const =0;
    virtual QVariant value( int column, bool* ok=0, int nQTDataRole = -1 ) const =0;
    virtual bool isValid() const =0;
    /*! Evaluates the data source. This virtual function is useful for parameter, data source, variable evaluation */
    virtual void evaluate( NCReportEvaluator* evaluator);

    virtual QStringList keywordList();
    virtual QStringList columnNames();
       //! Returns a vector with the information about the data fields.
    virtual QVector<NCReportDataFieldInfo> fieldInfo() const;
    virtual QString columnName(int column) const;
    virtual QString columnTitle(int column) const;
    virtual QString rowTitle(int row) const;
    virtual bool hasColumn(int column) const;
    virtual bool hasColumn(const QString& columnname) const;
    virtual int columnIndexByName(const QString& columnname) const;
    /*! Returns if data source can contain parameters. This is good for XML data source*/
    virtual bool hasParameters() const;
    /*! Returns parameter hash if applicable */
    virtual QHash<QString,QVariant>& parameters();
    virtual QVariant getUserFunctionValue(const QVariant &value, const QString& arguments ) const;
    virtual QString columnToolTip(int column) const;

    /*! Internal function for setting done flag */
    void setDone( bool );
    bool isDone() const;
    bool isOpened() const;
    void setOpenRole( OpenRoles );
    OpenRoles openRole() const;
    void setAvailable( bool set );
    bool isAvailable() const;

    int currentRow() const;
    int recno() const;
    const QList<int>& appearanceRoles() const;
    void addAppearanceRole( int role );
    void setUseAppearanceRoles( bool set );
    bool useAppearanceRoles() const;

    void setPrimaryKeyColumn( int column );
    int primaryKeyColumn() const;

    void setForeignKeyValue( const QString& key);
    QString foreignKeyValue() const;

    void setForceUpdateOnStepBackwordInRelation( bool set );
    bool forceUpdateOnStepBackwordInRelation() const;

signals:
    /*!
     * \brief updateRequestFKey
     * \param dsID
     * \param foreignKeyValue
     * Emits update request with foreign key value
     */
    void updateRequestFKey( const QString& dsID, const QString& foreignKeyValue );
    /*!
     * \brief updateRequest
     * \param dsID
     * \param data
     * Emits update request with data source data. This is the original (obsoloete) behaviour.
     */
    void updateRequest( const QString& dsID, const QString& data );

protected:
    void setOpened( bool set );
    NCReportError *error();
    int& recno();
    bool& flagEnd();
    bool& flagBegin();
    QString& specificationRef();

private:
    DataSourceType m_dataSourceType;
    ConnectionType m_connectionType;
    LocationType m_locationType;
    NCReportError *m_error;
    int m_recno;
    bool m_flagEnd, m_flagBegin;
    bool m_allowEmpty;
    QString m_specification;
    QString m_recordFilter;
    QString m_fileName;
    OpenRoles m_openRole;
    QString m_parentID;
    bool m_done;
    bool m_opened;
    bool m_available;
    NCReportDataSource *m_parent;
    QString m_encoding;
    int m_pkColumn;
    QString m_fKey;
    bool m_useAppearanceRoles;
    bool m_forceUpdateOnStepBackwordInRelation;

    QString m_briefDescription;
    QString m_longDescription;
    QString m_id;
    QString m_connectionID;
    QHash<QString,QVariant> m_parameters;
    QList<int> m_appearanceRoles;
};


#endif
